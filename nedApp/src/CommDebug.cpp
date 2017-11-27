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
    createParam("RspCmdLen",    asynParamInt32, &RspCmdLen, 0);     // WRITE - Command payload length
    createParam("RspCmd",       asynParamInt32, &RspCmd, 0x0);      // WRITE - Command to be sent to module
    createParam("RspVerifyId",  asynParamInt32, &RspVerifyId, 0);   // WRITE - Command verification id
    createParam("RspAck",       asynParamInt32, &RspAck, 0);        // WRITE - Command was acknowledged flag
    createParam("RspRsp",       asynParamInt32, &RspRsp, 0);        // WRITE - Command is response flag
    createParam("RspCmdVer",    asynParamInt32, &RspCmdVer, 0);     // WRITE - Command packet version
    createParam("RspModule",    asynParamOctet, &RspModule, 0x0);   // WRITE - Module address to communicate with
    createParam("RspRaw0",      asynParamInt32, &RspRaw0);          // Response packet raw word 0
    createParam("RspRaw1",      asynParamInt32, &RspRaw1);          // Response packet raw word 1
    createParam("RspRaw2",      asynParamInt32, &RspRaw2);          // Response packet raw word 2
    createParam("RspRaw3",      asynParamInt32, &RspRaw3);          // Response packet raw word 3
    createParam("RspRaw4",      asynParamInt32, &RspRaw4);          // Response packet raw word 4
    createParam("RspRaw5",      asynParamInt32, &RspRaw5);          // Response packet raw word 5
    createParam("RspRaw6",      asynParamInt32, &RspRaw6);          // Response packet raw word 6
    createParam("RspRaw7",      asynParamInt32, &RspRaw7);          // Response packet raw word 7
    createParam("RspTimeStamp", asynParamFloat64, &RspTimeStamp);   // Response time stamp in seconds with msec precision

    createParam("ReqSend",      asynParamInt32, &ReqSend);          // WRITE - Send cached packet
    createParam("ReqSniffer",   asynParamInt32, &ReqSniffer);       // WRITE - Enables listening to other plugins messages
    createParam("PktQueIndex",  asynParamInt32, &PktQueIndex, 0);   // Currently selected packet
    createParam("PktQueSize",   asynParamInt32, &PktQueSize, 0);    // Number of elements in packet buffer
    createParam("PktQueMaxSize",asynParamInt32, &PktQueMaxSize, 10);// Max num of elements in packet buffer

    callParamCallbacks();

    BasePlugin::connect(parentPlugins, MsgDasCmd);

    generatePacket(false);
}

asynStatus CommDebug::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    std::list<int> raws = { ReqRaw0, ReqRaw1, ReqRaw2, ReqRaw3, ReqRaw4, ReqRaw5, ReqRaw6, ReqRaw7 };
    bool fromRaw = false;

    if (pasynUser->reason == ReqSend) {
        generatePacket(false);
        sendPacket();
        return asynSuccess;
    } else if (pasynUser->reason == PktQueIndex) {
        selectRecvPacket(value);
        return asynSuccess;
    } else if (std::find(raws.begin(), raws.end(), pasynUser->reason) != raws.end()) {
        fromRaw = true;
    }

    asynStatus ret = BasePlugin::writeInt32(pasynUser, value);
    generatePacket(fromRaw);
    showSendPacket();
    return ret;
}

asynStatus CommDebug::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == ReqModule) {
        setStringParam(ReqModule, value);
        generatePacket(false);
        showSendPacket();
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

void CommDebug::sendPacket()
{
    // Invalidate all params
    setIntegerParam(RspVersion,   0);
    setIntegerParam(RspPriority,  0);
    setIntegerParam(RspType,      0);
    setIntegerParam(RspSequence,  0);
    setIntegerParam(RspLength,    0);
    setIntegerParam(RspCmdLen,    0);
    setIntegerParam(RspCmd,       0);
    setIntegerParam(RspVerifyId,  0);
    setIntegerParam(RspAck,       0);
    setIntegerParam(RspRsp,       0);
    setIntegerParam(RspCmdVer,    0);
    setStringParam(RspModule,    "");
    setIntegerParam(RspRaw0,      0);
    setIntegerParam(RspRaw1,      0);
    setIntegerParam(RspRaw2,      0);
    setIntegerParam(RspRaw3,      0);
    setIntegerParam(RspRaw4,      0);
    setIntegerParam(RspRaw5,      0);
    setIntegerParam(RspRaw6,      0);
    setIntegerParam(RspRaw7,      0);
    setDoubleParam(RspTimeStamp,  0);

    m_lastPacketQueue.erase(m_lastPacketQueue.begin(), m_lastPacketQueue.end());
    setIntegerParam(PktQueIndex, 0);
    setIntegerParam(PktQueSize, 0);

    callParamCallbacks();

    BasePlugin::sendUpstream(m_packet);
}

void CommDebug::recvDownstream(DasCmdPacketList *packets)
{
    bool changePacket = false;

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        const DasCmdPacket *packet = *it;

        if (savePacket(packet)) {
            changePacket = true;
        }
    }

    if (changePacket) {
        selectRecvPacket(0);
    }

    setIntegerParam(PktQueSize, m_lastPacketQueue.size());
    callParamCallbacks();

    this->unlock();
    sendDownstream(packets, false);
    this->lock();
}

