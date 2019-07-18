/* OccPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef OCC_PLUGIN_H
#define OCC_PLUGIN_H

#include "BasePortPlugin.h"

#include <occlib.h>

/**
 * OCC interface and top level data dispatcher
 *
 * An OccPlugin instance connects to OCC device for data exchange and status
 * querying. Incoming data (data received from OCC device) is distributed to
 * all registered plugins. Plugins receive pointer to read-only DMA memory and
 * it's their responsibility to process all data received. Optionally
 * OccPlugin can be enabled to copy data to a bigger buffer in application
 * memory space. In such case copying is done in dedicated thread. Outgoing data
 * sent by plugins is forwarded intact to the OCC device.
 *
 * Another thread is created to periodically poll OCC status with two user
 * configurable intervals (basic vs extended status).
 */
class epicsShareFunc OccPlugin : public BasePortPlugin {
    private:
        /**
         * Valid statuses of the OccPlugin and the OCC infrastructure.
         */
        enum Status {
            STAT_OK             = 0,    //!< No error
            STAT_RESETTING      = 1,    //!< Resetting OCC and internal OccPlugin state
            STAT_BUFFER_FULL    = 2,    //!< Copy buffer full
            STAT_BAD_DATA       = 3,    //!< Bad or corrupted data detected in queue
            STAT_PARTIAL_DATA   = 4,    //!< Partial data received, no more data afterwards
            STAT_OCC_NOT_INIT   = 11,   //!< OCC device not initialized, check LastErr for details
            STAT_OCC_ERROR      = 12,   //!< OCC error, check logs
            STAT_OCC_STALL      = 13,   //!< OCC DMA stalled
            STAT_OCC_FIFO_FULL  = 14,   //!< OCC FIFO overrun
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
         * @param[in] sourceId number gets added to some packets.
         */
        OccPlugin(const char *portName, const char *devfile, uint32_t localBufferSize);

        /**
         * Destructor
         */
        ~OccPlugin();

    private:
        int m_version;

        struct occ_handle *m_occ = nullptr;
        occ_status_t m_occStatusCache;
        std::unique_ptr<Thread> m_occStatusRefreshThread;
        epicsEvent m_statusEvent;
        static const int DEFAULT_BASIC_STATUS_INTERVAL;
        static const int DEFAULT_EXTENDED_STATUS_INTERVAL;

        /**
         * Overloaded method.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Send data to OCC.
         */
        bool send(const uint8_t *data, size_t len);

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

    private:
        int Status;
        int Reset;
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
        int RecvRate;
        int RxEn;
        int RxEnRb;
        int ErrPktEn;
        int ErrPktEnRb;
        int ReportFile;
};

#endif // OCC_PLUGIN_H
