/* RocPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef ROC_PLUGIN_H
#define ROC_PLUGIN_H

#include "BaseModulePlugin.h"
#include "Fifo.h"

#include <list>

/**
 * Plugin for ROC module.
 *
 * The RocPlugin extends BaseModulePlugin with the ROC board specifics.
 * It handles DISCOVER response to verify module type and READ_VERSION
 * which parses the version information. It also provides support for
 * High Voltage communication through RS232 port on ROC boards and
 * exposes that through StreamDevice friendly interfaces.
 *
 * Several firmware version are supported:
 *
 * * ROC hw=2.0 fw=4.5 as v45 (IN PROGRESS)
 * * ROC hw=5.2 fw=5.1 as v51
 * * ROC hw=5.2 fw=5.2 as v52
 * * ROC hw=5.2 fw=5.4 as v54
 * * ROC hw=5.2 fw=5.5 as v55 (via v54 plugin)
 * * ROC hw=5.2 fw=5.6 as v56
 * * ROC hw=5.2 fw=5.6 as v57
 *
 * In general firmware versions differ in status or configuration
 * parameters they provide. Those are available in individual files
 * called RocPlugin_<version>.cpp. RocPlugin will refuse to communicate
 * with unsupported versions.
 *
 * ROC v5.4 reports wrong packet length in READ_CONFIG response for which
 * a workaround is to overload rspReadConfig() function to modify the
 * packet before letting base implementation do the rest.  This is fixed in ROC v5.5
 *
 * ROC v5.6 is a variant of v5.2 that adds 3 extra registers to control pattern generator.
 *
 * ROC v5.7 is a variant of v5.5 that adds 3 extra registers to control pattern generator.
 */
class RocPlugin : public BaseModulePlugin {
    public: // variables
        static const int defaultInterfaceMask = BaseModulePlugin::defaultInterfaceMask | asynOctetMask;
        static const int defaultInterruptMask = BaseModulePlugin::defaultInterruptMask | asynOctetMask;

    private: // structures and definitions
        static const unsigned NUM_ROCPLUGIN_DYNPARAMS;      //!< Maximum number of asyn parameters, including the status and configuration parameters
        static const unsigned NUM_CHANNELS = 8;             //!< Number of channels connected to ROC
        static const float    NO_RESPONSE_TIMEOUT;          //!< Timeout to wait for response from ROC, in seconds

    private: // variables
        std::string m_version;                              //!< Version string as passed to constructor
        Fifo<char> m_hvBuffer;                              //!< FIFO buffer for data received from HV module but not yet processed

    public: // functions

        /**
         * Constructor for RocPlugin
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
        RocPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

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
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see RocPlugin::parseVersionRsp()
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
         * Handle READ_CONFIG response from v5.4.
         *
         * Function handles workaround for broken v5.4 firmware version which
         * appends 4 unexpected bytes at the end of the payload. Packet is copied
         * to internal buffer and the length is modified. Then BaseModulePlugin::rspReadConfig()
         * is invoked with the modified packet.
         * For non-v5.4 firmwares the function simply invokes BaseModulePlugin::rspReadConfig()
         * passing it the original packet.
         */
        bool rspReadConfig(const DasPacket *packet);

        /**
         * Override START response handler.
         *
         * Implemented only for v5.4 and v5.5 to detect successful acquisition start
         * and record it through Acquiring parameter. Other ROC version have a dedicated
         * status register for that.
         */
        bool rspStart(const DasPacket *packet);

        /**
         * Override STOP response handler.
         *
         * Implemented only for v5.4 and v5.5 to detect successful acquisition stop
         * and record it through Acquiring parameter. Other ROC version have a dedicated
         * status register for that.
         */
        bool rspStop(const DasPacket *packet);

        /**
         * Pass user command to HighVoltage module through RS232.
         *
         * HV command string is packed into OCC packet and sent to the
         * HV module. HV module does not have its own hardware id,
         * instead the ROC board is used as a router.
         *
         * @param[in] data String representing HV command, must include CR (ASCII 13)
         * @param[in] length Length of the string
         */
        void reqHvCmd(const char *data, uint32_t length);

        /**
         * Receive and join responses from HV module.
         *
         * ROC will send one OCC packet for each character from HV module
         * response. This function concatenates characters back together
         * and keep them in internal buffer until the user reads them.
         *
         * @param[in] packet with HV module response
         * @return true if packet was processed
         */
        bool rspHvCmd(const DasPacket *packet);

        /**
         * Create and register all status ROC v4.4/v4.5 parameters to be exposed to EPICS.
         */
        void createStatusParams_v45();

        /**
         * Create and register all config ROC v4.4/v4.5 parameters to be exposed to EPICS.
         */
        void createConfigParams_v45();

        /**
         * Create and register all status ROC v5.1 parameters to be exposed to EPICS.
         */
        void createStatusParams_v51();

        /**
         * Create and register all config ROC v5.1 parameters to be exposed to EPICS.
         */
        void createConfigParams_v51();

        /**
         * Create and register all temperature ROC v5.1 parameters to be exposed to EPICS.
         */
        void createTemperatureParams_v51();

        /**
         * Create and register all status ROC v5.2 parameters to be exposed to EPICS.
         */
        void createStatusParams_v52();

        /**
         * Create and register all config ROC v5.2 parameters to be exposed to EPICS.
         */
        void createConfigParams_v52();

        /**
         * Create and register all temperature ROC v5.2 parameters to be exposed to EPICS.
         */
        void createTemperatureParams_v52();

        /**
         * Create and register all status ROC v5.4/v5.5 parameters to be exposed to EPICS.
         */
        void createStatusParams_v54();

        /**
         * Create and register all config ROC v5.4/v5.5 parameters to be exposed to EPICS.
         */
        void createConfigParams_v54();

        /**
         * Create and register all temperature ROC v5.4 parameters to be exposed to EPICS.
         */
        void createTemperatureParams_v54();

        /**
         * Create and register all status ROC v5.6 parameters to be exposed to EPICS.
         */
        void createStatusParams_v56();

        /**
         * Create and register all status counter ROC v5.6 parameters to be exposed to EPICS.
         */
        void createCounterParams_v56();

        /**
         * Create and register all config ROC v5.6 parameters to be exposed to EPICS.
         */
        void createConfigParams_v56();

        /**
         * Create and register all temperature ROC v5.6 parameters to be exposed to EPICS.
         */
        void createTemperatureParams_v56();

        /**
         * Create and register all status ROC v5.7 parameters to be exposed to EPICS.
         */
        void createStatusParams_v57();

        /**
         * Create and register all status counter ROC v5.7 parameters to be exposed to EPICS.
         */
        void createCounterParams_v57();

        /**
         * Create and register all config ROC v5.7 parameters to be exposed to EPICS.
         */
        void createConfigParams_v57();

        /**
         * Create and register all temperature ROC v5.7 parameters to be exposed to EPICS.
         */
        void createTemperatureParams_v57();

    protected:
        #define FIRST_ROCPLUGIN_PARAM Acquiring
        int Acquiring;
        #define LAST_ROCPLUGIN_PARAM Acquiring
};

#endif // DSP_PLUGIN_H
