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

#include <algorithm>
#include <alarm.h>
#include <chrono>
#include <dirent.h>
#include <map>
#include <osiSock.h>
#include <sys/stat.h>
#include <string.h>

// EPICS includes
#include <alarm.h>
#include <osiSock.h>

/**
 * Return a unique id for section and channel pair that can be used to
 * identify the pair in m_configSectionSizes and m_configSectionOffsets
 */
#define SECTION_ID(section, channel) (((channel) * 0x10) + ((section) & 0xF))

const UnsignConvert *BaseModulePlugin::CONV_UNSIGN = new UnsignConvert();
const Sign2sComplementConvert *BaseModulePlugin::CONV_SIGN_2COMP = new Sign2sComplementConvert();
const SignMagnitudeConvert *BaseModulePlugin::CONV_SIGN_MAGN = new SignMagnitudeConvert();
const Hex2DecConvert *BaseModulePlugin::CONV_HEX2DEC = new Hex2DecConvert();
const Hex2DecConvert2K *BaseModulePlugin::CONV_HEX2DEC2K = new Hex2DecConvert2K();

/**
 * This is a global map from hardwareId to plugin name. Whenever a module is
 * registered, it is added to this map. Luckily modules are registered on
 * startup only and this global variable is read only afterwards => no need
 * for thread safety mechanisms.
 */
static std::map<uint32_t, std::string> g_namesMap;
static epicsMutex g_namesMapMutex;

