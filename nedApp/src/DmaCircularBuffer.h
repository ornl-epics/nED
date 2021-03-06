/* DmaCircularBuffer.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DMA_CIRCULAR_BUFFER_H
#define DMA_CIRCULAR_BUFFER_H

#include "BaseCircularBuffer.h"

struct occ_handle;

/**
 * A DMA circular buffer class.
 *
 * The DmaCircularBuffer works directly with the DMA buffer populated by the OCC
 * board. It does not make any copy of the data whatsoever. See occlib.h for
 * details.
 * There's no push() function implemented, instead OCC board pushes data to the
 * buffer directly. Push rate is always 0.
 */
class DmaCircularBuffer : public BaseCircularBuffer {
    public:
        /**
         * Create DMA circular buffer.
         */
        DmaCircularBuffer(struct occ_handle *occ);

        /**
         * Destroy DMA circular buffer.
         */
        ~DmaCircularBuffer();

        /**
         * Wait until some data is available in circular buffer.
         *
         * The function waits for OCC board to signal it has some data.
         * On success it updates the len argument with the number of bytes of
         * data available.
         *
         * @param[out] data Pointer to the start of the data.
         * @param[out] len On success, updated to the number of bytes of data.
         * @param[in] timeout Number of seconds to wait for data, 0 wait forever
         * @return Matches return code of occ_data_wait()
         */
        int wait(void **data, uint32_t *len, double timeout=0.0);

        /**
         * Advance consumer index.
         *
         * Instruct OCC board to advance consumer index.
         *
         * @return Matches return code of occ_data_ack()
         */
        int consume(uint32_t len);

        /**
         * Return true when no data is available in circular buffer.
         */
        bool empty();

        /**
         * Return buffer used space in bytes.
         */
        virtual uint32_t used() { return 0; };

        /**
         * Return buffer size in bytes.
         */
        virtual uint32_t size() { return 0; };

    private:
        struct occ_handle *m_occ;       //!< Pre-initialized OCC handle
};

#endif // DMA_CIRCULAR_BUFFER_H
