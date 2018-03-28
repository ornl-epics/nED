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
#include <list>
#include <map>
#include <math.h>
#include <sstream>

BasePortPlugin::BasePortPlugin(const char *pluginName, int blocking, int interfaceMask, int interruptMask,
                               int queueSize, int asynFlags, int priority, int stackSize)
    : BasePlugin(pluginName, blocking, interfaceMask, interruptMask, queueSize, asynFlags, priority, stackSize)
{
    createParam("BufUsed",          asynParamInt32,     &BufUsed);                  // READ - Virtual buffer used space
    createParam("BufSize",          asynParamInt32,     &BufSize);                  // READ - Virtual buffer size
    createParam("CopyRate",         asynParamInt32,     &CopyRate);                 // READ - Copy throughput in B/s
    createParam("ProcRate",         asynParamInt32,     &ProcRate);                 // READ - Data processing throughput in B/s
    createParam("OldPktsEn",        asynParamInt32,     &OldPktsEn, 0);             // WRITE - Enable support for old DAS 1.0 packets
    createParam("EventsFmt",        asynParamInt32,     &EventsFmt, 0);             // WRITE - Data type when not defined in packet (DAS 1.0 only)
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

void BasePortPlugin::recvUpstream(const DasCmdPacketList &packets)
{
    bool forceOldPkts = getBooleanParam(OldPktsEn);

    // Need to send packets one at a time - OCC limitation
    for (const auto& p: packets) {
        if (forceOldPkts) {
            DasPacket *packet;
            if ((p->getLength() + sizeof(DasPacket)) > m_sendBuffer.size()) {
                LOG_ERROR("Packet to big to send, skipping");
                continue;
            }

            // Send packet to DSP
            packet = DasPacket::initOptical(m_sendBuffer.data(), m_sendBuffer.size(), p);
            if (packet && !send(m_sendBuffer.data(), packet->getLength())) {
                LOG_ERROR("Failed to send packet");
                break;
            }

            // ... but also to everybody else since we don't know DSP id
            packet = DasPacket::initLvds(m_sendBuffer.data(), m_sendBuffer.size(), p);
            if (packet && !send(m_sendBuffer.data(), packet->getLength())) {
                LOG_ERROR("Failed to send packet");
                break;
            }

            // Support for some old hardware that doesn't respond to broadcast LVDS commands
            if (p->getCommand() == DasCmdPacket::CMD_DISCOVER && p->getModuleId() == DasCmdPacket::BROADCAST_ID) {
                packet = DasPacket::initLvds(m_sendBuffer.data(), m_sendBuffer.size(), p, true);
                if (packet && !send(m_sendBuffer.data(), packet->getLength())) {
                    LOG_ERROR("Failed to send packet");
                    break;
                }
            }

        } else {
            if (p->getLength() > m_sendBuffer.size()) {
                LOG_ERROR("Packet to big to send, skipping");
                continue;
            }

            if (p->getSequenceId() != 0) {
                if (!send(reinterpret_cast<const uint8_t *>(p), p->getLength())) {
                    LOG_ERROR("Failed to send packet");
                    break;
                }
            } else {
                Packet *packet = new (m_sendBuffer.data()) Packet(p);
                packet->setSequenceId(++m_sendId % 255);

                if (packet && !send(m_sendBuffer.data(), packet->getLength())) {
                    LOG_ERROR("Failed to send packet");
                    break;
                }
            }
        }
    }
}

void BasePortPlugin::recvUpstream(const DasPacketList &packets)
{
    if (getBooleanParam(OldPktsEn) == false) {
        LOG_ERROR("Failed to send packet, DAS 1.0 packets disabled");
        return;
    }

    // Need to send packets one at a time - OCC limitation
    for (const auto& packet: packets) {
        if (!send(reinterpret_cast<const uint8_t *>(packet), packet->getLength())) {
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

uint32_t BasePortPlugin::processData(const uint8_t *ptr, uint32_t size)
{
    bool forceOldPkts = getBooleanParam(OldPktsEn);
    auto dataFormat = static_cast<DasDataPacket::EventFormat>(getIntegerParam(EventsFmt));

    DasPacketList oldDas;
    DasDataPacketList dasData;
    DasCmdPacketList dasCmd;
    RtdlPacketList rtdls;
    ErrorPacketList errors;
    std::list<std::shared_ptr<uint8_t>> fromPool; //!< Packets to be returned back to pool
    uint32_t nPackets = 0; // Used for throwing an exception on first packet

    const uint8_t *end = ptr + size;
    while (ptr < end) {
        uint32_t bytesLeft = (end - ptr);

        // We don't know what we're receiving. It could be old DAS packet or
        // a new DAS header. They differ in the most significant 4 bits of the
        // first 32 bits received.
        auto version = reinterpret_cast<const Packet *>(ptr)->getVersion();

        const Packet *packet = nullptr;

        try {
            if (version == 0 || forceOldPkts) {
                // Old DAS packet
                const DasPacket *das1Packet = DasPacket::cast(ptr, bytesLeft);
                ptr += das1Packet->getLength();
                oldDas.push_back(das1Packet);

                // Convert to packet format used internally
                size_t bufsize = 2 * das1Packet->getLength(); // just an estimate how much space we need
                auto buffer = m_packetsPool.getPtr(bufsize);
                if (buffer) {
                    fromPool.push_back(buffer);
                    packet = das1Packet->convert(buffer.get(), bufsize, dataFormat);
                } else {
                    throw std::runtime_error("Failed to allocate packet from pool");
                }
                // Will add new-style packet to list below

            } else if (version == 1) {
                packet = Packet::cast(ptr, bytesLeft);

                if (m_recvId != 0xFFFFFFFF && packet->getSequenceId() != ((m_recvId+1) % 255) && packet->getSequenceId() != 0) {
                    LOG_ERROR("Expecting packet with sequence number %u, got %u", (m_recvId+1)%255, packet->getSequenceId());
                }
                m_recvId = packet->getSequenceId();
                ptr += packet->getLength();

            } else {
                throw std::runtime_error("Unsupported packet received");
            }
            nPackets++;
        } catch (...) {
            if (nPackets == 0)
                throw;
            break;
        }

        if (packet != nullptr) {
            // Put packet in the corresponding list
            switch (packet->getType()) {
            case Packet::TYPE_DAS_DATA:
                {
                    auto dataPacket = reinterpret_cast<const DasDataPacket *>(packet);
                    if (dataPacket->checkIntegrity())
                        dasData.push_back(dataPacket);
                    else
                        LOG_WARN("Discarding DAS data packet, integrity check failed");
                }
                break;
            case Packet::TYPE_RTDL:
                {
                    auto rtdlPacket = reinterpret_cast<const RtdlPacket *>(packet);
                    if (rtdlPacket->checkIntegrity())
                        rtdls.push_back(rtdlPacket);
                    else
                        LOG_WARN("Discarding RTDL packet, integrity check failed");
                }
                break;
            case Packet::TYPE_DAS_CMD:
                {
                    auto cmdPacket = reinterpret_cast<const DasCmdPacket *>(packet);
                    if (cmdPacket->checkIntegrity())
                        dasCmd.push_back(cmdPacket);
                    else
                        LOG_WARN("Discarding DAS command packet, integrity check failed");
                }
                break;
            case Packet::TYPE_ERROR:
                errors.push_back(reinterpret_cast<const ErrorPacket *>(packet));
                break;
            default:
                break;
            }
        }
    }

    // Publish all packets in parallel ..
    std::vector< std::unique_ptr<PluginMessage> > messages;
    if (!oldDas.empty())
        messages.push_back(sendDownstream(oldDas, false));
    if (!dasCmd.empty())
        messages.push_back(sendDownstream(dasCmd, false));
    if (!dasData.empty())
        messages.push_back(sendDownstream(dasData, false));
    if (!rtdls.empty())
        messages.push_back(sendDownstream(rtdls, false));
    if (!errors.empty())
        messages.push_back(sendDownstream(errors, false));

    // .. and wait for all of them to get released
    for (const auto& m: messages) {
        if (m) {
            m->waitAllReleased();
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
