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

        if (full()) {
            // There's no space in circular buffer, give consumers some extra
            // time and retry. Hopefully the OCC can host data for a while.
            // Otherwise a stall condition will occur which is detected in
            // OccPortDriver.
            wakeUpConsumer(0);
            epicsThreadSleep(THREAD_INTERVAL);
            continue;
        }

        status = occ_data_wait(m_occ, &data, &len, THREAD_INTERVAL*1000);
        if (status == -ETIME || len == 0)
            continue;
        if (status != 0) {
            wakeUpConsumer(status);
            break;
        }

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
