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
    public: // variables
        static const int defaultInterfaceMask = BaseModulePlugin::defaultInterfaceMask;
        static const int defaultInterruptMask = BaseModulePlugin::defaultInterruptMask;

    private: // structures and definitions
        static const unsigned NUM_ACPCPLUGIN_DYNPARAMS;     //!< Maximum number of asyn parameters, including the status and configuration parameters

    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for AcpcPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the ROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version ROC HW&SW version, ie. V5_50
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        AcpcPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

        /**
         * Try to parse the ROC version response packet an populate the structure.
         *
         * Function will parse all known ROC version responses and populate the
         * version structure. If the function returns false, it does not recognize
         * the response.
         *
         * All ROC boards except for v5.4 have the same response. v5.4 adds an extra
         * vendor field which the function disregards.
         *
         * When expectedLen parameter is non-zero, the function will only accept
         * the response that matches the size. This is useful when the version
         * is known in advance and this function can be used to verify that returned
         * version matches configured one. If the parsed version length doesn't match
         * the expected length, funtion returns false.
         *
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @param[in] expectedLen expected size of the version response, used to
         *                        verify the parsed packet matches this one
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version, size_t expectedLen=0);

    private: // functions
        /**
         * Verify the DISCOVER response is from ROC.
         *
         * @param[in] packet with response to DISCOVER
         * @return true if packet was parsed and type of module is ROC.
         */
        bool rspDiscover(const DasPacket *packet);

        /**
         * Overrided READ_VERSION handler dispatches real work to one of rspReadVersion_*
         *
         * @param[in] packet with response to READ_VERSION
         * @return true if packet was parsed and module version verified.
         */
        bool rspReadVersion(const DasPacket *packet);

        /**
         * Create and register all status ROC v4.4/v4.5 parameters to be exposed to EPICS.
         */
        void createStatusParams_v41();

        /**
         * Create and register all config ROC v4.4/v4.5 parameters to be exposed to EPICS.
         */
        void createConfigParams_v41();

    protected:
        #define FIRST_ACPCPLUGIN_PARAM Acquiring
        int none;
        #define LAST_ACPCPLUGIN_PARAM Acquiring
};

#endif // ACPC_PLUGIN_H
