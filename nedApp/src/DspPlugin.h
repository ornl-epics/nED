/* DspPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DSP_PLUGIN_H
#define DSP_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for DSP module.
 */
class DspPlugin : public BaseModulePlugin {
    private: // structures and definitions
        static const unsigned NUM_DSPPLUGIN_CONFIGPARAMS;   //!< This is used as a runtime assert check and must match number of configuration parameters
        static const unsigned NUM_DSPPLUGIN_STATUSPARAMS;   //!< This is used as a runtime assert check and must match number of status parameters
        static const double DSP_RESPONSE_TIMEOUT;           //!< Default DSP response timeout, in seconds
        std::string m_version;

    public:

        /**
         * Constructor for DspPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the DSP module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version Configured module version, must match the actual version
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        DspPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking);

        /**
         * Try to parse the DSP version response packet an populate the structure.
         *
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see DspPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

        /**
         * Configured version must match actual.
         *
         * @return true when they match, false otherwise.
         */
        bool checkVersion(const BaseModulePlugin::Version &version);

    private:

        /**
         * Create and register all configuration parameters to be exposed to EPICS.
         */
        void createConfigParams_v63();

        /**
         * Create and register all status parameters to be exposed to EPICS.
         */
        void createStatusParams_v63();

        /**
         * Create and register all configuration parameters to be exposed to EPICS.
         */
        void createConfigParams_v64();

        /**
         * Create and register all status parameters to be exposed to EPICS.
         */
        void createStatusParams_v64();

        /**
         * Create and register all counter parameters to be exposed to EPICS.
         */
        void createCounterParams_v64();
};

#endif // DSP_PLUGIN_H
