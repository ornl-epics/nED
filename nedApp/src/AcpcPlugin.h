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
    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for AcpcPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] hardwareId Hardware ID of the ROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version ROC HW&SW version, ie. V5_50
         */
        AcpcPlugin(const char *portName, const char *parentPlugins, const char *hardwareId, const char *version);

        /**
         * Try to parse the ACPC version response packet an populate the structure.
         *
         * Function will parse all known ACPC version responses and populate the
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
         * @see AcpcPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

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
         * Create and register all ACPC 18.0 parameters to be exposed to EPICS.
         */
        void createParams_v180();
};

#endif // ACPC_PLUGIN_H
