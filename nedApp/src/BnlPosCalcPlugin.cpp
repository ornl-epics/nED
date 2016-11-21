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

#define CALC_ALTERNATIVE

EPICS_REGISTER_PLUGIN(BnlPosCalcPlugin, 3, "Port name", string, "Dispatcher port name", string, "Buffer size in bytes", int);

#define NUM_BNLPOSCALCPLUGIN_PARAMS      ((int)(&LAST_BNLPOSCALCPLUGIN_PARAM - &FIRST_BNLPOSCALCPLUGIN_PARAM + 1)) + (20 + 17)*3

#define POS_SPECIAL     (1 << 30)

BnlPosCalcPlugin::BnlPosCalcPlugin(const char *portName, const char *dispatcherPortName, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_BNLPOSCALCPLUGIN_PARAMS, asynOctetMask, asynOctetMask)
    , m_correctionScale(1.0)
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

    createParam("ErrMem",       asynParamInt32, &ErrMem, m_bufferSize==1); // Buffer allocation error
    createParam("XyFractWidth", asynParamInt32, &XyFractWidth, 11);        // WRITE - Number of fraction bits in X,Y data
    createParam("LowChargeVetoEn",  asynParamInt32, &LowChargeVetoEn, 1);  // Toggle low charge vetos
    createParam("EdgeVetoEn",       asynParamInt32, &EdgeVetoEn, 1);       // Toggle edge vetos
    createParam("OverflowVetoEn",   asynParamInt32, &OverflowVetoEn, 1);   // Toggle overflow vetos
    createParam("MultiEventVetoEn", asynParamInt32, &MultiEventVetoEn, 1); // Toggle multi-event vetos
    createParam("CntEdgeVetos",     asynParamInt32, &CntEdgeVetos, 0);     // Number of vetoed events due to close to edge
    createParam("CntLowChargeVetos",asynParamInt32, &CntLowChargeVetos, 0);// Number of vetoed events doe to low charge
    createParam("CntOverflowVetos", asynParamInt32, &CntOverflowVetos, 0); // Number of vetoed events due to overflow flag
    createParam("CntMultiEventVetos", asynParamInt32, &CntMultiEventVetos, 0); // Number of vetoed events due to multiple peaks
    createParam("CntGoodEvents",    asynParamInt32, &CntGoodEvents, 0);    // Number of calculated events
    createParam("CntTotalEvents",   asynParamInt32, &CntTotalEvents, 0);   // Number of events
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);            // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);                // Reset counters
    createParam("CalcEn",       asynParamInt32, &CalcEn, 0);               // Toggle position calculation
    createParam("NumCalcValues",asynParamInt32, &NumCalcValues, 3);        // Number of values used in calculation
    createParam("CentroidMin",  asynParamInt32, &CentroidMin, 0);          // Centroid minimum parameter for X,Y calculation
#ifndef CALC_ALTERNATIVE
    createParam("XCentroidScale", asynParamInt32, &XCentroidScale, 100);   // X centroid scale factor
    createParam("YCentroidScale", asynParamInt32, &YCentroidScale, 70);    // Y centroid scale factor
#endif
    createParam("PvaName",      asynParamOctet, &PvaName);                 // PVA name for calculation verification data

    for (int i = 0; i < 20; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "X%dScale", i+1);
        createParam(buf,        asynParamInt32, &XScales[i], 0);
        snprintf(buf, sizeof(buf), "X%dOffset", i+1);
        createParam(buf,        asynParamInt32, &XOffsets[i], 0);
#ifdef CALC_ALTERNATIVE
        snprintf(buf, sizeof(buf), "X%dMinThreshold", i+1);
        createParam(buf,        asynParamInt32, &XMinThresholds[i], 0);
#endif
    }
    for (int i = 0; i < 17; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "Y%dScale", i+1);
        createParam(buf,        asynParamInt32, &YScales[i], 0);
        snprintf(buf, sizeof(buf), "Y%dOffset", i+1);
        createParam(buf,        asynParamInt32, &YOffsets[i], 0);
#ifdef CALC_ALTERNATIVE
        snprintf(buf, sizeof(buf), "Y%dMinThreshold", i+1);
        createParam(buf,        asynParamInt32, &YMinThresholds[i], 0);
#endif
    }
    callParamCallbacks();
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
        callParamCallbacks();
        return asynSuccess;
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

