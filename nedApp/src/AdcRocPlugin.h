/* AdcRocPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#ifndef ADC_ROC_PLUGIN_H
#define ADC_ROC_PLUGIN_H

#include "BaseModulePlugin.h"
#include "Fifo.h"

/**
 * Plugin for ADC ROC module.
 *
 * The AdcRocPlugin extends BaseModulePlugin with the ROC board specifics.
 * It handles DISCOVER response to verify module type and READ_VERSION
 * which parses the version information.
 * Several firmware version are supported:
 *
 * * ADC ROC hw=0.1 fw=0.3 as v03
 *
 * In general firmware versions differ in status or configuration
 * parameters they provide. Those are available in individual files
 * called AdcRocPlugin_<version>.cpp. AdcRocPlugin will refuse to communicate
 * with unsupported versions.
 *
 */
class AdcRocPlugin : public BaseModulePlugin {
    public: // functions

        /**
         * Constructor for AdcRocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version ADCROC HW&SW version, ie. V5_50
         */
        AdcRocPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir);

    private: // functions
        /**
         * Create and register all ADCROC v0.2 parameters to be exposed to EPICS.
         */
        void createParams_v02();

        /**
         * Create and register all ADCROC v0.3 parameters to be exposed to EPICS.
         */
        void createParams_v03();

        /**
         * Create and register all ADCROC v0.5 parameters to be exposed to EPICS.
         */
        void createParams_v05();

        /**
         * Create and register all ADCROC v0.6 parameters to be exposed to EPICS.
         *
         * ADC ROC 0.6 is the first firmware supporting new timing protocol between
         * DSP 7 and ROC.
         */
        void createParams_v06();

        /**
         * Create and register all ADCROC v0.7 parameters to be exposed to EPICS.
         *
         * ADC ROC 0.7 is the first one with 4 time domains that can be assigned
         * to input channels.
         */
        void createParams_v07();
};

#endif // ADC_ROC_PLUGIN_H
