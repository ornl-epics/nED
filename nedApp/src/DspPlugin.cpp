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

EPICS_REGISTER_PLUGIN(DspPlugin, 5, "Port name", string, "Parent plugins", string, "Version", string, "PVA params", string, "Config dir", string);

DspPlugin::DspPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *pvaParamsName, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, pvaParamsName, configDir, 4)
{
    std::string version(version_);
    if (version == "v51") {
        createParams_v51();
    } else if (version == "v52") {
        createParams_v52();
    } else if (version == "v63") {
        createParams_v63();
    } else if (version == "v64") {
        createParams_v64();
    } else if (version == "v65") {
        createParams_v65();
    } else if (version == "v66") {
        createParams_v66();
    } else if (version == "v67") {
        createParams_v67();
    } else if (version == "v71") {
        createParams_v71();
        setCmdVersion(1);
    } else if (version == "v72") {
        createParams_v72();
        setCmdVersion(1);
    } else if (version == "v710") {
        createParams_v710();
        setCmdVersion(1);
    } else if (version == "v80") {
        createParams_v80();
        setCmdVersion(1);
        m_timeSync.reset(new TimeSync(this));
        m_features |= (uint32_t)ModuleFeatures::TIME_SYNC;
    } else if (version == "v81") {
        createParams_v81();
        setCmdVersion(1);
        m_timeSync.reset(new TimeSync(this));
        m_features |= (uint32_t)ModuleFeatures::TIME_SYNC;
    } else {
        LOG_ERROR("Unsupported DSP version '%s'", version.c_str());
        return;
    }

    initParams();
}

asynStatus DspPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (m_timeSync && m_timeSync->setParam(pasynUser->reason, value))
        return asynSuccess;

    return BaseModulePlugin::writeInt32(pasynUser, value);
}

bool DspPlugin::processResponse(const DasCmdPacket *packet)
{
    if (packet->getCommand() == DasCmdPacket::CMD_TIME_SYNC && m_timeSync)
        return m_timeSync->rspTimeSync(packet);
    return BaseModulePlugin::processResponse(packet);
}
