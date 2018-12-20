/* DspWPlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspWPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(DspWPlugin, 4, "Port name", string, "Parent plugins", string, "Version", string, "Config dir", string);

DspWPlugin::DspWPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, 4)
{
    std::string version(version_);
    if (version == "v10") {
        createParams_v10();
    } else if (version == "v20") {
        createParams_v20();
    } else {
        LOG_ERROR("Unsupported DSP-W version '%s'", version.c_str());
        return;
    }

    callParamCallbacks();
    initParams();
}
