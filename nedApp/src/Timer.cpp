/* Timer.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Timer.h"

Timer::Timer(bool shared)
    : m_queue(epicsTimerQueueActive::allocate(shared))
    , m_timer(m_queue.createTimer())
{
}

Timer::~Timer()
{
    m_timer.destroy();
}

bool Timer::schedule(std::function<float()> &callback, float delay)
{
    if (isActive())
        return false;
    m_callback = callback;
    m_timer.start(*this, delay);
    return true;
}

epicsTimerNotify::expireStatus Timer::expire(const epicsTime & currentTime)
{
    if (m_callback) {
        float delay = m_callback();
        if (delay > 0)
            return expireStatus(restart, delay);
    }
    return expireStatus(noRestart);
}

bool Timer::cancel()
{
    bool running = (m_timer.getExpireDelay() != DBL_MAX);
    m_timer.cancel();
    m_callback = std::function<float()>();
    return running;
}

bool Timer::isActive()
{
    epicsTimer::expireInfo info = m_timer.getExpireInfo();
    return info.active;
}
