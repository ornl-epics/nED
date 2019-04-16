/* WRocPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "WRocPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(WRocPlugin, 4, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Config dir", string);

WRocPlugin::WRocPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, 4)
{
    std::string version(version_);
    if (version == "v01") {
        createParams_v01();
    }  else {
        LOG_ERROR("Unsupported WROC version '%s'", version.c_str());
        return;
    }

    initParams();
}
