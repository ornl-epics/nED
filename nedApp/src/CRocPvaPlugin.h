/* CRocPvaPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 */

#ifndef CROCROC_PVA_PLUGIN_H
#define CRICROC_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin that forwards BNLROC data to software clients over EPICS v4.
 */
class CRocPvaPlugin : public BasePvaPlugin {
    private:
        /**
         * A cache to store data until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector position_index;
            epics::pvData::PVUIntArray::svector pixel;
            epics::pvData::PVUShortArray::svector photon_count_g[20];
            epics::pvData::PVUShortArray::svector photon_count_x[11];
            epics::pvData::PVUShortArray::svector photon_count_y[7];
            epics::pvData::PVUShortArray::svector time_range[4];
        } m_cache;

        /**
         * Number of elements in each cache array.
         */
        static const uint32_t CACHE_SIZE;

    public:
        /**
         * Constructor
         *
         * @param[in] portName            asyn port name.
         * @param[in] dispatcherPortName  Name of the dispatcher asyn port to
         * @param[in] pvPrefix            Prefix for the PV Record
         */
        CRocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

        /**
         * Overloaded function to handle DataMode parameter.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process incoming data as 'RAW' formatted data.
         *
         * @param[in] packet Packet to be processed
         */
        void processRawData(const uint32_t *data, uint32_t count);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processRawData(BasePvaPlugin *this_, const uint32_t *data, uint32_t count) {
            reinterpret_cast<CRocPvaPlugin *>(this_)->processRawData(data, count);
        }

        /**
         * Process incoming data as 'EXTENDED' formatted data.
         *
         * @param[in] packet Packet to be processed
         */
        void processExtendedData(const uint32_t *data, uint32_t count);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processExtendedData(BasePvaPlugin *this_, const uint32_t *data, uint32_t count) {
            reinterpret_cast<CRocPvaPlugin *>(this_)->processExtendedData(data, count);
        }

        /**
         * Post data received (raw mode) and clear cache
         *
         * Cached data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pvRecord Structure to update.
         */
        void postRawData(const PvaNeutronData::shared_pointer& pvRecord);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void postRawData(BasePvaPlugin *this_,const PvaNeutronData::shared_pointer& pvRecord) {
            reinterpret_cast<CRocPvaPlugin *>(this_)->postRawData(pvRecord);
        }

        /**
         * Post data received (extended mode) and clear cache
         *
         * Cached data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pvRecord Structure to update.
         */
        void postExtendedData(const PvaNeutronData::shared_pointer& pvRecord);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void postExtendedData(BasePvaPlugin *this_, const PvaNeutronData::shared_pointer& pvRecord) {
            reinterpret_cast<CRocPvaPlugin *>(this_)->postExtendedData(pvRecord);
        }

        /**
         * Clear internal cache
         */
        void flushData();

        /**
         * Resize internal cache vectors.
         */
        void reserve();
};

#endif // BNLROC_PVA_PLUGIN_H
