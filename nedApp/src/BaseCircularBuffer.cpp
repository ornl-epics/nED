/* CircularBuffer.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseCircularBuffer.h"

#include <stdio.h>

BaseCircularBuffer::BaseCircularBuffer()
    : m_nPushed(0)
    , m_nRead(0)
{
    epicsTimeGetCurrent(&m_pushRateTime);
    epicsTimeGetCurrent(&m_readRateTime);
}

uint32_t BaseCircularBuffer::push(void *data, uint32_t len)
{
    m_rateLock.lock();
    m_nPushed += len;
    m_rateLock.unlock();

	return 0;
}

int BaseCircularBuffer::consume(uint32_t len)
{
    m_rateLock.lock();
    m_nRead += len;
    m_rateLock.unlock();

	return 0;
}

uint32_t BaseCircularBuffer::getPushRate()
{
    epicsTimeStamp now, past;
    double diff;
    uint32_t nBytes;

    epicsTimeGetCurrent(&now);

    m_rateLock.lock();
    nBytes = m_nPushed;
    m_nPushed = 0;
    past = m_pushRateTime;
    m_pushRateTime = now;
    m_rateLock.unlock();

    diff = epicsTimeDiffInSeconds(&now, &past);
    if (diff == 0.0)
        diff = 0.000001; // Insignificant time, but not 0.0
    return (uint32_t)(nBytes / diff);
}

uint32_t BaseCircularBuffer::getReadRate()
{
    epicsTimeStamp now, past;
    double diff;
    uint32_t nBytes;

    epicsTimeGetCurrent(&now);

    m_rateLock.lock();
    nBytes = m_nRead;
    m_nRead = 0;
    past = m_readRateTime;
    m_readRateTime = now;
    m_rateLock.unlock();

    diff = epicsTimeDiffInSeconds(&now, &past);
    if (diff == 0.0)
        diff = 0.000001; // Insignificant time, but not 0.0
    return (uint32_t)(nBytes / diff);
}

