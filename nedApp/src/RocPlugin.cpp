/* RocPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "RocPlugin.h"
#include "Log.h"

#include <cstring>

// EPICS includes
#include <alarm.h>

EPICS_REGISTER_PLUGIN(RocPlugin, 4, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Config dir", string);

/**
 * Convert channel number to VerifyId field in command packet.
 */
#define CHAN2CMDID(channel) ((channel == 0) ? 0 : 0x10 | (channel - 1))
#define CMDID2CHAN(cmdId)   ((cmdId & 0x10) ? ((cmdId & 0xF) + 1) : 0)

RocPlugin::RocPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, 2)
{
    bool havePreAmpTest = false;

    std::string version(version_);
    if (0) {
    } else if (version == "v14") {
        m_numChannels = 8;
        createParams_v14();
    } else if (version == "v43") {
        m_numChannels = 8;
        createParams_v43();
    } else if (version == "v44") {
        m_numChannels = 8;
        createParams_v45();
    } else if (version == "v45") {
        m_numChannels = 8;
        createParams_v45();
    } else if (version == "v47") {
        m_numChannels = 8;
        createParams_v47();
    } else if (version == "v50") {
        m_numChannels = 8;
        createParams_v50();
    } else if (version == "v51") {
        createParams_v51();
    } else if (version == "v52") {
        createParams_v52();
    } else if (version == "v54") {
        createParams_v54();
        m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].second = std::bind(&RocPlugin::rspReadConfigV54, this, std::placeholders::_1);
    } else if (version == "v55") {
        createParams_v54();
    } else if (version == "v56") {
        createParams_v56();
    } else if (version == "v57") {
        createParams_v57();
    } else if (version == "v58") {
        createParams_v58();
        havePreAmpTest = true;
    } else if (version == "v59") {
        createParams_v59();
        havePreAmpTest = true;
    } else if (version == "v510") {
        createParams_v510();
        havePreAmpTest = true;
    } else if (version == "v511") {
        createParams_v511();
        havePreAmpTest = true;
    } else if (version == "v512") {
        createParams_v512();
        havePreAmpTest = true;
    } else if (version == "v513") {
        createParams_v513();
        havePreAmpTest = true;
    } else {
        LOG_ERROR("Unsupported ROC version '%s'", version.c_str());
        return;
    }

    if (m_numChannels > 0) {
        m_cmdHandlers[DasCmdPacket::CMD_WRITE_CONFIG].first  = std::bind(&RocPlugin::reqWriteConfig, this);
        m_cmdHandlers[DasCmdPacket::CMD_WRITE_CONFIG].second = std::bind(&RocPlugin::rspWriteConfig, this, std::placeholders::_1);
        m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].first   = std::bind(&RocPlugin::reqParamsChan, this, DasCmdPacket::CMD_READ_CONFIG);
        m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].second  = std::bind(&RocPlugin::rspParamsChan, this, std::placeholders::_1, "CONFIG");
        m_cmdHandlers[DasCmdPacket::CMD_READ_STATUS].first   = std::bind(&RocPlugin::reqParamsChan, this, DasCmdPacket::CMD_READ_STATUS);
        m_cmdHandlers[DasCmdPacket::CMD_READ_STATUS].second  = std::bind(&RocPlugin::rspParamsChan, this, std::placeholders::_1, "STATUS");
        m_receivedRsp[DasCmdPacket::CMD_WRITE_CONFIG].reserve(m_numChannels+1);
        m_receivedRsp[DasCmdPacket::CMD_READ_CONFIG].reserve(m_numChannels+1);
        m_receivedRsp[DasCmdPacket::CMD_READ_STATUS].reserve(m_numChannels+1);
    }
    if (havePreAmpTest) {
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_CONFIG].first   = std::bind(&BaseModulePlugin::reqParams,  this, DasCmdPacket::CMD_PREAMP_TEST_CONFIG, "PREAMP_CFG");
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_CONFIG].second  = std::bind(&BaseModulePlugin::rspSimple,  this, std::placeholders::_1);
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_TRIGGER].first  = std::bind(&BaseModulePlugin::reqParams, this, DasCmdPacket::CMD_PREAMP_TEST_TRIGGER, "PREAMP_TRIG");
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_TRIGGER].second = std::bind(&BaseModulePlugin::rspSimple, this, std::placeholders::_1);
    }
    m_cmdHandlers[DasCmdPacket::CMD_HV_SEND].first                  = std::bind(&RocPlugin::reqHv, this);
    m_cmdHandlers[DasCmdPacket::CMD_HV_SEND].second                 = std::bind(&BaseModulePlugin::rspSimple, this, std::placeholders::_1);
    // CMD_HV_RECV is handled through custom processResponse()

    createParam("HvDelay",      asynParamFloat64, &HvDelay,     0.0); // READ - Time from HV request to first response character
    createParam("HvB2bDelay",   asynParamFloat64, &HvB2bDelay,  0.0); // READ - Time from HV request to last response character

    initParams();
}

