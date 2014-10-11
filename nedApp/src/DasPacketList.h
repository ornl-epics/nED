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

/**
 * List of DAS Packets with reference counter
 *
 * This list points to the memory block with DAS packets. The memory block
 * length needs not be aligned with the packet. This class ensures that
 * all DAS packets returned by it are contiguous and complete. The last block
 * in the memory block might be partial DAS packet and will not be considered
 * by this class.
 *
 * The reference count keeps data pointed to by this object valid until
 * reference count drops to 0. At that point the data can be replaced with
 * new data but not earlier.
 */
class DasPacketList
{
    public:
        /**
         * Constructor initializes internal structures.
         */
        DasPacketList();

        /**
         * Return first DAS packet in the list.
         *
         * @return First DAS packet or 0 if none.
         */
        const DasPacket *first() const;

        /**
         * Return next DAS packet in the list.
         *
         * @param[in] current Any valid packet previously returned from first() or next().
         * @return Next DAS packet or 0 if none.
         */
        const DasPacket *next(const DasPacket *current) const;

        /**
         * Increase internal reference count.
         *
         * After making the reservation, data pointed to by this class
         * is guaranteed not to change until a release() is called.
         *
         * @return True if reservation was made, false if there's no data.
         */
        bool reserve();

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
         * @return true when object points to new data, false if reference count is not 0.
         */
        bool reset(const uint8_t *addr, uint32_t length);

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
        bool reset(const DasPacketList *original);

        /**
         * Wait for all consumers to release the object.
         *
         * After the function returns, reference counter is guaranteed to be 0 and
         * object can not be reserved() again until next reset() returns.
         */
        void waitAllReleased() const;

    private:
        uint8_t *m_address;
        uint32_t m_length;
        uint32_t m_consumed;

        unsigned long m_refcount;
        mutable epicsMutex m_lock;
        mutable epicsEvent m_event;

        /**
         * Verify that the data pointed to is legimite DAS packet.
         *
         * @return Return the same pointer or 0 on error.
         */
        const DasPacket *_verifyPacket(const DasPacket *current) const;

        /**
         * Return true if reference count is 0, false otherwise.
         */
        bool released() const;
};

#endif // DASPACKETLIST_HPP
