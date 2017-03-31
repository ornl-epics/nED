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

EPICS_REGISTER_PLUGIN(DspWPlugin, 4, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Version", string);

const unsigned DspWPlugin::NUM_DSPWPLUGIN_PARAMS     = 100; // + ((int)(&LAST_DSPWPLUGIN_PARAM - &FIRST_DSPWPLUGIN_PARAM + 1))
const double DspWPlugin::DSPW_RESPONSE_TIMEOUT       = 1.0;

/**
 * DSP-W version response format
 */
struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    struct {
        unsigned day:8;
        unsigned month:8;
        unsigned year:8;
        unsigned revision:4;
        unsigned version:4;
    } hardware;
    struct {
        unsigned day:8;
        unsigned month:8;
        unsigned year:8;
        unsigned revision:4;
        unsigned version:4;
    } firmware;
#else
#error Missing DspWVersionRegister declaration
#endif
};

DspWPlugin::DspWPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_DSPW, false,
                       0, NUM_DSPWPLUGIN_PARAMS)
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

bool DspWPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getPayloadLength() != sizeof(RspReadVersion)) {
        return false;
    }

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->payload);
    version.hw_version  = response->hardware.version;
    version.hw_revision = response->hardware.revision;
    version.hw_year     = HEX_BYTE_TO_DEC(response->hardware.year) + 2000;
    version.hw_month    = HEX_BYTE_TO_DEC(response->hardware.month);
    version.hw_day      = HEX_BYTE_TO_DEC(response->hardware.day);
    version.fw_version  = response->firmware.version;
    version.fw_revision = response->firmware.revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->firmware.year) + 2000;
    version.fw_month    = HEX_BYTE_TO_DEC(response->firmware.month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->firmware.day);

    return true;
}
