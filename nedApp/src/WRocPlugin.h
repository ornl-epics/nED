/* WRocPlugin.h
 *
 * Copyright (c) 2019 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef W_ROC_PLUGIN_H
#define W_ROC_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for Wand ROC module.
 *
 */
class WRocPlugin : public BaseModulePlugin {
    public: // functions

        /**
         * Constructor for WRocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version WROC HW&SW version, ie. V5_50
         * @param[in] pvaParamsName Name of PVA pv to export parameters lists
         */
        WRocPlugin(const char *portName, const char *parentPlugins, const char *version, const char *pvaParamsName=nullptr, const char *configDir=nullptr);

    private: // functions
        /**
         * Create and register all WROC v0.1 parameters to be exposed to EPICS.
         */
        void createParams_v01();
};

#endif // W_ROC_PLUGIN_H
