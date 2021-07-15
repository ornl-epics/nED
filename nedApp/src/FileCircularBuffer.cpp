/* FileCircularBuffer.cpp
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FileCircularBuffer.h"
#include "Packet.h"
#include "DasPacket.h"

#include <epicsThread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

bool FileCircularBuffer::open(const std::string &path, std::string &error)
{
    if (m_fd != -1)
        ::close(m_fd);

    m_fd = ::open(path.c_str(), O_LARGEFILE);
    if (m_fd == -1) {
        error = strerror(errno);
    }
    m_timeDiff = std::numeric_limits<double>::min();
    m_reading = false;
    return (m_fd != -1);
}

void FileCircularBuffer::close()
{
    if (m_fd != -1) {
        ::close(m_fd);
        m_fd = -1;
        m_offset = 0;
    }
}

void FileCircularBuffer::start()
{
    if (m_fd != -1) {
        m_reading = true;
        m_timeDiff = std::numeric_limits<double>::min();
        m_startTime = epicsTime::getCurrent();
    }
}

void FileCircularBuffer::stop()
{
    m_reading = false;
}

void FileCircularBuffer::reset()
{
    if (m_fd != -1) {
        ::lseek(m_fd, 0, SEEK_SET);
    }
    m_timeDiff = std::numeric_limits<double>::min();
    m_startTime = epicsTime::getCurrent();
}

void FileCircularBuffer::setSpeed(float speed)
{
    m_speed = speed;
    m_timeDiff = std::numeric_limits<double>::min();
    m_startTime = epicsTime::getCurrent();
}

int FileCircularBuffer::wait(void **data, uint32_t *len, double timeout)
{
    epicsTime start{epicsTime::getCurrent()};
    uint32_t count = m_maxPackets;

    while ((epicsTime::getCurrent() - start) < timeout) {

        if (m_fd != -1) {
            // Calculate the latest timestamp for any packet in this iteration
            epicsTimeStamp maxTimeStamp{0, 0};
            if (m_timeDiff == std::numeric_limits<double>::min()) {
                // Force reading first packet to get its timestamp and be able to
                // calculate time difference of first packet to current time
                maxTimeStamp = { std::numeric_limits<uint32_t>::max(), 0 };
                count = 1;
            } else {
                epicsTime now{epicsTime::getCurrent()};
                double scaledDiff = (now - m_startTime);
                scaledDiff *= m_speed;
                maxTimeStamp = m_startTime - m_timeDiff + scaledDiff;
            }

            while (m_reading && count-- > 0) {
                epicsTime maxTime{maxTimeStamp};
                if (readPacket(maxTime) == false) {
                    // Either no packet or time past requested
                    break;
                }
                if (m_timeDiff == std::numeric_limits<double>::min() && maxTime != epicsTimeStamp{0, 0}) {
                    // Calculate offset when data was taken to current time
                    m_timeDiff = epicsTime::getCurrent() - maxTime;
                    break;
                }
            }

            if (m_offset != 0) {
                *data = m_buffer.data();
                *len = m_offset;
                // Rate-limit update
                BaseCircularBuffer::push(*data, *len);
                return 0;
            }
        }

        epicsThreadSleep(0.01);
    }

    return -ETIME;
}

int FileCircularBuffer::consume(uint32_t len)
{
    len = std::min(len, m_offset);
    if (len < m_offset) {
        ::lseek(m_fd, -1*(m_offset - len), SEEK_CUR);
    }
    m_offset -= len;

    // Rate-limit update
    return BaseCircularBuffer::consume(len);;
}

epicsTimeStamp getPacketTimeStamp(const DasPacket *packet)
{
    return packet->getTimeStamp();
}
epicsTimeStamp getPacketTimeStamp(const Packet *packet)
{
    if (packet->getType() == Packet::TYPE_DAS_DATA) {
        try {
            return DasDataPacket::cast(packet)->getTimeStamp();
        } catch (...) {
            // pass
        }
    }
    return epicsTimeStamp{0, 0};
}

template <class T>
bool readHeader(int fd, uint8_t *buffer, uint32_t len, uint32_t &hdrLen, uint32_t &payloadLen) {
    if (fd == -1) {
        throw std::runtime_error("Failed not opened");
    }

    if (len < sizeof(T))
        return false;

    int ret = ::read(fd, buffer, sizeof(T));
    if (ret == -1) {
        throw std::runtime_error("Failed to read header");
    }
    if (ret != static_cast<int>(sizeof(T))) {
        ::lseek(fd, -1*ret, SEEK_CUR);
        return false;
    }

    try {
        const T *packet = T::cast(buffer, len);
        hdrLen = sizeof(T);
        payloadLen = (packet->getLength() - sizeof(T));
        return true;
    } catch(...) {
        ::lseek(fd, -1*sizeof(T), SEEK_CUR);
        return false;
    }
}

bool FileCircularBuffer::readPacket(epicsTime &maxTime)
{
    if (m_fd == -1)
        return false;

    off_t fdOffset = ::lseek(m_fd, 0, SEEK_CUR);

    uint32_t hdrLen, payloadLen;
    uint8_t *buffer = m_buffer.data() + m_offset;
    uint32_t bufLen = m_buffer.size() - m_offset;
    try {
        if (m_oldPackets) {
            if (readHeader<DasPacket>(m_fd, buffer, bufLen, hdrLen, payloadLen) == false) {
                return false;
            }
        } else {
            if (readHeader<Packet>   (m_fd, buffer, bufLen, hdrLen, payloadLen) == false) {
                return false;
            }
        }
    } catch (...) {
        close();
        return false;
    }

    int ret = ::read(m_fd, buffer + hdrLen, payloadLen);
    if (ret == -1) {
        close();
        return false;
    }
    if (ret != static_cast<int>(payloadLen)) {
        ::lseek(m_fd, fdOffset, SEEK_SET);
        return false;
    }

    epicsTime packetTime;
    if (m_oldPackets) {
        packetTime = getPacketTimeStamp(reinterpret_cast<DasPacket *>(buffer));
    } else {
        packetTime = getPacketTimeStamp(reinterpret_cast<Packet *>(buffer));
    }
    if (packetTime > maxTime && maxTime != epicsTimeStamp{0, 0}) {
        ::lseek(m_fd, fdOffset, SEEK_SET);
        return false;
    }
    maxTime = packetTime;

    m_offset += hdrLen + payloadLen;
    return true;
}
