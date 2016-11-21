/* DasPacketList.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DasPacketList.h"

#include <epicsAlgorithm.h>

DasPacketList::DasPacketList()
    : m_refcount(0)
{
    //ctor
}

const DasPacket *DasPacketList::verifyPacket(const uint8_t *data, uint32_t dataLen) const
{
    const DasPacket *packet = reinterpret_cast<const DasPacket *>(data);

    if (dataLen < DasPacket::getMinLength())
        return 0;

    if (packet->isValid() == false)
        return 0;

    if (packet->getLength() > dataLen)
        return 0;

    return packet;
}

void DasPacketList::reserve()
{
    m_lock.lock();
    m_refcount++;
    m_lock.unlock();
}

void DasPacketList::release()
{
    m_lock.lock();
    if (m_refcount > 0)
        m_refcount--;
    if (m_refcount == 0)
        m_event.signal();
    m_lock.unlock();
}

uint32_t DasPacketList::reset(const uint8_t *data, uint32_t dataLen)
{
    uint32_t consumed = 0;

    m_lock.lock();
    if (m_refcount == 0) {

        clear();

        const DasPacket *packet = verifyPacket(data, dataLen);
        while (packet) {
            // Calculate length only once
            uint32_t packetLen = packet->getLength();

            if (packetLen > dataLen || packetLen == 0)
                break;

            dataLen  -= packetLen;
            data     += packetLen;
            consumed += packetLen;

            push_back(packet);
            packet = verifyPacket(data, dataLen);
        }

        m_refcount = 1;
    }
    m_lock.unlock();

    return consumed;
}

bool DasPacketList::reset(const DasPacket * const packet)
{
    if (reset(reinterpret_cast<const uint8_t *>(packet), packet->getLength()) != packet->getLength()) {
        m_lock.lock();
        clear();
        m_refcount = 0;
        m_lock.unlock();
        return false;
    }

    return true;
}

void DasPacketList::reset(const DasPacketList *original)
{
    m_lock.lock();
    assign(original->begin(), original->end());
    m_refcount = 1;
    m_lock.unlock();
}

void DasPacketList::waitAllReleased() const
{
    while (!released())
        m_event.wait();
}

bool DasPacketList::released() const
{
    bool released;

    m_lock.lock();
    released = (m_refcount == 0);
    m_lock.unlock();

    return released;
}
