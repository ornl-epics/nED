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
#include "ValueConvert.h"

#include <map>
#include <osiSock.h>
#include <string.h>

#define NUM_BASEMODULEPLUGIN_PARAMS ((int)(&LAST_BASEMODULEPLUGIN_PARAM - &FIRST_BASEMODULEPLUGIN_PARAM + 1))

#define VERIFY_DISCOVER_OK      (1 << 0)
#define VERIFY_DISCOVER_FAIL    (1 << 1)
#define VERIFY_VERSION_OK       (1 << 2)
#define VERIFY_VERSION_FAIL     (1 << 3)
#define VERIFY_DISCOVER_MASK    (VERIFY_DISCOVER_OK | VERIFY_DISCOVER_FAIL)
#define VERIFY_VERSION_MASK     (VERIFY_VERSION_OK  | VERIFY_VERSION_FAIL)

/**
 * Return a unique id for section and channel pair that can be used to
 * identify the pair in m_configSectionSizes and m_configSectionOffsets
 */
#define SECTION_ID(section, channel) (((channel) * 0x10) + ((section) & 0xF))

const float BaseModulePlugin::NO_RESPONSE_TIMEOUT = 1.0; // Default value, user can override
const float BaseModulePlugin::RESET_NO_RESPONSE_TIMEOUT = 5.0; // Overrides m_noResponseTimeout for CMD_RESET
const UnsignConvert *BaseModulePlugin::CONV_UNSIGN = new UnsignConvert();
const Sign2sComplementConvert *BaseModulePlugin::CONV_SIGN_2COMP = new Sign2sComplementConvert();
const SignMagnitudeConvert *BaseModulePlugin::CONV_SIGN_MAGN = new SignMagnitudeConvert();

/**
 * This is a global map from hardwareId to plugin name. Whenever a module is
 * registered, it is added to this map. Luckily modules are registered on
 * startup only and this global variable is read only afterwards => no need
 * for thread safety mechanisms.
 */
static std::map<uint32_t, std::string> g_namesMap;

BaseModulePlugin::BaseModulePlugin(const char *portName, const char *dispatcherPortName,
                                   const char *hardwareId, DasPacket::ModuleType hardwareType,
                                   bool behindDsp, int blocking, int numParams,
                                   int interfaceMask, int interruptMask)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_BASEMODULEPLUGIN_PARAMS + numParams, 1,
                 interfaceMask | defaultInterfaceMask, interruptMask | defaultInterruptMask)
    , m_hardwareId(ip2addr(hardwareId))
    , m_hardwareType(hardwareType)
    , m_waitingResponse(static_cast<DasPacket::CommandType>(0))
    , m_expectedChannel(0)
    , m_numChannels(0)
    , m_behindDsp(behindDsp)
{
    createParam("CmdRsp",       asynParamInt32, &CmdRsp,    LAST_CMD_NONE); // READ - Last command response status   (see BaseModulePlugin::LastCommandResponse)
    createParam("CmdReq",       asynParamInt32, &CmdReq);                   // WRITE - Send command to module        (see DasPacket::CommandType)
    createParam("HwId",         asynParamOctet, &HwId);                     // READ - Connected module hardware id
    createParam("HwType",       asynParamInt32, &HwType, hardwareType);     // READ - Module type                    (see DasPacket::ModuleType)
    createParam("HwDate",       asynParamOctet, &HwDate);                   // READ - Module hardware date
    createParam("HwVer",        asynParamInt32, &HwVer);                    // READ - Module hardware version
    createParam("HwRev",        asynParamInt32, &HwRev);                    // READ - Module hardware revision
    createParam("HwExpectVer",  asynParamInt32, &HwExpectVer);              // READ - Module expected hardware version
    createParam("HwExpectRev",  asynParamInt32, &HwExpectRev);              // READ - Module expected hardware revision
    createParam("FwDate",       asynParamOctet, &FwDate);                   // READ - Module firmware date
    createParam("FwVer",        asynParamInt32, &FwVer);                    // READ - Module firmware version
    createParam("FwRev",        asynParamInt32, &FwRev);                    // READ - Module firmware revision
    createParam("FwExpectVer",  asynParamInt32, &FwExpectVer);              // READ - Module expected firmware version
    createParam("FwExpectRev",  asynParamInt32, &FwExpectRev);              // READ - Module expected firmware revision
    createParam("Supported",    asynParamInt32, &Supported);                // READ - Is requested module version supported (0=not supported,1=supported)
    createParam("Verified",     asynParamInt32, &Verified, 0);              // READ - Flag whether module type and version were verified
    createParam("CfgSection",   asynParamInt32, &CfgSection, 0x0);          // WRITE - Select configuration section to be written with next WRITE_CONFIG request, 0 for all
    createParam("CfgChannel",   asynParamInt32, &CfgChannel, 0);            // WRITE - Select channel to be configured, 0 means main configuration
    createParam("NoRspTimeout", asynParamFloat64, &NoRspTimeout, NO_RESPONSE_TIMEOUT); // WRITE - Time to wait for response

    std::string hardwareIp = addr2ip(m_hardwareId);
    setStringParam(HwId, hardwareIp.c_str());
    setIntegerParam(CmdRsp, LAST_CMD_NONE);
    callParamCallbacks();

    g_namesMap[ip2addr(hardwareId)] = portName;
}

BaseModulePlugin::~BaseModulePlugin()
{}

