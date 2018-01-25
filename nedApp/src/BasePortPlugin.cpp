/* BasePortPlugin.cpp
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "BasePortPlugin.h"
#include "Log.h"

#include <cstring> // strerror
#include <math.h>
#include <sstream>

/**
 * A global variable to assign unique sourceId to all packets coming from this device.
 */
static uint32_t g_sourceId = 0;

BasePortPlugin::BasePortPlugin(const char *pluginName, int blocking, int interfaceMask, int interruptMask,
                       int queueSize, int asynFlags, int priority, int stackSize)
    : BasePlugin(pluginName, blocking, interfaceMask, interruptMask, queueSize, asynFlags, priority, stackSize)
    , m_sourceId(g_sourceId++)
{
    createParam("BufUsed",          asynParamInt32,     &BufUsed);                  // READ - Virtual buffer used space
    createParam("BufSize",          asynParamInt32,     &BufSize);                  // READ - Virtual buffer size
    createParam("CopyRate",         asynParamInt32,     &CopyRate);                 // READ - Copy to internal buffer throughput in B/s
    createParam("ProcRate",         asynParamInt32,     &ProcRate);                 // READ - Data processing throughput in B/s
    createParam("MaxPktSize",       asynParamInt32,     &MaxPktSize, 4000);         // WRITE - Maximum size of DAS packet payload
    createParam("OldPktsEn",        asynParamInt32,     &OldPktsEn, 0);             // WRITE - Enable support for old DAS 1.0 packets
    callParamCallbacks();

    m_processThread = std::unique_ptr<Thread>(new Thread(
        "Process incoming data",
        std::bind(&BasePortPlugin::processDataThread, this, std::placeholders::_1),
        epicsThreadGetStackSize(epicsThreadStackMedium),
        epicsThreadPriorityHigh
    ));
}

BasePortPlugin::~BasePortPlugin()
{
    if (m_processThread) {
        m_processThread->stop();
    }
}

asynStatus BasePortPlugin::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
    assert(m_circularBuffer != nullptr);

    if (pasynUser->reason == BufUsed) {
        *value = m_circularBuffer->used();
        return asynSuccess;
    }
    if (pasynUser->reason == BufSize) {
        *value = m_circularBuffer->size();
        return asynSuccess;
    }
    if (pasynUser->reason == ProcRate) {
        *value = m_circularBuffer->getReadRate();
        return asynSuccess;
    }
    if (pasynUser->reason == CopyRate) {
        // is 0 when no copy buffer is used
        *value = m_circularBuffer->getPushRate();
        return asynSuccess;
    }
    return asynPortDriver::readInt32(pasynUser, value);
}

void BasePortPlugin::recvUpstream(DasCmdPacketList *packets)
{
    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        DasCmdPacket *packet = *it;
        packet->sequence = (++m_sendId % 255);
        int len = ALIGN_UP(packet->length, 4);
        if (!send(reinterpret_cast<const uint8_t*>(packet), len)) {
            LOG_ERROR("Failed to send packet");
            break;
        }
    }

}

void BasePortPlugin::recvUpstream(DasPacketList *packets)
{
    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        DasPacket *packet = *it;
        int len = ALIGN_UP(packet->getLength(), 4);
        if (!send(reinterpret_cast<const uint8_t*>(packet), len)) {
            LOG_ERROR("Failed to send packet");
            break;
        }
    }
}

void BasePortPlugin::processDataThread(epicsEvent *shutdown)
{
    assert(m_circularBuffer != nullptr);

    uint32_t retryCounter = 0;
    
    LOG_INFO("Process thread started");

    while (shutdown->tryWait() == false) {
        void *data;
        uint32_t length;

        // Wait for data, use a timeout for data rate out calculation
        int ret = m_circularBuffer->wait(&data, &length, 1.0);
        if (ret == -ETIME || ret == -ECONNRESET) {
            continue;
        } else if (ret != 0) {
            handleRecvError(ret);
            LOG_ERROR("Unable to read data from buffer, processing thread stopped - %s(%d)\n", strerror(-ret), ret);
            break;
        }

        try {
            uint32_t left = processData(reinterpret_cast<uint8_t*>(data), length);
            m_circularBuffer->consume(length - left);
            retryCounter = 0;
        } catch (std::runtime_error &e) {
            if (retryCounter < 7) {
                LOG_DEBUG("Partial data in buffer, waiting for more (retry %u/6)", retryCounter);
                // Exponentially sleep up to ~1.1s, first pass doesn't sleep
                epicsThreadSleep(1e-5 * pow(10, retryCounter++));
                continue;
            }

            // Still doesn't have enough data, abort thread
            LOG_ERROR("Aborting processing thread: %s", e.what());
            dump(reinterpret_cast<const char *>(data), length);
            LOG_ERROR("dumped");
            handleRecvError(-ERANGE);
            LOG_ERROR("handled error");
            break;
        }
    }
    
    LOG_INFO("Process thread exited");
}

