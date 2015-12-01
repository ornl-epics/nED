/* PvaCalcVerify.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_CALC_VERIFY_H
#define PVA_CALC_VERIFY_H

#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

/**
 * An extended PVRecord for exporting firmware and software calculated data.
 *
 * Used for detectors extended mode when firmware and software are both
 * calculating the event for verification purposes. Data is exported using this
 * PV structure.
 *
 * This smarter PVRecord contains caching for all structure elements until
 * they are sent to client. Sending to client is atomic. Provided are functions
 * to create and destroy the object. For the entire object lifetime, object is
 * registered with PVA server automatically.
 */
class PvaCalcVerifyData : public epics::pvDatabase::PVRecord {
    public: // Cache values until posted
        epics::pvData::PVUIntArray::svector time_of_flight; //!< Cached time_of_flight values
        epics::pvData::PVDoubleArray::svector firmware_x;   //!< Cached firmware_x values
        epics::pvData::PVDoubleArray::svector firmware_y;   //!< Cached firmware_y values
        epics::pvData::PVDoubleArray::svector software_x;   //!< Cached software_x values
        epics::pvData::PVDoubleArray::svector software_y;   //!< Cached software_y values

    private: // Pointers in to the PV records' data structure
        epics::pvData::PVUIntArrayPtr time_of_flightPtr;
        epics::pvData::PVDoubleArrayPtr firmware_xPtr;
        epics::pvData::PVDoubleArrayPtr firmware_yPtr;
        epics::pvData::PVDoubleArrayPtr software_xPtr;
        epics::pvData::PVDoubleArrayPtr software_yPtr;
        uint32_t m_cacheSize;

    public:
        POINTER_DEFINITIONS(PvaCalcVerifyData);

        /**
         * Create PV record and register it with PVA server.
         *
         * A PV record is created and all internal structures are initialized.
         * If successful, record is registered with PVA server. A shared pointer
         * to the record is returned which is what PVA API expect.
         *
         * @param[in] recordName Record full name
         * @param[in] cacheSize Expected upper number of events before posting, used to optimize cache containers.
         * @return Shared pointer to the new object or invalid shared pointer.
         */
        static shared_pointer create(const std::string &recordName, uint32_t cacheSize=32768);

        /**
         * Unregister PV record from PVA server and destroy it.
         *
         * @param[in] record to be destroyed
         * @return true on success
         */
        static bool destroy(shared_pointer &record);

        /**
         * Post cached values as a an atomic PV record update.
         *
         * Cache is flushed after posting.
         *
         * @return true if update was posted, false if no values to post or due to an error.
         */
        bool post();

    protected:
        /**
         * Private constructor.
         *
         * To contain the PV structure and access function all within the same
         * class, there's a helper create() function and the constructor is
         * made private.
         */
        PvaCalcVerifyData(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure, uint32_t cacheSize=32768);

    private:
        bool init();
};

#endif // PVA_CALC_VERIFY_H
