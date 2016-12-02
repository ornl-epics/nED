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

#define MAX_POSITIONS 100
#define NUM_FLATFIELDPLUGIN_PARAMS ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1)) + (MAX_POSITIONS * 6)

#if defined(WIN32) || defined(_WIN32)
#   define PATH_SEPARATOR "\\"
#else
#   define PATH_SEPARATOR "/"
#endif

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 4, "Port name", string, "Dispatcher port name", string, "Correction tables file", string, "Buffer size", int);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importDir, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_FLATFIELDPLUGIN_PARAMS, asynOctetMask | asynFloat64Mask, asynOctetMask | asynFloat64Mask)
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
    createParam("PsEn",         asynParamInt32, &PsEn, 0);              // Switch to toggle photosum elimination
    createParam("CorrEn",       asynParamInt32, &CorrEn, 0);            // Switch to toggle applying flat field correction
    createParam("ConvEn",       asynParamInt32, &ConvEn, 0);            // Switch to toggle converting data to pixel id format
    createParam("CntGoodEvents",asynParamInt32, &CntGoodEvents, 0);     // Number of calculated events
    createParam("CntPosVetos",  asynParamInt32, &CntPosVetos, 0);       // Number of bad position vetos
    createParam("CntRangeVetos",asynParamInt32, &CntRangeVetos, 0);     // Number of bad X,Y range vetos
    createParam("CntPosCfgVetos",asynParamInt32, &CntPosCfgVetos, 0);   // Number of position configuration vetos
    createParam("CntPsVetos",   asynParamInt32, &CntPsVetos, 0);        // Number of photosum discriminated events
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

    int nParams = ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1));

    for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
        PosEnable[it->second.position_id] = -1;
        if (it->second.nTables > 0) {
            int param;
            char name[16];

            if ((nParams + 4) > NUM_FLATFIELDPLUGIN_PARAMS) {
                LOG_ERROR("Parameter table size not big enough, skipping some positions");
                break;
            }

            snprintf(name, sizeof(name), "Pos%u:Enable", it->second.position_id);
            createParam(name, asynParamInt32, &param, 0);
            PosEnable[it->second.position_id] = param;

            snprintf(name, sizeof(name), "Pos%u:Id", it->second.position_id);
            createParam(name, asynParamInt32, &param, 0);
            PosId[it->second.position_id] = param;

            snprintf(name, sizeof(name), "Pos%u:CorrX", it->second.position_id);
            createParam(name, asynParamInt32, &param, 0);
            PosCorrX[it->second.position_id] = param;

            snprintf(name, sizeof(name), "Pos%u:CorrY", it->second.position_id);
            createParam(name, asynParamInt32, &param, 0);
            PosCorrY[it->second.position_id] = param;

            snprintf(name, sizeof(name), "Pos%u:PsUpX", it->second.position_id);
            createParam(name, asynParamInt32, &param, 0);
            PosPsUpX[it->second.position_id] = param;

            snprintf(name, sizeof(name), "Pos%u:PsLowX", it->second.position_id);
            createParam(name, asynParamInt32, &param, 0);
            PosPsLowX[it->second.position_id] = param;
        }
    }

    callParamCallbacks();
}

FlatFieldPlugin::~FlatFieldPlugin()
{
    if (m_buffer != 0)
        free(m_buffer);
}

asynStatus FlatFieldPlugin::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
    for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
        if (pasynUser->reason == PosEnable[it->second.position_id]) {
            *value = it->second.enabled;
            return asynSuccess;
        } else if (pasynUser->reason == PosId[it->second.position_id]) {
            *value = (int)it->second.position_id;
            return asynSuccess;
        } else if (pasynUser->reason == PosCorrX[it->second.position_id]) {
            *value = (it->second.corrX != 0);
            return asynSuccess;
        } else if (pasynUser->reason == PosCorrY[it->second.position_id]) {
            *value = (it->second.corrY != 0);
            return asynSuccess;
        } else if (pasynUser->reason == PosPsUpX[it->second.position_id]) {
            *value = (it->second.psLowX != 0);
            return asynSuccess;
        } else if (pasynUser->reason == PosPsLowX[it->second.position_id]) {
            *value = (it->second.psUpX != 0);
            return asynSuccess;
        }
    }
    return BaseDispatcherPlugin::readInt32(pasynUser, value);
}

