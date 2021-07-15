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

EPICS_REGISTER_PLUGIN(TofCorrectPlugin, 3, "Port name", string, "Remote port name", string, "Number pixels", int);

TofCorrectPlugin::TofCorrectPlugin(const char *portName, const char *connectPortName, int nPixels)
    : BaseDispatcherPlugin(portName, connectPortName, /*blocking=*/0, NUM_TOFCORRECTPLUGIN_PARAMS+3*nPixels)
    , m_nReceived(0)
    , m_nProcessed(0)
    , m_corrections(nPixels)
    , PixelIds(nPixels)
    , TofOffsets(nPixels)
    , NCorrected(nPixels)
{
    createParam("FrameLen",     asynParamInt32, &FrameLen,    0); // WRITE - Frame length in nsec
    createParam("PoolSize",     asynParamInt32, &PoolSize,    0); // READ - Number of allocated packets
    createParam("NPixels",      asynParamInt32, &NPixels,     nPixels); // READ - Number of pixels to correct

    for (int i=0; i<nPixels; i++) {
        char name[16];
        int param;
        snprintf(name, sizeof(name), "TofOffset%d", i+1);
        createParam(name, asynParamInt32, &param, 0); // WRITE - Full TOF offset to be applied in nsec
        TofOffsets[i] = param;
        snprintf(name, sizeof(name), "PixelId%d", i+1);
        createParam(name, asynParamInt32, &param, 0);   // WRITE - Selected pixel id
        PixelIds[i] = param;
        snprintf(name, sizeof(name), "NCorrected%d", i+1);
        createParam(name, asynParamInt32, &param, 0); // READ - Number of corrected events
        NCorrected[i] = param;
    }

    // Initialize bare minimum, others should be set through PVs
    for (int i=0; i<nPixels; i++) {
        m_corrections[i].enabled = false;
        m_corrections[i].pixelId = 0;
        m_corrections[i].nEvents = 0;
    }
}

TofCorrectPlugin::~TofCorrectPlugin()
{
    for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
        delete it->packet;
    }
}

asynStatus TofCorrectPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == FrameLen) {
        m_frameLen = value / 100;
    } else {
        for (size_t i=0; i<m_corrections.size(); i++) {
            if (pasynUser->reason == TofOffsets[i]) {
                m_corrections[i].tofOffset = value/100;
                break;
            } else if (pasynUser->reason == PixelIds[i]) {
                m_corrections[i].pixelId = value;
                break;
            }
        }
    }

    for (size_t i=0; i<m_corrections.size(); i++) {
        m_corrections[i].enabled = false;
        if (m_frameLen > 0 && m_corrections[i].pixelId != 0) {
            uint32_t frameNum = m_corrections[i].tofOffset / m_frameLen;
            m_corrections[i].fineOffset = m_corrections[i].tofOffset % m_frameLen;
            m_corrections[i].coarseOffset = frameNum * m_frameLen;
            m_corrections[i].enabled = true;
        }
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

void TofCorrectPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    std::list<DasPacket *> allocPktsList;
    DasPacketList modifiedPktsList;

    this->lock();
    bool enabled = false;
    for (size_t i=0; i<m_corrections.size(); i++) {
        if (m_corrections[i].enabled == true) {
            enabled = true;
            break;
        }
    }
    this->unlock();

    if (enabled == false) {
        modifiedPktsList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&modifiedPktsList);
        modifiedPktsList.release();
        modifiedPktsList.waitAllReleased();
        modifiedPktsList.clear();
    } else {
        this->lock();
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
                        int index = -1;
                        for (size_t j=0; j<m_corrections.size(); j++) {
                            if (dstData[i+1] == (uint32_t)m_corrections[j].pixelId) {
                                index = j;
                                break;
                            }
                        }
                        if (index == -1 || m_corrections[index].enabled == false)
                            continue;

                        uint32_t tof = dstData[i] & 0x000FFFFF;
                        // Do the frame correction
                        if (tof < m_corrections[index].fineOffset) {
                            // Account for misaligned trigger
                            tof += m_corrections[index].coarseOffset + m_frameLen;
                        } else {
                            tof += m_corrections[index].coarseOffset;
                        }
                        dstData[i] = (dstData[i] & 0xFFF00000) | tof;

                        m_corrections[index].nEvents++;
                    }
                    modifiedPktsList.push_back(modifiedPacket);
                }
            } else {
                modifiedPktsList.push_back(packet);
            }
        }
        this->unlock();
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
    for (size_t i=0; i<m_corrections.size(); i++) {
        setIntegerParam(NCorrected[i], m_corrections[i].nEvents);
    }
    callParamCallbacks();
    this->unlock();
}

DasPacket *TofCorrectPlugin::allocPacket(uint32_t size)
{
    DasPacket *packet = reinterpret_cast<DasPacket *>(0);

    for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
        if (it->size >= size && it->inUse == false) {
            it->inUse = true;
            packet = it->packet;
            break;
        }
    }

    if (!packet) {
        packet = DasPacket::alloc(size);
        if (!packet) {
            LOG_ERROR("Failed to allocate packet buffer");
        } else {
            PoolEntry entry;
            entry.inUse = true;
            entry.size = size;
            entry.packet = packet;
            m_pool.push_back(entry);
            LOG_DEBUG("Increased packet pool to %zu", m_pool.size());
        }
    }

    return packet;
}

void TofCorrectPlugin::freePacket(DasPacket *packet)
{
    for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
        if (it->packet == packet) {
            if (it->inUse == false) {
                LOG_WARN("Corrupted pool, packet returned twice?");
            }
            it->inUse = false;
            return;
        }
    }
    LOG_WARN("Packet not found in pool, discarding");
    delete packet;
}
