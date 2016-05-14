/* CRocPosCalcPlugin.cpp
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Bits.h"
#include "CRocPosCalcPlugin.h"
#include "Log.h"

#include <cstring>
#include <likely.h>
#include <limits>
#include <math.h>

EPICS_REGISTER_PLUGIN(CRocPosCalcPlugin, 3, "Port name", string, "Dispatcher port name", string, "Buffer size in bytes", int);

#define NUM_CROCPOSCALCPLUGIN_PARAMS      ((int)(&LAST_CROCPOSCALCPLUGIN_PARAM - &FIRST_CROCPOSCALCPLUGIN_PARAM + 1)) + (20 + 17)*3

CRocPosCalcPlugin::CRocPosCalcPlugin(const char *portName, const char *dispatcherPortName, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_CROCPOSCALCPLUGIN_PARAMS, asynOctetMask, asynOctetMask)
{
    if (bufSize > 0) {
        m_bufferSize = bufSize;
    } else {
        m_bufferSize = 4*1024*1024;
        LOG_INFO("Setting default buffer size to %u bytes", m_bufferSize);
    }
    m_buffer = reinterpret_cast<uint8_t *>(malloc(m_bufferSize));
    if (m_buffer == 0) {
        m_bufferSize = 0;
    }

    createParam("ResetCnt",     asynParamInt32, &ResetCnt);                 // WRITE - Reset counters
    createParam("CalcEn",       asynParamInt32, &CalcEn, 0);                // WRITE - Toggle position calculation (0=disabled,1=enabled)
    createParam("OutExtMode",   asynParamInt32, &OutExtMode, 0);            // WRITE - Switch to toggle between normal and extended output format
    createParam("CntSplit",     asynParamInt32, &CntSplit, 0);              // READ - Number of packet train splits
    createParam("PassVetoes",   asynParamInt32, &PassVetoes, 0);            // WRITE - Allow vetoes in output stream (0=no, 1=yes)
    createParam("GNongapMaxRatio",  asynParamInt32, &GNongapMaxRatio, 0);   // WRITE - TODO
    createParam("EfficiencyBoost",  asynParamInt32, &EfficiencyBoost, 0);   // WRITE - Lower the minimum count restriction (0=no, 1=yes)
    createParam("TimeRange1Min",    asynParamInt32, &TimeRange1Min, 6);     // WRITE - Min counts in first time range bin
    createParam("TimeRange2Min",    asynParamInt32, &TimeRange2Min, 5);     // WRITE - Min counts in second time range bin
    createParam("TimeRangeDelayMin",asynParamInt32, &TimeRangeDelayMin, 15);// WRITE - Delayed event threshold
    createParam("TofResolution",    asynParamInt32, &TofResolution, 250);   // WRITE - Time between two events in 100ns
    createParam("ProcessMode",      asynParamInt32, &ProcessMode, 0);       // WRITE - Select event verification algorithm (0=legacy,1=new)

    createParam("CntTotalEvents",   asynParamInt32, &CntTotalEvents, 0);    // READ - Number of all events
    createParam("CntGoodEvents",    asynParamInt32, &CntGoodEvents, 0);     // READ - Number of good events
    createParam("CntVetoYLow",      asynParamInt32, &CntVetoYLow, 0);       // READ - Number of Y signal low
    createParam("CntVetoYNoise",    asynParamInt32, &CntVetoYNoise, 0);     // READ - Number of Y signal noisy
    createParam("CntVetoXLow",      asynParamInt32, &CntVetoXLow, 0);       // READ - Number of X signal low
    createParam("CntVetoXNoise",    asynParamInt32, &CntVetoXNoise, 0);     // READ - Number of X signal noisy
    createParam("CntVetoGLow",      asynParamInt32, &CntVetoGLow, 0);       // READ - Number of G signal low
    createParam("CntVetoGNoise",    asynParamInt32, &CntVetoGNoise, 0);     // READ - Number of G signal noisy
    createParam("CntVetoGGhost",    asynParamInt32, &CntVetoGGhost, 0);     // READ - Number of G ghost events
    createParam("CntVetoGNonAdj",   asynParamInt32, &CntVetoGNonAdj, 0);    // READ - Number of G non-adjacent events
    createParam("CntVetoBadPos",    asynParamInt32, &CntVetoBadPos, 0);     // READ - Number of events with invalid detector configuration, no such position configured
    createParam("CntVetoBadCalc",   asynParamInt32, &CntVetoBadCalc, 0);    // READ - Number of events with bad configuration, no such fiber coding
    createParam("CntVetoEcho",      asynParamInt32, &CntVetoEcho, 0);       // READ - Number of events too close to previous
    createParam("CntVetoTimeRange", asynParamInt32, &CntVetoTimeRange, 0);  // READ - Number of events rejected due to time range policy
    createParam("CntVetoDelayed",   asynParamInt32, &CntVetoDelayed, 0);    // READ - Number of events delayed based on time range bins

    // These might be more suitable in CROC configuration
    createParam("GWeights",     asynParamInt8Array, &GWeights);             // WRITE - G noise calculation weights
    createParam("XWeights",     asynParamInt8Array, &XWeights);             // WRITE - X noise calculation weights
    createParam("YWeights",     asynParamInt8Array, &YWeights);             // WRITE - Y noise calculation weights

    createParam("TimeRangeExp",     asynParamInt32, &TimeRangeExp);         // WRITE - Time range experimental rejection
    createParam("TimeRangeJason",   asynParamInt32, &TimeRangeJason);       // WRITE - Jason's time range rejection

    callParamCallbacks();
}

CRocPosCalcPlugin::~CRocPosCalcPlugin()
{
    // Prevent further parameter exchange
    enableCallbacks(false);

    // Must deallocate by name, positions table could have multiple entries
    // to the same pointer.
    m_detParamsByPosition.clear();
    for (auto it=m_detParamsByName.begin(); it!=m_detParamsByName.end(); it++) {
        free(it->second);
        m_detParamsByName.erase(it);
    }
}

asynStatus CRocPosCalcPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    bool handled = false;

    m_paramsMutex.lock();
    if (pasynUser->reason == REASON_PARAMS_EXCH) {
        // CRocPlugin is sending CROC parameter, cache it locally
        ParamsExch *p = reinterpret_cast<ParamsExch *>(pasynUser->userData);
        saveDetectorParam(p->portName, p->paramName, value);
        handled = true;
    } else if (pasynUser->reason == ResetCnt) {
        m_stats.reset();
        handled = true;
    } else if (pasynUser->reason == PassVetoes) {
        m_calcParams.passVetoes = (value != 0);
        handled = true;
    } else if (pasynUser->reason == EfficiencyBoost) {
        m_calcParams.efficiencyBoost = (value != 0);
        handled = true;
    } else if (pasynUser->reason == GNongapMaxRatio) {
        m_calcParams.gNongapMaxRatio = value / 100.0;
        handled = true;
    } else if (pasynUser->reason == TimeRange1Min) {
        m_calcParams.timeRange1Min = value;
        handled = true;
    } else if (pasynUser->reason == TimeRange2Min) {
        m_calcParams.timeRange2Min = value;
        handled = true;
    } else if (pasynUser->reason == TimeRangeDelayMin) {
        m_calcParams.timeRangeDelayMin = value;
        handled = true;
    } else if (pasynUser->reason == TofResolution) {
        m_calcParams.tofResolution = value;
        handled = true;
    } else if (pasynUser->reason == OutExtMode) {
        m_calcParams.outExtMode = (value != 0);
        handled = true;
    } else if (pasynUser->reason == ProcessMode) {
        m_calcParams.processModeNew = (value != 0);
        handled = true;
    } else if (pasynUser->reason == TimeRangeExp) {
        m_calcParams.timeRangeExperimental = (value != 0);
        handled = true;
    } else if (pasynUser->reason == TimeRangeJason) {
        m_calcParams.timeRangeJason = (value != 0);
        handled = true;
    }
    m_paramsMutex.unlock();
    if (handled == true)
        return asynSuccess;

    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

void CRocPosCalcPlugin::saveDetectorParam(const std::string &detector, const std::string &param, epicsInt32 value)
{
    CRocParams *params;

    LOG_DEBUG("Remote plugin %s set new param: %s=%d", detector.c_str(), param.c_str(), value);

    // First find existing CRocParam structure or create new one
    auto it = m_detParamsByName.find(detector);
    if (it == m_detParamsByName.end()) {
        params = new CRocParams;
        m_detParamsByName[detector] = params;
    } else {
        params = it->second;
    }

    // Now translate parameter described by string into the structure
    if (param == "PositionId") {
        // Make an entry in lookup-by-position table
        m_detParamsByPosition.erase(params->position);
        m_detParamsByPosition[value] = params;

        params->position = value;
        LOG_INFO("Registered detector %s, position %d", detector.c_str(), value);
    } else if (param == "TimeRange1") {
        params->timeRange1 = value;
    } else if (param == "TimeRange2") {
        params->timeRange2 = value;
    } else if (param == "TimeRange3") {
        params->timeRange3 = value;
    } else if (param == "TimeRange4") {
        params->timeRange4 = value;
    } else if (param == "XMin") {
        params->xMin = value;
    } else if (param == "YMin") {
        params->yMin = value;
    } else if (param == "YCntMax") {
        params->yCntMax = value;
    } else if (param == "XCntMax") {
        params->xCntMax = value;
    } else if (param == "FiberCoding") {
        switch (value) {
        case CRocParams::FIBER_CODING_V2:
        case CRocParams::FIBER_CODING_V3:
            params->fiberCoding = static_cast<CRocParams::FiberCoding>(value);
            break;
        default:
            LOG_ERROR("Not supported fiber coding '%d'", value);
            break;
        }
    } else if (param == "GMin") {
        params->gMin = value;
    } else if (param == "GMin2") {
        params->gMin2 = value;
    } else if (param == "GGapMin1") {
        params->gGapMin1 = value;
    } else if (param == "GGapMin2") {
        params->gGapMin2 = value;
    } else if (param == "GNoiseThreshold") {
        params->gNoiseThreshold = value;
    } else if (param == "XNoiseThreshold") {
        params->xNoiseThreshold = value;
    } else if (param == "YNoiseThreshold") {
        params->yNoiseThreshold = value;
    } else if (param == "TimeRangeMin") {
        params->timeRangeMin = value;
    } else if (param == "TimeRangeMinCnt") {
        params->timeRangeMinCnt = value;
    } else if (param.compare(0, 7, "GWeight") == 0) {
        for (int i=0; i<14; i++) {
            char name[32];
            snprintf(name, sizeof(name), "GWeight%d", i);
            if (param == name) {
                params->gWeights[i] = value;
                break;
            }
        }
    } else if (param.compare(0, 7, "XWeight") == 0) {
        for (int i=0; i<11; i++) {
            char name[32];
            snprintf(name, sizeof(name), "XWeight%d", i);
            if (param == name) {
//LOG_ERROR("xWeights[%d] = %d", i, value);
                params->xWeights[i] = value;
                break;
            }
        }
    } else if (param.compare(0, 7, "YWeight") == 0) {
        for (int i=0; i<7; i++) {
            char name[32];
            snprintf(name, sizeof(name), "YWeight%d", i);
            if (param == name) {
                params->yWeights[i] = value;
                break;
            }
        }
    }
}

void CRocPosCalcPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nSplits = 0;
    bool calcEn = false;
    DataMode inDataMode;

    this->lock();
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(CntSplit,       &nSplits);
    getBooleanParam(CalcEn,         &calcEn);
    inDataMode = getDataMode();
    this->unlock();

    if (inDataMode != BasePlugin::DATA_MODE_RAW && inDataMode != BasePlugin::DATA_MODE_EXTENDED)
        calcEn = false;

    nReceived += packetList->size();

    // Optimize pass thru mode
    if (calcEn == false) {
        m_packetList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&m_packetList);
        m_packetList.release();
        m_packetList.waitAllReleased();
        m_packetList.clear();
    } else {
        m_paramsMutex.lock();

        m_calcParams.inExtMode = (inDataMode == BasePlugin::DATA_MODE_EXTENDED);

        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;
            m_packetList.clear();
            m_packetList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;

                // Reuse the original packet if nothing to map
                if (packet->isNeutronData() == false) {
                    m_packetList.push_back(packet);
                    continue;
                }

                // Calculate space needed for this packet
                uint32_t pktSize = packet->length();
                if (!m_calcParams.inExtMode) {
                    // Convert to extended mode, add space for pixel field
                    uint32_t nEvents = (packet->getDataLength() / sizeof(CRocDataPacket::RawEvent));
                    pktSize += 4*nEvents;
                }

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < pktSize) {
                    nSplits++;
                    break;
                }

                nProcessed++;

                // Reserve part of buffer for this packet
                DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                m_packetList.push_back(newPacket);
                bufferOffset += pktSize;

                // Process the packet
                m_stats += processPacket(packet, newPacket);
            }

            sendToPlugins(&m_packetList);
            m_packetList.release();
            m_packetList.waitAllReleased();
        }

        m_paramsMutex.unlock();
    }

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntTotalEvents,     m_stats.getTotal());
    setIntegerParam(CntGoodEvents,      m_stats.get(CRocDataPacket::VETO_NO));
    setIntegerParam(CntVetoYLow,        m_stats.get(CRocDataPacket::VETO_Y_LOW_SIGNAL));
    setIntegerParam(CntVetoYNoise,      m_stats.get(CRocDataPacket::VETO_Y_HIGH_SIGNAL));
    setIntegerParam(CntVetoXLow,        m_stats.get(CRocDataPacket::VETO_X_LOW_SIGNAL));
    setIntegerParam(CntVetoXNoise,      m_stats.get(CRocDataPacket::VETO_X_HIGH_SIGNAL));
    setIntegerParam(CntVetoGLow,        m_stats.get(CRocDataPacket::VETO_G_LOW_SIGNAL));
    setIntegerParam(CntVetoGNoise,      m_stats.get(CRocDataPacket::VETO_G_HIGH_SIGNAL));
    setIntegerParam(CntVetoGGhost,      m_stats.get(CRocDataPacket::VETO_G_GHOST));
    setIntegerParam(CntVetoGNonAdj,     m_stats.get(CRocDataPacket::VETO_G_NON_ADJACENT));
    setIntegerParam(CntVetoBadPos,      m_stats.get(CRocDataPacket::VETO_INVALID_POSITION));
    setIntegerParam(CntVetoBadCalc,     m_stats.get(CRocDataPacket::VETO_INVALID_CALC));
    setIntegerParam(CntVetoEcho,        m_stats.get(CRocDataPacket::VETO_ECHO));
    setIntegerParam(CntVetoTimeRange,   m_stats.get(CRocDataPacket::VETO_TIMERANGE_BAD));
    setIntegerParam(CntVetoDelayed,     m_stats.get(CRocDataPacket::VETO_TIMERANGE_DELAYED));
    callParamCallbacks();
    this->unlock();
}

CRocPosCalcPlugin::Stats CRocPosCalcPlugin::processPacket(const DasPacket *inPacket, DasPacket *outPacket)
{
    Stats stats;
    uint32_t inEventSize = (m_calcParams.inExtMode ? sizeof(CRocDataPacket::ExtendedEvent) : sizeof(CRocDataPacket::RawEvent));
    uint32_t outEventSize = (m_calcParams.outExtMode ? sizeof(CRocDataPacket::ExtendedEvent) : sizeof(CRocDataPacket::NormalEvent));
    uint32_t tmp;
    uint32_t nInEvents, nOutEvents;

    // outPacket is guaranteed to be at least the size of srcPacket
    (void)inPacket->copyHeader(outPacket, inPacket->length());

    char *outData = const_cast<char *>(reinterpret_cast<const char *>(outPacket->getData(&tmp)));
    const char *inData = reinterpret_cast<const char *>(inPacket->getData(&nInEvents));
    nInEvents /= (inEventSize / sizeof(uint32_t));

    // Iterate over all events
    nOutEvents = 0;
    while (nInEvents-- > 0) {
        uint32_t pixel = 0;
        CRocDataPacket::VetoType ret;

        // Extended event is the same as raw event with an extra pixel at the end
        // Since we don't use extended mode, pretend it's always raw
        const CRocDataPacket::RawEvent *inEvent = reinterpret_cast<const CRocDataPacket::RawEvent *>(inData);

        auto it = m_detParamsByPosition.find(inEvent->position);
        if (it == m_detParamsByPosition.end()) {
            ret = CRocDataPacket::VETO_INVALID_POSITION;
            pixel = CRocDataPacket::VETO_INVALID_POSITION;
        } else {
            ret = calculatePixel(inEvent, it->second, pixel);
        }

        if (ret == CRocDataPacket::VETO_NO || m_calcParams.passVetoes) {
            if (m_calcParams.outExtMode) {
                CRocDataPacket::ExtendedEvent *outEvent = reinterpret_cast<CRocDataPacket::ExtendedEvent *>(outData);
                memcpy(outEvent, inEvent, inEventSize);
                outEvent->pixelid = pixel;
            } else {
                CRocDataPacket::NormalEvent *outEvent = reinterpret_cast<CRocDataPacket::NormalEvent *>(outData);
                outEvent->tof = inEvent->tof;
                outEvent->pixelid = pixel;
            }

            outData += outEventSize;
            nOutEvents++;
        }

        inData += inEventSize;

        stats.increment(ret);
    }
    outPacket->payload_length += nOutEvents * outEventSize;

    return stats;
}

/**
 * Calculates X and Y dimension from the raw counts and encodes it into pixel id
 * format. The encoded pixel id includes pixel position withing the detector and
 * the detector offset information. It may also include the veto type in case
 * event was rejected. In case multiple rejection conditions were met, only the
 * first one gets encoded into pixel id. A single detector defines 154x7=1078
 * pixels. The final encoding is as follows:
 * > pixel id = (0x8FC00000) | ([detector offset] + [xy])
 */
