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

// Forward declaration of the OCC API handler
struct occ_handle;

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
 *
 * Next table lists asyn parameters provided and can be used from EPICS PV infrastructure.
 * Some naming restrictions enforced by EPICS records apply:
 * - PV name length is limited to 28 characters in total, where the static prefix
 *   is BLXXX:Det:OccX: long, leaving 13 characters to asyn param name
 * - PV comment can be 28 characters long (text in brackets may be used to describe EPICS
 *   PV valid values)
 * asyn param    | asyn param type | init val | mode | Description                   |
 * ------------- | --------------- | -------- | ---- | ------------------------------
 * Status        | asynParamInt32  | 0        | RO   | Status of OccPortDriver       (0=OK,1=buffer full,2=OCC error,3=Data corrupted)
 * Command       | asynParamInt32  | 0        | RW   | Issue OccPortDriver command   (0=no action,1=reset)
 * LastErr       | asynParamInt32  | 0        | RO   | Last error code returned by OCC API
 * BoardType     | asynParamInt32  | 0        | RO   | OCC board type                (1=SNS PCI-X,2=SNS PCIe,15=simulator)
 * BoardFwVer    | asynParamInt32  | 0        | RO   | OCC board firmware version
 * OpticsPresent | asynParamInt32  | 0        | RO   | Is optical cable present      (0=not present,1=present)
 * RxStalled     | asynParamInt32  | 0        | RO   | Incoming data stalled         (0=not stalled,1=OCC stalled,2=copy stalled,3=both stalled)
 * ErrCrc        | asynParamInt32  | 0        | RO   | Number of CRC errors detected by OCC
 * ErrLength     | asynParamInt32  | 0        | RO   | Number of length errors detected by OCC
 * ErrFrame      | asynParamInt32  | 0        | RO   | Number of frame errors detected by OCC
 * FpgaTemp      | asynParamFloat64| 0.0      | RO   | FPGA temperature in Celsius
 * FpgaCoreVolt  | asynParamFloat64| 0.0      | RO   | FPGA core voltage in Volts
 * FpgaAuxVolt   | asynParamFloat64| 0.0      | RO   | FPGA aux voltage in Volts
 * SfpTemp       | asynParamFloat64| 0.0      | RO   | SFP temperature in Celsius
 * SfpRxPower    | asynParamFloat64| 0.0      | RO   | SFP RX power in uW
 * SfpTxPower    | asynParamFloat64| 0.0      | RO   | SFP TX power in uW
 * SfpVccPower   | asynParamFloat64| 0.0      | RO   | SFP VCC power in Volts
 * SfpTxBiasCur  | asynParamFloat64| 0.0      | RO   | SFP TX bias current in uA
 * StatusInt     | asynParamInt32  | 5        | RW   | OCC status refresh interval in s
 * ExtStatusInt  | asynParamInt32  | 60       | RW   | OCC extended status refresh interval in s
 * DmaBufUsed    | asynParamInt32  | 0        | RO   | DMA memory used space
 * DmaBufSize    | asynParamInt32  | 0        | RO   | DMA memory size
 * CopyBufUsed   | asynParamInt32  | 0        | RO   | Virtual buffer used space
 * CopyBufSize   | asynParamInt32  | 0        | RO   | Virtual buffer size
 * DataRateOut   | asynParamInt32  | 0        | RO   | Data processing throughput in B/s
 * RxEn          | asynParamInt32  | 0        | RW   | Enable incoming data          (0=disable,1=enable)
 * RxEnRbv       | asynParamInt32  | 0        | RO   | Incoming data enabled         (0=disabled,1=enabled)
 * ErrPktEn      | asynParamInt32  | 0        | RW   | Error packets output switch   (0=disable,1=enable)
 * ErrPktEnRbv   | asynParamInt32  | 0        | RO   | Error packets enabled         (0=disabled,1=enabled)
 * AutoReset     | asynParamInt32  | 0        | RW   | Auto reset on error switch    (0=disable,1=enable)
 * RstCntBad     | asynParamInt32  | 0        | RO   | Num corrupted queue auto-resets
 * RstCntOvrflw  | asynParamInt32  | 0        | RO   | Num FIFO overflow auto-resets
 * RstCntDma     | asynParamInt32  | 0        | RO   | Num DMA full auto-resets
 * RstCndCopy    | asynParamInt32  | 0        | RO   | Num buffer full auto-resets
 * RstCntErr     | asynParamInt32  | 0        | RO   | Num OCC error auto-resets
 */
class epicsShareFunc OccPortDriver : public asynPortDriver {
    private:
        /**
         * Valid statuses of the OccPortDriver and the OCC infrastructure.
         */
        enum {
            STAT_OK             = 0,    //!< No error
            STAT_BUFFER_FULL    = 1,    //!< Receive buffer is full, acquisition was stopped
            STAT_OCC_ERROR      = 2,    //!< OCC error was detected
            STAT_BAD_DATA       = 3,    //!< Bad or corrupted data detected in queue
            STAT_RESETTING      = 4,    //!< Resetting OCC and internal OccPortDriver state
            STAT_OCC_NOT_INIT   = 5,    //!< OCC device not initialized, check LastErr for details
        };

        /**
         * Stall events used to create a bitmask
         */
        enum {
            STALL_NONE          = 0,    //!< Not stalled
            STALL_DMA           = 1,    //!< DMA buffer stalled
            STALL_FIFO          = 2,    //!< Internal OCC FIFO overflowed
            STALL_COPY          = 3,    //!< Copy buffer stalled
        };

        /**
         * Recognized command values through Command parameter.
         */
        enum {
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
        BaseCircularBuffer *m_circularBuffer;
        Thread *m_occBufferReadThread;
        Thread *m_occStatusRefreshThread;
        epicsEvent m_statusEvent;
        epicsTimeStamp m_dataRateOutTime;                       //!< Used to track time since last DataRateOut parameter calculation, private to calculateDataRateOut() function
        uint32_t m_dataRateOutCount;                            //!< Used to track number of bytes since last DataRateOut parameter calculation, private to calculateDataRateOut() function
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
         * Overloaded method.
         */
        asynStatus writeGenericPointer(asynUser *pasynUser, void *pointer);

        /**
         * Helper function to create output asynPortDriver param with default value.
         */
        asynStatus createParam(const char *name, asynParamType type, int *index, int defaultValue);
        using asynPortDriver::createParam;

        /**
         * Calculate data processing rate
         */
        void calculateDataRateOut(uint32_t consumed);

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

    private:
        #define FIRST_OCCPORTDRIVER_PARAM Status
        int Status;
        int Command;
        int LastErr;
        int BoardType;
        int BoardFwVer;
        int BoardFwDate;
        int OpticsPresent;
        int RxStalled;
        int FpgaTemp;
        int FpgaCoreVolt;
        int FpgaAuxVolt;
        int ErrCrc;
        int ErrLength;
        int ErrFrame;
        int SfpTemp;
        int SfpRxPower;
        int SfpTxPower;
        int SfpVccPower;
        int SfpTxBiasCur;
        int StatusInt;
        int ExtStatusInt;
        int DmaBufUsed;
        int DmaBufSize;
        int CopyBufUsed;
        int CopyBufSize;
        int DataRateOut;
        int RxEn;
        int RxEnRbv;
        int ErrPktEn;
        int ErrPktEnRbv;
        int AutoReset;
        int RstCntBad;
        int RstCntOvrflw;
        int RstCntDma;
        int RstCntCopy;
        int RstCntErr;
        #define LAST_OCCPORTDRIVER_PARAM RstCntErr
};

#endif // OCCPORTDRIVER_H