BaseModulePlugin::BaseModulePlugin(const char *portName, const char *parentPlugins,
                                   const char *configDir, uint8_t wordSize,
                                   int interfaceMask, int interruptMask)
    : BasePlugin(portName, 0, interfaceMask | defaultInterfaceMask, interruptMask | defaultInterruptMask)
    , m_features(0)
    , m_hardwareId(0)
    , m_waitingResponse(static_cast<DasCmdPacket::CommandType>(0))
    , m_wordSize(wordSize)
    , m_parentPlugins(parentPlugins)
{
    // DSP uses 4 byte words, everybody else 2 bytes
    assert(wordSize==2 || wordSize==4);

    // configDir must be existing writable directory!
    char tmpfile[1024];
    snprintf(tmpfile, 1024, "%s/XXXXXX", configDir);
    int fd = mkstemp(tmpfile);
    if (fd == -1) {
        LOG_ERROR("Configuration directory '%s' does not exist or is not writable directory!", configDir);
    } else {
        m_configDir = configDir;
        close(fd);
        unlink(tmpfile);
    }

    // Initialize command/response handler array - 256 slots based on DasCmdPacket::CommandType
    RequestHandler req;
    ResponseHandler rsp;
    for (auto i=0; i<256; i++) {
        m_cmdHandlers.emplace_back(std::make_pair(req, rsp));
    }

    // Register some common command handlers.
    // Other commands can be registered in module specific classes.
    m_cmdHandlers[DasCmdPacket::CMD_DISCOVER].first         = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_DISCOVER);
    m_cmdHandlers[DasCmdPacket::CMD_DISCOVER].second        = std::bind(&BaseModulePlugin::rspParams,       this, std::placeholders::_1, "DISCOVER");
    m_cmdHandlers[DasCmdPacket::CMD_READ_VERSION].first     = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_READ_VERSION);
    m_cmdHandlers[DasCmdPacket::CMD_READ_VERSION].second    = std::bind(&BaseModulePlugin::rspParams,       this, std::placeholders::_1, "VERSION");
    m_cmdHandlers[DasCmdPacket::CMD_START].first            = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_START);
    m_cmdHandlers[DasCmdPacket::CMD_START].second           = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_STOP].first             = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_STOP);
    m_cmdHandlers[DasCmdPacket::CMD_STOP].second            = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_RESET].first            = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_RESET);
    m_cmdHandlers[DasCmdPacket::CMD_RESET].second           = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_RESET_LVDS].first       = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_RESET_LVDS);
    m_cmdHandlers[DasCmdPacket::CMD_RESET_LVDS].second      = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_TC_RESET].first         = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_TC_RESET);
    m_cmdHandlers[DasCmdPacket::CMD_TC_RESET].second        = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_TC_RESET_LVDS].first    = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_TC_RESET_LVDS);
    m_cmdHandlers[DasCmdPacket::CMD_TC_RESET_LVDS].second   = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_WRITE_CONFIG].first     = std::bind(&BaseModulePlugin::reqParams,       this, DasCmdPacket::CMD_WRITE_CONFIG, "CONFIG");
    m_cmdHandlers[DasCmdPacket::CMD_WRITE_CONFIG].second    = std::bind(&BaseModulePlugin::rspWriteConfig,  this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].first      = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_READ_CONFIG);
    m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].second     = std::bind(&BaseModulePlugin::rspParams,       this, std::placeholders::_1, "CONFIG");
    m_cmdHandlers[DasCmdPacket::CMD_READ_STATUS].first      = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_READ_STATUS);
    m_cmdHandlers[DasCmdPacket::CMD_READ_STATUS].second     = std::bind(&BaseModulePlugin::rspParams,       this, std::placeholders::_1, "STATUS");
    m_cmdHandlers[DasCmdPacket::CMD_READ_COUNTERS].first    = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_READ_COUNTERS);
    m_cmdHandlers[DasCmdPacket::CMD_READ_COUNTERS].second   = std::bind(&BaseModulePlugin::rspParams,       this, std::placeholders::_1, "COUNTERS");
    m_cmdHandlers[DasCmdPacket::CMD_RESET_COUNTERS].first   = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_RESET_COUNTERS);
    m_cmdHandlers[DasCmdPacket::CMD_RESET_COUNTERS].second  = std::bind(&BaseModulePlugin::rspSimple,       this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_READ_TEMPERATURE].first = std::bind(&BaseModulePlugin::reqSimple,       this, DasCmdPacket::CMD_READ_TEMPERATURE);
    m_cmdHandlers[DasCmdPacket::CMD_READ_TEMPERATURE].second= std::bind(&BaseModulePlugin::rspParams,       this, std::placeholders::_1, "TEMPERATURE");

    createParam("Enable",       asynParamInt32, &Enable,    1);             // WRITE - Enables this module
    createParam("Features",     asynParamInt32, &Features, 0);              // READ - Features supported by this module
    createParam("CmdRsp",       asynParamInt32, &CmdRsp,    LAST_CMD_NONE); // READ - Last command response status   (see BaseModulePlugin::LastCommandResponse)
    createParam("CmdReq",       asynParamInt32, &CmdReq);                   // WRITE - Send command to module        (see DasCmdPacket::CommandType)
    createParam("HwId",         asynParamOctet, &HwId);                     // WRITE - Connected module hardware id
    createParam("CfgSection",   asynParamInt32, &CfgSection, 0x0);          // WRITE - Select configuration section to be written with next WRITE_CONFIG request, 0 for all
    createParam("CfgChannel",   asynParamInt32, &CfgChannel, 0);            // WRITE - Select channel to be configured, 0 means main configuration
    createParam("NoRspTimeout", asynParamFloat64, &NoRspTimeout, 0.5);      // WRITE - Time to wait for response
    createParam("ConnValidDly", asynParamFloat64, &ConnValidDly, 1.0);      // WRITE - Time to wait before disconnecting from parent plugin(s)

    createParam("SaveConfig",   asynParamOctet, &SaveConfig);               // WRITE - Save configuration to file
    createParam("LoadConfig",   asynParamOctet, &LoadConfig);               // WRITE - Load configuration from file
    createParam("CopyConfig",   asynParamInt32, &CopyConfig);               // WRITE - Trigger configration copying
    createParam("Config1",      asynParamOctet, &Config1);                  // WRITE - Name of newest config
    createParam("Config2",      asynParamOctet, &Config2);                  // WRITE - Name of 2nd newest config
    createParam("Config3",      asynParamOctet, &Config3);                  // WRITE - Name of 3rd newest config
    createParam("Config4",      asynParamOctet, &Config4);                  // WRITE - Name of 4th newest config
    createParam("Config5",      asynParamOctet, &Config5);                  // WRITE - Name of 5th newest config

    createParam("ConfigSaved",  asynParamInt32, &ConfigSaved, 0);           // WRITE - Flag =1 when all the PVs are in config control
    createParam("ConfigApplied",asynParamInt32, &ConfigApplied, 0);         // WRITE - Flag =1 when all PVs are synchronized to module

    createRegParam("DISCOVER", "HwType",    true, 0, 0, 0,  8, 0, 0);
    createRegParam("DISCOVER", "DiscFill",  true, 0, 0, 0, 24, 8, 0);

    setIntegerParam(CmdRsp, LAST_CMD_NONE);

    std::list<std::string> configs = getListConfigs();
    unsigned i = 0;
    for (auto config: configs) {
        if (++i > 5)
            break;
        if (i == 1)
            loadConfig(config);
        setStringParam("Config" + std::to_string(i), config);
    }
    callParamCallbacks();

    // Let connect the first time, helps diagnose start-up problems
    if (std::string(parentPlugins).empty() == false) {
        BasePlugin::connect(parentPlugins, MsgDasCmd);
        std::function<float(void)> checkConnCb = std::bind(&BaseModulePlugin::checkConnection, this);
        m_connTimer.schedule(checkConnCb, 1.0);
    }
}

