/* BaseCircularBuffer.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASE_CIRCULAR_BUFFER_H
#define BASE_CIRCULAR_BUFFER_H

#include <stdint.h>
#include <epicsMutex.h>
#include <epicsTime.h>

/**
 * An abstract circular buffer class.
 */
class BaseCircularBuffer {
    public:
        /**
         * Constructor initializes rate meters.
         */
        BaseCircularBuffer();

        /**
         * Virtual destructor for polymorphism to work.
         */
        virtual ~BaseCircularBuffer() {}

        /**
         * Remove data from buffer and reset indexes.
         */
        virtual void clear() { };

        /**
         * Copy data from memory area data to the circular buffer.
         *
         * Base impementation only manages rate counters. Derived classes need
         * to implement rest of the logic. But they must also called base
         * implementation to do the rate accounting, although it always return
         * 0 bytes.
         *
         * @param[in] data Pointer to the start of the source buffer.
         * @param[out] len Number of bytes available in data.
         * @return Number of bytes copied, always 0 in base implementation.
         */
        virtual uint32_t push(void *data, uint32_t len);

        /**
         * Wait until some data is available in circular buffer.
         *
         * This is a blocking function. After it returns, it updates data
         * pointer to the start of the memory where the data is. It also
         * modifies the len parameter to reflect the amount of data that
         * is available.
         *
         * @retval 0 on success
         * @retval negative on error, actual values are implementation specific
         */
        virtual int wait(void **data, uint32_t *len, double timeout=0.0) = 0;

        /**
         * Advance consumer index.
         *
         * Base impementation only manages rate counters. Derived classes need
         * to implement rest of the logic. But they must also called base
         * implementation to do the rate accounting, although it always return
         * 0 bytes.
         *
         * @retval 0 on success
         * @retval negative on error, actual values are implementation specific.
         */
        virtual int consume(uint32_t len);

        /**
         * Return true when no data is available in circular buffer.
         */
        virtual bool empty() = 0;

        /**
         * Return buffer used space in bytes.
         */
        virtual uint32_t used() = 0;

        /**
         * Return buffer size in bytes.
         */
        virtual uint32_t size() = 0;

        /**
         * Return average rate at which data is pushed to circular buffer.
         *
         * Internally keeps track of how many bytes were pushed. Derived
         * classes must call push() which accounts new data. Every time rate is
         * calculated, timer is reset. So the rate returned is the average rate
         * since last reading.
         *
         * @return Number of bytes/s pushed.
         */
        virtual uint32_t getPushRate();

        /**
         * Return average rate at which data is read out of circular buffer..
         *
         * Internally keeps track of how many bytes were read out. Derived
         * classes must call consume() which accounts new data. Every time rate is
         * calculated, timer is reset. So the rate returned is the average rate
         * since last reading.
         *
         *
         * @return Number of bytes/s read.
         */
        virtual uint32_t getReadRate();

    protected:
        uint32_t m_nPushed;         //!< Number of bytes pushed since last calculation
        uint32_t m_nRead;           //!< Number of bytes read since last calculation
        epicsMutex m_rateLock;      //!< Protecting rate members

    private:
        epicsTimeStamp m_pushRateTime; //!< Timestamp of last push rate calculation
        epicsTimeStamp m_readRateTime; //!< Timestamp of last read rate calculation
};

#endif // BASE_CIRCULAR_BUFFER_H
