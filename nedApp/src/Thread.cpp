/* Thread.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include <Thread.h>

Thread::Thread(const char *name, std::function<void(epicsEvent *)> worker, unsigned int stackSize, unsigned int priority)
    : m_thread(*this, name, stackSize, priority)
    , m_running(false)
    , m_worker(worker)
{
    m_thread.start();
}

Thread::~Thread()
{
    // Make sure to stop worker function cleanly, EPICS will kill thread
    stop();
}

void Thread::start()
{
    m_mutex.lock();
    if (m_running == false)
        m_resume.signal();
    m_mutex.unlock();
}

void Thread::stop()
{
    m_mutex.lock();
    bool running = m_running;
    m_mutex.unlock();
        
    if (running == true) {
        m_pause.signal();

        do {
            m_paused.wait(0.5);
            
            m_mutex.lock();
            running = m_running;
            m_mutex.unlock();
        } while (running == true);

        // Clear our event in case thread exited on its own and didn't process signal
        m_pause.tryWait();
    } else {
        // Did the thread exit on its own?
        m_paused.tryWait();
    }
}

void Thread::run()
{
    while (true) {
        if (m_resume.wait(1.0) == true) {
            m_mutex.lock();
            m_running = true;
            m_mutex.unlock();

            if (m_worker)
                m_worker(&m_pause);

            m_mutex.lock();
            m_running = false;
            m_mutex.unlock();

            m_paused.signal();
        }
    }
}
