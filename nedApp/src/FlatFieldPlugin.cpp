/* FlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FlatFieldPlugin.h"
#include "likely.h"
#include "Log.h"

#include <dirent.h>
#include <cstring> // strerror()

#include <fstream>
#include <limits>
#include <string>

#define NUM_FLATFIELDPLUGIN_PARAMS ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1))

#define PIXID_ERR       (1 << 31)
#define POSITION_BITS   9
#define MAX_PIXEL_SIZE  (1 << POSITION_BITS)
#define POSITION_MASK   (MAX_PIXEL_SIZE - 1)

#if defined(WIN32) || defined(_WIN32)
#   define PATH_SEPARATOR "\\"
#else
#   define PATH_SEPARATOR "/"
#endif

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 4, "Port name", string, "Dispatcher port name", string, "Correction tables file", string, "Buffer size", int);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importDir, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_FLATFIELDPLUGIN_PARAMS, asynOctetMask | asynInt8ArrayMask | asynFloat64Mask, asynOctetMask | asynInt8ArrayMask | asynFloat64Mask)
    , m_tablesErr(1)
    , m_psEn(0)
    , m_corrEn(0)
    , m_convEn(0)
    , m_tableSizeX(0)
    , m_tableSizeY(0)
{
    if (bufSize > 0) {
        m_bufferSize = bufSize;
    } else {
        m_bufferSize = 4*1024*1024;
        LOG_ERROR("Invalid buffer size requested, setting default %u bytes", m_bufferSize);
    }
    m_buffer = reinterpret_cast<uint8_t *>(malloc(m_bufferSize));
    if (m_buffer == 0) {
        LOG_ERROR("Failed to allocate transformation buffer");
    }

    importFiles(importDir);

    createParam("ImportReport", asynParamOctet, &ImportReport);   // Generate textual file import report
    createParam("ImportDir",    asynParamOctet, &ImportDir, importDir); // Path to correction tables directory
    createParam("BufferSize",   asynParamInt32, &BufferSize, (int)m_bufferSize); // Allocated buffer size
    createParam("Positions",    asynParamInt8Array, &Positions);  // Array of configured positions
    createParam("TablesErr",    asynParamInt32, &TablesErr, m_tablesErr); // Default is 1 until expected positions are configured and checked
    createParam("PsEn",         asynParamInt32, &PsEn, 0);        // Switch to toggle photosum elimination
    createParam("CorrEn",       asynParamInt32, &CorrEn, 0);      // Switch to toggle applying flat field correction
    createParam("ConvEn",       asynParamInt32, &ConvEn, 0);      // Switch to toggle converting data to pixel id format
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels, probably due to missing correction tables
    createParam("CntError",     asynParamInt32, &CntError,  0);   // Number of unknown-error pixels
    createParam("CntPhotoSum",  asynParamInt32, &CntPhotoSum, 0); // Number of photo sum eliminated pixels
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);   // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    createParam("FfMode",       asynParamInt32, &FfMode, FF_PASS_THRU); // FlatField transformation mode (see FlatFieldPlugin::FfMode_t)
    // Next two params are in UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth",  asynParamInt32, &XyFractWidth, 24); // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth",  asynParamInt32, &PsFractWidth, 15); // WRITE - Number of fraction bits in PhotoSum data
    createParam("XRange",       asynParamFloat64, &XRange, 158.0); // WRITE - Maximum X values from detector
    createParam("YRange",       asynParamFloat64, &YRange, 158.0); // WRITE - Maximum Y values from detector
    createParam("PsLowDecBase", asynParamFloat64, &PsLowDecBase, 300.0); // WRITE - PhotoSum lower decrement base
    createParam("PsLowDecLim",  asynParamFloat64, &PsLowDecLim, 400.0); // WRITE - PhotoSum lower decrement limit
    callParamCallbacks();
}

FlatFieldPlugin::~FlatFieldPlugin()
{
    if (m_buffer != 0)
        free(m_buffer);
}

asynStatus FlatFieldPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(CntUnmap, 0);
            setIntegerParam(CntError, 0);
            setIntegerParam(CntSplit, 0);
            callParamCallbacks();
        }
        return asynSuccess;
    } else if (pasynUser->reason == XyFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason == PsFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason == FfMode) {
        if (value != FF_PASS_THRU && value != FF_FLATTEN && value != FF_CONVERT_ONLY)
            return asynError;
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

asynStatus FlatFieldPlugin::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    if (pasynUser->reason == XRange || pasynUser->reason == YRange) {
        if (value <= 0.0)
            return asynError;
    }
    return BaseDispatcherPlugin::writeFloat64(pasynUser, value);
}

asynStatus FlatFieldPlugin::writeInt8Array(asynUser *pasynUser, epicsInt8 *values, size_t nElements)
{
    if (pasynUser->reason == Positions) {
        // asyn will try to initialize the record with 0 elements
        if (nElements > 0) {
            // Disable all tables first
            for (auto it = m_tables.begin(); it != m_tables.end(); it++) {
                it->enabled = false;
            }

            // Enable only tables that meet requirements and are in the user list
            m_tablesErr = 0;
            int psEn, corrEn;
            getIntegerParam(PsEn,   &psEn);
            getIntegerParam(CorrEn, &corrEn);
            for (size_t i = 0; i < nElements; i++) {
                uint32_t position = values[i];
                if (position >= m_tables.size()) {
                    m_tablesErr = 1;
                    LOG_ERROR("Position %d not loaded", position);
                    continue;
                }
                if (psEn == 1 && (m_tables[position].psLowX.get() == 0 || m_tables[position].psUpX.get() == 0)) {
                    m_tablesErr = 1;
                    LOG_ERROR("Photosum tables for position %d not laded", position);
                    continue;
                }
                if (corrEn == 1 && (m_tables[position].corrX.get() == 0 || m_tables[position].corrY.get() == 0)) {
                    m_tablesErr = 1;
                    LOG_ERROR("X and Y correction tables for position %d not laded", position);
                    continue;
                }
                m_tables[position].enabled = true;
            }

            // If there's a request to enable a table but there was a problen meeting
            // the requirements, flag an error - user can use asynReport to diagnoze
            // what tables are loaded
            setIntegerParam(TablesErr, m_tablesErr);
            callParamCallbacks();
        }
        return asynSuccess;
    }
    return BaseDispatcherPlugin::writeInt8Array(pasynUser, values, nElements);
}

asynStatus FlatFieldPlugin::readInt8Array(asynUser *pasynUser, epicsInt8 *values, size_t nElements, size_t *nIn)
{
    if (pasynUser->reason == Positions) {
        int psEn, corrEn;
        getIntegerParam(PsEn, &psEn);
        getIntegerParam(CorrEn, &corrEn);
        *nIn = 0;
        for (size_t i = 0; i < m_tables.size(); i++) {
            bool loaded = false;
            if (corrEn && psEn) {
                // all 4 tables must be loaded
                if (m_tables[i].corrX.get()  != 0 && m_tables[i].corrY.get()  != 0 &&
                    m_tables[i].psLowX.get() != 0 && m_tables[i].psUpX.get() != 0) {
                    loaded = true;
                }
            } else if (corrEn) {
                // X and Y correction tables must be loaded
                if (m_tables[i].corrX.get()  != 0 && m_tables[i].corrY.get()  != 0) {
                    loaded = true;
                }
            } else if (psEn) {
                // X and Y correction tables must be loaded
                if (m_tables[i].psLowX.get() != 0 && m_tables[i].psUpX.get() != 0) {
                    loaded = true;
                }
            }

            if (loaded == true && *nIn < nElements) {
                *values = (uint8_t)i;
                values++;
                *nIn += 1;
            }
        }
        return asynSuccess;
    }
    return BaseDispatcherPlugin::writeInt8Array(pasynUser, values, nElements);
}

asynStatus FlatFieldPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == ImportReport) {
        *nActual = snprintf(value, nChars, "%s", m_reportText.str().c_str());
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void FlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    TransformErrors errors;
    int nSplits = 0;
    DasPacketList newPacketList;

    // Parametrise calculating algorithms based on detector.
    double xRange = 0.0;
    double yRange = 0.0;
    int xyFractWidth = 0;
    int psFractWidth = 0;

    if (m_buffer == 0 || m_tablesErr == 1) {
        LOG_ERROR("Flat field correction disabled due to import error");
        return;
    }

    this->lock();
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(CntUnmap,   &errors.nUnmapped);
    getIntegerParam(CntError,   &errors.nErrors);
    getIntegerParam(CntPhotoSum,&errors.nPhotoSum);
    getIntegerParam(CntSplit,   &nSplits);
    getIntegerParam(FfMode,     &m_ffMode);
    if (getDataMode() != BasePlugin::DATA_MODE_NORMAL)
        m_ffMode = FF_PASS_THRU;
    getIntegerParam(XyFractWidth, &xyFractWidth);
    getIntegerParam(PsFractWidth, &psFractWidth);
    getDoubleParam(XRange,      &xRange);
    getDoubleParam(YRange,      &yRange);

    // This is a trick to avoid locking access to member variables. Since
    // plugin design ensures a single instance of processDataUnlocked()
    // function at any time, we read in values and store them as class members.
    // This makes member variables const for the duration of this function.
    m_xScale = 1.0 * m_tableSizeX / ((1 << xyFractWidth) * xRange);
    m_yScale = 1.0 * m_tableSizeY / ((1 << xyFractWidth) * yRange);
    m_psScale = 1.0 / (1 << psFractWidth);
    // TODO: PsEn, CorrEn, ConvEn
    getDoubleParam(PsLowDecBase,&m_psLowDecBase);
    getDoubleParam(PsLowDecLim, &m_psLowDecLim);

    this->unlock();

    if (m_ffMode == FF_PASS_THRU) {
        sendToPlugins(packetList);
        nProcessed += packetList->size();
    } else {
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;

            newPacketList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < packet->length()) {
                    nSplits++;
                    break;
                }

                // Reuse the original packet if nothing to map
                if (packet->isNeutronData() == false) {
                    newPacketList.push_back(packet);
                    continue;
                }

                nProcessed++;

                // Reserve part of buffer for this packet, it may shrink from original but never grow
                DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                newPacketList.push_back(newPacket);
                bufferOffset += packet->length();

                // Do the X,Y -> pixid transformation - this could be parallelized
                errors += transformPacket(packet, newPacket);
            }

            sendToPlugins(&newPacketList);
            newPacketList.release();
            newPacketList.waitAllReleased();
        }
    }

    nReceived += packetList->size();

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntUnmap,   errors.nUnmapped);
    setIntegerParam(CntPhotoSum,errors.nPhotoSum);
    setIntegerParam(CntError,   errors.nErrors);
    callParamCallbacks();
    this->unlock();
}

FlatFieldPlugin::TransformErrors FlatFieldPlugin::transformPacket(const DasPacket *srcPacket, DasPacket *destPacket)
{
    TransformErrors errors;

    struct AcpcNormalData {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t position_x;
        uint32_t position_y;
        uint32_t photo_sum_x;
        uint32_t photo_sum_y;
    };

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents, nDestEvents;
    const AcpcNormalData *srcEvent= reinterpret_cast<const AcpcNormalData *>(srcPacket->getData(&nEvents));
    DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(destPacket->getData(&nDestEvents));
    nEvents /= (sizeof(AcpcNormalData) / sizeof(uint32_t));
    nDestEvents = 0;

    while (nEvents-- > 0) {
        // Skip error pixels
        if (srcEvent->position_index & PIXID_ERR) {
            errors.nErrors++;
            srcEvent++;
            continue;;
        }

        // Convert raw values into ones used by calculations
        uint32_t tof = srcEvent->time_of_flight;
        uint32_t position = srcEvent->position_index & 0x3FFF;
        double x = srcEvent->position_x * m_xScale;
        double y = srcEvent->position_y * m_yScale;
        double photosum_x = srcEvent->photo_sum_x * m_psScale;
        // unused srcEvent->photo_sum_y
        srcEvent++;

        // Check photo sum
        if (checkPhotoSumLimits(x, y, photosum_x, position) == false) {
            errors.nPhotoSum++;
            continue;
        }

        // Convert to TOF,pixelid tupple
        destEvent->tof = tof;
        destEvent->pixelid = xyToPixel(x, y, position);

        if ((destEvent->pixelid & 0x3FFFF) == 0) {
            errors.nUnmapped++;
            continue;
        }

        destEvent++;
        nDestEvents++;
    }

    destPacket->payload_length += nDestEvents * sizeof(DasPacket::Event);

    return errors;
}

uint32_t FlatFieldPlugin::xyToPixel(double x, double y, uint32_t position)
{
    if (likely(m_ffMode == FF_FLATTEN)) {
        if (position >= m_tables.size())
            return 0;

        std::shared_ptr<FlatFieldTable> xtable = m_tables[position].corrX;
        std::shared_ptr<FlatFieldTable> ytable = m_tables[position].corrY;
        if (m_tables[position].enabled == false || xtable.get() == 0 || ytable.get() == 0)
            return 0;

        unsigned xp = x;
        unsigned yp = y;

        // All tables of the same size, safe to compare against just one
        if (x < 0.0 || xp >= xtable->sizeX || y < 0.0 || yp >= xtable->sizeY)
            return 0;

        // All checks passed - do the correction
        double dx = (xp == 0 || xp == (xtable->sizeX - 1)) ? 0 : x - xp;
        double dy = (yp == 0 || yp == (ytable->sizeY - 1)) ? 0 : y - yp;

        x -= (dx * xtable->data[xp+1][yp]) + ((1 - dx) * xtable->data[xp][yp]);
        y -= (dy * ytable->data[xp][yp+1]) + ((1 - dy) * ytable->data[xp][yp]);
    }

    return (position << (2*POSITION_BITS)) | (((int)x & POSITION_MASK) << POSITION_BITS) | (((int)y & POSITION_MASK));
}

bool FlatFieldPlugin::checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position)
{
    if (unlikely(m_ffMode != FF_FLATTEN))
        return true;

    if (position >= m_tables.size())
        return false;

    std::shared_ptr<FlatFieldTable> upperLimits = m_tables[position].psUpX;
    std::shared_ptr<FlatFieldTable> lowerLimits = m_tables[position].psLowX;
    if (m_tables[position].enabled == false || upperLimits.get() == 0 || lowerLimits.get() == 0)
        return false;

    unsigned xp = x;
    unsigned yp = y;

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= upperLimits->sizeX || y < 0.0 || yp >= upperLimits->sizeY)
        return 0;

    double upperLimit = upperLimits->data[xp][yp];
    double lowerLimit = lowerLimits->data[xp][yp];

    return (lowerLimit <= photosum_x && photosum_x <= upperLimit);
}
void FlatFieldPlugin::importFiles(const std::string &path)
{
    struct dirent entry, *result;
    bool foundCorrTables = false;
    bool foundPsTables = false;

    m_reportText << "Importing files from " << path << ":" << std::endl;

    DIR *dir = opendir(path.c_str());
    if (dir == NULL) {
        LOG_ERROR("Failed to read import directory '%s': %s", path.c_str(), strerror(errno));
        m_reportText << "none (failed to read directory)" << std::endl;
        return;
    }

    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
        std::string filename = entry.d_name;
        if (filename != ".." && filename != ".") {
            std::shared_ptr<FlatFieldTable> table(new FlatFieldTable());
            if (table.get() == 0) {
                LOG_ERROR("Failed to initialize table");
                m_reportText << " * " << filename << ": error - failed to allocate table" << std::endl;
                continue;
            }
            if (table->import(path + PATH_SEPARATOR + filename) == false) {
                LOG_ERROR("Failed to import file '%s': %s", filename.c_str(), table->getImportError().c_str());
                m_reportText << " * " << filename << ": error - " << table->getImportError() << std::endl;
                continue;
            }

            // Initialize the common table size variable
            if (m_tableSizeX == 0) {
                m_tableSizeX = table->sizeX;
                m_tableSizeY = table->sizeY;
            }

            // Ensure all tables are of the same size
            if (m_tableSizeX != table->sizeX || m_tableSizeY != table->sizeY) {
                LOG_ERROR("Table size mismatch");
                m_reportText << " * " << filename << ": error - table size mismatch" << std::endl;
                continue;
            }

            // Make sure there's enough place in the m_corrTables
            if (m_tables.size() <= table->position)
                m_tables.resize(table->position + 1);

            // Push table to tables vector
            if (table->type == FlatFieldTable::TYPE_X_CORR) {
                if (m_tables[table->position].corrX.get() == 0) {
                    m_tables[table->position].corrX = table;
                    foundCorrTables = true;
                } else {
                    LOG_ERROR("Correction X table already loaded for position %u", table->position);
                    m_reportText << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_Y_CORR) {
                if (m_tables[table->position].corrY.get() == 0) {
                    m_tables[table->position].corrY = table;
                    foundCorrTables = true;
                } else {
                    LOG_ERROR("Correction Y table already loaded for position %u", table->position);
                    m_reportText << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_LOW) {
                if (m_tables[table->position].psLowX.get() == 0) {
                    m_tables[table->position].psLowX = table;
                    foundPsTables = true;
                } else {
                    LOG_ERROR("Photosum low X table already loaded for position %u", table->position);
                    m_reportText << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_UP) {
                if (m_tables[table->position].psUpX.get() == 0) {
                    m_tables[table->position].psUpX = table;
                    foundPsTables = true;
                } else {
                    LOG_ERROR("Photosum upper X table already loaded for position %u", table->position);
                    m_reportText << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            }

            LOG_INFO("Imported file '%s'", filename.c_str());
            m_reportText << " * " << filename << ": imported" << std::endl;
        }
    }
    closedir(dir);

    m_reportText << std::endl << "Position loaded:" << std::endl;

    // Check tables constraints for each position:
    // * all tables of the same size - already checked when tables created
    // * X and Y correction tables loaded if using correction (foundCorrTables==true)
    // * Lower and upper X photosum tables loaded if using photosum (foundPsTable==true)
    bool nPositions = 0;
    for (uint32_t i = 0; i < m_tables.size(); i++) {

        if (m_tables[i].corrX.get() == 0 && m_tables[i].corrY.get() == 0 && m_tables[i].psLowX.get() == 0 && m_tables[i].psUpX.get() == 0)
            continue;

        nPositions++;
        m_reportText << " * position " << i << ":" << std::endl;

        if (foundCorrTables == true) {
            if (m_tables[i].corrX.get() == 0) {
                LOG_ERROR("Missing X correction table for position %u", i);
                m_reportText << "   - X correction table missing" << std::endl;
            } else {
                m_reportText << "   - X correction table loaded" << std::endl;
            }
            if (m_tables[i].corrY.get() == 0) {
                LOG_ERROR("Missing Y correction table for position %u", i);
                m_reportText << "   - Y correction table missing" << std::endl;
            } else {
                m_reportText << "   - Y correction table loaded" << std::endl;
            }
        }

        if (foundPsTables == true) {
            if (m_tables[i].psLowX.get() == 0) {
                LOG_ERROR("Missing lower X photosum correction table for position %u", i);
                m_reportText << "   - lower X photosum table missing" << std::endl;
            } else {
                m_reportText << "   - lower X photosum table loaded" << std::endl;
            }
            if (m_tables[i].psUpX.get() == 0) {
                LOG_ERROR("Missing upper X photosum correction table for position %u", i);
                m_reportText << "   - upper X photosum table missing" << std::endl;
            } else {
                m_reportText << "   - upper X photosum table loaded" << std::endl;
            }
        }
    }

    if (nPositions == 0) {
        LOG_ERROR("Found imported positions");
        m_reportText << std::endl << " * none loaded" << std::endl;
    }
}

void FlatFieldPlugin::report(FILE *fp, int details)
{
    std::string text = m_reportText.str();
    fwrite(text.c_str(), 1, text.length(), fp);
    return BasePlugin::report(fp, details);
}
