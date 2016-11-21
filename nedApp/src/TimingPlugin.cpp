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
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NUM_TIMINGPLUGIN_PARAMS ((int)(&LAST_TIMINGPLUGIN_PARAM - &FIRST_TIMINGPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(TimingPlugin, 2, "Port name", string, "Remote port name", string);

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
    createParam("Connected",    asynParamInt32, &Connected,   0); // READ - Flag whether ETC connection is established
    createParam("FakeRtdlCnt",  asynParamInt32, &FakeRtdlCnt, 0); // READ - Number of fake RTDL packets generated
    createParam("EtcRtdlCnt",   asynParamInt32, &EtcRtdlCnt,  0); // READ - Number of packets received from ETC computer

    createFakeRtdl(m_rtdlPacket);
    // Use static values for RTDL frames
    m_rtdlPacket->payload[6] = 0x040e927d;
    m_rtdlPacket->payload[7] = 0x05db4924;
    m_rtdlPacket->payload[8] = 0x06363186;
    m_rtdlPacket->payload[9] = 0x070003c3;
    m_rtdlPacket->payload[10] = 0x080927c0;
    m_rtdlPacket->payload[11] = 0x0f000000;
    m_rtdlPacket->payload[12] = 0x11000001;
    m_rtdlPacket->payload[13] = 0x18000004;
    m_rtdlPacket->payload[14] = 0x19000029;
    m_rtdlPacket->payload[15] = 0x1a00003c;
    m_rtdlPacket->payload[16] = 0x1c07c000;
    m_rtdlPacket->payload[17] = 0x1d000062;
    m_rtdlPacket->payload[18] = 0x1e01435f;
    m_rtdlPacket->payload[19] = 0x1f000b12;
    m_rtdlPacket->payload[20] = 0x2000105a;
    m_rtdlPacket->payload[21] = 0x21000000;
    m_rtdlPacket->payload[22] = 0x22000000;
    m_rtdlPacket->payload[23] = 0x231e0deb;
    m_rtdlPacket->payload[24] = 0x24000f7b;
    m_rtdlPacket->payload[25] = 0x2503b056;
    m_rtdlPacket->payload[26] = 0x26041086;
    m_rtdlPacket->payload[27] = 0x27000000;
    m_rtdlPacket->payload[28] = 0x2800fffe;
    m_rtdlPacket->payload[29] = 0x29000002;
    m_rtdlPacket->payload[30] = 0x012fe9ea;
    m_rtdlPacket->payload[31] = 0x02fa1e2d;
    std::function<float(void)> timerCb = std::bind(&TimingPlugin::updateRtdl, this);
    m_timer.schedule(timerCb, 0.01);
}

TimingPlugin::~TimingPlugin()
{
    for (auto it=m_pool.begin(); it!=m_pool.end(); it++) {
        delete it->packet;
    }
}

asynStatus TimingPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Mode) {
        bool connected = false;
        asynStatus status = asynError;
        switch (value) {
        case 0:
            disconnectEtc();
            status = asynSuccess;
            break;
        case 1:
            int port;
            if (getIntegerParam(RecvPort, &port) != asynSuccess || port == 0) {
                LOG_ERROR("Can not connect to timing server: RecvPort not configured");
                break;
            }
            disconnectEtc();
            if (connectEtc(port) == false) {
                LOG_ERROR("Can not connect to timing server: connection refused");
                break;
            } else {
                connected = true;
                status = asynSuccess;
            }
            break;
        default:
            int oldmode;
            getIntegerParam(Mode, &oldmode);
            LOG_ERROR("Invalid operation mode '%d', reverting to %d", value, oldmode);
            value = oldmode;
            break;
        }

        setIntegerParam(Connected, connected);
        setIntegerParam(Mode, value);
        callParamCallbacks();
        return status;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void TimingPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    DasPacketList modifiedPktsList;
    DasPacketList originalPktsList;

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        DasPacket::DataTypeLegacy type = packet->getDataTypeLegacy();
        if (packet->isData() && type != DasPacket::DATA_TYPE_NEUTRON_RTDL) {
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

            const DasPacket *timedPacket = timestampPacket(packet, rtdl, type != DasPacket::DATA_TYPE_METADATA);
            if (timedPacket) {
                modifiedPktsList.push_back(timedPacket);
                m_nProcessed++;
            }
        } else {
            originalPktsList.push_back(packet);
        }
    }

    // Serialize sending to plugins - can not use asynPortDriver::lock()
    m_mutex.lock();

    // Packet order is not preserved here, but nearly all packets are expected
    // to be modified so it doesn't really matter much. It does simplify the code
    // much though.
    if (!modifiedPktsList.empty()) {
        modifiedPktsList.reserve();
        BaseDispatcherPlugin::sendToPlugins(&modifiedPktsList);
        modifiedPktsList.release();
        modifiedPktsList.waitAllReleased();
        // All plugins done with list of packets, put them back to pool
        for (auto it = modifiedPktsList.cbegin(); it != modifiedPktsList.cend(); it++) {
            freePacket(const_cast<DasPacket *>(*it));
        }
    }
    if (!originalPktsList.empty()) {
        originalPktsList.reserve();
        BaseDispatcherPlugin::sendToPlugins(&originalPktsList);
        originalPktsList.release();
        originalPktsList.waitAllReleased();
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

const DasPacket *TimingPlugin::timestampPacket(const DasPacket *src, const RtdlHeader *rtdl, bool onlyNeutrons) {
    DasPacket *packet;
    uint32_t nDwords;
    uint32_t maxSize = DasPacket::getMaxLength() - sizeof(RtdlHeader);

    if (src->getLength() > maxSize) {
        LOG_WARN("Packet size %u exceeds upper limit %u, not timestamping data packet", src->getLength(), maxSize);
        return reinterpret_cast<DasPacket *>(0);
    }

    // Get new packet buffer
    packet = allocPacket(src->getLength() + sizeof(RtdlHeader));
    if (!packet)
        return packet;

    // Copy packet header first
    memcpy(packet, src, sizeof(DasPacket));

    // Add RTDL header next
    RtdlHeader *rtdlHdr = reinterpret_cast<RtdlHeader *>(packet->getData(&nDwords));
    memcpy(rtdlHdr, rtdl, sizeof(RtdlHeader));
    packet->payload_length = sizeof(RtdlHeader);
    packet->datainfo.rtdl_present = 1;
    packet->datainfo.only_neutron_data = onlyNeutrons;

    // And finally copy the payload
    uint32_t *destPayload = packet->getData(&nDwords);
    const uint32_t *srcPayload = src->getData(&nDwords);
    memcpy(destPayload, srcPayload, nDwords * 4);
    packet->payload_length += nDwords * 4;

    return packet;
}

DasPacket *TimingPlugin::allocPacket(uint32_t size)
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

void TimingPlugin::freePacket(DasPacket *packet)
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

double TimingPlugin::updateRtdl()
{
    int enabled;
    epicsTimeStamp t1, t2;

    epicsTimeGetCurrent(&t1);

    this->lock();
    getIntegerParam(Enable, &enabled);
    this->unlock();

    if (enabled == 1) {
        bool sendRtdlPacket = false;
        int mode;
        getIntegerParam(Mode, &mode);

        this->lock();
        // This is the only place where m_rtdlPacket is changed
        if (mode == 0)
            sendRtdlPacket = createFakeRtdl(m_rtdlPacket);
        else if (mode == 1)
            sendRtdlPacket = recvRtdlFromEtc(m_rtdlPacket);

        if (sendRtdlPacket) {
            int counter;
            getIntegerParam(mode == 0 ? FakeRtdlCnt : EtcRtdlCnt, &counter);
            counter = (++counter > 0 ? counter : 0);
            setIntegerParam(mode == 0 ? FakeRtdlCnt : EtcRtdlCnt, counter);
            callParamCallbacks();
        }
        this->unlock();

        // NOTE: No guarantee that there's a new RTDL data in 16ms.
        //       Data for two pulses can be merged together in that case.

        if (sendRtdlPacket) {
            m_mutex.lock();
            DasPacketList packetList;
            packetList.reset(m_rtdlPacket); // reset automatically reserves
            BaseDispatcherPlugin::sendToPlugins(&packetList);
            packetList.release();
            packetList.waitAllReleased();
            m_mutex.unlock();
        }
    }

    epicsTimeGetCurrent(&t2);
    double wait = 1.0/60 - epicsTimeDiffInSeconds(&t2, &t1);
    if (wait < 0.0)
        wait = 1.0/60;

    return wait;
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
    rtdl->tsync_period = (rtdl->cycle == 0x1 ? 166662 : 166661);
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
            rtdl->tsync_period = (rtdl->cycle == 0x1 ? 166662 : 166661);
            rtdl->tsync_delay = 0x80000000;

            // Ring revolution period frame id is 4
            data[0] = (0x04 << 24) | (message.ring_period & 0xFFFFFF);

            return true;
        }
        // Eat other commands
    }

    return false;
}
