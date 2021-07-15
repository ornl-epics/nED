/* DmaCopier.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DMA_COPIER_H
#define DMA_COPIER_H

#include "CircularBuffer.h"
#include "Thread.h"

struct occ_handle;

/**
 * Thread moving data from DMA buffer to circular buffer
 */
class DmaCopier : public CircularBuffer, public Thread {
    public:
        /**
         * Create thread and initialize circular buffer
         *
         * @param[in] occ handle to OCC device
         * @param[in] bufferSize size of circular buffer in bytes
         */
        DmaCopier(struct occ_handle *occ, uint32_t bufferSize);

    private:
        struct occ_handle *m_occ;

        /**
         * Worker function running in thread.
         */
        void copyWorker(epicsEvent *shutdown);
};

#endif // DMA_COPIER_H
