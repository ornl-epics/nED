/* AdcRocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "Common.h"
#include "AdcRocPlugin.h"
#include "Log.h"

#include <cstring>

EPICS_REGISTER_PLUGIN(AdcRocPlugin, 3, "Port name", string, "Parent plugins", string, "Hw & SW version", string);

/**
 * ADC ROC version response format
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
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};

AdcRocPlugin::AdcRocPlugin(const char *portName, const char *parentPlugins, const char *version)
    : BaseModulePlugin(portName, parentPlugins, DasCmdPacket::MOD_TYPE_ADCROC, 2)
    , m_version(version)
{
    if (0) {
    } else if (m_version == "v02") {
        setIntegerParam(Supported, 1);
        createParams_v02();
        setExpectedVersion(0, 2);
    } else if (m_version == "v03") {
        setIntegerParam(Supported, 1);
        createParams_v03();
        setExpectedVersion(0, 3);
    } else if (m_version == "v05") {
        setIntegerParam(Supported, 1);
        createParams_v05();
        setExpectedVersion(0, 5);
    } else if (m_version == "v06") {
        setIntegerParam(Supported, 1);
        createParams_v06();
        setExpectedVersion(0, 6);
    } else if (m_version == "v07") {
        setIntegerParam(Supported, 1);
        createParams_v07();
        setExpectedVersion(0, 7);
    }  else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ADC ROC version '%s'", version);
        return;
    }

    callParamCallbacks();
    initParams();
}

DasCmdPacket::CommandType AdcRocPlugin::handleRequest(DasCmdPacket::CommandType command, double &timeout)
{
    switch (command) {
    case DasCmdPacket::CMD_PM_PULSE_RQST_ON:
        return reqTriggerPulseMagnet();
    case DasCmdPacket::CMD_PM_PULSE_RQST_OFF:
        return reqClearPulseMagnet();
    default:
        return BaseModulePlugin::handleRequest(command, timeout);
    }
}

bool AdcRocPlugin::handleResponse(const DasCmdPacket *packet)
{
    switch (packet->getCommand()) {
    case DasCmdPacket::CMD_PM_PULSE_RQST_ON:
        return rspTriggerPulseMagnet(packet);
    case DasCmdPacket::CMD_PM_PULSE_RQST_OFF:
        return rspClearPulseMagnet(packet);
    default:
        return BaseModulePlugin::handleResponse(packet);
    }
}

bool AdcRocPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    const RspReadVersion *response;
    if (packet->getCmdPayloadLength() == sizeof(RspReadVersion)) {
        response = reinterpret_cast<const RspReadVersion*>(packet->getCmdPayload());
    } else {
        return false;
    }
    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = 0;
    version.hw_month    = 0;
    version.hw_day      = 0;
    version.fw_version  = response->fw_version;
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->year >> 8) * 100 + HEX_BYTE_TO_DEC(response->year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->day);

    return true;
}

DasCmdPacket::CommandType AdcRocPlugin::reqTriggerPulseMagnet()
{
    sendUpstream(DasCmdPacket::CMD_PM_PULSE_RQST_ON);
    return DasCmdPacket::CMD_PM_PULSE_RQST_ON;
}

DasCmdPacket::CommandType AdcRocPlugin::reqClearPulseMagnet()
{
    sendUpstream(DasCmdPacket::CMD_PM_PULSE_RQST_OFF);
    return DasCmdPacket::CMD_PM_PULSE_RQST_OFF;
}

bool AdcRocPlugin::rspTriggerPulseMagnet(const DasCmdPacket *packet)
{
    return packet->isAcknowledge();
}

bool AdcRocPlugin::rspClearPulseMagnet(const DasCmdPacket *packet)
{
    return packet->isAcknowledge();
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
