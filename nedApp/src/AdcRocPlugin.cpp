/* AdcRocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "AdcRocPlugin.h"
#include "Log.h"

#include <cstring>

#define HEX_BYTE_TO_DEC(a)      ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

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
        createStatusParams_v02();
        createConfigParams_v02();
        createCounterParams_v02();
    } else if (m_version == "v03") {
        setIntegerParam(Supported, 1);
        createStatusParams_v03();
        createConfigParams_v03();
        createCounterParams_v03();
    }  else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ADC ROC version '%s'", version);
    }

    callParamCallbacks();
    initParams();
}

bool AdcRocPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if (version.hw_version == 0) {
        char ver[10];
        snprintf(ver, sizeof(ver), "v%u%u", version.fw_version, version.fw_revision);
        if (m_version == ver)
            return true;
    }

    return false;
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

DasPacket::CommandType AdcRocPlugin::reqTriggerPulse()
{
    sendToDispatcher(DasPacket::CMD_PM_PULSE_RQST_ON);
    return DasPacket::CMD_PM_PULSE_RQST_ON;
}

DasPacket::CommandType AdcRocPlugin::reqClearPulse()
{
    sendToDispatcher(DasPacket::CMD_PM_PULSE_RQST_OFF);
    return DasPacket::CMD_PM_PULSE_RQST_OFF;
}


// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
