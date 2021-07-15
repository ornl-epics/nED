/* BnlPosCalcPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BnlPosCalcPlugin.h"
#include "Log.h"

#include <likely.h>
#include <limits>
#include <cmath>

EPICS_REGISTER_PLUGIN(BnlPosCalcPlugin, 2, "Port name", string, "Parent plugins", string);

#define POS_SPECIAL     (1 << 30)

BnlPosCalcPlugin::BnlPosCalcPlugin(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, 1)
{
    m_calcParams.correctionScale = 1.0;

    createParam("ErrMem",           asynParamInt32, &ErrMem, 0);                // Buffer allocation error
    createParam("LowChargeVetoEn",  asynParamInt32, &LowChargeVetoEn, 1);       // Toggle low charge vetos
    createParam("EdgeVetoEn",       asynParamInt32, &EdgeVetoEn, 1);            // Toggle edge vetos
    createParam("OverflowVetoEn",   asynParamInt32, &OverflowVetoEn, 1);        // Toggle overflow vetos
    createParam("MultiEventVetoEn", asynParamInt32, &MultiEventVetoEn, 1);      // Toggle multi-event vetos
    createParam("CntEdgeVetos",     asynParamInt32, &CntEdgeVetos, 0);          // Number of vetoed events due to close to edge
    createParam("CntLowChargeVetos",asynParamInt32, &CntLowChargeVetos, 0);     // Number of vetoed events doe to low charge
    createParam("CntOverflowVetos", asynParamInt32, &CntOverflowVetos, 0);      // Number of vetoed events due to overflow flag
    createParam("CntMultiEventVetos", asynParamInt32, &CntMultiEventVetos, 0);  // Number of vetoed events due to multiple peaks
    createParam("CntGoodEvents",    asynParamInt32, &CntGoodEvents, 0);         // Number of calculated events
    createParam("CntTotalEvents",   asynParamInt32, &CntTotalEvents, 0);        // Number of events
    createParam("BadConfig",        asynParamInt32, &BadConfig, 0);             // Bad parameters config
    createParam("ResetCnt",         asynParamInt32, &ResetCnt);                 // Reset counters
    createParam("CalcEn",           asynParamInt32, &CalcEn, 0);                // Toggle position calculation
    createParam("NumCalcValues",    asynParamInt32, &NumCalcValues, 3);         // Number of values used in calculation
    createParam("CentroidMin",      asynParamInt32, &CentroidMin, 0);           // Centroid minimum parameter for X,Y calculation
    createParam("XCentroidScale",   asynParamInt32, &XCentroidScale, 100);      // X centroid scale factor
    createParam("YCentroidScale",   asynParamInt32, &YCentroidScale, 70);       // Y centroid scale factor

    for (int i = 0; i < 20; i++) {
        createParam("X" + std::to_string(i+1) + "Scale",          asynParamInt32, &XScales[i], 0);
        createParam("X" + std::to_string(i+1) + "Offset",         asynParamInt32, &XOffsets[i], 0);
        createParam("X" + std::to_string(i+1) + "MinThreshold",   asynParamInt32, &XMinThresholds[i], 0);
    }
    for (int i = 0; i < 17; i++) {
        createParam("Y" + std::to_string(i+1) + "Scale",          asynParamInt32, &YScales[i], 0);
        createParam("Y" + std::to_string(i+1) + "Offset",         asynParamInt32, &YOffsets[i], 0);
        createParam("Y" + std::to_string(i+1) + "MinThreshold",   asynParamInt32, &YMinThresholds[i], 0);
    }
    callParamCallbacks();

    // Defines the output X,Y precision, n=11 as in Qm.n
    m_calcParams.xyDivider = 1 << 11;

    BasePlugin::connect(parentPlugins, {MsgDasData});
}

asynStatus BnlPosCalcPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        m_stats.nTotal = 0;
        m_stats.nGood = 0;
        m_stats.nLowCharge = 0;
        m_stats.nEdge = 0;
        m_stats.nOverflow = 0;
        m_stats.nMultiEvent = 0;
        setIntegerParam(CntEdgeVetos,      0);
        setIntegerParam(CntLowChargeVetos, 0);
        setIntegerParam(CntOverflowVetos,  0);
        setIntegerParam(CntMultiEventVetos,0);
        setIntegerParam(CntGoodEvents,     0);
        setIntegerParam(CntTotalEvents,    0);
        setIntegerParam(BadConfig,         0);
        callParamCallbacks();
        return asynSuccess;
    } else if (pasynUser->reason == LowChargeVetoEn) {
        m_calcParams.lowChargeVetoEn = (value > 0);
        return asynSuccess;
    } else if (pasynUser->reason == EdgeVetoEn) {
        m_calcParams.edgeVetoEn = (value > 0);
        return asynSuccess;
    } else if (pasynUser->reason == OverflowVetoEn) {
        m_calcParams.overflowVetoEn = (value > 0);
        return asynSuccess;
    } else if (pasynUser->reason == MultiEventVetoEn) {
        m_calcParams.multiEventVetoEn = (value > 0);
        return asynSuccess;
    } else if (pasynUser->reason == NumCalcValues) {
        m_calcParams.nCalcValues = (value > 1 ? value : 1);
        return asynSuccess;
    } else if (pasynUser->reason == CentroidMin) {
        m_calcParams.centroidMin = value;
        return asynSuccess;
    } else if (pasynUser->reason == XCentroidScale) {
        m_calcParams.xCentroidScale = 1.0 + value/2048.0;
        return asynSuccess;
    } else if (pasynUser->reason == YCentroidScale) {
        m_calcParams.yCentroidScale = 1.0 + value/2048.0;
        return asynSuccess;
    }
    for (int i = 0; i < 20; i++) {
        if (pasynUser->reason == XScales[i]) {
            m_calcParams.xScales[i] = value;
            m_calcParams.xMinThresholds[i] = value * m_calcParams.xMinThresholdsUnscaled[i];
            return asynSuccess;
        } else if (pasynUser->reason == XOffsets[i]) {
            m_calcParams.xOffsets[i] = value;
            return asynSuccess;
        } else if (pasynUser->reason == XMinThresholds[i]) {
            m_calcParams.xMinThresholdsUnscaled[i] = value;
            m_calcParams.xMinThresholds[i] = m_calcParams.xScales[i] * value;
            return asynSuccess;
        }
    }
    for (int i = 0; i < 17; i++) {
        if (pasynUser->reason == YScales[i]) {
            m_calcParams.yScales[i] = value;
            m_calcParams.yMinThresholds[i] = value * m_calcParams.yMinThresholdsUnscaled[i];
            return asynSuccess;
        } else if (pasynUser->reason == YOffsets[i]) {
            m_calcParams.yOffsets[i] = value;
            return asynSuccess;
        } else if (pasynUser->reason == YMinThresholds[i]) {
            m_calcParams.yMinThresholdsUnscaled[i] = value;
            m_calcParams.yMinThresholds[i] = m_calcParams.yScales[i] * value;
            return asynSuccess;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus BnlPosCalcPlugin::recvParam(const std::string &remotePort, const std::string &paramName, epicsInt32 value)
{
    asynUser a;
    asynStatus ret = asynPortDriver::findParam(paramName.c_str(), &a.reason);
    if (ret == asynSuccess) {
        ret = writeInt32(&a, value);
        if (ret == asynSuccess) {
            // For the read-write asyn mechanism to pick up new values
            setIntegerParam(a.reason, value);
            callParamCallbacks();
        }
    }
    return ret;
}

void BnlPosCalcPlugin::recvDownstream(const DasDataPacketList &packets)
{
    DasDataPacketList outPackets;
    std::vector<DasDataPacket *> pooledPackets;

    if (getBooleanParam(CalcEn) == false) {
        // Optimize pass-thru
        sendDownstream(packets);
        return;
    }

    for (const auto &packet: packets) {
        std::pair<DasDataPacket*, Stats> res = processPacket(packet, m_calcParams);
        if (res.first == packet) {
            // Probably not in right data format for this plugin
            outPackets.push_back(res.first);
        } else if (res.first != nullptr) {
            // We got a newly allocated packet with modified events in it
            outPackets.push_back(res.first);
            pooledPackets.push_back(res.first);
            m_stats += res.second;
        } else {
            // Can't allocate packet
            addIntegerParam(ErrMem, 1);
        }
    }

    // Send to subscribed plugins and wait they complete processing
    sendDownstream(outPackets);

    for (auto &packet: pooledPackets) {
        m_packetsPool.put(packet);
    }

    setIntegerParam(CntEdgeVetos,      m_stats.nEdge);
    setIntegerParam(CntLowChargeVetos, m_stats.nLowCharge);
    setIntegerParam(CntOverflowVetos,  m_stats.nOverflow);
    setIntegerParam(CntMultiEventVetos,m_stats.nMultiEvent);
    setIntegerParam(CntGoodEvents,     m_stats.nGood);
    setIntegerParam(CntTotalEvents,    m_stats.nTotal);
    setIntegerParam(BadConfig,         m_stats.nBadConfig > 0 ? 1 : 0);
    callParamCallbacksRatelimit();
}

std::pair<DasDataPacket *, BnlPosCalcPlugin::Stats> BnlPosCalcPlugin::processPacket(const DasDataPacket *srcPacket, const BnlPosCalcPlugin::CalcParams &calcParams)
{
    Stats stats;
    uint32_t nEvents = srcPacket->getNumEvents();

    if (srcPacket->getEventsFormat() != DasDataPacket::EVENT_FMT_BNL_RAW) {
        return std::make_pair(const_cast<DasDataPacket *>(srcPacket), stats);
    }
    uint32_t newPacketLen = sizeof(DasDataPacket) + nEvents*sizeof(Event::BNL::Diag);
    DasDataPacket *packet = m_packetsPool.get(newPacketLen);
    if (packet) {
        packet->init(DasDataPacket::EVENT_FMT_BNL_DIAG, srcPacket->getTimeStamp(), nEvents);
        auto *destEvent = packet->getEvents<Event::BNL::Diag>();
        auto *srcEvent  = srcPacket->getEvents<const Event::BNL::Raw>();

        while (nEvents-- > 0) {
            *destEvent = *srcEvent;

            if (likely((srcEvent->position & POS_SPECIAL) == 0)) {
                double x,y;
                stats.nTotal++;

                int ret = calculatePosition(srcEvent, calcParams, &x, &y);
                if (ret == CALC_SUCCESS) {
                    destEvent->x = round(x * calcParams.xyDivider);
                    destEvent->y = round(y * calcParams.xyDivider);
                    stats.nGood++;
                } else {
                    destEvent->pixelid |= Event::Pixel::VETO_MASK;
                    destEvent->x = -1;
                    destEvent->y = -1;
                    switch (ret) {
                        case CALC_OVERFLOW_FLAG: stats.nOverflow++;   break;
                        case CALC_EDGE:          stats.nEdge++;       break;
                        case CALC_LOW_CHARGE:    stats.nLowCharge++;  break;
                        case CALC_MULTI_EVENT:   stats.nMultiEvent++; break;
                        case CALC_BAD_CONFIG:    stats.nBadConfig++;  break;
                        default:                                      break;
                    }
                }
            }

            destEvent++;
            srcEvent++;
        }
    }

    return std::make_pair(packet, stats);
}

BnlPosCalcPlugin::calc_return_t BnlPosCalcPlugin::calculatePosition(const Event::BNL::Raw *event, const BnlPosCalcPlugin::CalcParams &calcParams, double *x, double *y)
{
    int32_t xSamples[20];
    int32_t ySamples[17];
    int xPeakIndex = 0; // [0..19]
    int yPeakIndex = 0; // [0..16]
    int32_t denom;
    double num;
    int32_t xCalcGrade;
    int32_t yCalcGrade;
    int32_t left;
    int32_t right;
    uint32_t nMaxima;
    int prevGradient;

    // Check for overflow bit in any raw sample
    if (calcParams.overflowVetoEn == true &&
        ((event->sample_x1  & 0x1FFF) == 0x1FFF ||
         (event->sample_x2  & 0x1FFF) == 0x1FFF ||
         (event->sample_x3  & 0x1FFF) == 0x1FFF ||
         (event->sample_x4  & 0x1FFF) == 0x1FFF ||
         (event->sample_x5  & 0x1FFF) == 0x1FFF ||
         (event->sample_x6  & 0x1FFF) == 0x1FFF ||
         (event->sample_x7  & 0x1FFF) == 0x1FFF ||
         (event->sample_x8  & 0x1FFF) == 0x1FFF ||
         (event->sample_x9  & 0x1FFF) == 0x1FFF ||
         (event->sample_x10 & 0x1FFF) == 0x1FFF ||
         (event->sample_x11 & 0x1FFF) == 0x1FFF ||
         (event->sample_x12 & 0x1FFF) == 0x1FFF ||
         (event->sample_x13 & 0x1FFF) == 0x1FFF ||
         (event->sample_x14 & 0x1FFF) == 0x1FFF ||
         (event->sample_x16 & 0x1FFF) == 0x1FFF ||
         (event->sample_x17 & 0x1FFF) == 0x1FFF ||
         (event->sample_x18 & 0x1FFF) == 0x1FFF ||
         (event->sample_x19 & 0x1FFF) == 0x1FFF ||
         (event->sample_x20 & 0x1FFF) == 0x1FFF ||
         (event->sample_y1  & 0x1FFF) == 0x1FFF ||
         (event->sample_y2  & 0x1FFF) == 0x1FFF ||
         (event->sample_y3  & 0x1FFF) == 0x1FFF ||
         (event->sample_y4  & 0x1FFF) == 0x1FFF ||
         (event->sample_y5  & 0x1FFF) == 0x1FFF ||
         (event->sample_y6  & 0x1FFF) == 0x1FFF ||
         (event->sample_y7  & 0x1FFF) == 0x1FFF ||
         (event->sample_y8  & 0x1FFF) == 0x1FFF ||
         (event->sample_y9  & 0x1FFF) == 0x1FFF ||
         (event->sample_y10 & 0x1FFF) == 0x1FFF ||
         (event->sample_y11 & 0x1FFF) == 0x1FFF ||
         (event->sample_y12 & 0x1FFF) == 0x1FFF ||
         (event->sample_y13 & 0x1FFF) == 0x1FFF ||
         (event->sample_y14 & 0x1FFF) == 0x1FFF ||
         (event->sample_y16 & 0x1FFF) == 0x1FFF ||
         (event->sample_y17 & 0x1FFF) == 0x1FFF)) {
        return CALC_OVERFLOW_FLAG;
    }

    // Unpack raw data
    xSamples[0]  = calcParams.xScales[0]  * (16 * (event->sample_x1  & 0xFFF) - calcParams.xOffsets[0]);
    xSamples[1]  = calcParams.xScales[1]  * (16 * (event->sample_x2  & 0xFFF) - calcParams.xOffsets[1]);
    xSamples[2]  = calcParams.xScales[2]  * (16 * (event->sample_x3  & 0xFFF) - calcParams.xOffsets[2]);
    xSamples[3]  = calcParams.xScales[3]  * (16 * (event->sample_x4  & 0xFFF) - calcParams.xOffsets[3]);
    xSamples[4]  = calcParams.xScales[4]  * (16 * (event->sample_x5  & 0xFFF) - calcParams.xOffsets[4]);
    xSamples[5]  = calcParams.xScales[5]  * (16 * (event->sample_x6  & 0xFFF) - calcParams.xOffsets[5]);
    xSamples[6]  = calcParams.xScales[6]  * (16 * (event->sample_x7  & 0xFFF) - calcParams.xOffsets[6]);
    xSamples[7]  = calcParams.xScales[7]  * (16 * (event->sample_x8  & 0xFFF) - calcParams.xOffsets[7]);
    xSamples[8]  = calcParams.xScales[8]  * (16 * (event->sample_x9  & 0xFFF) - calcParams.xOffsets[8]);
    xSamples[9]  = calcParams.xScales[9]  * (16 * (event->sample_x10 & 0xFFF) - calcParams.xOffsets[9]);
    xSamples[10] = calcParams.xScales[10] * (16 * (event->sample_x11 & 0xFFF) - calcParams.xOffsets[10]);
    xSamples[11] = calcParams.xScales[11] * (16 * (event->sample_x12 & 0xFFF) - calcParams.xOffsets[11]);
    xSamples[12] = calcParams.xScales[12] * (16 * (event->sample_x13 & 0xFFF) - calcParams.xOffsets[12]);
    xSamples[13] = calcParams.xScales[13] * (16 * (event->sample_x14 & 0xFFF) - calcParams.xOffsets[13]);
    xSamples[14] = calcParams.xScales[14] * (16 * (event->sample_x15 & 0xFFF) - calcParams.xOffsets[14]);
    xSamples[15] = calcParams.xScales[15] * (16 * (event->sample_x16 & 0xFFF) - calcParams.xOffsets[15]);
    xSamples[16] = calcParams.xScales[16] * (16 * (event->sample_x17 & 0xFFF) - calcParams.xOffsets[16]);
    xSamples[17] = calcParams.xScales[17] * (16 * (event->sample_x18 & 0xFFF) - calcParams.xOffsets[17]);
    xSamples[18] = calcParams.xScales[18] * (16 * (event->sample_x19 & 0xFFF) - calcParams.xOffsets[18]);
    xSamples[19] = calcParams.xScales[19] * (16 * (event->sample_x20 & 0xFFF) - calcParams.xOffsets[19]);
    ySamples[0]  = calcParams.yScales[0]  * (16 * (event->sample_y1  & 0xFFF) - calcParams.yOffsets[0]);
    ySamples[1]  = calcParams.yScales[1]  * (16 * (event->sample_y2  & 0xFFF) - calcParams.yOffsets[1]);
    ySamples[2]  = calcParams.yScales[2]  * (16 * (event->sample_y3  & 0xFFF) - calcParams.yOffsets[2]);
    ySamples[3]  = calcParams.yScales[3]  * (16 * (event->sample_y4  & 0xFFF) - calcParams.yOffsets[3]);
    ySamples[4]  = calcParams.yScales[4]  * (16 * (event->sample_y5  & 0xFFF) - calcParams.yOffsets[4]);
    ySamples[5]  = calcParams.yScales[5]  * (16 * (event->sample_y6  & 0xFFF) - calcParams.yOffsets[5]);
    ySamples[6]  = calcParams.yScales[6]  * (16 * (event->sample_y7  & 0xFFF) - calcParams.yOffsets[6]);
    ySamples[7]  = calcParams.yScales[7]  * (16 * (event->sample_y8  & 0xFFF) - calcParams.yOffsets[7]);
    ySamples[8]  = calcParams.yScales[8]  * (16 * (event->sample_y9  & 0xFFF) - calcParams.yOffsets[8]);
    ySamples[9]  = calcParams.yScales[9]  * (16 * (event->sample_y10 & 0xFFF) - calcParams.yOffsets[9]);
    ySamples[10] = calcParams.yScales[10] * (16 * (event->sample_y11 & 0xFFF) - calcParams.yOffsets[10]);
    ySamples[11] = calcParams.yScales[11] * (16 * (event->sample_y12 & 0xFFF) - calcParams.yOffsets[11]);
    ySamples[12] = calcParams.yScales[12] * (16 * (event->sample_y13 & 0xFFF) - calcParams.yOffsets[12]);
    ySamples[13] = calcParams.yScales[13] * (16 * (event->sample_y14 & 0xFFF) - calcParams.yOffsets[13]);
    ySamples[14] = calcParams.yScales[14] * (16 * (event->sample_y15 & 0xFFF) - calcParams.yOffsets[14]);
    ySamples[15] = calcParams.yScales[15] * (16 * (event->sample_y16 & 0xFFF) - calcParams.yOffsets[15]);
    ySamples[16] = calcParams.yScales[16] * (16 * (event->sample_y17 & 0xFFF) - calcParams.yOffsets[16]);

    // Normalize samples and find peaks
    nMaxima = 0;
    if (xSamples[0] < calcParams.xMinThresholds[0])
        xSamples[0] = 0;
    prevGradient = xSamples[0];
    for (int i = 1; i < 20; i++) {
        // eliminate raw values below the threshold
        if (xSamples[i] < calcParams.xMinThresholds[i])
            xSamples[i] = 0;

        // find the global maximum
        if (xSamples[i] > xSamples[xPeakIndex])
            xPeakIndex = i;

        // find number of local maxima by detecting when gradient crosses 0
        // in positive-to-negative direction
        int gradient = xSamples[i] - xSamples[i-1];
        if (gradient < 0 && prevGradient >= 0) {
            // we found a local maxima at i-1
            if ((xSamples[i-1] + xSamples[i]) > calcParams.centroidMin)
                nMaxima++;
        }
        prevGradient = gradient;
    }
    if (nMaxima != 1) { // optimize for most likely case
        if (calcParams.multiEventVetoEn == true && nMaxima > 1) {
            return CALC_MULTI_EVENT;
        } else if (calcParams.lowChargeVetoEn == true && nMaxima == 0) {
            return CALC_LOW_CHARGE;
        }
    }

    nMaxima = 0;
    if (ySamples[0] < calcParams.yMinThresholds[0])
        ySamples[0] = 0;
    prevGradient = ySamples[0];
    for (int i = 1; i < 17; i++) {
        // eliminate raw values below the threshold
        if (ySamples[i] < calcParams.yMinThresholds[i])
            ySamples[i] = 0;

        // find the global maximum
        if (ySamples[i] > ySamples[yPeakIndex])
            yPeakIndex = i;

        // find number of local maxima by detecting when gradient crosses 0
        // in positive->negative direction
        int gradient = ySamples[i] - ySamples[i-1];
        if (gradient < 0 && prevGradient >= 0) {
            // we found a local maxima at i-1
            if ((ySamples[i-1] + ySamples[i]) > calcParams.centroidMin)
                nMaxima++;
        }
        prevGradient = gradient;
    }
    if (nMaxima != 1) { // optimize for most likely case
        if (calcParams.multiEventVetoEn == true && nMaxima > 1) {
            return CALC_MULTI_EVENT;
        } else if (calcParams.lowChargeVetoEn == true && nMaxima == 0) {
            return CALC_LOW_CHARGE;
        }
    }

    // Determine number of valid neighbours
    xCalcGrade = std::min(calcParams.nCalcValues/2, std::min(xPeakIndex, 19-xPeakIndex));
    yCalcGrade = std::min(calcParams.nCalcValues/2, std::min(yPeakIndex, 16-yPeakIndex));

    // Rule out edges
    if (calcParams.edgeVetoEn == true && calcParams.nCalcValues > 1 && (xCalcGrade == 0 || yCalcGrade == 0))
        return CALC_EDGE;

    // Tails must be continuosly falling, eliminate any (small) turn arounds
    // Big turn arounds are considered multi-event and were already rejected.
    left = right = xSamples[xPeakIndex];
    for (int i=1; i<=xCalcGrade; i++) {
        if (xSamples[xPeakIndex+i] > right)
            xSamples[xPeakIndex+i] = 0;
        right = xSamples[xPeakIndex+i];

        if (xSamples[xPeakIndex-i] > left)
            xSamples[xPeakIndex-i] = 0;
        left = xSamples[xPeakIndex-i];
    }
    left = right = ySamples[yPeakIndex];
    for (int i=1; i<=yCalcGrade; i++) {
        if (ySamples[yPeakIndex+i] > right)
            ySamples[yPeakIndex+i] = 0;
        right = ySamples[yPeakIndex+i];

        if (ySamples[yPeakIndex-i] > left)
            ySamples[yPeakIndex-i] = 0;
        left = ySamples[yPeakIndex-i];
    }

    // Calculate X,Y position
    if (calcParams.nCalcValues % 2 == 1) { // Odd number of calculation values
        // Interpolate X position using centroid method
        denom = xSamples[xPeakIndex];
        if (denom == 0)
            return CALC_BAD_CONFIG;
        num = 0.0;
        for (int i=1; i<=xCalcGrade; i++) {
            denom +=   (xSamples[xPeakIndex+i] + xSamples[xPeakIndex-i]);
            num   += i*(xSamples[xPeakIndex+i] - xSamples[xPeakIndex-i]);
        }
        *x = xPeakIndex + (num / denom);

        // Interpolate Y position using centroid method
        denom = ySamples[yPeakIndex];
        if (denom == 0)
            return CALC_BAD_CONFIG;
        num = 0.0;
        for (int i=1; i<=yCalcGrade; i++) {
            denom +=   (ySamples[yPeakIndex+i] + ySamples[yPeakIndex-i]);
            num   += i*(ySamples[yPeakIndex+i] - ySamples[yPeakIndex-i]);
        }
        *y = yPeakIndex + (num / denom);
    } else { // or even number of calculation values

        // Interpolate X position using centroid method
        denom = (xCalcGrade > 0 ? 0 : 1);
        if (denom == 0)
            return CALC_BAD_CONFIG;
        num = 0.0;
        if (xSamples[xPeakIndex-1] > xSamples[xPeakIndex+1])
            xPeakIndex--;
        for (int i=1; i<=xCalcGrade; i++) {
            denom +=         (xSamples[xPeakIndex+i] + xSamples[xPeakIndex-i+1]);
            num   += (i-0.5)*(xSamples[xPeakIndex+i] - xSamples[xPeakIndex-i+1]);
        }
        *x = xPeakIndex + 0.5 + (num / denom);

        // Interpolate Y position using centroid method
        denom = (yCalcGrade > 0 ? 0 : 1);
        if (denom == 0)
            return CALC_BAD_CONFIG;
        num = 0.0;
        if (ySamples[yPeakIndex-1] > ySamples[yPeakIndex+1])
            yPeakIndex--;
        for (int i=1; i<=yCalcGrade; i++) {
            denom +=         (ySamples[yPeakIndex+i] + ySamples[yPeakIndex-i+1]);
            num   += (i-0.5)*(ySamples[yPeakIndex+i] - ySamples[yPeakIndex-i+1]);
        }
        *y = yPeakIndex + 0.5 + (num / denom);
    }

    return CALC_SUCCESS;
}
