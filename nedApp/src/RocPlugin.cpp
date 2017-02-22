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

EPICS_REGISTER_PLUGIN(RocPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned RocPlugin::NUM_ROCPLUGIN_DYNPARAMS       = 670;  //!< Since supporting multiple versions with different number of PVs, this is just a maximum value

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

RocPlugin::RocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_ROC, true, blocking,
                       NUM_ROCPLUGIN_DYNPARAMS, defaultInterfaceMask, defaultInterruptMask)
    , m_version(version)
{
    if (0) {
    } else if (m_version == "v14" || m_version == "v14") {
        setNumChannels(8);
        setIntegerParam(Supported, 1);
        createParams_v14();
        setExpectedVersion(1, 4);
    } else if (m_version == "v44") {
        setNumChannels(8);
        setIntegerParam(Supported, 1);
        createParams_v45();
        setExpectedVersion(4, 4);
    } else if (m_version == "v45") {
        setNumChannels(8);
        setIntegerParam(Supported, 1);
        createParams_v45();
        setExpectedVersion(4, 5);
    } else if (m_version == "v47") {
        setNumChannels(8);
        setIntegerParam(Supported, 1);
        createParams_v47();
        setExpectedVersion(4, 7);
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
        createParam("Acquiring", asynParamInt32, &Acquiring); // v5.4 doesn't support Acquiring through registers, we simulate by receiving ACK on START
        setExpectedVersion(5, 4);
    } else if (m_version == "v55") {
        setIntegerParam(Supported, 1);
        createParams_v54();
        createParam("Acquiring", asynParamInt32, &Acquiring); // v5.4 doesn't support Acquiring through registers, we simulate by receiving ACK on START
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
    } else if (m_version == "v1515") {
        setIntegerParam(Supported, 1);
        createParams_v1515();
        setExpectedVersion(15, 15);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ROC version '%s'", version);
    }

    createParam("HvDelay",      asynParamFloat64, &HvDelay,     0.0); // READ - Time from HV request to first response character
    createParam("HvB2bDelay",   asynParamFloat64, &HvB2bDelay,  0.0); // READ - Time from HV request to last response character

    callParamCallbacks();
    initParams();
}

bool RocPlugin::processResponse(const DasPacket *packet)
{
    DasPacket::CommandType command = packet->getResponseType();

    // Once HV command is initiated with CMD_HV_SEND, ROC board first ACKs the CMD_HV_SEND.
    // Train of CMD_HV_RECV packets follow, one character from response per packet.
    // Number ov CMD_HV_RECV packets is dynamic, depending on the response length.
    switch (command) {
    case DasPacket::CMD_HV_SEND:
        return asynSuccess;
    case DasPacket::CMD_HV_RECV:
        rspHvCmd(packet);
        return asynSuccess;
    default:
        return BaseModulePlugin::processResponse(packet);
    }
}

DasPacket::CommandType RocPlugin::handleRequest(DasPacket::CommandType command, double &timeout)
{
    switch (command) {
    case DasPacket::CMD_PREAMP_TEST_CONFIG:
        return reqConfigPreAmp();
    case DasPacket::CMD_PREAMP_TEST_TRIGGER:
        return reqTriggerPreAmp();
    default:
        return BaseModulePlugin::handleRequest(command, timeout);
    }
}

bool RocPlugin::handleResponse(const DasPacket *packet)
{
    DasPacket::CommandType command = packet->getResponseType();

    switch (command) {
    case DasPacket::CMD_PREAMP_TEST_CONFIG:
        return rspConfigPreAmp(packet);
    case DasPacket::CMD_PREAMP_TEST_TRIGGER:
        return rspTriggerPreAmp(packet);
    default:
        return BaseModulePlugin::handleResponse(packet);
    }
}

asynStatus RocPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    // Only serving StreamDevice - puts reason as -1
    if (pasynUser->reason == -1) {
        // StreamDevice is sending entire string => no need to buffer the request.
        reqHvCmd(value, nChars);
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

        this->lock();

        return status;
    }
    return BaseModulePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

