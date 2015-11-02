/* BnlRocPvaPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 */

#ifndef BNLROC_PVA_PLUGIN_H
#define BNLROC_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin that forwards BNLROC data to software clients over EPICS v4.
 */
class BnlRocPvaPlugin : public BasePvaPlugin {
    private:
        /**
         * A cache to store data until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector position_index;
            epics::pvData::PVFloatArray::svector position_x;
            epics::pvData::PVFloatArray::svector position_y;
            epics::pvData::PVUShortArray::svector sample_x1;
            epics::pvData::PVUShortArray::svector sample_x2;
            epics::pvData::PVUShortArray::svector sample_x3;
            epics::pvData::PVUShortArray::svector sample_x4;
            epics::pvData::PVUShortArray::svector sample_x5;
            epics::pvData::PVUShortArray::svector sample_x6;
            epics::pvData::PVUShortArray::svector sample_x7;
            epics::pvData::PVUShortArray::svector sample_x8;
            epics::pvData::PVUShortArray::svector sample_x9;
            epics::pvData::PVUShortArray::svector sample_x10;
            epics::pvData::PVUShortArray::svector sample_x11;
            epics::pvData::PVUShortArray::svector sample_x12;
            epics::pvData::PVUShortArray::svector sample_x13;
            epics::pvData::PVUShortArray::svector sample_x14;
            epics::pvData::PVUShortArray::svector sample_x15;
            epics::pvData::PVUShortArray::svector sample_x16;
            epics::pvData::PVUShortArray::svector sample_x17;
            epics::pvData::PVUShortArray::svector sample_x18;
            epics::pvData::PVUShortArray::svector sample_x19;
            epics::pvData::PVUShortArray::svector sample_x20;
            epics::pvData::PVUShortArray::svector sample_y1;
            epics::pvData::PVUShortArray::svector sample_y2;
            epics::pvData::PVUShortArray::svector sample_y3;
            epics::pvData::PVUShortArray::svector sample_y4;
            epics::pvData::PVUShortArray::svector sample_y5;
            epics::pvData::PVUShortArray::svector sample_y6;
            epics::pvData::PVUShortArray::svector sample_y7;
            epics::pvData::PVUShortArray::svector sample_y8;
            epics::pvData::PVUShortArray::svector sample_y9;
            epics::pvData::PVUShortArray::svector sample_y10;
            epics::pvData::PVUShortArray::svector sample_y11;
            epics::pvData::PVUShortArray::svector sample_y12;
            epics::pvData::PVUShortArray::svector sample_y13;
            epics::pvData::PVUShortArray::svector sample_y14;
            epics::pvData::PVUShortArray::svector sample_y15;
            epics::pvData::PVUShortArray::svector sample_y16;
            epics::pvData::PVUShortArray::svector sample_y17;
        } m_cache;

        /**
         * Number of elements in each cache array.
         */
        static const uint32_t CACHE_SIZE;

        float m_xyDivider; //!< X,Y is in UQm.n format, divider is (1 << n)

    public:
        /**
         * Constructor
         *
         * @param[in] portName            asyn port name.
         * @param[in] dispatcherPortName  Name of the dispatcher asyn port to
         * @param[in] pvPrefix            Prefix for the PV Record
         */
        BnlRocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

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
            reinterpret_cast<BnlRocPvaPlugin *>(this_)->processRawData(data, count);
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
            reinterpret_cast<BnlRocPvaPlugin *>(this_)->processExtendedData(data, count);
        }

        /**
         * Process incoming data as normal neutron data.
         *
         * @param[in] packet Packet to be processed
         */
        void processNormalData(const uint32_t *data, uint32_t count);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processNormalData(BasePvaPlugin *this_, const uint32_t *data, uint32_t count) {
            reinterpret_cast<BnlRocPvaPlugin *>(this_)->processNormalData(data, count);
        }

        /**
         * Post data received (normal mode) and clear cache
         *
         * Cached data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pvRecord Structure to update.
         */
        void postNormalData(const PvaNeutronData::shared_pointer& pvRecord);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void postNormalData(BasePvaPlugin *this_, const PvaNeutronData::shared_pointer& pvRecord) {
            reinterpret_cast<BnlRocPvaPlugin *>(this_)->postNormalData(pvRecord);
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
            reinterpret_cast<BnlRocPvaPlugin *>(this_)->postRawData(pvRecord);
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
            reinterpret_cast<BnlRocPvaPlugin *>(this_)->postExtendedData(pvRecord);
        }

        /**
         * Clear internal cache
         */
        void flushData();

        /**
         * Resize internal cache vectors.
         */
        void reserve();

    private:
        #define FIRST_BNLROCPVAPLUGIN_PARAM NormalDataFormat
        int NormalDataFormat;
        int XyFractWidth;
        #define LAST_BNLROCPVAPLUGIN_PARAM XyFractWidth
};

#endif // BNLROC_PVA_PLUGIN_H