void RocPlugin::createChanStatusParam(const char *name, uint8_t channel, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    std::string chanParams = "STATUS_" + std::to_string(channel);
    createRegParam(chanParams.c_str(), name, true, offset, nBits, shift, 0, CONV_UNSIGN);
    m_features |= (uint32_t)ModuleFeatures::STATUS;
}

void RocPlugin::createChanConfigParam(const char *name, uint8_t channel, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value, const BaseConvert *conv)
{
    if (section >= '1' && section <= '9')
        section = section - '1' + 1;
    else if (section >= 'A' && section <= 'F')
        section = section - 'A' + 0xA;
    else {
        LOG_ERROR("Invalid section '%c' specified for parameter '%s'", section, name);
        return;
    }

    std::string chanParams = "CONFIG_" + std::to_string(channel);
    createRegParam(chanParams.c_str(), name, false, offset, nBits, shift, value, conv, section);

    std::string nameSaved(name);
    nameSaved += "_Saved";
    int index;
    if (createParam(nameSaved.c_str(), asynParamInt32, &index) != asynSuccess)
        LOG_ERROR("CONFIG save parameter '%s' cannot be created (already exist?)", nameSaved.c_str());
    m_features |= (uint32_t)ModuleFeatures::CONFIG;
}

asynStatus RocPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    // Only serving StreamDevice - puts reason as -1
    if (pasynUser->reason == -1) {
        // StreamDevice is sending entire string => no need to buffer the request.
        m_hvRequest = std::string(value, nChars);
        processRequest(DasCmdPacket::CMD_HV_SEND);
        *nActual = nChars;
        return asynSuccess;
    }

    return BaseModulePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

asynStatus RocPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    // Only serving StreamDevice - puts reason as -1
    if (pasynUser->reason == -1) {
        asynStatus status = asynSuccess;

        // Temporarily unlock the device or processResponse() will not run
        this->unlock();

        *nActual = m_hvBuffer.dequeue(value, nChars, pasynUser->timeout);

        if (*nActual == 0) {
            status = asynTimeout;
        } else if (eomReason) {
            if (value[*nActual - 1] == '\r') *eomReason |= ASYN_EOM_EOS;
            if (*nActual == nChars)          *eomReason |= ASYN_EOM_CNT;
        }

        char buffer[256];
        memset(buffer, 0, 256);
        strncpy(buffer, value, *nActual);
        //LOG_DEBUG("HV: %s", buffer);

        this->lock();

        return status;
    }
    return BaseModulePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

bool RocPlugin::processResponse(const DasCmdPacket *packet)
{
    if (packet->getCommand() == DasCmdPacket::CMD_HV_RECV) {
        return rspHv(packet);
    }
    if (m_numChannels > 0 && packet->getCmdId() != 0) {
        // We only process responses from sub-FPGAs. Handle main FPGA
        // response through BaseModulePlugin mechanism to engage
        // its book-keeping mechanism (for timeout etc.)
        if (packet->getCommand() == DasCmdPacket::CMD_WRITE_CONFIG) {
            return rspWriteConfig(packet);
        }
        if (packet->getCommand() == DasCmdPacket::CMD_READ_CONFIG) {
            return rspParamsChan(packet, "CONFIG");
        }
        if (packet->getCommand() == DasCmdPacket::CMD_READ_STATUS) {
            return rspParamsChan(packet, "STATUS");
        }
    }
    return BaseModulePlugin::processResponse(packet);
}

bool RocPlugin::reqWriteConfig()
{
    assert(m_numChannels > 0);

    // Send out channel requests first so that main FPGA response comes in
    // last and we can process it through regular response handlers.
    uint32_t data[1024];
    for (int channel = m_numChannels; channel > 0; channel--) {
        std::string chanParams = "CONFIG_" + std::to_string(channel);
        size_t len = packRegParams(chanParams, data, sizeof(data), 0);
        if (len == 0) {
            LOG_WARN("Skipping sending write config packet");
            continue;
        }
        sendUpstream(DasCmdPacket::CMD_WRITE_CONFIG, CHAN2CMDID(channel), data, len);
        m_receivedRsp[DasCmdPacket::CMD_WRITE_CONFIG][channel] = false;
        LOG_DEBUG("Sent WRITE_CONFIG packet for channel %u", channel);
    }

    size_t len = packRegParams("CONFIG", data, sizeof(data), 0);
    if (len == 0) {
        LOG_WARN("Skipping sending write config packet");
    } else {
        sendUpstream(DasCmdPacket::CMD_WRITE_CONFIG, CHAN2CMDID(0), data, len);
        m_receivedRsp[DasCmdPacket::CMD_WRITE_CONFIG][0] = false;
    }

    return true;
}

