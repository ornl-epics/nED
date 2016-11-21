/* TofCorrectPlugin.cpp
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 * @date July 13, 2016
 */

#include "TofCorrectPlugin.h"
#include "Log.h"

#include <string.h> // memcpy()

#define NUM_TOFCORRECTPLUGIN_PARAMS ((int)(&LAST_TOFCORRECTPLUGIN_PARAM - &FIRST_TOFCORRECTPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(TofCorrectPlugin, 2, "Port name", string, "Remote port name", string);

const uint32_t TofCorrectPlugin::PACKET_SIZE =  sizeof(DasPacket) + DasPacket::MaxLength;

TofCorrectPlugin::TofCorrectPlugin(const char *portName, const char *connectPortName)
    : BaseDispatcherPlugin(portName, connectPortName, /*blocking=*/0, NUM_TOFCORRECTPLUGIN_PARAMS)
    , m_nReceived(0)
    , m_nProcessed(0)
{
    createParam("PoolSize",     asynParamInt32, &PoolSize,    0); // READ - Number of allocated packets
    createParam("TofOffset1",   asynParamInt32, &TofOffset1,  0); // WRITE - Full TOF offset to be applied in nsec
    createParam("PixelId1",     asynParamInt32, &PixelId1,    0); // WRITE - Select pixel id
    createParam("FrameLen1",    asynParamInt32, &FrameLen1,   0); // WRITE - Frame length in nsec
    createParam("NCorrected1",  asynParamInt32, &NCorrected1, 0); // READ - Number of corrected events
    createParam("TofOffset2",   asynParamInt32, &TofOffset2,  0); // WRITE - Full TOF offset to be applied in nsec
    createParam("PixelId2",     asynParamInt32, &PixelId2,    0); // WRITE - Select pixel id
    createParam("FrameLen2",    asynParamInt32, &FrameLen2,   0); // WRITE - Frame length in nsec
    createParam("NCorrected2",  asynParamInt32, &NCorrected2, 0); // READ - Number of corrected events
    createParam("TofOffset3",   asynParamInt32, &TofOffset3,  0); // WRITE - Full TOF offset to be applied in nsec
    createParam("PixelId3",     asynParamInt32, &PixelId3,    0); // WRITE - Select pixel id
    createParam("FrameLen3",    asynParamInt32, &FrameLen3,   0); // WRITE - Frame length in nsec
    createParam("NCorrected3",  asynParamInt32, &NCorrected3, 0); // READ - Number of corrected events
}

void TofCorrectPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    std::list<DasPacket *> allocPktsList;
    DasPacketList modifiedPktsList;
    int tofOffset[3];
    int pixelId[3];
    int frameLen[3];
    uint32_t coarseOffset[3];
    uint32_t fineOffset[3];
    uint32_t frameNum[3];
    int32_t nEvents[3];

    this->lock();
    getIntegerParam(TofOffset1,  &tofOffset[0]);
    getIntegerParam(FrameLen1,   &frameLen[0]);
    getIntegerParam(PixelId1,    &pixelId[0]);
    getIntegerParam(NCorrected1, &nEvents[0]);
    getIntegerParam(TofOffset2,  &tofOffset[1]);
    getIntegerParam(FrameLen2,   &frameLen[1]);
    getIntegerParam(PixelId2,    &pixelId[1]);
    getIntegerParam(NCorrected2, &nEvents[1]);
    getIntegerParam(TofOffset3,  &tofOffset[2]);
    getIntegerParam(FrameLen3,   &frameLen[2]);
    getIntegerParam(PixelId3,    &pixelId[2]);
    getIntegerParam(NCorrected3, &nEvents[2]);
    this->unlock();

    bool enabled = false;
    for (int i=0; i<3; i++) {
        if (pixelId[i] != 0) {
            enabled = true;
            if (frameLen[i] == 0)
                frameLen[i] = 1;
            tofOffset[i] /= 100;
            frameLen[i] /= 100;
            fineOffset[i] = tofOffset[i] % frameLen[i];
            frameNum[i] = tofOffset[i] / frameLen[i];
            coarseOffset[i] = frameNum[i] * frameLen[i];
        }
    }

    if (enabled == false) {
        modifiedPktsList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&modifiedPktsList);
        modifiedPktsList.release();
        modifiedPktsList.waitAllReleased();
        modifiedPktsList.clear();
    } else {
        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;

            if (packet->isMetaData()) {
                DasPacket *modifiedPacket = allocPacket(packet->getLength());
                if (!modifiedPacket)
                    continue;
                allocPktsList.push_back(modifiedPacket);

                if (packet->copyHeader(modifiedPacket, packet->getLength())) {
                    uint32_t nDwords;
                    uint32_t *dstData = modifiedPacket->getData(&nDwords);
                    const uint32_t *srcData = packet->getData(&nDwords);

                    // memcpy() is fast
                    memcpy(dstData, srcData, nDwords * 4);
                    modifiedPacket->payload_length += nDwords * 4;

                    for (uint32_t i=0; (i+1)<nDwords; i+=2) {
                        // Find 0-2 index into correction params table,
                        // or skip event if pixel not to be corrected
                        int index = -1;
                        for (int j=0; j<3; j++) {
                            if (dstData[i+1] == (uint32_t)pixelId[j]) {
                                index = j;
                                break;
                            }
                        }
                        if (index == -1)
                            continue;

                        uint32_t tof = dstData[i] & 0x000FFFFF;
                        // Do the frame correction
                        if (tof < fineOffset[index]) {
                            // Account for misaligned trigger
                            tof += coarseOffset[index] + frameLen[index];
                        } else {
                            tof += coarseOffset[index];
                        }
                        dstData[i] = (dstData[i] & 0xFFF00000) | tof;

                        nEvents[index]++;
                    }
                    modifiedPktsList.push_back(modifiedPacket);
                }
            } else {
                modifiedPktsList.push_back(packet);
            }
        }
    }

    // Serialize sending to plugins - can not use asynPortDriver::lock()
    m_mutex.lock();

    if (!modifiedPktsList.empty()) {
        modifiedPktsList.reserve();
        BaseDispatcherPlugin::sendToPlugins(&modifiedPktsList);
        modifiedPktsList.release();
        modifiedPktsList.waitAllReleased();
    }
    // All plugins done with list of packets, put them back to pool
    for (auto it = allocPktsList.cbegin(); it != allocPktsList.cend(); it++) {
        freePacket(const_cast<DasPacket *>(*it));
    }

    m_mutex.unlock();

    // Update parameters
    this->lock();
    m_nReceived += packetList->size();
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    setIntegerParam(PoolSize,   m_pool.size());
    setIntegerParam(NCorrected1,nEvents[0] > 0 ? nEvents[0] : 0);
    setIntegerParam(NCorrected2,nEvents[1] > 0 ? nEvents[1] : 0);
    setIntegerParam(NCorrected3,nEvents[2] > 0 ? nEvents[2] : 0);
    callParamCallbacks();
    this->unlock();
}

DasPacket *TofCorrectPlugin::allocPacket(uint32_t size)
{
    DasPacket *packet;

    if (size > PACKET_SIZE) {
        LOG_ERROR("New packet size must not exceed %u", PACKET_SIZE);
        return reinterpret_cast<DasPacket *>(0);
    }

    if (m_pool.size() > 0) {
        packet = m_pool.front();
        m_pool.pop_front();
    } else {
        packet = reinterpret_cast<DasPacket *>(calloc(1, PACKET_SIZE));
        if (!packet) {
            LOG_ERROR("Failed to allocate packet buffer");
            return reinterpret_cast<DasPacket *>(0);
        }
    }

    return packet;
}

void TofCorrectPlugin::freePacket(DasPacket *packet)
{
    m_pool.push_back(packet);
}
