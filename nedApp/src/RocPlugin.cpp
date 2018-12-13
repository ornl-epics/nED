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
 * ROC V5 version response format
 */
struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     // Board revision number
    unsigned hw_version:8;      // Board version number
    unsigned fw_revision:8;     // Firmware revision number
    unsigned fw_version:8;      // Firmware version number
    unsigned year:16;           // Year
    unsigned day:8;             // Day
    unsigned month:8;           // Month
#else
#error Missing RspReadVersionV5 declaration
#endif // BITFIELD_LSB_FIRST
};

/**
 * ROC V5 fw 5.4 adds vendor field.
 */
struct RspReadVersion_v54 : public RspReadVersion {
    uint32_t vendor_id;
};

/**
 * GE ROC V2 version response format
 *
 * The upper byte is always 0x20=32. This doesn't play well
 * with the rest of the code so we redefine the version meaning
 * and only use half of the hw version an leave the rest 12 bits
 * for revision, which ends up much bigger than any SNS revisions.
 */
struct RspReadVersionGE {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:12;    // Board revision number
    unsigned hw_version:4;      // Board version number
    unsigned fw_revision:8;     // Firmware revision number
    unsigned fw_version:8;      // Firmware version number
#else
#error Missing RspReadVersionV5 declaration
#endif // BITFIELD_LSB_FIRST
};