BaseModulePlugin::~BaseModulePlugin()
{}

asynStatus BaseModulePlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == HwId) {
        *nActual = nChars;

        std::string ip(value, nChars);
        uint32_t hardwareId = ip2addr(ip);
        if (hardwareId == 0) {
            setParamAlarmStatus(HwId, epicsAlarmWrite);
            setParamAlarmSeverity(HwId, epicsSevInvalid);
            callParamCallbacks();
            return asynError;
        }

        g_namesMapMutex.lock();
        if (m_hardwareId != 0)
            g_namesMap.erase(m_hardwareId);
        g_namesMap[hardwareId] = portName;
        g_namesMapMutex.unlock();

        m_hardwareId = hardwareId;
        setStringParam(HwId, addr2ip(hardwareId));
        callParamCallbacks();
        return asynSuccess;
    }
    if (pasynUser->reason == SaveConfig) {
        std::string name(value, nChars);
        *nActual = nChars;
        if (!saveConfig(name)) {
            LOG_ERROR("Failed to save configuration '%s'", name.c_str());
            return asynError;
        }

        setIntegerParam(ConfigSaved, 1);

        // Synchronize PVs for the LoadConfigMenu
        std::list<std::string> configs = getListConfigs();
        unsigned i = 0;
        for (auto config: configs) {
            if (++i > 5)
                break;
            setStringParam("Config" + std::to_string(i), config);
        }
        callParamCallbacks();

        return asynSuccess;
    }
    if (pasynUser->reason == LoadConfig) {
        std::string name(value, nChars);
        *nActual = nChars;
        if (!loadConfig(name)) {
            if (!name.empty())
                LOG_ERROR("Failed to load configuration '%s'", name.c_str());
            return asynError;
        }
        setIntegerParam(ConfigSaved, 1);
        callParamCallbacks();
        return asynSuccess;
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

asynStatus BaseModulePlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == CmdReq) {
        if (!processRequest(static_cast<DasCmdPacket::CommandType>(value)))
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
    if (pasynUser->reason == CopyConfig) {
        copyConfig();
        setIntegerParam(ConfigApplied, 0);
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
            LOG_ERROR("Parameter %s value %d out of bounds", getParamName(jt->first).c_str(), value);
            return asynError;
        } else {
            setIntegerParam(jt->first, value);
            setIntegerParam(ConfigApplied, 0);
            setIntegerParam(ConfigSaved, 0);
            callParamCallbacks();
            return asynSuccess;
        }
    }

    // Just issue default handler to see if it can handle it
    return BasePlugin::writeInt32(pasynUser, value);
}

