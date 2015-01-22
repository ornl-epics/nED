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

AdaraPlugin::AdaraPlugin(const char *portName, const char *dispatcherPortName, int blocking, int numDsps)
    : BaseSocketPlugin(portName, dispatcherPortName, blocking, NUM_ADARAPLUGIN_PARAMS)
    , m_nTransmitted(0)
    , m_nProcessed(0)
    , m_nReceived(0)
    , m_nUnexpectedDspDrops(0)
    , m_nPacketsPrevPulse(0)
    , m_heartbeatActive(true)
    , m_dataPktType(ADARA_PKT_TYPE_RAW_EVENT)
{
    m_lastSentTimestamp = { 0, 0 };

    if (numDsps < 0) {
        numDsps = 1;
        LOG_WARN("Raising number of DSPs to 1");
    } else if (numDsps > ADARA_MAX_NUM_DSPS) {
        numDsps = ADARA_MAX_NUM_DSPS;
        LOG_WARN("Maximum number of DSPs is %d", ADARA_MAX_NUM_DSPS);
    }

    // Partially populate source mapping, only assign SMS source IDs, DSP ids
    // will be assigned when some data is received. But we need to have source
    // IDs as early as SMS connects to announce them.
    for (int i = 0; i < numDsps; i++) {
        m_dspSources.push_back(DspSource(0x0, i*2, i*2+1));
    }

    createParam("BadPulsCnt",   asynParamInt32, &BadPulsCnt, 0); // READ - Num dropped packets associated to already completed pulse
    createParam("BadDspCnt",    asynParamInt32, &BadDspCnt,  0); // READ - Num dropped packets from unexpected DSP
    createParam("PixelsMapped", asynParamInt32, &PixelsMapped, 0); // WRITE - Tells whether data packets are flagged as raw or mapped pixel data
    callParamCallbacks();
}

AdaraPlugin::~AdaraPlugin()
{
}

asynStatus AdaraPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == PixelsMapped) {
        m_dataPktType = (value == 0 ? ADARA_PKT_TYPE_RAW_EVENT : ADARA_PKT_TYPE_MAPPED_EVENT);
    }
    return BaseSocketPlugin::writeInt32(pasynUser, value);
}

void AdaraPlugin::processData(const DasPacketList * const packetList)
{
    uint32_t outpacket[10];

    if (getDataMode() != DATA_MODE_NORMAL)
        return;

    m_nReceived += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        // Don't even bother with packet inspection if there's noone interested
        if (connectClient() == false)
            break;

        if (packet->isRtdl()) {
            const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
            epicsTimeStamp timestamp = { 0, 0 };
            if (rtdl != 0)
                timestamp = { rtdl->timestamp_sec, rtdl->timestamp_nsec };

            // Pass a single RTDL packet for a given timestamp. DSP in most cases
            // sends two almost identical RTDL packets which differ only in source id
            // and packet/data flag.
            if (epicsTimeEqual(&timestamp, &m_lastRtdlTimestamp) == 0) { // time not equal
                outpacket[0] = 30*sizeof(uint32_t);
                outpacket[1] = ADARA_PKT_TYPE_RTDL;

                // The RTDL packet contents is just what ADARA expects.
                // Prefix that with the length and type of packet.
                m_heartbeatActive = false;
                if (send(outpacket, sizeof(uint32_t)*2) &&
                    send(packet->payload, sizeof(uint32_t)*std::min(packet->getPayloadLength(), 32U))) {
                    m_nTransmitted++;
                    epicsTimeGetCurrent(&m_lastSentTimestamp);
                    m_lastRtdlTimestamp = timestamp;
                }
                m_heartbeatActive = true;
            }
            m_nProcessed++;

        } else if (packet->isData()) {
            const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
            if (rtdl != 0) {
                uint32_t eventsCount;
                const DasPacket::Event *events = reinterpret_cast<const DasPacket::Event *>(packet->getData(&eventsCount));
                eventsCount /= (sizeof(DasPacket::Event) / sizeof(uint32_t));
                epicsTimeStamp currentTs = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
                epicsTimeStamp prevTs;
                SourceSequence *seq = findSourceSequence(packet->source, packet->isNeutronData());

                // Account and drop packets from unexpected DSP
                if (!seq) {
                    m_nUnexpectedDspDrops++;
                    LOG_ERROR("Unexpected number of DSPs detected, dropping packet");
                    continue;
                }

                prevTs.secPastEpoch = seq->rtdl.timestamp_sec;
                prevTs.nsec         = seq->rtdl.timestamp_nsec;

                // Account and drop packets from already completed pulses - should always be 0
                if (epicsTimeGreaterThan(&prevTs, &currentTs) != 0) {
                    m_nPacketsPrevPulse++;
                    LOG_ERROR("Received a packet associated to an already completed pulse, dropping packet");
                    continue;
                }

                // When transition to new pulse is detected, inject EOP packet for previous pulse
                if (epicsTimeEqual(&currentTs, &prevTs) == 0) {
                    if (prevTs.secPastEpoch > 0 && prevTs.nsec > 0) {
                        outpacket[0] = 24;
                        outpacket[1] = m_dataPktType;
                        outpacket[2] = seq->rtdl.timestamp_sec;
                        outpacket[3] = seq->rtdl.timestamp_nsec;
                        outpacket[4] = seq->sourceId;
                        outpacket[5] = (1 << 31) | ((seq->pulseSeq & 0x7FF) << 16) | ((seq->totalSeq++) & 0xFFFF);
                        outpacket[6] = seq->rtdl.charge;
                        outpacket[7] = seq->rtdl.general_info;
                        outpacket[8] = seq->rtdl.tsync_width;
                        outpacket[9] = seq->rtdl.tsync_delay;

                        m_heartbeatActive = false;
                        (void)send(outpacket, sizeof(uint32_t)*10);
                        m_heartbeatActive = true;
                    }
                    seq->pulseSeq = 0;
                    seq->rtdl = *rtdl; // Cache current packet RTDL for the next injected packet
                }

                outpacket[0] = 24 + sizeof(DasPacket::Event)*eventsCount;
                outpacket[1] = m_dataPktType;
                outpacket[2] = rtdl->timestamp_sec;
                outpacket[3] = rtdl->timestamp_nsec;
                outpacket[4] = seq->sourceId;
                outpacket[5] = ((seq->pulseSeq++ & 0x7FF) << 16) + (seq->totalSeq++ & 0xFFFF);
                outpacket[6] = rtdl->charge;
                outpacket[7] = rtdl->general_info;
                outpacket[8] = rtdl->tsync_width;
                outpacket[9] = rtdl->tsync_delay;

                m_heartbeatActive = false;
                if (send(outpacket, sizeof(uint32_t)*10) &&
                    send(reinterpret_cast<const uint32_t*>(events), sizeof(DasPacket::Event)*eventsCount)) {
                    m_nTransmitted++;
                    epicsTimeGetCurrent(&m_lastSentTimestamp);
                }
                m_heartbeatActive = true;
                m_nProcessed++;
            }
        }
    }

    // Update parameters
    setIntegerParam(TxCount,        m_nTransmitted);
    setIntegerParam(ProcCount,      m_nProcessed);
    setIntegerParam(RxCount,        m_nReceived);
    setIntegerParam(BadPulsCnt,     m_nPacketsPrevPulse);
    setIntegerParam(BadDspCnt,      m_nUnexpectedDspDrops);
    callParamCallbacks();
}

