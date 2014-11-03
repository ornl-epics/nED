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
         * Overloaded function to handle DataMode parameter.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process incoming packet as normal ACPC neutron data.
         *
         * Iterate through packet events and save their data to cache.
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
        void processNormalPacket(const DasPacket * const packet);

        /**
         * Static C callable wrapper for member function of the same name.
         */
        static void processNormalPacket(BasePvaPlugin *this_, const DasPacket * const packet) {
            reinterpret_cast<AcpcPvaPlugin *>(this_)->processNormalPacket(packet);
        }

        /**
         * Post data received so far and clear cache
         *
         * Cached data is posted as a single update to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pulseTime Timestamp of pulse to be posted.
         * @param[in] pulseCharge Pulse charge
         * @param[in] pulseSeq Pulse sequence number, monotonically increasing
         */
        void postNormalData(const epicsTimeStamp &pulseTime, double pulseCharge, uint32_t pulseSeq);

        /**
         * Static C callable wrapper for member function of the same name.
         */
        static void postNormalData(BasePvaPlugin *this_, const epicsTimeStamp &pulseTime, double pulseCharge, uint32_t pulseSeq) {
            reinterpret_cast<AcpcPvaPlugin *>(this_)->postNormalData(pulseTime, pulseCharge, pulseSeq);
        }

    private:
        /**
         * A cache to store data until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector pixel;
            epics::pvData::PVUIntArray::svector position_index;
            epics::pvData::PVUIntArray::svector position_x;
            epics::pvData::PVUIntArray::svector position_y;
            epics::pvData::PVUIntArray::svector photo_sum_x;
            epics::pvData::PVUIntArray::svector photo_sum_y;
        } m_cache;
};

#endif // ACPC_PVA_PLUGIN_H
