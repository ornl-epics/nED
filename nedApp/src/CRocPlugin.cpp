/* CRocPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "CRocPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(CRocPlugin, 5, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Config dir", string, "PosCalc port name", string);

CRocPlugin::CRocPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir, const char *posCalcPortName)
    : BaseModulePlugin(portName, parentPlugins, configDir, 2)
    , m_posCalcPort(posCalcPortName)
{
    std::string version(version_);

    if (0) {
    } else if (version == "v93") {
        createParams_v93();
    } else if (version == "v94") {
        createParams_v94();
    } else {
        LOG_ERROR("Unsupported CROC version '%s'", version.c_str());
        return;
    }

    initParams();
}

asynStatus CRocPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (!m_posCalcPort.empty())
        BasePlugin::sendParam(m_posCalcPort, getParamName(pasynUser->reason), value);
    return BaseModulePlugin::writeInt32(pasynUser, value);
}
