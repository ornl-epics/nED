/* DebugPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseModulePlugin.h"
#include "DebugPlugin.h"
#include "Log.h"

#include <algorithm>

EPICS_REGISTER_PLUGIN(DebugPlugin, 2, "Port name", string, "Dispatcher port name", string);

#define NUM_GENERICMODULEPLUGIN_PARAMS      ((int)(&LAST_GENERICMODULEPLUGIN_PARAM - &FIRST_GENERICMODULEPLUGIN_PARAM + 1))

DebugPlugin::DebugPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_GENERICMODULEPLUGIN_PARAMS, 1,
                 defaultInterfaceMask, defaultInterruptMask)
    , m_hardwareId(0)
    , m_payloadLen(0)
    , m_expectedResponse(static_cast<DasPacket::CommandType>(0))
    , m_lastConfigPayloadLen(0)
{
    createParam("ReqDest",      asynParamOctet, &ReqDest);      // WRITE - Module address to communicate with
    createParam("ReqCmd",       asynParamInt32, &ReqCmd);       // WRITE - Command to be sent to module
    createParam("ReqIsDsp",     asynParamInt32, &ReqIsDsp);     // WRITE - Is the module we communicate with behinds the DSP, using LVDS link
    createParam("RspCmd",       asynParamInt32, &RspCmd);       // READ - Response command (see DasPacket::CommandType)
    createParam("RspCmdAck",    asynParamInt32, &RspCmdAck);    // READ - Response ACK/NACK
    createParam("RspHwType",    asynParamInt32, &RspHwType);    // READ - Hardware type (see DasPacket::ModuleType)
    createParam("RspSrc",       asynParamOctet, &RspSrc);       // READ - Response source address
    createParam("RspRouter",    asynParamOctet, &RspRouter);    // READ - Response router address
    createParam("RspDest",      asynParamOctet, &RspDest);      // READ - Response destination address
    createParam("RspLen",       asynParamInt32, &RspLen);       // READ - Response length in bytes
    createParam("RspDataLen",   asynParamInt32, &RspDataLen);   // READ - Response payload length in bytes
    createParam("RspData",      asynParamOctet, &RspData);      // READ - Response payload
    createParam("ByteGrp",      asynParamInt32, &ByteGrp);      // WRITE - Byte grouping mode
    createParam("Channel",      asynParamInt32, &Channel, 0);   // WRITE - Select channel to send command to (read/write config only)

    callParamCallbacks();
}

asynStatus DebugPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ReqCmd) {
        // Allow any value from the client.
        request(static_cast<const DasPacket::CommandType>(value & 0xFF));
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus DebugPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == ReqDest)
        m_hardwareId = BaseModulePlugin::ip2addr(std::string(value, nChars));
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);;
}

asynStatus DebugPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == RspData) {
        int byteGrp = GROUP_2_BYTES_SWAPPED;
        getIntegerParam(ByteGrp, &byteGrp);

        *nActual = 0;

        for (uint32_t i = 0; i < m_payloadLen; i++) {
            int len;
            uint32_t val = m_payload[i];
            switch (byteGrp) {
            case GROUP_2_BYTES:
                len = snprintf(value, nChars, "%04X %04X ", (val >> 16) & 0xFFFF, val & 0xFFFF);
                break;
            case GROUP_4_BYTES:
                len = snprintf(value, nChars, "%08X ", val);
                break;
            case GROUP_2_BYTES_SWAPPED:
            default:
                len = snprintf(value, nChars, "%04X %04X ", val & 0xFFFF, (val >> 16) & 0xFFFF);
                break;
            }
            if (len >= static_cast<int>(nChars) || len == -1)
                break;

            nChars -= len;
            *nActual += len;
            value += len;
        }
        if (eomReason) *eomReason |= ASYN_EOM_EOS;
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void DebugPlugin::request(const DasPacket::CommandType command)
{
    DasPacket *packet;
    int isDsp;
    int channel = 0;

    if (m_hardwareId == 0)
        return;

    (void)getIntegerParam(ReqIsDsp, &isDsp);
    (void)getIntegerParam(Channel, &channel);

    if (isDsp == 1)
        packet = DasPacket::createOcc(DasPacket::HWID_SELF, m_hardwareId, command, channel, m_lastConfigPayloadLen, m_lastConfigPayload);
    else
        packet = DasPacket::createLvds(DasPacket::HWID_SELF, m_hardwareId, command, channel, m_lastConfigPayloadLen, m_lastConfigPayload);

    if (packet) {
        BasePlugin::sendToDispatcher(packet);
        delete packet;

        m_expectedResponse = command;

        int nSent = 0;
        getIntegerParam(TxCount, &nSent);
        setIntegerParam(TxCount, ++nSent);

        // Invalidate all params
        setIntegerParam(RspCmd,     0);
        setIntegerParam(RspCmdAck,  0);
        setIntegerParam(RspHwType,  0);
        setStringParam(RspSrc,      "");
        setStringParam(RspRouter,   "");
        setStringParam(RspDest,     "");
        setIntegerParam(RspLen,     0);
        setIntegerParam(RspDataLen, 0);
        m_payloadLen = 0;

        callParamCallbacks();
    }
}

void DebugPlugin::processData(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);

    nReceived += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        // Silently skip packets we're not interested in
        if (!packet->isResponse() || packet->getSourceAddress() != m_hardwareId)
            continue;

        if (response(packet))
            nProcessed++;
    }

    setIntegerParam(RxCount,    nReceived);
    setIntegerParam(ProcCount,  nProcessed);
    callParamCallbacks();
}

bool DebugPlugin::response(const DasPacket *packet)
{
    DasPacket::CommandType responseCmd = packet->getResponseType();
    if (m_expectedResponse != responseCmd)
        return false;

    setIntegerParam(RspCmd,     responseCmd);
    setIntegerParam(RspCmdAck,  (packet->cmdinfo.command == DasPacket::RSP_NACK || packet->cmdinfo.command == DasPacket::RSP_ACK) ? packet->cmdinfo.command : 0);
    setIntegerParam(RspHwType,  static_cast<int>(packet->cmdinfo.module_type));
    setStringParam(RspSrc,      BaseModulePlugin::addr2ip(packet->getSourceAddress()).c_str());
    setStringParam(RspRouter,   BaseModulePlugin::addr2ip(packet->getRouterAddress()).c_str());
    setStringParam(RspDest,     BaseModulePlugin::addr2ip(packet->destination).c_str());
    setIntegerParam(RspLen,     sizeof(DasPacket) + packet->payload_length);
    setIntegerParam(RspDataLen, packet->getPayloadLength());

    // Cache the payload to read it through readOctet()
    m_payloadLen = std::min(packet->getPayloadLength()/4, static_cast<uint32_t>(sizeof(m_payload)));
    memcpy(m_payload, packet->getPayload(), m_payloadLen*4);

    // Cache last READ_CONFIG response separately, allowing other commands to be executed
    // before using this payload for writing configuration
    // Don't enforce MAC address checking, allowing to transfer the same configuration to
    // multiple boards of the same type.
    if (responseCmd == DasPacket::CMD_READ_CONFIG) {
        m_lastConfigPayloadLen = m_payloadLen * 4;
        memcpy(m_lastConfigPayload, m_payload, m_lastConfigPayloadLen);
    }

    return true;
}
