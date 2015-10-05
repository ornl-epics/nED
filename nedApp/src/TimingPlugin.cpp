/* TimingPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "TimingPlugin.h"
#include "Log.h"

#include <cstring> // memcpy
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NUM_TIMINGPLUGIN_PARAMS ((int)(&LAST_TIMINGPLUGIN_PARAM - &FIRST_TIMINGPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(TimingPlugin, 2, "Port name", string, "Remote port name", string);

const uint32_t TimingPlugin::PACKET_SIZE =  sizeof(DasPacket) + DasPacket::MaxLength;

TimingPlugin::TimingPlugin(const char *portName, const char *connectPortName)
    : BaseDispatcherPlugin(portName, connectPortName, /*blocking=*/0, NUM_TIMINGPLUGIN_PARAMS)
    , m_nReceived(0)
    , m_nProcessed(0)
    , m_timer(false)
    , m_socket(-1)
{
    m_rtdlPacket = reinterpret_cast<DasPacket *>(
        callocMustSucceed(1, sizeof(DasPacket) + sizeof(RtdlHeader) + 104, "Failed to allocate RTDL packet")
    );
    m_rtdlPacket->source = 0x0CC;
    m_rtdlPacket->destination = 0x0CC;
    m_rtdlPacket->cmdinfo.is_command = 1;
    m_rtdlPacket->cmdinfo.command = DasPacket::CMD_RTDL;
    m_rtdlPacket->payload_length = sizeof(RtdlHeader) + 104;

    // Nobody (ADARA) doesn't really use RTDL frames information.
    // ADARA ignores it if MSB of 32-bit frame is 0x00. calloc() zeroed it,
    // don't change for fake or network RTDL.

    createParam("PoolSize",     asynParamInt32, &PoolSize,    0); // READ - Number of allocated packets
    createParam("Mode",         asynParamInt32, &Mode,        0); // WRITE - Source where to ger RTDL data from
    createParam("RecvPort",     asynParamInt32, &RecvPort, 8055); // WRITE - Remote port

    createFakeRtdl(m_rtdlPacket);
    std::function<float(void)> timerCb = std::bind(&TimingPlugin::updateRtdl, this);
    m_timer.schedule(timerCb, 0.01);
}

