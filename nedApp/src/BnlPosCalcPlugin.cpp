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

EPICS_REGISTER_PLUGIN(BnlPosCalcPlugin, 3, "Port name", string, "Dispatcher port name", string, "Buffer size in bytes", int);

#define NUM_DATACONVERTPLUGIN_PARAMS      ((int)(&LAST_DATACONVERTPLUGIN_PARAM - &FIRST_DATACONVERTPLUGIN_PARAM + 1)) + (20 + 17)*2

#define POS_SPECIAL     (1 << 30)

BnlPosCalcPlugin::BnlPosCalcPlugin(const char *portName, const char *dispatcherPortName, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_DATACONVERTPLUGIN_PARAMS)
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
    createParam("CntVetoEvents",asynParamInt32, &CntVetoEvents, 0);        // Number of vetoed events
    createParam("CntGoodEvents",asynParamInt32, &CntGoodEvents, 0);        // Number of calculated events
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);            // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);                // Reset counters
    createParam("CalcEn",       asynParamInt32, &CalcEn, 0);               // Toggle position calculation
    createParam("CentroidMin",  asynParamInt32, &CentroidMin, 0);          // Centroid minimum parameter for X,Y calculation
    createParam("XCentroidScale", asynParamInt32, &XCentroidScale, 100);   // X centroid scale factor
    createParam("YCentroidScale", asynParamInt32, &YCentroidScale, 70);    // Y centroid scale factor

    for (int i = 0; i < 20; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "X%dScale", i+1);
        createParam(buf,        asynParamInt32, &XScales[i], 0);
        snprintf(buf, sizeof(buf), "X%dOffset", i+1);
        createParam(buf,        asynParamInt32, &XOffsets[i], 0);
    }
    for (int i = 0; i < 17; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "Y%dScale", i+1);
        createParam(buf,        asynParamInt32, &YScales[i], 0);
        snprintf(buf, sizeof(buf), "Y%dOffset", i+1);
        createParam(buf,        asynParamInt32, &YOffsets[i], 0);
    }
    callParamCallbacks();
}

void BnlPosCalcPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nSplits = 0;
    int nVeto = 0;
    int nGood = 0;
    bool calcEn = false;
    int val;

    this->lock();
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(CntVetoEvents,  &nVeto);
    getIntegerParam(CntGoodEvents,  &nGood);
    getIntegerParam(CntSplit,       &nSplits);
    getBooleanParam(CalcEn,         &calcEn);
    // Although these are class variables, only set them here and not from writeInt32().
    // This prevents thread race conditions since the code below is not in thread safe section.
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
            getIntegerParam(XScales[i],  &m_xScales[i]);
            getIntegerParam(XOffsets[i], &m_xOffsets[i]);
        }
        for (int i = 0; i < 17; i++) {
            getIntegerParam(YScales[i],  &m_yScales[i]);
            getIntegerParam(YOffsets[i], &m_yOffsets[i]);
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
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;
            m_packetList.clear();
            m_packetList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;
                uint32_t nGoodTmp = 0;
                uint32_t nVetoTmp = 0;

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < packet->length()) {
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
                bufferOffset += packet->length();

                // Process the packet - only raw mode supported for now
                processPacket(packet, newPacket, nGoodTmp, nVetoTmp);
                nGood += nGoodTmp;
                nVeto += nVetoTmp;
            }

            sendToPlugins(&m_packetList);
            m_packetList.release();
            m_packetList.waitAllReleased();
        }
    }

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntVetoEvents, nVeto    % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntGoodEvents, nGood    % std::numeric_limits<int32_t>::max());
    callParamCallbacks();
    this->unlock();
}

void BnlPosCalcPlugin::processPacket(const DasPacket *srcPacket, DasPacket *destPacket, uint32_t &nCalced, uint32_t &nVetoed)
{
    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents, nDestEvents, nTemp;
    const BnlDataPacket::RawEvent *srcEvent= reinterpret_cast<const BnlDataPacket::RawEvent *>(srcPacket->getData(&nEvents));
    BnlDataPacket::NormalEvent *destEvent = reinterpret_cast<BnlDataPacket::NormalEvent *>(destPacket->getData(&nDestEvents));
    nEvents /= (sizeof(BnlDataPacket::RawEvent) / sizeof(uint32_t));

    nTemp = nEvents;
    while (nTemp-- > 0) {
        if (likely((srcEvent->position & POS_SPECIAL) == 0)) {
            double x,y;
            if (calculatePosition(srcEvent, &x, &y) == true) {
                destEvent->tof = srcEvent->tof;
                destEvent->position = srcEvent->position;
                destEvent->x = round(x * m_xyDivider);
                destEvent->y = round(y * m_xyDivider);
                destEvent++;
                nDestEvents++;
            }
        }
        srcEvent++;
    }
    destPacket->payload_length += nDestEvents * sizeof(BnlDataPacket::NormalEvent);
    nCalced = nDestEvents;
    nVetoed = (nEvents - nDestEvents);
}

bool BnlPosCalcPlugin::calculatePosition(const BnlDataPacket::RawEvent *event, double *x, double *y)
{
    int32_t xSamples[20];
    int32_t ySamples[17];
    uint8_t xMaxIndex = 0; // [0..19]
    uint8_t yMaxIndex = 0; // [0..16]
    int32_t denom;

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

    // For code sanity, don't optimize finding max position into data parsing
    // above. Any good compiler will roll-out the for loop anyway.
    for (int i = 1; i < 20; i++) {
        if (xSamples[i] > xSamples[xMaxIndex])
            xMaxIndex = i;
    }
    for (int i = 1; i < 17; i++) {
        if (ySamples[i] > ySamples[yMaxIndex])
            yMaxIndex = i;
    }

    // Filter out common vetoes early
    if (xMaxIndex == 0 || xMaxIndex == 19 || yMaxIndex == 0 || yMaxIndex == 16)
        return false;

    // Interpolate X position
    denom = xSamples[xMaxIndex+1] + xSamples[xMaxIndex] + xSamples[xMaxIndex-1];
    if (denom < m_centroidMin)
        return false;
    *x = xMaxIndex + (m_xCentroidScale * (xSamples[xMaxIndex+1] - xSamples[xMaxIndex-1]) / denom);

    // Interpolate Y position
    denom = ySamples[yMaxIndex+1] + ySamples[yMaxIndex] + ySamples[yMaxIndex-1];
    if (denom < m_centroidMin)
        return false;
    *y = yMaxIndex + (m_yCentroidScale * (ySamples[yMaxIndex+1] - ySamples[yMaxIndex-1]) / denom);

    return true;
}