asynStatus FlatFieldPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(RxCount, 0);
            setIntegerParam(ProcCount, 0);
            setIntegerParam(CntGoodEvents, 0);
            setIntegerParam(CntPosVetos, 0);
            setIntegerParam(CntRangeVetos, 0);
            setIntegerParam(CntPosCfgVetos, 0);
            setIntegerParam(CntPsVetos, 0);
            setIntegerParam(CntSplit, 0);
            callParamCallbacks();
            m_evCounters.reset();
        }
        return asynSuccess;
    } else if (pasynUser->reason == XyFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason == PsFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason ==  || pasynUser->reason == YMaxOut) {
        if (value < 1 || value >= 1024)
            return asynError;
    } else if (pasynUser->reason == CorrEn) {
        // Check that we have both flat-field correction tables for all positions
        if (value != 0) {
            for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
                if (it->second.nTables == 0 || it->second.enabled == false) continue;
                if (it->second.corrX == 0 || it->second.corrY == 0) {
                    LOG_ERROR("Disabling position %u due to lack of flat field correction tables", it->second.position_id);
                    setIntegerParam(PosEnable[it->second.position_id], 0);
                }
            }
            callParamCallbacks();
        }
    } else if (pasynUser->reason == PsEn) {
        if (value != 0) {
            for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
                if (it->second.nTables == 0 || it->second.enabled == false) continue;
                if (it->second.psLowX == 0 || it->second.psUpX == 0) {
                    LOG_ERROR("Disabling position %u due to lack of photo sum tables", it->second.position_id);
                    setIntegerParam(PosEnable[it->second.position_id], 0);
                }
            }
            callParamCallbacks();
        }
    } else {
        for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
            if (pasynUser->reason == PosEnable[it->second.position_id]) {
                asynStatus ret = asynSuccess;
                int psEn, corrEn;
                getIntegerParam(PsEn, &psEn);
                getIntegerParam(CorrEn, &corrEn);
                if (value == 1 && psEn == 1 && (it->second.psLowX == 0 || it->second.psUpX == 0)) {
                    LOG_ERROR("Failed to enable position %u, missing photo sum tables", it->second.position_id);
                    value = 0;
                    ret = asynError;
                } else if (value == 1 && corrEn == 1 && (it->second.corrX == 0 || it->second.corrY == 0)) {
                    LOG_ERROR("Failed to enable position %u, missing correction tables", it->second.position_id);
                    value = 0;
                    ret = asynError;
                } else {
                    it->second.enabled = (value != 0);
                }
                setIntegerParam(PosEnable[it->second.position_id], value);
                callParamCallbacks();
                return ret;
            }
        }
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