RocPlugin::RocPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, DasCmdPacket::MOD_TYPE_ROC, 2)
    , m_version(version)
{
    bool havePreAmpTest = false;

    if (0) {
    } else if (m_version == "v14") {
        m_numChannels = 8;
        setIntegerParam(Supported, 1);
        createParams_v14();
        setExpectedVersion(1, 4);
    } else if (m_version == "v43") {
        m_numChannels = 8;
        setIntegerParam(Supported, 1);
        createParams_v43();
        setExpectedVersion(4, 3);
    } else if (m_version == "v44") {
        m_numChannels = 8;
        setIntegerParam(Supported, 1);
        createParams_v45();
        setExpectedVersion(4, 4);
    } else if (m_version == "v45") {
        m_numChannels = 8;
        setIntegerParam(Supported, 1);
        createParams_v45();
        setExpectedVersion(4, 5);
    } else if (m_version == "v47") {
        m_numChannels = 8;
        setIntegerParam(Supported, 1);
        createParams_v47();
        setExpectedVersion(4, 7);
    } else if (m_version == "v50") {
        m_numChannels = 8;
        setIntegerParam(Supported, 1);
        createParams_v50();
        setExpectedVersion(5, 0);
    } else if (m_version == "v51") {
        setIntegerParam(Supported, 1);
        createParams_v51();
        setExpectedVersion(5, 1);
    } else if (m_version == "v52") {
        setIntegerParam(Supported, 1);
        createParams_v52();
        setExpectedVersion(5, 2);
    } else if (m_version == "v54") {
        setIntegerParam(Supported, 1);
        createParams_v54();
        m_cmdHandlers[DasCmdPacket::CMD_READ_CONFIG].second = std::bind(&RocPlugin::rspReadConfigV54, this, std::placeholders::_1);
        setExpectedVersion(5, 4);
    } else if (m_version == "v55") {
        setIntegerParam(Supported, 1);
        createParams_v54();
        setExpectedVersion(5, 5);
    } else if (m_version == "v56") {
        setIntegerParam(Supported, 1);
        createParams_v56();
        setExpectedVersion(5, 6);
    } else if (m_version == "v57") {
        setIntegerParam(Supported, 1);
        createParams_v57();
        setExpectedVersion(5, 7);
    } else if (m_version == "v58") {
        setIntegerParam(Supported, 1);
        createParams_v58();
        setExpectedVersion(5, 8);
        havePreAmpTest = true;
    } else if (m_version == "v59") {
        setIntegerParam(Supported, 1);
        createParams_v59();
        setExpectedVersion(5, 9);
        havePreAmpTest = true;
    } else if (m_version == "v510") {
        setIntegerParam(Supported, 1);
        createParams_v510();
        setExpectedVersion(5, 10);
        havePreAmpTest = true;
    } else if (m_version == "v511") {
        setIntegerParam(Supported, 1);
        createParams_v511();
        setExpectedVersion(5, 11);
        havePreAmpTest = true;
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ROC version '%s'", version);
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

asynStatus RocPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (m_numChannels > 0 && pasynUser->reason == CmdReq) {
        if (value == DasCmdPacket::CMD_WRITE_CONFIG ||
            value == DasCmdPacket::CMD_READ_CONFIG ||
            value == DasCmdPacket::CMD_READ_STATUS) {

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

bool RocPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    memset(&version, 0, sizeof(BaseModulePlugin::Version));

    if (packet->getCmdPayloadLength() == sizeof(RspReadVersionGE)) {
        const RspReadVersionGE *response = reinterpret_cast<const RspReadVersionGE*>(packet->getCmdPayload());

        version.hw_version  = response->hw_version;
        version.hw_revision = response->hw_revision;
        version.fw_version  = response->fw_version;
        version.fw_revision = response->fw_revision;

        return true;

    } else if (packet->getCmdPayloadLength() == sizeof(RspReadVersion) ||
               packet->getCmdPayloadLength() == sizeof(RspReadVersion_v54)) {

        const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getCmdPayload());

        version.hw_version  = response->hw_version;
        version.hw_revision = response->hw_revision;
        version.fw_version  = response->fw_version;
        version.fw_revision = response->fw_revision;
        version.fw_year     = HEX_BYTE_TO_DEC(response->year >> 8) * 100 + HEX_BYTE_TO_DEC(response->year);
        version.fw_month    = HEX_BYTE_TO_DEC(response->month);
        version.fw_day      = HEX_BYTE_TO_DEC(response->day);
        // Skip vendor id from V5.4

        return true;
    }

    return false;
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
    size_t len = packRegParams("CONFIG", data, sizeof(data), m_expectedChannel, 0);
    if (len == 0) {
        LOG_WARN("Skipping sending write config packet");
        return false;
    }

    auto channel = m_expectedChannel;
    if (channel > 0)
        channel |= 0x10;

    sendUpstream(DasCmdPacket::CMD_WRITE_CONFIG, channel, data, len);
    return true;
}

bool RocPlugin::rspWriteConfig(const DasCmdPacket* packet)
{
    if (!packet->isAcknowledge())
        return false;

    auto channel = m_expectedChannel;
    if (channel > 0)
        channel |= 0x10;

    // Turns back to 0 after last channel, works also for m_numChannels == 0
    m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);

    if (packet->getCmdId() != channel) {
        LOG_WARN("Expecting WRITE_CONFIG response for channel %u, received channel %u", channel & 0xF, packet->getCmdId() & 0xF);
        return false;
    }

    return true;
}

bool RocPlugin::reqParamsChan(DasCmdPacket::CommandType command)
{
    auto channel = m_expectedChannel;
    if (channel > 0)
        channel |= 0x10;
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
    if (m_params[params].sizes.size() > 1) {
        section = SECTION_ID(0xF, m_expectedChannel);
        expectLength = ALIGN_UP((m_params["CONFIG"].offsets[section] + m_params["CONFIG"].sizes[section])*wordSize, 4);
    }

    auto channel = m_expectedChannel;
    if (channel > 0)
        channel |= 0x10;

    // Turns back to 0 after last channel, works also for m_numChannels == 0
    m_expectedChannel = (m_expectedChannel + 1) % (m_numChannels + 1);

    if (packet->getCmdId() != channel) {
        LOG_WARN("Expecting %s response for channel %u, received channel %u", params.c_str(), channel & 0xF, packet->getCmdId() & 0xF);
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
    if (m_version == "v54") {

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
    }

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
