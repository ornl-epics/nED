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

/**
 * Return a unique id for section and channel pair that can be used to
 * identify the pair in m_configSectionSizes and m_configSectionOffsets
 */
#define SECTION_ID(section, channel) (((channel) * 0x10) + ((section) & 0xF))

const float BaseModulePlugin::NO_RESPONSE_TIMEOUT = 2.0;

BaseModulePlugin::BaseModulePlugin(const char *portName, const char *dispatcherPortName,
                                   const char *hardwareId, DasPacket::ModuleType hardwareType,
                                   bool behindDsp, int blocking, int numParams,
                                   int interfaceMask, int interruptMask)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_BASEMODULEPLUGIN_PARAMS + numParams, 1,
                 interfaceMask | defaultInterfaceMask, interruptMask | defaultInterruptMask)
    , m_hardwareId(ip2addr(hardwareId))
    , m_hardwareType(hardwareType)
    , m_countersPayloadLength(0)
    , m_upgradePayloadLength(0)
    , m_temperaturePayloadLength(0)
    , m_verifySM(ST_TYPE_VERSION_INIT)
    , m_waitingResponse(static_cast<DasPacket::CommandType>(0))
    , m_configInitialized(false)
    , m_expectedChannel(0)
    , m_numChannels(0)
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
    createParam("HwType",       asynParamInt32, &HwType, hardwareType);     // READ - Module type                    (see DasPacket::ModuleType)
    createParam("HwDate",       asynParamOctet, &HwDate);                   // READ - Module hardware date
    createParam("HwVer",        asynParamInt32, &HwVer);                    // READ - Module hardware version
    createParam("HwRev",        asynParamInt32, &HwRev);                    // READ - Module hardware revision
    createParam("FwDate",       asynParamOctet, &FwDate);                   // READ - Module firmware date
    createParam("FwVer",        asynParamInt32, &FwVer);                    // READ - Module firmware version
    createParam("FwRev",        asynParamInt32, &FwRev);                    // READ - Module firmware revision
    createParam("Supported",    asynParamInt32, &Supported);                // READ - Is requested module version supported (0=not supported,1=supported)
    createParam("Verified",     asynParamInt32, &Verified);                 // READ - Flag whether module type and version were verified
    createParam("CfgSection",   asynParamInt32, &CfgSection, 0x0);          // WRITE - Select configuration section to be written with next WRITE_CONFIG request, 0 for all
    createParam("CfgChannel",   asynParamInt32, &CfgChannel, 0);            // WRITE - Select channel to be configured, 0 means main configuration
    createParam("UpgradeFile",  asynParamOctet, &UpgradeFile);              // WRITE - Path to the firmware file to be programmed
    createParam("UpgradePktSize",asynParamInt32,&UpgradePktSize, 256);      // WRITE - Maximum payload size for split program file transfer
    createParam("UpgradeStatus",asynParamInt32, &UpgradeStatus, UPGRADE_NOT_SUPPORTED); // READ -Remote upgrade status
    createParam("UpgradeSize",  asynParamInt32, &UpgradeSize, 0);           // READ - Total firmware size in bytes
    createParam("UpgradePos",   asynParamInt32, &UpgradePos, 0);            // READ - Bytes already sent to remote porty
    createParam("UpgradeAbort", asynParamInt32, &UpgradeAbort, 0);          // WRITE - Abort current upgrade sequence

    std::string hardwareIp = addr2ip(m_hardwareId);
    setStringParam(HwId, hardwareIp.c_str());
    setIntegerParam(CmdRsp, LAST_CMD_NONE);
    callParamCallbacks();

    m_remoteUpgrade.inProgress = false;
    m_remoteUpgrade.buffer = 0;
    m_remoteUpgrade.bufferSize = 0;
    m_remoteUpgrade.position = 0;

    m_statusPayloadLengths.push_back(0);
}

BaseModulePlugin::~BaseModulePlugin()
{}

