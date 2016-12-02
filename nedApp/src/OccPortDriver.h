/* OccPortDriver.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef OCCPORTDRIVER_H
#define OCCPORTDRIVER_H

#include "BaseCircularBuffer.h"
#include "DasPacketList.h"
#include "Thread.h"

#include <asynPortDriver.h>
#include <epicsThread.h>

#include <occlib.h>

/**
 * OCC interface and top level data dispatcher
 *
 * An OccPortDriver instance connects to OCC device for data exchange and status
 * querying. Incoming data (data received from OCC device) is distributed to
 * all registered plugins. Plugins receive pointer to read-only DMA memory and
 * it's their responsibility to process all data received. Optionally
 * OccPortDriver can be enabled to copy data to a bigger buffer in application
 * memory space. In such case copying is done in dedicated thread. Outgoing data
 * sent by plugins is forwarded intact to the OCC device.
 *
 * Another thread is created to periodically poll OCC status with two user
 * configurable intervals (basic vs extended status).
 */
class epicsShareFunc OccPortDriver : public asynPortDriver {
    private:
        /**
         * Valid statuses of the OccPortDriver and the OCC infrastructure.
         */
        enum Status {
            STAT_OK             = 0,    //!< No error
            STAT_RESETTING      = 1,    //!< Resetting OCC and internal OccPortDriver state
            STAT_BUFFER_FULL    = 2,    //!< Copy buffer full
            STAT_BAD_DATA       = 3,    //!< Bad or corrupted data detected in queue
            STAT_PARTIAL_DATA   = 4,    //!< Partial data received, no more data afterwards
            STAT_OCC_NOT_INIT   = 11,   //!< OCC device not initialized, check LastErr for details
            STAT_OCC_ERROR      = 12,   //!< OCC error, check logs
            STAT_OCC_STALL      = 13,   //!< OCC DMA stalled
            STAT_OCC_FIFO_FULL  = 14,   //!< OCC FIFO overrun
        };

        /**
         * Recognized command values through Command parameter.
         */
        enum Command {
            CMD_NONE            = 0,    //!< No action
            CMD_RESET           = 1,    //!< Reset OCC device and internal state
        };

    public:
        /**
         * Constructor
         *
         * @param[in] portName Name of the asyn port to which plugins can connect
         * @param[in] devfile Absolute path to OCC device file
         * @param[in] localBufferSize If not zero, a local buffer will be created
         *            where all data from OCC DMA buffer will be copied to as soon
         *            as it is available.
         */
        OccPortDriver(const char *portName, const char *devfile, uint32_t localBufferSize);

        /**
         * Destructor
         */
        ~OccPortDriver();

    private:
        int m_version;
        int m_test;

        struct occ_handle *m_occ;
        occ_status_t m_occStatusCache;
        BaseCircularBuffer *m_circularBuffer;
        Thread *m_occBufferReadThread;
        Thread *m_occStatusRefreshThread;
        epicsEvent m_statusEvent;
        static const int DEFAULT_BASIC_STATUS_INTERVAL;
        static const int DEFAULT_EXTENDED_STATUS_INTERVAL;

        /**
         * Send list of packets to the plugins.
         *
         * @param messageType Message type to which plugins are registered to receive.
         * @param packetList List of packets received from OCC.
         */
        void sendToPlugins(int messageType, const DasPacketList *packetList);

        /**
         * Overloaded method.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded method to read asyn int32 parameters
         */
        asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);

        /**
         * Overloaded method.
         */
        asynStatus writeGenericPointer(asynUser *pasynUser, void *pointer);

        /**
         * Helper function to create output asynPortDriver param with default value.
         */
        asynStatus createParam(const char *name, asynParamType type, int *index, int defaultValue);
        using asynPortDriver::createParam;

        /**
         * Report an error detected in receive data thread
         */
        void handleRecvError(int ret);

        /**
         * Reset OCC device and all internal states, including restarting read threads.
         *
         * This function must not be called during global lock.
         */
        void reset();

        /**
         * Process data from OCC buffer and dispatch it to the registered plugins.
         *
         * Monitor OCC buffer for new data. When it's available, transform it into
         * list of packets and send the list to the registered plugins. Wait for all
         * plugins to complete processing it and than advance OCC buffer consumer
         * index for the amount of bytes processed. Start monitoring again.
         */
        void processOccDataThread(epicsEvent *shutdown);

        /**
         * Thread refreshing OCC status periodically.
         *
         * Querying for OCC status can block for some time and could interfere with
         * other driver functionality. For this reason it must be in its own thread.
         *
         * The thread is driven by StatusInt and ExtStatusInt parameters. When StatusInt
         * expires, it queries for OCC status and updates all the parameters. If the
         * ExtStatusInt also expired, it will also query for parameters which take longer
         * to update but are not required to refresh frequently.
         */
        void refreshOccStatusThread(epicsEvent *shutdown);

        /**
         * Retrieve OCC status and update relevant PVs.
         *
         * Full OCC status is slow and may take time in order of ms. When set,
         * basic_status parameter forces fast OCC status. Only PVs that change
         * for a given mode are updated.
         */
        void refreshOccStatus(bool basic_status);

        /**
         * Dump raw data to log in hex format
         *
         * @param[in] data to be dumped
         * @param[in] length of data in bytes
         */
        void dump(const char *data, uint32_t length);

    private:
        #define FIRST_OCCPORTDRIVER_PARAM Status
        int Status;
        int Command;
        int LastErr;
        int HwType;
        int HwVer;
        int FwVer;
        int FwDate;
        int ConStatus;
        int RxStalled;
        int FpgaSn;
        int FpgaTemp;
        int FpgaCoreV;
        int FpgaAuxV;
        int ErrCrc;
        int ErrLength;
        int ErrFrame;
        int SfpSerNo;
        int SfpPartNo;
        int SfpType;
        int SfpTemp;
        int SfpRxPower;
        int SfpTxPower;
        int SfpVccPow;
        int SfpTxBiasC;
        int StatusInt;
        int ExtStatInt;
        int DmaUsed;
        int DmaSize;
        int BufUsed;
        int BufSize;
        int RecvRate;
        int CopyRate;
        int ProcRate;
        int RxEn;
        int RxEnRb;
        int ErrPktEn;
        int ErrPktEnRb;
        int MaxPktSize;
        #define LAST_OCCPORTDRIVER_PARAM MaxPktSize
};

#endif // OCCPORTDRIVER_H
