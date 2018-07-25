/* ArocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "ArocPlugin.h"
#include "Log.h"
#include "Common.h"

EPICS_REGISTER_PLUGIN(ArocPlugin, 3, "Port name", string, "Dispatcher port name", string, "Hw & SW version", string);

/**
 * AROC version response format
 */
struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     //!< Board revision number
    unsigned hw_version:8;      //!< Board version number
    unsigned fw_revision:8;     //!< Firmware revision number
    unsigned fw_version:8;      //!< Firmware version number
    unsigned year:16;           //!< Year
    unsigned day:8;             //!< Day
    unsigned month:8;           //!< Month
#else
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};

ArocPlugin::ArocPlugin(const char *portName, const char *parentPlugins, const char *version)
    : BaseModulePlugin(portName, parentPlugins, DasCmdPacket::MOD_TYPE_AROC, 2)
    , m_version(version)
{
    if (m_version == "v23") {
        setIntegerParam(Supported, 1);
        createParams_v23();
        setExpectedVersion(2, 3);
    } else if (m_version == "v24") {
        setIntegerParam(Supported, 1);
        createParams_v24();
        setExpectedVersion(2, 4);
    } else if (m_version == "v255255") {
        setIntegerParam(Supported, 1);
        createParams_v255255();
        setExpectedVersion(255, 255);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported AROC version '%s'", version);
    }

    callParamCallbacks();
    initParams();
}

bool ArocPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getCmdPayloadLength() != sizeof(RspReadVersion))
        return false;

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion *>(packet->getCmdPayload());

    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = 0;
    version.hw_month    = 0;
    version.hw_day      = 0;
    version.fw_version  = response->fw_version;
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->year >> 8) * 100 + HEX_BYTE_TO_DEC(response->year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->day);

    return true;
}
