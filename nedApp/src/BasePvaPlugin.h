/* BasePvaPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASE_PVA_PLUGIN_H
#define BASE_PVA_PLUGIN_H

#include "BasePlugin.h"
#include "PvaNeutronData.h"
#include "PvaMetaData.h"

/**
 * Abstract plugin for exporting data through EPICSv4 pvaccess
 *
 * The base PVA plugin provides functionality common to all plugins that need
 * to export data through EPICSv4 pvaccess. All detectors share the same EPICSv4
 * PVRecord structure definition. This class creates 1 instance of that PVRecord.
 * BasePvaPlugin class guarantees that the m_pvRecord is created automatically
 * and is always available to the specialized plugin, or the plugin will be
 * disabled from start on regardless the attempt to enable it through PV.
 *
 * Derived classes should handle DataMode switching mode and set new callbacks
 * to handle data. This is the most efficient way of packet processing data
 * decoupled from packet detection.
 */
class BasePvaPlugin : public BasePlugin {
    public:
        /*
         * C callback function to process neutrons packet data.
         *
         * @param[in] this_ Pointer to plugin instance (this).
         * @param[in] Pointer to the start of the data.
         * @param[in] Size of the data in 4-bytes
         */
        typedef void (*ProcessDataCb)(BasePvaPlugin *this_, const uint32_t *data, uint32_t dataLen);

        /*
         * C callback function to post neutrons data collected since previous post.
         *
         * This calback defined by derived plugin is invoked when data needs
         * to be pushed out. This happens when new pulse is detected or there's
         * currently no more data available. The timeStamp and proton_charge
         * attributes of Neutrons structure are already populated before the
         * callback is invoked. Also the transaction on pvRecord has been
         * started before the callback is invoked.
         *
         * @param[in] this_ Pointer to plugin instance (this).
         * @param[in] pvRecord PV record to push neutrons data to.
         */
        typedef void (*PostDataCb)(BasePvaPlugin *, const PvaNeutronData::shared_pointer& pvRecord);

        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvName Name for the EPICSv4 PV serving the data.
         */
        BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName);

        /**
         * Abstract destructor.
         */
        virtual ~BasePvaPlugin() = 0;

        /**
         * Overloaded writeInt32 handler.
         *
         * Intercept Enable parameter to disallow enabling the plugin if
         * pvRecord initialization failed.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function to receive all OCC data.
         *
         * Function filters out non-data packets and packets without valid RTDL
         * header. It ensures all packet accounting. It delegates processing of
         * packet to external function provided through callbacks. When new pulse
         * is detected or when running out of data, a callback to post data is called.
         */
        void processData(const DasPacketList * const packetList);

        /**
         * Parse data as metadata and populate metadata arrays.
         *
         * @param[in] data Raw data to be parsed.
         * @param[in] count Number of 4-byte elements in raw data.
         */
        void processMetaData(const uint32_t *data, uint32_t count);

        /**
         * Set callback functions for processing neutrons packet and posting data.
         *
         * Static C functions were selected for performance reasons. At
         * least packet processing callback is invoked quite often and other
         * implementations (virtual function, std::function are all slower).
         *
         * @param[in] procNeutronsCb Neutrons packet data processing function
         * @param[in] postNeutronsCb Function to be called when PV update should be done.
         */
        void setCallbacks(ProcessDataCb procNeutronCb, PostDataCb postNeutronsCb);

    protected:
        PvaNeutronData::shared_pointer m_pvNeutrons;
        PvaNeutronData::shared_pointer m_pvMetadata;

    private:
        uint32_t m_nReceived;       //!< Number of packets received
        uint32_t m_nProcessed;      //!< Number of data packets processed
        epicsTimeStamp m_pulseTime; //!< Current pulse EPICS timestamp
        double m_pulseCharge;       //!< Current pulse charge
        uint32_t m_postSeq;         //!< Current post sequence id
        ProcessDataCb m_processNeutronCb; //!< Callback function ptr to process neutron packet data
        PostDataCb m_postNeutronsCb;//!< Callback function ptr to post data collected so far

        /**
         * A cache to store metadata until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector pixel;
        } m_cacheMeta;

        /**
         * Invoke derived plugin callback to send data.
         */
        void postData(bool postNeutrons, bool postMetadata);

    private: // statics

        /**
         * Number of elements in each cache array.
         */
        static const uint32_t CACHE_SIZE;

        /**
         * Neutrons PV id
         */
        static const std::string PV_NEUTRONS;

        /**
         * Metadata PV id
         */
        static const std::string PV_METADATA;

    private: // asyn parameters
        #define FIRST_BASEPVAPLUGIN_PARAM 0
        #define LAST_BASEPVAPLUGIN_PARAM 0
};

#endif // BASE_PVA_PLUGIN_H
