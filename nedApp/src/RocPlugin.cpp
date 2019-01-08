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
 * Return a unique id for section and channel pair that can be used to
 * identify the pair m_params tables.
 */
#define SECTION_ID(section, channel) (((channel) * 0x10) + ((section) & 0xF))

/**
 * Convert channel number to VerifyId field in command packet.
 */
#define CHAN2VERID(channel) ((channel == 0) ? 0 : 0x10 | (channel - 1))

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
    } else {
        LOG_ERROR("Unsupported ROC version '%s'", version.c_str());
        return;
    }

    if (m_numChannels > 0) {
        m_cmdQueueSize += m_numChannels;
        m_cmdHandlers[DasCmdPacket::CMD_WRITE_CONFIG].first  = std::bind(&RocPlugin::reqWriteConfig, this);
        m_cmdHandlers[DasCmdPacket::CMD_WRITE_CONFIG].second = std::bind(&RocPlugin::rspWriteConfig, this, std::placeholders::_1);
        m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].first   = std::bind(&RocPlugin::reqParamsChan, this, DasCmdPacket::CMD_READ_CONFIG);
        m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].second  = std::bind(&RocPlugin::rspParamsChan, this, std::placeholders::_1, "CONFIG");
        m_cmdHandlers[DasCmdPacket::CMD_READ_STATUS].first   = std::bind(&RocPlugin::reqParamsChan, this, DasCmdPacket::CMD_READ_STATUS);
        m_cmdHandlers[DasCmdPacket::CMD_READ_STATUS].second  = std::bind(&RocPlugin::rspParamsChan, this, std::placeholders::_1, "STATUS");
    }
    if (havePreAmpTest) {
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_CONFIG].first   = std::bind(&BaseModulePlugin::reqParams,  this, DasCmdPacket::CMD_PREAMP_TEST_CONFIG, "PREAMP_CFG");
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_CONFIG].second  = std::bind(&BaseModulePlugin::rspSimple,  this, std::placeholders::_1);
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_TRIGGER].first  = std::bind(&BaseModulePlugin::reqParams, this, DasCmdPacket::CMD_PREAMP_TEST_TRIGGER, "PREAMP_TRIG");
        m_cmdHandlers[DasCmdPacket::CMD_PREAMP_TEST_TRIGGER].second = std::bind(&BaseModulePlugin::rspSimple, this, std::placeholders::_1);
    }
    m_cmdHandlers[DasCmdPacket::CMD_HV_SEND].first                  = std::bind(&RocPlugin::reqHv,     this);
    m_cmdHandlers[DasCmdPacket::CMD_HV_SEND].second                 = std::bind(&RocPlugin::rspHv,     this, std::placeholders::_1);
    // CMD_HV_RECV is handled through custom processResponse()

    createParam("HvDelay",      asynParamFloat64, &HvDelay,     0.0); // READ - Time from HV request to first response character
    createParam("HvB2bDelay",   asynParamFloat64, &HvB2bDelay,  0.0); // READ - Time from HV request to last response character

    initParams(m_numChannels);
}

void RocPlugin::createChanStatusParam(const char *name, uint8_t channel, uint32_t offset, uint32_t nBits, uint32_t shift)
{
    uint8_t section = SECTION_ID(0, channel);
    createRegParam("STATUS", name, true, channel, section, offset, nBits, shift, 0);
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

    uint8_t sectionId = SECTION_ID(section, channel);
    createRegParam("CONFIG", name, false, channel, sectionId, offset, nBits, shift, value, conv);

    std::string nameSaved(name);
    nameSaved += "_Saved";
    int index;
    if (createParam(nameSaved.c_str(), asynParamInt32, &index) != asynSuccess)
        LOG_ERROR("CONFIG save parameter '%s' cannot be created (already exist?)", nameSaved.c_str());
    m_features |= (uint32_t)ModuleFeatures::CONFIG;
}

