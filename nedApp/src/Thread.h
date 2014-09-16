#ifndef NED_THREAD_H
#define NED_THREAD_H

#include <epicsThread.h>
#include <functional>

/**
 * Stoppable thread object
 *
 * The is an EPICS thread abstraction to provide stoppable and startable threading
 * mechanism. This is accomplished with always running thread which invokes
 * user specified function when started. When stopped, user function is
 * instructed to exit and when it does, the thread goes into waiting state until
 * started again.
 */
class Thread : public epicsThreadRunable {
    public:
        /**
         * Create thread object and let it run. Don't execute worker
         * function until started.
         *
         * @param[in] name Thread name
         * @param[in] worker User provided worker function which must stop when signaled
         * @param[in] stackSize Passed to epicsThread
         * @param[in] priority Passed to epicsThread
         */
        Thread(const char *name,
               std::function<void(epicsEvent *)> worker,
               unsigned int stackSize=epicsThreadGetStackSize(epicsThreadStackMedium),
               unsigned int priority=epicsThreadPriorityMedium);

        /**
         * Stop user function and destroy thread.
         */
        ~Thread();

        /**
         * Start thread for the first time or resume previously stopped thread.
         */
        void start();

        /**
         * Send worker function signal to shutdown and wait it returns.
         */
        void stop();

    private:
        epicsThread m_thread;   //!< Thread object
        bool m_running;         //!< Flag when the user function is running
        epicsMutex m_mutex;     //!< Lock for m_running member
        epicsEvent m_resume;    //!< Event that triggers starting the thread again
        epicsEvent m_pause;     //!< Event which user function receives
        epicsEvent m_paused;    //!< Event that tells when the user function exited
        std::function<void(epicsEvent *)> m_worker; //!< User function

        /**
         * Thread main function, encapsulates user worker function.
         */
        void run();
};

#endif // NED_THREAD_H