asynStatus FlatFieldPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == ImportReport) {
        *nActual = snprintf(value, nChars, "%s", m_importReport.c_str());
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void FlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
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

    if (m_buffer == 0) {
        LOG_ERROR("Flat field correction disabled, no memory");
        return;
    }

    this->lock();
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(CntSplit,       &nSplits);
    getIntegerParam(XyFractWidth,   &xyFractWidth);
    getIntegerParam(PsFractWidth,   &psFractWidth);
    getDoubleParam(XMaxIn,          &xMaxIn);
    getDoubleParam(YMaxIn,          &yMaxIn);
    getIntegerParam(XMaxOut,        &xMaxOut);
    getIntegerParam(YMaxOut,        &yMaxOut);
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
                if (remain < packet->getLength()) {
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
                bufferOffset += packet->getLength();

                // Process the packet
                EventCounters counters;
                processPacket(packet, newPacket, corrEn, psEn, convEn, counters);
                m_evCounters += counters;
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
    setIntegerParam(CntGoodEvents,  m_evCounters.nGood);
    setIntegerParam(CntPosVetos,    m_evCounters.nPosition);
    setIntegerParam(CntRangeVetos,  m_evCounters.nRange);
    setIntegerParam(CntPosCfgVetos, m_evCounters.nPosCfg);
    setIntegerParam(CntPsVetos,     m_evCounters.nPhotosum);
    callParamCallbacks();
    this->unlock();
}

void FlatFieldPlugin::processPacket(const DasPacket *srcPacket, DasPacket *destPacket, bool correct, bool photosum, bool convert, EventCounters &counters)
{
    uint32_t nVetoes[5] = {}; // Array of vetoes - needs to match number of VetoType entries

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
    (void)srcPacket->copyHeader(destPacket, srcPacket->getLength());

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
        if (photosum == true) {
            VetoType veto = checkPhotoSumLimits(x, y, photosum_x, position);
            if (veto != VETO_NO) {
                nVetoes[veto]++;
                srcEvent++;
                continue;
            }
        }

        // Apply flat field correction
        if (correct == true) {
            VetoType veto = correctPosition(x, y, position);
            if (veto != VETO_NO) {
                nVetoes[veto]++;
                srcEvent++;
                continue;
            }
        }

        // Not a veto, check what output format should we do
        if (convert == true) {
            // Detect an overlap between configured position index and local pixel id
            uint32_t pixelMask = m_xMaskOut | m_yMaskOut;
            if (position & pixelMask) {
                nVetoes[VETO_POSITION_CFG]++;
                srcEvent++;
                continue;
            }

            DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(newPayload);

            destEvent->tof = srcEvent->tof & 0xFFFFFFF;
            destEvent->pixelid = position;
            destEvent->pixelid |= ((uint32_t)round(x * m_xScaleOut) & m_xMaskOut);
            destEvent->pixelid |= ((uint32_t)round(y * m_yScaleOut) & m_yMaskOut);

            newPayload += sizeof(DasPacket::Event) / sizeof(uint32_t);
            destPacket->payload_length += sizeof(DasPacket::Event);
        } else {
            Event *destEvent = reinterpret_cast<Event *>(newPayload);

            destEvent->tof = srcEvent->tof & 0xFFFFFFF;
            destEvent->position = position & 0xFF;
            destEvent->x = x / m_xScaleIn;
            destEvent->y = y / m_yScaleIn;
            destEvent->photosum_x = srcEvent->photosum_x;
            destEvent->photosum_y = srcEvent->photosum_y;

            newPayload += sizeof(Event) / sizeof(uint32_t);
            destPacket->payload_length += sizeof(Event);
        }

        nVetoes[VETO_NO]++;
        srcEvent++;
    }

    counters.nGood     = nVetoes[VETO_NO];
    counters.nPosition = nVetoes[VETO_POSITION];
    counters.nRange    = nVetoes[VETO_RANGE];
    counters.nPosCfg   = nVetoes[VETO_POSITION_CFG];
    counters.nPhotosum = nVetoes[VETO_PHOTOSUM];

    counters.nTotal = counters.nGood;
    for (size_t i=0; i<sizeof(nVetoes)/sizeof(uint32_t); i++) {
        counters.nTotal += nVetoes[i];
    }
}

FlatFieldPlugin::VetoType FlatFieldPlugin::correctPosition(double &x, double &y, uint32_t position)
{
    std::shared_ptr<FlatFieldTable> xtable = m_tables[position].corrX;
    std::shared_ptr<FlatFieldTable> ytable = m_tables[position].corrY;
    if (!xtable || !ytable || m_tables[position].enabled == false)
        return VETO_POSITION;

    unsigned xp = x;
    unsigned yp = y;

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= (xtable->sizeX-1) || y < 0.0 || yp >= (xtable->sizeY-1))
        return VETO_RANGE;

    // All checks passed - do the correction
    double dx = (xp == 0 || xp == (xtable->sizeX - 1)) ? 0 : x - xp;
    double dy = (yp == 0 || yp == (ytable->sizeY - 1)) ? 0 : y - yp;

    x -= (dx * xtable->data[xp+1][yp]) + ((1 - dx) * xtable->data[xp][yp]);
    y -= (dy * ytable->data[xp][yp+1]) + ((1 - dy) * ytable->data[xp][yp]);

    return VETO_NO;
}