void BaseModulePlugin::setNumChannels(uint32_t n)
{
    assert(m_numChannels == 0); // Prevent running multiple times
    assert(n <= 16);
    m_numChannels = n;

    // Only read status supports channels, read/write config is done differently
    m_statusPayloadLengths.resize(n + 1);
    for (uint32_t i = 1; i <= n; i++) {
        m_statusPayloadLengths[i] = 0;
    }
}

asynStatus BaseModulePlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int cfgSection;

    if (pasynUser->reason == CmdReq) {
        if (m_waitingResponse != 0) {
            LOG_WARN("Command '%d' not allowed while waiting for 0x%02X response", value, m_waitingResponse);
            return asynError;
        }

        m_expectedChannel = 0;
        m_cfgSectionCnt = 0; // used to correctly report CmdRsp when 0 channel succeeds but other channels don't have registers in particular section

        setIntegerParam(CfgChannel, m_expectedChannel);
        setIntegerParam(CmdRsp, LAST_CMD_WAIT);
        callParamCallbacks();

        do {
            switch (value) {
            case DasPacket::CMD_DISCOVER:
                m_waitingResponse = reqDiscover();
                break;
            case DasPacket::CMD_READ_VERSION:
                m_waitingResponse = reqReadVersion();
                break;
            case DasPacket::CMD_READ_STATUS:
                m_waitingResponse = reqReadStatus(m_expectedChannel);
                break;
            case DasPacket::CMD_READ_STATUS_COUNTERS:
                m_waitingResponse = reqReadStatusCounters();
                break;
            case DasPacket::CMD_RESET_STATUS_COUNTERS:
                m_waitingResponse = reqResetStatusCounters();
                break;
            case DasPacket::CMD_READ_CONFIG:
                m_waitingResponse = reqReadConfig(m_expectedChannel);
                break;
            case DasPacket::CMD_WRITE_CONFIG:
                getIntegerParam(CfgSection, &cfgSection);
                m_waitingResponse = reqWriteConfig(cfgSection, m_expectedChannel);
                break;
            case DasPacket::CMD_START:
                m_waitingResponse = reqStart();
                break;
            case DasPacket::CMD_STOP:
                m_waitingResponse = reqStop();
                break;
            case DasPacket::CMD_PM_PULSE_RQST_ON:
                m_waitingResponse = reqPulse();
                break;
            case DasPacket::CMD_PM_PULSE_RQST_OFF:
                m_waitingResponse = reqPulseClear();
                break;
            case DasPacket::CMD_UPGRADE:
                m_waitingResponse = reqUpgrade();
                break;
            case DasPacket::CMD_READ_TEMPERATURE:
                m_waitingResponse = reqReadTemperature();
                break;
            default:
                setIntegerParam(CmdRsp, LAST_CMD_ERROR);
                callParamCallbacks();
                LOG_WARN("Unrecognized '%d' command", SM_ACTION_CMD(value));
                return asynError;
            }

            if (m_waitingResponse != static_cast<DasPacket::CommandType>(0)) {
                if (!scheduleTimeoutCallback(m_waitingResponse, NO_RESPONSE_TIMEOUT))
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
    if (pasynUser->reason == UpgradePktSize) {
        // Enforce 4 bytes aligned transfers
        value = ALIGN_UP(value, 4);
        setIntegerParam(UpgradePktSize, value);
        callParamCallbacks();
        return BasePlugin::writeInt32(pasynUser, value);
    }
    if (pasynUser->reason == UpgradeAbort) {
        if (remoteUpgradeInProgress() == true) {
            remoteUpgradeStop();
            setIntegerParam(UpgradeStatus, UPGRADE_USER_ABORT);
            setIntegerParam(UpgradePos, 0);
            callParamCallbacks();
        }
        return asynSuccess;
    }

    // Not a command, it's probably the new configuration option
    std::map<int, struct ParamDesc>::iterator it = m_configParams.find(pasynUser->reason);
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

asynStatus BaseModulePlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == UpgradeFile) {
        uint32_t length;
        if (fileSize(std::string(value, nChars), length) == false)
            return asynError;

        *nActual = nChars;
        setIntegerParam(UpgradeSize, length);
        callParamCallbacks();
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
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
    bool ack = false;
    DasPacket::CommandType command = packet->getResponseType();

    if (m_waitingResponse != command) {
        LOG_WARN("Response '0x%02X' not allowed while waiting for 0x%02X", command, m_waitingResponse);
        return false;
    }
    m_waitingResponse = static_cast<DasPacket::CommandType>(0);

    // Check that channel from packet and expected channel number match
    if (! ((m_expectedChannel == 0 && packet->cmdinfo.is_channel == 0 && packet->cmdinfo.channel == 0) ||
           (packet->cmdinfo.is_channel == (m_expectedChannel > 0) && packet->cmdinfo.channel == (m_expectedChannel - 1))) ) {
        LOG_WARN("Expecting response for channel %d, got for channel %d\n", m_expectedChannel, packet->cmdinfo.channel + 1);
        setIntegerParam(CmdRsp, LAST_CMD_ERROR);
        callParamCallbacks();
        return false;
    }

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
        ack = rspReadConfig(packet, m_expectedChannel);
        break;
    case DasPacket::CMD_READ_STATUS:
        ack = rspReadStatus(packet, m_expectedChannel);
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
        ack = rspWriteConfig(packet, m_expectedChannel);
        break;
    case DasPacket::CMD_START:
        ack = rspStart(packet);
        break;
    case DasPacket::CMD_STOP:
        ack = rspStop(packet);
        break;
    case DasPacket::CMD_PM_PULSE_RQST_ON:
        ack = rspPulse(packet);
        break;
    case DasPacket::CMD_PM_PULSE_RQST_OFF:
        ack = rspPulseClear(packet);
        break;
    case DasPacket::CMD_UPGRADE:
        ack = rspUpgrade(packet);
        break;
    case DasPacket::CMD_READ_TEMPERATURE:
        ack = rspReadTemperature(packet);
        break;
    default:
        LOG_WARN("Received unhandled response 0x%02X", command);
        return false;
    }

    // What follows is a state machine for multiple channel commands.
    // Until there are more channels, re-issue the same command for next
    // channel in line. Don't update CmdRsp until the last channel or
    // when there was an error. Bail out on first error.

    // Turns back to 0 after last channel, works also for m_numChannels == 0
    m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);

    if (ack != LAST_CMD_OK) {
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
                if (!scheduleTimeoutCallback(m_waitingResponse, NO_RESPONSE_TIMEOUT))
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

    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_VERSION response after timeout");
        return false;
    }

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

    sendToDispatcher(DasPacket::CMD_READ_STATUS, channel);
    return DasPacket::CMD_READ_STATUS;
}

