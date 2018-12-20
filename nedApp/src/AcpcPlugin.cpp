/* AcpcPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(AcpcPlugin, 4, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Config dir", string);

AcpcPlugin::AcpcPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, 2)
{
    std::string version(version_);
    if (0) {
    } else if (version == "v144") {
        createParams_v144();
    } else if (version == "v171") {
        createParams_v171();
    } else if (version == "v172") {
        createParams_v172();
    } else if (version == "v173") {
        createParams_v173();
    } else if (version == "v174") {
        createParams_v174();
    } else if (version == "v180") {
        createParams_v180();
    } else {
        LOG_ERROR("Unsupported ACPC version '%s'", version.c_str());
        return;
    }

    initParams();
}