asynStatus TimingPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Mode) {
        switch (value) {
        case 0:
            disconnectEtc();
            break;
        case 1:
            int port;
            if (getIntegerParam(RecvPort, &port) != asynSuccess || port == 0) {
                LOG_ERROR("Can not connect to timing server: RecvPort not configured");
                return asynError;
            }
            disconnectEtc();
            if (connectEtc(port) == false) {
                LOG_ERROR("Can not connect to timing server: connection refused");
                return asynError;
            }
            break;
        default:
            return asynError;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void TimingPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    DasPacketList modifiedPktsList;
    DasPacketList originalPktsList;

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        if (packet->isData() && packet->getRtdlHeader() == 0) {
            RtdlHeader *rtdl = 0;

            if (packet->datainfo.subpacket_start) {
                this->lock();
                if (packet->isNeutronData())
                    m_neutronsRtdl = *m_rtdlPacket->getRtdlHeader();
                else
                    m_metaRtdl = *m_rtdlPacket->getRtdlHeader();
                this->unlock();

                // TODO: If RTDL is the same, entire train should be tossed or
                //       start/end flags need to be modified.
            }

            if (packet->isNeutronData())
                rtdl = &m_neutronsRtdl;
            else
                rtdl = &m_metaRtdl;

            const DasPacket *timedPacket = timestampPacket(packet, rtdl);
            if (timedPacket) {
                modifiedPktsList.push_back(timedPacket);
                m_nProcessed++;
            }
        } else {
            originalPktsList.push_back(packet);
        }
    }

    // Can not use asynPortDriver::lock()
    m_mutex.lock();

    // Packet order is not preserved here, but nearly all packets are expected
    // to be modified so it doesn't really matter much. It does simplify the code
    // much though.
    if (!modifiedPktsList.empty()) {
        modifiedPktsList.reserve();
        BaseDispatcherPlugin::sendToPlugins(&modifiedPktsList);
        // All plugins done with list of packets, put them back to pool
        for (auto it = modifiedPktsList.cbegin(); it != modifiedPktsList.cend(); it++) {
            freePacket(const_cast<DasPacket *>(*it));
        }
    }
    if (!originalPktsList.empty()) {
        originalPktsList.reserve();
        BaseDispatcherPlugin::sendToPlugins(&originalPktsList);
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

const DasPacket *TimingPlugin::timestampPacket(const DasPacket *src, const RtdlHeader *rtdl) {
    DasPacket *packet;
    uint32_t nDwords;

    if (src->length() + sizeof(RtdlHeader) > PACKET_SIZE) {
        LOG_WARN("Packet size %u exceeds upper limit %lu, not timestamping data packet", src->length(), PACKET_SIZE - sizeof(RtdlHeader));
        return reinterpret_cast<DasPacket *>(0);
    }

    // Get new packet buffer
    packet = allocPacket(src->length() + sizeof(RtdlHeader));
    if (!packet)
        return packet;

    // Copy packet header first
    memcpy(packet, src, sizeof(DasPacket));

    // Add RTDL header next
    RtdlHeader *rtdlHdr = reinterpret_cast<RtdlHeader *>(packet->getData(&nDwords));
    memcpy(rtdlHdr, rtdl, sizeof(RtdlHeader));
    packet->payload_length = sizeof(RtdlHeader);
    packet->datainfo.rtdl_present = 1;

    // And finally copy the payload
    const uint32_t *srcPayload = src->getData(&nDwords);
    uint32_t *destPayload = packet->getData(&nDwords);
    memcpy(destPayload, srcPayload, nDwords * 4);
    packet->payload_length += nDwords * 4;

    return packet;
}

DasPacket *TimingPlugin::allocPacket(uint32_t size)
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

void TimingPlugin::freePacket(DasPacket *packet)
{
    m_pool.push_back(packet);
}

double TimingPlugin::updateRtdl()
{
    bool sendRtdlPacket = false;
    int mode;

    getIntegerParam(Mode, &mode);

    this->lock();
    // Must not block
    if (mode == 0)
        sendRtdlPacket = createFakeRtdl(m_rtdlPacket);
    else if (mode == 1)
        sendRtdlPacket = recvRtdlFromEtc(m_rtdlPacket);
    this->unlock();

    // NOTE: No guarantee that there's a new RTDL data in 16ms.
    //       Data for two pulses can be merged together in that case.

    if (sendRtdlPacket) {
        m_mutex.lock();
        DasPacketList packetList;
        packetList.reset(m_rtdlPacket);
        BaseDispatcherPlugin::sendToPlugins(&packetList);
        m_mutex.unlock();
    }

    return 0.001;
}

bool TimingPlugin::createFakeRtdl(DasPacket *packet)
{
    epicsTimeStamp now, rtdlTime;
    RtdlHeader *rtdl = const_cast<RtdlHeader *>(packet->getRtdlHeader());

    epicsTimeGetCurrent(&now);
    now.nsec = (now.nsec / 16666667) * 16666667; // assume 60Hz
    rtdlTime.secPastEpoch = rtdl->timestamp_sec;
    rtdlTime.nsec = rtdl->timestamp_nsec;

    // Based on 60Hz, skip if no change
    if (epicsTimeEqual(&now, &rtdlTime) == 1)
        return false;

    rtdl->timestamp_sec = now.secPastEpoch;
    rtdl->timestamp_nsec = now.nsec;
    rtdl->cycle = (rtdl->cycle + 1) % 600;
    rtdl->last_cycle_veto = 0;
    rtdl->pulse.charge = 100000 + 5 * rand();
    rtdl->tsync_width = (rtdl->cycle == 0x1 ? 166662 : 166661);
    rtdl->tstat = 30;
    rtdl->pulse.flavor = (rtdl->cycle > 0 ? RtdlHeader::RTDL_FLAVOR_TARGET_1 : RtdlHeader::RTDL_FLAVOR_NO_BEAM);
    rtdl->tof_full_offset = 1;

    return true;
}

bool TimingPlugin::connectEtc(uint32_t port)
{
    struct sockaddr_in addr;

    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == -1)
        return false;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(m_socket);
        m_socket = -1;
        return false;
    }

    int bcast = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (char *)&bcast, sizeof(bcast)) == -1) {
        close(m_socket);
        m_socket = -1;
        return false;
    }

    int flags = fcntl(m_socket, F_GETFL, 0);
    if (flags == -1 || fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(m_socket);
        m_socket = -1;
        return false;
    }

    return true;
}

void TimingPlugin::disconnectEtc()
{
    if (m_socket != -1) {
        close(m_socket);
        m_socket = -1;
    }
}

bool TimingPlugin::recvRtdlFromEtc(DasPacket *packet)
{
    struct EtcRtdlPacket {
        uint32_t receive_id;
        uint32_t command;
        uint32_t size;
        uint32_t spares[3];

        uint32_t nsec;
        uint32_t secPastEpoch;
        uint32_t pulse_type;
        uint32_t veto_status;
        uint32_t pulse_charge;
        uint32_t stored_turns;
        uint32_t ring_period;
        uint32_t spare;
    } message;

    // m_socket is O_NONBLOCK
    while (recv(m_socket, &message, sizeof(message), 0) > 0) {
        if (message.command == 0x10000 && message.size >= 32) {
            RtdlHeader *rtdl = const_cast<RtdlHeader *>(packet->getRtdlHeader());
            uint32_t *data = reinterpret_cast<uint32_t *>(rtdl) + sizeof(RtdlHeader)/4;

            rtdl->timestamp_sec = message.secPastEpoch;
            rtdl->timestamp_nsec = message.nsec;
            rtdl->charge = message.pulse_type;
            rtdl->pulse.bad = (message.pulse_type >> 6) & 0x1;
            rtdl->pulse.charge = message.pulse_charge & 0xFFFFFF;
            rtdl->cycle = (rtdl->cycle + 1) % 600;
            rtdl->tsync_width = (rtdl->cycle == 0x1 ? 166662 : 166661);
            rtdl->tsync_delay = 0x80000000;

            // Ring revolution period frame id is 4
            data[0] = 0x04 | (message.ring_period & 0xFFFFFF);

            return true;
        }
        // Eat other commands
    }

    return false;
}