bool BaseModulePlugin::processRequest(DasCmdPacket::CommandType command)
{
    if (getBooleanParam(Enable) == false) {
        LOG_WARN("Plugin not enabled");
        return false;
    }
    if (m_hardwareId == 0) {
        LOG_WARN("No hardware ID set");
        return false;
    }

    auto req = m_cmdHandlers[static_cast<int>(command)].first;
    auto rsp = m_cmdHandlers[static_cast<int>(command)].second;
    if (!req) {
        LOG_WARN("No handler registered for %s", DasCmdPacket::commandToText(command).c_str());
        setParamAlarmStatus(CmdReq, epicsAlarmWrite);
        setParamAlarmSeverity(CmdReq, epicsSevInvalid);
        setIntegerParam(CmdRsp, LAST_CMD_SKIPPED);
        callParamCallbacks();
        return false;
    }

    if (m_cmdQueue.size() >= m_cmdQueueSize) {
        LOG_WARN("Command queue full");
        setParamAlarmStatus(CmdReq, epicsAlarmWrite);
        setParamAlarmSeverity(CmdReq, epicsSevInvalid);
        setIntegerParam(CmdRsp, LAST_CMD_SKIPPED);
        callParamCallbacks();
        return false;
    }

    setIntegerParam(CmdRsp, LAST_CMD_WAIT);
    callParamCallbacks();

    m_cmdQueue.push_back(command);
    processQueuedRequests();
    return true;
}

void BaseModulePlugin::processQueuedRequests()
{
    while (m_waitingResponse == 0 && m_cmdQueue.empty() == false) {
        auto command = m_cmdQueue.front();
        m_cmdQueue.pop_front();
        bool sent = m_cmdHandlers[command].first();
        if (!sent) {
            LOG_WARN("Failed to send %s request", DasCmdPacket::commandToText(command).c_str());
            setParamAlarmStatus(CmdReq, epicsAlarmWrite);
            setParamAlarmSeverity(CmdReq, epicsSevInvalid);
            setIntegerParam(CmdRsp, LAST_CMD_SKIPPED);
            callParamCallbacks();
            continue;
        }

        if (m_cmdHandlers[command].second) {
            LOG_INFO("Sent %s request, expecting response", DasCmdPacket::commandToText(command).c_str());
            setIntegerParam(CmdRsp, LAST_CMD_WAIT);
            callParamCallbacks();

            m_waitingResponse = command;
            scheduleTimeoutCallback(getDoubleParam(NoRspTimeout));
            break;
        } else {
            LOG_INFO("Sent %s request, no response expected", DasCmdPacket::commandToText(command).c_str());
            setIntegerParam(CmdRsp, LAST_CMD_OK);
            callParamCallbacks();
            // No need to prolong m_connLastActive as we're not expecting response
        }
    }
}

void BaseModulePlugin::sendUpstream(DasCmdPacket::CommandType command, uint8_t channel, uint32_t *payload, uint32_t length)
{
    std::array<uint8_t, 1024> buffer;
    DasCmdPacket *packet = DasCmdPacket::init(buffer.data(), buffer.size(), m_hardwareId, command, m_cmdVer, false, false, channel, length, payload);
    if (!packet) {
        LOG_ERROR("Failed to create and send packet");
        return;
    }

    BaseModulePlugin::sendUpstream(packet);
}

void BaseModulePlugin::sendUpstream(const DasCmdPacket* packet)
{
    if (m_hardwareId != 0) {
        m_connLastActive = epicsTime::getCurrent();

        if (!isConnected()) {
            connect(m_parentPlugins, {MsgDasCmd});
        }

        LOG_INFO("Sending packet %s", packet->getCommandText().c_str());
        BasePlugin::sendUpstream(packet);
    }
}

