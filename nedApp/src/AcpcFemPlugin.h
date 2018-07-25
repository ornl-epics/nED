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
    private: // structures and definitions
        static const unsigned NUM_ACPCFEMPLUGIN_DYNPARAMS;  //!< Maximum number of asyn parameters, including the status and configuration parameters

    private: // variables
        std::string m_version;              //!< Version string as passed to constructor

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
        AcpcFemPlugin(const char *portName, const char *parentPlugins, const char *version);

        /**
         * Try to parse the ACPC FEM version response packet an populate the structure.
         *
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see AcpcFemPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

        /**
         * Configured version must match actual.
         *
         * @return true when they match, false otherwise.
         */
        bool checkVersion(const BaseModulePlugin::Version &version);

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
