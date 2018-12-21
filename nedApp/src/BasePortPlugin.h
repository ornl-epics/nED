/* BasePortPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASE_PORT_PLUGIN_H
#define BASE_PORT_PLUGIN_H

#include "BasePlugin.h"
#include "BaseCircularBuffer.h"
#include "ObjectPool.h"
#include "Thread.h"

#include <vector>

/**
 * A port plugin connect directly to the source hardware device in order to exchange packets.
 *
 * Port plugins are the origin of packets and are usually first in chain. They
 * don't connect to parent plugins, instead the communicate to hardware directly
 * in order to send and receive packets. BasePortPlugin implements common
 * functionality to all port plugins.
 */
class epicsShareFunc BasePortPlugin : public BasePlugin {
    public:
        /**
         * Constructor
         *
         * @param[in] pluginName asyn port name.
         * @param[in] parentPorts Comma separated list of remote ports to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports, this bit mask is added to default one.
         * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks), this bit mask is added to default one.
         * @param[in] queueSize Max number of inter-plugin messages
         * @param[in] asynFlags Flags when creating the asyn port driver; includes ASYN_CANBLOCK and ASYN_MULTIDEVICE.
         * @param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
         * @param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
         */
        BasePortPlugin(const char *pluginName, int blocking=0, int interfaceMask=0, int interruptMask=0,
                   int queueSize=5, int asynFlags=0, int priority=0, int stackSize=0);

        /**
         * Destructor
         */
        ~BasePortPlugin();

        /**
         * Overloaded method to read asyn int32 parameters
         */
        asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);

        /**
         * Overloaded method invoked by iocsh asynReport()
         *
         * After calling parent class report(), it reports the last data received.
         */
        void report(FILE *fp, int details);

        /**
         * Process all available data from buffer.
         *
         * This is the worker function called from the thread. It parses through the
         * buffer until no more packets can be extracted. Each packet is verified to
         * be valid or exception is thrown. Received packets are put in lists and sent
         * to all subscribed plugins. Function returns number of bytes left in
         * the buffer.
         * To allow to debug incoming data, function tries to eat as much data as
         * possible and return the number of bytes processed. It only throws when
         * it can't process even a single packet from the beginning of the buffer.
         *
         * @param[in] ptr to buffer to be processed
         * @param[in] size of data to be processed
         * @raise std::range_error when supported packet verifcation failed
         * @raise std::runtime_error when non-supported packet was received
         * @return Number of bytes not processed.
         */
        virtual uint32_t processData(const uint8_t *ptr, uint32_t size);

        /**
         * Send command packets to OCC.
         */
        virtual void recvUpstream(const DasCmdPacketList &packets);

        /**
         * Send command packets to OCC.
         */
        virtual void recvUpstream(const DasPacketList &packets);

        /**
         * Send data to device - derived class should re-implement this.
         */
        virtual bool send(const uint8_t *data, size_t len) { return false; };

        /**
         * Report an error detected in receive data thread
         */
        virtual void handleRecvError(int ret) {};

    protected:
        std::unique_ptr<Thread> m_processThread;        //!< Thread processing data from buffer
        BaseCircularBuffer *m_circularBuffer = nullptr; //!< Derived class must provide circular buffer

        /**
         * Dump part of data from buffer.
         */
        void dump(const char *data, uint32_t len);

    private:
        unsigned m_sendId = 0;                          //!< Output packets sequence number
        unsigned m_recvId = 0;                          //!< Last received packet sequence number
        std::array<uint8_t, 4096> m_sendBuffer;         //!< Buffer used for sending
        ObjectPool<uint8_t> m_packetsPool{true};        //!< Packets pool management, used for converting DAS 1.0 -> DAS 2.0 packets
        const void *m_lastData{nullptr};                //!< Last data received
        uint32_t m_lastDataLen{0};                      //!< Length of last data received

        /**
         * Process data from buffer and dispatch it to the registered plugins.
         *
         * Monitor buffer for new data. When it's available, transform it into
         * list of packets and send the list to the registered plugins. Wait for all
         * plugins to complete processing it and than advance buffer consumer
         * index for the amount of bytes processed. Start monitoring again.
         */
        void processDataThread(epicsEvent *shutdown);

    protected:
        int BufUsed;
        int BufSize;
        int CopyRate;
        int ProcRate;
        int OldPktsEn;
        int EventsFmt;
        int DumpCmdPkts;
        int CntDropPkts;
};

#endif // BASE_PORT_PLUGIN_H