asynStatus BnlPosCalcPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == PvaName) {
        PvaCalcVerifyData::destroy(m_pva);
        if (nChars > 0) {
            m_pva = PvaCalcVerifyData::create(value);
            if (!m_pva)
                LOG_ERROR("Cannot create PVA record '%s'", value);
        }
    }
    return BaseDispatcherPlugin::writeOctet(pasynUser, value, nChars, nActual);;
}

void BnlPosCalcPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nSplits = 0;
    bool calcEn = false;
    int val;

    this->lock();
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(CntSplit,       &nSplits);
    getBooleanParam(CalcEn,         &calcEn);
    // Although these are class variables, only set them here and not from writeInt32().
    // This prevents thread race conditions since the code below is not in thread safe section.
    getBooleanParam(LowChargeVetoEn,&m_lowChargeVetoEn);
    getBooleanParam(EdgeVetoEn,     &m_edgeVetoEn);
    getBooleanParam(OverflowVetoEn, &m_overflowVetoEn);
    getBooleanParam(MultiEventVetoEn, &m_multiEventVetoEn);
    getIntegerParam(NumCalcValues,  &m_nCalcValues);
    if (m_nCalcValues < 1) m_nCalcValues = 1;
    getIntegerParam(XyFractWidth,   &val);
    if (val > 15) val = 15;
    if (val < 0)  val = 0;
    m_xyDivider = 1 << val;
    getIntegerParam(CentroidMin,    &m_centroidMin);
    getIntegerParam(XCentroidScale, &val);
    m_xCentroidScale = 1.0 + val/2048.0;
    getIntegerParam(YCentroidScale, &val);
    m_yCentroidScale = 1.0 + val/2048.0;

    if (calcEn == true) {
        for (int i = 0; i < 20; i++) {
            getIntegerParam(XScales[i],        &m_xScales[i]);
            getIntegerParam(XOffsets[i],       &m_xOffsets[i]);
#ifdef CALC_ALTERNATIVE
            getIntegerParam(XMinThresholds[i], &m_xMinThresholds[i]);
            m_xMinThresholds[i] *= m_xScales[i];
#endif
        }
        for (int i = 0; i < 17; i++) {
            getIntegerParam(YScales[i],        &m_yScales[i]);
            getIntegerParam(YOffsets[i],       &m_yOffsets[i]);
#ifdef CALC_ALTERNATIVE
            getIntegerParam(YMinThresholds[i], &m_yMinThresholds[i]);
            m_yMinThresholds[i] *= m_yScales[i];
#endif
        }
    }
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
        bool extendedMode = (getDataMode() == BasePlugin::DATA_MODE_EXTENDED);

        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;
            m_packetList.clear();
            m_packetList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < packet->getLength()) {
                    nSplits++;
                    break;
                }

                nProcessed++;

                // Reuse the original packet if nothing to map
                if (packet->isNeutronData() == false) {
                    m_packetList.push_back(packet);
                    continue;
                }

                // Reserve part of buffer for this packet, it may shrink from original but never grow
                DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                m_packetList.push_back(newPacket);
                bufferOffset += packet->getLength();

                // Process the packet - only raw mode supported for now
                m_stats += processPacket(packet, newPacket, extendedMode);
            }

            sendToPlugins(&m_packetList);
            m_packetList.release();
            m_packetList.waitAllReleased();
        }

        if (extendedMode && m_pva)
            m_pva->post();

    }

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntEdgeVetos,      m_stats.nEdge);
    setIntegerParam(CntLowChargeVetos, m_stats.nLowCharge);
    setIntegerParam(CntOverflowVetos,  m_stats.nOverflow);
    setIntegerParam(CntMultiEventVetos,m_stats.nMultiEvent);
    setIntegerParam(CntGoodEvents,     m_stats.nGood);
    setIntegerParam(CntTotalEvents,    m_stats.nTotal);
    callParamCallbacks();
    this->unlock();
}

