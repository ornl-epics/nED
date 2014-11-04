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
         * C callback function to process packet data.
         *
         * @param[in] this_ Pointer to plugin instance (this).
         * @param[in] packet Packet to be processed.
         */
        typedef void (*ProcessPacketCb)(BasePvaPlugin *this_, const DasPacket *packet);

        /*
         * C callback function to post all data collected since previous post.
         *
         * @param[in] this_ Pointer to plugin instance (this).
         * @param[in] time Time of the pulse that we're sending data for.
         * @param[in] charge Charge of the pulse that we're sending data for.
         * @param[in] sequenceId Sequence ID of the posting.
         */
        typedef void (*PostDataCb)(BasePvaPlugin *, const epicsTimeStamp &time, double charge, uint32_t sequenceId);

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
         * Function filters out non-neutron packets and packets without valid RTDL
         * header. It ensures all packet accounting. It delegates processing of
         * packet to external function provided through callbacks. When new pulse
         * is detected or when running out of data, a callback to post data is called.
         */
        void processData(const DasPacketList * const packetList);

        /**
         * Set callback functions for processing packet and posting data.
         *
         * Static C functions were selected for performance reasons. At
         * least packet processing callback is invoked quite often and other
         * implementations (virtual function, std::function are all slower).
         *
         * @param[in] procCb Packet processing function
         * @param[in] postCb Function to be called when PV update should be done.
         */
        void setCallbacks(ProcessPacketCb procCb, PostDataCb postCb);

    protected:
        PvaNeutronData::shared_pointer m_pvRecord;

    private:
        uint32_t m_nReceived;       //!< Number of packets received
        uint32_t m_nProcessed;      //!< Number of data packets processed
        epicsTimeStamp m_pulseTime; //!< Current pulse EPICS timestamp
        double m_pulseCharge;       //!< Current pulse charge
        uint32_t m_postSeq;         //!< Current post sequence id
        ProcessPacketCb m_processPacketCb;  //!< Callback function ptr to process packet data
        PostDataCb m_postDataCb;    //!< Callback function ptr to post data collected so far

    private: // asyn parameters
        #define FIRST_BASEPVAPLUGIN_PARAM 0
        #define LAST_BASEPVAPLUGIN_PARAM 0
};

#endif // BASE_PVA_PLUGIN_H
