/* PvaNeutronData.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_NEUTRON_DATA_H
#define PVA_NEUTRON_DATA_H

#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

/**
 * Common EPICSv4 structure for (almost) all detector data.
 *
 * The structure combines data from all detectors in normal, raw and extended
 * mode. Record gets updated by updating public members in single or group mode.
 *
 * Structure is also an EPICSv4 PVrecord which means it's responsible for sending
 * data to the clients. The PvaNeutronData tries to hide most dirty work away by
 * providing two modes of operation:
 *   - using cache and sending cache contents atomically
 *   - embedding variable updates inside transaction, when transaction is closed
 *     data gets send to clients
 */
class PvaNeutronData : public epics::pvDatabase::PVRecord {
    public: // Pointers in to the PV records' data structure
        epics::pvData::PVTimeStamp    timeStamp;        //!< Pulses' absolute EPICS timestamp, in sec.nsec since January 1, 1990
        epics::pvData::PVDoublePtr    proton_charge;    //!< Pulse proton charge
        epics::pvData::PVUIntArrayPtr time_of_flight;   //!< Time of flight offest from pulse start
        epics::pvData::PVUIntArrayPtr pixel;            //!< Pixel ID
        epics::pvData::PVUIntArrayPtr position_index;   //!< Position index mapping
        epics::pvData::PVUIntArrayPtr position_x;       //!< Position X
        epics::pvData::PVUIntArrayPtr position_y;       //!< Position Y
        epics::pvData::PVUIntArrayPtr photo_sum_x;      //!< Photo sum X
        epics::pvData::PVUIntArrayPtr photo_sum_y;      //!< Photo sum Y

    public:
        POINTER_DEFINITIONS(PvaNeutronData);

        /**
         * Create an instance of PvaNeutronData::shared_pointer object.
         *
         * The function first creates a pvStructure object and passes it to
         * private constructor to obtain object instance. It then initializes
         * the structure fields and creates links to the structure field for
         * faster access.
         *
         * @param[in] recordName Record full name
         * @return Shared pointer to the new object or invalid shared pointer.
         */
        static shared_pointer create(const std::string &recordName);

    protected:
        /**
         * Private constructor.
         *
         * To contain the PV structure and access function all within the same
         * class, there's a helper create() function and the constructor is
         * made private.
         */
        PvaNeutronData(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure);

    private:
        bool init();
};

#endif // PVA_NEUTRON_DATA_H
