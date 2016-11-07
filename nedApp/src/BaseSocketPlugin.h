/* BaseSocketPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASESOCKET_PLUGIN_H
#define BASESOCKET_PLUGIN_H

#include "BasePlugin.h"

/**
 * Plugin with server socket capabilities
 *
 * This abstract plugin provides server socket functionality. When listen IP and
 * port parameters are being set through asyn write handlers, plugin will
 * automatically open single listening socket. There's no thread running to
 * check when the client actually connects, instead derived class should
 * periodically call connectClient() function.
 */
class BaseSocketPlugin : public BasePlugin {
    private:
        int m_listenSock;           //!< Socket for incoming connections, -1 when not listening
        int m_clientSock;           //!< Client socket to send/receive data to/from, -1 when no client

    public:
        static const int defaultInterfaceMask = asynOctetMask | asynFloat64Mask | BasePlugin::defaultInterfaceMask;
        static const int defaultInterruptMask = asynOctetMask | asynFloat64Mask | BasePlugin::defaultInterruptMask;

        enum {
            STATUS_NOT_INIT     = 0, //!< Not initialized
            STATUS_LISTENING    = 1, //!< Listening
            STATUS_BAD_IP       = 2, //!< Can not resolve hostname/IP
            STATUS_SOCKET_ERR   = 3, //!< Socket error
            STATUS_BIND_PERM    = 4, //!< Not allowed to bind to port
            STATUS_IN_USE       = 5, //!< Socket already in use
        };

        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         * @param[in] numParams The number of parameters that the derived class supports.
         * @param[in] maxAddr The maximum  number of asyn addr addresses this driver supports. 1 is minimum.
         * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports.
         * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks)
         * @param[in] asynFlags Flags when creating the asyn port driver; includes ASYN_CANBLOCK and ASYN_MULTIDEVICE.
         * @param[in] autoConnect The autoConnect flag for the asyn port driver.
         * @param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
         * @param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
         */
        BaseSocketPlugin(const char *portName, const char *dispatcherPortName, int blocking, int numParams=0,
                     int maxAddr=1, int interfaceMask=defaultInterfaceMask, int interruptMask=defaultInterruptMask,
                     int asynFlags=0, int autoConnect=1, int priority=0, int stackSize=0);

        /**
         * Destructor
         */
        virtual ~BaseSocketPlugin();

        /**
         * Handle BaseSocketPlugin integer parameters changes.
         */
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Handle BaseSocketPlugin string parameter changes.
         */
        virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Setup listening socket.
         *
         * The old listening socket is closed only if new one is successfully created.
         *
         * @param[in] host Hostname or IP address.
         * @param[in] port Port number to listen on.
         * @return true when configured, false otherwise.
         */
        bool setupListeningSocket(const std::string &host, uint16_t port);

        /**
         * Check whether there's a remote client connected to the server port.
         *
         * Function does not validate connection. If the client disconnected but
         * the socket is still valid, function might return true.
         *
         * @return true if connected.
         */
        bool isClientConnected();

        /**
         * Try to connect client socket, don't block.
         *
         * Caller must hold a lock. When returned with true, m_clientSock is a valid
         * client id. Function updates CLIENT_IP parameter.
         *
         * @return true if connected, false otherwise.
         */
        bool connectClient();

        /**
         * Disconnect client if connected.
         *
         * Caller must hold a lock. Function updates CLIENT_IP parameter.
         *
         * @return true when disconnected or no client, false on error.
         */
        void disconnectClient();

        /**
         * Send all data to client through m_clientSock.
         *
         * Function ensures all data has been written to socket or the connection
         * is dropped. When socket's buffer is full, the function will increase
         * the ClientInactive parameter every second. It releases the lock while
         * waiting, allowing CloseClient parameter to be used to kill the
         * connection from outside.
         *
         * Caller of this function must ensure locked access.
         *
         * Function can not be extended with a timeout parameter as the
         * implementation could not guarantee data integrity on the socket -
         * partial data can be transmitted to client when timeout would occur.
         *
         * @param[in] data Data to be sent through the socket.
         * @param[in] length Length of data in bytes.
         * @return true if all data has been sent, false on error
         */
        bool send(const uint32_t *data, uint32_t length);

        /**
         * Receive as much data as available from the socket in specified time.
         *
         * Function waits for at most `timeout' number of seconds if there's no
         * data immediately available. Once any data is available, it will read
         * as much as available or max `length' bytes, whicheve is smaller.
         *
         * Caller of this function must ensure plugin is locked. Function will
         * unlock while waiting for data and allow other threads to run.
         *
         * @param[in] data Buffer where data is put.
         * @param[in] length Buffer size
         * @param[in] timeout in seconds to wait for data if not avaialbe immediately.
         * @param[out] actual Number of bytes read.
         * @return true if any data has been received, false on error
         */
        bool recv(uint32_t *data, uint32_t length, double timeout, uint32_t *actual);

        /**
         * Periodically called to check client connection status or new client
         *
         * Check for new incoming client, connect it and update ClientIp PV.
         *
         * Function is run by the epicsTimer in a background thread shared by
         * timers from all plugins. The timer is initialized in constructor for
         * the first run, later it's driven solely by the return value of this
         * function. If the function returns 0, timer is canceled. Otherwise it
         * delays the next execution for the number of seconds returned.
         * Base implementation of this function reads the delay from a PV every
         * time before it returns.
         * Note: If timer is canceled, it's never restarted.
         *
         * @return Return delay in seconds before invoking the function again,
         *         or 0 to cancel the timer.
         */
        virtual float checkClient();

        /**
         * Signal that a new client is connected.
         *
         * When this function is called, client connection is already established
         * and ready to use. There's periodic check for new client which is driven
         * by the CheckInt parameter.
         */
        virtual void clientConnected() {};

        /**
         * Signal that current client has disconnected.
         *
         * Called when client disconnect has been detected. There's no active
         * mechanism to check whether the client is still alive. Detect mechanism
         * is based on the error returned by writing to socket.
         */
        virtual void clientDisconnected() {};

    protected:
        #define FIRST_BASESOCKETPLUGIN_PARAM ListenIP
        int ListenIP;
        int ListenPort;
        int ListenStatus;
        int ClientIP;
        int CheckInt;
        int ClientInactive;
        int CloseClient;
        #define LAST_BASESOCKETPLUGIN_PARAM CloseClient
};

#endif // BASESOCKET_PLUGIN_H
