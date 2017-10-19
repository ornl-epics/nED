/* EmptyPulsePlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 * @date September 27, 2017
 */

#include "EmptyPulsePlugin.h"
#include "Log.h"

#include <string.h> // memcpy()

#define NUM_EMPTYPULSEPLUGIN_PARAMS ((int)(&LAST_EMPTYPULSEPLUGIN_PARAM - &FIRST_EMPTYPULSEPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(EmptyPulsePlugin, 3, "Port name", string, "Remote port name", string, "Number pixels", int);

EmptyPulsePlugin::EmptyPulsePlugin(const char *portName, const char *connectPortName, int blocking)
    : BaseDispatcherPlugin(portName, connectPortName, blocking, NUM_EMPTYPULSEPLUGIN_PARAMS)
    , m_nReceived(0)
    , m_nProcessed(0)
{
    createParam("InjectEn",    asynParamInt32, &InjectEn,    0); // WRITE - Flag to enable empty packet injection
    createParam("RtdlBufSize", asynParamInt32, &RtdlBufSize,10); // WRITE - Max number of RTDLs in queue
    createParam("SkipPulses",  asynParamInt32, &SkipPulses,  0); // WRITE - Number of pulses to skip, 1 for 30 Hz operation
    createParam("PktPoolSize", asynParamInt32, &PktPoolSize, 0); // READ - Number of allocated packets
}

EmptyPulsePlugin::~EmptyPulsePlugin()
{
    for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
        delete it->packet;
    }
}

void EmptyPulsePlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    std::list<DasPacket *> allocPktsList;
    DasPacketList modifiedPktsList;
    bool enabled = true;
    int rtdlFifoMaxSize = 10;
    int skipRtdls = 0;

    this->lock();
    getBooleanParam(InjectEn, &enabled);
    getIntegerParam(RtdlBufSize, &rtdlFifoMaxSize);
    getIntegerParam(SkipPulses, &skipRtdls);
    this->unlock();

    if (enabled == false) {
        modifiedPktsList.reset(packetList); // reset() automatically reserves
    } else {
        this->lock();
        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;
            const RtdlHeader *rtdl = packet->getRtdlHeader();

            if (packet->isRtdl() && rtdl) {
                // Store a single instance of RTDL in queue,
                // Limited by rtdlFifoMaxSize to prevent memory leaks in case
                // of no events. Queue is reduced on incoming data packets.
                if (m_rtdls.back().timestamp_sec  != rtdl->timestamp_sec ||
                    m_rtdls.back().timestamp_nsec != rtdl->timestamp_nsec) {

                    m_rtdls.push_back(*rtdl);
                    if (m_rtdls.size() > (size_t)rtdlFifoMaxSize) {
                        m_rtdls.pop_front();
                    }
                }
            } else if (packet->isNeutronData() && rtdl) {

                bool foundRtdl = false;
                for (auto it=m_rtdls.begin(); it!=m_rtdls.end(); it++) {
                    if (it->timestamp_sec  == rtdl->timestamp_sec &&
                        it->timestamp_nsec == rtdl->timestamp_nsec) {

                        foundRtdl = (it != m_rtdls.begin());
                        break;
                    }
                }

                if (foundRtdl) {
                    // Remove previous pulse RTDL
                    m_rtdls.pop_front();

                    int remain = skipRtdls;
                    while (m_rtdls.front().timestamp_sec  != rtdl->timestamp_sec ||
                           m_rtdls.front().timestamp_nsec != rtdl->timestamp_nsec) {

                        if (remain == 0) {
                            DasPacket *emptyPacket = allocPacket(m_rtdls.front());
                            if (emptyPacket) {
                                modifiedPktsList.push_back(emptyPacket);
                                allocPktsList.push_back(emptyPacket);
                            }
                            remain = skipRtdls;
                        } else {
                            remain--;
                        }
                        m_rtdls.pop_front();
                    }
                }
            }
            modifiedPktsList.push_back(packet);
        }
        if (!modifiedPktsList.empty()) {
            modifiedPktsList.reserve();
        }
        this->unlock();
    }

    // Serialize sending to plugins - can not use asynPortDriver::lock()
    m_mutex.lock();

    if (!modifiedPktsList.empty()) {
        // modifiedPktsList was previously reserve()d
        BaseDispatcherPlugin::sendToPlugins(&modifiedPktsList);
        modifiedPktsList.release();
        modifiedPktsList.waitAllReleased();
    }
    // All plugins done with list of packets, put them back to pool
    for (auto it = allocPktsList.begin(); it != allocPktsList.end(); it++) {
        freePacket(*it);
    }

    m_mutex.unlock();

    // Update parameters
    this->lock();
    m_nReceived += packetList->size();
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    setIntegerParam(PktPoolSize,m_pool.size());
    callParamCallbacks();
    this->unlock();
}

DasPacket *EmptyPulsePlugin::allocPacket(const RtdlHeader &rtdl)
{
    size_t size = sizeof(DasPacket) + sizeof(RtdlHeader);
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

    if (packet) {
        memset(packet, 0, sizeof(DasPacket));
        packet->source = 0x0CC;
        packet->destination = 0x0CC;
        packet->datainfo.subpacket_start = 1;
        packet->datainfo.subpacket_end = 1;
        packet->datainfo.only_neutron_data = 1;
        packet->datainfo.rtdl_present = 1;
        packet->payload_length = sizeof(RtdlHeader);
        memcpy(packet->payload, &rtdl, sizeof(RtdlHeader));
    }

    return packet;
}

void EmptyPulsePlugin::freePacket(DasPacket *packet)
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