BnlPosCalcPlugin::Stats BnlPosCalcPlugin::processPacket(const DasPacket *srcPacket, DasPacket *destPacket, bool extendedMode)
{
    Stats stats;
    uint32_t eventSize = (extendedMode ? sizeof(BnlDataPacket::ExtendedEvent) : sizeof(BnlDataPacket::RawEvent));

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->getLength());

    uint32_t nEvents, nDestEvents;
    const char *data = reinterpret_cast<const char *>(srcPacket->getData(&nEvents));
    BnlDataPacket::NormalEvent *destEvent = reinterpret_cast<BnlDataPacket::NormalEvent *>(destPacket->getData(&nDestEvents));
    nEvents /= (eventSize / sizeof(uint32_t));

    nDestEvents = 0;
    while (nEvents-- > 0) {
        const BnlDataPacket::RawEvent *srcEvent= reinterpret_cast<const BnlDataPacket::RawEvent *>(data);

        if (likely((srcEvent->position & POS_SPECIAL) == 0)) {
            double x,y;

            stats.nTotal++;

            int ret = calculatePosition(srcEvent, &x, &y);
            if (ret == CALC_SUCCESS) {
                destEvent->tof = srcEvent->tof;
                destEvent->position = srcEvent->position;
                destEvent->x = round(x * m_xyDivider);
                destEvent->y = round(y * m_xyDivider);
                destEvent++;
                nDestEvents++;
                stats.nGood++;
            } else if (ret == CALC_OVERFLOW_FLAG) {
                stats.nOverflow++;
            } else if (ret == CALC_EDGE) {
                stats.nEdge++;
            } else if (ret == CALC_LOW_CHARGE) {
                stats.nLowCharge++;
            } else if (ret == CALC_MULTI_EVENT) {
                stats.nMultiEvent++;
            }

            if (unlikely(extendedMode && m_pva)) {
                const BnlDataPacket::ExtendedEvent *extEvent = reinterpret_cast<const BnlDataPacket::ExtendedEvent *>(data);
                m_pva->time_of_flight.push_back(extEvent->tof);
                m_pva->firmware_x.push_back(extEvent->x / m_xyDivider);
                m_pva->firmware_y.push_back(extEvent->y / m_xyDivider);
                m_pva->software_x.push_back(x);
                m_pva->software_y.push_back(y);
            }
        }

        data += eventSize;
    }
    destPacket->payload_length += nDestEvents * sizeof(BnlDataPacket::NormalEvent);

    return stats;
}

