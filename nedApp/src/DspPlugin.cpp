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
#include "TimeSync.h"

#include <epicsAlgorithm.h>
#include <osiSock.h>
#include <string.h>

#include <chrono>
#include <functional>
#include <string>

EPICS_REGISTER_PLUGIN(DspPlugin, 4, "Port name", string, "Parent plugins", string, "Version", string, "Config dir", string);

const double DspPlugin::DSP_RESPONSE_TIMEOUT            = 1.0;

/**
 * DSP 6.x version response format
 */
#ifdef BITFIELD_LSB_FIRST
struct RspReadVersion {
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
};

struct RspReadVersion7 {
    unsigned hw_revision:8;     // Board revision number
    unsigned hw_version:8;      // Board version number
    unsigned fw_revision:8;     // Firmware revision number
    unsigned fw_version:8;      // Firmware version number
    unsigned year:16;           // Year
    unsigned day:8;             // Day
    unsigned month:8;           // Month
};
#else
#error Missing DspVersionRegister declaration
#endif

DspPlugin::DspPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, DasCmdPacket::MOD_TYPE_DSP, 4)
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
    } else if (m_version == "v71") {
        createParams_v71();
        setIntegerParam(Supported, 1);
        setExpectedVersion(7, 1);
        setCmdVersion(1);
        m_timeSync.reset(new TimeSync(this));
        m_features |= (uint32_t)ModuleFeatures::TIME_SYNC;
    } else if (m_version == "v72") {
        createParams_v72();
        setIntegerParam(Supported, 1);
        setExpectedVersion(7, 2);
        setCmdVersion(1);
        m_timeSync.reset(new TimeSync(this));
        m_features |= (uint32_t)ModuleFeatures::TIME_SYNC;
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported DSP version '%s'", version);
        return;
    }

    initParams();
}

bool DspPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getCmdPayloadLength() == sizeof(RspReadVersion)) {
        const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getCmdPayload());
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
    } else if (packet->getCmdPayloadLength() == sizeof(RspReadVersion7)) {
        const RspReadVersion7 *response = reinterpret_cast<const RspReadVersion7*>(packet->getCmdPayload());
        version.hw_version  = response->hw_version;
        version.hw_revision = response->hw_revision;
        version.hw_year     = 0;
        version.hw_month    = 0;
        version.hw_day      = 0;
        version.fw_version  = response->fw_version;
        version.fw_revision = response->fw_revision;
        version.fw_year     = HEX_SHORT_TO_DEC(response->year);
        version.fw_month    = HEX_BYTE_TO_DEC(response->month);
        version.fw_day      = HEX_BYTE_TO_DEC(response->day);

        return true;
    }
    return false;
}

bool DspPlugin::rspTimeSync(const DasCmdPacket *packet)
{
    if (m_timeSync)
        m_timeSync->rspTimeSync(packet);
    return true;
}

asynStatus DspPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (m_timeSync && m_timeSync->setParam(pasynUser->reason, value))
        return asynSuccess;

    return BaseModulePlugin::writeInt32(pasynUser, value);
}

bool DspPlugin::processResponse(const DasCmdPacket *packet)
{
    if (packet->getCommand() == DasCmdPacket::CMD_TIME_SYNC && m_timeSync) {
        (void)m_timeSync->rspTimeSync(packet);
        return true;
    }
    return BaseModulePlugin::processResponse(packet);
}
