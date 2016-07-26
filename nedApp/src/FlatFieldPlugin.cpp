/* FlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Bits.h"
#include "FlatFieldPlugin.h"
#include "likely.h"
#include "Log.h"

#include <dirent.h>
#include <cmath>
#include <cstring> // strerror()

#include <fstream>
#include <limits>
#include <string>

#define NUM_FLATFIELDPLUGIN_PARAMS ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1))

#if defined(WIN32) || defined(_WIN32)
#   define PATH_SEPARATOR "\\"
#else
#   define PATH_SEPARATOR "/"
#endif

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 4, "Port name", string, "Dispatcher port name", string, "Correction tables file", string, "Buffer size", int);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importDir, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_FLATFIELDPLUGIN_PARAMS, asynOctetMask | asynInt8ArrayMask | asynFloat64Mask, asynOctetMask | asynInt8ArrayMask | asynFloat64Mask)
    , m_tablesErr(1)
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

    createParam("ImportReport", asynParamOctet, &ImportReport);         // Generate textual file import report
    createParam("ImportDir",    asynParamOctet, &ImportDir, importDir); // Path to correction tables directory
    createParam("BufferSize",   asynParamInt32, &BufferSize, (int)m_bufferSize); // Allocated buffer size
    createParam("Positions",    asynParamInt8Array, &Positions);        // Array of configured positions
    createParam("PositionsReport", asynParamOctet, &PositionsReport);   // Array of configured positions
    createParam("TablesErr",    asynParamInt32, &TablesErr, m_tablesErr); // Default is 1 until expected positions are configured and checked
    createParam("PsEn",         asynParamInt32, &PsEn, 0);              // Switch to toggle photosum elimination
    createParam("CorrEn",       asynParamInt32, &CorrEn, 0);            // Switch to toggle applying flat field correction
    createParam("ConvEn",       asynParamInt32, &ConvEn, 0);            // Switch to toggle converting data to pixel id format
    createParam("CntVetoEvents",asynParamInt32, &CntVetoEvents, 0);     // Number of vetoed events
    createParam("CntGoodEvents",asynParamInt32, &CntGoodEvents, 0);     // Number of calculated events
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);         // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);             // Reset counters
    // Next two params are in UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth", asynParamInt32, &XyFractWidth, 24);     // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth", asynParamInt32, &PsFractWidth, 15);     // WRITE - Number of fraction bits in PhotoSum data
    createParam("XMaxIn",       asynParamFloat64, &XMaxIn, 158.0);      // WRITE - Defines input X range
    createParam("YMaxIn",       asynParamFloat64, &YMaxIn, 158.0);      // WRITE - Defines input Y range
    createParam("XMaxOut",      asynParamInt32, &XMaxOut, 511);         // WRITE - Converted max X value
    createParam("YMaxOut",      asynParamInt32, &YMaxOut, 511);         // WRITE - Converted max Y value
    createParam("TablesSizeX",  asynParamInt32, &TablesSizeX, (int)m_tableSizeX); // READ - All tables X size
    createParam("TablesSizeY",  asynParamInt32, &TablesSizeY, (int)m_tableSizeY); // READ - All tables Y size
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
            setIntegerParam(RxCount, 0);
            setIntegerParam(ProcCount, 0);
            setIntegerParam(CntGoodEvents, 0);
            setIntegerParam(CntVetoEvents, 0);
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
    } else if (pasynUser->reason == XMaxOut || pasynUser->reason == YMaxOut) {
        if (value < 1 || value >= 1024)
            return asynError;
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

asynStatus FlatFieldPlugin::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    if (pasynUser->reason == XMaxIn || pasynUser->reason == YMaxIn) {
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
            for (size_t i = 0; i < m_tables.size(); i++) {
                m_tables[i].enabled = false;
                if (m_tables[i].psLowX.get() == 0 && m_tables[i].psUpX.get() == 0 &&
                    m_tables[i].corrX.get()  == 0 && m_tables[i].corrY.get() == 0) {
                    m_tables[i].init = false;
                }
            }

            // Copy list of enabled list to m_tables
            m_tablesErr = 0;
            bool psEn, corrEn;
            getBooleanParam(PsEn,   &psEn);
            getBooleanParam(CorrEn, &corrEn);
            for (size_t i = 0; i < nElements; i++) {
                uint32_t position = values[i];
                if (position >= m_tables.size()) {
                    m_tablesErr = 1;
                    LOG_ERROR("Position %d not loaded", position);
                    m_tables.resize(position+1);
                }
                m_tables[position].enabled = true;
                m_tables[position].init = true;

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
        bool psEn, corrEn;
        getBooleanParam(PsEn, &psEn);
        getBooleanParam(CorrEn, &corrEn);
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
        *nActual = snprintf(value, nChars, "%s", m_importReport.c_str());
        return asynSuccess;
    } else if (pasynUser->reason == PositionsReport) {
        bool psEn, corrEn;
        getBooleanParam(PsEn, &psEn);
        getBooleanParam(CorrEn, &corrEn);
        std::string report = generatePositionsReport(psEn, corrEn);
        *nActual = snprintf(value, nChars, "%s", report.c_str());
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void FlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    TransformErrors errors;
    int nVeto = 0;
    int nGood = 0;
    int nSplits = 0;
    DasPacketList newPacketList;
    bool psEn = false;
    bool corrEn = false;
    bool convEn = false;
    double xMaxIn, yMaxIn;
    int xMaxOut, yMaxOut;
    bool passthru = false;

    // Parametrise calculating algorithms based on detector.
    int xyFractWidth = 0;
    int psFractWidth = 0;

    if (m_buffer == 0 || m_tablesErr == 1) {
        LOG_ERROR("Flat field correction disabled due to import error");
        return;
    }

    this->lock();
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(CntVetoEvents,  &nVeto);
    getIntegerParam(CntGoodEvents,  &nGood);
    getIntegerParam(CntSplit,       &nSplits);
    getIntegerParam(XyFractWidth,   &xyFractWidth);
    getIntegerParam(PsFractWidth,   &psFractWidth);
    getDoubleParam(XMaxIn,          &xMaxIn);
    getDoubleParam(YMaxIn,          &yMaxIn);
    getIntegerParam(XMaxOut,        &xMaxOut); // Guaranteed to be in 10-bit range
    getIntegerParam(YMaxOut,        &yMaxOut); // Guaranteed to be in 10-bit range
    getBooleanParam(PsEn,           &psEn);
    getBooleanParam(ConvEn,         &convEn);
    getBooleanParam(CorrEn,         &corrEn);

    // This is a trick to avoid locking access to member variables. Since
    // plugin design ensures a single instance of processDataUnlocked()
    // function at any time, we read in values and store them as class members.
    // This makes member variables const for the duration of this function.
    m_psScale = 1.0 / (1 << psFractWidth);
    m_xScaleIn = 1.0 * (m_tableSizeX - 1) / ((1 << xyFractWidth) * xMaxIn);
    m_yScaleIn = 1.0 * (m_tableSizeY - 1) / ((1 << xyFractWidth) * yMaxIn);
    m_xScaleOut = 1.0 * Bits::roundUpPower2(yMaxOut) * xMaxOut / (m_tableSizeX - 1);
    m_yScaleOut = 1.0 *                                yMaxOut / (m_tableSizeY - 1);
    m_xMaskOut = (Bits::roundUpPower2(xMaxOut) - 1) * Bits::roundUpPower2(yMaxOut);
    m_yMaskOut = (Bits::roundUpPower2(yMaxOut) - 1);
    this->unlock();

    if (psEn == false && corrEn == false && convEn == false)
        passthru = true;

    // Optimize passthru mode
    if (passthru == true) {
        sendToPlugins(packetList);
    } else {
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;
            DasPacketList newPacketList;
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

                // Process the packet
                processPacket(packet, newPacket, corrEn, psEn, convEn, nGood, nVeto);
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
    setIntegerParam(CntVetoEvents, nVeto);
    setIntegerParam(CntGoodEvents, nGood);
    callParamCallbacks();
    this->unlock();
}

void FlatFieldPlugin::processPacket(const DasPacket *srcPacket, DasPacket *destPacket, bool correct, bool photosum, bool convert, int &nGood, int &nVeto)
{

    // This structure represents an event from 2D detectors. It
    // matches AcpcDataPacket::NormalEvent and BnlDataPacket::NormalEvent.
    struct Event {
        uint32_t tof;
        uint32_t position;
        uint32_t x;
        uint32_t y;
        uint32_t photosum_x;
        uint32_t photosum_y;
    };

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents;
    const Event *srcEvent= reinterpret_cast<const Event *>(srcPacket->getData(&nEvents));
    nEvents /= (sizeof(Event) / sizeof(uint32_t));

    uint32_t nDestEvents;
    uint32_t *newPayload = reinterpret_cast<uint32_t *>(destPacket->getData(&nDestEvents));
    nDestEvents = 0;

    while (nEvents-- > 0) {
        double x = srcEvent->x * m_xScaleIn;
        double y = srcEvent->y * m_yScaleIn;
        double photosum_x = srcEvent->photosum_x * m_psScale;
        uint32_t position = srcEvent->position;

        // Check photo sum first
        if (photosum == true && checkPhotoSumLimits(x, y, photosum_x, position) == false) {
            nVeto++;
            srcEvent++;
            continue;
        }

        // Apply flat field correction
        if (correct == true && correctPosition(x, y, position) == false) {
            nVeto++;
            srcEvent++;
            continue;
        }

        // Not a veto, check what output format should we do
        if (convert == true) {
            DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(newPayload);

            destEvent->tof = srcEvent->tof & 0xFFFFFFF;
            destEvent->pixelid = (srcEvent->position & 0xFF) << 20; // incompatible with dcomserver, he shifts for 16 or 18 bits depending on HighRes
            destEvent->pixelid |= ((uint32_t)round(x * m_xScaleOut) & m_xMaskOut);
            destEvent->pixelid |= ((uint32_t)round(y * m_yScaleOut) & m_yMaskOut);

            newPayload += sizeof(DasPacket::Event) / sizeof(uint32_t);
            destPacket->payload_length += sizeof(DasPacket::Event);
        } else {
            Event *destEvent = reinterpret_cast<Event *>(newPayload);

            destEvent->tof = srcEvent->tof & 0xFFFFFFF;
            destEvent->position = srcEvent->position;
            destEvent->x = x / m_xScaleIn;
            destEvent->y = y / m_yScaleIn;
            destEvent->photosum_x = srcEvent->photosum_x;
            destEvent->photosum_y = srcEvent->photosum_y;

            newPayload += sizeof(Event) / sizeof(uint32_t);
            destPacket->payload_length += sizeof(Event);
        }

        nGood++;
        srcEvent++;
    }
}

bool FlatFieldPlugin::correctPosition(double &x, double &y, uint32_t position)
{
    if (position >= m_tables.size())
        return false;

    std::shared_ptr<FlatFieldTable> xtable = m_tables[position].corrX;
    std::shared_ptr<FlatFieldTable> ytable = m_tables[position].corrY;
    if (m_tables[position].enabled == false || xtable.get() == 0 || ytable.get() == 0)
        return false;

    unsigned xp = x;
    unsigned yp = y;

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= (xtable->sizeX-1) || y < 0.0 || yp >= (xtable->sizeY-1))
        return false;

    // All checks passed - do the correction
    double dx = (xp == 0 || xp == (xtable->sizeX - 1)) ? 0 : x - xp;
    double dy = (yp == 0 || yp == (ytable->sizeY - 1)) ? 0 : y - yp;

    x -= (dx * xtable->data[xp+1][yp]) + ((1 - dx) * xtable->data[xp][yp]);
    y -= (dy * ytable->data[xp][yp+1]) + ((1 - dy) * ytable->data[xp][yp]);

    return true;
}

bool FlatFieldPlugin::checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position)
{
    if (position >= m_tables.size())
        return false;

    std::shared_ptr<FlatFieldTable> upperLimits = m_tables[position].psUpX;
    std::shared_ptr<FlatFieldTable> lowerLimits = m_tables[position].psLowX;
    if (m_tables[position].enabled == false || upperLimits.get() == 0 || lowerLimits.get() == 0)
        return false;

    unsigned xp = x;
    unsigned yp = y;

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= (upperLimits->sizeX-1) || y < 0.0 || yp >= (upperLimits->sizeY-1))
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
    std::ostringstream importReport;

    importReport << "Importing files from " << path << ":" << std::endl;

    DIR *dir = opendir(path.c_str());
    if (dir == NULL) {
        LOG_ERROR("Failed to read import directory '%s': %s", path.c_str(), strerror(errno));
        importReport << "none (failed to read directory)" << std::endl;
        return;
    }

    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
        std::string filename = entry.d_name;
        if (filename != ".." && filename != ".") {
            std::shared_ptr<FlatFieldTable> table(new FlatFieldTable());
            if (table.get() == 0) {
                LOG_ERROR("Failed to initialize table");
                importReport << " * " << filename << ": error - failed to allocate table" << std::endl;
                continue;
            }
            if (table->import(path + PATH_SEPARATOR + filename) == false) {
                LOG_ERROR("Failed to import file '%s': %s", filename.c_str(), table->getImportError().c_str());
                importReport << " * " << filename << ": error - " << table->getImportError() << std::endl;
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
                importReport << " * " << filename << ": error - table size mismatch" << std::endl;
                continue;
            }

            // Make sure there's enough place in the m_corrTables
            if (m_tables.size() <= table->position) {
                m_tables.resize(table->position + 1);
                m_tables[table->position].init = true;
            }

            // Push table to tables vector
            if (table->type == FlatFieldTable::TYPE_X_CORR) {
                if (m_tables[table->position].corrX.get() == 0) {
                    m_tables[table->position].corrX = table;
                    foundCorrTables = true;
                } else {
                    LOG_ERROR("Correction X table already loaded for position %u", table->position);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_Y_CORR) {
                if (m_tables[table->position].corrY.get() == 0) {
                    m_tables[table->position].corrY = table;
                    foundCorrTables = true;
                } else {
                    LOG_ERROR("Correction Y table already loaded for position %u", table->position);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_LOW) {
                if (m_tables[table->position].psLowX.get() == 0) {
                    m_tables[table->position].psLowX = table;
                    foundPsTables = true;
                } else {
                    LOG_ERROR("Photosum low X table already loaded for position %u", table->position);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_UP) {
                if (m_tables[table->position].psUpX.get() == 0) {
                    m_tables[table->position].psUpX = table;
                    foundPsTables = true;
                } else {
                    LOG_ERROR("Photosum upper X table already loaded for position %u", table->position);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            }

            LOG_INFO("Imported file '%s'", filename.c_str());
            importReport << " * " << filename << ": imported" << std::endl;
        }
    }
    closedir(dir);
    m_importReport = importReport.str();

    // Check tables constraints for each position:
    // * all tables of the same size - already checked when tables created
    // * X and Y correction tables loaded if using correction (foundCorrTables==true)
    // * Lower and upper X photosum tables loaded if using photosum (foundPsTable==true)
    bool nPositions = 0;
    for (uint32_t i = 0; i < m_tables.size(); i++) {

        if (m_tables[i].corrX.get() == 0 && m_tables[i].corrY.get() == 0 && m_tables[i].psLowX.get() == 0 && m_tables[i].psUpX.get() == 0)
            continue;

        nPositions++;

        if (foundCorrTables == true) {
            if (m_tables[i].corrX.get() == 0) {
                LOG_ERROR("Missing X correction table for position %u", i);
            }
            if (m_tables[i].corrY.get() == 0) {
                LOG_ERROR("Missing Y correction table for position %u", i);
            }
        }

        if (foundPsTables == true) {
            if (m_tables[i].psLowX.get() == 0) {
                LOG_ERROR("Missing lower X photosum correction table for position %u", i);
            }
            if (m_tables[i].psUpX.get() == 0) {
                LOG_ERROR("Missing upper X photosum correction table for position %u", i);
            }
        }
    }
}

std::string FlatFieldPlugin::generatePositionsReport(bool psEn, bool corrEn)
{
    std::ostringstream report;
    for (size_t i = 0; i < m_tables.size(); i++) {
        // Skip sparse vector entries
        if (m_tables[i].init == false)
            continue;

        report << "Position " << i << (m_tables[i].enabled == false ? " NOT " : " ") << "enabled:" << std::endl;
        if (psEn) {
            report << " * upper X photosum table " << (m_tables[i].psUpX.get()  != 0 ? "loaded" : "MISSING") << std::endl;
            report << " * lower X photosum table " << (m_tables[i].psLowX.get() != 0 ? "loaded" : "MISSING") << std::endl;
        }
        if (corrEn) {
            report << " * X correction table " << (m_tables[i].corrX.get()  != 0 ? "loaded" : "MISSING") << std::endl;
            report << " * Y correction table " << (m_tables[i].corrY.get()  != 0 ? "loaded" : "MISSING") << std::endl;
        }
    }
    return report.str();
}

void FlatFieldPlugin::report(FILE *fp, int details)
{
    bool psEn, corrEn;
    getBooleanParam(PsEn, &psEn);
    getBooleanParam(CorrEn, &corrEn);

    std::string positionsReport = generatePositionsReport(psEn, corrEn);
    fwrite(positionsReport.c_str(), 1, positionsReport.length(), fp);
    fwrite(m_importReport.c_str(), 1, m_importReport.length(), fp);
    return BasePlugin::report(fp, details);
}
