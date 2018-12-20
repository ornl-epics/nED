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

EPICS_REGISTER_PLUGIN(ArocPlugin, 4, "Port name", string, "Dispatcher port name", string, "Hw & SW version", string, "Config dir", string);

ArocPlugin::ArocPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, 2)
{
    std::string version(version_);
    if (version == "v22") {
        createParams_v22();
    } else if (version == "v23") {
        createParams_v23();
    } else if (version == "v24") {
        createParams_v24();
    } else if (version == "v25") {
        createParams_v25();
    } else if (version == "v255255") {
        createParams_v255255();
    } else {
        LOG_ERROR("Unsupported AROC version '%s'", version.c_str());
        return;
    }

    initParams();
}
