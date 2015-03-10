#ifndef ROC_PVA_PLUGIN_H
#define ROC_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin that forwards ROC data to software clients over EPICS v4.
 */
class RocPvaPlugin : public BasePvaPlugin {
    private:
        /**
         * A cache to store data until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector pixel;
            epics::pvData::PVUIntArray::svector position_index;
            epics::pvData::PVUIntArray::svector sample_a1;
            epics::pvData::PVUIntArray::svector sample_b1;

            epics::pvData::PVUIntArray::svector meta_time_of_flight;
            epics::pvData::PVUIntArray::svector meta_pixel;
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
        RocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

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
            reinterpret_cast<RocPvaPlugin *>(this_)->processRawData(data, count);
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
            reinterpret_cast<RocPvaPlugin *>(this_)->processExtendedData(data, count);
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
            reinterpret_cast<RocPvaPlugin *>(this_)->processNormalData(data, count);
        }

        /**
         * Process incoming data as meta data.
         *
         * @param[in] packet Packet to be processed
         */
        void processMetaData(const uint32_t *data, uint32_t count);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processMetaData(BasePvaPlugin *this_, const uint32_t *data, uint32_t count) {
            reinterpret_cast<RocPvaPlugin *>(this_)->processMetaData(data, count);
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
            reinterpret_cast<RocPvaPlugin *>(this_)->postNormalData(pvRecord);
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
            reinterpret_cast<RocPvaPlugin *>(this_)->postRawData(pvRecord);
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
            reinterpret_cast<RocPvaPlugin *>(this_)->postExtendedData(pvRecord);
        }

        /**
         * Post meta data received and clear cache
         *
         * Cached meta data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pulseTime     Timestamp of pulse to be posted.
         * @param[in] pulseCharge   Pulse charge
         * @param[in] pulseSeq      Pulse seq number, monotonically increasing
         */
        void postMetaData();
};

#endif // ROC_PVA_PLUGIN_H
