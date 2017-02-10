/* PvaXyPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_XY_PLUGIN_H
#define PVA_XY_PLUGIN_H

#include "BasePlugin.h"

#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

/**
 * Plugin for exporting neutron data in X,Y,PhotoSum format through PVA.
 *
 * Based on data format announced in packet header, X,Y and X,Y,PS formats are
 * processed and pushed into the PVA channel.
 */
class PvaXyPlugin : public BasePlugin {
    private:
        /**
         * Extended PVRecord containing X,Y,PS fields
         */
        struct PvaRecord : public epics::pvDatabase::PVRecord {
            epics::pvData::PVTimeStamp     timeStamp;       //!< Pulses' absolute EPICS timestamp, in sec.nsec since January 1, 1990
            epics::pvData::PVUIntArrayPtr  time_of_flight;  //!< Time of flight offest from pulse start
            epics::pvData::PVUIntArrayPtr  position_index;  //!< Position index mapping
            epics::pvData::PVFloatArrayPtr x;               //!< Position X
            epics::pvData::PVFloatArrayPtr y;               //!< Position Y
            epics::pvData::PVFloatArrayPtr photo_sum_x;     //!< Photo sum X
            epics::pvData::PVFloatArrayPtr photo_sum_y;     //!< Photo sum Y
            // Following is needed for inherited members to work correctly.
            PvaRecord(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure) {}
            friend PvaXyPlugin;
            POINTER_DEFINITIONS(PvaRecord);
        };

        /**
         * A cache to store data until it's posted.
         */
        struct {
            epics::pvData::PVUIntArray::svector time_of_flight;
            epics::pvData::PVUIntArray::svector position_index;
            epics::pvData::PVFloatArray::svector x;
            epics::pvData::PVFloatArray::svector y;
            epics::pvData::PVFloatArray::svector photo_sum_x;
            epics::pvData::PVFloatArray::svector photo_sum_y;
        } m_cache;

    public: // functions
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvPrefix Prefix for the PV Record
         */
        PvaXyPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

        /**
         * Overloaded function to handle DataMode parameter.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process all incoming packets.
         */
        void processData(const DasPacketList * const packetList);

        /**
         * Process data buffer as X,Y events.
         */
        void processXyData(const uint32_t *data, uint32_t count);

        /**
         * Process data buffer as X,Y,PhotoSum events.
         */
        void processXyPsData(const uint32_t *data, uint32_t count);

        /**
         * Post data received so far and clear cache
         *
         * Cached data is posted as a single update to EPICSv4 PV.
         * Caller must ensure plugin is locked while calling this function.
         *
         * @param[in] pvRecord to be updated
         */
        void postData(const PvaRecord::shared_pointer& pvRecord);

    private:

        /**
         * PVA channel record.
         */
        PvaRecord::shared_pointer m_pvRecord;

        /**
         * Number of elements in each cache array.
         */
        static const uint32_t CACHE_SIZE;

        uint32_t m_xyDivider; // X,Y is in UQm.n format, divider is (1 << n)
        uint32_t m_photosumDivider; // Photo Sum is in UQm.n format, divider is (1 << n)

        /**
         * Create and initialize PVA channel record.
         */
        PvaRecord::shared_pointer createPvRecord(const char *channelName);
    protected:
        #define FIRST_PVAXYPLUGIN_PARAM XyFractWidth
        int XyFractWidth;
        int PsFractWidth;
        #define LAST_PVAXYPLUGIN_PARAM PsFractWidth
};

#endif // PVA_XY_PLUGIN_H
