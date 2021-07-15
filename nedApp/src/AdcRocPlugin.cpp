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

EPICS_REGISTER_PLUGIN(AdcRocPlugin, 5, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Params PV", string, "Config dir", string);

AdcRocPlugin::AdcRocPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *pvaParamsName, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, pvaParamsName, configDir, 2)
{
    std::string version(version_);
    if (0) {
    } else if (version == "v02") {
        createParams_v02();
    } else if (version == "v03") {
        createParams_v03();
    } else if (version == "v05") {
        createParams_v05();
    } else if (version == "v06") {
        createParams_v06();
    } else if (version == "v07") {
        createParams_v07();
    }  else {
        LOG_ERROR("Unsupported ADC ROC version '%s'", version.c_str());
        return;
    }

    m_cmdHandlers[DasCmdPacket::CMD_PM_PULSE_RQST_ON].first   = std::bind(&BaseModulePlugin::reqSimple,  this, DasCmdPacket::CMD_PM_PULSE_RQST_ON);
    m_cmdHandlers[DasCmdPacket::CMD_PM_PULSE_RQST_ON].second  = std::bind(&BaseModulePlugin::rspSimple,  this, std::placeholders::_1);
    m_cmdHandlers[DasCmdPacket::CMD_PM_PULSE_RQST_OFF].first  = std::bind(&BaseModulePlugin::reqSimple,  this, DasCmdPacket::CMD_PM_PULSE_RQST_OFF);
    m_cmdHandlers[DasCmdPacket::CMD_PM_PULSE_RQST_OFF].second  = std::bind(&BaseModulePlugin::rspSimple, this, std::placeholders::_1);

    initParams();
}