float AdaraPlugin::checkClient()
{
    int heartbeatInt;
    epicsTimeStamp now;

    getIntegerParam(CheckInt, &heartbeatInt);
    epicsTimeGetCurrent(&now);
    double inactive = epicsTimeDiffInSeconds(&now, &m_lastSentTimestamp);

    if (m_heartbeatActive && isClientConnected() && inactive > heartbeatInt) {
        uint32_t outpacket[4];

        outpacket[0] = 0;
        outpacket[1] = ADARA_PKT_TYPE_HEARTBEAT;
        outpacket[2] = now.secPastEpoch;
        outpacket[3] = now.nsec;

        // If sending fails, send() will automatically close the socket
        (void)send(outpacket, sizeof(outpacket));
    }
    return BaseSocketPlugin::checkClient();
}

void AdaraPlugin::clientConnected()
{
    uint32_t outpacket[4 + 2*ADARA_MAX_NUM_DSPS];
    epicsTimeStamp now;
    epicsTimeGetCurrent(&now);
    int idx = 0;

    outpacket[idx++] = sizeof(uint32_t) * m_dspSources.size() * 2;
    outpacket[idx++] = ADARA_PKT_TYPE_SOURCE_LIST;
    outpacket[idx++] = now.secPastEpoch;
    outpacket[idx++] = now.nsec;
    for (auto it = m_dspSources.begin(); it != m_dspSources.end(); it++) {
        outpacket[idx++] = it->neutronSeq.sourceId;
        outpacket[idx++] = it->metadataSeq.sourceId;
    }

// TODO: SMS can survive without the list, but it's sensitive about inactive sources
//       Temporarily disable sending list until more controlled way is introduced.
//       According to Jeeem, SMS han survive without it.
//    (void)send(outpacket, sizeof(uint32_t)*idx);
}

AdaraPlugin::SourceSequence* AdaraPlugin::findSourceSequence(uint32_t dspId, bool neutron)
{
    for (auto it = m_dspSources.begin(); it != m_dspSources.end(); it++) {
        if (it->dspId == 0x0) {
            it->dspId = dspId;
        }
        if (it->dspId == dspId) {
            return (neutron ? &it->neutronSeq : &it->metadataSeq);
        }
    }
    return 0;
}
