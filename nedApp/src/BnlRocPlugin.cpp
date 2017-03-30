/* BnlRocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "Common.h"
#include "BnlRocPlugin.h"
#include "Log.h"

#include <cstring>

EPICS_REGISTER_PLUGIN(BnlRocPlugin, 5, "Port name", string,
        "Dispatcher port name", string, "Hardware ID", string,
        "Hw & SW version", string, "Blocking", int);

/* Since supporting multiple versions with different number of PVs, this is
 * just a maximum value. */
const unsigned BnlRocPlugin::NUM_BNLROCPLUGIN_DYNPARAMS       = 650;

/**
 * BNL ROC version response format
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

BnlRocPlugin::BnlRocPlugin(const char *portName, const char *dispatcherPortName,
        const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId,
            DasPacket::MOD_TYPE_BNLROC, true, blocking,
            NUM_BNLROCPLUGIN_DYNPARAMS, defaultInterfaceMask,
            defaultInterruptMask)
    , m_version(version)
{
    if (0) {
    } else if (m_version == "v00") {
        setIntegerParam(Supported, 1);
        createParams_v00();
        setExpectedVersion(0, 0);
    } else if (m_version == "v20") {
        setIntegerParam(Supported, 1);
        createParams_v20();
        setExpectedVersion(2, 0);
    } else if (m_version == "v21") {
        setIntegerParam(Supported, 1);
        createParams_v20();
        setExpectedVersion(2, 1);
    }  else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported BNL ROC version '%s'", version);
        return;
    }

    callParamCallbacks();
    initParams();
}

bool BnlRocPlugin::parseVersionRsp(const DasPacket *packet,
        BaseModulePlugin::Version &version)
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
    version.fw_year     = HEX_BYTE_TO_DEC(response->year >> 8) * 100 +
                            HEX_BYTE_TO_DEC(response->year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->day);

    return true;
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in
// custom files for two reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
