/* FemPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef FEM_PLUGIN_H
#define FEM_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for FEM module.
 */
class FemPlugin : public BaseModulePlugin {
    private: // structures and definitions
        static const unsigned NUM_FEMPLUGIN_DYNPARAMS;  //!< Maximum number of asyn parameters, including the status and configuration parameters

    private: // variables
        std::string m_version;              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for FemPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the ROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version FEM HW&SW version, ie. V10_50
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        FemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

        /**
         * Try to parse the FEM version response packet an populate the structure.
         *
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see FemPlugin::parseVersionRsp()
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

    private: // functions
        /**
         * Create and register all FEM10 parameters to be exposed to EPICS.
         */
        void createParams_v32();

        /**
         * Create and register all FEM9 v35 parameters to be exposed to EPICS.
         */
        void createParams_v35();

        /**
         * Create and register all FEM9 v36 parameters to be exposed to EPICS.
         */
        void createParams_v36();

        /**
         * Create and register all FEM9 v37 parameters to be exposed to EPICS.
         */
        void createParams_v37();

        /**
         * Create and register all FEM9 v38 parameters to be exposed to EPICS.
         */
        void createParams_v38();
};

#endif // DSP_PLUGIN_H