void BaseModulePlugin::recvDownstream(const DasCmdPacketList &packetList)
{
    if (getBooleanParam(Enable) == true && m_hardwareId != 0) {
        for (auto it = packetList.cbegin(); it != packetList.cend(); it++) {
            const DasCmdPacket *packet = *it;

            // Silently skip packets we're not interested in
            if (!packet->isResponse() || packet->getModuleId() != m_hardwareId)
                continue;

            (void)processResponse(packet);
        }
    }
}

bool BaseModulePlugin::processResponse(const DasCmdPacket *packet)
{
    auto command = packet->getCommand();
    if (m_waitingResponse != command) {
        LOG_WARN("Ignoring unexpected response %s", packet->getCommandText().c_str());
        return false;
    }

    m_waitingResponse = static_cast<DasCmdPacket::CommandType>(0);
    if (!cancelTimeoutCallback()) {
        LOG_WARN("Received %s response after timeout", packet->getCommandText().c_str());
        return false;
    }

    LOG_INFO("Received %s %s", packet->getCommandText().c_str(), (packet->isAcknowledge() ? "ACK" : "NACK"));

    auto handler = m_cmdHandlers[static_cast<int>(command)].second;
    if (handler) {
        setIntegerParam(CmdRsp, handler(packet) ? LAST_CMD_OK : LAST_CMD_ERROR);
        callParamCallbacks();
    }

    processQueuedRequests();
    return true;
}

bool BaseModulePlugin::reqSimple(DasCmdPacket::CommandType command)
{
    sendUpstream(command);
    return true;
}

bool BaseModulePlugin::rspSimple(const DasCmdPacket *packet)
{
    return packet->isAcknowledge();
}

bool BaseModulePlugin::rspWriteConfig(const DasCmdPacket *packet)
{
    if (!packet->isAcknowledge())
        return false;

    setIntegerParam(ConfigApplied, 1);
    callParamCallbacks();
    return true;
}

bool BaseModulePlugin::reqParams(DasCmdPacket::CommandType command, const std::string& params)
{
    uint32_t data[1024];
    size_t len = packRegParams(params, data, sizeof(data), 0, 0);
    if (len == 0) {
        LOG_WARN("No parameters named %s, skip sending packet", params.c_str());
        return false;
    }

    sendUpstream(command, 0, data, len);
    return true;
}

bool BaseModulePlugin::rspParams(const DasCmdPacket *packet, const std::string& params)
{
    uint32_t payloadLength = ALIGN_UP(packet->getCmdPayloadLength(), 4);
    uint32_t expectLength = ALIGN_UP(m_params[params].sizes[0]*m_wordSize, 4);
    if (m_params[params].sizes.size() > 1) {
        expectLength = ALIGN_UP((m_params["CONFIG"].offsets[0xF] + m_params["CONFIG"].sizes[0xF])*m_wordSize, 4);
    }
    if (payloadLength != expectLength) {
        LOG_ERROR("Received wrong %s response based on payload length; received %u, expected %u",
                  packet->getCommandText().c_str(),
                  packet->getCmdPayloadLength(),
                  m_params[params].sizes[0]*m_wordSize);
        setParamsAlarm(params, epicsAlarmRead);
        return false;
    }

    setParamsAlarm(params, epicsAlarmNone);
    unpackRegParams(params, packet->getCmdPayload(), payloadLength, 0);
    return true;
}

void BaseModulePlugin::setParamsAlarm(DasCmdPacket::CommandType command, int alarm)
{
    switch (command) {
    case DasCmdPacket::CMD_READ_STATUS:         setParamsAlarm("STATUS",      alarm); break;
    case DasCmdPacket::CMD_READ_COUNTERS:       setParamsAlarm("COUNTERS",    alarm); break;
    case DasCmdPacket::CMD_READ_CONFIG:         setParamsAlarm("CONFIG",      alarm); break;
    case DasCmdPacket::CMD_WRITE_CONFIG:        setParamsAlarm("CONFIG",      alarm); break;
    case DasCmdPacket::CMD_READ_TEMPERATURE:    setParamsAlarm("TEMPERATURE", alarm); break;
    default:                                    break;
    }
}

