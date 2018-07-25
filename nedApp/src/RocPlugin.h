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
 * * ROC hw=2.65 fw=1.4 as v14 (GE board)
 * * ROC hw=2.0 fw=4.3 as v43
 * * ROC hw=2.0 fw=4.4 as v44
 * * ROC hw=2.0 fw=4.5 as v45
 * * ROC hw=2.0 fw=4.7 as v47
 * * ROC hw=2.0 fw=5.0 as v50 (NOTE: potential fw version clash with hw=5.2 fw=5.0)
 * * ROC hw=5.2 fw=5.1 as v51
 * * ROC hw=5.2 fw=5.2 as v52
 * * ROC hw=5.2 fw=5.4 as v54
 * * ROC hw=5.2 fw=5.5 as v55 (via v54 plugin)
 * * ROC hw=5.2 fw=5.6 as v56
 * * ROC hw=5.2 fw=5.6 as v57
 * * ROC hw=5.2 fw=5.8 as v58
 * * ROC hw=5.2 fw=5.8 as v59
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

        epicsTimeStamp m_sendHvTime;

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
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] version ROC HW&SW version, ie. V5_50
         */
        RocPlugin(const char *portName, const char *parentPlugins, const char *version);

        /**
         * Process ROC custom commands.
         *
         * This includes handling pre-amp configuration and trigger responses.
         *
         * @param[in] command requested.
         * @param[out] timeout before giving up waiting for response, default is 2.0
         * @return Response to be waited for.
         */
        DasCmdPacket::CommandType handleRequest(DasCmdPacket::CommandType command, double &timeout);

        /**
         * Process ROC custom response commands.
         *
         * This includes handling pre-amp configuration and trigger responses.
         *
         * @param[in] packet to be handled
         */
        bool handleResponse(const DasCmdPacket *packet);

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
        bool rspReadConfig(const DasCmdPacket *packet, uint8_t channel);

        /**
         * Override START response handler.
         *
         * Implemented only for v5.4 and v5.5 to detect successful acquisition start
         * and record it through Acquiring parameter. Other ROC version have a dedicated
         * status register for that.
         */
        bool rspStart(const DasCmdPacket *packet);

        /**
         * Override STOP response handler.
         *
         * Implemented only for v5.4 and v5.5 to detect successful acquisition stop
         * and record it through Acquiring parameter. Other ROC version have a dedicated
         * status register for that.
         */
        bool rspStop(const DasCmdPacket *packet);

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
        bool rspHvCmd(const DasCmdPacket *packet);

        /**
         * Sends pre-amp configuration.
         *
         * Functionality not implemented and always returns 0.
         *
         * @return Response to wait for.
         */
        DasCmdPacket::CommandType reqConfigPreAmp();

        /**
         * Sends a pre-amp trigger command.
         *
         * Functionality not implemented and always returns 0.
         *
         * @return Response to wait for.
         */
        DasCmdPacket::CommandType reqTriggerPreAmp();

        /**
         * Handle pre-amp configuration response.
         */
        bool rspConfigPreAmp(const DasCmdPacket *packet);

        /**
         * Handle pre-amp trigger response.
         */
        bool rspTriggerPreAmp(const DasCmdPacket *packet);

        /**
         * Create pre-amp config parameter.
         */
        void createPreAmpCfgParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, int value);

        /**
         * Create pre-amp trigger parameter.
         */
        void createPreAmpTrigParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, int value);

        /**
         * Create and register all ROC v1.4 parameters to be exposed to EPICS.
         */
        void createParams_v14();

        /**
         * Create and register all ROC v4.3 parameters to be exposed to EPICS.
         */
        void createParams_v43();

        /**
         * Create and register all ROC v4.4/v4.5 parameters to be exposed to EPICS.
         */
        void createParams_v45();

        /**
         * Create and register all ROC v4.7 parameters to be exposed to EPICS.
         */
        void createParams_v47();

        /**
         * Create and register all ROC v5.0 parameters to be exposed to EPICS.
         */
        void createParams_v50();

        /**
         * Create and register all ROC v5.1 parameters to be exposed to EPICS.
         */
        void createParams_v51();

        /**
         * Create and register all ROC v5.2 parameters to be exposed to EPICS.
         */
        void createParams_v52();

        /**
         * Create and register all ROC v5.4/v5.5 parameters to be exposed to EPICS.
         */
        void createParams_v54();

        /**
         * Create and register all ROC v5.6 parameters to be exposed to EPICS.
         */
        void createParams_v56();

        /**
         * Create and register all ROC v5.7 parameters to be exposed to EPICS.
         */
        void createParams_v57();

        /**
         * Create and register all ROC v5.8 parameters.
         */
        void createParams_v58();

        /**
         * Create and register all ROC v5.9 parameters.
         */
        void createParams_v59();

    protected:
        #define FIRST_ROCPLUGIN_PARAM Acquiring
        int Acquiring;
        int HvDelay;
        int HvB2bDelay;
        #define LAST_ROCPLUGIN_PARAM HvB2bDelay
};

#endif // DSP_PLUGIN_H
