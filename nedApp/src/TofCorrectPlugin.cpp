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
    createParam("TofOffset",    asynParamInt32, &TofOffset,   0); // WRITE - Full TOF offset to be applied in nsec
    createParam("PixelId",      asynParamInt32, &PixelId,     0); // WRITE - Select pixel id
    createParam("FrameLen",     asynParamInt32, &FrameLen,    0); // WRITE - Frame length in nsec
}

void TofCorrectPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    std::list<DasPacket *> allocPktsList;
    DasPacketList modifiedPktsList;
    int tofOffset;
    int pixelId;
    int frameLen;
    uint32_t coarseOffset;
    uint32_t fineOffset;
    uint32_t frameNum;

    this->lock();
    getIntegerParam(TofOffset, &tofOffset);
    getIntegerParam(FrameLen, &frameLen);
    getIntegerParam(PixelId, &pixelId);
    this->unlock();

//LOG_ERROR("tofOffset=%d, frameLen=%d, pixelId=0x%08X", tofOffset, frameLen, pixelId);
    if (frameLen == 0 || tofOffset == 0) {
        modifiedPktsList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&modifiedPktsList);
        modifiedPktsList.release();
        modifiedPktsList.waitAllReleased();
        modifiedPktsList.clear();
    } else {
        tofOffset /= 100;
        frameLen /= 100;
        fineOffset = tofOffset % frameLen;
        frameNum = tofOffset / frameLen;
        coarseOffset = frameNum * frameLen;
        
        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;
    
            if (packet->isMetaData()) {
//LOG_ERROR("tofOffset=%d frameLen=%d, fineOffset=%d, frameNum=%d, coarseOffset=%d", tofOffset, frameLen, fineOffset, frameNum, coarseOffset);
                DasPacket *modifiedPacket = allocPacket(packet->length());
                if (!modifiedPacket)
                    continue;
                allocPktsList.push_back(modifiedPacket);
    
                if (packet->copyHeader(modifiedPacket, packet->length())) {
                    uint32_t nDwords;
                    uint32_t *dstData = modifiedPacket->getData(&nDwords);
                    const uint32_t *srcData = packet->getData(&nDwords);

                    // memcpy() is fast
                    memcpy(dstData, srcData, nDwords * 4);
                    modifiedPacket->payload_length += nDwords * 4;

                    for (uint32_t i=0; (i+1)<nDwords; i+=2) {
                        // Only match selected pixelId
                        if (dstData[i+1] == (uint32_t)pixelId) {
                            uint32_t tof = dstData[i] & 0x000FFFFF;
                            // Do the frame correction
                            if (tof < fineOffset) {
                                // Account for misaligned trigger
                                tof += coarseOffset + frameLen;
                            } else {
                                tof += coarseOffset;
                            }
                            dstData[i] = (dstData[i] & 0xFFF00000) | tof;
//LOG_ERROR("adjusting pixel 0x%08X: %d => %d", pixelId, srcData[i], dstData[i]);
                        }
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
