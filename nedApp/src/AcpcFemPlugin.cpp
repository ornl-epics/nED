/* AcpcFemPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcFemPlugin.h"
#include "Log.h"

#define NUM_ACPCFEMPLUGIN_PARAMS    0 //((int)(&LAST_ACPCFEMPLUGIN_PARAM - &FIRST_ACPCFEMPLUGIN_PARAM + 1))
#define HEX_BYTE_TO_DEC(a)      ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

EPICS_REGISTER_PLUGIN(AcpcFemPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned AcpcFemPlugin::NUM_ACPCFEMPLUGIN_DYNPARAMS       = 250;

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


AcpcFemPlugin::AcpcFemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_ACPCFEM, true,
                       blocking, NUM_ACPCFEMPLUGIN_PARAMS + NUM_ACPCFEMPLUGIN_DYNPARAMS)
    , m_version(version)
{
    createStatusParams_v22();

    setIntegerParam(Supported, 1);

    LOG_DEBUG("Number of configured dynamic parameters: %zu", m_statusParams.size() + m_configParams.size());

    callParamCallbacks();
}

bool AcpcFemPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if (version.hw_version == 2 && version.hw_revision == 5 && version.fw_version == 2 && version.fw_revision == 2)
        return true;

    return false;
}

bool AcpcFemPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
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
