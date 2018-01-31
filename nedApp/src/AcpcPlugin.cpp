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

EPICS_REGISTER_PLUGIN(AcpcPlugin, 4, "Port name", string, "Parent plugins", string, "Hardware ID", string, "Hw & SW version", string);

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

AcpcPlugin::AcpcPlugin(const char *portName, const char *parentPlugins, const char *hardwareId, const char *version)
    : BaseModulePlugin(portName, parentPlugins, hardwareId, DasCmdPacket::MOD_TYPE_ACPC, 2)
    , m_version(version)
{
    if (0) {
    } else if (m_version == "v144") {
        setIntegerParam(Supported, 1);
        createParams_v144();
        setExpectedVersion(14, 4);
    } else if (m_version == "v171") {
        setIntegerParam(Supported, 1);
        createParams_v171();
        setExpectedVersion(17, 1);
    } else if (m_version == "v172") {
        setIntegerParam(Supported, 1);
        createParams_v172();
        setExpectedVersion(17, 2);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ACPC version '%s'", version);
    }

    callParamCallbacks();
    initParams();
}

bool AcpcPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    if ((packet->getLength() - sizeof(DasCmdPacket)) != sizeof(RspReadVersion))
        return false;

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getCmdPayload());

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
