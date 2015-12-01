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
 * Plugin for exporting ACPC neutron data through EPICSv4 pvaccess.
 *
 * There are four modes supported by this plugin:
 *   - in normal mode ACPC calculates X,Y position and photosum but the format
 *     is different than DasPacket::Event. This mode is useful for flat-field
 *     calibration.
 *   - in normal mode FlatFieldEn parameter can be turned on to tell this
 *     plugin that data has been previously flat-field corrected and transformed
 *     into DasPacket::Event format.
 *   - in raw mode ACPC passes through AROC data
 *   - in verbose mode ACPC calculates X,Y, photosum but also gives
 *     detailed sample data (currently not implemented)
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
         * @param[in] pvPrefix Prefix for the PV Record
         */
        AcpcPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

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
        void processNormalData(const uint32_t *data, uint32_t count);

        /**
         * Static C callable wrapper for member function of the same name.
         */
        static void processNormalData(BasePvaPlugin *this_, const uint32_t *data, uint32_t count) {
            reinterpret_cast<AcpcPvaPlugin *>(this_)->processNormalData(data, count);
        }

        /**
         * Post data received so far and clear cache
         *
         * Cached data is posted as a single update to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pvRecord to be updated
         */
        void postNormalData(const PvaNeutronData::shared_pointer& pvRecord);

        /**
         * Static C callable wrapper for member function of the same name.
         */
        static void postNormalData(BasePvaPlugin *this_, const PvaNeutronData::shared_pointer& pvRecord) {
            reinterpret_cast<AcpcPvaPlugin *>(this_)->postNormalData(pvRecord);
        }

        /**
         * Process incoming packet as raw ACPC neutron data.
         *
         * Iterate through packet events and save their data to cache.
         *
         * Recent ACPC firmwares produce events data in normal mode as:
         *   - time of flight
         *   - position index
         *   - 8 * sample A
         *   - 8 * sample B
         */
        void processRawData(const uint32_t *data, uint32_t count);

        /**
         * Static C callable wrapper for member function of the same name.
         */
        static void processRawData(BasePvaPlugin *this_, const uint32_t *data, uint32_t count) {
            reinterpret_cast<AcpcPvaPlugin *>(this_)->processRawData(data, count);
        }

        /**
         * Post raw data received so far and clear cache
         *
         * Cached data is posted as a single update to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pvRecord to be updated
         */
        void postRawData(const PvaNeutronData::shared_pointer& pvRecord);

        /**
         * Static C callable wrapper for member function of the same name.
         */
        static void postRawData(BasePvaPlugin *this_, const PvaNeutronData::shared_pointer& pvRecord) {
            reinterpret_cast<AcpcPvaPlugin *>(this_)->postRawData(pvRecord);
        }

        /**
         * Clear internal cache
         */
        void flushData();

    private:
        /**
         * A cache to store data until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector pixel;
            epics::pvData::PVUIntArray::svector position_index;
            epics::pvData::PVFloatArray::svector position_x;
            epics::pvData::PVFloatArray::svector position_y;
            epics::pvData::PVFloatArray::svector photo_sum_x;
            epics::pvData::PVFloatArray::svector photo_sum_y;
            epics::pvData::PVUIntArray::svector sample_a8;
            epics::pvData::PVUIntArray::svector sample_b8;
        } m_cache;

        /**
         * Number of elements in each cache array.
         */
        static const uint32_t CACHE_SIZE;

        uint32_t m_xyDivider; // X,Y is in UQm.n format, divider is (1 << n)
        uint32_t m_photosumDivider; // Photo Sum is in UQm.n format, divider is (1 << n)

    protected:
        #define FIRST_ACPCPVAPLUGIN_PARAM FlatFieldEn
        int FlatFieldEn;
        int XyFractWidth;
        int PsFractWidth;
        #define LAST_ACPCPVAPLUGIN_PARAM PsFractWidth
};

#endif // ACPC_PVA_PLUGIN_H
