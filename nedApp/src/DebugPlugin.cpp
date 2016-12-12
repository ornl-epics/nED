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
#include "Common.h"
#include "Log.h"

#include <algorithm>

EPICS_REGISTER_PLUGIN(DebugPlugin, 2, "Port name", string, "Dispatcher port name", string);

#define NUM_GENERICMODULEPLUGIN_PARAMS      ((int)(&LAST_GENERICMODULEPLUGIN_PARAM - &FIRST_GENERICMODULEPLUGIN_PARAM + 1))

DebugPlugin::DebugPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_GENERICMODULEPLUGIN_PARAMS, 1,
                 defaultInterfaceMask, defaultInterruptMask)
    , m_payloadLen(0)
{
    createParam("ReqDest",      asynParamOctet, &ReqDest, 0x0); // WRITE - Module address to communicate with
    createParam("ReqCmd",       asynParamInt32, &ReqCmd, 0x0);  // WRITE - Command to be sent to module
    createParam("ReqIsDsp",     asynParamInt32, &ReqIsDsp, 0);  // WRITE - Is the module we communicate with behinds the DSP, using LVDS link
    createParam("ReqSend",      asynParamInt32, &ReqSend);      // WRITE - Send cached packet
    createParam("RspCmd",       asynParamInt32, &RspCmd);       // READ - Response command (see DasPacket::CommandType)
    createParam("RspCmdAck",    asynParamInt32, &RspCmdAck);    // READ - Response ACK/NACK
    createParam("RspFlag",      asynParamInt32, &RspFlag);      // READ - Response flag present
    createParam("RspHwType",    asynParamInt32, &RspHwType);    // READ - Hardware type (see DasPacket::ModuleType)
    createParam("RspSrc",       asynParamOctet, &RspSrc);       // READ - Response source address
    createParam("RspRouter",    asynParamOctet, &RspRouter);    // READ - Response router address
    createParam("RspDest",      asynParamOctet, &RspDest);      // READ - Response destination address
    createParam("RspLen",       asynParamInt32, &RspLen);       // READ - Response length in bytes
    createParam("RspDataLen",   asynParamInt32, &RspDataLen);   // READ - Response payload length in bytes
    createParam("RspData",      asynParamOctet, &RspData);      // READ - Response payload
    createParam("ByteGrp",      asynParamInt32, &ByteGrp);      // WRITE - Byte grouping mode
    createParam("Channel",      asynParamInt32, &Channel, 0);   // Raw packet dword 0
    createParam("RawPkt0",      asynParamInt32, &RawPkt0, 0);   // Raw packet dword 0
    createParam("RawPkt1",      asynParamInt32, &RawPkt1, 0);   // Raw packet dword 1
    createParam("RawPkt2",      asynParamInt32, &RawPkt2, 0);   // Raw packet dword 2
    createParam("RawPkt3",      asynParamInt32, &RawPkt3, 0);   // Raw packet dword 3
    createParam("RawPkt4",      asynParamInt32, &RawPkt4, 0);   // Raw packet dword 4
    createParam("RawPkt5",      asynParamInt32, &RawPkt5, 0);   // Raw packet dword 5
    createParam("RawPkt6",      asynParamInt32, &RawPkt6, 0);   // Raw packet dword 6
    createParam("RawPkt7",      asynParamInt32, &RawPkt7, 0);   // Raw packet dword 7
    createParam("RawPkt8",      asynParamInt32, &RawPkt8, 0);   // Raw packet dword 8
    createParam("RawPkt9",      asynParamInt32, &RawPkt9, 0);   // Raw packet dword 9
    createParam("RawPkt10",     asynParamInt32, &RawPkt10, 0);  // Raw packet dword 10
    createParam("RawPkt11",     asynParamInt32, &RawPkt11, 0);  // Raw packet dword 11
    createParam("RawPkt12",     asynParamInt32, &RawPkt12, 0);  // Raw packet dword 12
    createParam("RawPkt13",     asynParamInt32, &RawPkt13, 0);  // Raw packet dword 13
    createParam("RawPkt14",     asynParamInt32, &RawPkt14, 0);  // Raw packet dword 14
    createParam("RawPkt15",     asynParamInt32, &RawPkt15, 0);  // Raw packet dword 15
    createParam("RawPkt16",     asynParamInt32, &RawPkt16, 0);  // Raw packet dword 16
    createParam("RawPkt17",     asynParamInt32, &RawPkt17, 0);  // Raw packet dword 17

    callParamCallbacks();

    generatePacket();
}

