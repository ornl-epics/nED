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
 * * ADC ROC hw=0.1 fw=0.3 as v03
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
         * Try to parse the ADCROC version response packet an populate the structure.
         *
         * Function will parse all known ADCROC version responses and populate the
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
         * @see AdcRocPlugin::parseVersionRsp()
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
         * Process ADC ROC custom commands.
         *
         * This includes handling pulsed magnet trigger and clear commands.
         *
         * @param[in] command requested.
         * @param[out] timeout before giving up waiting for response, default is 2.0
         * @return Response to be waited for.
         */
        DasPacket::CommandType handleRequest(DasPacket::CommandType command, double &timeout);

        /**
         * Process ADC ROC custom response commands.
         *
         * This includes handling pulsed magnet trigger and clear commands.
         *
         * @param[in] packet to be handled
         */
        bool handleResponse(const DasPacket *packet);

        /**
         * Request a single pulse magnet pulse
         *
         * Function sends a simple packet with request to trigger one pulse.
         * Pulse must be cleared using reqClearPulse() before next one is being
         * triggered.
         *
         * Valid only on modified ADC ROC that supports pulse magnet interface.
         */
        DasPacket::CommandType reqTriggerPulseMagnet();

        /**
         * Request clearing previous pulse magnet pulse.
         *
         * After a pulse magnet pulse is triggered it must be cleared (line
         * deasserted) before the next trigger can be sent.
         *
         * Valid only on modified ADC ROC that supports pulse magnet interface.
         */
        DasPacket::CommandType reqClearPulseMagnet();

        /**
         * Handle pulsed magnet trigger response.
         *
         * @param[in] packet with response to pulse request
         * @retval true Received command ACK in time.
         * @retval false Timeout has occurred or NACK received.
         */
        bool rspTriggerPulseMagnet(const DasPacket *packet);

        /**
         * Handle pulsed magnet clear response.
         *
         * @param[in] packet with response to pulse clear
         * @retval true Received command ACK in time.
         * @retval false Timeout has occurred or NACK received.
         */
        bool rspClearPulseMagnet(const DasPacket *packet);

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
};

#endif // ADC_ROC_PLUGIN_H