asynStatus RocPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (m_numChannels > 0 && pasynUser->reason == CmdReq) {
        if (value == DasCmdPacket::CMD_WRITE_CONFIG ||
            value == DasCmdPacket::CMD_READ_CONFIG ||
            value == DasCmdPacket::CMD_READ_STATUS) {

            m_expectedChannel = 0;

            // Schedule sending 9 packets, our reqWriteConfig() will change the channel
            for (uint8_t i = 0; i < 9; i++) {
                processRequest(static_cast<DasCmdPacket::CommandType>(value));
            }
            return asynSuccess;
        }
    }
    return BaseModulePlugin::writeInt32(pasynUser, value);
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
        LOG_DEBUG("HV: %s", buffer);

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
    return BaseModulePlugin::processResponse(packet);
}

bool RocPlugin::reqWriteConfig()
{
    uint32_t data[1024];
    size_t len = packRegParams("CONFIG", data, sizeof(data), m_expectedChannel);
    if (len == 0) {
        LOG_WARN("Skipping sending write config packet");
        return false;
    }

    uint8_t channel = 0;
    if (m_expectedChannel > 0)
        channel = 0x10 | (m_expectedChannel - 1);

    sendUpstream(DasCmdPacket::CMD_WRITE_CONFIG, channel, data, len);
    return true;
}

bool RocPlugin::rspWriteConfig(const DasCmdPacket* packet)
{
    uint8_t channel = 0;
    if (m_expectedChannel > 0)
        channel = 0x10 | (m_expectedChannel - 1);

    // Turns back to 0 after last channel, works also for m_numChannels == 0
    m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);

    if (!packet->isAcknowledge())
        return false;

    if (packet->getCmdId() != channel) {
        LOG_WARN("Expecting WRITE_CONFIG response for channel %u, received channel %u", channel & 0xF, packet->getCmdId() & 0xF);
        return false;
    }

    return true;
}

bool RocPlugin::reqParamsChan(DasCmdPacket::CommandType command)
{
    uint8_t channel = CHAN2VERID(m_expectedChannel);
    sendUpstream(command, channel, nullptr, 0);
    return true;
}

bool RocPlugin::rspParamsChan(const DasCmdPacket* packet, const std::string& params)
{
    if (!packet->isAcknowledge())
        return false;

    uint8_t wordSize = 2;
    uint32_t payloadLength = ALIGN_UP(packet->getCmdPayloadLength(), 4);
    uint32_t section = SECTION_ID(0x0, m_expectedChannel);
    uint32_t expectLength = ALIGN_UP(m_params[params].sizes[section]*wordSize, 4);
    if (m_params[params].sizes.size() > 1 && params == "CONFIG") {
        section = SECTION_ID(0xF, m_expectedChannel);
        expectLength = ALIGN_UP((m_params["CONFIG"].offsets[section] + m_params["CONFIG"].sizes[section])*wordSize, 4);
    }

    uint8_t channel = m_expectedChannel;

    // Turns back to 0 after last channel, works also for m_numChannels == 0
    m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);

    if (packet->getCmdId() != CHAN2VERID(channel)) {
        LOG_WARN("Expecting %s response for channel %u, received channel %u", params.c_str(), channel, (packet->getCmdId()+1) & 0xF);
        return false;
    }

    if (payloadLength != expectLength) {
        if (channel == 0) {
            LOG_ERROR("Received wrong %s response based on length; received %u, expected %u",
                      params.c_str(), payloadLength, expectLength);
        } else {
            LOG_ERROR("Received wrong channel %u %s response based on length; received %u, expected %u",
                      channel, params.c_str(), payloadLength, expectLength);
        }
        setParamsAlarm(params, epicsAlarmRead);
        return false;
    }

    setParamsAlarm(params, epicsAlarmNone);
    unpackRegParams(params, packet->getCmdPayload(), payloadLength, channel);

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
    createRegParam("PREAMP_CFG", name, false, 0, 0x0, offset, nBits, shift, value, CONV_UNSIGN);
}

void RocPlugin::createPreAmpTrigParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, int value)
{
    createRegParam("PREAMP_TRIG", name, false, 0, 0x0, offset, nBits, shift, value, CONV_UNSIGN);
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
