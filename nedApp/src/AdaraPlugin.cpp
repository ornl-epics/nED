/* AdaraPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AdaraPlugin.h"
#include "Event.h"
#include "Log.h"

#include <algorithm>
#include <string.h>

EPICS_REGISTER_PLUGIN(AdaraPlugin, 3, "port name", string, "Parent data plugins", string, "Parent RTDL plugins", string);

#define ADARA_PKT_TYPE_RAW_EVENT    0x00000000
#define ADARA_PKT_TYPE_RTDL         0x00000100
#define ADARA_PKT_TYPE_SOURCE_LIST  0x00000200
#define ADARA_PKT_TYPE_MAPPED_EVENT 0x00000300
#define ADARA_PKT_TYPE_HEARTBEAT    0x00400900

AdaraPlugin::AdaraPlugin(const char *portName, const char *dataPlugins, const char *rtdlPlugins)
    : BaseSocketPlugin(portName)
{
    createParam("Enable",       asynParamInt32, &Enable, 0);     // WRITE - Enable sending events to SMS
    createParam("Reset",        asynParamInt32, &Reset, 0);      // WRITE - Reset internal logic
    createParam("RtdlCacheSize",asynParamInt32, &RtdlCacheSize, 30);// WRITE - Number of RTDLs to cache
    createParam("CntDataPkts",  asynParamInt32, &CntDataPkts, 0);// WRITE - Number of data packets sent to SMS
    createParam("CntRtdlPkts",  asynParamInt32, &CntRtdlPkts, 0);// WRITE - Number of RTDL packets sent to SMS
    createParam("CntPingPkts",  asynParamInt32, &CntPingPkts, 0);// WRITE - Number of heartbeat packets sent to SMS
    createParam("DetEventsEn",  asynParamInt32, &DetEventsEn, 1);// WRITE - Enable forwarding detector events
    createParam("MonEventsEn",  asynParamInt32, &MonEventsEn, 1);// WRITE - Enable forwarding beam monitor events
    createParam("SigEventsEn",  asynParamInt32, &SigEventsEn, 1);// WRITE - Enable forwarding signal events
    createParam("AdcEventsEn",  asynParamInt32, &AdcEventsEn, 1);// WRITE - Enable forwarding ADC events
    createParam("ChopEventsEn", asynParamInt32, &ChopEventsEn, 1);// WRITE - Enable forwarding chopper events
    callParamCallbacks();

    BasePlugin::connect(dataPlugins, MsgDasData);
    BasePlugin::connect(rtdlPlugins, MsgDasRtdl);
}

asynStatus AdaraPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Reset) {
        reset();
        return asynSuccess;
    }
    if (pasynUser->reason == DetEventsEn) {
        m_eventsSel[int(Event::Pixel::Type::NEUTRON)] = (value > 0);
    } else if (pasynUser->reason == MonEventsEn) {
        m_eventsSel[int(Event::Pixel::Type::BEAM_MONITOR)] = (value > 0);
    } else if (pasynUser->reason == SigEventsEn) {
        m_eventsSel[int(Event::Pixel::Type::SIGNAL)] = (value > 0);
    } else if (pasynUser->reason == AdcEventsEn) {
        m_eventsSel[int(Event::Pixel::Type::ADC)] = (value > 0);
    } else if (pasynUser->reason == ChopEventsEn) {
        m_eventsSel[int(Event::Pixel::Type::CHOPPER)] = (value > 0);
    }
    return BaseSocketPlugin::writeInt32(pasynUser, value);
}

void AdaraPlugin::clientConnected()
{
    LOG_INFO("ADARA client connected");
    reset();
}

void AdaraPlugin::reset()
{
    m_packetSeq = 0;
    m_cachedRtdl.clear();
    epicsTimeGetCurrent(&m_lastDataTimestamp);

    setIntegerParam(CntDataPkts, 0);
    setIntegerParam(CntRtdlPkts, 0);
    setIntegerParam(CntPingPkts, 0);
    callParamCallbacks();
}

bool AdaraPlugin::sendHeartbeat()
{
    uint32_t outpacket[4];
    epicsTimeStamp now;
    epicsTimeGetCurrent(&now);

    outpacket[0] = 0;
    outpacket[1] = ADARA_PKT_TYPE_HEARTBEAT;
    outpacket[2] = now.secPastEpoch;
    outpacket[3] = now.nsec;

    // If sending fails, send() will automatically close the socket
    this->unlock();
    bool ret = send(outpacket, 4*sizeof(uint32_t));
    this->lock();

    if (ret) {
        addIntegerParam(CntPingPkts, 1);
        callParamCallbacks();
    }

    return ret;
}

bool AdaraPlugin::sendRtdl(epicsTimeStamp timestamp, const RtdlHeader &rtdl, const std::vector<RtdlPacket::RtdlFrame> &frames)
{
    uint32_t outpacket[34];
    memset(outpacket, 0, sizeof(outpacket));

    outpacket[0] = 30*sizeof(uint32_t);
    outpacket[1] = ADARA_PKT_TYPE_RTDL;

    // RtdlHeader in legacy system is in format that SMS expects
    memcpy(&outpacket[2], reinterpret_cast<const void *>(&rtdl), sizeof(RtdlHeader));

    uint8_t off = 8;
    for (auto frame = frames.begin(); frame != frames.end(); frame++) {
        if ((*frame).id == 4) {
            // SMS needs frame 4 at index 8
            outpacket[8] = (*frame).raw;
        } else if (++off < 34) {
            outpacket[off] = (*frame).raw;
        }
    }

    this->unlock();
    bool ret = send(outpacket, sizeof(outpacket));
    this->lock();
    return ret;
}

template <typename T>
bool AdaraPlugin::sendEvents(epicsTimeStamp &timestamp, bool mapped, const T *events, uint32_t nEvents)
{
    size_t len = 10 + (nEvents * sizeof(Event::Pixel)/sizeof(uint32_t));
    try {
        m_buffer.reserve(len);
    } catch (...) {
        LOG_ERROR("Failed to allocate send buffer of %zu bytes", len);
        return false;
    }

    uint32_t *outpacket = m_buffer.data();

    DataInfo info;
    for (auto it = m_cachedRtdl.begin(); it != m_cachedRtdl.end(); it++) {
        if (it->first == timestamp) {
            info = it->second;
            break;
        }
    }

    outpacket[0] = 24;
    outpacket[1] = (mapped ? ADARA_PKT_TYPE_MAPPED_EVENT : ADARA_PKT_TYPE_RAW_EVENT);
    outpacket[2] = timestamp.secPastEpoch;
    outpacket[3] = timestamp.nsec;
    outpacket[4] = info.sourceId;
    outpacket[5] = ((info.pulseSeq++ & 0x7FFF) << 16) | (m_packetSeq++);
    outpacket[6] = info.rtdl.charge;
    outpacket[7] = info.rtdl.general_info;
    outpacket[8] = 0; // TSYNC period
    outpacket[9] = 0; // TSYNC delay

    len = 10;
    for (uint32_t i = 0; i < nEvents; i++) {
        uint8_t type = (events[i].pixelid >> 28) & 0x7;
        if (m_eventsSel[type] == true) {
            outpacket[0] += sizeof(Event::Pixel);
            outpacket[10+2*i] = events[i].tof;
            outpacket[11+2*i] = events[i].pixelid;
            len += sizeof(Event::Pixel) / sizeof(uint32_t);
        }
    }

    this->unlock();
    bool ret = send(outpacket, len*sizeof(uint32_t));
    this->lock();

    return ret;
}

void AdaraPlugin::recvDownstream(const RtdlPacketList &packets)
{
    if (getBooleanParam(Enable) == false)
        return;
    uint32_t maxCacheLen = getIntegerParam(RtdlCacheSize);

    uint32_t sentPackets = 0;
    for (const auto &packet: packets) {
        DataInfo info;
        epicsTime timestamp(packet->getTimeStamp());
        auto frames = packet->getRtdlFrames();
        info.rtdl = packet->getRtdlHeader();

        bool sent = false;
        for (auto it = m_cachedRtdl.begin(); it != m_cachedRtdl.end(); it++) {
            if (it->first == timestamp) {
                sent = true;
                break;
            }
        }

        if (sent == false && connectClient() == true) {
            if (sendRtdl(timestamp, info.rtdl, frames)) {
                sentPackets++;
                m_cachedRtdl.emplace_front(std::make_pair(timestamp, info));
                while (m_cachedRtdl.size() > maxCacheLen) {
                    m_cachedRtdl.pop_back();
                }
            } else {
                LOG_WARN("Failed to send RTDL packet to ADARA socket");
            }
        }
    }

    addIntegerParam(CntRtdlPkts, sentPackets);
    callParamCallbacks();
}

void AdaraPlugin::recvDownstream(const DasDataPacketList &packets)
{
    // Don't even bother with packet inspection if there's noone interested
    if (connectClient() == false || getBooleanParam(Enable) == false)
        return;

    uint32_t sentPackets = 0;
    for (const auto &packet: packets) {
        uint32_t nEvents = packet->getNumEvents();
        epicsTimeStamp timestamp = packet->getTimeStamp();
        bool mapped = packet->getEventsMapped();

        bool sent = false;
        if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_PIXEL) {
            sent = sendEvents(timestamp, mapped, packet->getEvents<Event::Pixel>(), nEvents);
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_META) {
            sent = sendEvents(timestamp, mapped, packet->getEvents<Event::Pixel>(), nEvents);
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_TIME_CALIB) {
            // Timing event is guaranteed to be sent out with every acquisition frame
            sent = sendEvents(timestamp, mapped, (Event::Pixel*)nullptr, 0);
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_BNL_DIAG) {
            sent = sendEvents(timestamp, mapped, packet->getEvents<Event::BNL::Diag>(), nEvents);
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_CROC_DIAG) {
            sent = sendEvents(timestamp, mapped, packet->getEvents<Event::CROC::Diag>(), nEvents);
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_ACPC_DIAG) {
            sent = sendEvents(timestamp, mapped, packet->getEvents<Event::ACPC::Diag>(), nEvents);
        }

        sentPackets += (sent ? 1 : 0);
    }

    if (packets.size() > 0) {
        epicsTimeGetCurrent(&m_lastDataTimestamp);
        addIntegerParam(CntDataPkts, sentPackets);
        callParamCallbacksRatelimit();
    }
}
