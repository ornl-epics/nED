/* DspPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspPlugin.h"
#include "Log.h"

#include <epicsAlgorithm.h>
#include <osiSock.h>
#include <string.h>

#include <functional>
#include <string>

#define NUM_DSPPLUGIN_PARAMS    0 // ((int)(&LAST_DSPPLUGIN_PARAM - &FIRST_DSPPLUGIN_PARAM + 1))
#define HEX_BYTE_TO_DEC(a)      ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

EPICS_REGISTER_PLUGIN(DspPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Version", string, "Blocking", int);

const unsigned DspPlugin::NUM_DSPPLUGIN_CONFIGPARAMS    = 472;
const unsigned DspPlugin::NUM_DSPPLUGIN_STATUSPARAMS    = 116;
const double DspPlugin::DSP_RESPONSE_TIMEOUT            = 1.0;

/**
 * DSP 6.x version response format
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
    uint32_t eeprom_code1;
    uint32_t eeprom_code2;
#else
#error Missing DspVersionRegister declaration
#endif
};

DspPlugin::DspPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_DSP, false,
                       blocking, NUM_DSPPLUGIN_PARAMS + NUM_DSPPLUGIN_CONFIGPARAMS + NUM_DSPPLUGIN_STATUSPARAMS)
    , m_version(version)
{
    if (m_version == "v63") {
        createParams_v63();
        setIntegerParam(Supported, 1);
    } else if (m_version == "v64") {
        createParams_v64();
        setIntegerParam(Supported, 1);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported DSP version '%s'", version);
    }

    callParamCallbacks();
    initParams();
}

bool DspPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
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

bool DspPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if (version.hw_version == 2 && version.hw_revision == 4) {
        if (version.fw_version == 6 && version.fw_revision == 3 && m_version == "v63")
            return true;
        if (version.fw_version == 6 && version.fw_revision == 4 && m_version == "v64")
            return true;
    }

    return false;
}
