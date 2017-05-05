/* AdaraPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AdaraPlugin.h"
#include "Log.h"

#include <algorithm>

EPICS_REGISTER_PLUGIN(AdaraPlugin, 4, "port name", string, "dispatcher port", string, "blocking callbacks", int, "Neutron source id", int);

#define NUM_ADARAPLUGIN_PARAMS      ((int)(&LAST_ADARAPLUGIN_PARAM - &FIRST_ADARAPLUGIN_PARAM + 1))

#define ADARA_PKT_TYPE_RAW_EVENT    0x00000000
#define ADARA_PKT_TYPE_RTDL         0x00000100
#define ADARA_PKT_TYPE_SOURCE_LIST  0x00000200
#define ADARA_PKT_TYPE_MAPPED_EVENT 0x00000300
#define ADARA_PKT_TYPE_HEARTBEAT    0x00400900

/**
 * A thread-safe class that returns unique number every time it's queried.
 *
 * SMS needs a unique ID for every data source. Each source id needs to be
 * unique accross all AdaraPlugin instances.
 */
class SourceListIndex {
    public:
        SourceListIndex() : m_index(0) {}
        uint32_t get() {
            m_mutex.lock();
            uint32_t index = m_index++;
            m_mutex.unlock();
            return index;
        }
    private:
        epicsMutex m_mutex;
        uint32_t m_index;
};

//!< Make a single instance accross all AdaraPlugin instances.
static SourceListIndex g_sourceList;

AdaraPlugin::AdaraPlugin(const char *portName, const char *dispatcherPortName, int blocking, int numDsps)
    : BaseSocketPlugin(portName, dispatcherPortName, blocking, NUM_ADARAPLUGIN_PARAMS)
    , m_nTransmitted(0)
    , m_nProcessed(0)
    , m_nReceived(0)
    , m_dataPktType(ADARA_PKT_TYPE_RAW_EVENT)
{
    m_lastDataTimestamp = { 0, 0 };

    createParam("PixelsMapped", asynParamInt32, &PixelsMapped, 0); // WRITE - Tells whether data packets are flagged as raw or mapped pixel data
    createParam("NeutronsEn",   asynParamInt32, &NeutronsEn, 0); // WRITE - Enable forwarding neutron events
    createParam("MetadataEn",   asynParamInt32, &MetadataEn, 0); // WRITE - Enable forwarding metadata events
    createParam("Reset",        asynParamInt32, &Reset, 0);      // WRITE - Reset internal logic
    callParamCallbacks();
}

AdaraPlugin::~AdaraPlugin()
{
}

asynStatus AdaraPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == PixelsMapped) {
        m_dataPktType = (value == 0 ? ADARA_PKT_TYPE_RAW_EVENT : ADARA_PKT_TYPE_MAPPED_EVENT);
    } else if (pasynUser->reason == Reset) {
        reset();
        return asynSuccess;
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
    for (auto it=m_sources.begin(); it!=m_sources.end(); it++) {
        // Reset the sequence, not the source id
        it->second.reset();
    }
    m_lastRtdlTimestamp.secPastEpoch = 0;
    m_lastRtdlTimestamp.nsec = 0;
}

void AdaraPlugin::sendHeartbeat()
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
    send(outpacket, 4*sizeof(uint32_t));
    this->lock();
}

bool AdaraPlugin::sendRtdl(const uint32_t data[32])
{
    uint32_t outpacket[2];
    bool ret;

    outpacket[0] = 30*sizeof(uint32_t);
    outpacket[1] = ADARA_PKT_TYPE_RTDL;

    // The RTDL packet contents is just what ADARA expects.
    // Prefix that with the length and type of packet.
    this->unlock();
    ret = send(outpacket, 2*sizeof(uint32_t));
    if (ret) {
        ret = send(data, 32*sizeof(uint32_t));
    }
    this->lock();
    return ret;
}

bool AdaraPlugin::sendEvents(SourceSequence *seq, const DasPacket::Event *events, uint32_t eventsCount, bool neutrons, bool endOfPulse)
{
    uint32_t outpacket[10];
    bool ret;
    uint32_t dataFlags = (neutrons ? 0x1 : 0x2);

    outpacket[0] = 24 + sizeof(DasPacket::Event)*eventsCount;
    outpacket[1] = m_dataPktType;
    outpacket[2] = seq->rtdl.timestamp_sec;
    outpacket[3] = seq->rtdl.timestamp_nsec;
    outpacket[4] = seq->sourceId;
    outpacket[5] = ((int)endOfPulse << 31) | ((seq->pulseSeq & 0x7FF) << 16) | ((seq->totalSeq++) & 0xFFFF);
    outpacket[6] = (dataFlags << 27) | seq->rtdl.charge;
    outpacket[7] = seq->rtdl.general_info;
    outpacket[8] = seq->rtdl.tsync_period;
    outpacket[9] = seq->rtdl.tsync_delay;

    this->unlock();
    ret = send(outpacket, 10*sizeof(uint32_t));
    if (eventsCount > 0 && ret) {
        ret = send(reinterpret_cast<const uint32_t*>(events), eventsCount*sizeof(DasPacket::Event));
    }
    this->lock();

    return ret;
}

