/* AcpcFemPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef ACPC_FEM_PLUGIN_H
#define ACPC_FEM_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for ACPC FEM module.
 */
class AcpcFemPlugin : public BaseModulePlugin {
    public: // functions

        /**
         * Constructor for AcpcFemPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version ACPC FEM HW&SW version, ie. V10_50
         */
        AcpcFemPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir);

    private: // functions
        /**
         * Create and register all ACPC FEM parameters to be exposed to EPICS.
         */
        void createParams_v14();

        /**
         * Create and register all ACPC FEM parameters to be exposed to EPICS.
         */
        void createParams_v22();
};

#endif // ACPC_FEM_PLUGIN_H
