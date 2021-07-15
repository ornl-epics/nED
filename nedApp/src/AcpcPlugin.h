/* AcpcPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef ACPC_PLUGIN_H
#define ACPC_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for ACPC module.
 */
class AcpcPlugin : public BaseModulePlugin {
    public: // functions

        /**
         * Constructor for AcpcPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] pvaParamsName Name of PVA pv to export parameters lists
         * @param[in] version ROC HW&SW version, ie. V5_50
         */
        AcpcPlugin(const char *portName, const char *parentPlugins, const char *version, const char *pvaParamsName=nullptr, const char *configDir=nullptr);

    private: // functions
        /**
         * Create and register all ACPC 14.4 parameters to be exposed to EPICS.
         */
        void createParams_v144();

        /**
         * Create and register all ACPC 17.1 parameters to be exposed to EPICS.
         */
        void createParams_v171();

        /**
         * Create and register all ACPC 17.2 parameters to be exposed to EPICS.
         */
        void createParams_v172();

        /**
         * Create and register all ACPC 17.3 parameters to be exposed to EPICS.
         */
        void createParams_v173();

        /**
         * Create and register all ACPC 17.4 parameters to be exposed to EPICS.
         */
        void createParams_v174();

        /**
         * Create and register all ACPC 18.0 parameters to be exposed to EPICS.
         */
        void createParams_v180();
};

#endif // ACPC_PLUGIN_H
