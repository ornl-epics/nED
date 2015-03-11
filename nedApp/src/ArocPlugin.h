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
    public: // variables
        static const int defaultInterfaceMask = BaseModulePlugin::defaultInterfaceMask;
        static const int defaultInterruptMask = BaseModulePlugin::defaultInterruptMask;

    private: // structures and definitions
        static const unsigned NUM_ROCPLUGIN_DYNPARAMS;      //!< Maximum number of asyn parameters, including the status and configuration parameters

    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for ArocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the ROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version AROC HW&SW version, ie. V5_50
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        ArocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

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
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see ArocPlugin::parseVersionRsp()
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
         * Create and register all status ROC v4.4/v4.5 parameters to be exposed to EPICS.
         */
        void createStatusParams_v23();

        /**
         * Create and register all config AROC v2.3 parameters to be exposed to EPICS.
         */
        void createConfigParams_v23();
};

#endif // AROC_PLUGIN_H
