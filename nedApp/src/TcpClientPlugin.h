/* TcpClientPlugin.h
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef TCP_CLIENT_PLUGIN_H
#define TCP_CLIENT_PLUGIN_H

#include "BasePortPlugin.h"

/**
 * TODO!!!
 */
class epicsShareFunc TcpClientPlugin : public BasePortPlugin {
    private:
        int m_socket = -1;                      //!< Client socket id or -1
        std::unique_ptr<Thread> m_copyThread;   //!< Thread processing data from buffer

    public:
        /**
         * Constructor
         *
         * @param[in] portName Name of the asyn port to which plugins can connect
         * @param[in] localBufferSize If not zero, a local buffer will be created
         *            where all data from OCC DMA buffer will be copied to as soon
         *            as it is available.
         */
        TcpClientPlugin(const char *portName, uint32_t localBufferSize);
        
        ~TcpClientPlugin();

    private:
        /**
         * Overloaded method to handle writing octets from PVs.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded method.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Reset OCC device and all internal states, including restarting read threads.
         *
         * This function must not be called during global lock.
         */
        void reset();

        /**
         * Establish socket connection to remote host and port.
         */
        int connect(const std::string &host, uint16_t port, std::string &error);

        /**
         * Use RemoteHost and RemotePort parameters to connect socket.
         */
        bool connect();

        /**
         * Disconnect socket and set status and error PVs.
         */
        void disconnect(const std::string &error="");

        /**
         * Send data to socket.
         */
        bool send(const uint8_t *data, size_t len);

        /**
         * Copy data from socket to buffer.
         */
        void copyDataThread(epicsEvent *shutdown);

        /**
         * Report an error detected in receive data thread
         */
        void handleRecvError(int ret);

    private:
        int Status;
        int StatusText;
        int Connect;
        int Disconnect;
        int RemoteHost;
        int RemotePort;
};

#endif // TCP_CLIENT_PLUGIN_H
