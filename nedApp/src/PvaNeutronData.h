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
 * Structure extends EPICSv4 PVrecord which means it's responsible for sending
 * data to the clients.
 */
class PvaNeutronData : public epics::pvDatabase::PVRecord {
    public: // Pointers in to the PV records' data structure
        epics::pvData::PVTimeStamp    timeStamp;        //!< Pulses' absolute EPICS timestamp, in sec.nsec since January 1, 1990
        epics::pvData::PVDoublePtr    proton_charge;    //!< Pulse proton charge in Coulombs
        epics::pvData::PVUIntArrayPtr time_of_flight;   //!< Time of flight offest from pulse start
        epics::pvData::PVUIntArrayPtr pixel;            //!< Pixel ID
        epics::pvData::PVUIntArrayPtr sample_a1;        //!< LPSD ONLY: ADC samples, or AROC ADC sample A1
        epics::pvData::PVUIntArrayPtr sample_a2;        //!< CROC ONLY: TimeRange samples, or AROC ADC sample A2
        epics::pvData::PVUIntArrayPtr sample_a3;        //!< AROC ADC sample A3
        epics::pvData::PVUIntArrayPtr sample_a4;        //!< AROC ADC sample A4
        epics::pvData::PVUIntArrayPtr sample_a5;        //!< AROC ADC sample A5
        epics::pvData::PVUIntArrayPtr sample_a6;        //!< AROC ADC sample A6
        epics::pvData::PVUIntArrayPtr sample_a7;        //!< AROC ADC sample A7
        epics::pvData::PVUIntArrayPtr sample_a8;        //!< AROC ADC sample A8
        epics::pvData::PVUShortArrayPtr sample_x1;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x2;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x3;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x4;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x5;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x6;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x7;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x8;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x9;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x10;     //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x11;     //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x12;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x13;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x14;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x15;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x16;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x17;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x18;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x19;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_x20;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y1;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y2;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y3;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y4;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y5;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y6;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y7;      //!< BNLROC & CROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y8;      //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y9;      //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y10;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y11;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y12;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y13;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y14;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y15;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y16;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_y17;     //!< BNLROC: ADC samples
        epics::pvData::PVUShortArrayPtr sample_g1;      //!< CROC: photon count in g1
        epics::pvData::PVUShortArrayPtr sample_g2;      //!< CROC: photon count in g2
        epics::pvData::PVUShortArrayPtr sample_g3;      //!< CROC: photon count in g3
        epics::pvData::PVUShortArrayPtr sample_g4;      //!< CROC: photon count in g4
        epics::pvData::PVUShortArrayPtr sample_g5;      //!< CROC: photon count in g5
        epics::pvData::PVUShortArrayPtr sample_g6;      //!< CROC: photon count in g6
        epics::pvData::PVUShortArrayPtr sample_g7;      //!< CROC: photon count in g7
        epics::pvData::PVUShortArrayPtr sample_g8;      //!< CROC: photon count in g8
        epics::pvData::PVUShortArrayPtr sample_g9;      //!< CROC: photon count in g9
        epics::pvData::PVUShortArrayPtr sample_g10;     //!< CROC: photon count in g10
        epics::pvData::PVUShortArrayPtr sample_g11;     //!< CROC: photon count in g11
        epics::pvData::PVUShortArrayPtr sample_g12;     //!< CROC: photon count in g12
        epics::pvData::PVUShortArrayPtr sample_g13;     //!< CROC: photon count in g13
        epics::pvData::PVUShortArrayPtr sample_g14;     //!< CROC: photon count in g14
        epics::pvData::PVUIntArrayPtr sample_a48;       //!< ACPC ONLY: ADC samples
        epics::pvData::PVUIntArrayPtr sample_b1;        //!< LPSD ONLY: ADC samples, or AROC ADC sample B1
        epics::pvData::PVUIntArrayPtr sample_b2;        //!< AROC ADC sample B2
        epics::pvData::PVUIntArrayPtr sample_b3;        //!< AROC ADC sample B3
        epics::pvData::PVUIntArrayPtr sample_b4;        //!< AROC ADC sample B4
        epics::pvData::PVUIntArrayPtr sample_b5;        //!< AROC ADC sample B5
        epics::pvData::PVUIntArrayPtr sample_b6;        //!< AROC ADC sample B6
        epics::pvData::PVUIntArrayPtr sample_b7;        //!< AROC ADC sample B7
        epics::pvData::PVUIntArrayPtr sample_b8;        //!< AROC ADC sample B8
        epics::pvData::PVUIntArrayPtr sample_b12;       //!< AROC ONLY: Diagnostic values
        epics::pvData::PVUIntArrayPtr position_index;   //!< Position index mapping
        epics::pvData::PVFloatArrayPtr position_x;      //!< Position X
        epics::pvData::PVFloatArrayPtr position_y;      //!< Position Y
        epics::pvData::PVFloatArrayPtr photo_sum_x;     //!< Photo sum X
        epics::pvData::PVFloatArrayPtr photo_sum_y;     //!< Photo sum Y
        epics::pvData::PVUShortArrayPtr time_range1;    //!< CROC ONLY: photon counts in time bin 1
        epics::pvData::PVUShortArrayPtr time_range2;    //!< CROC ONLY: photon counts in time bin 2
        epics::pvData::PVUShortArrayPtr time_range3;    //!< CROC ONLY: photon counts in time bin 3
        epics::pvData::PVUShortArrayPtr time_range4;    //!< CROC ONLY: photon counts in time bin 4

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
