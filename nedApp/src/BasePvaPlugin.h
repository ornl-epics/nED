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
 * The base plugin also ensures a common output mode switching interface for
 * all specialized plugins.
 * Specialization plugins only need to implement one or many OCC data processing
 * functions to extract detector specific data and push it to m_pvRecord object.
 * There's no field in DasPacket that describes the payload. We rely on external
 * party to flag current mode through Mode parameter. Based on that appropriate
 * processData*() function is invoked.
 */
class BasePvaPlugin : public BasePlugin {
    public:
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
         * Specialized plugins are advised to use one of more specific
         * functions.
         */
        void processData(const DasPacketList * const packetList);

        /**
         * Process incoming packet as normal detector data.
         */
        virtual void processNormalPacket(const DasPacket * const packet) {};

        /**
         * Process incoming packet as raw detector data.
         */
        virtual void processRawPacket(const DasPacket * const packet) {};

        /**
         * Process incoming packet as extended detector data.
         */
        virtual void processExtendedPacket(const DasPacket * const packet) {};

        /**
         * Post normal data.
         *
         * New pulse was detected based on RTDL timestamp and the data for
         * previous pulse collected so far should be posted.
         *
         * @param[in] pulseTime Previous pulse timestamp for which data should be posted.
         * @param[in] pulseCharge Previous pulse charge.
         * @param[in] postSeq Id of the post, monotonically increases for every post
         */
        virtual void postNormalData(const epicsTimeStamp &pulseTime, uint32_t pulseCharge, uint32_t postSeq) {};

        /**
         * Post raw data.
         *
         * New pulse was detected based on RTDL timestamp and the data for
         * previous pulse collected so far should be posted.
         *
         * @param[in] pulseTime Previous pulse timestamp for which data should be posted.
         * @param[in] pulseCharge Previous pulse charge.
         * @param[in] postSeq Id of the post, monotonically increases for every post
         */
        virtual void postRawData(const epicsTimeStamp &pulseTime, uint32_t pulseCharge, uint32_t postSeq) {};

        /**
         * Post extended data.
         *
         * New pulse was detected based on RTDL timestamp and the data for
         * previous pulse collected so far should be posted.
         *
         * @param[in] pulseTime Previous pulse timestamp for which data should be posted.
         * @param[in] pulseCharge Previous pulse charge.
         * @param[in] postSeq Id of the post, monotonically increases for every post
         */
        virtual void postExtendedData(const epicsTimeStamp &pulseTime, uint32_t pulseCharge, uint32_t postSeq) {};

    protected:
        PvaNeutronData::shared_pointer m_pvRecord;

    private: // variables
        uint32_t m_userTagCounter;
        uint32_t m_nReceived;
        uint32_t m_nProcessed;
        epicsTimeStamp m_currentPulseTime;
        uint32_t m_currentPulseCharge;

    private: // functions
        void postData(const epicsTimeStamp &pulseTime, uint32_t pulseCharge);

    private: // asyn parameters
        #define FIRST_BASEPVAPLUGIN_PARAM 0
        #define LAST_BASEPVAPLUGIN_PARAM 0
};

#endif // BASE_PVA_PLUGIN_H
