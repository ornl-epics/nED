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

EPICS_REGISTER_PLUGIN(AdcRocPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned AdcRocPlugin::NUM_ADCROCPLUGIN_DYNPARAMS       = 650;  //!< Since supporting multiple versions with different number of PVs, this is just a maximum value

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

AdcRocPlugin::AdcRocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_ADCROC, true, blocking,
                       NUM_ADCROCPLUGIN_DYNPARAMS, defaultInterfaceMask, defaultInterruptMask)
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
    }  else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ADC ROC version '%s'", version);
        return;
    }

    callParamCallbacks();
    initParams();
}

DasPacket::CommandType AdcRocPlugin::handleRequest(DasPacket::CommandType command, double &timeout)
{
    switch (command) {
    case DasPacket::CMD_PM_PULSE_RQST_ON:
        return reqTriggerPulseMagnet();
    case DasPacket::CMD_PM_PULSE_RQST_OFF:
        return reqClearPulseMagnet();
    default:
        return BaseModulePlugin::handleRequest(command, timeout);
    }
}

bool AdcRocPlugin::handleResponse(const DasPacket *packet)
{
    DasPacket::CommandType command = packet->getResponseType();

    switch (command) {
    case DasPacket::CMD_PM_PULSE_RQST_ON:
        return rspTriggerPulseMagnet(packet);
    case DasPacket::CMD_PM_PULSE_RQST_OFF:
        return rspClearPulseMagnet(packet);
    default:
        return BaseModulePlugin::handleResponse(packet);
    }
}

bool AdcRocPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    const RspReadVersion *response;
    if (packet->getPayloadLength() == sizeof(RspReadVersion)) {
        response = reinterpret_cast<const RspReadVersion*>(packet->getPayload());
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

DasPacket::CommandType AdcRocPlugin::reqTriggerPulseMagnet()
{
    sendToDispatcher(DasPacket::CMD_PM_PULSE_RQST_ON);
    return DasPacket::CMD_PM_PULSE_RQST_ON;
}

DasPacket::CommandType AdcRocPlugin::reqClearPulseMagnet()
{
    sendToDispatcher(DasPacket::CMD_PM_PULSE_RQST_OFF);
    return DasPacket::CMD_PM_PULSE_RQST_OFF;
}

bool AdcRocPlugin::rspTriggerPulseMagnet(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

bool AdcRocPlugin::rspClearPulseMagnet(const DasPacket *packet)
{
    return (packet->cmdinfo.command == DasPacket::RSP_ACK);
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
