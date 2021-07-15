/* PluginMessage.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "PluginMessage.h"

PluginMessage::PluginMessage(const void *msg)
    : m_refcount(0)
    , m_msg(msg)
{}

PluginMessage::~PluginMessage()
{
    m_lock.lock();
    unsigned long refcount = m_refcount;
    m_lock.unlock();
    assert(refcount == 0);
}

void PluginMessage::claim()
{
    m_lock.lock();
    m_refcount++;
    m_lock.unlock();
}

void PluginMessage::release()
{
    m_lock.lock();
    if (m_refcount > 0)
        m_refcount--;
    if (m_refcount == 0)
        m_event.signal();
    m_lock.unlock();
}


void PluginMessage::waitAllReleased()
{
    while (!released())
        m_event.wait();
}

bool PluginMessage::released()
{
    bool released;

    m_lock.lock();
    released = (m_refcount == 0);
    m_lock.unlock();

    return released;
}
