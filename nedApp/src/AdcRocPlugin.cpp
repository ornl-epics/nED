/* AdcRocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "AdcRocPlugin.h"
#include "Log.h"

#include <cstring>

#define HEX_BYTE_TO_DEC(a)      ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

EPICS_REGISTER_PLUGIN(AdcRocPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned AdcRocPlugin::NUM_ADCROCPLUGIN_DYNPARAMS       = 650;  //!< Since supporting multiple versions with different number of PVs, this is just a maximum value

/**
 * ADC ROC version response format
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

AdcRocPlugin::AdcRocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_ADCROC, true, blocking,
                       NUM_ADCROCPLUGIN_DYNPARAMS, defaultInterfaceMask, defaultInterruptMask)
    , m_version(version)
{
    if (0) {
    } else if (m_version == "v02") {
        setIntegerParam(Supported, 1);
        createStatusParams_v02();
        createConfigParams_v02();
        createCounterParams_v02();
    } else if (m_version == "v03") {
        setIntegerParam(Supported, 1);
        createStatusParams_v03();
        createConfigParams_v03();
        createCounterParams_v03();
    }  else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported ADC ROC version '%s'", version);
    }

    LOG_DEBUG("Number of configured dynamic parameters: %zu", m_statusParams.size() + m_configParams.size());

    callParamCallbacks();
}

bool AdcRocPlugin::processResponse(const DasPacket *packet)
{
    DasPacket::CommandType command = packet->getResponseType();

    switch (command) {
    default:
        return BaseModulePlugin::processResponse(packet);
    }
}

asynStatus AdcRocPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    /* GSG TODO:  Anything I need to handle here?
    // Only serving StreamDevice - puts reason as -1
    if (pasynUser->reason == -1) {
        // StreamDevice is sending entire string => no need to buffer the request.
        reqHvCmd(value, nChars);
        *nActual = nChars;
        return asynSuccess;
    }
   */
    return BaseModulePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

asynStatus AdcRocPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    /* GSG TODO: Anything I need to handle here?
    // Only serving StreamDevice - puts reason as -1
    if (pasynUser->reason == -1) {
        asynStatus status = asynSuccess;

        // Temporarily unlock the device or processResponse() will not run
        this->unlock();

        *nActual = m_hvBuffer.dequeue(value, nChars, pasynUser->timeout);

        if (*nActual == 0) {
            status = asynTimeout;
        } else if (eomReason) {
            if (value[*nActual - 1] == '\r') *eomReason |= ASYN_EOM_EOS;
            if (*nActual == nChars)          *eomReason |= ASYN_EOM_CNT;
        }

        this->lock();

        return status;
    }
    */
    return BaseModulePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

bool AdcRocPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if (version.hw_version == 0) {
        char ver[10];
        snprintf(ver, sizeof(ver), "v%u%u", version.fw_version, version.fw_revision);
        if (m_version == ver)
            return true;
    }

    return false;
}

bool AdcRocPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
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
    version.fw_year     = HEX_BYTE_TO_DEC(response->year >> 8) * 100 + HEX_BYTE_TO_DEC(response->year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->day);

    return true;
}

bool AdcRocPlugin::rspReadConfig(const DasPacket *packet)
{
    /* GSG TODO: Need to verify that we don't have to do this....this was
     * required only because v54 firmware was broken and appended four extra
     * bytes at the end of the payload.
    if (m_version == "v02") {
        uint8_t buffer[130]; // actual size of the READ_CONFIG packet

        if (packet->length() > sizeof(buffer)) {
            LOG_ERROR("Received v02 READ_CONFIG response bigger than expected");
            return asynError;
        }

        // Packet in shared queue must not be modified. So we make a copy.
        memcpy(buffer, packet, packet->length());
        packet = reinterpret_cast<const DasPacket *>(buffer);
        const_cast<DasPacket *>(packet)->payload_length -= 4; // This is the only reason we're doing all the buffering

        return BaseModulePlugin::rspReadConfig(packet);
    }
   */
    return BaseModulePlugin::rspReadConfig(packet);
}

// createStatusParams_v* and createConfigParams_v* functions are implemented in custom files for two
// reasons:
// * easy parsing through scripts in tools/ directory
// * easily compare PVs between ROC versions
