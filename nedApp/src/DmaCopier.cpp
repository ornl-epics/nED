/* DmaCopier.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DmaCopier.h"

#include <epicsThread.h>
#include <errno.h>
#include <occlib.h>

#define THREAD_INTERVAL           0.1   // Thread resolution time to exit in seconds

DmaCopier::DmaCopier(struct occ_handle *occ, uint32_t bufferSize)
    : CircularBuffer(bufferSize)
    , Thread("DmaCopier", std::bind(&DmaCopier::copyWorker, this, std::placeholders::_1), epicsThreadGetStackSize(epicsThreadStackBig), epicsThreadPriorityHigh)
    , m_occ(occ)
{
    Thread::start();
}

void DmaCopier::copyWorker(epicsEvent *shutdown)
{
    while (shutdown->tryWait() == false) {
        void *data;
        size_t len;
        int status;


        status = occ_data_wait(m_occ, &data, &len, THREAD_INTERVAL*1000);
        if (status == -ETIME || (len == 0 && status == 0))
            continue;
        if (status != 0) {
            wakeUpConsumer(status);
            break;
        }

        // Successful push() will wake up consumer
        len = CircularBuffer::push(data, len);
        if (len == 0) {
            wakeUpConsumer(-ENODATA);
            break;
        }

        status = occ_data_ack(m_occ, len);
        if (status != 0) {
            wakeUpConsumer(status);
            break;
        }
    }
}
