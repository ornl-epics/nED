/* AcpcPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"
#include "Common.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(AcpcPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned AcpcPlugin::NUM_ACPCPLUGIN_DYNPARAMS       = 260;  //!< Since supporting multiple versions with different number of PVs, this is just a maximum value

/**
 * ACPC version response format
 */
struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     //!< Board revision number
    unsigned hw_version:8;      //!< Board version number
    unsigned hw_year:16;        //!< Board year
    unsigned hw_day:8;          //!< Board day
    unsigned hw_month:8;        //!< Board month
    unsigned fw_revision:8;     //!< Firmware revision number
    unsigned fw_version:8;      //!< Firmware version number
    unsigned fw_year:16;        //!< Firmware year
    unsigned fw_day:8;          //!< Firmware day
    unsigned fw_month:8;        //!< Firmware month
    unsigned filler1:32;        //!< Not used
    unsigned filler2:32;        //!< Not used
#else
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};

AcpcPlugin::AcpcPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_ACPC, true, blocking,
                       NUM_ACPCPLUGIN_DYNPARAMS, defaultInterfaceMask, defaultInterruptMask)
    , m_version(version)
{
    if (0) {
    } else if (m_version == "v144") {
        setIntegerParam(Supported, 1);
        createStatusParams_v144();
        createConfigParams_v144();
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ACPC version '%s'", version);
    }

    LOG_DEBUG("Number of configured dynamic parameters: %zu", m_statusParams.size() + m_configParams.size());

    callParamCallbacks();
}

bool AcpcPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if (version.hw_version == 4) {
        char ver[10];
        snprintf(ver, sizeof(ver), "v%u%u", version.fw_version, version.fw_revision);
        if (m_version == ver)
            return true;
    }

    return false;
}

bool AcpcPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getPayloadLength() != sizeof(RspReadVersion))
        return false;

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getPayload());

    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = HEX_BYTE_TO_DEC(response->hw_year) + 2000;
    version.hw_month    = HEX_BYTE_TO_DEC(response->hw_month);
    version.hw_day      = HEX_BYTE_TO_DEC(response->hw_day);
    version.fw_version  = response->fw_version;
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->fw_year) + 2000;
    version.fw_month    = HEX_BYTE_TO_DEC(response->fw_month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->fw_day);

    return true;
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
