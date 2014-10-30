/* AcpcPvaPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef ACPC_PVA_PLUGIN_H
#define ACPC_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin for exporting ACPC data through EPICSv4 pvaccess
 *
 * There are three modes supported by this plugin:
 *   - in normal mode ACPC calculates X,Y position and photosum
 *   - in raw mode ACPC passes through AROC data
 *   - in verbose mode ACPC calculates X,Y, photosum but also gives
 *     detailed sample data
 *
 * Plugin ensures to update all required PV structure fields for selected mode.
 * It filters out command packets and packets with no RTDL information.
 * It does not detect old pulses. But it does guarantee not to push data from
 * different pulses in single update. Data for single pulse might be split into
 * several pushes.
 */
class AcpcPvaPlugin : public BasePvaPlugin {
    public: // functions
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvName Name for the EPICSv4 PV serving the data.
         */
        AcpcPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName);

        /**
         * Process incoming data as normal ACPC neutron data.
         *
         * Recent ACPC firmwares produce events data in normal mode as 6 4-byte
         * values:
         *   - time of flight
         *   - position index
         *   - position x
         *   - position y
         *   - photo sum x
         *   - photo sum y
         */
        virtual void processDataNormal(const DasPacketList * const packetList);
};

#endif // ACPC_PVA_PLUGIN_H