void BaseModulePlugin::setNumChannels(uint32_t n)
{
    assert(m_numChannels == 0); // Prevent running multiple times
    assert(n <= 16);
    m_numChannels = n;
}

asynStatus BaseModulePlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == CmdReq) {
        if (!processRequest(static_cast<DasPacket::CommandType>(value)))
            return asynError;
        return asynSuccess;
    }
    if (pasynUser->reason == CfgSection) {
        if (value < 0x0 || value > 0xF) {
            LOG_ERROR("Invalid configuration section %d", value);
            return asynError;
        }
        setIntegerParam(CfgSection, value);
        callParamCallbacks();
        return asynSuccess;
    }
    // Not a command, it's probably one of the writeable parameters
    for (auto it = m_params.begin(); it != m_params.end(); it++) {
        if (it->second.readonly)
            continue;

        std::map<int, struct ParamDesc>::iterator jt = it->second.mapping.find(pasynUser->reason);
        if (jt == it->second.mapping.end())
            continue;

        if (jt->second.convert->checkBounds(value, jt->second.width) == false) {
            LOG_ERROR("Parameter %s value %d out of bounds", getParamName(jt->first), value);
            return asynError;
        } else {
            setIntegerParam(jt->first, value);
            callParamCallbacks();
            return asynSuccess;
        }
    }

    // Just issue default handler to see if it can handle it
    return BasePlugin::writeInt32(pasynUser, value);
}

bool BaseModulePlugin::processRequest(DasPacket::CommandType command)
{
    double timeout;
    getDoubleParam(NoRspTimeout, &timeout);

    if (m_waitingResponse != 0) {
        LOG_WARN("Command '0x%02X' not allowed while waiting for 0x%02X response", command, m_waitingResponse);
        return false;
    }

    m_expectedChannel = 0;
    m_cfgSectionCnt = 0; // used to correctly report CmdRsp when 0 channel succeeds but other channels don't have registers in particular section

    setIntegerParam(CfgChannel, m_expectedChannel);
    setIntegerParam(CmdRsp, LAST_CMD_WAIT);
    callParamCallbacks();

    do {
        m_waitingResponse = handleRequest(command, timeout);

        if (m_waitingResponse != static_cast<DasPacket::CommandType>(0)) {
            if (!scheduleTimeoutCallback(m_waitingResponse, timeout))
               LOG_WARN("Failed to schedule CmdRsp timeout callback");
            setIntegerParam(CmdRsp, LAST_CMD_WAIT);

            // Increase this for all packets, although only used in rspWriteConfig()
            m_cfgSectionCnt++;
        } else {
            // No such section for non-channel command, are the more channels to try?
            m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);
            if (m_expectedChannel > 0) {
                continue;
            }
            setIntegerParam(CmdRsp, LAST_CMD_SKIPPED);
        }
        break; // Needs `break; } while(1)' because of continue
    } while (1);

    callParamCallbacks();

    return true;
}

DasPacket::CommandType BaseModulePlugin::handleRequest(DasPacket::CommandType command, double &timeout)
{
    int cfgSection;
    switch (command) {
    case DasPacket::CMD_RESET:
        LOG_INFO("Sending %s command", cmd2str(command));
        timeout = RESET_NO_RESPONSE_TIMEOUT;
        return reqReset();
    case DasPacket::CMD_RESET_LVDS:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqResetLvds();
    case DasPacket::CMD_TC_RESET:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqTcReset();
    case DasPacket::CMD_TC_RESET_LVDS:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqTcResetLvds();
    case DasPacket::CMD_DISCOVER:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqDiscover();
    case DasPacket::CMD_READ_VERSION:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqReadVersion();
    case DasPacket::CMD_READ_STATUS:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqReadStatus(m_expectedChannel);
    case DasPacket::CMD_READ_STATUS_COUNTERS:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqReadStatusCounters();
    case DasPacket::CMD_RESET_STATUS_COUNTERS:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqResetStatusCounters();
    case DasPacket::CMD_READ_CONFIG:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqReadConfig(m_expectedChannel);
    case DasPacket::CMD_WRITE_CONFIG:
        getIntegerParam(CfgSection, &cfgSection);
        command = (DasPacket::CommandType)((int)command + cfgSection);
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqWriteConfig(cfgSection, m_expectedChannel);
    case DasPacket::CMD_START:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqStart();
    case DasPacket::CMD_STOP:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqStop();
    case DasPacket::CMD_UPGRADE:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqUpgrade();
    case DasPacket::CMD_READ_TEMPERATURE:
        LOG_INFO("Sending %s command", cmd2str(command));
        return reqReadTemperature();
    default:
        LOG_ERROR("Skip unrecognized 0x%02X command", command);
        return static_cast<DasPacket::CommandType>(0);
    }
}

