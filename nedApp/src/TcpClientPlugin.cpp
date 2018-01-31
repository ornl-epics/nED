/* TcpClientPlugin.cpp
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "CircularBuffer.h"
#include "Log.h"
#include "TcpClientPlugin.h"

#include <alarm.h> // EPICS alarm and severity enumerations
#include <cstring> // strerror
#include <osiSock.h>
#include <poll.h>

EPICS_REGISTER_PLUGIN(TcpClientPlugin, 2, "Port name", string, "Buffer size", int);

TcpClientPlugin::TcpClientPlugin(const char *portName, uint32_t bufferSize)
    : BasePortPlugin(portName, 0, asynOctetMask, asynOctetMask)
{
    createParam("Status",           asynParamInt32,     &Status, 0);                // READ - Status of TcpClientPlugin       (see TcpClientPlugin::Status)
    createParam("StatusText",       asynParamOctet,     &StatusText, "");  	        // READ - Current error text if any
    createParam("Connect",          asynParamInt32,     &Connect);                  // WRITE - Force re-connecting the socket
    createParam("Disconnect",       asynParamInt32,     &Disconnect);               // WRITE - Disconnect socket
    createParam("RemoteHost",       asynParamOctet,     &RemoteHost);               // WRITE - Remote hostname or IP to connect to
    createParam("RemotePort",       asynParamInt32,     &RemotePort);               // WRITE - Remote port to connect to
    callParamCallbacks();

    m_circularBuffer = new CircularBuffer(bufferSize);
    if (!m_circularBuffer) {
        LOG_ERROR("Failed to allocate circular buffer");
        return;
    }
    m_copyThread = std::unique_ptr<Thread>(new Thread(
        "Copy socket data to buffer",
        std::bind(&TcpClientPlugin::copyDataThread, this, std::placeholders::_1),
        epicsThreadGetStackSize(epicsThreadStackMedium),
        epicsThreadPriorityHigh
    ));
    if (!m_copyThread) {
        LOG_ERROR("Failed to create processing thread");
    } else {
        m_copyThread->start();
    }
    if (!m_processThread) {
        LOG_ERROR("Failed to create processing thread");
    } else {
        m_processThread->start();
    }
}

TcpClientPlugin::~TcpClientPlugin()
{
    if (m_copyThread) m_copyThread->stop();
    if (m_processThread) m_processThread->stop();
    if (m_circularBuffer) delete m_circularBuffer;
}

int TcpClientPlugin::connect(const std::string &host, uint16_t port, std::string &error)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERROR("Failed to create socket - %s", strerror(errno));
        error = "socket error";
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, '0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        LOG_ERROR("Failed to resolve IP address '%s'", host.c_str());
        error = "bad hostname";
        return -1;
    }

    if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to connect to '%s:%u' - %s", host.c_str(), port, strerror(errno));
        error = "connection refused";
        return -1;
    }

    LOG_INFO("Connected to '%s:%u'", host.c_str(), port);
    return sock;
}

bool TcpClientPlugin::connect()
{
    int port = getIntegerParam(RemotePort);
    std::string host = getStringParam(RemoteHost);

    if (port != 0 && !host.empty()) {
        if (m_socket != -1) {
            LOG_WARN("Already connected");
            return true;
        }
        
        m_circularBuffer->clear();

        std::string error;
        m_socket = connect(host, port, error);
        if (m_socket >= 0) {
            setIntegerParam(Status, 1);
            setStringParam(StatusText, "connected");
            callParamCallbacks();
            // In case processing thread aborted on its own. Should we make a Reset PV instead?
            m_processThread->start();
            return true;
        } else {
            setIntegerParam(Status, 0);
            setStringParam(StatusText, error);
            callParamCallbacks();
            return false;
        }
    }
    return false;
}

void TcpClientPlugin::disconnect(const std::string &error)
{
    if (m_socket != -1) {
        close(m_socket);
        m_socket = -1;
        LOG_INFO("Closed socket");
        setIntegerParam(Status, 0);
        setStringParam(StatusText, error.empty() ? "disconnected" : error);
        callParamCallbacks();
    }
}

asynStatus TcpClientPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == RemoteHost) {
        setStringParam(RemoteHost, std::string(value, nChars));
        *nActual = nChars;
        connect();
        return asynSuccess;
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

asynStatus TcpClientPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Connect) {
        connect();
        return asynSuccess;
    } else if (pasynUser->reason == Disconnect) {
        disconnect();
        return asynSuccess;
    } else if (pasynUser->reason == RemotePort) {
        if (value < 0 || value > 65535) {
            return asynError;
        }
        setIntegerParam(RemotePort, value);
        connect();
        return asynSuccess;
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

bool TcpClientPlugin::send(const uint8_t *data, size_t len)
{
    if (m_socket == -1) {
        LOG_ERROR("Failed to send to socket, not connected");
        return false;
    }

    int ret = ::send(m_socket, data, len, 0);
    if (ret < 0 || static_cast<size_t>(ret) != len) {
        // TODO: This could enhanced with proper poll()ing and waits
        LOG_ERROR("Failed to send packet - %s", strerror(errno));
        disconnect("send error");
        return false;
    }

    return true;
}

void TcpClientPlugin::copyDataThread(epicsEvent *shutdown)
{
    std::vector<uint8_t> buffer;
    buffer.reserve(10 * 1024);
    
    LOG_INFO("Copy thread started");

    while (shutdown->tryWait() == false) {
        this->lock();
        int sock = m_socket;
        this->unlock();

        // Use poll() to avoid busy waiting
        struct pollfd pollfd;
        pollfd.fd = sock; // negative value supported by poll()
        pollfd.events = POLLIN;

        int ret = ::poll(&pollfd, 1, 100);
        if (ret == 1 && (pollfd.revents & POLLIN)) {
            ret = ::read(sock, buffer.data(), buffer.capacity());
            if (ret > 0) {
                m_circularBuffer->push(buffer.data(), ret);
            } else {
                this->lock();
                disconnect(ret == -1 ? "receive error" : "server closed connection");
                this->unlock();
            }
        }
    }
    LOG_INFO("Copy thread exited");
}

void TcpClientPlugin::handleRecvError(int ret)
{
    this->lock();
    setIntegerParam(Status, 0);
    setStringParam(StatusText, "processing error");
    callParamCallbacks();
    this->unlock();
}