void BaseModulePlugin::setParamsAlarm(const std::string &section, int alarm)
{
    auto it = m_params.find(section);
    if (it != m_params.end()) {
        for (auto jt = it->second.mapping.begin(); jt != it->second.mapping.end(); jt++){
            setParamAlarmStatus(jt->first, alarm);
            setParamAlarmSeverity(jt->first, alarm != epicsAlarmNone ? epicsSevInvalid : 0);
        }
    }
}

void BaseModulePlugin::createStatusParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    createRegParam("STATUS", name, true, 0, 0x0, offset, nBits, shift, 0);
    m_features |= (uint32_t)ModuleFeatures::STATUS;
}

void BaseModulePlugin::createCounterParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    createRegParam("COUNTERS", name, true, 0, 0x0, offset, nBits, shift, 0);
    m_features |= (uint32_t)ModuleFeatures::COUNTERS;
}

void BaseModulePlugin::createConfigParam(const char *name, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value, const BaseConvert *conv)
{
    if (section >= '1' && section <= '9')
        section = section - '1' + 1;
    else if (section >= 'A' && section <= 'F')
        section = section - 'A' + 0xA;
    else {
        LOG_ERROR("Invalid section '%c' specified for parameter '%s'", section, name);
        return;
    }

    createRegParam("CONFIG", name, false, 0, section, offset, nBits, shift, value, conv);

    std::string nameSaved(name);
    nameSaved += "_Saved";
    int index;
    if (createParam(nameSaved.c_str(), asynParamInt32, &index) != asynSuccess)
        LOG_ERROR("CONFIG save parameter '%s' cannot be created (already exist?)", nameSaved.c_str());
    m_features |= (uint32_t)ModuleFeatures::CONFIG;
}

void BaseModulePlugin::createMetaConfigParam(const char *name, uint32_t nBits, int value, const BaseConvert *conv)
{
    createRegParam("META", name, false, 0, 0, 0, nBits, 0, value, conv);
}

void BaseModulePlugin::createTempParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, const BaseConvert *conv)
{
    createRegParam("TEMPERATURE", name, true, 0, 0x0, offset, nBits, shift, 0, conv);
    m_features |= (uint32_t)ModuleFeatures::TEMPERATURE;
}

void BaseModulePlugin::createUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, const BaseConvert *conv)
{
    createRegParam("UPGRADE", name, true, 0, 0x0, offset, nBits, shift, 0, conv);
    m_features |= (uint32_t)ModuleFeatures::UPGRADE;
}

void BaseModulePlugin::linkUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    linkRegParam("UPGRADE", name, true, 0, 0x0, offset, nBits, shift);
    m_features |= (uint32_t)ModuleFeatures::UPGRADE;
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

float BaseModulePlugin::noResponseCleanup()
{
    float expire = 0.0;

    // This callback called only from the Timer::expire() - must lock
    lock();

    if (m_waitingResponse != 0) {
        LOG_WARN("Timeout waiting for %s response", DasCmdPacket::commandToText(m_waitingResponse).c_str());
        setParamsAlarm(m_waitingResponse, epicsAlarmTimeout);
        setIntegerParam(CmdRsp, LAST_CMD_TIMEOUT);
        callParamCallbacks();
        m_waitingResponse = static_cast<DasCmdPacket::CommandType>(0);
    }

    // Process any pending requests
    processQueuedRequests();
    if (m_waitingResponse != 0) {
        // Scheduling m_noRspTimer must have failed in processQueueRequests()
        // since it was called from timer callback routine.
        // Let's fix that here.
        expire = getDoubleParam(NoRspTimeout);
    }

    unlock();

    // Timer will auto-cancel when expire==0
    return expire;
}

bool BaseModulePlugin::scheduleTimeoutCallback(double delay)
{
    std::function<float(void)> timeoutCb = std::bind(&BaseModulePlugin::noResponseCleanup, this);
    return m_noRspTimer.schedule(timeoutCb, delay);
}

