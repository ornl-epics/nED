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
    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor
        std::string m_posCalcPort;                          //!< Plugin to send parameters to.

    public: // functions

        /**
         * Constructor for BnlRocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] hardwareId Hardware ID of the BNLROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version BNLROC HW&SW version, ie. V5_50
         * @param[in] posCalcPortName Plugin to send parameters to.
         */
        BnlRocPlugin(const char *portName, const char *parentPlugins, const char *hardwareId, const char *version, const char *posCalcPortName);

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
        static bool parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see BnlRocPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

        /**
         * Overloaded method to send all parameters to BnlPosCalcPlugin
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

    private: // functions
        /**
         * Create and register all BNLROC v0.0 parameters to be exposed to EPICS.
         */
        void createParams_v00();

        /**
         * Create and register all BNLROC v2.0 parameters to be exposed to EPICS.
         */
        void createParams_v20();
};

#endif // BNL_ROC_PLUGIN_H
