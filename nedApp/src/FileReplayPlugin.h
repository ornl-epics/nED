/* FileReplayPlugin.h
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef FILE_REPLAY_PLUGIN_H
#define FILE_REPLAY_PLUGIN_H

#include "BasePortPlugin.h"

class FileCircularBuffer;

/**
 * TODO!!!
 */
class epicsShareFunc FileReplayPlugin : public BasePortPlugin {
    private:
        FileCircularBuffer m_file;

    public:
        /**
         * Constructor
         *
         * @param[in] portName Name of the asyn port to which plugins can connect
         */
        FileReplayPlugin(const char *portName);

        ~FileReplayPlugin();

    private:
        /**
         * Overloaded method to handle writing octets from PVs.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual) override;

        /**
         * Overloaded method.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

        /**
         * Overloaded method.
         */
        asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value) override;

        /**
         * Send data to socket.
         */
        bool send(const uint8_t *data, size_t len) override;

        /**
         * Report an error detected in receive data thread
         */
        void handleRecvError(int ret) override;

    private:
        int Status;
        int StatusText;
        int FilePath;
        int Control;
        int Speed;
        int MaxPackets;
        int Running;
};

#endif // FILE_REPLAY_PLUGIN_H
