/* CommDebugPlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseModulePlugin.h"
#include "CommDebugPlugin.h"
#include "Common.h"
#include "Log.h"

#include <algorithm>
#include <cstring>

EPICS_REGISTER_PLUGIN(CommDebugPlugin, 2, "Port name", string, "Parent plugins", string);

CommDebugPlugin::CommDebugPlugin(const char *portName, const char *parentPlugins)
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
    createParam("RspRtdlFrames",asynParamInt32, &RspRtdlFrames);    // READ - RTDL number of frames
    createParam("RspDataSourceId",asynParamInt32, &RspDataSourceId);// READ - Data source id field
    createParam("RspDataFormat",asynParamInt32, &RspDataFormat);    // READ - Data format
    createParam("RspDataTime",  asynParamOctet, &RspDataTime);      // READ - Data time, seconds converter to time string in ETC
    createParam("RspDataTimeNsec",asynParamInt32, &RspDataTimeNsec);// READ - Data time, nano-seconds

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

    BasePlugin::connect(parentPlugins, { MsgDasCmd, MsgDasRtdl, MsgDasData, MsgError });

    generatePacket(false);
}

asynStatus CommDebugPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
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
        showRecvPacket(reinterpret_cast<const Packet *>(m_zeroes));
        m_liveTimeout = epicsTime::getCurrent() + 0.5; // Allow .5 seconds for responses to arrive
        return asynSuccess;
    } else if (pasynUser->reason == ResetQues) {
        m_recvQue.clear();
        m_sendQue.clear();
        showRecvPacket(reinterpret_cast<const Packet *>(m_zeroes));
        showRecvPacket(reinterpret_cast<const Packet *>(m_zeroes));
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

asynStatus CommDebugPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
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

void CommDebugPlugin::generatePacket(bool fromRawPvs)
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
        int bufferLen = sizeof(m_buffer) - sizeof(DasCmdPacket);
        int payloadLen = getIntegerParam(ReqCmdLen)-6;
        payloadLen = std::max(0, std::min(bufferLen, payloadLen));

        if (payloadLen >=  4) { int param; getIntegerParam(ReqRaw4, &param); m_buffer[0] = param; }
        if (payloadLen >=  8) { int param; getIntegerParam(ReqRaw5, &param); m_buffer[1] = param; }
        if (payloadLen >= 12) { int param; getIntegerParam(ReqRaw6, &param); m_buffer[2] = param; }
        if (payloadLen >= 16) { int param; getIntegerParam(ReqRaw7, &param); m_buffer[3] = param; }

        DasCmdPacket::init(reinterpret_cast<uint8_t *>(m_buffer),
                           sizeof(m_buffer),
                           BaseModulePlugin::ip2addr(getStringParam(ReqModule)),
                           static_cast<const DasCmdPacket::CommandType>(getIntegerParam(ReqCmd) & 0xFF),
                           getBooleanParam(ReqVersion),
                           getBooleanParam(ReqAck),
                           getBooleanParam(ReqRsp),
                           getIntegerParam(ReqVerifyId) & 0x1FF,
                           payloadLen,
                           m_buffer);

        m_packet->setPriority(getBooleanParam(ReqPriority));
        m_packet->setVersion(getIntegerParam(ReqVersion) & 0xF);
        m_packet->setSequenceId(getIntegerParam(ReqSequence) & 0xFF);
    }
}

void CommDebugPlugin::recvDownstream(const DasCmdPacketList &packets)
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
        for (auto it = packets.begin(); it != packets.cend(); it++) {
            const DasCmdPacket *packet = *it;

            if (sniffer) {
                if ((moduleId == 0 || moduleId == packet->getModuleId()) &&
                    (filterCmd == 0 || filterCmd == packet->getCommand())) {

                    savePacket(packet, m_recvQue, recvQueMaxSize);
                }
            } else {
                savePacket(packet, m_recvQue, recvQueMaxSize);
            }
        }
        if (!packets.empty())
            showRecvPacket(0);
    }
}

void CommDebugPlugin::recvDownstream(const DasDataPacketList &packets)
{
    int filterType = getIntegerParam(FilterPktType);
    int recvQueMaxSize = getIntegerParam(RecvQueMaxSize);
    bool sniffer = getBooleanParam(Sniffer);

    this->unlock();
    sendDownstream(packets, false);
    this->lock();

    if (sniffer && (filterType == 0 || filterType == Packet::TYPE_DAS_DATA)) {
        for (auto it = packets.cbegin(); it != packets.cend(); it++) {
            savePacket(*it, m_recvQue, recvQueMaxSize);
        }
        if (!packets.empty())
            showRecvPacket(0);
    }
}

void CommDebugPlugin::recvDownstream(const RtdlPacketList &packets)
{
    int filterType = getIntegerParam(FilterPktType);
    int recvQueMaxSize = getIntegerParam(RecvQueMaxSize);
    bool sniffer = getBooleanParam(Sniffer);

    this->unlock();
    sendDownstream(packets, false);
    this->lock();

    if (sniffer && (filterType == 0 || filterType == Packet::TYPE_RTDL)) {
        for (auto it = packets.cbegin(); it != packets.cend(); it++) {
            savePacket(*it, m_recvQue, recvQueMaxSize);
        }
        if (!packets.empty())
            showRecvPacket(0);
    }
}

void CommDebugPlugin::recvDownstream(const ErrorPacketList &packets)
{
    int filterType = getIntegerParam(FilterPktType);
    int recvQueMaxSize = getIntegerParam(RecvQueMaxSize);
    bool sniffer = getBooleanParam(Sniffer);

    this->unlock();
    sendDownstream(packets, false);
    this->lock();

    if (sniffer && (filterType == 0 || filterType == Packet::TYPE_ERROR)) {
        for (auto it = packets.cbegin(); it != packets.cend(); it++) {
            savePacket(*it, m_recvQue, recvQueMaxSize);
        }
        if (!packets.empty())
            showRecvPacket(0);
    }
}

void CommDebugPlugin::recvUpstream(const DasCmdPacketList &packets)
{
    bool sniffer = getBooleanParam(Sniffer);
    sendUpstream(packets);

    if (sniffer && !packets.empty()) {
        int filterCmd;
        uint32_t moduleId;

        getIntegerParam(FilterCmd, &filterCmd);
        moduleId = BaseModulePlugin::ip2addr(getStringParam(FilterModule));

        for (auto it = packets.begin(); it != packets.end(); it++) {
            const DasCmdPacket *packet = *it;

            if ((moduleId == 0 || moduleId == packet->getModuleId()) &&
                (filterCmd == 0 || filterCmd == packet->getCommand())) {

                savePacket(packet, m_sendQue, getIntegerParam(SendQueMaxSize));
            }
        }
        showSentPacket(0);
    }
}

void CommDebugPlugin::savePacket(const Packet *packet, std::list<PacketDesc> &que, int maxQueSize)
{
    if (maxQueSize < 1)
        maxQueSize = 1;

    // Cache the payload to read it through readOctet()
    PacketDesc pkt;
    pkt.length = std::min(packet->getLength(), static_cast<uint32_t>(sizeof(pkt.data)));
    memcpy(pkt.data, packet, pkt.length);
    while ((int)que.size() >= maxQueSize)
        que.pop_back();
    que.push_front(pkt);
}

void CommDebugPlugin::showSentPacket(int index)
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

void CommDebugPlugin::showSentPacket(const DasCmdPacket *packet, int index)
{
    const uint32_t *raw = reinterpret_cast<const uint32_t *>(packet);

    setIntegerParam(ReqVersion,   packet->getVersion());
    setIntegerParam(ReqPriority,  packet->isPriority());
    setIntegerParam(ReqType,      packet->getType());
    setIntegerParam(ReqSequence,  packet->getSequenceId());
    setIntegerParam(ReqLength,    packet->getLength());
    setIntegerParam(ReqCmdLen,    packet->getCmdPayloadLength());
    setIntegerParam(ReqCmd,       packet->getCommand());
    setIntegerParam(ReqVerifyId,  packet->getCmdId());
    setIntegerParam(ReqAck,       packet->isAcknowledge());
    setIntegerParam(ReqRsp,       packet->isResponse());
    setIntegerParam(ReqCmdVer,    packet->getCmdVer());
    setStringParam(ReqModule,     packet->getModuleIdStr());

    if (packet->getLength() >=  0) setIntegerParam(ReqRaw0, raw[0]);
    if (packet->getLength() >=  4) setIntegerParam(ReqRaw1, raw[1]);
    if (packet->getLength() >=  8) setIntegerParam(ReqRaw2, raw[2]);
    if (packet->getLength() >= 12) setIntegerParam(ReqRaw3, raw[3]);
    if (packet->getLength() >= 16) setIntegerParam(ReqRaw4, raw[4]);
    if (packet->getLength() >= 20) setIntegerParam(ReqRaw5, raw[5]);
    if (packet->getLength() >= 24) setIntegerParam(ReqRaw6, raw[6]);
    if (packet->getLength() >= 28) setIntegerParam(ReqRaw7, raw[7]);

    setIntegerParam(SendQueIndex, index);
    setIntegerParam(SendQueSize, m_sendQue.size());
    callParamCallbacks();
}

void CommDebugPlugin::showRecvPacket(int index)
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

void CommDebugPlugin::showRecvPacket(const Packet *packet, int index)
{
    const uint32_t *raw = reinterpret_cast<const uint32_t *>(packet);

    setIntegerParam(RspVersion,     packet->getVersion());
    setIntegerParam(RspPriority,    packet->isPriority());
    setIntegerParam(RspType,        packet->getType());
    setIntegerParam(RspSequence,    packet->getSequenceId());
    setIntegerParam(RspLength,      packet->getLength());

    if (packet->getVersion() != 0) {
        if (packet->getType() == Packet::TYPE_DAS_CMD) {
            auto *cmdPacket = DasCmdPacket::cast(packet);
            setIntegerParam(RspCmdLen,      cmdPacket->getCmdPayloadLength());
            setIntegerParam(RspCmd,         cmdPacket->getCommand());
            setIntegerParam(RspCmdVerifyId, cmdPacket->getCmdId());
            setIntegerParam(RspCmdAck,      cmdPacket->isAcknowledge());
            setIntegerParam(RspCmdRsp,      cmdPacket->isResponse());
            setIntegerParam(RspCmdVer,      cmdPacket->getCmdVer());
            setStringParam(RspCmdModule,    cmdPacket->getModuleIdStr());

        } else if (packet->getType() == Packet::TYPE_ERROR) {
            auto *errPacket = ErrorPacket::cast(packet);
            setIntegerParam(RspErrCode,     errPacket->code);
            //setIntegerParam(RspErrSourceId, errPacket->source);

        } else if (packet->getType() == Packet::TYPE_RTDL) {
            auto *rtdlPacket = RtdlPacket::cast(packet);
            setIntegerParam(RspRtdlFrames,  rtdlPacket->getNumRtdlFrames());

        } else if (packet->getType() == Packet::TYPE_DAS_DATA) {
            auto *dataPacket = DasDataPacket::cast(packet);
            epicsTimeStamp timestamp = dataPacket->getTimeStamp();
            epicsTime t(timestamp);
            char timeStr[64], nsecStr[16];
            t.strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S.");
            snprintf(nsecStr, sizeof(nsecStr), "%09u", timestamp.nsec);
            strncat(timeStr, nsecStr, sizeof(timeStr));

            setIntegerParam(RspDataFormat,  dataPacket->getEventsFormat());
            setStringParam (RspDataTime,    timeStr);
            setIntegerParam(RspDataTimeNsec,timestamp.nsec);
        }
    }

    if (packet->getLength() >=  0) setIntegerParam(RspRaw0, raw[0]);
    if (packet->getLength() >=  4) setIntegerParam(RspRaw1, raw[1]);
    if (packet->getLength() >=  8) setIntegerParam(RspRaw2, raw[2]);
    if (packet->getLength() >= 12) setIntegerParam(RspRaw3, raw[3]);
    if (packet->getLength() >= 16) setIntegerParam(RspRaw4, raw[4]);
    if (packet->getLength() >= 20) setIntegerParam(RspRaw5, raw[5]);
    if (packet->getLength() >= 24) setIntegerParam(RspRaw6, raw[6]);
    if (packet->getLength() >= 28) setIntegerParam(RspRaw7, raw[7]);

    setIntegerParam(RecvQueIndex, index);
    setIntegerParam(RecvQueSize, m_recvQue.size());
    callParamCallbacks();
}
