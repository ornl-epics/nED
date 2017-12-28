/* RtdlPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef RTDL_PLUGIN_H
#define RTDL_PLUGIN_H

#include "BasePlugin.h"

#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/pvEnumerated.h>
#include <pv/pvDisplay.h>
#include <pv/standardPVField.h>

#include <memory>

/**
 * Gather and present statistical information of the incoming data
 */
class RtdlPlugin : public BasePlugin {
    public: // functions
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvName name of a PVA record used to export RTDL data
         */
        RtdlPlugin(const char *portName, const char *parentPlugins, const char *pvName);

        /**
         * Process downstream RTDL packets
         */
        void recvDownstream(DasRtdlPacketList *packets);

    private:

        /**
         * PVAccess PV record.
         */
        class PvaRecord : public epics::pvDatabase::PVRecord {
            public:
                POINTER_DEFINITIONS(PvaRecord);

                /**
                 * Allocate and initialize PvaRecord.
                 */
                static PvaRecord::shared_pointer create(const std::string &recordName);

                /**
                 * Attach all PV structures.
                 */
                bool init();

                /**
                 * Publish a single atomic update of the PV, take values from packet.
                 */
                bool update(DasRtdlPacket *packet);

            private:
                uint32_t m_sequence;
                epics::pvData::PVTimeStamp      pvTimeStamp;
                epics::pvData::PVDoublePtr      pvProtonCharge; //!< Pulse proton charge in Coulombs
                epics::pvData::PVDisplay        pvProtonChargeDisplay;
                epics::pvData::PVBooleanPtr     pvBadPulse;
                epics::pvData::PVEnumerated     pvPulseFlavor;
                epics::pvData::PVBooleanPtr     pvBadVetoFrame;
                epics::pvData::PVBooleanPtr     pvBadCycleFrame;
                epics::pvData::PVUBytePtr       pvTstat;
                epics::pvData::PVUShortPtr      pvLastCycleVeto;
                epics::pvData::PVUShortPtr      pvCycle;
                epics::pvData::PVUIntPtr        pvTsyncPeriod;
                epics::pvData::PVDisplay        pvTsyncPeriodDisplay;
                epics::pvData::PVUIntPtr        pvTofOffset;
                epics::pvData::PVDisplay        pvTofOffsetDisplay;
                epics::pvData::PVUIntPtr        pvFrameOffset;
                epics::pvData::PVBooleanPtr     pvOffsetEnabled;
                epics::pvData::PVUIntArrayPtr   pvFrames;

                /**
                 * C'tor.
                 */
                PvaRecord(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure);
        };

        epicsTime m_lastRtdlTime;  //!< Time from last processed RTDL packet
        PvaRecord::shared_pointer m_record;

    private: // asyn parameters
        int Timestamp;
        int BadPulse;
        int PulseFlavor;
        int PulseCharge;
        int BadVetoFrame;
        int BadCycleFrame;
        int Tstat;
        int PrevCycleVeto;
        int Cycle;
        int TsyncPeriod;
        int TofFullOffset;
        int FrameOffset;
        int TofFixOffset;
        int RingPeriod;
        int ErrorsFutureTime;
        int ErrorsPastTime;
        int PvaName;
};

#endif // RTDL_PLUGIN_H
