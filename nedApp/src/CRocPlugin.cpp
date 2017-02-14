/* CRocPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "CRocPlugin.h"
#include "Log.h"

#include <cstring>

EPICS_REGISTER_PLUGIN(CRocPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "PosCalc port name", string);

const unsigned CRocPlugin::NUM_CROCPLUGIN_DYNPARAMS       = 650;  //!< Since supporting multiple versions with different number of PVs, this is just a maximum value

CRocPlugin::CRocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, const char *posCalcPortName)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_CROC, true, 0,
                       NUM_CROCPLUGIN_DYNPARAMS, defaultInterfaceMask, defaultInterruptMask)
    , m_version(version)
    , m_posCalcPort(posCalcPortName)
{
    if (0) {
    } else if (m_version == "v93") {
        setIntegerParam(Supported, 1);
        createParams_v93();
        setExpectedVersion(9, 3);
    } else if (m_version == "v94") {
        setIntegerParam(Supported, 1);
        createParams_v94();
        setExpectedVersion(9, 4);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported CROC version '%s'", version);
    }

    callParamCallbacks();
    initParams();
}

bool CRocPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
/**
 * ROC V5 version response format
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
    version.fw_year     = HEX_BYTE_TO_DEC(response->year >> 8) * 100 + HEX_BYTE_TO_DEC(response->year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->day);

    return true;
}

asynStatus CRocPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (!m_posCalcPort.empty())
        BasePlugin::sendParam(m_posCalcPort, getParamName(pasynUser->reason), value);
    return BaseModulePlugin::writeInt32(pasynUser, value);
}

// createParams_v* function is implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