asynStatus DebugPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ReqSend) {
        if (value == 1)
            sendPacket();
        return asynSuccess;
    } else if (pasynUser->reason == ReqIsDsp) {
        setIntegerParam(ReqIsDsp, value);
        generatePacket();
        return asynSuccess;
    } else if (pasynUser->reason == ReqCmd) {
        setIntegerParam(ReqCmd, value);
        generatePacket();
        return asynSuccess;
    } else if (pasynUser->reason == RawPkt0) {
        m_rawPacket[0] = value;
    } else if (pasynUser->reason == RawPkt1) {
        m_rawPacket[1] = value;
    } else if (pasynUser->reason == RawPkt2) {
        m_rawPacket[2] = value;
    } else if (pasynUser->reason == RawPkt3) {
        value = ALIGN_UP(value, 4);
        if (value > 12*4)
            value = 12*4;
        m_rawPacket[3] = value;
    } else if (pasynUser->reason == RawPkt4) {
        m_rawPacket[4] = value;
    } else if (pasynUser->reason == RawPkt5) {
        m_rawPacket[5] = value;
    } else if (pasynUser->reason == RawPkt6) {
        m_rawPacket[6] = value;
    } else if (pasynUser->reason == RawPkt7) {
        m_rawPacket[7] = value;
    } else if (pasynUser->reason == RawPkt8) {
        m_rawPacket[8] = value;
    } else if (pasynUser->reason == RawPkt9) {
        m_rawPacket[9] = value;
    } else if (pasynUser->reason == RawPkt10) {
        m_rawPacket[10] = value;
    } else if (pasynUser->reason == RawPkt11) {
        m_rawPacket[11] = value;
    } else if (pasynUser->reason == RawPkt12) {
        m_rawPacket[12] = value;
    } else if (pasynUser->reason == RawPkt13) {
        m_rawPacket[13] = value;
    } else if (pasynUser->reason == RawPkt14) {
        m_rawPacket[14] = value;
    } else if (pasynUser->reason == RawPkt15) {
        m_rawPacket[15] = value;
    } else if (pasynUser->reason == RawPkt16) {
        m_rawPacket[16] = value;
    } else if (pasynUser->reason == RawPkt17) {
        m_rawPacket[17] = value;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus DebugPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == ReqDest) {
        setStringParam(ReqDest, value);
        generatePacket();
        *nActual = nChars;
        return asynSuccess;
    }
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