AdaraPlugin::SourceSequence *AdaraPlugin::findSource(bool neutron, uint32_t id)
{
    uint64_t mapId = ((uint64_t)neutron << 32) | (uint64_t)id;
    SourceSequence *seq = 0;

    std::map<uint64_t, SourceSequence>::iterator it = m_sources.find(mapId);
    if (it == m_sources.end()) {
        m_sources[mapId].sourceId = g_sourceList.get();
        it = m_sources.find(mapId);
    }
    if (it != m_sources.end()) {
        seq = &it->second;
    }

    return seq;
}

void AdaraPlugin::processData(const DasPacketList * const packetList)
{
    int neutronsEn = 0, metadataEn = 0;

    getIntegerParam(NeutronsEn, &neutronsEn);
    getIntegerParam(MetadataEn, &metadataEn);

    m_nReceived += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        // Don't even bother with packet inspection if there's noone interested
        if (connectClient() == false)
            break;

        if (packet->isRtdl()) {
            const RtdlHeader *rtdl = packet->getRtdlHeader();
            epicsTimeStamp timestamp = { 0, 0 };
            if (rtdl != 0)
                timestamp = { rtdl->timestamp_sec, rtdl->timestamp_nsec };

            // Pass a single RTDL packet for a given timestamp. DSP in most cases
            // sends two almost identical RTDL packets which differ only in source id
            // and packet/data flag.
            // RTDL packets must be the same across multiple DSPs since they
            // originate from single source - accelerator.
            if (epicsTimeGreaterThan(&timestamp, &m_lastRtdlTimestamp)) {
                // The RTDL packet contents is just what ADARA expects.
                // Prefix that with the length and type of packet.
                if (packet->getPayloadLength() == 128) {
                    if (sendRtdl(packet->payload)) {
                        m_nTransmitted++;
                    }
                    m_lastRtdlTimestamp = timestamp;

                    // Disabling this allows the plugin to send heartbeat every time there's
                    // no data which seems to help SMS do the book-keeping.
                    //epicsTimeGetCurrent(&m_lastSentTimestamp);
                }
            }
            m_nProcessed++;

        } else if (packet->isData()) {
            const RtdlHeader *rtdl = packet->getRtdlHeader();
            if (rtdl != 0) {

                // Skip if disabled
                if (packet->isNeutronData()) {
                    if (neutronsEn == 0) continue;
                } else {
                    if (metadataEn == 0) continue;
                }

                SourceSequence *seq = findSource(packet->isNeutronData(), packet->getSourceAddress());
                if (seq == 0) {
                    continue;
                }

                epicsTimeStamp currentTs = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
                epicsTimeStamp prevTs;
                prevTs.secPastEpoch = seq->rtdl.timestamp_sec;
                prevTs.nsec         = seq->rtdl.timestamp_nsec;

                // When transition to new pulse is detected, inject EOP packet for previous pulse
                if (epicsTimeEqual(&currentTs, &prevTs) == 0) {
                    if (prevTs.secPastEpoch > 0 && prevTs.nsec > 0 && sendEvents(seq, 0, 0, packet->isNeutronData(), true)) {
                        epicsTimeGetCurrent(&m_lastDataTimestamp);
                        m_nTransmitted++;
                    }
                    seq->pulseSeq = 0;
                    seq->rtdl = *rtdl; // Cache current packet RTDL for the next injected packet
                }

                uint32_t eventsCount;
                const DasPacket::Event *events = reinterpret_cast<const DasPacket::Event *>(packet->getData(&eventsCount));
                eventsCount /= (sizeof(DasPacket::Event) / sizeof(uint32_t));
                if (sendEvents(seq, events, eventsCount, packet->isNeutronData())) {
                    epicsTimeGetCurrent(&m_lastDataTimestamp);
                    m_nTransmitted++;
                }
                m_nProcessed++;
            }
        }
    }

    // Update parameters
    setIntegerParam(TxCount,        m_nTransmitted);
    setIntegerParam(ProcCount,      m_nProcessed);
    setIntegerParam(RxCount,        m_nReceived);
    callParamCallbacks();
}

float AdaraPlugin::checkClient()
{
    int heartbeatInt;
    epicsTimeStamp now;

    getIntegerParam(CheckInt, &heartbeatInt);
    epicsTimeGetCurrent(&now);
    double inactive = epicsTimeDiffInSeconds(&now, &m_lastDataTimestamp);

    if (isClientConnected() && inactive > heartbeatInt) {
        sendHeartbeat();
    }
    return BaseSocketPlugin::checkClient();
}