FlatFieldPlugin::VetoType FlatFieldPlugin::checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position)
{
    std::shared_ptr<FlatFieldTable> upperLimits = m_tables[position].psUpX;
    std::shared_ptr<FlatFieldTable> lowerLimits = m_tables[position].psLowX;
    if (!upperLimits || !lowerLimits || m_tables[position].enabled == false)
        return VETO_POSITION;

    unsigned xp = nearbyint(x);
    unsigned yp = nearbyint(y);

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= (upperLimits->sizeX-1) || y < 0.0 || yp >= (upperLimits->sizeY-1))
        return VETO_RANGE;

    double upperLimit = upperLimits->data[xp][yp];
    double lowerLimit = lowerLimits->data[xp][yp];

    if (lowerLimit <= photosum_x && photosum_x <= upperLimit)
        return VETO_NO;
    else
        return VETO_PHOTOSUM;
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

            // Push table to tables container
            if (table->type == FlatFieldTable::TYPE_X_CORR) {
                if (m_tables[table->pixel_offset].corrX.get() == 0) {
                    m_tables[table->pixel_offset].corrX = table;
                    m_tables[table->pixel_offset].position_id = table->position_id;
                    m_tables[table->pixel_offset].nTables++;
                    foundCorrTables = true;
                } else {
                    LOG_ERROR("Correction X table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_Y_CORR) {
                if (m_tables[table->pixel_offset].corrY.get() == 0) {
                    m_tables[table->pixel_offset].corrY = table;
                    m_tables[table->pixel_offset].position_id = table->position_id;
                    m_tables[table->pixel_offset].nTables++;
                    foundCorrTables = true;
                } else {
                    LOG_ERROR("Correction Y table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_LOW) {
                if (m_tables[table->pixel_offset].psLowX.get() == 0) {
                    m_tables[table->pixel_offset].psLowX = table;
                    m_tables[table->pixel_offset].position_id = table->position_id;
                    m_tables[table->pixel_offset].nTables++;
                    foundPsTables = true;
                } else {
                    LOG_ERROR("Photosum low X table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_UP) {
                if (m_tables[table->pixel_offset].psUpX.get() == 0) {
                    m_tables[table->pixel_offset].psUpX = table;
                    m_tables[table->pixel_offset].position_id = table->position_id;
                    m_tables[table->pixel_offset].nTables++;
                    foundPsTables = true;
                } else {
                    LOG_ERROR("Photosum upper X table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
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
    for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {

        if (it->second.corrX.get() == 0 && it->second.corrY.get() == 0 && it->second.psLowX.get() == 0 && it->second.psUpX.get() == 0)
            continue;

        nPositions++;

        if (foundCorrTables == true) {
            if (it->second.corrX.get() == 0) {
                LOG_ERROR("Missing X correction table for position %u", it->second.position_id);
            }
            if (it->second.corrY.get() == 0) {
                LOG_ERROR("Missing Y correction table for position %u", it->second.position_id);
            }
        }

        if (foundPsTables == true) {
            if (it->second.psLowX.get() == 0) {
                LOG_ERROR("Missing lower X photosum correction table for position %u", it->second.position_id);
            }
            if (it->second.psUpX.get() == 0) {
                LOG_ERROR("Missing upper X photosum correction table for position %u", it->second.position_id);
            }
        }
    }
}

std::string FlatFieldPlugin::generatePositionsReport(bool psEn, bool corrEn)
{
    std::ostringstream report;
    for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
        // Skip sparse vector entries
        if (it->second.nTables == 0)
            continue;

        report << "Position " << it->second.position_id << (it->second.enabled == false ? " NOT " : " ") << "enabled:" << std::endl;
        if (psEn) {
            report << " * upper X photosum table " << (it->second.psUpX.get()  != 0 ? "loaded" : "MISSING") << std::endl;
            report << " * lower X photosum table " << (it->second.psLowX.get() != 0 ? "loaded" : "MISSING") << std::endl;
        }
        if (corrEn) {
            report << " * X correction table " << (it->second.corrX.get()  != 0 ? "loaded" : "MISSING") << std::endl;
            report << " * Y correction table " << (it->second.corrY.get()  != 0 ? "loaded" : "MISSING") << std::endl;
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