void DebugPlugin::generatePacket()
{
    DasPacket *packet;
    int cmd, isDsp;
    char destIp[20];
    getIntegerParam(ReqCmd,     &cmd);
    getIntegerParam(ReqIsDsp,   &isDsp);
    getStringParam(ReqDest,     sizeof(destIp), destIp);

    int dest = BaseModulePlugin::ip2addr(std::string(destIp, sizeof(destIp)));
    DasPacket::CommandType command = static_cast<const DasPacket::CommandType>(cmd & 0xFF);
    if (isDsp == 1)
        packet = DasPacket::createOcc(DasPacket::HWID_SELF, dest, command, 0, 0);
    else
        packet = DasPacket::createLvds(DasPacket::HWID_SELF, dest, command, 0, 0);

    memset(m_rawPacket, 0, sizeof(m_rawPacket));
    if (packet) {
        memcpy(m_rawPacket, packet, std::min((size_t)packet->getLength(), sizeof(m_rawPacket)));
        delete packet;
    }

    setIntegerParam(RawPkt0,  m_rawPacket[0]);
    setIntegerParam(RawPkt1,  m_rawPacket[1]);
    setIntegerParam(RawPkt2,  m_rawPacket[2]);
    setIntegerParam(RawPkt3,  m_rawPacket[3]);
    setIntegerParam(RawPkt4,  m_rawPacket[4]);
    setIntegerParam(RawPkt5,  m_rawPacket[5]);
    setIntegerParam(RawPkt6,  m_rawPacket[6]);
    setIntegerParam(RawPkt7,  m_rawPacket[7]);
    setIntegerParam(RawPkt8,  m_rawPacket[8]);
    setIntegerParam(RawPkt9,  m_rawPacket[9]);
    setIntegerParam(RawPkt10, m_rawPacket[10]);
    setIntegerParam(RawPkt11, m_rawPacket[11]);
    setIntegerParam(RawPkt12, m_rawPacket[12]);
    setIntegerParam(RawPkt13, m_rawPacket[13]);
    setIntegerParam(RawPkt14, m_rawPacket[14]);
    setIntegerParam(RawPkt15, m_rawPacket[15]);
    setIntegerParam(RawPkt16, m_rawPacket[16]);
    setIntegerParam(RawPkt17, m_rawPacket[17]);
    callParamCallbacks();
}

void DebugPlugin::sendPacket()
{
    DasPacket *packet = reinterpret_cast<DasPacket *>(m_rawPacket);
    BasePlugin::sendToDispatcher(packet);

    int nSent = 0;
    getIntegerParam(TxCount, &nSent);
    setIntegerParam(TxCount, ++nSent);

    // Invalidate all params
    setIntegerParam(RspCmd,     0);
    setIntegerParam(RspCmdAck,  0);
    setIntegerParam(RspFlag,    0);
    setIntegerParam(RspHwType,  0);
    setStringParam(RspSrc,      "");
    setStringParam(RspRouter,   "");
    setStringParam(RspDest,     "");
    setIntegerParam(RspLen,     0);
    setIntegerParam(RspDataLen, 0);
    m_payloadLen = 0;

    callParamCallbacks();
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

        if (parseCmd(packet))
            nProcessed++;
    }

    setIntegerParam(RxCount,    nReceived);
    setIntegerParam(ProcCount,  nProcessed);
    callParamCallbacks();
}

bool DebugPlugin::parseCmd(const DasPacket *packet)
{
    if (!packet->isCommand())
        return false;

    DasPacket::CommandType responseCmd = packet->getCommandType();

    setIntegerParam(RspCmd,     responseCmd);
    setIntegerParam(RspCmdAck,  (packet->cmdinfo.command == DasPacket::RSP_NACK || packet->cmdinfo.command == DasPacket::RSP_ACK) ? packet->cmdinfo.command : 0);
    setIntegerParam(RspFlag,    static_cast<int>(packet->isResponse()));
    setIntegerParam(RspHwType,  static_cast<int>(packet->cmdinfo.module_type));
    setStringParam(RspSrc,      BaseModulePlugin::addr2ip(packet->getSourceAddress()).c_str());
    setStringParam(RspRouter,   BaseModulePlugin::addr2ip(packet->getRouterAddress()).c_str());
    setStringParam(RspDest,     BaseModulePlugin::addr2ip(packet->destination).c_str());
    setIntegerParam(RspLen,     sizeof(DasPacket) + packet->payload_length);
    setIntegerParam(RspDataLen, packet->getPayloadLength());

    // Cache the payload to read it through readOctet()
    m_payloadLen = std::min(packet->getPayloadLength(), static_cast<uint32_t>(sizeof(m_payload)));
    memcpy(m_payload, packet->getPayload(), m_payloadLen);

    return true;
}
