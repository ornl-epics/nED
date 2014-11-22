/* FemPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FemPlugin.h"
#include "Log.h"

#define NUM_FEMPLUGIN_PARAMS    0 //((int)(&LAST_FEMPLUGIN_PARAM - &FIRST_FEMPLUGIN_PARAM + 1))
#define HEX_BYTE_TO_DEC(a)      ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

EPICS_REGISTER_PLUGIN(FemPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned FemPlugin::NUM_FEMPLUGIN_DYNPARAMS       = 290;

struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     // Board revision number
    unsigned hw_version:8;      // Board version number
    unsigned hw_year:16;        // Board year
    unsigned hw_day:8;          // Board day
    unsigned hw_month:8;        // Board month
    unsigned fw_revision:8;     // Firmware revision number
    unsigned fw_version:8;      // Firmware version number
    unsigned fw_year:16;        // Firmware year
    unsigned fw_day:8;          // Firmware day
    unsigned fw_month:8;        // Firmware month
#else
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};

FemPlugin::FemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, true,
                       blocking, NUM_FEMPLUGIN_PARAMS + NUM_FEMPLUGIN_DYNPARAMS)
    , m_version(version)
{
    if (m_version == "v32" || m_version == "v34") {
        setIntegerParam(Supported, 1);
        createStatusParams_v32();
        createConfigParams_v32();
    } else if (m_version == "v35") {
        setIntegerParam(Supported, 1);
        createStatusParams_v35();
        createConfigParams_v35();
/* TODO: FEM interface not yet finalized
    } else if (m_version == "v36") {
        setIntegerParam(Supported, 1);
        createStatusParams_v36();
        createConfigParams_v36();
*/
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported FEM version '%s'", version);
    }

    LOG_DEBUG("Number of configured dynamic parameters: %zu", m_statusParams.size() + m_configParams.size());

    callParamCallbacks();
}

bool FemPlugin::rspDiscover(const DasPacket *packet)
{
    return (BaseModulePlugin::rspDiscover(packet) &&
            packet->cmdinfo.module_type == DasPacket::MOD_TYPE_FEM);
}

bool FemPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getPayload());

    if (packet->getPayloadLength() != sizeof(RspReadVersion)) {
        return false;
    }

    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = response->hw_year;
    version.hw_month    = HEX_BYTE_TO_DEC(response->hw_year >> 8) * 100 + HEX_BYTE_TO_DEC(response->hw_year);
    version.hw_day      = HEX_BYTE_TO_DEC(response->hw_day);
    version.fw_version  = HEX_BYTE_TO_DEC(response->fw_version);
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->fw_year >> 8) * 100 + HEX_BYTE_TO_DEC(response->fw_year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->fw_month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->fw_day);
    return true;
}

bool FemPlugin::rspReadVersion(const DasPacket *packet)
{
    char date[20];
    BaseModulePlugin::Version version;

    if (!BaseModulePlugin::rspReadVersion(packet))
        return false;

    if (!parseVersionRsp(packet, version)) {
        LOG_ERROR("Received unexpected READ_VERSION response for this FEM type");
        return false;
    }

    setIntegerParam(HwVer, version.hw_version);
    setIntegerParam(HwRev, version.hw_revision);
    snprintf(date, sizeof(date), "%04d/%02d/%02d", version.hw_year, version.hw_month, version.hw_day);
    setStringParam(HwDate, date);
    setIntegerParam(FwVer, version.fw_version);
    setIntegerParam(FwRev, version.fw_revision);
    snprintf(date, sizeof(date), "%04d/%02d/%02d", version.fw_year, version.fw_month, version.fw_day);
    setStringParam(FwDate, date);

    callParamCallbacks();

    if ((version.hw_version == 10 && version.hw_revision == 2) ||
        (version.hw_version == 10 && version.hw_revision == 9)) {
        char ver[10];
        snprintf(ver, sizeof(ver), "v%u%u", version.fw_version, version.fw_revision);
        if (m_version == ver)
            return true;
    }

    LOG_WARN("Unsupported ROC version");
    return false;
}
