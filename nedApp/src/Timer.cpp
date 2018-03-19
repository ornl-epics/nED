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
    , m_active(false)
{
}

Timer::~Timer()
{
    m_timer.destroy();
}

bool Timer::schedule(std::function<float()> &callback, float delay)
{
    if (m_timer.getExpireDelay() == DBL_MAX)
        return false;
    m_callback = callback;
    m_timer.start(*this, delay);
    m_active = true;
    return true;
}

epicsTimerNotify::expireStatus Timer::expire(const epicsTime & currentTime)
{
    float delay = m_callback();
    if (delay > 0) {
        return expireStatus(restart, delay);
    } else {
        cancel();
        return expireStatus(noRestart);
    }
}

bool Timer::cancel()
{
    if (m_active) {
        m_callback = std::function<float()>();
        m_active = false;
        return true;
    }
    return false;
}

bool Timer::isActive()
{
    return m_active;
}
