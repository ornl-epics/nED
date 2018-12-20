/* BnlRocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "BnlRocPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(BnlRocPlugin, 5, "Port name", string,
        "Parent plugins", string, "Hw & SW version", string,
        "Config dir", string, "BnlPosCalc plugin", string);

BnlRocPlugin::BnlRocPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir, const char *posCalcPortName)
    : BaseModulePlugin(portName, parentPlugins, configDir, 2)
    , m_posCalcPort(posCalcPortName)
{
    std::string version(version_);
    if (0) {
    } else if (version == "v00") {
        createParams_v00();
    } else if (version == "v20") {
        createParams_v20();
    } else if (version == "v21") {
        createParams_v20();
    }  else {
        LOG_ERROR("Unsupported BNL ROC version '%s'", version.c_str());
        return;
    }

    initParams();
}

asynStatus BnlRocPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (!m_posCalcPort.empty())
        BasePlugin::sendParam(m_posCalcPort, getParamName(pasynUser->reason), value);
    return BaseModulePlugin::writeInt32(pasynUser, value);
}
