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
    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for ArocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version AROC HW&SW version, ie. V5_50
         */
        ArocPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir);

        /**
         * Try to parse the AROC version response packet an populate the structure.
         *
         * Function will parse all known AROC version responses and populate the
         * version structure. If the function returns false, it does not recognize
         * the response.
         *
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see ArocPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

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
