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

    createParam("ResetCnt",     asynParamInt32, &ResetCnt);                // Reset counters
    createParam("CalcEn",       asynParamInt32, &CalcEn, 0);               // Toggle position calculation
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);            // Number of packet train splits

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
    if (pasynUser->reason == REASON_PARAMS_EXCH) {
        // CRocPlugin is sending CROC parameter, cache it locally
        ParamsExch *p = reinterpret_cast<ParamsExch *>(pasynUser->userData);
        saveDetectorParam(p->portName, p->paramName, value);
        return asynSuccess;
    }

    if (pasynUser->reason == ResetCnt) {
        m_paramsMutex.lock();
        m_stats.reset();
        m_paramsMutex.unlock();
        return asynSuccess;
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

void CRocPosCalcPlugin::saveDetectorParam(const std::string &detector, const std::string &param, epicsInt32 value)
{
    CRocParams *params;

    m_paramsMutex.lock();
    // First find existing CRocParam structure or create new one
    auto it = m_detParamsByName.find(detector);
    if (it == m_detParamsByName.end()) {
        params = new CRocParams;
        m_detParamsByName[detector] = params;
    } else {
        params = it->second;
    }

    // Now translate parameter described by string into the structure
    if (param == "Position") {
        // Make an entry in lookup-by-position table
        m_detParamsByPosition.erase(params->position);
        m_detParamsByPosition[value] = params;

        params->position = value;
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
    } else if (param == "MapMode") {
        switch (value) {
        case CRocParams::ENCODE_V3:
            params->mapMode = CRocParams::ENCODE_V3;
            break;
        case CRocParams::ENCODE_MULTI_GAP_REQ:
        default:
            params->mapMode = CRocParams::ENCODE_MULTI_GAP_REQ;
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
    }

    m_paramsMutex.unlock();
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
    inDataMode = getDataMode();
    this->unlock();

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
        m_calcParams.outExtMode = true;

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
    // TODO: Rest of the statistics
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
                CRocDataPacket::NormalEvent *outEvent = reinterpret_cast<CRocDataPacket::NormalEvent *>(outPacket->getData(&nOutEvents));
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

CRocDataPacket::VetoType CRocPosCalcPlugin::calculatePixel(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint32_t &pixel)
{
    CRocDataPacket::VetoType ret;
    double x,y;

    ret = calculateYPosition(event, detParams, y);
    if (ret != CRocDataPacket::VETO_NO) {
        pixel = ret;
        return ret;
    }
    if (y < 0 || y >= 7) {
        pixel = CRocDataPacket::VETO_OUT_OF_RANGE;
        return CRocDataPacket::VETO_OUT_OF_RANGE;
    }

    ret = calculateXPosition(event, detParams, x);
    if (ret != CRocDataPacket::VETO_NO)
        return ret;
    if (x < 0 || x >= 14*11) {
        pixel = CRocDataPacket::VETO_OUT_OF_RANGE;
        return CRocDataPacket::VETO_OUT_OF_RANGE;
    }

    pixel = 7*(uint8_t)x + (uint8_t)y;
    return CRocDataPacket::VETO_NO;
}

void CRocPosCalcPlugin::findMaxIndexes(const uint8_t *values, size_t size, uint8_t &max1, uint8_t &max2, uint8_t &max3)
{
    max1 = max2 = max3 = 0;
    for (size_t i = 0; i < size; i++) {
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
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateYPosition(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, double &y)
{
    uint8_t yCnt = 0;
    uint8_t yMaxIndex = 0;
    uint8_t ySecondMaxIndex = 0;
    uint8_t yThirdMaxIndex = 0;
    uint8_t yIndex = 0;

    for (int i=0; i<7; i++) {
        if (event->photon_count_y[i] >= detParams->yMin)
            yCnt++;
    }
    if (yCnt >= detParams->yCntMax)
        return CRocDataPacket::VETO_Y_HIGH_SIGNAL;

    findMaxIndexes(event->photon_count_y, 7, yMaxIndex, ySecondMaxIndex, yThirdMaxIndex);

    // Try efficiency boost
    if (yCnt == 0) {
        if (!m_calcParams.efficiencyBoost)
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        if ((event->photon_count_y[yMaxIndex] + event->photon_count_y[ySecondMaxIndex]) < detParams->yMin)
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        int distance = abs(yMaxIndex - ySecondMaxIndex);
        if (distance > 1)
            return CRocDataPacket::VETO_Y_LOW_SIGNAL;
        // Distance is 1 and there's enough photons combined, continue
    }

    // Implementation only for YMaxOut==7, TODO for YMaxOut==13
    yIndex = yMaxIndex;
    if (event->photon_count_y[ySecondMaxIndex] > (0.75*event->photon_count_y[yMaxIndex]) &&
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

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateXPosition(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, double &x)
{
    uint8_t xCnt = 0;
    uint8_t xMaxIndex = 0;
    uint8_t xSecondMaxIndex = 0;
    uint8_t xThirdMaxIndex = 0;

    // Check veto conditions for X
    for (int i=0; i<11; i++) {
        if (event->photon_count_x[i] >= detParams->xMin)
            xCnt++;
    }
    if (xCnt >= detParams->xCntMax)
        return CRocDataPacket::VETO_X_HIGH_SIGNAL;

    findMaxIndexes(event->photon_count_x, 11, xMaxIndex, xSecondMaxIndex, xThirdMaxIndex);
    if (xCnt == 0) {
        if (!m_calcParams.efficiencyBoost)
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        if ((event->photon_count_x[xMaxIndex] + event->photon_count_x[xSecondMaxIndex]) < detParams->xMin)
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        int distance = abs(xMaxIndex - xSecondMaxIndex);
        if (distance > 1 && distance < 9)
            return CRocDataPacket::VETO_X_LOW_SIGNAL;
        // Distance is 1 and there's enough photons combined, continue
    }

//    assert(detParams->pixelCheckMode == CRocParams::CROC_PIXELCHECK_USEMAX); // Need to support other modes?

    // Two max values found, posibly adjust X index
    uint8_t xIndex = xMaxIndex;
    if (event->photon_count_x[xMaxIndex] == event->photon_count_x[xSecondMaxIndex]) {
        uint8_t v1 = event->photon_count_x[xMaxIndex] - event->photon_count_x[xThirdMaxIndex];
        uint8_t v2 = event->photon_count_x[xSecondMaxIndex] - event->photon_count_x[xThirdMaxIndex];
        if (v2 < v1)
            xIndex = xSecondMaxIndex;
    }

    CRocDataPacket::VetoType ret;
    uint8_t gIndex;
    switch (detParams->mapMode) {
    case CRocParams::ENCODE_MULTI_GAP_REQ:
        ret = calculateGPositionMultiGapReq(event, detParams, xIndex, gIndex);
        break;
    case CRocParams::ENCODE_V3:
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

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateGPositionMultiGapReq(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t xIndex, uint8_t &gIndex)
{
    uint8_t gMaxIndex;
    uint8_t gSecondMaxIndex;
    uint8_t gThirdMaxIndex;

    findMaxIndexes(event->photon_count_g, 14, gMaxIndex, gSecondMaxIndex, gThirdMaxIndex);

    // Start with the obvious index, encoding might shift it by 1 in either direction
    gIndex = gMaxIndex;

    if ((xIndex == 0 || xIndex == 10) && gMaxIndex > 0 && gMaxIndex < 13) {
        if (event->photon_count_g[gMaxIndex] < detParams->gGapMin1)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        if (event->photon_count_g[gSecondMaxIndex] < detParams->gGapMin2)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        int distance = abs(gMaxIndex - gSecondMaxIndex);
        if (distance > 1)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        if ((xIndex == 0  && gMaxIndex < gSecondMaxIndex) ||
            (xIndex == 10 && gMaxIndex > gSecondMaxIndex)) {

                gIndex = gSecondMaxIndex;
        }
    } else {
        if (event->photon_count_g[gMaxIndex] < detParams->gMin)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;;
        uint8_t cnt = m_calcParams.gNongapMaxRatio * event->photon_count_g[gMaxIndex];
        if (event->photon_count_g[gSecondMaxIndex] > cnt)
            return CRocDataPacket::VETO_G_HIGH_SIGNAL;
    }

    return CRocDataPacket::VETO_NO;
}

CRocDataPacket::VetoType CRocPosCalcPlugin::calculateGPositionNencode(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &xIndex, uint8_t &gIndex)
{
    uint8_t gMaxIndex;
    uint8_t gSecondMaxIndex;
    uint8_t xMaxIndex;
    uint8_t xSecondMaxIndex;
    uint8_t tmp;

    findMaxIndexes(event->photon_count_g, 14, gMaxIndex, gSecondMaxIndex, tmp);
    findMaxIndexes(event->photon_count_x, 11, xMaxIndex, xSecondMaxIndex, tmp);

    if (xIndex > 0 && xIndex < 10) {
        if (event->photon_count_g[gMaxIndex] < detParams->gMin)
            return CRocDataPacket::VETO_G_LOW_SIGNAL;
        uint8_t cnt = m_calcParams.gNongapMaxRatio * event->photon_count_g[gMaxIndex];
        if (event->photon_count_g[gSecondMaxIndex] > cnt)
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
            if (event->photon_count_g[gMaxIndex] + event->photon_count_g[gSecondMaxIndex] < detParams->gMin)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
        } else {
            if (event->photon_count_g[gMaxIndex] < detParams->gMin)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;

            if (event->photon_count_g[gSecondMaxIndex] > detParams->gGapMin2)
                return CRocDataPacket::VETO_G_LOW_SIGNAL;
        }
    }

    // TODO: explain this!!!
    if ((gIndex == 4 && xIndex == 11) || (gIndex == 6 && gIndex == 2))
        xIndex = 1;

    return CRocDataPacket::VETO_NO;
}
