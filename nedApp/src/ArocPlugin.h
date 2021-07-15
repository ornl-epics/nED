/* ArocPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef AROC_PLUGIN_H
#define AROC_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for AROC module.
 *
 * The ArocPlugin extends BaseModulePlugin with the AROC board specifics.
 * It mainly defines version specific register mappings.
 */
class ArocPlugin : public BaseModulePlugin {
    public: // functions

        /**
         * Constructor for ArocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] pvaParamsName Name of PVA pv to export parameters lists
         * @param[in] version AROC HW&SW version, ie. V5_50
         */
        ArocPlugin(const char *portName, const char *parentPlugins, const char *version, const char *pvaParamsName=nullptr, const char *configDir=nullptr);

    private: // functions

        /**
         * Create and register all AROC v2.3 parameters to be exposed to EPICS.
         */
        void createParams_v22();

        /**
         * Create and register all AROC v2.3 parameters to be exposed to EPICS.
         */
        void createParams_v23();

        /**
         * Create and register all AROC v2.4 parameters to be exposed to EPICS.
         */
        void createParams_v24();

        /**
         * Create and register all AROC v2.5 parameters to be exposed to EPICS.
         */
        void createParams_v25();

        /**
         * Create and register all AROC v255.255 parameters to be exposed to EPICS.
         */
        void createParams_v255255();
};

#endif // AROC_PLUGIN_H
