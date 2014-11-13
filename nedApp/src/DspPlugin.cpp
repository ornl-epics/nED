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

const unsigned DspPlugin::NUM_DSPPLUGIN_CONFIGPARAMS    = 272;
const unsigned DspPlugin::NUM_DSPPLUGIN_STATUSPARAMS    = 100;
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
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, false,
                       blocking, NUM_DSPPLUGIN_PARAMS + NUM_DSPPLUGIN_CONFIGPARAMS + NUM_DSPPLUGIN_CONFIGPARAMS)
    , m_version(version)
{
    if (m_configParams.size() != NUM_DSPPLUGIN_CONFIGPARAMS) {
        LOG_ERROR("Number of config params mismatch, expected %d but got %lu", NUM_DSPPLUGIN_CONFIGPARAMS, m_configParams.size());
    }

    setIntegerParam(HwType, DasPacket::MOD_TYPE_DSP);
    if (m_version == "v63") {
        createConfigParams_v63();
        createStatusParams_v63();
        setIntegerParam(Supported, 1);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported DSP version '%s'", version);
    }

    callParamCallbacks();
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

bool DspPlugin::rspDiscover(const DasPacket *packet)
{
    return (BaseModulePlugin::rspDiscover(packet) &&
            packet->cmdinfo.module_type == DasPacket::MOD_TYPE_DSP);
}

bool DspPlugin::rspReadVersion(const DasPacket *packet)
{
    char date[20];

    if (!BaseModulePlugin::rspReadVersion(packet))
        return false;

    BaseModulePlugin::Version version;
    if (!parseVersionRsp(packet, version)) {
        LOG_WARN("Bad READ_VERSION response");
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

    if (version.hw_version == 2 && version.hw_revision == 4 && version.fw_version == 6 && version.fw_revision == 3 && m_version == "v63") {
        return true;
    }

    LOG_WARN("Unsupported DSP version");
    return false;
}
