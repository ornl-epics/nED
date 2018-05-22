/* BaseSocketPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseSocketPlugin.h"
#include "likely.h"
#include "Log.h"

#include <fcntl.h>
#include <poll.h>
#include <osiSock.h>
#include <string.h> // strerror

BaseSocketPlugin::BaseSocketPlugin(const char *portName)
    : BasePlugin(portName, 1)
    , m_listenSock(-1)
    , m_clientSock(-1)
{
    createParam("ListenIp",     asynParamOctet,     &ListenIP);         // WRITE - Hostname or IP address to listen to
    createParam("ListenPort",   asynParamInt32,     &ListenPort);       // WRITE - Port number to listen to
    createParam("ListenStatus", asynParamInt32,     &ListenStatus, STATUS_NOT_INIT);  // READ - Listening status
    createParam("ClientIp",     asynParamOctet,     &ClientIP, "");     // READ - Client IP if connected, or empty string
    createParam("CheckInt",     asynParamInt32,     &CheckInt, 2);      // WRITE - Check client interval in seconds
    createParam("ClientInactive",asynParamFloat64,  &ClientInactive, 0.0);// READ - Number of seconds of client inactivity
    createParam("CloseClient",  asynParamInt32,     &CloseClient, 0);   // WRITE - Force client disconnect
    callParamCallbacks();

    m_lastClientActivity = { 0, 0 };

    // Schedule a period task to check for incoming client
    std::function<float(void)> watchdogCb = std::bind(&BaseSocketPlugin::checkClient, this);
    scheduleCallback(watchdogCb, 2.0);
}

bool BaseSocketPlugin::recv(uint32_t *data, uint32_t length, double timeout, uint32_t *actual)
{
    struct pollfd fds;
    int timeout_ms = (timeout > 0 ? timeout * 1000 : 0);
    int ret;

    this->lock();
    fds.fd = m_clientSock;
    this->unlock();
    fds.events = POLLIN;
    fds.revents = 0;

    ret = poll(&fds, 1, timeout_ms);

    if (ret != 1 || fds.revents != POLLIN) {
        if (ret == -1) {
            LOG_ERROR("Closed socket due to poll() failure - %s", strerror(errno));
            disconnectClient();
        }
        *actual = 0;
        return false;
    }

    // poll() returned successfully, read() will either read data or return -1
    // immediately, non-blocking.
    ret = read(fds.fd, reinterpret_cast<void *>(data), length);
    if (ret == -1) {
        LOG_ERROR("Closed socket due to write error - %s", strerror(errno));
        disconnectClient();
        *actual = 0;
        return false;
    } else if (ret > 0) {
        this->lock();
        epicsTimeGetCurrent(&m_lastClientActivity);
        this->unlock();
    }

    *actual = ret;

    return (ret > 0);
}

bool BaseSocketPlugin::send(const uint32_t *data, uint32_t length)
{
    const char *rest = reinterpret_cast<const char *>(data);
    epicsTimeStamp t1;
    int myErrno = 0;
    int retries = 3; // 1 second for each retry defined when client connects
    int socket;
    epicsTimeStamp lastClientActivity = { 0, 0 };

    // Cache the same socket for the duration of this function. If client
    // disconnects and a new client is connected,
    // we don't want to send partial data to new client but rather fail this function.
    this->lock();
    socket = m_clientSock;
    this->unlock();

    if (socket == -1)
        return false;

    epicsTimeGetCurrent(&t1);
    while (length > 0 && retries-- > 0) {
        ssize_t sent = write(socket, rest, length);
        if (likely(sent == length)) {
            // Optimize most likely path, mainly avoid time consuming unlocking/locking
            epicsTimeGetCurrent(&lastClientActivity);
            length = 0;
            break;
        } else if (sent > 0) {
            epicsTimeGetCurrent(&lastClientActivity);
            length -= sent;
            rest += sent;
            myErrno = 0;
            continue;
        } else if (sent == -1) {
            myErrno = errno;
            // SO_SNDTIMEO will interrupt every 1.0s
            if (myErrno == EAGAIN || myErrno == EWOULDBLOCK) {
                // Export client inactivity time through parameter.
                // PV database can be configured to automatically disconnect
                // at certain threshold.
                epicsTimeStamp t2;
                epicsTimeGetCurrent(&t2);
                double inactive = epicsTimeDiffInSeconds(&t2, &t1);
                this->lock();
                setDoubleParam(ClientInactive, inactive);
                callParamCallbacks();
                this->unlock();
            }
        } else { // sent == 0
            myErrno = ENOSPC;
        }
    }

    this->lock();
    if (length > 0) {
        disconnectClient();
        if (retries == 0) {
            LOG_ERROR("Closed socket, too many retries");
        } else {
            LOG_ERROR("Closed socket due to a write error - %s", strerror(myErrno));
        }
    } else {
        m_lastClientActivity = lastClientActivity;
    }
    this->unlock();

    return (length == 0);
}

asynStatus BaseSocketPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    asynStatus status;

    if (pasynUser->reason == ListenPort) {
        int prevValue;
        char host[256];

        if (value <= 0 || value > 0xFFFF)
            return asynError;

        status = getIntegerParam(ListenPort, &prevValue);
        if (status == asynSuccess && prevValue == value)
            return asynSuccess;

        status = setIntegerParam(ListenPort, value);
        if (status != asynSuccess) {
            LOG_ERROR("Error setting ListenPort parameter");
            return status;
        }
        callParamCallbacks();

        status = getStringParam(ListenIP, sizeof(host), host);
        if (status == asynSuccess && strlen(host) > 0) {
            (void)setupListeningSocket(host, value);
        }

        return asynSuccess;
    } else if (pasynUser->reason == CheckInt) {
        // If we were to add support for canceling the timer through value 0 here,
        // we'd have to implement canceling old timer and scheduling new one.
        if (value < 1)
            value = 1;
        setIntegerParam(CheckInt, value);
        callParamCallbacks();
        return asynSuccess;
    } else if (pasynUser->reason == CloseClient) {
        if (value > 0) {
            disconnectClient();
            LOG_INFO("Disconnected client on user request");
        }
    }

    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus BaseSocketPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    asynStatus status = asynError;

    if (pasynUser->reason == ListenIP) {
        int port;
        char prevValue[128];
        std::string host(value, nChars);
        *nActual = nChars;

        status = getStringParam(ListenIP, sizeof(prevValue), prevValue);
        if (status == asynSuccess && strncmp(prevValue, value, sizeof(prevValue)) == 0)
            return asynSuccess;

        status = setStringParam(ListenIP, value);
        if (status != asynSuccess) {
            LOG_ERROR("Error setting ListenIP parameter");
            return status;
        }
        callParamCallbacks();

        status = getIntegerParam(ListenPort, &port);
        if (status == asynSuccess && ListenPort > 0) {
            (void)setupListeningSocket(host.c_str(), port);
        }

        return asynSuccess;
    }
    return status;
}

bool BaseSocketPlugin::setupListeningSocket(const std::string &host, uint16_t port)
{
    struct sockaddr_in sockaddr;
    int sock;
    int optval;
    int status = STATUS_NOT_INIT;

    if (m_listenSock != -1) {
        close(m_listenSock);
        m_listenSock = -1;
    }

    do {
        if (aToIPAddr(host.c_str(), 0, &sockaddr) != 0) {
            LOG_ERROR("Cannot resolve host '%s' to IP address", host.c_str());
            status = STATUS_BAD_IP;
            break;
        }
        sockaddr.sin_port = htons(port);

        sock = epicsSocketCreate(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            char sockErrBuf[64];
            status = STATUS_SOCKET_ERR;
            epicsSocketConvertErrnoToString(sockErrBuf, sizeof(sockErrBuf));
            LOG_ERROR("Failed to create stream socket - %s", sockErrBuf);
            break;
        }

        optval = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) != 0) {
            status = STATUS_SOCKET_ERR;
            LOG_ERROR("Failed to set socket parameters - %s", strerror(errno));
            close(sock);
            break;
        }

        if (::bind(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) != 0) {
            status = (errno == EACCES ? STATUS_BIND_PERM : STATUS_SOCKET_ERR);
            LOG_ERROR("Failed to bind to socket - %s", strerror(errno));
            close(sock);
            break;
        }

        if (listen(sock, 1) != 0) {
            status = (errno == EADDRINUSE ? STATUS_IN_USE : STATUS_SOCKET_ERR);
            LOG_ERROR("Failed to listen to socket - %s", strerror(errno));
            close(sock);
            break;
        }
        status = STATUS_LISTENING;
        LOG_INFO("Listening on %s:%u", host.c_str(), port);
    } while (0);

    if (status == STATUS_LISTENING) {
        m_listenSock = sock;
    }

    setIntegerParam(ListenStatus, status);
    callParamCallbacks();

    return (status == STATUS_LISTENING);
}

bool BaseSocketPlugin::isClientConnected()
{
    return (m_clientSock != -1);
}

bool BaseSocketPlugin::connectClient()
{
    char clientip[128];
    struct pollfd fds;
    struct timeval timeout;

    if (m_clientSock != -1)
        return true;

    fds.fd = m_listenSock; // POSIX allows negative values, revents becomes 0
    fds.events = POLLIN;
    fds.revents = 0;

    // Non-blocking check
    if (poll(&fds, 1, 0) == 0 || fds.revents != POLLIN)
        return false;

    // There should be client waiting now - accept() won't block
    struct sockaddr client;
    socklen_t len = sizeof(struct sockaddr);
    m_clientSock = accept(m_listenSock, &client, &len);
    if (m_clientSock == -1)
        return false;

    sockAddrToDottedIP(&client, clientip, sizeof(clientip));
    setStringParam(ClientIP, clientip);
    callParamCallbacks();

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(m_clientSock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0) {
        // Socket remains in blocking mode, hope client doesn't stall
        LOG_ERROR("Failed to set socket timeout");
    }

    clientConnected();

    LOG_INFO("New TCP client from %s", clientip);

    return true;
}

void BaseSocketPlugin::disconnectClient()
{
    if (m_clientSock != -1) {
        (void)fcntl(m_clientSock, F_SETFL, O_NONBLOCK);
        (void)close(m_clientSock);
        m_clientSock = -1;

        setStringParam(ClientIP, "");
        callParamCallbacks();

        clientDisconnected();
    }
}

float BaseSocketPlugin::checkClient()
{
    int checkInt;
    epicsTimeStamp now;

    getIntegerParam(CheckInt, &checkInt);
    epicsTimeGetCurrent(&now);

    double inactive = epicsTimeDiffInSeconds(&now, &m_lastClientActivity);

    if (isClientConnected() && inactive >= checkInt) {
        sendHeartbeat();
    }

    // Now check again in case heartbeat failed and closed the socket
    if (!isClientConnected()) {
        connectClient();
    }

    return checkInt;
}
