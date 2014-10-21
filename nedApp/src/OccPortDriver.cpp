/* OccPortDriver.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "OccPortDriver.h"
#include "DmaCircularBuffer.h"
#include "DmaCopier.h"
#include "EpicsRegister.h"
#include "BasePlugin.h"
#include "Log.h"

#include <cstring> // strerror
#include <cstddef>
#include <errno.h>
#include <occlib.h>

#include "DspPlugin.h"

static const int asynMaxAddr       = 1;
static const int asynInterfaceMask = asynInt32Mask | asynOctetMask | asynGenericPointerMask | asynDrvUserMask | asynFloat64Mask; // don't remove DrvUserMask or you'll break callback's reasons
static const int asynInterruptMask = asynInt32Mask | asynOctetMask | asynGenericPointerMask | asynFloat64Mask;
static const int asynFlags         = 0;
static const int asynAutoConnect   = 1;
static const int asynPriority      = 0;
static const int asynStackSize     = 0;

#define NUM_OCCPORTDRIVER_PARAMS ((int)(&LAST_OCCPORTDRIVER_PARAM - &FIRST_OCCPORTDRIVER_PARAM + 1))

EPICS_REGISTER(Occ, OccPortDriver, 3, "Port name", string, "OCC device file", string, "Local buffer size", int);

const int OccPortDriver::DEFAULT_BASIC_STATUS_INTERVAL = 5;     //!< How often to update frequent OCC status parameters
const int OccPortDriver::DEFAULT_EXTENDED_STATUS_INTERVAL = 60; //!< How ofter to update less frequently changing OCC status parameters

OccPortDriver::OccPortDriver(const char *portName, const char *devfile, uint32_t localBufferSize)
    : asynPortDriver(portName, asynMaxAddr, NUM_OCCPORTDRIVER_PARAMS, asynInterfaceMask,
                     asynInterruptMask, asynFlags, asynAutoConnect, asynPriority, asynStackSize)
    , m_occ(NULL)
{
    int status;

    // Register params with asyn
    createParam("Status",           asynParamInt32,     &Status,            STAT_OK);
    createParam("LastErr",          asynParamInt32,     &LastErr,           0);
    createParam("BoardType",        asynParamInt32,     &BoardType);
    createParam("BoardFwVer",       asynParamInt32,     &BoardFwVer);
    createParam("BoardFwDate",      asynParamInt32,     &BoardFwDate);
    createParam("OpticsPresent",    asynParamInt32,     &OpticsPresent);
    createParam("RxStalled",        asynParamInt32,     &RxStalled,         STALL_NONE);
    createParam("Command",          asynParamInt32,     &Command);
    createParam("FpgaTemp",         asynParamFloat64,   &FpgaTemp);
    createParam("FpgaCoreVolt",     asynParamFloat64,   &FpgaCoreVolt);
    createParam("FpgaAuxVolt",      asynParamFloat64,   &FpgaAuxVolt);
    createParam("ErrCrc",           asynParamInt32,     &ErrCrc);
    createParam("ErrLength",        asynParamInt32,     &ErrLength);
    createParam("ErrFrame",         asynParamInt32,     &ErrFrame);
    createParam("SfpTemp",          asynParamFloat64,   &SfpTemp);
    createParam("SfpRxPower",       asynParamFloat64,   &SfpRxPower);
    createParam("SfpTxPower",       asynParamFloat64,   &SfpTxPower);
    createParam("SfpVccPower",      asynParamFloat64,   &SfpVccPower);
    createParam("SfpTxBiasCur",     asynParamFloat64,   &SfpTxBiasCur);
    createParam("StatusInt",        asynParamInt32,     &StatusInt,         DEFAULT_BASIC_STATUS_INTERVAL);
    createParam("ExtStatusInt",     asynParamInt32,     &ExtStatusInt,      DEFAULT_EXTENDED_STATUS_INTERVAL);
    createParam("DmaBufUsed",       asynParamInt32,     &DmaBufUsed);
    createParam("DmaBufSize",       asynParamInt32,     &DmaBufSize);
    createParam("CopyBufUsed",      asynParamInt32,     &CopyBufUsed);
    createParam("CopyBufSize",      asynParamInt32,     &CopyBufSize);
    createParam("DataRateOut",      asynParamInt32,     &DataRateOut);
    createParam("RxEn",             asynParamInt32,     &RxEn);
    createParam("RxEnRbv",          asynParamInt32,     &RxEnRbv);
    createParam("ErrPktEn",         asynParamInt32,     &ErrPktEn);
    createParam("ErrPktEnRbv",      asynParamInt32,     &ErrPktEnRbv);
    createParam("AutoReset",        asynParamInt32,     &AutoReset,         0);
    createParam("RstCntBad",        asynParamInt32,     &RstCntBad,         0);
    createParam("RstCntOvrflw",     asynParamInt32,     &RstCntOvrflw,      0);
    createParam("RstCntDma",        asynParamInt32,     &RstCntDma,         0);
    createParam("RstCntCopy",       asynParamInt32,     &RstCntCopy,        0);
    createParam("RstCntErr",        asynParamInt32,     &RstCntErr,         0);

    // Initialize OCC board
    status = occ_open(devfile, OCC_INTERFACE_OPTICAL, &m_occ);
    setIntegerParam(LastErr, -status);
    if (status != 0) {
        setIntegerParam(Status, STAT_OCC_NOT_INIT);
        setIntegerParam(LastErr, -status);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to open OCC device - %s(%d)\n", strerror(-status), status);
        m_occ = NULL;
    }

    callParamCallbacks();

    if (m_occ != NULL) {
        // Start DMA copy thread or use DMA buffer directly
        if (localBufferSize > 0)
            m_circularBuffer = new DmaCopier(m_occ, localBufferSize);
        else
            m_circularBuffer = new DmaCircularBuffer(m_occ);
        if (!m_circularBuffer) {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to create circular buffer handler\n");
            return;
        }

        m_occBufferReadThread = new Thread(
            "Process incoming data",
            std::bind(&OccPortDriver::processOccDataThread, this, std::placeholders::_1),
            epicsThreadGetStackSize(epicsThreadStackMedium),
            epicsThreadPriorityHigh
        );
        m_occBufferReadThread->start();
        m_occStatusRefreshThread = new Thread(
            "OCC status",
            std::bind(&OccPortDriver::refreshOccStatusThread, this, std::placeholders::_1),
            epicsThreadGetStackSize(epicsThreadStackSmall),
            epicsThreadPriorityLow
        );
        m_occStatusRefreshThread->start();
    }
}

OccPortDriver::~OccPortDriver()
{
    m_occBufferReadThread->stop();
    m_occStatusRefreshThread->stop();

    delete m_occBufferReadThread;
    delete m_occStatusRefreshThread;
    delete m_circularBuffer;

    if (m_occ) {
        int status = occ_close(m_occ);
        if (status != 0) {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to close OCC device - %s(%d)\n", strerror(-status), status);
        }
    }
}

void OccPortDriver::refreshOccStatusThread(epicsEvent *shutdown)
{
    int ret;
    epicsTimeStamp lastExtStatusUpdate = { 0, 0 };
    occ_status_t occstatus; // Keep at function scope so that it caches values between runs
    bool basic_status = true;
    bool first_run = true;

    epicsTimeGetCurrent(&lastExtStatusUpdate);
    while (shutdown->tryWait() == false) {
        int refreshPeriod;

        if (!basic_status)
            epicsTimeGetCurrent(&lastExtStatusUpdate);

        // This one can take long time to execute, don't lock the driver while it's executing
        ret = occ_status(m_occ, &occstatus, basic_status);

        this->lock();

        if (ret != 0) {
            setIntegerParam(LastErr, -ret);
            LOG_ERROR("Failed to query OCC status: %s(%d)", strerror(-ret), ret);
        } else {
            setIntegerParam(BoardType,      occstatus.board);
            setIntegerParam(BoardFwVer,     occstatus.firmware_ver);
            setIntegerParam(BoardFwDate,    occstatus.firmware_date);
            setIntegerParam(OpticsPresent,  occstatus.optical_signal);
            setIntegerParam(RxEnRbv,        occstatus.rx_enabled);
            setIntegerParam(ErrPktEnRbv,    occstatus.err_packets_enabled);
            setDoubleParam(FpgaTemp,        occstatus.fpga_temp);
            setDoubleParam(FpgaCoreVolt,    occstatus.fpga_core_volt);
            setDoubleParam(FpgaAuxVolt,     occstatus.fpga_aux_volt);
            setIntegerParam(ErrCrc,         occstatus.err_crc);
            setIntegerParam(ErrLength,      occstatus.err_length);
            setIntegerParam(ErrFrame,       occstatus.err_frame);
            setDoubleParam(SfpTemp,         occstatus.sfp_temp);
            setDoubleParam(SfpRxPower,      occstatus.sfp_rx_power);
            setDoubleParam(SfpTxPower,      occstatus.sfp_tx_power);
            setDoubleParam(SfpVccPower,     occstatus.sfp_vcc_power);
            setDoubleParam(SfpTxBiasCur,    occstatus.sfp_tx_bias_cur);

            setIntegerParam(DmaBufUsed,     occstatus.dma_used);
            setIntegerParam(DmaBufSize,     occstatus.dma_size);
            setIntegerParam(CopyBufUsed,    m_circularBuffer->used());
            setIntegerParam(CopyBufSize,    m_circularBuffer->size());

            if (occstatus.stalled)
                setIntegerParam(RxStalled,  STALL_DMA);
            else if (occstatus.overflowed)
                setIntegerParam(RxStalled,  STALL_FIFO);
        }

        callParamCallbacks();

        if (first_run == false) {
            // Set basic status as default, if signal is received before timeout
            // or not time for extended status yet
            basic_status = true;

            // Determine refresh interval
            if (getIntegerParam(StatusInt, &refreshPeriod) != asynSuccess)
                refreshPeriod = DEFAULT_BASIC_STATUS_INTERVAL;
            else if (refreshPeriod < 1) // prevent querying to often
                refreshPeriod = 1;
        } else {
            // First run is completed and PINIed PVs are initialized and happy,
            // execute second round with extended params right away
            first_run = false;
            refreshPeriod = 0;
            lastExtStatusUpdate = { 0, 0 };
        }

        this->unlock();

        if (m_statusEvent.wait(refreshPeriod) == false) {
            // Timer expired, check if we need to query extended status next round
            epicsTimeStamp now;
            epicsTimeGetCurrent(&now);

            this->lock();
            if (getIntegerParam(ExtStatusInt, &refreshPeriod) != asynSuccess)
                refreshPeriod = DEFAULT_EXTENDED_STATUS_INTERVAL;
            if (refreshPeriod < 1)
                refreshPeriod = 1;
            this->unlock();

            if (epicsTimeDiffInSeconds(&now, &lastExtStatusUpdate) >= refreshPeriod) {
                basic_status = false;
                epicsTimeGetCurrent(&lastExtStatusUpdate);
            }
        }
    }
}

asynStatus OccPortDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int ret;

    if (m_occ == NULL) {
        // OCC not initialized, nothing that we can do here
        LOG_ERROR("OCC device not initialized");
        return asynError;
    }

    if (pasynUser->reason == Command) {
        switch (value) {
        case CMD_RESET:
            this->unlock();
            reset();
            this->lock();
            return asynSuccess;
        default:
            LOG_ERROR("Unrecognized command '%d'", value);
            return asynError;
        }
    } else if (pasynUser->reason == RxEn) {
        if ((ret = occ_enable_rx(m_occ, value > 0)) != 0) {
            LOG_ERROR("Unable to %s optical link - %s(%d)", (value > 0 ? "enable" : "disable"), strerror(-ret), ret);
            setIntegerParam(LastErr, -ret);
            callParamCallbacks();
            return asynError;
        }

        setIntegerParam(RxEn, value);
        if (value == 0) {
            // RX could be switched off in the middle of the incoming packet.
            // Second half of that packet would show up in the queue next time
            // enabled. Calling occ_reset() to avoid it.
            this->unlock();
            reset();
            this->lock();
        } else {
            // There's a thread to refresh OCC status, including RX enabled
            m_statusEvent.signal();
        }
    } else if (pasynUser->reason == ErrPktEn) {
        if ((ret = occ_enable_error_packets(m_occ, value > 0)) != 0) {
            LOG_ERROR("Unable to %s error packets output - %s(%d)", (value > 0 ? "enable" : "disable"), strerror(-ret), ret);
            setIntegerParam(LastErr, -ret);
            callParamCallbacks();
            return asynError;
        }

        // There's a thread to refresh OCC status, including error packets enabled
        m_statusEvent.signal();
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

asynStatus OccPortDriver::writeGenericPointer(asynUser *pasynUser, void *pointer)
{
    if (m_occ == NULL) {
        LOG_ERROR("OCC device not initialized");
        return asynError;
    }

    if (pasynUser->reason == REASON_OCCDATA) {
        DasPacketList *packetList = reinterpret_cast<DasPacketList *>(pointer);

        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;
            int ret = occ_send(m_occ, reinterpret_cast<const void *>(packet), packet->length());
            if (ret != 0) {
                setIntegerParam(LastErr, -ret);
                setIntegerParam(Status, STAT_OCC_ERROR);
                callParamCallbacks();
                LOG_ERROR("Unable to send data to OCC - %s(%d)\n", strerror(-ret), ret);
                return asynError;
            }
        }
    }
    return asynSuccess;
}

asynStatus OccPortDriver::createParam(const char *name, asynParamType type, int *index, int defaultValue)
{
    asynStatus status = asynPortDriver::createParam(name, type, index);
    if (status == asynSuccess)
        status = setIntegerParam(*index, defaultValue);
    return status;
}

void OccPortDriver::calculateDataRateOut(uint32_t consumed)
{
    epicsTimeStamp now;
    double difftime;

    this->lock();
    m_dataRateOutCount += consumed;

    epicsTimeGetCurrent(&now);
    difftime = epicsTimeDiffInSeconds(&now, &m_dataRateOutTime);
    if (difftime > 1.0) {
        double throughput = static_cast<double>(m_dataRateOutCount) / difftime;
        m_dataRateOutTime = now;
        m_dataRateOutCount = 0;

        setIntegerParam(DataRateOut, throughput);
        callParamCallbacks();
    }
    this->unlock();
}

void OccPortDriver::reset() {
    int rxEnabled = 0;
    int errPktEnabled = 0;
    DmaCopier *dmaCopier = 0;

    try {
        dmaCopier = dynamic_cast<DmaCopier *>(m_circularBuffer);
    } catch (std::exception &e) {}

    // Flag resetting mode, status thread will recover
    this->lock();
    setIntegerParam(Status, STAT_RESETTING);
    callParamCallbacks();
    getIntegerParam(RxEn, &rxEnabled);
    getIntegerParam(ErrPktEn, &errPktEnabled);
    this->unlock();

    // Stop all threads and clear all states
    if (dmaCopier)
        dmaCopier->stop();
    m_occBufferReadThread->stop();
    m_circularBuffer->clear();

    // Don't start threads on error, they would trigger another stall/corruption
    if (occ_reset(m_occ) != 0)
        return;

    // Restore threads and other relevant params
    if (dmaCopier)
        dmaCopier->start();
    m_occBufferReadThread->start();
    if (rxEnabled > 0)
        (void)occ_enable_rx(m_occ, true);
    if (errPktEnabled)
        (void)occ_enable_error_packets(m_occ, true);

    // Refresh the status parameters
    m_statusEvent.signal();

    // Flag resetting mode, status thread will recover
    this->lock();
    setIntegerParam(Status, STAT_OK);
    callParamCallbacks();
    this->unlock();
}

void OccPortDriver::handleRecvError(int ret)
{
    int autoReset = 0;
    int resetParam = 0;

    this->lock();
    setIntegerParam(DataRateOut, 0);

    if (ret == -EBADMSG) {
        setIntegerParam(Status, STAT_BAD_DATA);
        resetParam = RstCntBad;

    } else if (ret == -EOVERFLOW) { // OCC FIFO overflow
        setIntegerParam(LastErr, EOVERFLOW);
        setIntegerParam(Status, STAT_BUFFER_FULL);
        setIntegerParam(RxStalled, STALL_FIFO);
        resetParam = RstCntOvrflw;

    } else if (ret == -ENOSPC) { // OCC DMA full
        setIntegerParam(LastErr, ENOSPC);
        setIntegerParam(Status, STAT_BUFFER_FULL);
        setIntegerParam(RxStalled, STALL_DMA);
        resetParam = RstCntDma;

    } else if (ret == -ENODATA) { // DmaCopier full
        setIntegerParam(Status, STAT_BUFFER_FULL);
        setIntegerParam(RxStalled, STALL_COPY);
        resetParam = RstCntCopy;

    } else {
        setIntegerParam(LastErr, -ret);
        setIntegerParam(Status, STAT_OCC_ERROR);
        resetParam = RstCntErr;
    }

    getIntegerParam(AutoReset, &autoReset);
    if (autoReset) {
        int value;
        getIntegerParam(resetParam, &value);
        setIntegerParam(resetParam, value+1);
    }

    callParamCallbacks();
    this->unlock();

    if (autoReset)
        reset();
}

void OccPortDriver::processOccDataThread(epicsEvent *shutdown)
{
    void *data;
    uint32_t length;
    uint32_t consumed;
    DasPacketList packetsList;

    // Initialize members used in helper function calculateDataRateOut()
    epicsTimeGetCurrent(&m_dataRateOutTime);
    m_dataRateOutCount = 0;

    while (shutdown->tryWait() == false) {
        consumed = 0;

        // Wait for data, use a timeout for data rate out calculation
        int ret = m_circularBuffer->wait(&data, &length, 1.0);
        if (ret == -ETIME) {
            calculateDataRateOut(0);
            continue;
        } else if (ret != 0) {
            handleRecvError(ret);
            LOG_ERROR("Unable to receive data from OCC, stopped - %s(%d)\n", strerror(-ret), ret);
            break;
        }

        consumed = packetsList.reset(reinterpret_cast<uint8_t*>(data), length);

        if (packetsList.empty() == false) {
            // Notify everybody about new data
            sendToPlugins(REASON_OCCDATA, &packetsList);

            // Plugins have been notified, hopefully they're all non-blocking.
            // While waiting for threads to synchronize, we have some time.

            // Calculate data processing throughput every second
            calculateDataRateOut(consumed);

            // Decrease reference counter and wait for everybody else to do the same
            packetsList.release(); // reset() set it to 1
            packetsList.waitAllReleased();

            // Nobody is using data anymore
            m_circularBuffer->consume(consumed);
        } else if (length > DasPacket::MinLength) { // Corrupted data check
            handleRecvError(-EBADMSG);
            LOG_ERROR("Corrupted data in queue, aborting process thread");
            break;
        }
    }
}

void OccPortDriver::sendToPlugins(int messageType, const DasPacketList *packetList)
{
    const void *addr = reinterpret_cast<const void *>(packetList);
    doCallbacksGenericPointer(const_cast<void *>(addr), messageType, 0);
}
