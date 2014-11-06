#ifndef ROC_PVA_PLUGIN_H
#define ROC_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin that forwards ROC data to software clients over EPICS v4.
 *
 * RocPlugin provides following asyn parameters:
 * asyn param    | asyn param type | init val | mode | Description
 * ------------- | --------------- | -------- | ---- | -----------
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
        } m_cache;
    public:
        /**
         * Constructor
	     *
	     * @param[in] portName            asyn port name.
	     * @param[in] dispatcherPortName  Name of the dispatcher asyn port to 
         *                                  connect to.
	     * @param[in] pvPrefix            Prefix for the PV Record
         */
        RocPvaPlugin(const char *portName, const char *dispatcherPortName, 
            const char *pvPrefix);

        /**
         * Overloaded function to handle DataMode parameter.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

	    /**
         * Process incoming data as 'RAW' formatted data.
         *
	     * @param[in] packet Packet to be processed
         */
        void processRawPacket(const DasPacket * const packet);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processRawPacket(BasePvaPlugin *this_, const DasPacket *
            const packet) {
            reinterpret_cast<RocPvaPlugin *>(this_)->processRawPacket(packet);
        }

        /**
         * Process incoming data as 'EXTENDED' formatted data.
         *
	     * @param[in] packet Packet to be processed
         */
        void processExtendedPacket(const DasPacket * const packet);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processExtendedPacket(BasePvaPlugin *this_, const DasPacket *
            const packet) {
            reinterpret_cast<RocPvaPlugin *>(this_)->processExtendedPacket(packet);
        }

	    /**
         * Process incoming data as normal neutron data.
         *
	     * @param[in] packet Packet to be processed
         */
        void processNormalPacket(const DasPacket * const packet);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void processNormalPacket(BasePvaPlugin *this_, const DasPacket *
            const packet) {
            reinterpret_cast<RocPvaPlugin *>
                (this_)->processNormalPacket(packet);
        }

        /**
         * Post data received (normal mode) and clear cache
         *
         * Cached data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pulseTime     Timestamp of pulse to be posted.
         * @param[in] pulseCharge   Pulse charge
         * @param[in] pulseSeq      Pulse seq number, monotonically increasing
         */
        void postNormalData(const epicsTimeStamp &pulseTime, double pulseCharge,
            uint32_t pulseSeq);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void postNormalData(BasePvaPlugin *this_, 
            const epicsTimeStamp &pulseTime, double pulseCharge, 
            uint32_t pulseSeq) {
            reinterpret_cast<RocPvaPlugin *>(this_)->postNormalData(pulseTime, 
                pulseCharge, pulseSeq);
        }

        /**
         * Post data received (raw mode) and clear cache
         *
         * Cached data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pulseTime     Timestamp of pulse to be posted.
         * @param[in] pulseCharge   Pulse charge
         * @param[in] pulseSeq      Pulse seq number, monotonically increasing
         */
        void postRawData(const epicsTimeStamp &pulseTime, double pulseCharge,
            uint32_t pulseSeq);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void postRawData(BasePvaPlugin *this_, 
            const epicsTimeStamp &pulseTime, double pulseCharge, 
            uint32_t pulseSeq) {
            reinterpret_cast<RocPvaPlugin *>(this_)->postRawData(pulseTime, 
                pulseCharge, pulseSeq);
        }
      
        /**
         * Post data received (extended mode) and clear cache
         *
         * Cached data is posted as a single event to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pulseTime     Timestamp of pulse to be posted.
         * @param[in] pulseCharge   Pulse charge
         * @param[in] pulseSeq      Pulse seq number, monotonically increasing
         */
        void postExtendedData(const epicsTimeStamp &pulseTime, double pulseCharge,
            uint32_t pulseSeq);

        /**
         * Static C callable wrapper for member function of the same name
         */
        static void postExtendedData(BasePvaPlugin *this_, 
            const epicsTimeStamp &pulseTime, double pulseCharge, 
            uint32_t pulseSeq) {
            reinterpret_cast<RocPvaPlugin *>(this_)->postExtendedData(pulseTime, 
                pulseCharge, pulseSeq);
        }

 };

#endif // ROC_PVA_PLUGIN_H
