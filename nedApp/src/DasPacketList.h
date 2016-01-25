/* DasPacketList.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DASPACKETLIST_HPP
#define DASPACKETLIST_HPP

#include "DasPacket.h"

#include <epicsEvent.h>
#include <epicsMutex.h>
#include <vector>

/**
 * List of DAS Packets with reference counter
 *
 * The DasPacketList is a container of pointer to the DAS packets. Current
 * implementation uses std::vector but any standard container could be used.
 * Don't use std::vector indexed iterator (operator[]), possible future container
 * replacement.
 *
 * The reference count keeps data pointed to by this object valid until
 * reference count drops to 0. At that point the data can be replaced with
 * new data but not earlier.
 */
class DasPacketList : public std::vector<const DasPacket *>
{
    public:
        /**
         * Constructor initializes internal structures.
         */
        DasPacketList();

        /**
         * Increase internal reference count.
         *
         * After making the reservation, data pointed to by this class
         * is guaranteed not to change until a release() is called.
         *
         * Not to be confused with std::vector<>::reserve() function.
         */
        void reserve();

        /**
         * Decrease internal reference count and claim consumed data.
         *
         * Every call to reserve() must be followed by a call to this
         * function. In addition, reset() sets reference count to 1.
         *
         * After the client gets finished with processing DAS packets
         * described by this list, it should release the data by calling
         * this function.
         */
        void release();

        /**
         * Point list to new OCC data.
         *
         * The reference count must be 0 for this function to succeed.
         * The function should only be used by producers, check
         * the list friend classes.
         * This function will set the reference count to 1.
         *
         * @param[in] addr Pointer to the memory block where DAS packets data starts.
         * @param[in] length Memory block length in bytes.
         * @return Number of bytes parsed as valid packet data
         */
        uint32_t reset(const uint8_t *addr, uint32_t length);

        /**
         * Point list to a single packet.
         *
         * This is a convinience wrapper function for the reset(const uint8_t*, uint32_t)
         */
        bool reset(const DasPacket * const packet);

        /**
         * Point list to the same data as original list.
         *
         * This is a convinience wrapper function for the reset(const uint8_t*, uint32_t)
         */
        void reset(const DasPacketList *original);

        /**
         * Wait for all consumers to release the object.
         *
         * After the function returns, reference counter is guaranteed to be 0 and
         * object can not be reserved() again until next reset() returns.
         */
        void waitAllReleased() const;

    private:
        unsigned long m_refcount;
        mutable epicsMutex m_lock;
        mutable epicsEvent m_event;

        /**
         * Verify that the data pointed to is legimite DAS packet.
         *
         * @return Return the same pointer or 0 on error.
         */
        const DasPacket *verifyPacket(const uint8_t *data, uint32_t dataLen) const;

        /**
         * Return true if reference count is 0, false otherwise.
         */
        bool released() const;
};

#endif // DASPACKETLIST_HPP
