/* DspWPlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DSPW_PLUGIN_H
#define DSPW_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for DSP-W module.
 */
class DspWPlugin : public BaseModulePlugin {
    private: // structures and definitions
        static const double DSPW_RESPONSE_TIMEOUT;           //!< Default DSP-W response timeout, in seconds
        std::string m_version;

    public:

        /**
         * Constructor for DspWPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Plugins to connect to
         * @param[in] hardwareId Hardware ID of the DSP module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version Configured module version, must match the actual version
         */
        DspWPlugin(const char *portName, const char *parentPlugins, const char *hardwareId, const char *version);

        /**
         * Try to parse the DSP-W version response packet an populate the structure.
         *
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see DspPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

    private:

        /**
         * Create and register all 1.0 parameters to be exposed to EPICS.
         */
        void createParams_v10();
};

#endif // DSPW_PLUGIN_H
