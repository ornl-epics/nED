/* ProxyPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Log.h"
#include "ProxyPlugin.h"

#include <osiSock.h>
#include <string.h> // strerror

EPICS_REGISTER_PLUGIN(ProxyPlugin, 3, "port name", string, "dispatcher port", string, "blocking callbacks", int);

#define NUM_PROXYPLUGIN_PARAMS      0

ProxyPlugin::ProxyPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BaseSocketPlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_PROXYPLUGIN_PARAMS)
    , m_nTransmitted(0)
    , m_nProcessed(0)
    , m_nReceived(0)
    , m_recvThread("ProxyRecv", std::bind(&ProxyPlugin::sockReceive, this, std::placeholders::_1))
{
    m_recvThread.start();
}

ProxyPlugin::~ProxyPlugin()
{
    m_recvThread.stop();
}

void ProxyPlugin::processData(const DasPacketList * const packetList)
{
    // Do we need to connect the client? There's no extra thread that would wait
    // for client, instead we rely on the incoming data rate to trigger this function
    // quite often.
    bool clientConnected = connectClient();

    m_nReceived += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        // Don't even bother with packet inspection if there's noone interested
        if (!clientConnected)
            continue;

       if (send(reinterpret_cast<const uint32_t *>(packet), packet->getLength()))
            m_nTransmitted++;

        m_nProcessed++;
    }

    // Update parameters
    setIntegerParam(TxCount,    m_nTransmitted);
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

/* Receive thread worker function. */
void ProxyPlugin::sockReceive(epicsEvent *shutdown)
{
    DasPacketList packetsList;

    // Maximum time the loop will run. Too short and it will eat CPU time,
    // too long kills interactive shutdown. 1 second sounds reasonable tradeoff.
    double loopTime = 1.0;

    // Selecting the buffer size depends on expected throughput. With only
    // relatively small command packets going down, few kB is reasonable.
    // Largest commands in use are about 300bytes long.
    size_t bufferSize = 16 * 1024;
    char *buffer = (char *)mallocMustSucceed(bufferSize, "Failed to allocate socket send buffer");

    while (shutdown->tryWait() == false) {
        uint32_t received;

        this->lock();
        bool connected = isClientConnected();
        this->unlock();

        if (!connected) {
            epicsThreadSleep(loopTime);
            // There's a periodic timer checking for client, wait until it connects
            continue;
        }

        if (recv((uint32_t *)buffer, bufferSize, loopTime, &received) == true) {
            uint32_t consumed = packetsList.reset(reinterpret_cast<uint8_t*>(buffer), received);
            uint32_t nPackets = 0;

            this->lock();
            for (auto it = packetsList.cbegin(); it != packetsList.cend(); it++) {
                sendToDispatcher(*it);
                nPackets++;
            }
            this->unlock();

            packetsList.release();

            if (consumed != received)
                LOG_WARN("Some data received from socket can't be parsed, received %u b, processed %u b (%u packets)", received, consumed, nPackets);
        }
    }
}

void ProxyPlugin::clientConnected()
{
    LOG_INFO("Client connected to OCC proxy");
}

void ProxyPlugin::sendHeartbeat()
{
    DasPacket *packet = DasPacket::createOcc(DasPacket::HWID_SELF, DasPacket::HWID_BROADCAST, DasPacket::CMD_READ_STATUS, 0, 0);
    if (packet) {
        packet->setResponse();
        send(reinterpret_cast<uint32_t*>(packet), packet->getLength());
        delete packet;
    }
}
