/* BnlRocPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#ifndef BNL_ROC_PLUGIN_H
#define BNL_ROC_PLUGIN_H

#include "BaseModulePlugin.h"
#include "Fifo.h"

#include <list>

/**
 * Plugin for BNL ROC module.
 *
 * The BnlRocPlugin extends BaseModulePlugin with the ROC board specifics.
 * It handles DISCOVER response to verify module type and READ_VERSION
 * which parses the version information.
 * Several firmware version are supported:
 *
 * * BNL ROC hw=0.1 fw=0.3 as v03
 *
 * In general firmware versions differ in status or configuration
 * parameters they provide. Those are available in individual files
 * called BnlRocPlugin_<version>.cpp. BnlRocPlugin will refuse to communicate
 * with unsupported versions.
 *
 */
class BnlRocPlugin : public BaseModulePlugin {
    public: // variables
        static const int defaultInterfaceMask = BaseModulePlugin::defaultInterfaceMask | asynOctetMask;
        static const int defaultInterruptMask = BaseModulePlugin::defaultInterruptMask | asynOctetMask;

    private: // structures and definitions
        static const unsigned NUM_BNLROCPLUGIN_DYNPARAMS;      //!< Maximum number of asyn parameters, including the status and configuration parameters

    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for BnlRocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the BNLROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version BNLROC HW&SW version, ie. V5_50
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        BnlRocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

        /**
         * Try to parse the BNLROC version response packet an populate the structure.
         *
         * Function will parse all known BNLROC version responses and populate the
         * version structure. If the function returns false, it does not recognize
         * the response.
         *
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @param[in] expectedLen expected size of the version response, used to
         *                        verify the parsed packet matches this one
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see BnlRocPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

    private: // functions
        /**
         * Create and register all status BNLROC v0.0 parameters to be exposed to EPICS.
         */
        void createStatusParams_v00();

        /**
         * Create and register all config BNLROC v0.0 parameters to be exposed to EPICS.
         */
        void createConfigParams_v00();

        /**
         * Create and register all status counter BNLROC v0.0 parameters to be exposed to EPICS.
         */
        //void createCounterParams_v00();
        /**
         * Create and register all status BNLROC v2.0 parameters to be exposed to EPICS.
         */
        void createStatusParams_v20();

        /**
         * Create and register all config BNLROC v2.0 parameters to be exposed to EPICS.
         */
        void createConfigParams_v20();

        /**
         * Create and register all status counter BNLROC v2.0 parameters to be exposed to EPICS.
         */
        //void createCounterParams_v20();

    protected:
};

#endif // BNL_ROC_PLUGIN_H
