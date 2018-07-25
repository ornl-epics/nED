/* CRocPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef CROC_PLUGIN_H
#define CROC_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for CROC module.
 *
 */
class CRocPlugin : public BaseModulePlugin {
    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor
        std::string m_posCalcPort;                          //!< Name of the CRocPosCalcPlugin port for sending this CROC parameters

    public: // functions

        /**
         * Constructor for CRocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version ROC HW&SW version, ie. V5_50
         * @param[in] posCalcPortName Name of the CROC position calculation plugin
         */
        CRocPlugin(const char *portName, const char *parentPlugins, const char *version, const char *posCalcPortName=0);

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
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see RocPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

        /**
         * Handle passing parameters from other plugins.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    private: // functions

        /**
         * Create and register all CROC v9.3 parameters to be exposed to EPICS.
         */
        void createParams_v93();

        /**
         * Create and register all CROC v9.4 parameters to be exposed to EPICS.
         */
        void createParams_v94();
};

#endif // CROC_PLUGIN_H