void CommDebug::recvUpstream(DasCmdPacketList *packets)
{
    bool sniffer = false;
    sendUpstream(MsgDasCmd, packets);

    getBooleanParam(ReqSniffer, sniffer);

    if (sniffer && !packets->empty()) {
        DasCmdPacket *packet = packets->back();
        memcpy(m_buffer, packet, std::min(sizeof(m_buffer), static_cast<size_t>(packet->length)));
        m_packet->length = std::min(sizeof(m_buffer), static_cast<size_t>(packet->length));
        showSendPacket();
    }
}

bool CommDebug::savePacket(const DasCmdPacket *packet)
{
    int maxQueSize = 0;
    getIntegerParam(PktQueMaxSize, &maxQueSize);
    if (maxQueSize <= 0)
        return false;

    // Cache the payload to read it through readOctet()
    PacketDesc pkt;
    pkt.length = std::min(packet->length, static_cast<uint32_t>(sizeof(pkt.data)));
    memcpy(pkt.data, packet, pkt.length);
    epicsTimeGetCurrent(&pkt.timestamp);
    while ((int)m_lastPacketQueue.size() >= maxQueSize)
        m_lastPacketQueue.pop_back();
    m_lastPacketQueue.push_front(pkt);

    return true;
}

void CommDebug::showSendPacket()
{
    setIntegerParam(ReqVersion,   m_packet->version);
    setIntegerParam(ReqPriority,  m_packet->priority);
    setIntegerParam(ReqType,      m_packet->type);
    setIntegerParam(ReqSequence,  m_packet->sequence);
    setIntegerParam(ReqLength,    m_packet->length);
    setIntegerParam(ReqCmdLen,    m_packet->cmd_length);
    setIntegerParam(ReqCmd,       m_packet->command);
    setIntegerParam(ReqVerifyId,  m_packet->cmd_sequence);
    setIntegerParam(ReqAck,       m_packet->acknowledge);
    setIntegerParam(ReqRsp,       m_packet->response);
    setIntegerParam(ReqCmdVer,    m_packet->lvds_version);
    setStringParam(ReqModule,     BaseModulePlugin::addr2ip(m_packet->module_id));

    setIntegerParam(ReqRaw0,      m_buffer[0]);
    setIntegerParam(ReqRaw1,      m_buffer[1]);
    setIntegerParam(ReqRaw2,      m_buffer[2]);
    setIntegerParam(ReqRaw3,      m_buffer[3]);
    setIntegerParam(ReqRaw4,      m_buffer[4]);
    setIntegerParam(ReqRaw5,      m_buffer[5]);
    setIntegerParam(ReqRaw6,      m_buffer[6]);
    setIntegerParam(ReqRaw7,      m_buffer[7]);

    callParamCallbacks();
}

void CommDebug::selectRecvPacket(int index)
{
    struct timespec rspTimeStamp; // In POSIX time

    if (m_lastPacketQueue.empty())
        return;

    if (index < 0)
        index *= -1;
    if (index >= (int)m_lastPacketQueue.size())
        index = (int)m_lastPacketQueue.size() - 1;

    auto it = m_lastPacketQueue.begin();
    for (int i=0; i<index; i++) {
        if (it == m_lastPacketQueue.end()) {
            return;
        }
        it++;
    }

    DasCmdPacket *packet = reinterpret_cast<DasCmdPacket*>(it->data);

    setIntegerParam(RspVersion,   packet->version);
    setIntegerParam(RspPriority,  packet->priority);
    setIntegerParam(RspType,      packet->type);
    setIntegerParam(RspSequence,  packet->sequence);
    setIntegerParam(RspLength,    packet->length);
    setIntegerParam(RspCmdLen,    packet->cmd_length);
    setIntegerParam(RspCmd,       packet->command);
    setIntegerParam(RspVerifyId,  packet->cmd_sequence);
    setIntegerParam(RspAck,       packet->acknowledge);
    setIntegerParam(RspRsp,       packet->response);
    setIntegerParam(RspCmdVer,    packet->lvds_version);
    setStringParam(RspModule,     BaseModulePlugin::addr2ip(packet->module_id));

    if (it->length >=  0) setIntegerParam(RspRaw0, it->data[0]);
    if (it->length >=  4) setIntegerParam(RspRaw1, it->data[1]);
    if (it->length >=  8) setIntegerParam(RspRaw2, it->data[2]);
    if (it->length >= 12) setIntegerParam(RspRaw3, it->data[3]);
    if (it->length >= 16) setIntegerParam(RspRaw4, it->data[4]);
    if (it->length >= 20) setIntegerParam(RspRaw5, it->data[5]);
    if (it->length >= 24) setIntegerParam(RspRaw6, it->data[6]);
    if (it->length >= 28) setIntegerParam(RspRaw7, it->data[7]);

    // Return responses' POSIX time with milisecond precision
    epicsTimeToTimespec(&rspTimeStamp, &it->timestamp);
    double rspTime = 1000*rspTimeStamp.tv_sec + (uint32_t)(rspTimeStamp.tv_nsec / 1e6);
    setDoubleParam(RspTimeStamp,rspTime);
    setIntegerParam(PktQueIndex,index);
    callParamCallbacks();
}
