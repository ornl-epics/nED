/* DspWPlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DSPW_PLUGIN_H
#define DSPW_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for DSP-W module.
 */
class DspWPlugin : public BaseModulePlugin {
    public:

        /**
         * Constructor for DspWPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] pvaParamsName Name of PVA pv to export parameters lists
         * @param[in] version Configured module version, must match the actual version
         */
        DspWPlugin(const char *portName, const char *parentPlugins, const char *version, const char *pvaParamsName=nullptr, const char *configDir=nullptr);

    private:

        /**
         * Create and register all 1.0 parameters to be exposed to EPICS.
         */
        void createParams_v10();

        /**
         * Create and register all 2.0 parameters to be exposed to EPICS.
         */
        void createParams_v20();
};

#endif // DSPW_PLUGIN_H
