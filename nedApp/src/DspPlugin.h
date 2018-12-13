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

#include <memory>

class TimeSync;

/**
 * Plugin for DSP module.
 */
class DspPlugin : public BaseModulePlugin {
    private: // structures and definitions
        static const double DSP_RESPONSE_TIMEOUT;           //!< Default DSP response timeout, in seconds
        std::string m_version;
        std::unique_ptr<TimeSync> m_timeSync;

    public:

        /**
         * Constructor for DspPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version Configured module version, must match the actual version
         */
        DspPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir);

        /**
         * Try to parse the DSP version response packet an populate the structure.
         *
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see DspPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

        /**
         * Process local parameters.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

        /**
         * Process TimeSync responses, pass-thru the rest.
         */
        bool processResponse(const DasCmdPacket *packet) override;

    private:

        /**
         * Create and register all 5.1 parameters to be exposed to EPICS.
         *
         * DSP 5.1 is not DSP-T and can not do timing.
         */
        void createParams_v51();

        /**
         * Create and register all 5.2 parameters to be exposed to EPICS.
         *
         * DSP 5.2 is not DSP-T and can not do timing.
         */
        void createParams_v52();

        /**
         * Create and register all 6.3 parameters to be exposed to EPICS.
         */
        void createParams_v63();

        /**
         * Create and register all 6.4 parameters to be exposed to EPICS.
         */
        void createParams_v64();

        /**
         * Create and register all 6.5 parameters to be exposed to EPICS.
         */
        void createParams_v65();

        /**
         * Create and register all 6.6 parameters to be exposed to EPICS.
         */
        void createParams_v66();

        /**
         * Create and register all 6.7 parameters to be exposed to EPICS.
         */
        void createParams_v67();

        /**
         * Create and register all 7.1 parameters to be exposed to EPICS.
         */
        void createParams_v71();

        /**
         * Create and register all 7.2 parameters to be exposed to EPICS.
         */
        void createParams_v72();
};

#endif // DSP_PLUGIN_H