uint32_t BasePortPlugin::processData(uint8_t *ptr, uint32_t size)
{
    int maxPktSize = getIntegerParam(MaxPktSize);
    bool forceOldPkts = getBooleanParam(OldPktsEn);
    bool first = true;

    DasPacketList oldDas;
    DasDataPacketList dasData;
    DasCmdPacketList dasCmd;
    DasRtdlPacketList dasRtdl;
    ErrorPacketList errors;

    uint8_t *end = ptr + size;
    while (ptr < end) {
        uint32_t bytesLeft = (end - ptr);
        uint32_t bytesProcessed = 0;

        // We don't know what we're receiving. It could be old DAS packet or
        // a new DAS header. They differ in the most significant 4 bits of the
        // first 32 bits received.
        uint32_t version = (*reinterpret_cast<uint32_t *>(ptr) >> 28);

        try {
            if (version == 0 || forceOldPkts) {
                // Old DAS packet
                DasPacket *packet = reinterpret_cast<DasPacket *>(ptr);

                if (bytesLeft < sizeof(DasPacket)) {
                    std::ostringstream error;
                    error << "Not enough data to describe old DAS packet header, needed " << sizeof(DasPacket) << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (bytesLeft < packet->getLength()) {
                    std::ostringstream error;
                    error << "Not enough data to describe old DAS packet, needed " << packet->getLength() << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (static_cast<uint32_t>(maxPktSize) < packet->getLength()) {
                    std::ostringstream error;
                    error << "Old DAS packet of " << packet->getLength() << " bytes exceeds " << maxPktSize << " bytes threshold";
                    throw std::range_error(error.str());
                }

                oldDas.push_back(packet);
                bytesProcessed = packet->getLength();
                if (bytesProcessed == 0) {
                    throw std::range_error("failed to parse old DAS packet");
                }

            } else if (version == 1) {
                // New SNS packet header
                Packet *packet = reinterpret_cast<Packet *>(ptr);

                if (bytesLeft < sizeof(Packet)) {
                    std::ostringstream error;
                    error << "Not enough data to describe DAS packet header, needed " << sizeof(Packet) << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (bytesLeft < packet->length) {
                    std::ostringstream error;
                    error << "Not enough data to describe DAS packet, needed " << packet->length << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (static_cast<uint32_t>(maxPktSize) < packet->length) {
                    std::ostringstream error;
                    error << "DAS packet of " << packet->length << " bytes exceeds " << maxPktSize << " bytes threshold";
                    throw std::range_error(error.str());
                }

                if (m_recvId != 0xFFFFFFFF && packet->sequence != ((m_recvId+1) % 255)) {
                    LOG_ERROR("Expecting packet with sequence number %u, got %u", (m_recvId+1)%255, packet->sequence);
                }
                m_recvId = packet->sequence;

                if (packet->type == Packet::TYPE_DAS_DATA) {
                    reinterpret_cast<DasDataPacket *>(packet)->source = m_sourceId;
                    dasData.push_back(reinterpret_cast<DasDataPacket *>(packet));
                } else if (packet->type == Packet::TYPE_DAS_RTDL) {
                    reinterpret_cast<DasRtdlPacket *>(packet)->source = m_sourceId;
                    dasRtdl.push_back(reinterpret_cast<DasRtdlPacket *>(packet));
                } else if (packet->type == Packet::TYPE_DAS_CMD) {
                    dasCmd.push_back(reinterpret_cast<DasCmdPacket *>(packet));
                } else if (packet->type == Packet::TYPE_ERROR) {
                    reinterpret_cast<ErrorPacket *>(packet)->source = m_sourceId;
                    dasRtdl.push_back(reinterpret_cast<DasRtdlPacket *>(packet));
                }

                bytesProcessed = packet->length;

            } else {
                throw std::runtime_error("Unsupported packet received");
            }
        } catch (std::exception &e) {
            if (first)
                throw;
            else
                break;
        }

        ptr += bytesProcessed;
        first = false;
    }

    // Publish all packets in parallel ..
    std::vector< std::shared_ptr<PluginMessage> > messages;
    if (!oldDas.empty())
        messages.push_back(sendDownstream(&oldDas, false));
    if (!dasCmd.empty())
        messages.push_back(sendDownstream(&dasCmd, false));
    if (!dasData.empty())
        messages.push_back(sendDownstream(&dasData, false));
    if (!dasRtdl.empty())
        messages.push_back(sendDownstream(&dasRtdl, false));
    if (!errors.empty())
        messages.push_back(sendDownstream(&errors, false));

    // .. and wait for all of them to get released
    for (auto it = messages.begin(); it != messages.end(); it++) {
        if (!!(*it)) {
            (*it)->waitAllReleased();
        }
    }

    return (end - ptr);
}

void BasePortPlugin::dump(const char *data, uint32_t len)
{
    char buffer[64] = { 0 };
    char *ptr = buffer;
    const uint32_t maxlen = 4096;

    LOG_DEBUG("Dump %u bytes of raw data at offset %p", len, data);

    // OCC library is always 4-byte aligned, so the buffer should
    // be good for 4-byte reads even if the packet is not 4-byte
    // aligned.
    for (uint32_t i = 0; i < std::min(len, maxlen)/4; i++) {
        if ((i % 4) == 0 && i > 0) {
            LOG_DEBUG("%s", buffer);
            buffer[0] = 0;
            ptr = buffer;
        }
        snprintf(ptr, buffer + sizeof(buffer) - ptr, "0x%08X ", *(uint32_t *)(data + i*4));
        ptr += 11;
    }
    if (ptr > 0) {
        LOG_DEBUG("%s", buffer);
    }
    if (len > maxlen) {
        LOG_DEBUG("... truncated to %u bytes", maxlen);
    }
}
