/* CommDebug.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseModulePlugin.h"
#include "CommDebug.h"
#include "Common.h"
#include "Log.h"

#include <algorithm>
#include <cstring>

EPICS_REGISTER_PLUGIN(CommDebug, 2, "Port name", string, "Parent plugins", string);

CommDebug::CommDebug(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, 0, asynOctetMask | asynFloat64Mask, asynOctetMask | asynFloat64Mask)
{
    m_packet = reinterpret_cast<DasCmdPacket *>(m_buffer);

    createParam("ReqVersion",   asynParamInt32, &ReqVersion, 1);    // WRITE - Packet version to be sent out
    createParam("ReqPriority",  asynParamInt32, &ReqPriority, 0);   // WRITE - Packet priority - usually set by modules only
    createParam("ReqType",      asynParamInt32, &ReqType, 8);       // WRITE - Packet type - only works with 8
    createParam("ReqSequence",  asynParamInt32, &ReqSequence, 0);   // WRITE - Packet sequence number
    createParam("ReqLength",    asynParamInt32, &ReqLength, 0);     // WRITE - Packet length in bytes
    createParam("ReqCmdLen",    asynParamInt32, &ReqCmdLen, 0);     // WRITE - Command payload length
    createParam("ReqCmd",       asynParamInt32, &ReqCmd, 0x0);      // WRITE - Command to be sent to module
    createParam("ReqVerifyId",  asynParamInt32, &ReqVerifyId, 0);   // WRITE - Command verification id
    createParam("ReqAck",       asynParamInt32, &ReqAck, 0);        // WRITE - Command was acknowledged flag
    createParam("ReqRsp",       asynParamInt32, &ReqRsp, 0);        // WRITE - Command is response flag
    createParam("ReqCmdVer",    asynParamInt32, &ReqCmdVer, 0);     // WRITE - Command packet version
    createParam("ReqModule",    asynParamOctet, &ReqModule, 0x0);   // WRITE - Module address to communicate with
    createParam("ReqRaw0",      asynParamInt32, &ReqRaw0);          // Request packet raw word 0
    createParam("ReqRaw1",      asynParamInt32, &ReqRaw1);          // Request packet raw word 1
    createParam("ReqRaw2",      asynParamInt32, &ReqRaw2);          // Request packet raw word 2
    createParam("ReqRaw3",      asynParamInt32, &ReqRaw3);          // Request packet raw word 3
    createParam("ReqRaw4",      asynParamInt32, &ReqRaw4);          // Request packet raw word 4
    createParam("ReqRaw5",      asynParamInt32, &ReqRaw5);          // Request packet raw word 5
    createParam("ReqRaw6",      asynParamInt32, &ReqRaw6);          // Request packet raw word 6
    createParam("ReqRaw7",      asynParamInt32, &ReqRaw7);          // Request packet raw word 7

    createParam("RspVersion",   asynParamInt32, &RspVersion, 1);    // WRITE - Packet version to be sent out
    createParam("RspPriority",  asynParamInt32, &RspPriority, 0);   // WRITE - Packet priority - usually set by modules only
    createParam("RspType",      asynParamInt32, &RspType, 8);       // WRITE - Packet type - only works with 8
    createParam("RspSequence",  asynParamInt32, &RspSequence, 0);   // WRITE - Packet sequence number
    createParam("RspLength",    asynParamInt32, &RspLength, 0);     // WRITE - Packet length in bytes
    createParam("RspRaw0",      asynParamInt32, &RspRaw0);          // Response packet raw word 0
    createParam("RspRaw1",      asynParamInt32, &RspRaw1);          // Response packet raw word 1
    createParam("RspRaw2",      asynParamInt32, &RspRaw2);          // Response packet raw word 2
    createParam("RspRaw3",      asynParamInt32, &RspRaw3);          // Response packet raw word 3
    createParam("RspRaw4",      asynParamInt32, &RspRaw4);          // Response packet raw word 4
    createParam("RspRaw5",      asynParamInt32, &RspRaw5);          // Response packet raw word 5
    createParam("RspRaw6",      asynParamInt32, &RspRaw6);          // Response packet raw word 6
    createParam("RspRaw7",      asynParamInt32, &RspRaw7);          // Response packet raw word 7
    createParam("RspTimeStamp", asynParamFloat64, &RspTimeStamp);   // Response time stamp in seconds with msec precision

    createParam("RspErrSourceId",asynParamInt32,&RspErrSourceId);   // READ - RTDL source id field
    createParam("RspErrCode",   asynParamInt32, &RspErrCode);       // READ - RTDL number of frames
    createParam("RspCmdLen",    asynParamInt32, &RspCmdLen);        // READ - Command payload length
    createParam("RspCmd",       asynParamInt32, &RspCmd);           // READ - Command to be sent to module
    createParam("RspCmdVerifyId", asynParamInt32, &RspCmdVerifyId); // READ - Command verification id
    createParam("RspCmdAck",    asynParamInt32, &RspCmdAck);        // READ - Command was acknowledged flag
    createParam("RspCmdRsp",    asynParamInt32, &RspCmdRsp);        // READ - Command is response flag
    createParam("RspCmdVer",    asynParamInt32, &RspCmdVer);        // READ - Command packet version
    createParam("RspCmdModule", asynParamOctet, &RspCmdModule);     // READ - Module address to communicate with
    createParam("RspRtdlSourceId",asynParamInt32, &RspRtdlSourceId);// READ - RTDL source id field
    createParam("RspRtdlFrames",asynParamInt32, &RspRtdlFrames);    // READ - RTDL number of frames
    createParam("RspRtdlTime",  asynParamOctet, &RspRtdlTime);      // READ - RTDL time, seconds converter to time string in ETC
    createParam("RspRtdlTimeNsec",asynParamInt32, &RspRtdlTimeNsec);// READ - RTDL time, nano-seconds
    createParam("RspRtdlPcharge",asynParamInt32,&RspRtdlPcharge);   // READ - RTDL pulse proton charge
    createParam("RspRtdlFlavor",asynParamInt32, &RspRtdlFlavor);    // READ - RTDL pulse flavors
    createParam("RspRtdlBad",   asynParamInt32, &RspRtdlBad);       // READ - RTDL bad pulse flag
    createParam("RspRtdlCycle", asynParamInt32, &RspRtdlCycle);     // READ - RTDL pulse cycle number (0-599)
    createParam("RspRtdlVeto",  asynParamInt32, &RspRtdlVeto);      // READ - RTDL last pulse veto
    createParam("RspRtdlTstat", asynParamInt32, &RspRtdlTstat);     // READ - RTDL tstat
    createParam("RspRtdlBadFr", asynParamInt32, &RspRtdlBadFr);     // READ - RTDL bad cycle frame
    createParam("RspRtdlBadVeto",asynParamInt32,&RspRtdlBadVeto);   // READ - RTDL bad last cycle veto frame
    createParam("RspRtdlTsync", asynParamInt32, &RspRtdlTsync);     // READ - RTDL applied TSYNC
    createParam("RspRtdlTofOff",asynParamInt32, &RspRtdlTofOff);    // READ - RTDL applied TOF offset
    createParam("RspRtdlFrOff", asynParamInt32, &RspRtdlFrOff);     // READ - RTDL applied frame offset
    createParam("RspRtdlOffEn", asynParamInt32, &RspRtdlOffEn);     // READ - RTDL TOF enabled flag

    createParam("ReqSend",      asynParamInt32, &ReqSend);          // WRITE - Send cached packet
    createParam("SendQueIndex", asynParamInt32, &SendQueIndex, 0);  // Currently selected sent packet
    createParam("SendQueSize",  asynParamInt32, &SendQueSize, 0);   // Number of elements in sent send buffer
    createParam("SendQueMaxSize",asynParamInt32,&SendQueMaxSize, 50);// Max num of elements in send buffer
    createParam("RecvQueIndex", asynParamInt32, &RecvQueIndex, 0);  // Currently selected receive packet
    createParam("RecvQueSize",  asynParamInt32, &RecvQueSize, 0);   // Number of elements in receive buffer
    createParam("RecvQueMaxSize",asynParamInt32,&RecvQueMaxSize, 50);// Max num of elements in receive buffer

    createParam("Sniffer",      asynParamInt32, &Sniffer);          // WRITE - Enables listening to other plugins messages
    createParam("FilterPktType",asynParamInt32, &FilterPktType);    // WRITE - Enables filtering based on packet type
    createParam("FilterCmd",    asynParamInt32, &FilterCmd);        // WRITE - Enables filtering based on command
    createParam("FilterModule", asynParamOctet, &FilterModule);     // WRITE - Enables filtering based on hardware id
    createParam("ResetQues",    asynParamInt32, &ResetQues);        // WRITE - Clears packet FIFOs

    callParamCallbacks();

    std::list<int> types = { MsgDasCmd, MsgDasRtdl, MsgError };
    BasePlugin::connect(parentPlugins, types);

    generatePacket(false);
    memset(&m_emptyPacket, 0, sizeof(DasCmdPacket));
}

asynStatus CommDebug::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    std::list<int> raws = { ReqRaw0, ReqRaw1, ReqRaw2, ReqRaw3, ReqRaw4, ReqRaw5, ReqRaw6, ReqRaw7 };
    std::list<int> reqPvs = { ReqVersion, ReqPriority, ReqType, ReqSequence, ReqLength, ReqCmdLen, ReqCmd, ReqVerifyId, ReqAck, ReqRsp, ReqCmdVer, ReqModule };
    bool fromRaw = false;
    bool regenerate = false;

    if (pasynUser->reason == ReqSend) {
        m_recvQue.clear();
        m_sendQue.clear();
        generatePacket(false);
        BasePlugin::sendUpstream(m_packet);
        showSentPacket(m_packet);
        showRecvPacket(&m_emptyPacket);
        m_liveTimeout = epicsTime::getCurrent() + 0.5; // Allow .5 seconds for responses to arrive
        return asynSuccess;
    } else if (pasynUser->reason == ResetQues) {
        m_recvQue.clear();
        m_sendQue.clear();
        showSentPacket(&m_emptyPacket);
        showRecvPacket(&m_emptyPacket);
        return asynSuccess;
    } else if (pasynUser->reason == SendQueIndex) {
        showSentPacket(value);
        return asynSuccess;
    } else if (pasynUser->reason == RecvQueIndex) {
        showRecvPacket(value);
        return asynSuccess;
    } else if (std::find(raws.begin(), raws.end(), pasynUser->reason) != raws.end()) {
        fromRaw = true;
        regenerate = true;
    } else if (std::find(reqPvs.begin(), reqPvs.end(), pasynUser->reason) != reqPvs.end()) {
        regenerate = true;
    }

    asynStatus ret = BasePlugin::writeInt32(pasynUser, value);
    if (regenerate) {
        generatePacket(fromRaw);
        showSentPacket(m_packet);
    }
    return ret;
}

asynStatus CommDebug::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == ReqModule) {
        setStringParam(ReqModule, value);
        generatePacket(false);
        showSentPacket(m_packet);
        *nActual = nChars;
        return asynSuccess;
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);;
}

void CommDebug::generatePacket(bool fromRawPvs)
{
    if (fromRawPvs) {
        int param;

        getIntegerParam(ReqRaw0, &param);   m_buffer[0] = param;
        getIntegerParam(ReqRaw1, &param);   m_buffer[1] = param;
        getIntegerParam(ReqRaw2, &param);   m_buffer[2] = param;
        getIntegerParam(ReqRaw3, &param);   m_buffer[3] = param;
        getIntegerParam(ReqRaw4, &param);   m_buffer[4] = param;
        getIntegerParam(ReqRaw5, &param);   m_buffer[5] = param;
        getIntegerParam(ReqRaw6, &param);   m_buffer[6] = param;
        getIntegerParam(ReqRaw7, &param);   m_buffer[7] = param;

    } else {
        int cmd;
        char moduleId[20];
        bool ack;
        bool rsp;
        int verifyId;
        uint32_t payload[0];
        int payloadLen=0;
        getIntegerParam(ReqCmd,         &cmd);
        getIntegerParam(ReqVerifyId,    &verifyId);
        getIntegerParam(ReqCmdLen,      &payloadLen);
        getBooleanParam(ReqAck,         ack);
        getBooleanParam(ReqRsp,         rsp);
        payloadLen -= DasCmdPacket::getHeaderLen();
        payloadLen = std::max(0, std::min(16, payloadLen));

        if (payloadLen >=  4) { int param; getIntegerParam(ReqRaw4, &param); payload[0] = param; }
        if (payloadLen >=  8) { int param; getIntegerParam(ReqRaw5, &param); payload[1] = param; }
        if (payloadLen >= 12) { int param; getIntegerParam(ReqRaw6, &param); payload[2] = param; }
        if (payloadLen >= 16) { int param; getIntegerParam(ReqRaw7, &param); payload[3] = param; }

        getStringParam(ReqModule,   sizeof(moduleId), moduleId);
        int module = BaseModulePlugin::ip2addr(std::string(moduleId, sizeof(moduleId)));
        DasCmdPacket::CommandType command = static_cast<const DasCmdPacket::CommandType>(cmd & 0xFF);
        m_packet->init(module, command, ack, rsp, verifyId, payload, payloadLen);

        int param;
        getIntegerParam(ReqVersion, &param);    m_packet->version = param;
        getIntegerParam(ReqPriority, &param);   m_packet->priority = (param > 0);
        getIntegerParam(ReqSequence, &param);   m_packet->sequence = param;
        getIntegerParam(ReqLength, &param);     m_packet->length = std::min(64, std::max(0, param));
    }
}

void CommDebug::recvDownstream(DasCmdPacketList *packets)
{
    int filterType = getIntegerParam(FilterPktType);
    int filterCmd = getIntegerParam(FilterCmd);
    int recvQueMaxSize = getIntegerParam(RecvQueMaxSize);
    uint32_t moduleId = BaseModulePlugin::ip2addr( getStringParam(FilterModule) );
    bool sniffer = getBooleanParam(Sniffer);

    this->unlock();
    sendDownstream(packets, false);
    this->lock();

    if (!sniffer) {
        epicsTime now = epicsTime::getCurrent();
        if (now > m_liveTimeout)
            return;
        filterType = Packet::TYPE_DAS_CMD;
    }

    if (filterType == 0 || filterType == Packet::TYPE_DAS_CMD) {
        for (auto it = packets->cbegin(); it != packets->cend(); it++) {
            const DasCmdPacket *packet = *it;

            if (sniffer) {
                if ((moduleId == 0 || moduleId == packet->module_id) &&
                    (filterCmd == 0 || filterCmd == packet->command)) {

                    savePacket(packet, m_recvQue, recvQueMaxSize);
                }
            } else {
                savePacket(packet, m_recvQue, recvQueMaxSize);
            }
        }
        if (!packets->empty())
            showRecvPacket(0);
    }
}

void CommDebug::recvDownstream(DasRtdlPacketList *packets)
{
    int filterType = getIntegerParam(FilterPktType);
    int recvQueMaxSize = getIntegerParam(RecvQueMaxSize);
    bool sniffer = getBooleanParam(Sniffer);

    this->unlock();
    sendDownstream(packets, false);
    this->lock();

    if (sniffer && (filterType == 0 || filterType == Packet::TYPE_DAS_RTDL)) {
        for (auto it = packets->cbegin(); it != packets->cend(); it++) {
            savePacket(*it, m_recvQue, recvQueMaxSize);
        }
        if (!packets->empty())
            showRecvPacket(0);
    }
}

void CommDebug::recvDownstream(ErrorPacketList *packets)
{
    int filterType = getIntegerParam(FilterPktType);
    int recvQueMaxSize = getIntegerParam(RecvQueMaxSize);
    bool sniffer = getBooleanParam(Sniffer);

    this->unlock();
    sendDownstream(packets, false);
    this->lock();

    if (sniffer && (filterType == 0 || filterType == Packet::TYPE_ERROR)) {
        for (auto it = packets->cbegin(); it != packets->cend(); it++) {
            savePacket(*it, m_recvQue, recvQueMaxSize);
        }
        if (!packets->empty())
            showRecvPacket(0);
    }
}

void CommDebug::recvUpstream(DasCmdPacketList *packets)
{
    bool sniffer = getBooleanParam(Sniffer);
    sendUpstream(packets);

    if (sniffer && !packets->empty()) {
        int filterCmd;
        char moduleStr[20];
        uint32_t moduleId;

        getIntegerParam(FilterCmd, &filterCmd);
        getStringParam(FilterModule,   sizeof(moduleStr), moduleStr);
        moduleId = BaseModulePlugin::ip2addr(moduleStr);

        for (auto it = packets->begin(); it != packets->end(); it++) {
            DasCmdPacket *packet = *it;

            if ((moduleId == 0 || moduleId == packet->module_id) &&
                (filterCmd == 0 || filterCmd == packet->command)) {

                savePacket(packet, m_sendQue, getIntegerParam(SendQueMaxSize));
            }
        }
        showSentPacket(0);
    }
}

void CommDebug::savePacket(const Packet *packet, std::list<PacketDesc> &que, int maxQueSize)
{
    if (maxQueSize < 1)
        maxQueSize = 1;

    // Cache the payload to read it through readOctet()
    PacketDesc pkt;
    pkt.length = std::min(packet->length, static_cast<uint32_t>(sizeof(pkt.data)));
    memcpy(pkt.data, packet, pkt.length);
    while ((int)que.size() >= maxQueSize)
        que.pop_back();
    que.push_front(pkt);
}

void CommDebug::showSentPacket(int index)
{
    if (!m_sendQue.empty()) {
        if (index < 0)
            index = 0;
        if ((size_t)index >= m_sendQue.size())
            index = m_sendQue.size() - 1;

        auto it = m_sendQue.begin();
        for (int i = 0; i < index; i++) {
            it++;
        }

        showSentPacket( reinterpret_cast<DasCmdPacket*>(it->data), index );
    }
}

void CommDebug::showSentPacket(DasCmdPacket *packet, int index)
{
    uint32_t *raw = reinterpret_cast<uint32_t *>(packet);

    setIntegerParam(ReqVersion,   packet->version);
    setIntegerParam(ReqPriority,  packet->priority);
    setIntegerParam(ReqType,      packet->type);
    setIntegerParam(ReqSequence,  packet->sequence);
    setIntegerParam(ReqLength,    packet->length);
    setIntegerParam(ReqCmdLen,    packet->cmd_length);
    setIntegerParam(ReqCmd,       packet->command);
    setIntegerParam(ReqVerifyId,  packet->cmd_sequence);
    setIntegerParam(ReqAck,       packet->acknowledge);
    setIntegerParam(ReqRsp,       packet->response);
    setIntegerParam(ReqCmdVer,    packet->lvds_version);
    setStringParam(ReqModule,     BaseModulePlugin::addr2ip(packet->module_id));

    if (packet->length >=  0) setIntegerParam(ReqRaw0, raw[0]);
    if (packet->length >=  4) setIntegerParam(ReqRaw1, raw[1]);
    if (packet->length >=  8) setIntegerParam(ReqRaw2, raw[2]);
    if (packet->length >= 12) setIntegerParam(ReqRaw3, raw[3]);
    if (packet->length >= 16) setIntegerParam(ReqRaw4, raw[4]);
    if (packet->length >= 20) setIntegerParam(ReqRaw5, raw[5]);
    if (packet->length >= 24) setIntegerParam(ReqRaw6, raw[6]);
    if (packet->length >= 28) setIntegerParam(ReqRaw7, raw[7]);

    setIntegerParam(SendQueIndex, index);
    setIntegerParam(SendQueSize, m_sendQue.size());
    callParamCallbacks();
}

void CommDebug::showRecvPacket(int index)
{
    if (!m_recvQue.empty()) {
        if (index < 0)
            index = 0;
        if ((size_t)index >= m_recvQue.size())
            index = m_recvQue.size() - 1;

        auto it = m_recvQue.begin();
        for (int i = 0; i < index; i++) {
            it++;
        }

        showRecvPacket( reinterpret_cast<Packet *>(it->data), index );
    }
}

void CommDebug::showRecvPacket(Packet *packet, int index)
{
    uint32_t *raw = reinterpret_cast<uint32_t *>(packet);

    setIntegerParam(RspVersion,     packet->version);
    setIntegerParam(RspPriority,    packet->priority);
    setIntegerParam(RspType,        packet->type);
    setIntegerParam(RspSequence,    packet->sequence);
    setIntegerParam(RspLength,      packet->length);

    if (packet->version != 0) {
        if (packet->type == Packet::TYPE_DAS_CMD) {
            DasCmdPacket *cmdPacket = reinterpret_cast<DasCmdPacket *>(packet);
            setIntegerParam(RspCmdLen,      cmdPacket->cmd_length);
            setIntegerParam(RspCmd,         cmdPacket->command);
            setIntegerParam(RspCmdVerifyId, cmdPacket->cmd_sequence);
            setIntegerParam(RspCmdAck,      cmdPacket->acknowledge);
            setIntegerParam(RspCmdRsp,      cmdPacket->response);
            setIntegerParam(RspCmdVer,      cmdPacket->lvds_version);
            setStringParam(RspCmdModule,    BaseModulePlugin::addr2ip(cmdPacket->module_id));

        } else if (packet->type == Packet::TYPE_ERROR) {
            ErrorPacket *errPacket = reinterpret_cast<ErrorPacket *>(packet);
            setIntegerParam(RspErrCode,     errPacket->code);
            setIntegerParam(RspErrSourceId, errPacket->source);

        } else if (packet->type == Packet::TYPE_DAS_RTDL) {
            DasRtdlPacket *rtdlPacket = reinterpret_cast<DasRtdlPacket *>(packet);
            epicsTimeStamp ts = { rtdlPacket->timestamp_sec, rtdlPacket->timestamp_nsec };
            epicsTime t(ts);
            char timeStr[64], nsecStr[16];
            t.strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S.");
            snprintf(nsecStr, sizeof(nsecStr), "%09u", rtdlPacket->timestamp_nsec);
            strncat(timeStr, nsecStr, sizeof(timeStr));

            setIntegerParam(RspRtdlSourceId,rtdlPacket->source);
            setIntegerParam(RspRtdlFrames,  rtdlPacket->num_frames);
            setStringParam (RspRtdlTime,    timeStr);
            setIntegerParam(RspRtdlTimeNsec,rtdlPacket->timestamp_nsec);
            setIntegerParam(RspRtdlPcharge, rtdlPacket->pulse.charge);
            setIntegerParam(RspRtdlFlavor,  rtdlPacket->pulse.flavor);
            setIntegerParam(RspRtdlBad,     rtdlPacket->pulse.bad);
            setIntegerParam(RspRtdlCycle,   rtdlPacket->pulse.cycle);
            setIntegerParam(RspRtdlVeto,    rtdlPacket->pulse.last_cycle_veto);
            setIntegerParam(RspRtdlTstat,   rtdlPacket->pulse.tstat);
            setIntegerParam(RspRtdlBadFr,   rtdlPacket->pulse.bad_cycle_frame);
            setIntegerParam(RspRtdlBadVeto, rtdlPacket->pulse.bad_veto_frame);
            setIntegerParam(RspRtdlTsync,   rtdlPacket->correction.tsync_period * 100);
            setIntegerParam(RspRtdlTofOff,  rtdlPacket->correction.tof_fixed_offset * 100);
            setIntegerParam(RspRtdlFrOff,   rtdlPacket->correction.frame_offset);
            setIntegerParam(RspRtdlOffEn,   rtdlPacket->correction.tof_full_offset);
        }
    }

    if (packet->length >=  0) setIntegerParam(RspRaw0, raw[0]);
    if (packet->length >=  4) setIntegerParam(RspRaw1, raw[1]);
    if (packet->length >=  8) setIntegerParam(RspRaw2, raw[2]);
    if (packet->length >= 12) setIntegerParam(RspRaw3, raw[3]);
    if (packet->length >= 16) setIntegerParam(RspRaw4, raw[4]);
    if (packet->length >= 20) setIntegerParam(RspRaw5, raw[5]);
    if (packet->length >= 24) setIntegerParam(RspRaw6, raw[6]);
    if (packet->length >= 28) setIntegerParam(RspRaw7, raw[7]);

    setIntegerParam(RecvQueIndex, index);
    setIntegerParam(RecvQueSize, m_recvQue.size());
    callParamCallbacks();
}
