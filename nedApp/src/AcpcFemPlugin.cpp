/* AcpcFemPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcFemPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(AcpcFemPlugin, 4, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Config dir", string);

AcpcFemPlugin::AcpcFemPlugin(const char *portName, const char *parentPlugins, const char *version_, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, 2)
{
    std::string version(version_);
    if (version == "v14") {
        createParams_v14();
    } else if (version == "v22") {
        createParams_v22();
    } else {
        LOG_ERROR("Unsupported ACPC FEM version %s", version.c_str());
        return;
    }

    initParams();
}