bool BaseModulePlugin::cancelTimeoutCallback()
{
    // We need to unlock the port to prevent dead-lock when Timer is just
    // expiring. The cancel() function will wait for the expire callback
    // to complete, which never happens if the port remains locked.
    unlock();
    bool canceled = m_noRspTimer.cancel();
    lock();
    return canceled;
}

size_t BaseModulePlugin::packRegParams(const std::string& group, uint32_t *payload, size_t size, uint8_t channel, uint8_t section)
{
    uint32_t payloadLength;
    if (section == 0x0) {
        uint32_t section_f = SECTION_ID((m_params[group].sections ? 0xF : 0x0), channel);
        payloadLength = m_params[group].offsets[section_f] + m_params[group].sizes[section_f];
    } else {
        uint32_t sectionId = SECTION_ID(section, channel);
        payloadLength = m_params[group].sizes[sectionId];
    }
    payloadLength *= m_wordSize;

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
            LOG_ERROR("Failed to get parameter %s value", getParamName(it->first).c_str());
            return false;
        }

        if (m_wordSize == 2) {
            shift += (offset % 2 == 0 ? 0 : 16);
            offset /= 2;
        }

        if (offset >= length) {
            // Unlikely, but rather sure than sorry
            LOG_ERROR("Parameter %s offset out of range", getParamName(it->first).c_str());
            continue;
        }

        value = it->second.convert->toRaw(value, it->second.width);
        payload[offset] |= value << shift;
        if ((it->second.width + shift) > 32) {
            payload[offset+1] |= value >> (it->second.width - (32 - shift));
        }
    }

    return payloadLength;
}

void BaseModulePlugin::unpackRegParams(const std::string& group, const uint32_t *payload, size_t size, uint8_t channel)
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

            auto jt = offsets.find(it->second.section);
            if (jt != offsets.end()) {
                offset += jt->second;
            }
        }

        if (m_wordSize == 2) {
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
    desc.convert = conv;
    m_params[group].mapping[index] = desc;

    uint32_t length = offset + 1;
    if (m_wordSize == 2 && nBits > 16)
        length++;

    m_params[group].sizes[section] = std::max(m_params[group].sizes[section], length);

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
    desc.convert = CONV_UNSIGN;
    m_params[group].mapping[index] = desc;

    uint32_t length = offset + 1;
    if (m_wordSize == 2 && nBits > 16)
        length++;
    length *= m_wordSize;

    m_params[group].sizes[section] = std::max(m_params[group].sizes[section], length);

    if (readonly == false)
        m_params[group].readonly = false;
    if (section != 0x0)
        m_params[group].sections = true;
}