bool BaseModulePlugin::rspReadStatus(const DasPacket *packet, uint8_t channel)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_STATUS response after timeout");
        return false;
    }

    if (packet->getPayloadLength() != ALIGN_UP(m_statusPayloadLengths[channel], 4)) {
        LOG_ERROR("Received wrong READ_STATUS response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), ALIGN_UP(m_statusPayloadLengths[channel], 4));
        return false;
    }

    extractParams(packet, m_statusParams);
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

    extractParams(packet, m_counterParams);
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadConfig(uint8_t channel)
{
    // We can be called from anywhere, verify parameter
    if (channel > m_numChannels) {
        LOG_ERROR("Can not read channel %u configuration, module only supports %u channels", channel, m_numChannels);
        return static_cast<DasPacket::CommandType>(0);
    }

    sendToDispatcher(DasPacket::CMD_READ_CONFIG, channel);
    return DasPacket::CMD_READ_CONFIG;
}

bool BaseModulePlugin::rspReadConfig(const DasPacket *packet, uint8_t channel)
{
    int wordsize = (m_behindDsp ? 2 : 4);

    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_CONFIG response after timeout");
        return false;
    }

    if (!m_configInitialized) {
        recalculateConfigParams();
        m_configInitialized = true;
    }

    // Response contains registers for all sections for a selected channel or global configuration
    uint32_t section_f = SECTION_ID(0xF, channel);
    uint32_t length = m_configSectionOffsets[section_f] + m_configSectionSizes[section_f];
    length *= wordsize;

    if (packet->getPayloadLength() != ALIGN_UP(length, 4)) {
        LOG_ERROR("Received wrong READ_CONFIG response based on length; received %uB, expected %uB",
                  packet->getPayloadLength(), ALIGN_UP(length, 4));
        return false;
    }

    extractParams(packet, m_configParams, m_configSectionOffsets);
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqWriteConfig(uint8_t section, uint8_t channel)
{
    int wordsize = (m_behindDsp ? 2 : 4);

    if (!m_configInitialized) {
        recalculateConfigParams();
        m_configInitialized = true;
    }

    // We can be called from anywhere, verify parameter
    if (channel > m_numChannels) {
        LOG_ERROR("Can not configure channel %u, module only supports %u channels", channel, m_numChannels);
        return static_cast<DasPacket::CommandType>(0);
    }

    uint32_t payloadLength;
    if (section == 0x0) {
        uint32_t section_f = SECTION_ID(0xF, channel);
        payloadLength = m_configSectionOffsets[section_f] + m_configSectionSizes[section_f];
    } else {
        uint32_t sectionId = SECTION_ID(section, channel);
        payloadLength = m_configSectionSizes[sectionId];
    }
    payloadLength *= wordsize;

    // Skip empty sections
    if (payloadLength == 0) {
        return static_cast<DasPacket::CommandType>(0);
    }

    uint32_t length = ALIGN_UP(payloadLength, 4) / 4;
    uint32_t data[length]; // length doesn't exceed 256B, safe to put on stack
    for (uint32_t i=0; i<length; i++) {
        data[i] = 0;
    }

    for (auto it=m_configParams.begin(); it != m_configParams.end(); it++) {
        uint32_t mask = (0x1ULL << it->second.width) - 1;
        int shift = it->second.shift;
        int value = 0;
        uint32_t offset = it->second.offset;

        if (it->second.channel != channel) {
            continue;
        } else if (section == 0x0) {
            uint32_t sectionId = SECTION_ID(it->second.section, it->second.channel);
            offset += m_configSectionOffsets[sectionId];
        } else if (section != it->second.section) {
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
    sendToDispatcher(rsp, channel, data, payloadLength);
    return rsp;
}

bool BaseModulePlugin::rspWriteConfig(const DasPacket *packet, uint8_t channel)
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

DasPacket::CommandType BaseModulePlugin::reqPulse()
{
    sendToDispatcher(DasPacket::CMD_PM_PULSE_RQST_ON);
    return DasPacket::CMD_PM_PULSE_RQST_ON;
}

DasPacket::CommandType BaseModulePlugin::reqPulseClear()
{
    sendToDispatcher(DasPacket::CMD_PM_PULSE_RQST_OFF);
    return DasPacket::CMD_PM_PULSE_RQST_OFF;
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
        LOG_WARN("Received CMD_STOP response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

bool BaseModulePlugin::rspPulse(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received CMD_PM_PULSE_RQST_ON response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

bool BaseModulePlugin::rspPulseClear(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received CMD_PM_PULSE_RQST_OFF response after timeout");
        return false;
    }
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

DasPacket::CommandType BaseModulePlugin::reqUpgrade()
{
    if (remoteUpgradeInProgress() == false) {
        if (remoteUpgradeStart() == false) {
            setIntegerParam(UpgradeStatus, UPGRADE_INIT_FAILED);
            callParamCallbacks();
            return static_cast<DasPacket::CommandType>(0);
        }
    }

    setIntegerParam(UpgradeStatus, UPGRADE_IN_PROGRESS);
    callParamCallbacks();

    if (remoteUpgradeSend() == false) {
        // No more data to send, retain status
        return static_cast<DasPacket::CommandType>(0);
    }

    return DasPacket::CMD_UPGRADE;
}

bool BaseModulePlugin::rspUpgrade(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received CMD_UPGRADE response after timeout");
        return false;
    }
    if (packet->cmdinfo.command != DasPacket::RSP_ACK) {
        remoteUpgradeStop();
        setIntegerParam(UpgradeStatus, UPGRADE_CANCELED);
        setIntegerParam(UpgradePos, 0);
        callParamCallbacks();
        return false;
    }

    extractParams(packet, m_upgradeParams);
    return true;
}

DasPacket::CommandType BaseModulePlugin::reqReadTemperature()
{
    sendToDispatcher(DasPacket::CMD_READ_TEMPERATURE);
    return DasPacket::CMD_READ_TEMPERATURE;
}

bool BaseModulePlugin::rspReadTemperature(const DasPacket *packet)
{
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received READ_TEMPERATURE response after timeout");
        return false;
    }

    if (packet->getPayloadLength() != ALIGN_UP(m_temperaturePayloadLength, 4)) {
        LOG_ERROR("Received wrong READ_TEMP response based on length; "
                  "received %u, expected %u",
                  packet->getPayloadLength(), ALIGN_UP(m_temperaturePayloadLength, 4));
        return false;
    }

    extractParams(packet, m_temperatureParams);
    return true;
}

void BaseModulePlugin::createStatusParam(const char *name, uint8_t channel, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Status parameter '%s' cannot be created (already exist?)", name);
        return;
    }

    ParamDesc desc;
    desc.section = 0;
    desc.channel = channel;
    desc.initVal = 0;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    m_statusParams[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    m_statusPayloadLengths[channel] = std::max(m_statusPayloadLengths[channel], length*wordsize);
}

void BaseModulePlugin::createCounterParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Counter parameter '%s' cannot be created (already exist?)", name);
        return;
    }

    ParamDesc desc;
    desc.section = 0;
    desc.channel = 0;
    desc.initVal = 0;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    m_counterParams[index] = desc;

    uint32_t length = offset +1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    m_countersPayloadLength = std::max(m_countersPayloadLength, length*wordsize);
}

void BaseModulePlugin::createConfigParam(const char *name, uint8_t channel, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Config parameter '%s' cannot be created (already exist?)", name);
        return;
    }
    setIntegerParam(index, value);

    if (section >= '1' && section <= '9')
        section = section - '1' + 1;
    else if (section >= 'A' && section <= 'F')
        section = section - 'A' + 0xA;
    else {
        LOG_ERROR("Invalid section '%c' specified for parameter '%s'", section, name);
        return;
    }

    ParamDesc desc;
    desc.section = section;
    desc.channel = channel;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    desc.initVal = value;
    m_configParams[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    // m_configPayloadLength is calculated *after* we create all sections
    uint32_t sectionId = SECTION_ID(section, channel);
    m_configSectionSizes[sectionId] = std::max(m_configSectionSizes[sectionId], length);
}

void BaseModulePlugin::createTempParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    int index;
    if (createParam(name, asynParamInt32, &index) != asynSuccess) {
        LOG_ERROR("Temperature parameter '%s' cannot be created (already exist?)", name);
        return;
    }

    ParamDesc desc;
    desc.section = 0;
    desc.channel = 0;
    desc.initVal = 0;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    m_temperatureParams[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    m_temperaturePayloadLength = std::max(m_temperaturePayloadLength, length*wordsize);
}

void BaseModulePlugin::linkUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    int index;
    if (findParam(name, &index) != asynSuccess) {
        LOG_ERROR("Can not find existing parameter '%s' to be linked", name);
        return;
    }

    ParamDesc desc;
    desc.section = 0;
    desc.channel = 0;
    desc.initVal = 0;
    desc.offset  = offset;
    desc.shift   = shift;
    desc.width   = nBits;
    m_upgradeParams[index] = desc;

    uint32_t length = offset + 1;
    if (m_behindDsp && nBits > 16)
        length++;
    uint32_t wordsize = (m_behindDsp ? 2 : 4);
    m_upgradePayloadLength = std::max(m_upgradePayloadLength, length*wordsize);
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

        if (command == DasPacket::CMD_UPGRADE) {
            remoteUpgradeStop();
            setIntegerParam(UpgradeStatus, UPGRADE_CANCELED);
            setIntegerParam(UpgradePos, 0);
        }

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

bool BaseModulePlugin::remoteUpgradeStart() {

    if (m_remoteUpgrade.file.is_open())
        m_remoteUpgrade.file.close();

    char filepath[1024];
    if (getStringParam(UpgradeFile, sizeof(filepath), filepath) != asynSuccess) {
        LOG_WARN("No remote upgrade file provided");
        return false;
    }

    // Open the file
    m_remoteUpgrade.file.open(filepath, std::ifstream::in);
    if (m_remoteUpgrade.file.fail()) {
        LOG_ERROR("Can not open remote upgrade file '%s'", filepath);
        return false;
    }

    // Determine file size
    m_remoteUpgrade.file.seekg(0, m_remoteUpgrade.file.end);
    int length = m_remoteUpgrade.file.tellg();
    m_remoteUpgrade.file.seekg(0, m_remoteUpgrade.file.beg);

    // Get user defined packet size
    int chunkSize = 256; // should be good default
    if (getIntegerParam(UpgradePktSize, &chunkSize) != asynSuccess) {
        LOG_ERROR("No remote upgrade packet size defined");
        m_remoteUpgrade.file.close();
        return false;
    }
    chunkSize = ALIGN_UP(chunkSize, 4);

    // Allocate buffer if required
    if (chunkSize > m_remoteUpgrade.bufferSize) {
        m_remoteUpgrade.buffer = reinterpret_cast<uint32_t*>(realloc(m_remoteUpgrade.buffer, chunkSize));
        if (!m_remoteUpgrade.buffer) {
            LOG_ERROR("Can not allocate memory for programming");
            m_remoteUpgrade.bufferSize = 0;
            m_remoteUpgrade.file.close();
            return false;
        }
        m_remoteUpgrade.bufferSize = chunkSize;
    }

    m_remoteUpgrade.position = 0;
    setIntegerParam(UpgradeSize, length);
    setIntegerParam(UpgradePos, m_remoteUpgrade.position);
    setIntegerParam(UpgradePktSize, chunkSize);
    callParamCallbacks();

    m_remoteUpgrade.inProgress = true;
    return true;
}

bool BaseModulePlugin::remoteUpgradeSend()
{
    if (m_remoteUpgrade.file.good() == false)
        return false;

    m_remoteUpgrade.file.read(reinterpret_cast<char*>(m_remoteUpgrade.buffer), m_remoteUpgrade.bufferSize);
    int nRead = m_remoteUpgrade.file.gcount();

    if (nRead <= 0)
        return false;

    m_remoteUpgrade.position += nRead;
    setIntegerParam(UpgradePos, m_remoteUpgrade.position);
    callParamCallbacks();

    sendToDispatcher(DasPacket::CMD_UPGRADE, 0, m_remoteUpgrade.buffer, nRead);
    return true;
}

bool BaseModulePlugin::remoteUpgradeDone()
{
    return m_remoteUpgrade.file.eof();
}

bool BaseModulePlugin::remoteUpgradeInProgress()
{
    return m_remoteUpgrade.inProgress;
}

void BaseModulePlugin::remoteUpgradeStop()
{
    if (m_remoteUpgrade.file.is_open())
        m_remoteUpgrade.file.close();
    m_remoteUpgrade.inProgress = false;
}

void BaseModulePlugin::extractParams(const DasPacket *packet, const std::map<int, ParamDesc> &table, const std::map<int, uint32_t> &sectOffsets)
{
    const uint32_t *payload = packet->getPayload();
    for (auto it=table.begin(); it != table.end(); it++) {
        int shift = it->second.shift;
        int offset = it->second.offset;
        uint8_t channel = 0;

        if (packet->cmdinfo.is_channel)
            channel = packet->cmdinfo.channel + 1;

        // Skip parameters that are not included in this response
        if (it->second.channel != channel)
            continue;

        // Handle configuration parameters
        if (it->second.section != 0) {

            auto jt = sectOffsets.find(SECTION_ID(it->second.section, it->second.channel));
            if (jt != sectOffsets.end()) {
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
        setIntegerParam(it->first, value);
    }
    callParamCallbacks();
};