void BaseModulePlugin::sendToDispatcher(DasPacket::CommandType command, uint8_t channel, uint32_t *payload, uint32_t length)
{
    DasPacket *packet;
    if (m_behindDsp)
        packet = DasPacket::createLvds(DasPacket::HWID_SELF, m_hardwareId, command, channel, length, payload);
    else
        packet = DasPacket::createOcc(DasPacket::HWID_SELF, m_hardwareId, command, channel, length, payload);

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
    // Handle internal state machines first
    for (auto it=m_stateMachines.begin(); it!=m_stateMachines.end(); it++) {
        if ((*it)(packet) == true)
            return true;
    }

    DasPacket::CommandType command = packet->getResponseType();
    if (m_waitingResponse != command) {
        LOG_WARN("Ignoring unexpected response %s (0x%02X)", cmd2str(command), command);
        return false;
    }
    m_waitingResponse = static_cast<DasPacket::CommandType>(0);

    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received %s response after timeout", cmd2str(command));
        return false;
    }

    if (packet->cmdinfo.command == DasPacket::RSP_ACK)
        LOG_INFO("Received %s ACK", cmd2str(packet->getResponseType()));
    else if (packet->cmdinfo.command == DasPacket::RSP_NACK)
        LOG_INFO("Received %s NACK", cmd2str(packet->getResponseType()));
    else
        LOG_INFO("Received %s response", cmd2str(packet->getResponseType()));

    bool ack = handleResponse(packet);

    // What follows is a state machine for multiple channel commands.
    // Until there are more channels, re-issue the same command for next
    // channel in line. Don't update CmdRsp until the last channel or
    // when there was an error. Bail out on first error.

    // Turns back to 0 after last channel, works also for m_numChannels == 0
    m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);

    if (ack == false) {
        // Break sequence on any error, CfgChannel holds currently failed channel
        setIntegerParam(CmdRsp, LAST_CMD_ERROR);
    } else if (m_expectedChannel == 0) {
        // This is response for last channel in sequence or module doesn't support channels
        setIntegerParam(CmdRsp, LAST_CMD_OK);
    } else {
        bool processed = true;

        switch (command) {
        case DasPacket::CMD_READ_CONFIG:
            m_waitingResponse = reqReadConfig(m_expectedChannel);
            break;
        case DasPacket::CMD_READ_STATUS:
            m_waitingResponse = reqReadStatus(m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG:
            m_waitingResponse = reqWriteConfig(0, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_1:
            m_waitingResponse = reqWriteConfig(1, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_2:
            m_waitingResponse = reqWriteConfig(2, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_3:
            m_waitingResponse = reqWriteConfig(3, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_4:
            m_waitingResponse = reqWriteConfig(4, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_5:
            m_waitingResponse = reqWriteConfig(5, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_6:
            m_waitingResponse = reqWriteConfig(6, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_7:
            m_waitingResponse = reqWriteConfig(7, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_8:
            m_waitingResponse = reqWriteConfig(8, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_9:
            m_waitingResponse = reqWriteConfig(9, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_A:
            m_waitingResponse = reqWriteConfig(10, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_B:
            m_waitingResponse = reqWriteConfig(11, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_C:
            m_waitingResponse = reqWriteConfig(12, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_D:
            m_waitingResponse = reqWriteConfig(13, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_E:
            m_waitingResponse = reqWriteConfig(14, m_expectedChannel);
            break;
        case DasPacket::CMD_WRITE_CONFIG_F:
            m_waitingResponse = reqWriteConfig(15, m_expectedChannel);
            break;
        default:
            setIntegerParam(CmdRsp, LAST_CMD_OK);
            processed = false;
            break;
        }
        if (processed) {
            if (m_waitingResponse != static_cast<DasPacket::CommandType>(0)) {
                double timeout;
                getDoubleParam(NoRspTimeout, &timeout);
                if (!scheduleTimeoutCallback(m_waitingResponse, timeout))
                    LOG_WARN("Failed to schedule CmdRsp timeout callback");
                setIntegerParam(CmdRsp, LAST_CMD_WAIT);

                m_cfgSectionCnt++;
            } else if (m_cfgSectionCnt > 0) {
                setIntegerParam(CmdRsp, LAST_CMD_OK);
            } else {
                setIntegerParam(CmdRsp, LAST_CMD_SKIPPED);
            }
        }
    }

    callParamCallbacks();
    return true;
}

bool BaseModulePlugin::handleResponse(const DasPacket *packet)
{
    bool ack = false;
    DasPacket::CommandType command = packet->getResponseType();
    int verified;

    getIntegerParam(Verified, &verified);

    switch (command) {
    case DasPacket::CMD_RESET:
        return rspReset(packet);
    case DasPacket::CMD_RESET_LVDS:
        return rspResetLvds(packet);
    case DasPacket::CMD_TC_RESET:
        return rspTcReset(packet);
    case DasPacket::CMD_TC_RESET_LVDS:
        return rspTcResetLvds(packet);
    case DasPacket::CMD_DISCOVER:
        ack = rspDiscover(packet);
        verified &= ~VERIFY_DISCOVER_MASK;
        verified |= (ack ? VERIFY_DISCOVER_OK : VERIFY_DISCOVER_FAIL);
        setIntegerParam(Verified, verified);
        return ack;
    case DasPacket::CMD_READ_VERSION:
        ack = rspReadVersion(packet);
        verified &= ~VERIFY_VERSION_MASK;
        verified |= (ack ? VERIFY_VERSION_OK : VERIFY_VERSION_FAIL);
        setIntegerParam(Verified, verified);
        return ack;
    case DasPacket::CMD_READ_CONFIG:
        if (! ((m_expectedChannel == 0 && packet->cmdinfo.is_channel == 0 && packet->cmdinfo.channel == 0) ||
               (packet->cmdinfo.is_channel == (m_expectedChannel > 0) && packet->cmdinfo.channel == (m_expectedChannel - 1))) ) {
            LOG_ERROR("Expecting read config response for channel %d, got for channel %d\n", m_expectedChannel, packet->cmdinfo.channel + 1);
            return false;
        }
        return rspReadConfig(packet, m_expectedChannel);
    case DasPacket::CMD_READ_STATUS:
        if (! ((m_expectedChannel == 0 && packet->cmdinfo.is_channel == 0 && packet->cmdinfo.channel == 0) ||
               (packet->cmdinfo.is_channel == (m_expectedChannel > 0) && packet->cmdinfo.channel == (m_expectedChannel - 1))) ) {
            LOG_ERROR("Expecting read status response for channel %d, got for channel %d\n", m_expectedChannel, packet->cmdinfo.channel + 1);
            return false;
        }
        return rspReadStatus(packet, m_expectedChannel);
    case DasPacket::CMD_READ_STATUS_COUNTERS:
        return rspReadStatusCounters(packet);
    case DasPacket::CMD_RESET_STATUS_COUNTERS:
        return rspResetStatusCounters(packet);
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
        if (! ((m_expectedChannel == 0 && packet->cmdinfo.is_channel == 0 && packet->cmdinfo.channel == 0) ||
               (packet->cmdinfo.is_channel == (m_expectedChannel > 0) && packet->cmdinfo.channel == (m_expectedChannel - 1))) ) {
            LOG_ERROR("Expecting write config response for channel %d, got for channel %d\n", m_expectedChannel, packet->cmdinfo.channel + 1);
            return false;
        }
        return rspWriteConfig(packet, m_expectedChannel);
    case DasPacket::CMD_START:
        return rspStart(packet);
    case DasPacket::CMD_STOP:
        return rspStop(packet);
    case DasPacket::CMD_UPGRADE:
        return rspUpgrade(packet);
    case DasPacket::CMD_READ_TEMPERATURE:
        return rspReadTemperature(packet);
    default:
        LOG_WARN("Unhandled %s response (0x%02X)", cmd2str(command), command);
        return false;
    }
}

DasPacket::CommandType BaseModulePlugin::reqReset()
{
    sendToDispatcher(DasPacket::CMD_RESET);
    return DasPacket::CMD_RESET;
}

bool BaseModulePlugin::rspReset(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqResetLvds()
{
    sendToDispatcher(DasPacket::CMD_RESET_LVDS);
    return DasPacket::CMD_RESET_LVDS;
}

bool BaseModulePlugin::rspResetLvds(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqTcReset()
{
    sendToDispatcher(DasPacket::CMD_TC_RESET);
    return DasPacket::CMD_TC_RESET;
}

bool BaseModulePlugin::rspTcReset(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqTcResetLvds()
{
    sendToDispatcher(DasPacket::CMD_TC_RESET_LVDS);
    return DasPacket::CMD_TC_RESET_LVDS;
}

bool BaseModulePlugin::rspTcResetLvds(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqDiscover()
{
    sendToDispatcher(DasPacket::CMD_DISCOVER);
    return DasPacket::CMD_DISCOVER;
}

bool BaseModulePlugin::rspDiscover(const DasPacket *packet)
{
    return (m_hardwareType == packet->cmdinfo.module_type);
}

DasPacket::CommandType BaseModulePlugin::reqReadVersion()
{
    sendToDispatcher(DasPacket::CMD_READ_VERSION);
    return DasPacket::CMD_READ_VERSION;
}

bool BaseModulePlugin::rspReadVersion(const DasPacket *packet)
{
    char date[20];
    Version version;

    if (!parseVersionRspM(packet, version)) {
        LOG_WARN("Bad READ_VERSION response");
        return false;
    }

    setIntegerParam(HwVer, version.hw_version);
    setIntegerParam(HwRev, version.hw_revision);
    snprintf(date, sizeof(date), "%04d/%02d/%02d", version.hw_year, version.hw_month, version.hw_day);
    setStringParam(HwDate, date);
    setIntegerParam(FwVer, version.fw_version);
    setIntegerParam(FwRev, version.fw_revision);
    snprintf(date, sizeof(date), "%04d/%02d/%02d", version.fw_year, version.fw_month, version.fw_day);
    setStringParam(FwDate, date);

    callParamCallbacks();

    return checkVersion(version);
}

DasPacket::CommandType BaseModulePlugin::reqReadStatus(uint8_t channel)
{
    // We can be called from anywhere, verify parameter
    if (channel > m_numChannels) {
        LOG_ERROR("Can not read channel %u configuration, module only supports %u channels", channel, m_numChannels);
        return static_cast<DasPacket::CommandType>(0);
    }

    if (m_params["STATUS"].mapping.empty()) {
        // Detector has no status parameters
        return static_cast<DasPacket::CommandType>(0);
    }

    sendToDispatcher(DasPacket::CMD_READ_STATUS, channel);
    return DasPacket::CMD_READ_STATUS;
}

bool BaseModulePlugin::rspReadStatus(const DasPacket *packet, uint8_t channel)
{
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    uint32_t section = SECTION_ID(0x0, channel);
    uint32_t length = ALIGN_UP(m_params["STATUS"].sizes[section]*wordsize, 4);
    if (packet->getPayloadLength() != length) {
        LOG_ERROR("Received wrong READ_STATUS response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), length);
        return false;
    }

    unpackRegParams("STATUS", packet->getPayload(), packet->getPayloadLength(), channel);
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadStatusCounters()
{
    if (m_params["COUNTERS"].mapping.empty()) {
        // Detector has no status parameters
        return static_cast<DasPacket::CommandType>(0);
    }

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
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

bool BaseModulePlugin::rspReadStatusCounters(const DasPacket *packet)
{
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    uint32_t length = ALIGN_UP(m_params["COUNTERS"].sizes[0]*wordsize, 4);
    if (packet->getPayloadLength() != length) {
        LOG_ERROR("Received wrong READ_STATUS_COUNTERS response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), length);
        return false;
    }

    unpackRegParams("COUNTERS", packet->getPayload(), packet->getPayloadLength());
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadConfig(uint8_t channel)
{
    // We can be called from anywhere, verify parameter
    if (channel > m_numChannels) {
        LOG_ERROR("Can not read channel %u configuration, module only supports %u channels", channel, m_numChannels);
        return static_cast<DasPacket::CommandType>(0);
    }

    if (m_params["CONFIG"].mapping.empty()) {
        // Detector has no configuration parameters
        return static_cast<DasPacket::CommandType>(0);
    }

    sendToDispatcher(DasPacket::CMD_READ_CONFIG, channel);
    return DasPacket::CMD_READ_CONFIG;
}

bool BaseModulePlugin::rspReadConfig(const DasPacket *packet, uint8_t channel)
{
    // Response contains registers for all sections for a selected channel or global configuration
    uint32_t section_f = SECTION_ID(0xF, channel);
    uint32_t length = m_params["CONFIG"].offsets[section_f] + m_params["CONFIG"].sizes[section_f];
    int wordsize = (m_behindDsp ? 2 : 4);
    length *= wordsize;

    if (packet->getPayloadLength() != ALIGN_UP(length, 4)) {
        LOG_ERROR("Received wrong READ_CONFIG response based on length; received %uB, expected %uB",
                  packet->getPayloadLength(), ALIGN_UP(length, 4));
        return false;
    }

    unpackRegParams("CONFIG", packet->getPayload(), packet->getPayloadLength(), channel);
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqWriteConfig(uint8_t section, uint8_t channel)
{
    uint32_t data[1024];
    size_t len = packRegParams("CONFIG", data, sizeof(data), channel, section);
    if (len == 0) {
        LOG_WARN("Skipping sending write config packet");
        return static_cast<DasPacket::CommandType>(0);
    }

    DasPacket::CommandType rsp;
    switch (section) {
        case 0x0: rsp = DasPacket::CMD_WRITE_CONFIG;   break;
        case 0x1: rsp = DasPacket::CMD_WRITE_CONFIG_1; break;
        case 0x2: rsp = DasPacket::CMD_WRITE_CONFIG_2; break;
        case 0x3: rsp = DasPacket::CMD_WRITE_CONFIG_3; break;
        case 0x4: rsp = DasPacket::CMD_WRITE_CONFIG_4; break;
        case 0x5: rsp = DasPacket::CMD_WRITE_CONFIG_5; break;
        case 0x6: rsp = DasPacket::CMD_WRITE_CONFIG_6; break;
        case 0x7: rsp = DasPacket::CMD_WRITE_CONFIG_7; break;
        case 0x8: rsp = DasPacket::CMD_WRITE_CONFIG_8; break;
        case 0x9: rsp = DasPacket::CMD_WRITE_CONFIG_9; break;
        case 0xA: rsp = DasPacket::CMD_WRITE_CONFIG_A; break;
        case 0xB: rsp = DasPacket::CMD_WRITE_CONFIG_B; break;
        case 0xC: rsp = DasPacket::CMD_WRITE_CONFIG_C; break;
        case 0xD: rsp = DasPacket::CMD_WRITE_CONFIG_D; break;
        case 0xE: rsp = DasPacket::CMD_WRITE_CONFIG_E; break;
        case 0xF: rsp = DasPacket::CMD_WRITE_CONFIG_F; break;
        default:
            return static_cast<DasPacket::CommandType>(0);
    }
    sendToDispatcher(rsp, channel, data, len);
    return rsp;
}

bool BaseModulePlugin::rspWriteConfig(const DasPacket *packet, uint8_t channel)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqStart()
{
    sendToDispatcher(DasPacket::CMD_START);
    return DasPacket::CMD_START;
}

bool BaseModulePlugin::rspStart(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqStop()
{
    sendToDispatcher(DasPacket::CMD_STOP);
    return DasPacket::CMD_STOP;
}

bool BaseModulePlugin::rspStop(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqUpgrade(const char *data, uint32_t size)
{
    if (data == 0 || size == 0) {
        data = 0;
        size = 0;
    }
    sendToDispatcher(DasPacket::CMD_UPGRADE, 0, (uint32_t*)data, size);
    return DasPacket::CMD_UPGRADE;
}

bool BaseModulePlugin::rspUpgrade(const DasPacket *packet)
{
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    uint32_t length = ALIGN_UP(m_params["UPGRADE"].sizes[0]*wordsize, 4);
    if (packet->getPayloadLength() != ALIGN_UP(length, 4)) {
        LOG_ERROR("Received wrong READ_UPGRADE response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), length);
        return false;
    }

    unpackRegParams("UPGRADE", packet->getPayload(), packet->getPayloadLength());
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadTemperature()
{
    if (m_params["TEMPERATURE"].mapping.empty()) {
        // Detector has no configuration parameters
        return static_cast<DasPacket::CommandType>(0);
    }

    sendToDispatcher(DasPacket::CMD_READ_TEMPERATURE);
    return DasPacket::CMD_READ_TEMPERATURE;
}

bool BaseModulePlugin::rspReadTemperature(const DasPacket *packet)
{
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    uint32_t length = ALIGN_UP(m_params["TEMPERATURE"].sizes[0]*wordsize, 4);
    if (packet->getPayloadLength() != ALIGN_UP(length, 4)) {
        LOG_ERROR("Received wrong READ_TEMP response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), length);
        return false;
    }

    unpackRegParams("TEMPERATURE", packet->getPayload(), packet->getPayloadLength());
    return true;
}

void BaseModulePlugin::createStatusParam(const char *name, uint8_t channel, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    createRegParam("STATUS", name, true, channel, 0x0, offset, nBits, shift, 0);
}

void BaseModulePlugin::createCounterParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    createRegParam("COUNTERS", name, true, 0, 0x0, offset, nBits, shift, 0);
}

void BaseModulePlugin::createChanConfigParam(const char *name, uint8_t channel, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value, const BaseConvert *conv)
{
    if (section >= '1' && section <= '9')
        section = section - '1' + 1;
    else if (section >= 'A' && section <= 'F')
        section = section - 'A' + 0xA;
    else {
        LOG_ERROR("Invalid section '%c' specified for parameter '%s'", section, name);
        return;
    }

    createRegParam("CONFIG", name, false, channel, section, offset, nBits, shift, value, conv);
}

void BaseModulePlugin::createMetaConfigParam(const char *name, uint32_t nBits, int value, const BaseConvert *conv)
{
    createRegParam("META", name, false, 0, 0, 0, nBits, 0, value, conv);
}

void BaseModulePlugin::createTempParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, const BaseConvert *conv)
{
    createRegParam("TEMPERATURE", name, true, 0, 0x0, offset, nBits, shift, 0, conv);
}

void BaseModulePlugin::createUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, const BaseConvert *conv)
{
    createRegParam("UPGRADE", name, true, 0, 0x0, offset, nBits, shift, 0, conv);
}

void BaseModulePlugin::linkUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    linkRegParam("UPGRADE", name, true, 0, 0x0, offset, nBits, shift);
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
        LOG_WARN("Timeout waiting for %s response", cmd2str(command));
        m_waitingResponse = static_cast<DasPacket::CommandType>(0);
        setIntegerParam(CmdRsp, LAST_CMD_TIMEOUT);
        callParamCallbacks();
    }
    return 0;
}

bool BaseModulePlugin::scheduleTimeoutCallback(DasPacket::CommandType command, double delay)
{
    std::function<float(void)> timeoutCb = std::bind(&BaseModulePlugin::noResponseCleanup, this, command);
    m_timeoutTimer = scheduleCallback(timeoutCb, delay);
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
    for (uint32_t channel = 0; channel <= m_numChannels; channel++) {
        // Calculate section offsets
        m_configSectionOffsets[SECTION_ID(0x1, channel)] = 0x0;
        for (uint8_t section=0x2; section<=0xF; section++) {
            uint32_t currSectionId = SECTION_ID(section, channel);
            uint32_t prevSectionId = SECTION_ID(section - 1, channel);
            m_configSectionOffsets[currSectionId] = m_configSectionOffsets[prevSectionId] + m_configSectionSizes[prevSectionId];
            LOG_WARN("Section 0x%01X channel %u size=%u offset=%u", section, channel, m_configSectionSizes[currSectionId], m_configSectionOffsets[currSectionId]);
        }
    }
}

size_t BaseModulePlugin::packRegParams(const char *group, uint32_t *payload, size_t size, uint8_t channel, uint8_t section)
{

    // We can be called from anywhere, verify parameter
    if (channel > m_numChannels) {
        LOG_ERROR("Module only supports %u channels", m_numChannels);
        return false;
    }

    uint32_t payloadLength;
    if (section == 0x0) {
        uint32_t section_f = SECTION_ID((m_params[group].sections ? 0xF : 0x0), channel);
        payloadLength = m_params[group].offsets[section_f] + m_params[group].sizes[section_f];
    } else {
        uint32_t sectionId = SECTION_ID(section, channel);
        payloadLength = m_params[group].sizes[sectionId];
    }
    int wordsize = (m_behindDsp ? 2 : 4);
    payloadLength *= wordsize;

    if (payloadLength > size) {
        LOG_ERROR("Buffer not big enough to put register data into");
        return false;
    }

    // Skip empty sections
    if (payloadLength == 0) {
        return false;
    }

    uint32_t length = ALIGN_UP(payloadLength, 4) / 4;
    for (uint32_t i=0; i<length; i++) {
        payload[i] = 0;
    }

    std::map<int, ParamDesc> &table = m_params[group].mapping;
    std::map<int, uint32_t> &offsets = m_params[group].offsets;
    for (auto it=table.begin(); it != table.end(); it++) {
        int shift = it->second.shift;
        int value = 0;
        uint32_t offset = it->second.offset;

        if (it->second.channel != channel) {
            continue;
        } else if (section == 0x0) {
            uint32_t sectionId = SECTION_ID(it->second.section, it->second.channel);
            offset += offsets[sectionId];
        } else if (section != it->second.section) {
            continue;
        }

        if (getIntegerParam(it->first, &value) != asynSuccess) {
            // This should not happen. It's certainly error when creating and parameters.
            LOG_ERROR("Failed to get parameter %s value", getParamName(it->first));
            return false;
        }

        if (m_behindDsp) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }

        if (offset >= length) {
            // Unlikely, but rather sure than sorry
            LOG_ERROR("Parameter %s offset out of range", getParamName(it->first));
            continue;
        }

        value = it->second.convert->toRaw(value, it->second.width);
        payload[offset] |= value << shift;
        if ((it->second.width + shift) > 32) {
            payload[offset+1] |= value >> (it->second.width -(32 - shift));
        }
    }

    return payloadLength;
}

void BaseModulePlugin::unpackRegParams(const char *group, const uint32_t *payload, size_t size, uint8_t channel)
{
    std::map<int, ParamDesc> &table = m_params[group].mapping;
    std::map<int, uint32_t> &offsets = m_params[group].offsets;

    for (auto it=table.begin(); it != table.end(); it++) {
        int shift = it->second.shift;
        int offset = it->second.offset;

        // Skip parameters that are not included in this response
        if (it->second.channel != channel)
            continue;

        // Handle configuration parameters
        if (it->second.section != 0) {

            auto jt = offsets.find(SECTION_ID(it->second.section, it->second.channel));
            if (jt != offsets.end()) {
                offset += jt->second;
            }
        }

        if (m_behindDsp) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }
        int value = payload[offset] >> shift;
        if ((shift + it->second.width) > 32) {
            value |= payload[offset + 1] << (32 - shift);
        }
        value &= (0x1ULL << it->second.width) - 1;
        value = it->second.convert->fromRaw(value, it->second.width);
        setIntegerParam(it->first, value);
    }
    callParamCallbacks();
}

void BaseModulePlugin::createRegParam(const char *group, const char *name, bool readonly, uint8_t channel, uint8_t section, uint16_t offset, uint8_t nBits, uint8_t shift, uint32_t value, const BaseConvert *conv)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("%s parameter '%s' cannot be created (already exist?)", group, name);
        return;
    }
    setIntegerParam(index, value);

    if (m_params.find(group) == m_params.end()) {
        m_params[group].readonly = true;
        m_params[group].sections = false;
    }

    ParamDesc desc;
    desc.section = section;
    desc.channel = channel;
    desc.initVal = value;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    desc.convert.reset(conv);
    m_params[group].mapping[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;

    uint32_t sectionId = SECTION_ID(section, channel);
    m_params[group].sizes[sectionId] = std::max(m_params[group].sizes[sectionId], length);

    if (readonly == false)
        m_params[group].readonly = false;
    if (section != 0x0)
        m_params[group].sections = true;
}

void BaseModulePlugin::linkRegParam(const char *group, const char *name, bool readonly, uint8_t channel, uint8_t section, uint16_t offset, uint8_t nBits, uint8_t shift)
{
    int index;
    if (findParam(name, &index) != asynSuccess) {
        LOG_ERROR("Can not find existing parameter '%s' to be linked", name);
        return;
    }

    if (m_params.find(group) == m_params.end()) {
        m_params[group].readonly = true;
        m_params[group].sections = false;
    }

    ParamDesc desc;
    desc.section = section;
    desc.channel = channel;
    desc.initVal = 0;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    desc.convert.reset(CONV_UNSIGN);
    m_params[group].mapping[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    length *= wordsize;

    uint32_t sectionId = SECTION_ID(section, channel);
    m_params[group].sizes[sectionId] = std::max(m_params[group].sizes[sectionId], length);

    if (readonly == false)
        m_params[group].readonly = false;
    if (section != 0x0)
        m_params[group].sections = true;
}

void BaseModulePlugin::initParams()
{
    // Go through all groups and recalculate sections sizes and offsets
    for (auto it = m_params.begin(); it != m_params.end(); it++) {

        std::map<int, uint32_t> &sizes = it->second.sizes;
        std::map<int, uint32_t> &offsets = it->second.offsets;

        // Section sizes have already been calculated in createConfigParams()
        for (uint32_t channel = 0; channel <= m_numChannels; channel++) {

            if (it->second.sections == true) {
                // Calculate section offsets
                offsets[SECTION_ID(0x1, channel)] = 0x0;
                for (uint8_t section=0x2; section<=0xF; section++) {
                    uint32_t currSectionId = SECTION_ID(section, channel);
                    uint32_t prevSectionId = SECTION_ID(section - 1, channel);
                    offsets[currSectionId] = offsets[prevSectionId] + sizes[prevSectionId];
                    LOG_DEBUG("Section 0x%01X channel %u size=%u offset=%u", section, channel, sizes[currSectionId], offsets[currSectionId]);
                }
            }
        }
    }
}

const char *BaseModulePlugin::cmd2str(const DasPacket::CommandType cmd)
{
    switch (cmd) {
    case DasPacket::CMD_READ_VERSION:
        return "READ_VERSION";
    case DasPacket::CMD_READ_CONFIG:
        return "READ_CONFIG";
    case DasPacket::CMD_READ_STATUS:
        return "READ_STATUS";
    case DasPacket::CMD_READ_TEMPERATURE:
        return "READ_TEMPERATURE";
    case DasPacket::CMD_READ_STATUS_COUNTERS:
        return "READ_COUNTERS";
    case DasPacket::CMD_RESET_STATUS_COUNTERS:
        return "RESET_COUNTERS";
    case DasPacket::CMD_WRITE_CONFIG:
        return "WRITE_CONFIG";
    case DasPacket::CMD_WRITE_CONFIG_1:
        return "WRITE_CONFIG_1";
    case DasPacket::CMD_WRITE_CONFIG_2:
        return "WRITE_CONFIG_2";
    case DasPacket::CMD_WRITE_CONFIG_3:
        return "WRITE_CONFIG_3";
    case DasPacket::CMD_WRITE_CONFIG_4:
        return "WRITE_CONFIG_4";
    case DasPacket::CMD_WRITE_CONFIG_5:
        return "WRITE_CONFIG_5";
    case DasPacket::CMD_WRITE_CONFIG_6:
        return "WRITE_CONFIG_6";
    case DasPacket::CMD_WRITE_CONFIG_7:
        return "WRITE_CONFIG_7";
    case DasPacket::CMD_WRITE_CONFIG_8:
        return "WRITE_CONFIG_8";
    case DasPacket::CMD_WRITE_CONFIG_9:
        return "WRITE_CONFIG_9";
    case DasPacket::CMD_WRITE_CONFIG_A:
        return "WRITE_CONFIG_A";
    case DasPacket::CMD_WRITE_CONFIG_B:
        return "WRITE_CONFIG_B";
    case DasPacket::CMD_WRITE_CONFIG_C:
        return "WRITE_CONFIG_C";
    case DasPacket::CMD_WRITE_CONFIG_D:
        return "WRITE_CONFIG_D";
    case DasPacket::CMD_WRITE_CONFIG_E:
        return "WRITE_CONFIG_E";
    case DasPacket::CMD_WRITE_CONFIG_F:
        return "WRITE_CONFIG";
    case DasPacket::CMD_HV_SEND:
        return "HV_SEND";
    case DasPacket::CMD_HV_RECV:
        return "HV_RECV";
    case DasPacket::CMD_EEPROM_ERASE:
        return "EEPROM_ERASE";
    case DasPacket::CMD_EEPROM_LOAD:
        return "EEPROM_LOAD";
    case DasPacket::CMD_EEPROM_READ:
        return "EEPROM_READ";
    case DasPacket::CMD_EEPROM_WRITE:
        return "EEPROM_WRITE";
    case DasPacket::CMD_EEPROM_READ_WORD:
        return "EEPROM_READ_WORD";
    case DasPacket::CMD_EEPROM_WRITE_WORD:
        return "EEPROM_WRITE_WORD";
    case DasPacket::CMD_UPGRADE:
        return "UPGRADE";
    case DasPacket::CMD_DISCOVER:
        return "DISCOVER";
    case DasPacket::CMD_RESET:
        return "RESET";
    case DasPacket::CMD_RESET_LVDS:
        return "RESET_LVDS";
    case DasPacket::CMD_TC_RESET:
        return "T&C_RESET";
    case DasPacket::CMD_TC_RESET_LVDS:
        return "T&C_RESET_LVDS";
    case DasPacket::CMD_START:
        return "START";
    case DasPacket::CMD_STOP:
        return "STOP";
    case DasPacket::CMD_TSYNC:
        return "TSYNC";
    case DasPacket::CMD_RTDL:
        return "RTDL";
    case DasPacket::CMD_PM_PULSE_RQST_ON:
        return "PM_PULSE_ON";
    case DasPacket::CMD_PM_PULSE_RQST_OFF:
        return "PM_PULSE_OFF";
    case DasPacket::CMD_PREAMP_TEST_CONFIG:
        return "PREAMP_TEST_CONFIG";
    case DasPacket::CMD_PREAMP_TEST_TRIGGER:
        return "PREAMP_TEST_TRIGGER";
    default:
        return "<unknown>";
    }
}

void BaseModulePlugin::registerResponseHandler(std::function<bool(const DasPacket *)> &callback)
{
    m_stateMachines.push_back(callback);
}

bool BaseModulePlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    int hw_version;
    int hw_revision;
    int fw_version;
    int fw_revision;
    getIntegerParam(HwExpectVer, &hw_version);
    getIntegerParam(HwExpectRev, &hw_revision);
    getIntegerParam(FwExpectVer, &fw_version);
    getIntegerParam(FwExpectRev, &fw_revision);

    if (fw_version != 0 && static_cast<uint8_t>(fw_version) != version.fw_version) {
        LOG_ERROR("Expecting firmware version %d, module returned %u", fw_version, version.fw_version);
        return false;
    }
    if (fw_revision != 0 && static_cast<uint8_t>(fw_revision) != version.fw_revision) {
        LOG_ERROR("Expecting firmware revision %d, module returned %u", fw_revision, version.fw_revision);
        return false;
    }
    if (hw_version != 0 && static_cast<uint8_t>(hw_version) != version.hw_version) {
        LOG_ERROR("Expecting hardware version %d, module returned %u", hw_version, version.hw_version);
        return false;
    }
    if (hw_revision != 0 && static_cast<uint8_t>(hw_revision) != version.hw_revision) {
        LOG_ERROR("Expecting hardware revision %d, module returned %u", hw_revision, version.hw_revision);
        return false;
    }
    return true;
}

void BaseModulePlugin::setExpectedVersion(uint8_t fw_version, uint8_t fw_revision, uint8_t hw_version, uint8_t hw_revision)
{
    setIntegerParam(HwExpectVer, hw_version);
    setIntegerParam(HwExpectRev, hw_revision);
    setIntegerParam(FwExpectVer, fw_version);
    setIntegerParam(FwExpectRev, fw_revision);
}

std::string BaseModulePlugin::getModuleName(uint32_t hardwareId)
{
    auto name = g_namesMap.find(hardwareId);
    if (name != g_namesMap.end())
        return name->second;
    return "";
}