void BaseModulePlugin::initParams(uint32_t nChannels)
{
    // Go through all groups and recalculate sections sizes and offsets
    for (auto it = m_params.begin(); it != m_params.end(); it++) {

        std::map<int, uint32_t> &sizes = it->second.sizes;
        std::map<int, uint32_t> &offsets = it->second.offsets;

        // Section sizes have already been calculated in createConfigParams()
        for (uint32_t channel = 0; channel <= nChannels; channel++) {

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

    setIntegerParam(Features, m_features);
    callParamCallbacks();
}

std::string BaseModulePlugin::getModuleName(uint32_t hardwareId)
{
    std::string name = "";
    g_namesMapMutex.lock();
    auto it = g_namesMap.find(hardwareId);
    if (it != g_namesMap.end())
        name = it->second;
    g_namesMapMutex.unlock();
    return name;
}

void BaseModulePlugin::getModuleNames(std::list<std::string> &modules)
{
    modules.clear();
    g_namesMapMutex.lock();
    for (auto it = g_namesMap.begin(); it != g_namesMap.end(); it++) {
        modules.push_back(it->second);
    }
    g_namesMapMutex.unlock();
}

float BaseModulePlugin::checkConnection()
{
    this->lock();
    epicsTime expireTime = m_connLastActive + getDoubleParam(ConnValidDly);
    if (isConnected() && expireTime < epicsTime::getCurrent()) {
        disconnect();
    }
    this->unlock();
    return 1.0;
}

static bool invalidChar(char ch)
{
    return (!std::isalnum(ch) && ch != '_' && ch != '.' && ch != '-');
}

std::string BaseModulePlugin::getConfigPath(const std::string &name, bool existing)
{
    std::string filepath = m_configDir + getPortName() + "_";
    std::string configName = name;
    std::replace_if(configName.begin(), configName.end(), invalidChar, '_');
    if (configName.empty()) {
        auto time_now = std::chrono::system_clock::now();
        auto time_now_t = std::chrono::system_clock::to_time_t(time_now);
        auto now_tm = *std::localtime(&time_now_t);
        char buf[32];
        std::strftime(buf, 32, "%Y-%m-%d", &now_tm);
        configName = buf;
    }

    filepath += configName;
    if (existing) {
        std::string path = filepath + ".sav";
        std::ifstream f_exists(path);
        if (f_exists.is_open() == true)
            return path;
    } else {
        for (int i = 0; i < 100; i++) {
            std::string path = filepath;
            if (i > 0)
                path += "_" + std::to_string(i);
            path += ".sav";
            std::ifstream f_exists(path);
            if (f_exists.is_open() == false)
                return path;
        }
    }
    return std::string();
}

std::string BaseModulePlugin::parseConfigName(const std::string &filename)
{
    std::string portName = getPortName();
    if (filename.compare(0, portName.length() + 1, portName + "_") != 0)
        return std::string();

    std::string name = filename.substr(portName.length() + 1);
    if (name.length() > 4 && name.compare(name.length() - 5, 4, ".sav"))
        name = name.substr(0, name.length() - 4);
    return name;
}

bool BaseModulePlugin::saveConfig(const std::string &name)
{
    std::string filepath = getConfigPath(name, false);
    if (filepath.empty() == true)
        return false;

    std::ofstream f(filepath);
    auto it = m_params.find("CONFIG");
    if (it != m_params.end()) {
        for (auto jt = it->second.mapping.begin(); jt != it->second.mapping.end(); jt++) {
            std::string param = getParamName(jt->first);
            int value = getIntegerParam(jt->first);
            f << param << " " << value << std::endl;
        }
    }
    return true;
}

bool BaseModulePlugin::loadConfig(const std::string &name)
{
    std::string filepath = getConfigPath(name, true);
    if (filepath.empty() == true)
        return false;

    std::string param;
    int value;
    std::ifstream f(filepath);
    while (f >> param >> value) {
        (void)setIntegerParam(param + "_Saved", value);
    }
    callParamCallbacks();
    return true;
}

void BaseModulePlugin::copyConfig()
{
    auto it = m_params.find("CONFIG");
    if (it != m_params.end()) {
        for (auto jt = it->second.mapping.begin(); jt != it->second.mapping.end(); jt++) {
            std::string param = getParamName(jt->first) + "_Saved";
            int index;
            asynStatus ret = asynPortDriver::findParam(param.c_str(), &index);
            if (ret == asynSuccess) {
                int value;
                ret = getIntegerParam(index, &value);
                if (ret == asynSuccess)
                    setIntegerParam(jt->first, value);
            }
        }
    }
    callParamCallbacks();
}

std::list<std::string> BaseModulePlugin::getListConfigs()
{
    std::list<time_t> times;
    std::list<std::string> configs;
    std::string portName = getPortName();
    DIR *dirp = opendir(m_configDir.c_str());
    struct dirent *dp = nullptr;
    if (dirp != nullptr) {
        while ((dp = readdir(dirp)) != nullptr) {
            std::string name = parseConfigName(dp->d_name);
            if (name.empty() == false) {
                struct stat st;
                std::string path = m_configDir + dp->d_name;
                if (stat(path.c_str(), &st) == 0) {
                    auto it = times.begin();
                    auto jt = configs.begin();
                    for (; it != times.end() && st.st_ctime < *it; it++, jt++);
                    times.insert(it, st.st_ctime);
                    configs.insert(jt, name);
                }
            }
        }
        closedir(dirp);
    }
    return configs;
}
