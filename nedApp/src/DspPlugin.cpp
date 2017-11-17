/* DspPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "DspPlugin.h"
#include "Log.h"

#include <epicsAlgorithm.h>
#include <osiSock.h>
#include <string.h>

#include <functional>
#include <string>

EPICS_REGISTER_PLUGIN(DspPlugin, 4, "Port name", string, "Parent plugins", string, "Hardware ID", string, "Version", string);

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

DspPlugin::DspPlugin(const char *portName, const char *parentPlugins, const char *hardwareId, const char *version)
    : BaseModulePlugin(portName, parentPlugins, hardwareId, DasCmdPacket::MOD_TYPE_DSP, false)
    , m_version(version)
{
    if (m_version == "v51") {
        createParams_v51();
        setIntegerParam(Supported, 1);
        setExpectedVersion(5, 1);
    } else if (m_version == "v52") {
        createParams_v52();
        setIntegerParam(Supported, 1);
        setExpectedVersion(5, 2);
    } else if (m_version == "v63") {
        createParams_v63();
        setIntegerParam(Supported, 1);
        setExpectedVersion(6, 3);
    } else if (m_version == "v64") {
        createParams_v64();
        setIntegerParam(Supported, 1);
        setExpectedVersion(6, 4);
    } else if (m_version == "v65") {
        createParams_v65();
        setIntegerParam(Supported, 1);
        setExpectedVersion(6, 5);
    } else if (m_version == "v66") {
        createParams_v66();
        setIntegerParam(Supported, 1);
        setExpectedVersion(6, 6);
    } else if (m_version == "v67") {
        createParams_v67();
        setIntegerParam(Supported, 1);
        setExpectedVersion(6, 7);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported DSP version '%s'", version);
        return;
    }

    callParamCallbacks();
    initParams();
}

bool DspPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    if ((packet->length - sizeof(DasCmdPacket)) != sizeof(RspReadVersion)) {
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
