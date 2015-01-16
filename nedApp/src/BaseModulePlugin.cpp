/* BaseModulePlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseModulePlugin.h"
#include "Common.h"
#include "Log.h"

#include <osiSock.h>
#include <string.h>

#define NUM_BASEMODULEPLUGIN_PARAMS ((int)(&LAST_BASEMODULEPLUGIN_PARAM - &FIRST_BASEMODULEPLUGIN_PARAM + 1))

const float BaseModulePlugin::NO_RESPONSE_TIMEOUT = 2.0;

BaseModulePlugin::BaseModulePlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId,
                                   bool behindDsp, int blocking, int numParams, int interfaceMask, int interruptMask)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_BASEMODULEPLUGIN_PARAMS + numParams, 1,
                 interfaceMask | defaultInterfaceMask, interruptMask | defaultInterruptMask)
    , m_hardwareId(ip2addr(hardwareId))
    , m_statusPayloadLength(0)
    , m_countersPayloadLength(0)
    , m_configPayloadLength(0)
    , m_verifySM(ST_TYPE_VERSION_INIT)
    , m_waitingResponse(static_cast<DasPacket::CommandType>(0))
    , m_behindDsp(behindDsp)
{
    m_verifySM.addState(ST_TYPE_VERSION_INIT,       SM_ACTION_ACK(DasPacket::CMD_DISCOVER),         ST_TYPE_OK);
    m_verifySM.addState(ST_TYPE_VERSION_INIT,       SM_ACTION_ERR(DasPacket::CMD_DISCOVER),         ST_TYPE_ERR);
    m_verifySM.addState(ST_TYPE_VERSION_INIT,       SM_ACTION_ACK(DasPacket::CMD_READ_VERSION),     ST_VERSION_OK);
    m_verifySM.addState(ST_TYPE_VERSION_INIT,       SM_ACTION_ERR(DasPacket::CMD_READ_VERSION),     ST_VERSION_ERR);
    m_verifySM.addState(ST_TYPE_OK,                 SM_ACTION_ACK(DasPacket::CMD_READ_VERSION),     ST_TYPE_VERSION_OK);
    m_verifySM.addState(ST_TYPE_OK,                 SM_ACTION_ERR(DasPacket::CMD_READ_VERSION),     ST_VERSION_ERR);
    m_verifySM.addState(ST_VERSION_OK,              SM_ACTION_ACK(DasPacket::CMD_DISCOVER),         ST_TYPE_VERSION_OK);
    m_verifySM.addState(ST_VERSION_OK,              SM_ACTION_ERR(DasPacket::CMD_DISCOVER),         ST_TYPE_ERR);

    createParam("CmdRsp",       asynParamInt32, &CmdRsp,    LAST_CMD_NONE); // READ - Last command response status   (see BaseModulePlugin::LastCommandResponse)
    createParam("CmdReq",       asynParamInt32, &CmdReq);                   // WRITE - Send command to module        (see DasPacket::CommandType)
    createParam("HwId",         asynParamOctet, &HwId);                     // READ - Connected module hardware id
    createParam("HwType",       asynParamInt32, &HwType);                   // READ - Module type                    (see DasPacket::ModuleType)
    createParam("HwDate",       asynParamOctet, &HwDate);                   // READ - Module hardware date
    createParam("HwVer",        asynParamInt32, &HwVer);                    // READ - Module hardware version
    createParam("HwRev",        asynParamInt32, &HwRev);                    // READ - Module hardware revision
    createParam("FwDate",       asynParamOctet, &FwDate);                   // READ - Module firmware date
    createParam("FwVer",        asynParamInt32, &FwVer);                    // READ - Module firmware version
    createParam("FwRev",        asynParamInt32, &FwRev);                    // READ - Module firmware revision
    createParam("Supported",    asynParamInt32, &Supported);                // READ - Is requested module version supported (0=not supported,1=supported)
    createParam("Verified",     asynParamInt32, &Verified);                 // READ - Flag whether module type and version were verified
    createParam("CfgSection",   asynParamInt32, &CfgSection, '0');          // WRITE - Select configuration section to be written with next WRITE_CONFIG request, 0 for all

    std::string hardwareIp = addr2ip(m_hardwareId);
    setStringParam(HwId, hardwareIp.c_str());
    setIntegerParam(CmdRsp, LAST_CMD_NONE);
    callParamCallbacks();
}

BaseModulePlugin::~BaseModulePlugin()
{}

asynStatus BaseModulePlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == CmdReq) {
        if (m_waitingResponse != 0) {
            LOG_WARN("Command '%d' not allowed while waiting for 0x%02X response", value, m_waitingResponse);
            return asynError;
        }

        setIntegerParam(CmdRsp, LAST_CMD_WAIT);
        callParamCallbacks();

        switch (value) {
        case DasPacket::CMD_DISCOVER:
            m_waitingResponse = reqDiscover();
            break;
        case DasPacket::CMD_READ_VERSION:
            m_waitingResponse = reqReadVersion();
            break;
        case DasPacket::CMD_READ_STATUS:
            m_waitingResponse = reqReadStatus();
            break;
        case DasPacket::CMD_READ_STATUS_COUNTERS:
            m_waitingResponse = reqReadStatusCounters();
            break;
        case DasPacket::CMD_RESET_STATUS_COUNTERS:
            m_waitingResponse = reqResetStatusCounters();
            break;
        case DasPacket::CMD_READ_CONFIG:
            m_waitingResponse = reqReadConfig();
            break;
        case DasPacket::CMD_WRITE_CONFIG:
            m_waitingResponse = reqWriteConfig();
            break;
        case DasPacket::CMD_START:
            m_waitingResponse = reqStart();
            break;
        case DasPacket::CMD_STOP:
            m_waitingResponse = reqStop();
            break;
        default:
            setIntegerParam(CmdRsp, LAST_CMD_ERROR);
            callParamCallbacks();
            LOG_WARN("Unrecognized '%d' command", SM_ACTION_CMD(value));
            return asynError;
        }

        if (m_waitingResponse != static_cast<DasPacket::CommandType>(0)) {
            if (!scheduleTimeoutCallback(m_waitingResponse, 
                                         NO_RESPONSE_TIMEOUT))
               LOG_WARN("Failed to schedule CmdRsp timeout callback");
            setIntegerParam(CmdRsp, LAST_CMD_WAIT);
        } else {
            setIntegerParam(CmdRsp, LAST_CMD_SKIPPED);
        }
        callParamCallbacks();

        return asynSuccess;
    }
    if (pasynUser->reason == CfgSection) {
        if (value < '0' || value > 'F' || (value > '9' && value < 'A')) {
            LOG_ERROR("Invalid configuration section %d", value);
            return asynError;
        }
        setIntegerParam(CfgSection, value);
        callParamCallbacks();
        return asynSuccess;
    }

    // Not a command, it's probably the new configuration option
    std::map<int, struct ConfigParamDesc>::iterator it = m_configParams.find(pasynUser->reason);
    if (it != m_configParams.end()) {
        uint32_t mask = (0x1ULL << it->second.width) - 1;
        if (static_cast<int>(value & mask) != value) {
            LOG_ERROR("Parameter %s value %d out of bounds", getParamName(it->first), value);
            return asynError;
        } else {
            setIntegerParam(it->first, value);
            callParamCallbacks();
            return asynSuccess;
        }
    }

    // Just issue default handler to see if it can handle it
    return BasePlugin::writeInt32(pasynUser, value);
}

void BaseModulePlugin::sendToDispatcher(DasPacket::CommandType command, uint32_t *payload, uint32_t length)
{
    DasPacket *packet;
    if (m_behindDsp)
        packet = DasPacket::createLvds(DasPacket::HWID_SELF, m_hardwareId, command, length, payload);
    else
        packet = DasPacket::createOcc(DasPacket::HWID_SELF, m_hardwareId, command, length, payload);

    if (packet) {
        BasePlugin::sendToDispatcher(packet);
        delete packet;
    } else {
        LOG_ERROR("Failed to create and send packet");
    }
}

void BaseModulePlugin::processData(const DasPacketList * const packetList)
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

        if (processResponse(packet))
            nProcessed++;
    }

    setIntegerParam(RxCount,    nReceived);
    setIntegerParam(ProcCount,  nProcessed);
    callParamCallbacks();
}

bool BaseModulePlugin::processResponse(const DasPacket *packet)
{
    bool ack = false;
    DasPacket::CommandType command = packet->getResponseType();

    if (m_waitingResponse != command) {
        LOG_WARN("Response '0x%02X' not allowed while waiting for 0x%02X", command, m_waitingResponse);
        return false;
    }
    m_waitingResponse = static_cast<DasPacket::CommandType>(0);

    switch (command) {
    case DasPacket::CMD_DISCOVER:
        ack = rspDiscover(packet);
        m_verifySM.transition(ack ? SM_ACTION_ACK(DasPacket::CMD_DISCOVER) : SM_ACTION_ERR(DasPacket::CMD_DISCOVER));
        setIntegerParam(Verified, m_verifySM.getCurrentState());
        break;
    case DasPacket::CMD_READ_VERSION:
        ack = rspReadVersion(packet);
        m_verifySM.transition(ack ? SM_ACTION_ACK(DasPacket::CMD_READ_VERSION) : SM_ACTION_ERR(DasPacket::CMD_READ_VERSION));
        setIntegerParam(Verified, m_verifySM.getCurrentState());
        break;
    case DasPacket::CMD_READ_CONFIG:
        ack = rspReadConfig(packet);
        break;
    case DasPacket::CMD_READ_STATUS:
        ack = rspReadStatus(packet);
        break;
    case DasPacket::CMD_READ_STATUS_COUNTERS:
        ack = rspReadStatusCounters(packet);
        break;
    case DasPacket::CMD_RESET_STATUS_COUNTERS:
        ack = rspResetStatusCounters(packet);
        break;
    case DasPacket::CMD_WRITE_CONFIG:
    case DasPacket::CMD_WRITE_CONFIG_1:
    case DasPacket::CMD_WRITE_CONFIG_2:
    case DasPacket::CMD_WRITE_CONFIG_3:
    case DasPacket::CMD_WRITE_CONFIG_4:
    case DasPacket::CMD_WRITE_CONFIG_5:
    case DasPacket::CMD_WRITE_CONFIG_6:
    case DasPacket::CMD_WRITE_CONFIG_7:
    case DasPacket::CMD_WRITE_CONFIG_8:
    case DasPacket::CMD_WRITE_CONFIG_9:
    case DasPacket::CMD_WRITE_CONFIG_A:
    case DasPacket::CMD_WRITE_CONFIG_B:
    case DasPacket::CMD_WRITE_CONFIG_C:
    case DasPacket::CMD_WRITE_CONFIG_D:
    case DasPacket::CMD_WRITE_CONFIG_E:
    case DasPacket::CMD_WRITE_CONFIG_F:
        ack = rspWriteConfig(packet);
        break;
    case DasPacket::CMD_START:
        ack = rspStart(packet);
        break;
    case DasPacket::CMD_STOP:
        ack = rspStop(packet);
        break;
    default:
        LOG_WARN("Received unhandled response 0x%02X", command);
        return false;
    }

    setIntegerParam(CmdRsp, (ack ? LAST_CMD_OK : LAST_CMD_ERROR));
    callParamCallbacks();
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqDiscover()
{
    sendToDispatcher(DasPacket::CMD_DISCOVER);
    return DasPacket::CMD_DISCOVER;
}

bool BaseModulePlugin::rspDiscover(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received DISCOVER response after timeout");
        return false;
    }
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadVersion()
{
    sendToDispatcher(DasPacket::CMD_READ_VERSION);
    return DasPacket::CMD_READ_VERSION;
}

bool BaseModulePlugin::rspReadVersion(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_VERSION response after timeout");
        return false;
    }
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadStatus()
{
    sendToDispatcher(DasPacket::CMD_READ_STATUS);
    return DasPacket::CMD_READ_STATUS;
}

bool BaseModulePlugin::rspReadStatus(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_STATUS response after timeout");
        return false;
    }

    if (packet->getPayloadLength() != ALIGN_UP(m_statusPayloadLength, 4)) {
        LOG_ERROR("Received wrong READ_STATUS response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), ALIGN_UP(m_statusPayloadLength, 4));
        return false;
    }

    const uint32_t *payload = packet->getPayload();
    for (auto it=m_statusParams.begin(); it != m_statusParams.end(); it++) {
        int offset = it->second.offset;
        int shift = it->second.shift;
        if (m_behindDsp) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }
        int value = payload[offset] >> shift;
        if ((shift + it->second.width) > 32) {
            value |= payload[offset + 1] << (32 - shift);
        }
        value &= (0x1ULL << it->second.width) - 1;
        setIntegerParam(it->first, value);
    }
    callParamCallbacks();
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadStatusCounters()
{
    sendToDispatcher(DasPacket::CMD_READ_STATUS_COUNTERS);
    return DasPacket::CMD_READ_STATUS_COUNTERS;
}

DasPacket::CommandType BaseModulePlugin::reqResetStatusCounters()
{
    sendToDispatcher(DasPacket::CMD_RESET_STATUS_COUNTERS);
    return DasPacket::CMD_RESET_STATUS_COUNTERS;
}

bool BaseModulePlugin::rspResetStatusCounters(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received RESET_STATUS response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

bool BaseModulePlugin::rspReadStatusCounters(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_STATUS_COUNTERS response after timeout");
        return false;
    }

    if (packet->getPayloadLength() != ALIGN_UP(m_countersPayloadLength, 4)) {
        LOG_ERROR("Received wrong READ_STATUS_COUNTERS response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), ALIGN_UP(m_countersPayloadLength, 4));
        return false;
    }

    const uint32_t *payload = packet->getPayload();
    for (std::map<int, StatusParamDesc>::iterator it=m_counterParams.begin(); it != m_counterParams.end(); it++) {
        int offset = it->second.offset;
        int shift = it->second.shift;
        if (m_behindDsp) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }
        int value = payload[offset] >> shift;
        if ((shift + it->second.width) > 32) {
            value |= payload[offset + 1] << (32 - shift);
        }
        value &= (0x1ULL << it->second.width) - 1;
        setIntegerParam(it->first, value);
    }
    callParamCallbacks();
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadConfig()
{
    sendToDispatcher(DasPacket::CMD_READ_CONFIG);
    return DasPacket::CMD_READ_CONFIG;
}

bool BaseModulePlugin::rspReadConfig(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_CONFIG response after timeout");
        return false;
    }

    if (m_configPayloadLength == 0)
        recalculateConfigParams();

    if (packet->getPayloadLength() != ALIGN_UP(m_configPayloadLength, 4)) {
        LOG_ERROR("Received wrong READ_CONFIG response based on length; "
                  "received %uB, expected %uB",
                  packet->getPayloadLength(), ALIGN_UP(m_configPayloadLength, 4));
        return false;
    }

    const uint32_t *payload = packet->getPayload();
    for (auto it=m_configParams.begin(); it != m_configParams.end(); it++) {
        int offset = m_configSectionOffsets[it->second.section] + it->second.offset;
        int shift = it->second.shift;
        if (m_behindDsp) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }
        int value = payload[offset] >> shift;
        if ((shift + it->second.width) > 32) {
            value |= payload[offset + 1] << (32 - shift);
        }
        value &= (0x1ULL << it->second.width) - 1;
        setIntegerParam(it->first, value);
    }
    callParamCallbacks();
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqWriteConfig()
{
    int cfgSection = 0;
    getIntegerParam(CfgSection, &cfgSection);

    if (m_configPayloadLength == 0)
        recalculateConfigParams();

    uint32_t payloadLength;
    if (cfgSection == '0') {
        payloadLength = m_configPayloadLength;
    } else {
        int wordsize = (m_behindDsp ? 2 : 4);
        payloadLength = m_configSectionSizes[cfgSection] * wordsize;
    }

    // Skip empty sections
    if (payloadLength == 0) {
        return static_cast<DasPacket::CommandType>(0);
    }

    uint32_t length = ALIGN_UP(payloadLength, 4) / 4;
    uint32_t data[length];
    for (uint32_t i=0; i<length; i++) {
        data[i] = 0;
    }

    for (auto it=m_configParams.begin(); it != m_configParams.end(); it++) {
        uint32_t mask = (0x1ULL << it->second.width) - 1;
        int shift = it->second.shift;
        int value = 0;
        uint32_t offset = it->second.offset;

        if (cfgSection == '0') {
            offset += m_configSectionOffsets[it->second.section];
        } else if (cfgSection != it->second.section) {
            continue;
        }

        if (getIntegerParam(it->first, &value) != asynSuccess) {
            // This should not happen. It's certainly error when creating and parameters.
            LOG_ERROR("Failed to get parameter %s value", getParamName(it->first));
            return static_cast<DasPacket::CommandType>(0);
        }
        if (static_cast<int>(value & mask) != value) {
            // This should not happen. It's certainly error when setting new value for parameter
            LOG_WARN("Parameter %s value out of range", getParamName(it->first));
        }
        value &= mask;

        if (m_behindDsp) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }

LOG_ERROR("offset=%d length=%d", offset, length);
        if (offset >= length) {
            // Unlikely, but rather sure than sorry
            LOG_ERROR("Parameter %s offset out of range", getParamName(it->first));
            continue;
        }

        data[offset] |= value << shift;
        if ((it->second.width + shift) > 32) {
            data[offset+1] |= value >> (it->second.width -(32 - shift + 1));
        }
    }

    DasPacket::CommandType rsp;
    switch (cfgSection) {
        case '0': rsp = DasPacket::CMD_WRITE_CONFIG;   break;
        case '1': rsp = DasPacket::CMD_WRITE_CONFIG_1; break;
        case '2': rsp = DasPacket::CMD_WRITE_CONFIG_2; break;
        case '3': rsp = DasPacket::CMD_WRITE_CONFIG_3; break;
        case '4': rsp = DasPacket::CMD_WRITE_CONFIG_4; break;
        case '5': rsp = DasPacket::CMD_WRITE_CONFIG_5; break;
        case '6': rsp = DasPacket::CMD_WRITE_CONFIG_6; break;
        case '7': rsp = DasPacket::CMD_WRITE_CONFIG_7; break;
        case '8': rsp = DasPacket::CMD_WRITE_CONFIG_8; break;
        case '9': rsp = DasPacket::CMD_WRITE_CONFIG_9; break;
        case 'A': rsp = DasPacket::CMD_WRITE_CONFIG_A; break;
        case 'B': rsp = DasPacket::CMD_WRITE_CONFIG_B; break;
        case 'C': rsp = DasPacket::CMD_WRITE_CONFIG_C; break;
        case 'D': rsp = DasPacket::CMD_WRITE_CONFIG_D; break;
        case 'E': rsp = DasPacket::CMD_WRITE_CONFIG_E; break;
        case 'F': rsp = DasPacket::CMD_WRITE_CONFIG_F; break;
        default:
            return static_cast<DasPacket::CommandType>(0);
    }
    sendToDispatcher(rsp, data, payloadLength);
    return rsp;
}

bool BaseModulePlugin::rspWriteConfig(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_STATUS response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqStart()
{
    sendToDispatcher(DasPacket::CMD_START);
    return DasPacket::CMD_START;
}

bool BaseModulePlugin::rspStart(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received CMD_START response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqStop()
{
    sendToDispatcher(DasPacket::CMD_STOP);
    return DasPacket::CMD_STOP;
}

bool BaseModulePlugin::rspStop(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received CMD_STPO response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

void BaseModulePlugin::createStatusParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Module status parameter '%s' cannot be created (already exist?)", name);
        return;
    }

    StatusParamDesc desc;
    desc.offset = offset;
    desc.shift = shift;
    desc.width = nBits;
    m_statusParams[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    m_statusPayloadLength = std::max(m_statusPayloadLength, length*wordsize);
}

void BaseModulePlugin::createCounterParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Module status counter parameter '%s' cannot be created (already exist?)", name);
        return;
    }

    StatusParamDesc desc;
    desc.offset = offset;
    desc.shift = shift;
    desc.width = nBits;
    m_counterParams[index] = desc;

    uint32_t length = offset +1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    m_countersPayloadLength = std::max(m_countersPayloadLength, length*wordsize);
}

void BaseModulePlugin::createConfigParam(const char *name, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Module config parameter '%s' cannot be created (already exist?)", name);
        return;
    }
    setIntegerParam(index, value);

    ConfigParamDesc desc;
    desc.section = section;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    desc.initVal = value;
    m_configParams[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    // m_configPayloadLength is calculated *after* we create all sections
    m_configSectionSizes[section] = std::max(m_configSectionSizes[section], length);
}

uint32_t BaseModulePlugin::ip2addr(const std::string &text)
{
    uint32_t id = 0;

    if (text.substr(0, 2) == "0x") {
        char *endptr;
        id = strtoul(text.c_str(), &endptr, 16);
        if (*endptr != 0)
            id = 0;
    } else {
        struct in_addr hwid;
        if (hostToIPAddr(text.c_str(), &hwid) == 0)
            id = ntohl(hwid.s_addr);
    }
    return id;
}

std::string BaseModulePlugin::addr2ip(uint32_t addr)
{
    char buf[16] = "";
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF);
    buf[sizeof(buf)-1] = '\0';
    return std::string(buf);
}

float BaseModulePlugin::noResponseCleanup(DasPacket::CommandType command)
{
    if (m_waitingResponse == command) {
        m_waitingResponse = static_cast<DasPacket::CommandType>(0);
        setIntegerParam(CmdRsp, LAST_CMD_TIMEOUT);
        callParamCallbacks();
    }
    return 0;
}

bool BaseModulePlugin::scheduleTimeoutCallback(DasPacket::CommandType command, double delay)
{
    std::function<float(void)> timeoutCb = std::bind(&BaseModulePlugin::noResponseCleanup, this, command);
    m_timeoutTimer = scheduleCallback(timeoutCb, NO_RESPONSE_TIMEOUT);
    return (m_timeoutTimer.get() != 0);
}

bool BaseModulePlugin::cancelTimeoutCallback()
{
    bool canceled = false;
    if (m_timeoutTimer) {
        canceled = m_timeoutTimer->cancel();
        m_timeoutTimer.reset();
    }
    return canceled;
}

void BaseModulePlugin::recalculateConfigParams()
{
    // Section sizes have already been calculated in createConfigParams()

    // Calculate section offsets
    m_configSectionOffsets['1'] = 0x0;
    for (char i='1'; i<='9'; i++) {
        m_configSectionOffsets[i] = m_configSectionOffsets[i-1] + m_configSectionSizes[i-1];
        LOG_WARN("Section '%c' size=%u offset=%u", i, m_configSectionSizes[i], m_configSectionOffsets[i]);
    }
    m_configSectionOffsets['A'] = m_configSectionOffsets['9'] + m_configSectionSizes['9'];
    LOG_WARN("Section '%c' size=%u offset=%u", 'A', m_configSectionSizes['A'], m_configSectionOffsets['A']);
    for (char i='B'; i<='F'; i++) {
        m_configSectionOffsets[i] = m_configSectionOffsets[i-1] + m_configSectionSizes[i-1];
        LOG_WARN("Section '%c' size=%u offset=%u", i, m_configSectionSizes[i], m_configSectionOffsets[i]);
    }

    // Calculate total required payload size in bytes
    m_configPayloadLength = m_configSectionOffsets['F'] + m_configSectionSizes['F'];
    int wordsize = (m_behindDsp ? 2 : 4);
    m_configPayloadLength = m_configPayloadLength * wordsize;
}
