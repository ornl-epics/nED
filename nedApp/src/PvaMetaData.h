/* PvaMetaData.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_META_DATA_H
#define PVA_META_DATA_H

#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

/**
 * EPICSv4 structure for meta data.
 *
 * Structure extends EPICSv4 PVrecord which means it's responsible for sending
 * data to the clients.
 */
class PvaMetaData : public epics::pvDatabase::PVRecord {
    public: // Pointers in to the PV records' data structure
        epics::pvData::PVTimeStamp    timeStamp;        //!< Pulses' absolute EPICS timestamp, in sec.nsec since January 1, 1990
        epics::pvData::PVDoublePtr    proton_charge;    //!< Pulse proton charge in Coulombs
        epics::pvData::PVUIntArrayPtr time_of_flight;   //!< Time of flight offest from pulse start
        epics::pvData::PVUIntArrayPtr pixel;            //!< Pixel ID

    public:
        POINTER_DEFINITIONS(PvaMetaData);

        /**
         * Create an instance of PvaMetaData::shared_pointer object.
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
        PvaMetaData(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure);

    private:
        bool init();
};

#endif // PVA_META_DATA_H
