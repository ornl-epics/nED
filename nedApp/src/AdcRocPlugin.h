/* AdcRocPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#ifndef ADC_ROC_PLUGIN_H
#define ADC_ROC_PLUGIN_H

#include "BaseModulePlugin.h"
#include "Fifo.h"

#include <list>

/**
 * Plugin for ADC ROC module.
 *
 * The AdcRocPlugin extends BaseModulePlugin with the ROC board specifics.
 * It handles DISCOVER response to verify module type and READ_VERSION
 * which parses the version information.
 * Several firmware version are supported:
 *
 * * ADC ROC hw=?.? fw=?.? as v02 (IN PROGRESS)
 *
 * In general firmware versions differ in status or configuration
 * parameters they provide. Those are available in individual files
 * called AdcRocPlugin_<version>.cpp. AdcRocPlugin will refuse to communicate
 * with unsupported versions.
 *
 */
class AdcRocPlugin : public BaseModulePlugin {
    public: // variables
        static const int defaultInterfaceMask = BaseModulePlugin::defaultInterfaceMask | asynOctetMask;
        static const int defaultInterruptMask = BaseModulePlugin::defaultInterruptMask | asynOctetMask;

    private: // structures and definitions
        static const unsigned NUM_ADCROCPLUGIN_DYNPARAMS;      //!< Maximum number of asyn parameters, including the status and configuration parameters
        static const unsigned NUM_CHANNELS = 8;             //!< Number of channels connected to ADCROC
        static const float    NO_RESPONSE_TIMEOUT;          //!< Timeout to wait for response from ADCROC, in seconds

    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor

    public: // functions

        /**
         * Constructor for AdcRocPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the ADCROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version ADCROC HW&SW version, ie. V5_50
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        AdcRocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

        /**
         * Process RS232 packets only, let base implementation do the rest.
         */
        bool processResponse(const DasPacket *packet);

        /**
         * Send string/byte data to PVs
         */
        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

        /**
         * Receive string/byte data to PVs
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Try to parse the ADCROC version response packet an populate the structure.
         *
         * Function will parse all known ADCROC version responses and populate the
         * version structure. If the function returns false, it does not recognize
         * the response.
         *
         * When expectedLen parameter is non-zero, the function will only accept
         * the response that matches the size. This is useful when the version
         * is known in advance and this function can be used to verify that returned
         * version matches configured one. If the parsed version length doesn't match
         * the expected length, function returns false.
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
         * Verify the DISCOVER response is from ADCROC.
         *
         * @param[in] packet with response to DISCOVER
         * @return true if packet was parsed and type of module is ADCROC.
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
         * Handle READ_CONFIG response.
         *
         * For normal firmwares the function simply invokes BaseModulePlugin::rspReadConfig()
         * passing it the original packet.
         */
        bool rspReadConfig(const DasPacket *packet);

        /**
         * Create and register all status ADCROC v0.2 parameters to be exposed to EPICS.
         */
        void createStatusParams_v02();

        /**
         * Create and register all config ADCROC v0.2 parameters to be exposed to EPICS.
         */
        void createConfigParams_v02();

        /**
         * Create and register all status counter ADCROC v0.2 parameters to be exposed to EPICS.
         */
        void createCounterParams_v02();

        /**
         * Create and register all status ADCROC v0.3 parameters to be exposed to EPICS.
         */
        void createStatusParams_v03();

        /**
         * Create and register all config ADCROC v0.3 parameters to be exposed to EPICS.
         */
        void createConfigParams_v03();

        /**
         * Create and register all status counter ADCROC v0.3 parameters to be exposed to EPICS.
         */
        void createCounterParams_v03();

    protected:
        /*
        #define FIRST_ADCROCPLUGIN_PARAM Acquiring
        int Acquiring;
        #define LAST_ADCROCPLUGIN_PARAM Acquiring
        */
};

#endif // ADC_ROC_PLUGIN_H
