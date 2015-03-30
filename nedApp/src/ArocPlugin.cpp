/* ArocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "ArocPlugin.h"
#include "Log.h"
#include "Common.h"

EPICS_REGISTER_PLUGIN(ArocPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned ArocPlugin::NUM_ROCPLUGIN_DYNPARAMS       = 400;  //!< Since supporting multiple versions with different number of PVs, this is just a maximum value

/**
 * AROC version response format
 */
struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     //!< Board revision number
    unsigned hw_version:8;      //!< Board version number
    unsigned fw_revision:8;     //!< Firmware revision number
    unsigned fw_version:8;      //!< Firmware version number
    unsigned year:16;           //!< Year
    unsigned day:8;             //!< Day
    unsigned month:8;           //!< Month
#else
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};

ArocPlugin::ArocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_AROC, true, blocking,
                       NUM_ROCPLUGIN_DYNPARAMS, defaultInterfaceMask, defaultInterruptMask)
    , m_version(version)
{
    if (m_version == "v23") {
        setIntegerParam(Supported, 1);
        createStatusParams_v23();
        createConfigParams_v23();
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported AROC version '%s'", version);
    }

    LOG_DEBUG("Number of configured dynamic parameters: %zu", m_statusParams.size() + m_configParams.size());

    callParamCallbacks();
}

bool ArocPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getPayloadLength() != sizeof(RspReadVersion))
        return false;

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion *>(packet->getPayload());

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

bool ArocPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if (version.hw_version == 2) {
        char ver[10];
        snprintf(ver, sizeof(ver), "v%u%u", version.fw_version, version.fw_revision);
        if (m_version == ver)
            return true;
    }

    return false;
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