CRocDataPacket::VetoType CRocPosCalcPlugin::calculatePixel(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint32_t &pixel)
{
    CRocDataPacket::VetoType veto = CRocDataPacket::VETO_NO;
    CRocDataPacket::VetoType tmpVeto = CRocDataPacket::VETO_NO;
    uint8_t x,y;

    if (m_calcParams.timeRangeJason)
        tmpVeto = checkTimeRangeJason(event, detParams);
    else
        tmpVeto = checkTimeRange(event, detParams);
    if (tmpVeto != CRocDataPacket::VETO_NO) {
        if (m_calcParams.passVetoes == false) return tmpVeto;
        if (veto == CRocDataPacket::VETO_NO) veto = tmpVeto;
    }

    if (m_calcParams.processModeNew)
        tmpVeto = calculateYPositionNew(event, detParams, y);
    else
        tmpVeto = calculateYPosition(event, detParams, y);
    if (tmpVeto != CRocDataPacket::VETO_NO) {
        if (m_calcParams.passVetoes == false) return tmpVeto;
        if (veto == CRocDataPacket::VETO_NO) veto = tmpVeto;
    }
    if (y >= 7) {
        if (m_calcParams.passVetoes == false) return CRocDataPacket::VETO_OUT_OF_RANGE;
        if (veto == CRocDataPacket::VETO_NO) veto = CRocDataPacket::VETO_OUT_OF_RANGE;
        y = 6;
    }

    if (m_calcParams.processModeNew)
        tmpVeto = calculateXPositionNew(event, detParams, x);
    else
        tmpVeto = calculateXPosition(event, detParams, x);
    if (tmpVeto != CRocDataPacket::VETO_NO) {
        if (m_calcParams.passVetoes == false) return tmpVeto;
        if (veto == CRocDataPacket::VETO_NO) veto = tmpVeto;
    }
    if (x >= 14*11) {
        if (m_calcParams.passVetoes == false) return CRocDataPacket::VETO_OUT_OF_RANGE;
        if (veto == CRocDataPacket::VETO_NO) veto = CRocDataPacket::VETO_OUT_OF_RANGE;
        x = 14*11 - 1;
    }

    pixel = detParams->position + 7*x + y;
    if (veto != CRocDataPacket::VETO_NO)
        pixel |= veto;

    return veto;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::checkTimeRangeJason(const CRocDataPacket::RawEvent *event, const CRocParams *detParams)
{
    uint32_t delaySum = (event->time_range[1] + event->time_range[2] + event->time_range[3]);

    // Check for echo event
    if (event->tof < (detParams->tofLast + m_calcParams.tofResolution)) {
        return CRocDataPacket::VETO_ECHO;
    }

    // Check time ranges first
    if (event->time_range[0] < m_calcParams.timeRange1Min || event->time_range[1] < m_calcParams.timeRange2Min) {
        if (event->time_range[0] < m_calcParams.timeRange1Min &&
            delaySum >= m_calcParams.timeRangeDelayMin) {
            return CRocDataPacket::VETO_TIMERANGE_DELAYED;
        } else {
            return CRocDataPacket::VETO_TIMERANGE_BAD;
        }
    }

    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::checkTimeRange(const CRocDataPacket::RawEvent *event, const CRocParams *detParams)
{
    uint8_t cnt = 0;
    for (int i=0; i<4; i++) {
        if (event->time_range[i] >= detParams->timeRangeMin) {
            cnt++;
        }
    }

    if (cnt < detParams->timeRangeMinCnt) {
        return CRocDataPacket::VETO_TIMERANGE_BAD;
    }
    if (m_calcParams.timeRangeExperimental &&
        (event->time_range[1] < detParams->timeRangeMin || event->time_range[2] < detParams->timeRangeMin)) {
        return CRocDataPacket::VETO_TIMERANGE_BAD;
    }
    return CRocDataPacket::VETO_NO;
}

uint8_t CRocPosCalcPlugin::findMaxIndexes(const uint8_t *values, size_t size, uint8_t &max1, uint8_t &max2, uint8_t &max3)
{
    max1 = max2 = max3 = 0;
    for (size_t i = 1; i < size; i++) {
        if (values[i] > values[max1]) {
            max3 = max2;
            max2 = max1;
            max1 = i;
        } else if (values[i] > values[max2]) {
            max3 = max2;
            max2 = i;
        } else if (values[i] > values[max3] ) {
            max3 = i;
        }
    }
    if (max3 != 0 || values[0] != 0) return 3;
    if (max2 != 0 || values[0] != 0) return 2;
    if (max1 != 0 || values[0] != 0) return 1;
    return 0;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateYPosition(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &y)
{
    uint8_t yCnt = 0;
    uint8_t yMaxIndex = 0;
    uint8_t ySecondMaxIndex = 0;
    uint8_t yThirdMaxIndex = 0;
    uint8_t yIndex = 0;
    uint8_t nYmaxes;

    for (int i=0; i<7; i++) {
        if (event->photon_count_y[i] >= detParams->yMin)
            yCnt++;
    }
    if (yCnt >= detParams->yCntMax)
        return CRocDataPacket::VETO_Y_HIGH_SIGNAL;

    nYmaxes = findMaxIndexes(event->photon_count_y, 7, yMaxIndex, ySecondMaxIndex, yThirdMaxIndex);

    // Try efficiency boost
    if (yCnt == 0) {
        if (!m_calcParams.efficiencyBoost)
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        if (nYmaxes > 1) {
            if ((event->photon_count_y[yMaxIndex] + event->photon_count_y[ySecondMaxIndex]) < detParams->yMin)
                return CRocDataPacket::VETO_Y_LOW_SIGNAL;
            int distance = abs(yMaxIndex - ySecondMaxIndex);
            if (distance > 1)
                return CRocDataPacket::VETO_Y_LOW_SIGNAL;
            // Distance is 1 and there's enough photons combined, continue
        }
    }

    // Implementation only for YMaxOut==7, TODO for YMaxOut==13
    yIndex = yMaxIndex;
    if (nYmaxes > 1 &&
        event->photon_count_y[ySecondMaxIndex] > (0.75*event->photon_count_y[yMaxIndex]) &&
        event->photon_count_y[ySecondMaxIndex] > detParams->yMin) {

        int distance = abs(yMaxIndex - ySecondMaxIndex);
        if (m_calcParams.checkAdjTube && distance > 1)
            return CRocDataPacket::VETO_Y_NON_ADJACENT;

        if (event->photon_count_y[ySecondMaxIndex] == event->photon_count_y[yMaxIndex]) {
            int v1 = abs(event->photon_count_y[yThirdMaxIndex] - event->photon_count_y[ySecondMaxIndex]);
            int v2 = abs(event->photon_count_y[yThirdMaxIndex] - event->photon_count_y[yMaxIndex]);
            if (v1 < v2)
                yIndex = ySecondMaxIndex;
        }
    }

    // TODO: missing directions from dcom

    y = yIndex;

#ifdef DO_CENTROID
    // Now the centroid position calculation
    if (yIndex > 0 && yIndex < 6) {
        int nsum = event->photon_count_y[yIndex+1] - event->photon_count_y[yIndex-1];
        int dsum = event->photon_count_y[yIndex-1] + event->photon_count_y[yIndex] + event->photon_count_y[yIndex+1];
        y += 1.0 * nsum / dsum;
    }
#endif

    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateXPosition(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &x)
{
    uint8_t xCnt = 0;
    uint8_t xMaxIndex = 0;
    uint8_t xSecondMaxIndex = 0;
    uint8_t xThirdMaxIndex = 0;
    uint8_t nXmaxes;

    // Check veto conditions for X
    for (int i=0; i<11; i++) {
        if (event->photon_count_x[i] >= detParams->xMin)
            xCnt++;
    }
    if (xCnt >= detParams->xCntMax)
        return CRocDataPacket::VETO_X_HIGH_SIGNAL;

    nXmaxes = findMaxIndexes(event->photon_count_x, 11, xMaxIndex, xSecondMaxIndex, xThirdMaxIndex);
    if (xCnt == 0) {
        if (!m_calcParams.efficiencyBoost)
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        if (nXmaxes > 1) {
            if ((event->photon_count_x[xMaxIndex] + event->photon_count_x[xSecondMaxIndex]) < detParams->xMin)
                return CRocDataPacket::VETO_X_LOW_SIGNAL;
            int distance = abs(xMaxIndex - xSecondMaxIndex);
            if (distance > 1 && distance < 9)
                return CRocDataPacket::VETO_X_LOW_SIGNAL;
            // Distance is 1 and there's enough photons combined, continue
        }
    }

//    assert(detParams->pixelCheckMode == CRocParams::CROC_PIXELCHECK_USEMAX); // Need to support other modes?

    uint8_t xIndex = xMaxIndex;
    if (nXmaxes > 1 && event->photon_count_x[xMaxIndex] == event->photon_count_x[xSecondMaxIndex]) {
        // Two max values found, posibly adjust X index
        uint8_t v1 = event->photon_count_x[xMaxIndex] - event->photon_count_x[xThirdMaxIndex];
        uint8_t v2 = event->photon_count_x[xSecondMaxIndex] - event->photon_count_x[xThirdMaxIndex];
        if (v2 < v1)
            xIndex = xSecondMaxIndex;
    }

    CRocDataPacket::VetoType ret;
    uint8_t gIndex;
    switch (detParams->fiberCoding) {
    case CRocParams::FIBER_CODING_V2:
        ret = calculateGPositionMultiGapReq(event, detParams, xIndex, gIndex);
        break;
    case CRocParams::FIBER_CODING_V3:
        ret = calculateGPositionNencode(event, detParams, xIndex, gIndex);
        break;
    default:
        ret = CRocDataPacket::VETO_INVALID_CALC;
        break;
    }
    if (ret != CRocDataPacket::VETO_NO)
        return ret;

    x = 11*gIndex + xIndex;
    return CRocDataPacket::VETO_NO;
}

inline double CRocPosCalcPlugin::calculateGNoise(const uint8_t *values, uint8_t maxIndex, const uint8_t weights[14])
{
    uint32_t noise = 0;
    for (uint8_t i=0; i<14; i++) {
        noise += values[i] * weights[abs(i-maxIndex)];
    }
    return 1.0*noise/values[maxIndex];
}

inline double CRocPosCalcPlugin::calculateXNoise(const uint8_t *values, uint8_t maxIndex, const uint8_t weights[11])
{
    uint32_t noise = 0;
    for (uint8_t i=0; i<11; i++) {
//LOG_ERROR("values[%d]=%d weights[%d]=%d", i, values[i], abs(i-maxIndex), weights[abs(i-maxIndex)]);
        noise += values[i] * weights[abs(i-maxIndex)];
    }
    return 1.0*noise/values[maxIndex];
}

inline double CRocPosCalcPlugin::calculateYNoise(const uint8_t *values, uint8_t maxIndex, const uint8_t weights[7])
{
    uint32_t noise = 0;
    for (uint8_t i=0; i<7; i++) {
        noise += values[i] * weights[abs(i-maxIndex)];
    }
    return 1.0*noise/values[maxIndex];
}

inline bool CRocPosCalcPlugin::findMaxIndex(const uint8_t *values, size_t size, uint8_t &max)
{
    max = 0;
    bool found = (values[0] > 0);
    for (size_t i = 1; i < size; i++) {
        if (values[i] > values[max]) {
            max = i;
            found = true;
        }
    }
    return found;
}

/**
 * Find the direction in which the neighbour with higher counts is.
 *
 * @return positive number when more counts were detected on the right side from
 *         maxIndex, negative value when more counts on the left side and
 *         0 left and right side are equal.
 */
inline int32_t CRocPosCalcPlugin::findDirection(const uint8_t *values, size_t size, uint8_t maxIndex)
{
    int32_t left = 0;
    int32_t right = 0;
    if (maxIndex > 0) {
        left = values[maxIndex-1];
    }
    if (maxIndex < 13) {
        right = values[maxIndex+1];
    }
    return right - left;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateYPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &y)
{
    uint8_t yMaxIndex;

    if (findMaxIndex(event->photon_count_y, 7, yMaxIndex) == false) {
        return CRocDataPacket::VETO_Y_LOW_SIGNAL;
    }

    double noise = calculateYNoise(event->photon_count_y, yMaxIndex, detParams->yWeights);
    if (noise > detParams->yNoiseThreshold) {
        return CRocDataPacket::VETO_Y_HIGH_SIGNAL;
    }

    if (event->photon_count_y[yMaxIndex] < detParams->yMin) {
        if (!m_calcParams.efficiencyBoost || noise > 1.0) {
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        }
    }

    y = yMaxIndex;
    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateXPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &x)
{
    uint8_t gMaxIndex;
    uint8_t xMaxIndex;

    if (findMaxIndex(event->photon_count_g, 14, gMaxIndex) == false) {
        return CRocDataPacket::VETO_G_LOW_SIGNAL;
    }

    double gNoise = calculateGNoise(event->photon_count_g, gMaxIndex, detParams->gWeights);
//LOG_ERROR("gNoise=%.1f threshold=%d", gNoise, detParams->gNoiseThreshold);
    if (gNoise > detParams->gNoiseThreshold) {
        return CRocDataPacket::VETO_G_HIGH_SIGNAL;
    }

    if (event->photon_count_g[gMaxIndex] < detParams->gMin) {
        if (!m_calcParams.efficiencyBoost || gNoise > 1.0) {
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        }
    }

    if (findMaxIndex(event->photon_count_x, 11, xMaxIndex) == false) {
        return CRocDataPacket::VETO_X_LOW_SIGNAL;
    }

    double xNoise = calculateXNoise(event->photon_count_x, xMaxIndex, detParams->xWeights);
//LOG_ERROR("xNoise=%.1f threshold=%d", xNoise, detParams->xNoiseThreshold);
    if (xNoise > detParams->xNoiseThreshold) {
        return CRocDataPacket::VETO_X_HIGH_SIGNAL;
    }

    if (event->photon_count_x[xMaxIndex] < detParams->xMin) {
        if (!m_calcParams.efficiencyBoost || xNoise > 1.0) {
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        }
    }

    // Inspect edges and consider modifying G or X index
    if (xMaxIndex == 0 || xMaxIndex == 10) {
        int32_t gDirection = findDirection(event->photon_count_g, 14, gMaxIndex);
        if (detParams->fiberCoding == CRocParams::FIBER_CODING_V2) {
            // 1-to-1 mapping, but an event might get G group wrong
            if (gDirection > 0) {
                assert(gMaxIndex < 13);
                if (event->photon_count_x[0] > event->photon_count_x[10] &&
                    event->photon_count_x[10] >= detParams->xMin) {
                    // example:
                    //            v
                    // G: 0 . . 0 9 7 0 . . 0
                    // X: 7 0 . . . . 0 4 5
                    //    ^
                    gMaxIndex++;
                }
            } else if (gDirection < 0) {
                assert(gMaxIndex > 0);
                if (event->photon_count_x[10] > event->photon_count_x[0] &&
                    event->photon_count_x[0] >= detParams->xMin) {
                    // example:
                    //              v
                    // G: 0 . . 0 7 9 0 . . 0
                    // X: 5 4 0 . . . . 0 7
                    //                    ^
                    gMaxIndex--;
                }
            }
        } else if (detParams->fiberCoding == CRocParams::FIBER_CODING_V3) {
            // every second G group has X0 and X11 swapped
            if (gDirection < 0 && xMaxIndex == 10) {
                // example:
                //              v
                // G: 0 . . 0 3 5 0 . . 0
                // X: 3 0 . . . . . 0 5
                //                    ^
                xMaxIndex = 0;
            } else if (gDirection > 0 && xMaxIndex == 0) {
                // example:
                //            v
                // G: 0 . . 0 5 3 0 . . 0
                // X: 5 0 . . . . . 0 3
                //    ^
                xMaxIndex = 10;
            } else if (gDirection == 0 && (gMaxIndex % 2) == 1) {
                if (xMaxIndex == 0) {
                    xMaxIndex = 10;
                } else /* (xMaxIndex == 10) */ {
                    xMaxIndex = 0;
                }
            }
        }
    }

    x = 11*gMaxIndex + xMaxIndex;
    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateGPositionMultiGapReq(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t xIndex, uint8_t &gIndex)
{
    // Reimplemented from dcomserver CROC_CALC_LNEWMGR
    uint8_t gMaxIndex;
    uint8_t gSecondMaxIndex;
    uint8_t gThirdMaxIndex;
    uint8_t nGmaxes;

    nGmaxes = findMaxIndexes(event->photon_count_g, 14, gMaxIndex, gSecondMaxIndex, gThirdMaxIndex);

    // Start with the obvious index, encoding might shift it by 1 in either direction
    gIndex = gMaxIndex;

    if ((xIndex == 0 || xIndex == 10) && gMaxIndex > 0 && gMaxIndex < 13) {
        if (event->photon_count_g[gMaxIndex] < detParams->gGapMin1)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        if (nGmaxes > 1) {
            if (event->photon_count_g[gSecondMaxIndex] < detParams->gGapMin2)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
            int distance = abs(gMaxIndex - gSecondMaxIndex);
            if (distance > 1)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
            if ((xIndex == 0  && gMaxIndex < gSecondMaxIndex) ||
                (xIndex == 10 && gMaxIndex > gSecondMaxIndex)) {

                    gIndex = gSecondMaxIndex;
            }
        }
    } else {
        if (event->photon_count_g[gMaxIndex] < detParams->gMin)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;;
        uint8_t cnt = m_calcParams.gNongapMaxRatio * event->photon_count_g[gMaxIndex];
        if (nGmaxes > 1 && event->photon_count_g[gSecondMaxIndex] > cnt)
            return CRocDataPacket::VETO_G_HIGH_SIGNAL;
    }

    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateGPositionNencode(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &xIndex, uint8_t &gIndex)
{
    // Reimplemented from dcomserver CROC_CALCP_NENCODE_BALANCE
    uint8_t gMaxIndex;
    uint8_t gSecondMaxIndex;
    uint8_t xMaxIndex;
    uint8_t xSecondMaxIndex;
    uint8_t tmp;
    uint8_t nGmaxes;

    nGmaxes = findMaxIndexes(event->photon_count_g, 14, gMaxIndex, gSecondMaxIndex, tmp);
    findMaxIndexes(event->photon_count_x, 11, xMaxIndex, xSecondMaxIndex, tmp);

    if (nGmaxes == 0)
        return CRocDataPacket::VETO_G_LOW_SIGNAL;

    if (xIndex > 0 && xIndex < 10) {
        if (event->photon_count_g[gMaxIndex] < detParams->gMin)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        uint8_t cnt = m_calcParams.gNongapMaxRatio * event->photon_count_g[gMaxIndex];
        if (nGmaxes > 1 && event->photon_count_g[gSecondMaxIndex] > cnt)
            return CRocDataPacket::VETO_G_HIGH_SIGNAL;
    } else if (xSecondMaxIndex == 1 || xSecondMaxIndex == 9) {
        uint8_t cnt = std::max(detParams->xMin, (uint32_t)(0.65 * event->photon_count_x[xIndex]));
        if (event->photon_count_x[xSecondMaxIndex] >= cnt) {

            if (gMaxIndex & 0x1) { // group is odd 1,3,etc
                if ((xSecondMaxIndex == 1 && xIndex == 10) ||
                    (xSecondMaxIndex == 9 && xIndex == 0)) {

                    xIndex = xSecondMaxIndex;
                } else {
                    return CRocDataPacket::VETO_G_GHOST;
                }
            } else { // group is even 2,4,etc
                if ((xSecondMaxIndex == 1 && xIndex == 0) ||
                    (xSecondMaxIndex == 9 && xIndex == 10)) {
                    xIndex = xSecondMaxIndex;
                } else {
                    return CRocDataPacket::VETO_G_GHOST;
                }
            }
        }
    }

    if (xIndex == 0 && xIndex == 10) {
        if (abs(gMaxIndex - gSecondMaxIndex) == 1) {
            if (nGmaxes > 1 && event->photon_count_g[gMaxIndex] + event->photon_count_g[gSecondMaxIndex] < detParams->gMin)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
        } else {
            if (event->photon_count_g[gMaxIndex] < detParams->gMin)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;

            if (nGmaxes > 1 && event->photon_count_g[gSecondMaxIndex] > detParams->gGapMin2)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
        }
    }

    // TODO: explain this!!!
    if ((gIndex == 4 && xIndex == 11) || (gIndex == 6 && gIndex == 2))
        xIndex = 1;

    return CRocDataPacket::VETO_NO;
}

// ============= CRocPosCalcPlugin::Stats class implementation ============= //

#define VETO2INT(a) (((a) & ~0x80000000) >> 22)
CRocPosCalcPlugin::Stats::Stats()
{
    reset();
}

void CRocPosCalcPlugin::Stats::reset()
{
    for (size_t i=0; i<sizeof(counters)/sizeof(int32_t); i++) {
        counters[i] = 0;
    }
}

CRocPosCalcPlugin::Stats &CRocPosCalcPlugin::Stats::operator+=(const Stats &rhs)
{
    int64_t count = getTotal() + rhs.getTotal();
    if (count > std::numeric_limits<int32_t>::max()) {
        // This is not good, un-controlled resetting
        reset();
    } else {
        for (size_t i=0; i<sizeof(counters)/sizeof(int32_t); i++) {
            counters[i] += rhs.counters[i];
        }
    }
    return *this;
}

void CRocPosCalcPlugin::Stats::increment(CRocDataPacket::VetoType type)
{
    counters[VETO2INT(type)]++;
}

int32_t CRocPosCalcPlugin::Stats::get(CRocDataPacket::VetoType type) const
{
    return counters[VETO2INT(type)];
}

int32_t CRocPosCalcPlugin::Stats::getTotal() const
{
    int32_t count = 0;
    for (size_t i=0; i<sizeof(counters)/sizeof(int32_t); i++) {
        count += counters[i];
    }
    return count;
}
