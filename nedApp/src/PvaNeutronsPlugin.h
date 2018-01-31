/* PvaNeutronsPlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_NEUTRONS_H
#define PVA_NEUTRONS_H

#include "BasePlugin.h"

#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

#include <memory>

/**
 * Gather and present statistical information of the incoming data
 */
class PvaNeutronsPlugin : public BasePlugin {
    public: // functions

        enum {
            STATUS_READY        = 0,
            STATUS_BAD_CONFIG   = 1,
            STATUS_INIT_ERROR   = 2,
            STATUS_SEND_ERROR   = 3,
        };

        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvName name of a PVA record used to export RTDL data
         */
        PvaNeutronsPlugin(const char *portName, const char *parentPlugins, const char *pvName);

        /**
         * Process downstream data packets
         */
        void recvDownstream(const DasDataPacketList &packets);

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
                bool update(const DasDataPacket *packet);

            private:
                uint32_t m_sequence;
                epics::pvData::PVTimeStamp      pvTimeStamp;    //!< Time stamp common to all events
                epics::pvData::PVBooleanPtr     pvLogical;      //!< Flags whether pixels are mapped to logical ids
                epics::pvData::PVUIntPtr        pvNumEvents;    //!< Number of events in tof,pixelid arrays
                epics::pvData::PVUIntArrayPtr   pvTimeOfFlight; //!< Time offset relative to time stamp
                epics::pvData::PVUIntArrayPtr   pvPixel;        //!< Pixel ID

                /**
                 * C'tor.
                 */
                PvaRecord(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure);
        };

        PvaRecord::shared_pointer m_record;

    private: // asyn parameters
        int Status; // See PvaNeutronsPlugin::STATUS_*
        int PvaName; // PV name
};

#endif // PVA_NEUTRONS_H