bool RocPlugin::rspWriteConfig(const DasCmdPacket* packet)
{
    std::string cmdStr = DasCmdPacket::commandToText(DasCmdPacket::CMD_WRITE_CONFIG);
    uint8_t channel = CMDID2CHAN(packet->getCmdId());

    if (channel > m_numChannels) {
        LOG_WARN("Unexpected %s response for channel %u", cmdStr.c_str(), channel);
        return false;
    }

    if (channel > 0) {
        // Main FPGA response is logged from BaseModulePlugin but not others
        LOG_INFO("Received %s %s for channel %u", cmdStr.c_str(), packet->isAcknowledge() ? "ACK" : "NACK", channel);
    }

    if (m_receivedRsp[DasCmdPacket::CMD_WRITE_CONFIG][channel] == true)
        LOG_WARN("Already received %s response for channel %u, ignoring", cmdStr.c_str(), channel);
    else
        m_receivedRsp[DasCmdPacket::CMD_WRITE_CONFIG][channel] = true;

    if (!packet->isAcknowledge())
        return false;

    // So far so good. Unless we're processing the main FPGA response
    // (which comes in last), we can exit.
    if (channel > 0)
        return true;

    uint8_t missingResponses = m_numChannels + 1;
    for (uint8_t i = 0; i <= m_numChannels; i++) {
        if (m_receivedRsp[DasCmdPacket::CMD_WRITE_CONFIG][i] == true) {
            missingResponses--;
        }
    }
    if (missingResponses == 0) {
        setIntegerParam(ConfigApplied, 1);
        callParamCallbacks();
        return true;
    } else {
        LOG_DEBUG("Missing %s response from %u channels", cmdStr.c_str(), missingResponses);
        return false;
    }
}

bool RocPlugin::reqParamsChan(DasCmdPacket::CommandType command)
{
    assert(m_numChannels > 0);

    // Send out channel requests first so that main FPGA response comes in
    // last and we can process it through BaseModulePlugin response handlers.
    for (int channel = m_numChannels; channel > 0; channel--) {
        uint8_t cmdId = CHAN2CMDID(channel);
        sendUpstream(command, cmdId, nullptr, 0);
        m_receivedRsp[command][channel] = false;
        LOG_DEBUG("Sent %s packet for channel %u", DasCmdPacket::commandToText(command).c_str(), channel);
    }

    sendUpstream(command);
    m_receivedRsp[command][0] = false;
    return true;
}

bool RocPlugin::rspParamsChan(const DasCmdPacket* packet, const std::string& params)
{
    uint8_t channel = CMDID2CHAN(packet->getCmdId());

    if (channel > m_numChannels) {
        LOG_WARN("Unexpected %s response for channel %u", packet->getCommandText().c_str(), channel);
        return false;
    }

    if (channel > 0) {
        // Main FPGA response is logged from BaseModulePlugin but not others
        LOG_INFO("Received %s %s for channel %u", packet->getCommandText().c_str(), packet->isAcknowledge() ? "ACK" : "NACK", channel);
    }

    if (m_receivedRsp.find(packet->getCommand()) == m_receivedRsp.end()) {
        LOG_ERROR("Unsupported response type %s for module with channels", packet->getCommandText().c_str());
        return false;
    }

    if (m_receivedRsp[packet->getCommand()][channel] == true)
        LOG_WARN("Already received %s response for channel %u, ignoring", packet->getCommandText().c_str(), channel);
    else
        m_receivedRsp[packet->getCommand()][channel] = true;

    if (!packet->isAcknowledge())
        return false;

    uint8_t wordSize = 2;
    std::string chanParam = params;
    if (channel > 0)
        chanParam += "_" + std::to_string(channel);
    uint32_t payloadLength = ALIGN_UP(packet->getCmdPayloadLength(), 4);
    uint32_t expectLength = ALIGN_UP(m_params[chanParam].size*wordSize, 4);

    if (payloadLength != expectLength) {
        if (channel == 0) {
            LOG_ERROR("Received wrong %s response based on length; received %u, expected %u",
                      params.c_str(), payloadLength, expectLength);
        } else {
            LOG_ERROR("Received wrong channel %u %s response based on length; received %u, expected %u",
                      channel, params.c_str(), payloadLength, expectLength);
        }
        setParamsAlarm(chanParam, epicsAlarmRead);
        return false;
    }

    setParamsAlarm(chanParam, epicsAlarmNone);
    unpackRegParams(chanParam, packet->getCmdPayload(), payloadLength);

    if (channel == 0) {
        // Main FPGA response is last, so at this point we should have received
        // all the others.
        uint8_t missingResponses = m_numChannels + 1;
        for (uint8_t i = 0; i <= m_numChannels; i++) {
            if (m_receivedRsp[packet->getCommand()][i] == true) {
                missingResponses--;
            }
        }
        if (missingResponses > 0) {
            LOG_DEBUG("Missing %s response from %u channels", packet->getCommandText().c_str(), missingResponses);
            return false;
        }
    }

    return true;
}