bool RocPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    memset(&version, 0, sizeof(BaseModulePlugin::Version));

    if (packet->getPayloadLength() == sizeof(RspReadVersionGE)) {
        const RspReadVersionGE *response = reinterpret_cast<const RspReadVersionGE*>(packet->getPayload());

        version.hw_version  = response->hw_version;
        version.hw_revision = response->hw_revision;
        version.fw_version  = response->fw_version;
        version.fw_revision = response->fw_revision;

        return true;

    } else if (packet->getPayloadLength() == sizeof(RspReadVersion) ||
               packet->getPayloadLength() == sizeof(RspReadVersion_v54)) {

        const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getPayload());

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

bool RocPlugin::rspReadConfig(const DasPacket *packet, uint8_t channel)
{
    if (m_version == "v54") {
        uint8_t buffer[480]; // actual size of the READ_CONFIG v5.4 packet

        if (packet->getLength() > sizeof(buffer)) {
            LOG_ERROR("Received v5.4 READ_CONFIG response bigger than expected");
            return asynError;
        }

        // Packet in shared queue must not be modified. So we make a copy.
        memcpy(buffer, packet, packet->getLength());
        packet = reinterpret_cast<const DasPacket *>(buffer);
        const_cast<DasPacket *>(packet)->payload_length -= 4; // This is the only reason we're doing all the buffering
    }

    return BaseModulePlugin::rspReadConfig(packet, channel);
}

bool RocPlugin::rspStart(const DasPacket *packet)
{
    bool ack = BaseModulePlugin::rspStart(packet);
    if (m_version == "v54" || m_version == "v55") {
        setIntegerParam(Acquiring, (ack ? 1 : 0));
        callParamCallbacks();
    }
    return ack;
}

bool RocPlugin::rspStop(const DasPacket *packet)
{
    bool ack = BaseModulePlugin::rspStop(packet);
    if (m_version == "v54" || m_version == "v55") {
        setIntegerParam(Acquiring, (ack ? 0 : 1));
        callParamCallbacks();
    }
    return ack;
}

void RocPlugin::reqHvCmd(const char *data, uint32_t length)
{
    uint32_t buffer[32] = { 0 }; // Initialize all to 0
    uint32_t bufferLen = length * 2;

    // Every character in protocol needs to be prefixed with a zero byte when sent over OCC
    for (uint32_t i = 0; i < length; i++) {
        buffer[i/2] |= data[i] << (16*(i%2));
    }
    sendToDispatcher(DasPacket::CMD_HV_SEND, 0, buffer, bufferLen);

    epicsTimeGetCurrent(&m_sendHvTime);
}

bool RocPlugin::rspHvCmd(const DasPacket *packet)
{
    const uint32_t *payload = packet->getPayload();
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

DasPacket::CommandType RocPlugin::reqConfigPreAmp()
{
    uint32_t buffer[128];
    uint32_t length = packRegParams("PREAMP_CFG", buffer, sizeof(buffer));

    if (length == 0)
        return static_cast<DasPacket::CommandType>(0);

    sendToDispatcher(DasPacket::CMD_PREAMP_TEST_CONFIG, 0, buffer, length);
    return DasPacket::CMD_PREAMP_TEST_CONFIG;
}

DasPacket::CommandType RocPlugin::reqTriggerPreAmp()
{
    uint32_t recharge = 0xFFFF;
    uint32_t buffer[128];
    uint32_t length = packRegParams("PREAMP_TRIG", buffer, sizeof(buffer));

    if (length == 0)
        return static_cast<DasPacket::CommandType>(0);

    sendToDispatcher(DasPacket::CMD_PREAMP_TEST_TRIGGER, 0, &recharge, length);
    sendToDispatcher(DasPacket::CMD_PREAMP_TEST_TRIGGER, 0, buffer, length);
    return DasPacket::CMD_PREAMP_TEST_TRIGGER;
}

bool RocPlugin::rspConfigPreAmp(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

bool RocPlugin::rspTriggerPreAmp(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
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