BnlPosCalcPlugin::calc_return_t BnlPosCalcPlugin::calculatePosition(const BnlDataPacket::RawEvent *event, double *x, double *y)
{
    int32_t xSamples[20];
    int32_t ySamples[17];
    int xPeakIndex = 0; // [0..19]
    int yPeakIndex = 0; // [0..16]
    int32_t denom;
    double num;
#ifdef CALC_ALTERNATIVE
    int32_t xCalcGrade;
    int32_t yCalcGrade;
    int32_t left;
    int32_t right;
    uint32_t nMaxima;
    int prevGradient;
#endif

    // Check for overflow bit in any raw sample
    if (m_overflowVetoEn == true &&
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
    xSamples[0]  = m_xScales[0]  * (16 * (event->sample_x1  & 0xFFF) - m_xOffsets[0]);
    xSamples[1]  = m_xScales[1]  * (16 * (event->sample_x2  & 0xFFF) - m_xOffsets[1]);
    xSamples[2]  = m_xScales[2]  * (16 * (event->sample_x3  & 0xFFF) - m_xOffsets[2]);
    xSamples[3]  = m_xScales[3]  * (16 * (event->sample_x4  & 0xFFF) - m_xOffsets[3]);
    xSamples[4]  = m_xScales[4]  * (16 * (event->sample_x5  & 0xFFF) - m_xOffsets[4]);
    xSamples[5]  = m_xScales[5]  * (16 * (event->sample_x6  & 0xFFF) - m_xOffsets[5]);
    xSamples[6]  = m_xScales[6]  * (16 * (event->sample_x7  & 0xFFF) - m_xOffsets[6]);
    xSamples[7]  = m_xScales[7]  * (16 * (event->sample_x8  & 0xFFF) - m_xOffsets[7]);
    xSamples[8]  = m_xScales[8]  * (16 * (event->sample_x9  & 0xFFF) - m_xOffsets[8]);
    xSamples[9]  = m_xScales[9]  * (16 * (event->sample_x10 & 0xFFF) - m_xOffsets[9]);
    xSamples[10] = m_xScales[10] * (16 * (event->sample_x11 & 0xFFF) - m_xOffsets[10]);
    xSamples[11] = m_xScales[11] * (16 * (event->sample_x12 & 0xFFF) - m_xOffsets[11]);
    xSamples[12] = m_xScales[12] * (16 * (event->sample_x13 & 0xFFF) - m_xOffsets[12]);
    xSamples[13] = m_xScales[13] * (16 * (event->sample_x14 & 0xFFF) - m_xOffsets[13]);
    xSamples[14] = m_xScales[14] * (16 * (event->sample_x15 & 0xFFF) - m_xOffsets[14]);
    xSamples[15] = m_xScales[15] * (16 * (event->sample_x16 & 0xFFF) - m_xOffsets[15]);
    xSamples[16] = m_xScales[16] * (16 * (event->sample_x17 & 0xFFF) - m_xOffsets[16]);
    xSamples[17] = m_xScales[17] * (16 * (event->sample_x18 & 0xFFF) - m_xOffsets[17]);
    xSamples[18] = m_xScales[18] * (16 * (event->sample_x19 & 0xFFF) - m_xOffsets[18]);
    xSamples[19] = m_xScales[19] * (16 * (event->sample_x20 & 0xFFF) - m_xOffsets[19]);
    ySamples[0]  = m_yScales[0]  * (16 * (event->sample_y1  & 0xFFF) - m_yOffsets[0]);
    ySamples[1]  = m_yScales[1]  * (16 * (event->sample_y2  & 0xFFF) - m_yOffsets[1]);
    ySamples[2]  = m_yScales[2]  * (16 * (event->sample_y3  & 0xFFF) - m_yOffsets[2]);
    ySamples[3]  = m_yScales[3]  * (16 * (event->sample_y4  & 0xFFF) - m_yOffsets[3]);
    ySamples[4]  = m_yScales[4]  * (16 * (event->sample_y5  & 0xFFF) - m_yOffsets[4]);
    ySamples[5]  = m_yScales[5]  * (16 * (event->sample_y6  & 0xFFF) - m_yOffsets[5]);
    ySamples[6]  = m_yScales[6]  * (16 * (event->sample_y7  & 0xFFF) - m_yOffsets[6]);
    ySamples[7]  = m_yScales[7]  * (16 * (event->sample_y8  & 0xFFF) - m_yOffsets[7]);
    ySamples[8]  = m_yScales[8]  * (16 * (event->sample_y9  & 0xFFF) - m_yOffsets[8]);
    ySamples[9]  = m_yScales[9]  * (16 * (event->sample_y10 & 0xFFF) - m_yOffsets[9]);
    ySamples[10] = m_yScales[10] * (16 * (event->sample_y11 & 0xFFF) - m_yOffsets[10]);
    ySamples[11] = m_yScales[11] * (16 * (event->sample_y12 & 0xFFF) - m_yOffsets[11]);
    ySamples[12] = m_yScales[12] * (16 * (event->sample_y13 & 0xFFF) - m_yOffsets[12]);
    ySamples[13] = m_yScales[13] * (16 * (event->sample_y14 & 0xFFF) - m_yOffsets[13]);
    ySamples[14] = m_yScales[14] * (16 * (event->sample_y15 & 0xFFF) - m_yOffsets[14]);
    ySamples[15] = m_yScales[15] * (16 * (event->sample_y16 & 0xFFF) - m_yOffsets[15]);
    ySamples[16] = m_yScales[16] * (16 * (event->sample_y17 & 0xFFF) - m_yOffsets[16]);

#ifndef CALC_ALTERNATIVE
    // For code sanity, don't optimize finding peak position into data parsing
    // above. Any good compiler will roll-out the for loop anyway.
    for (int i = 1; i < 20; i++) {
        if (xSamples[i] > xSamples[xPeakIndex])
            xPeakIndex = i;
    }
    for (int i = 1; i < 17; i++) {
        if (ySamples[i] > ySamples[yPeakIndex])
            yPeakIndex = i;
    }

    // Filter out common vetoes early
    if (xPeakIndex == 0 || xPeakIndex == 19 || yPeakIndex == 0 || yPeakIndex == 16)
        return CALC_EDGE;

    // Interpolate X position
    denom = xSamples[xPeakIndex+1] + xSamples[xPeakIndex] + xSamples[xPeakIndex-1];
    num   = xSamples[xPeakIndex+1]                        - xSamples[xPeakIndex-1];
    if (denom < m_centroidMin)
        return CALC_LOW_CHARGE;
    *x = xPeakIndex + (m_xCentroidScale * num / denom);

    // Interpolate Y position
    denom = ySamples[yPeakIndex+1] + ySamples[yPeakIndex] + ySamples[yPeakIndex-1];
    num   = ySamples[yPeakIndex+1]                        - ySamples[yPeakIndex-1];
    if (denom < m_centroidMin)
        return CALC_LOW_CHARGE;
    *y = yPeakIndex + (m_yCentroidScale * num / denom);

    return CALC_SUCCESS;

#else // CALC_ALTERNATIVE

    // Normalize samples and find peaks
    nMaxima = 0;
    if (xSamples[0] < m_xMinThresholds[0])
        xSamples[0] = 0;
    prevGradient = xSamples[0];
    for (int i = 1; i < 20; i++) {
        // eliminate raw values below the threshold
        if (xSamples[i] < m_xMinThresholds[i])
            xSamples[i] = 0;

        // find the global maximum
        if (xSamples[i] > xSamples[xPeakIndex])
            xPeakIndex = i;

        // find number of local maxima by detecting when gradient crosses 0
        // in positive-to-negative direction
        int gradient = xSamples[i] - xSamples[i-1];
        if (gradient < 0 && prevGradient >= 0) {
            // we found a local maxima at i-1
            if ((xSamples[i-1] + xSamples[i]) > m_centroidMin)
                nMaxima++;
        }
        prevGradient = gradient;
    }
    if (nMaxima != 1) { // optimize for most likely case
        if (m_multiEventVetoEn == true && nMaxima > 1) {
            return CALC_MULTI_EVENT;
        } else if (m_lowChargeVetoEn == true && nMaxima == 0) {
            return CALC_LOW_CHARGE;
        }
    }

    nMaxima = 0;
    if (ySamples[0] < m_yMinThresholds[0])
        ySamples[0] = 0;
    prevGradient = ySamples[0];
    for (int i = 1; i < 17; i++) {
        // eliminate raw values below the threshold
        if (ySamples[i] < m_yMinThresholds[i])
            ySamples[i] = 0;

        // find the global maximum
        if (ySamples[i] > ySamples[yPeakIndex])
            yPeakIndex = i;

        // find number of local maxima by detecting when gradient crosses 0
        // in positive->negative direction
        int gradient = ySamples[i] - ySamples[i-1];
        if (gradient < 0 && prevGradient >= 0) {
            // we found a local maxima at i-1
            if ((ySamples[i-1] + ySamples[i]) > m_centroidMin)
                nMaxima++;
        }
        prevGradient = gradient;
    }
    if (nMaxima != 1) { // optimize for most likely case
        if (m_multiEventVetoEn == true && nMaxima > 1) {
            return CALC_MULTI_EVENT;
        } else if (m_lowChargeVetoEn == true && nMaxima == 0) {
            return CALC_LOW_CHARGE;
        }
    }

    // Determine number of valid neighbours
    xCalcGrade = std::min(m_nCalcValues/2, std::min(xPeakIndex, 19-xPeakIndex));
    yCalcGrade = std::min(m_nCalcValues/2, std::min(yPeakIndex, 16-yPeakIndex));

    // Rule out edges
    if (m_edgeVetoEn == true && m_nCalcValues > 1 && (xCalcGrade == 0 || yCalcGrade == 0))
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
    if (m_nCalcValues % 2 == 1) { // Odd number of calculation values
        // Interpolate X position using centroid method
        denom = xSamples[xPeakIndex];
        num = 0.0;
        for (int i=1; i<=xCalcGrade; i++) {
            denom +=   (xSamples[xPeakIndex+i] + xSamples[xPeakIndex-i]);
            num   += i*(xSamples[xPeakIndex+i] - xSamples[xPeakIndex-i]);
        }
        *x = xPeakIndex + (num / denom);

        // Interpolate Y position using centroid method
        denom = ySamples[yPeakIndex];
        num = 0.0;
        for (int i=1; i<=yCalcGrade; i++) {
            denom +=   (ySamples[yPeakIndex+i] + ySamples[yPeakIndex-i]);
            num   += i*(ySamples[yPeakIndex+i] - ySamples[yPeakIndex-i]);
        }
        *y = yPeakIndex + (num / denom);
    } else { // or even number of calculation values

        // Interpolate X position using centroid method
        denom = (xCalcGrade > 0 ? 0 : 1);
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
#endif // CALC_ALTERNATIVE
}
