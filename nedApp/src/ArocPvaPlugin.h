/* ArocPvaPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef AROC_PVA_PLUGIN_H
#define AROC_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin for exporting AROC data through EPICSv4 pvaccess
 *
 * There are three modes supported by this plugin, normal neutron event mode
 * through the flat-field correction plugin, AROC raw mode and AROC extended
 * mode.
 */
class ArocPvaPlugin : public BasePvaPlugin {
    public: // functions
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        ArocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

        /**
         * Process incoming data as normal detector data.
         */
        virtual void processDataNormal(const DasPacketList * const packetList);

        /**
         * Process incoming data as raw detector data.
         */
        void processDataRaw(const DasPacketList * const packetList);

        /**
         * Process incoming data as extended detector data.
         */
        void processDataExtended(const DasPacketList * const packetList);

    private: // asyn parameters
        #define FIRST_AROCPVAPLUGIN_PARAM Mode
        #define LAST_AROCPVAPLUGIN_PARAM Mode
};

#endif // AROC_PVA_PLUGIN_H
