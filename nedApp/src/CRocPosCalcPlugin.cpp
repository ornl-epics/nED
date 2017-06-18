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

#include <algorithm>
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
    createParam("TimeRangeSumMax",  asynParamInt32, &TimeRangeSumMax, 600); // WRITE - High threshold for integrated time ranges
    createParam("EchoDeadTime",     asynParamInt32, &EchoDeadTime, 250);    // WRITE - Time between two local events in 100ns
    createParam("EchoDeadArea",     asynParamInt32, &EchoDeadArea, 160);    // WRITE - Number of pixels defining the echo-detection area
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
    } else if (pasynUser->reason == TimeRangeSumMax) {
        m_calcParams.timeRangeSumMax = value;
        handled = true;
    } else if (pasynUser->reason == EchoDeadTime) {
        m_calcParams.echoDeadTime = value;
        handled = true;
    } else if (pasynUser->reason == EchoDeadArea) {
        m_calcParams.echoDeadArea = value / 2;
        handled = true;
    } else if (pasynUser->reason == OutExtMode) {
        m_calcParams.outExtMode = (value != 0);
        handled = true;
    } else if (pasynUser->reason == ProcessMode) {
        m_calcParams.processModeNew = (value != 0);
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
        params->position = 0xFFFFFFFF;
        m_detParamsByName[detector] = params;
    } else {
        params = it->second;
    }

    // Now translate parameter described by string into the structure
    if (param == "PositionId") {
        if (params->position != 0xFFFFFFFF)
            m_detParamsByPosition.erase(params->position);
        // Make an entry in lookup-by-position table
        params->position = value;
        m_detParamsByPosition[params->position] = params;
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
                uint32_t pktSize = packet->getLength();
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
    uint64_t pulseId = 0;

    // Determine pulseId based on RTDL time
    const RtdlHeader *rtdl = inPacket->getRtdlHeader();
    if (rtdl) {
        pulseId = ((uint64_t)rtdl->timestamp_sec << 32) | rtdl->timestamp_nsec;
    }

    // outPacket is guaranteed to be at least the size of srcPacket
    (void)inPacket->copyHeader(outPacket, inPacket->getLength());

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
            ret = calculatePixel(inEvent, it->second, pulseId, pixel);
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

static inline uint32_t diff_unsigned(uint32_t a, uint32_t b)
{
    if (a > b) {
        return (a-b);
    } else {
        return (b-a);
    }
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
CRocDataPacket::VetoType CRocPosCalcPlugin::calculatePixel(const CRocDataPacket::RawEvent *event, CRocParams *detParams, uint64_t pulseId, uint32_t &pixel)
{
    CRocDataPacket::VetoType veto = CRocDataPacket::VETO_NO;
    CRocDataPacket::VetoType tmpVeto = CRocDataPacket::VETO_NO;
    uint8_t x,y;

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
        if (veto == CRocDataPacket::VETO_NO) veto = CRocDataPacket::VETO_OUT_OF_RANGE;
        if (m_calcParams.passVetoes == false) return veto;
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
    if (veto == CRocDataPacket::VETO_NO) {
        // Reduce the echo events - strong neutrons with light response spanning
        // over multiple acq frames
        if (pulseId == detParams->pulseId && (event->tof - detParams->lastTof) < m_calcParams.echoDeadTime && diff_unsigned(pixel, detParams->lastPixelId) < m_calcParams.echoDeadArea) {
            veto = CRocDataPacket::VETO_ECHO;
        } else {
            detParams->lastTof = event->tof;
            detParams->lastPixelId = pixel;
            detParams->pulseId = pulseId;
        }
    }
    if (veto != CRocDataPacket::VETO_NO) {
        pixel |= veto;
    }

    return veto;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::checkTimeRange(const CRocDataPacket::RawEvent *event, const CRocParams *detParams)
{
    uint8_t cnt = 0;
    uint32_t sum = 0;
    for (int i=0; i<4; i++) {
        sum += event->time_range[i];
        if (event->time_range[i] >= detParams->timeRangeMin) {
            cnt++;
        }
    }
    if (sum > m_calcParams.timeRangeSumMax) {
        return CRocDataPacket::VETO_TIMERANGE_BAD;
    }
    if (cnt < detParams->timeRangeMinCnt) {
        return CRocDataPacket::VETO_TIMERANGE_BAD;
    }

    if (event->time_range[0] < m_calcParams.timeRange1Min && event->time_range[1] < m_calcParams.timeRange2Min) {
        uint32_t delaySum = (event->time_range[1] + event->time_range[2] + event->time_range[3]);
        if (delaySum >= m_calcParams.timeRangeDelayMin) {
            return CRocDataPacket::VETO_TIMERANGE_DELAYED;
        } else {
            return CRocDataPacket::VETO_TIMERANGE_BAD;
        }
    }

    return CRocDataPacket::VETO_NO;
}

inline std::vector<uint8_t> CRocPosCalcPlugin::sortIndexesDesc(const uint8_t *values, size_t size) {
    std::vector<uint8_t> sorted(size);

    for (size_t i=0; i<size; i++) {
        sorted[i]=i;
    }

	for (size_t i=0; i<size; i++) {
		for(size_t j=i+1; j<size; j++) {
            if (values[sorted[i]] < values[sorted[j]]) {
                uint8_t tmp = sorted[j];
				sorted[j] = sorted[i];
				sorted[i] = tmp;
			}
		}
	}
    return sorted;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateYPosition(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &y)
{
    uint8_t totalCnt = 0;
    for (uint8_t i=0; i<7; i++) {
        if (event->photon_count_y[i] > detParams->yMin) {
            totalCnt++;
        }
    }
    if (totalCnt > detParams->yCntMax)
        return CRocDataPacket::VETO_Y_HIGH_SIGNAL;

    std::vector<uint8_t> ySorted = sortIndexesDesc(event->photon_count_y, 7);

    y = ySorted[0];
    if (event->photon_count_y[ySorted[0]] < detParams->yMin) {
        if (!m_calcParams.efficiencyBoost)
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        if ((event->photon_count_y[ySorted[0]] + event->photon_count_y[ySorted[1]]) <= detParams->yMin || diff_unsigned(ySorted[0], ySorted[1]) > 1) {
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        }
    }

    // When equal, pick the Y index with stronger neighbours response
    if (event->photon_count_y[ySorted[0]] == event->photon_count_y[ySorted[1]]) {
        if ( (ySorted[1] - ySorted[2]) < (ySorted[0] - ySorted[2]) ) {
            y = ySorted[1];
        }
    }

    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateXPosition(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &x)
{
    uint8_t totalCnt = 0;
    for (uint8_t i=0; i<11; i++) {
        if (event->photon_count_x[i] >= detParams->xMin) {
            totalCnt++;
        }
    }
    if (totalCnt > detParams->xCntMax)
        return CRocDataPacket::VETO_X_HIGH_SIGNAL;

    std::vector<uint8_t> xSorted = sortIndexesDesc(event->photon_count_x, 11);
    std::vector<uint8_t> gSorted = sortIndexesDesc(event->photon_count_g, 14);

    // Start with the obvious index, encoding might shift it by 1 in either direction
    uint8_t gIndex = gSorted[0];
    uint8_t xIndex = xSorted[0];

    // Try the efficiency boost
    if (event->photon_count_x[xIndex] < detParams->xMin) {
        if (!m_calcParams.efficiencyBoost)
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        if ((event->photon_count_x[xSorted[0]] + event->photon_count_x[xSorted[1]]) > detParams->xMin) {
            uint8_t distance = diff_unsigned(xSorted[0], xSorted[1]);
            // X must be adjacent. 0,10 are adjacent to both 1 and 9
            if (distance > 1 && distance < 9) {
                return CRocDataPacket::VETO_X_LOW_SIGNAL;
            }
        } else {
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        }
    }

    // When equal, pick the X index with stronger neighbours response
    if (event->photon_count_x[xSorted[0]] == event->photon_count_x[xSorted[1]]) {
        if ( (xSorted[1] - xSorted[2]) < (xSorted[0] - xSorted[2]) ) {
            xIndex = xSorted[1];
        }
    }

    // Find the right G,X due to encoding loss of information
    switch (detParams->fiberCoding) {
    case CRocParams::FIBER_CODING_V2:
        // Reimplemented from dcomserver CROC_CALC_LNEWMGR
        if ((xIndex == 0 || xIndex == 10) && gIndex > 0 && gIndex < 13) {
            if (event->photon_count_g[gSorted[0]] < detParams->gGapMin1) {
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
            }
            if (event->photon_count_g[gSorted[1]] < detParams->gGapMin2) {
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
            }
            if (diff_unsigned(gSorted[0], gSorted[1]) > 1) {
                return CRocDataPacket::VETO_G_NON_ADJACENT;
            }
            if ((xIndex == 0  && gSorted[0] < gSorted[1]) || (xIndex == 10 && gSorted[0] > gSorted[1])) {
                gIndex = gSorted[1];
            }
        } else {
            if (event->photon_count_g[gSorted[0]] < detParams->gMin) {
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
            }
            uint8_t gThreshold = m_calcParams.gNongapMaxRatio * event->photon_count_g[gSorted[0]];
            if (event->photon_count_g[gSorted[1]] > gThreshold) {
                return CRocDataPacket::VETO_G_HIGH_SIGNAL;
            }
        }
        break;
    case CRocParams::FIBER_CODING_V3:
        // Reimplemented from dcomserver CROC_CALCP_NENCODE_BALANCE
        if (xIndex > 0 && xIndex < 10) {
            if (event->photon_count_g[gIndex] < detParams->gMin)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
            uint8_t gThreshold = m_calcParams.gNongapMaxRatio * event->photon_count_g[gIndex];
            if (event->photon_count_g[gSorted[1]] > gThreshold)
                return CRocDataPacket::VETO_G_HIGH_SIGNAL;
        } else if (xIndex == 1 || xIndex == 9) {
            uint8_t xThreshold = 0.65 * event->photon_count_x[xIndex];
            if (detParams->xMin >= xThreshold) {
                xThreshold = detParams->xMin + 1;
            }
            if (event->photon_count_x[xSorted[1]] >= xThreshold) {
                if ((gIndex % 2) == 1) {
                    if ((xSorted[1] == 1 && xSorted[0] == 10) || (xSorted[1] == 9 && xSorted[0] == 0)) {
                        xIndex = xSorted[1];
                    } else {
                        return CRocDataPacket::VETO_G_GHOST;
                    }
                } else {
                    if ((xSorted[1] == 1 && xSorted[0] == 0)  || (xSorted[1] == 9 && xSorted[10] == 0)) {
                        xIndex = xSorted[1];
                    } else {
                        return CRocDataPacket::VETO_G_GHOST;
                    }
                }
            }
        }

        // Second round - xIndex might be already be altered
        if (xIndex == 0 || xIndex == 10) {
            if (event->photon_count_g[gSorted[0]] < detParams->gMin)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;

            if ((gIndex % 2) == 1) {
                xIndex = (xIndex == 10 ? 0 : 10);
            }
            if (diff_unsigned(gSorted[0], gSorted[1]) > 1) {
                if (event->photon_count_g[gSorted[1]] > detParams->gGapMin1)
                    return CRocDataPacket::VETO_G_GHOST;
            } else {
                if ((event->photon_count_g[gSorted[0]] + event->photon_count_g[gSorted[1]]) < detParams->gMin)
                    return CRocDataPacket::VETO_G_LOW_SIGNAL;
            }
        }
        break;
    default:
        return CRocDataPacket::VETO_INVALID_CALC;
    }

    x = 11*gIndex + xIndex;
    return CRocDataPacket::VETO_NO;
}

inline double CRocPosCalcPlugin::calculateNoise(const uint8_t *values, size_t size, uint8_t maxIndex, const uint8_t *weights)
{
    uint32_t noise = 0;
    for (uint8_t i=0; i<size; i++) {
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

inline float CRocPosCalcPlugin::findDirection(const uint8_t *values, size_t size, uint8_t maxIndex)
{
    int32_t left = 0;
    int32_t right = 0;
    float center = values[maxIndex];
    if (maxIndex > 0) {
        left = values[maxIndex-1];
    }
    if (maxIndex < (size-1)) {
        right = values[maxIndex+1];
    }
    return (right - left)/center;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateYPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &y)
{
    uint8_t yMaxIndex;

    if (findMaxIndex(event->photon_count_y, 7, yMaxIndex) == false) {
        return CRocDataPacket::VETO_Y_LOW_SIGNAL;
    }

    double noise = calculateNoise(event->photon_count_y, 7, yMaxIndex, detParams->yWeights);
    if (noise > detParams->yNoiseThreshold) {
        return CRocDataPacket::VETO_Y_HIGH_SIGNAL;
    }

    if (event->photon_count_y[yMaxIndex] < detParams->yMin) {
        // Allow low signal single channel events when efficiency boost is enabled
        if (!m_calcParams.efficiencyBoost || noise > 1.0) {
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        }
    }
    y = yMaxIndex;
    return CRocDataPacket::VETO_NO;
}

/**
 * Fiber mapping v3 correction:
 * Every even G index (index starts at 0) has x1 fiber swapped with x11. This is
 * to help more accurtely determine the position on the edge between two Gs.
 * Knowing that, post-processing correction is necessary for even G indexes.
 * Consider these 2 cases:
 * \verbatim
   Event 1:              v
    # | | | | | | | | | | # | | | | | | | | | | # | | ...
              G=1                      G=2

   Possible readouts:
   X: 0 1 0 0 0 0 0 0 1 3 8
   G: 7 4 0 0 0 0 0 0 0 0 0 0 0 0
   or
   G: 4 7 0 0 0 0 0 0 0 0 0 0 0 0

   Event 2:                v
    # | | | | | | | | | | # | | | | | | | | | | # | | ...
             G=1                      G=2

   Possible readout:
   X: 0 3 1 0 0 0 0 0 0 1 8
   G: 7 4 0 0 0 0 0 0 0 0 0 0 0 0
   or
   G: 4 7 0 0 0 0 0 0 0 0 0 0 0 0
   \endverbatim
 *
 * The 2 readouts are almost the same except for the possible difference in X
 * readout. Common correction for both cases is to correct swap X position
 * 0<=>10 when G is even number and closest G neighbour is on the opposite side
 * of X.
 *
 * Similar exercise can be made for G=2 and G=3.
 */
CRocDataPacket::VetoType CRocPosCalcPlugin::calculateXPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &x)
{
    // G rejection
    uint8_t gMaxIndex;
    if (findMaxIndex(event->photon_count_g, 14, gMaxIndex) == false) {
        return CRocDataPacket::VETO_G_LOW_SIGNAL;
    }
    double gNoise = calculateNoise(event->photon_count_g, 14, gMaxIndex, detParams->gWeights);
    if (gNoise > detParams->gNoiseThreshold) {
        return CRocDataPacket::VETO_G_HIGH_SIGNAL;
    }
    if (event->photon_count_g[gMaxIndex] < detParams->gMin) {
        // Allow low signal single channel events when efficiency boost is enabled
        if (!m_calcParams.efficiencyBoost || gNoise > 1.0) {
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        }
    }

    // X rejection
    uint8_t xMaxIndex;
    if (findMaxIndex(event->photon_count_x, 11, xMaxIndex) == false) {
        return CRocDataPacket::VETO_X_LOW_SIGNAL;
    }
    double xNoise = calculateNoise(event->photon_count_x, 11, xMaxIndex, detParams->xWeights);
    if (xNoise > detParams->xNoiseThreshold) {
        return CRocDataPacket::VETO_X_HIGH_SIGNAL;
    }
    if (event->photon_count_x[xMaxIndex] < detParams->xMin) {
        // Allow low signal single channel events when efficiency boost is enabled
        if (!m_calcParams.efficiencyBoost || xNoise > 1.0) {
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        }
    }

    // Inspect edges and consider modifying G or X index
    if (detParams->fiberCoding == CRocParams::FIBER_CODING_V2) {
        float gDirection = findDirection(event->photon_count_g, 14, gMaxIndex);
        if ((xMaxIndex == 0 || xMaxIndex == 10) && gMaxIndex > 0 && gMaxIndex < 13) {
            // 1-to-1 mapping, but an event might get G group wrong
            if (gDirection > 0) {
                assert(gMaxIndex < 13);
                // Reject ghosts with indipendent discrimination
                if (event->photon_count_g[gMaxIndex] < detParams->gGapMin1) {
                    return CRocDataPacket::VETO_G_GHOST;
                }
                if (event->photon_count_g[gMaxIndex+1] < detParams->gGapMin2) {
                    return CRocDataPacket::VETO_G_GHOST;
                }
                if (event->photon_count_x[0] > event->photon_count_x[10]) {
                    gMaxIndex++;
                } else if (event->photon_count_x[0] == event->photon_count_x[10]) {
                    if (event->photon_count_x[1] >= event->photon_count_x[9]) {
                        gMaxIndex++;
                    } else {
                        xMaxIndex = 10;
                    }
                }
            } else if (gDirection < 0) {
                assert(gMaxIndex > 0);
                // Reject ghosts with indipendent discrimination
                if (event->photon_count_g[gMaxIndex] < detParams->gGapMin1) {
                    return CRocDataPacket::VETO_G_GHOST;
                }
                if (event->photon_count_g[gMaxIndex-1] < detParams->gGapMin2) {
                    return CRocDataPacket::VETO_G_GHOST;
                }
                if (event->photon_count_x[10] > event->photon_count_x[0]) {
                    gMaxIndex--;
                } else if (event->photon_count_x[10] == event->photon_count_x[0]) {
                    if (event->photon_count_x[9] >= event->photon_count_x[1]) {
                        gMaxIndex--;
                    } else {
                        xMaxIndex = 0;
                    }
                }
            } else {
                // Left and right neighbours are the same, are they both 0?
                uint8_t leftG  = (gMaxIndex > 0  ? event->photon_count_g[gMaxIndex-1] : 0);
                uint8_t rightG = (gMaxIndex < 13 ? event->photon_count_g[gMaxIndex+1] : 0);
                if (leftG > 0 && rightG > 0) {
                    // Imposible to deduce the location, must reject
                    return CRocDataPacket::VETO_G_GHOST;
                } else if (leftG == 0 && rightG == 0) {
                    if (event->photon_count_x[0] > detParams->xMin && event->photon_count_x[10] > detParams->xMin) {
                        // Response on both edges of X, but a single G?
                        return CRocDataPacket::VETO_G_GHOST;
                    }
                }
            }
        } else {
            uint8_t gThreshold = m_calcParams.gNongapMaxRatio * event->photon_count_g[gMaxIndex];
            if (gDirection > 0 && event->photon_count_g[gMaxIndex+1] > gThreshold)
                return CRocDataPacket::VETO_G_HIGH_SIGNAL;
            else if (gDirection < 0 && event->photon_count_g[gMaxIndex-1] > gThreshold)
                return CRocDataPacket::VETO_G_HIGH_SIGNAL;
        }
    } else if (detParams->fiberCoding == CRocParams::FIBER_CODING_V3) {
        if (xMaxIndex == 0 || xMaxIndex == 10) {
            float gDirection = findDirection(event->photon_count_g, 14, gMaxIndex);
            // Every second G group has X0 and X10 swapped, see function comment
            // for details.
            if ((gMaxIndex % 2) == 1) {
                if (gDirection == 0) {
                    // gDirection == 0 when right neighbour matches left one or
                    // they're both zero
                    assert(gMaxIndex > 0);
                    if (event->photon_count_g[gMaxIndex-1] > 0) {
                        // As left neighbour is non zero, right one is non-zero too.
                        // We can't deduce the proper location and must reject.
                        return CRocDataPacket::VETO_G_GHOST;
                    } else {
                        // Left and right neighbours are both zero, yet the
                        // PMTs are swapped. Swap again to make it right.
                        xMaxIndex = (xMaxIndex == 0 ? 10 : 0);
                    }
                } else if (gDirection < 0 && xMaxIndex == 10) {
                    xMaxIndex = 0;
                } else if (gDirection > 0 && xMaxIndex == 0) {
                    xMaxIndex = 10;
                }
            }
        }
    }

    x = 11*gMaxIndex + xMaxIndex;
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
    total = 0;
}

CRocPosCalcPlugin::Stats &CRocPosCalcPlugin::Stats::operator+=(const Stats &rhs)
{
    if ((total + rhs.total) > (unsigned)std::numeric_limits<int32_t>::max()) {
        // This is not good, un-controlled resetting
        reset();
    }

    for (size_t i=0; i<sizeof(counters)/sizeof(int32_t); i++) {
        counters[i] += rhs.counters[i];
    }
    total += rhs.total;
    return *this;
}

void CRocPosCalcPlugin::Stats::increment(CRocDataPacket::VetoType type)
{
    if (total >= (unsigned)std::numeric_limits<int32_t>::max()) {
        reset();
    }
    total++;
    counters[VETO2INT(type)]++;
}

int32_t CRocPosCalcPlugin::Stats::get(CRocDataPacket::VetoType type) const
{
    return counters[VETO2INT(type)];
}

int32_t CRocPosCalcPlugin::Stats::getTotal() const
{
    return (int32_t)total;
}