bool RocPlugin::rspReadConfigV54(const DasCmdPacket *packet)
{
    uint8_t buffer[480]; // actual size of the READ_CONFIG v5.4 packet
    if (packet->getLength() > sizeof(buffer)) {
        LOG_ERROR("Received v5.4 READ_CONFIG response bigger than expected");
        return false;
    }

    // Packet in shared queue must not be modified. So we make a copy.
    packet = DasCmdPacket::init(buffer, sizeof(buffer),
                                packet->getModuleId(),
                                packet->getCommand(),
                                0,
                                packet->isAcknowledge(),
                                packet->isResponse(),
                                /* channel= */0,
                                packet->getCmdPayloadLength()-4, // This is the only reason we're doing all the buffering
                                packet->getCmdPayload());

    return BaseModulePlugin::rspParams(packet, "CONFIG");
}

bool RocPlugin::reqHv()
{
    if (m_hvRequest.empty())
        return false;

    uint32_t buffer[32] = { 0 }; // Initialize all to 0
    uint32_t bufferLen = m_hvRequest.length() * 2;

    // Every character in protocol needs to be prefixed with a zero byte when sent over OCC
    for (uint32_t i = 0; i < m_hvRequest.length(); i++) {
        buffer[i/2] |= m_hvRequest.at(i) << (16*(i%2));
    }
    sendUpstream(DasCmdPacket::CMD_HV_SEND, 0, buffer, bufferLen);

    epicsTimeGetCurrent(&m_sendHvTime);
    m_hvRequest.clear();
    return true;
}

bool RocPlugin::rspHv(const DasCmdPacket *packet)
{
    const uint32_t *payload = packet->getCmdPayload();
    epicsTimeStamp now;
    double diff;

    // Single character per OCC packet
    char byte = payload[0] & 0xFF;

    switch (byte) {
    case '?':
        epicsTimeGetCurrent(&now);
        diff = epicsTimeDiffInSeconds(&now, &m_sendHvTime);
        setDoubleParam(HvDelay, diff);
        callParamCallbacks();
        break;
    case '\r':
        epicsTimeGetCurrent(&now);
        diff = epicsTimeDiffInSeconds(&now, &m_sendHvTime);
        setDoubleParam(HvB2bDelay, diff);
        callParamCallbacks();
        break;
    default:
        break;
    }

    m_hvBuffer.enqueue(&byte, 1);

    return true;
}

void RocPlugin::createPreAmpCfgParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, int value)
{
    createRegParam("PREAMP_CFG", name, false, offset, nBits, shift, value, CONV_UNSIGN);
}

void RocPlugin::createPreAmpTrigParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, int value)
{
    createRegParam("PREAMP_TRIG", name, false, offset, nBits, shift, value, CONV_UNSIGN);
}

bool RocPlugin::saveConfig(const std::string &name)
{
    if (m_numChannels == 0)
        return BaseModulePlugin::saveConfig(name);

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
    for (unsigned channel = 1; channel <= m_numChannels; channel++) {
        auto it = m_params.find("CONFIG_" + std::to_string(channel));
        if (it != m_params.end()) {
            for (auto jt = it->second.mapping.begin(); jt != it->second.mapping.end(); jt++) {
                std::string param = getParamName(jt->first);
                int value = getIntegerParam(jt->first);
                f << param << " " << value << std::endl;
            }
        }
    }
    return true;
}

void RocPlugin::copyConfig()
{
    if (m_numChannels == 0)
        return BaseModulePlugin::copyConfig();

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
    for (unsigned channel = 1; channel <= m_numChannels; channel++) {
        auto it = m_params.find("CONFIG_" + std::to_string(channel));
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
    }
    callParamCallbacks();
}


// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
