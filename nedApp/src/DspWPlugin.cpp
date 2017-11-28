/* DspWPlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "DspWPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(DspWPlugin, 4, "Port name", string, "Parent plugins", string, "Hardware ID", string, "Version", string);

const double DspWPlugin::DSPW_RESPONSE_TIMEOUT       = 1.0;

/**
 * DSP-W version response format
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

DspWPlugin::DspWPlugin(const char *portName, const char *parentPlugins, const char *hardwareId, const char *version)
    : BaseModulePlugin(portName, parentPlugins, hardwareId, DasCmdPacket::MOD_TYPE_DSPW, 4)
    , m_version(version)
{
    if (m_version == "v10") {
        createParams_v10();
        setIntegerParam(Supported, 1);
        setExpectedVersion(1, 0);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported DSP-W version '%s'", version);
    }

    callParamCallbacks();
    initParams();
}

bool DspWPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getPayloadLength() != sizeof(RspReadVersion)) {
        return false;
    }

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->payload);
    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = 0;
    version.hw_month    = 0;
    version.hw_day      = 0;
    version.fw_version  = response->fw_version;
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->year) + 2000;
    version.fw_month    = HEX_BYTE_TO_DEC(response->month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->day);

    return true;
}
