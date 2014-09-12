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

EPICS_REGISTER(Occ, OccPortDriver, 2, "Port name", string, "Local buffer size", int);

const float OccPortDriver::DEFAULT_BASIC_STATUS_INTERVAL = 5.0;     //!< How often to update frequent OCC status parameters
const float OccPortDriver::DEFAULT_EXTENDED_STATUS_INTERVAL = 60.0; //!< How ofter to update less frequently changing OCC status parameters

extern "C" {
    static void processOccDataC(void *drvPvt)
    {
        OccPortDriver *driver = reinterpret_cast<OccPortDriver *>(drvPvt);
        driver->processOccDataThread();
    }
    static void refreshOccStatusC(void *drvPvt)
    {
        OccPortDriver *driver = reinterpret_cast<OccPortDriver *>(drvPvt);
        driver->refreshOccStatusThread();
    }
}

OccPortDriver::OccPortDriver(const char *portName, uint32_t localBufferSize)
	: asynPortDriver(portName, asynMaxAddr, NUM_OCCPORTDRIVER_PARAMS, asynInterfaceMask,
	                 asynInterruptMask, asynFlags, asynAutoConnect, asynPriority, asynStackSize)
    , m_occ(NULL)
    , m_occBufferReadThreadId(0)
{
    int status;

    // Register params with asyn
    createParam("Status",           asynParamInt32,     &Status);
    createParam("LastErr",          asynParamInt32,     &LastErr);
    createParam("BoardType",        asynParamInt32,     &BoardType);
    createParam("BoardFwVer",       asynParamInt32,     &BoardFwVer);
    createParam("BoardFwDate",      asynParamInt32,     &BoardFwDate);
    createParam("OpticsPresent",    asynParamInt32,     &OpticsPresent);
    createParam("OpticsEnabled",    asynParamInt32,     &OpticsEnabled);
    createParam("RxStalled",        asynParamInt32,     &RxStalled);
    createParam("ErrPktsEnabled",   asynParamInt32,     &ErrPktsEnabled);
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
    createParam("StatusInt",        asynParamFloat64,   &StatusInt);
    createParam("ExtStatusInt",     asynParamFloat64,   &ExtStatusInt);
    createParam("DmaBufUsed",       asynParamInt32,     &DmaBufUsed);
    createParam("DmaBufSize",       asynParamInt32,     &DmaBufSize);
    createParam("CopyBufUsed",      asynParamInt32,     &CopyBufUsed);
    createParam("CopyBufSize",      asynParamInt32,     &CopyBufSize);
    createParam("DataRateOut",      asynParamInt32,     &DataRateOut);

    setIntegerParam(Status, STAT_OK);
    setDoubleParam(StatusInt, DEFAULT_BASIC_STATUS_INTERVAL);
    setDoubleParam(ExtStatusInt, DEFAULT_EXTENDED_STATUS_INTERVAL);
    setIntegerParam(DataRateOut, 0);
    setIntegerParam(RxStalled, STALL_NONE);

    // Initialize OCC board
    status = occ_open(portName, OCC_INTERFACE_OPTICAL, &m_occ);
    setIntegerParam(LastErr, -status);
    if (status != 0) {
        setIntegerParam(Status, STAT_OCC_ERROR);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to open OCC device - %s(%d)\n", strerror(-status), status);
        m_occ = NULL;
        return;
    }

    // Start DMA copy thread or use DMA buffer directly
    if (localBufferSize > 0)
        m_circularBuffer = new DmaCopier(m_occ, localBufferSize);
    else
        m_circularBuffer = new DmaCircularBuffer(m_occ);
    if (!m_circularBuffer) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to create circular buffer handler\n");
        return;
    }

    callParamCallbacks();

    m_occStatusRefreshThreadId = epicsThreadCreate("OCC status",
                                                   epicsThreadPriorityLow,
                                                   epicsThreadGetStackSize(epicsThreadStackSmall),
                                                   (EPICSTHREADFUNC)refreshOccStatusC,
                                                   this);

    m_occBufferReadThreadId = epicsThreadCreate(portName,
                                                epicsThreadPriorityHigh,
                                                epicsThreadGetStackSize(epicsThreadStackMedium),
                                                (EPICSTHREADFUNC)processOccDataC,
                                                this);
}

OccPortDriver::~OccPortDriver()
{
    // Kick threads out of waiting for OCC
    occ_reset(m_occ);

    delete m_circularBuffer;

    if (m_occ) {
        int status = occ_close(m_occ);
        if (status != 0) {
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to close OCC device - %s(%d)\n", strerror(-status), status);
        }
    }
}

void OccPortDriver::refreshOccStatusThread()
{
    int ret;
    epicsTimeStamp lastExtStatusUpdate = { 0, 0 };
    occ_status_t occstatus; // Keep at function scope so that it caches values between runs
    bool basic_status = true;
    bool first_run = true;

    epicsTimeGetCurrent(&lastExtStatusUpdate);
    while (true) {
        double refreshPeriod;

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
            setIntegerParam(OpticsEnabled,  occstatus.rx_enabled);
            setIntegerParam(ErrPktsEnabled, occstatus.err_packets_enabled);
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
            if (getDoubleParam(StatusInt, &refreshPeriod) != asynSuccess)
                refreshPeriod = DEFAULT_BASIC_STATUS_INTERVAL;
            else if (refreshPeriod < 1.0) // prevent querying to often
                refreshPeriod = 1.0;
        } else {
            // First run is completed and PINIed PVs are initialized and happy,
            // execute second round with extended params right away
            first_run = false;
            refreshPeriod = 0.1;
            lastExtStatusUpdate = { 0, 0 };
        }

        this->unlock();

        if (m_statusEvent.wait(refreshPeriod) == false) {
            // Timer expired, check if we need to query extended status next round
            epicsTimeStamp now;
            epicsTimeGetCurrent(&now);

            this->lock();
            if (getDoubleParam(ExtStatusInt, &refreshPeriod) != asynSuccess)
                refreshPeriod = DEFAULT_EXTENDED_STATUS_INTERVAL;
            if (refreshPeriod < 1.0)
                refreshPeriod = 1.0;
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
    if (pasynUser->reason == Command) {
        switch (value) {
        case CMD_OPTICS_ENABLE:
        case CMD_OPTICS_DISABLE:
            ret = occ_enable_rx(m_occ, value == CMD_OPTICS_ENABLE);
            if (ret != 0) {
                LOG_ERROR("Unable to %s optical link - %s(%d)", (value == CMD_OPTICS_ENABLE ? "enable" : "disable"), strerror(-ret), ret);
                return asynError;
            }
            // There's a thread to refresh OCC status, including RX enabled
            m_statusEvent.signal();
            return asynSuccess;
        case CMD_ERROR_PACKETS_ENABLE:
        case CMD_ERROR_PACKETS_DISABLE:
            ret = occ_enable_error_packets(m_occ, value == CMD_ERROR_PACKETS_ENABLE);
            if (ret != 0) {
                LOG_ERROR("Unable to %s error packets - %s(%d)", (value == CMD_ERROR_PACKETS_ENABLE ? "enable" : "disable"), strerror(-ret), ret);
                return asynError;
            }
            // There's a thread to refresh OCC status, including error packets enabled
            m_statusEvent.signal();
            return asynSuccess;
        default:
            LOG_ERROR("Unrecognized command '%d'", value);
            return asynError;
        }
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

asynStatus OccPortDriver::writeGenericPointer(asynUser *pasynUser, void *pointer)
{
    if (pasynUser->reason == REASON_OCCDATA) {
        DasPacketList *packets = reinterpret_cast<DasPacketList *>(pointer);
        const DasPacket *packet = packets->first();

        int ret = occ_send(m_occ, reinterpret_cast<const void *>(packet), packet->length());
        if (ret != 0) {
            setIntegerParam(LastErr, -ret);
            setIntegerParam(Status, STAT_OCC_ERROR);
            callParamCallbacks();
            LOG_ERROR("Unable to send data to OCC - %s(%d)\n", strerror(-ret), ret);
            return asynError;
        }

    }
    return asynSuccess;
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

void OccPortDriver::handleRecvError(int ret)
{
    this->lock();
    setIntegerParam(DataRateOut, 0);

    if (ret == -EBADMSG) {
        setIntegerParam(Status, STAT_BAD_DATA);
    } else {
        setIntegerParam(LastErr, -ret);
        if (ret == -EOVERFLOW) { // DMA buffer full or FIFO overflow => OCC error
            m_statusEvent.signal();
            setIntegerParam(Status, STAT_BUFFER_FULL);
        } else if (ret == -ENOSPC) { // Local circular buffer is full
            setIntegerParam(Status, STAT_BUFFER_FULL);
            setIntegerParam(RxStalled, STALL_COPY);
        } else {
            setIntegerParam(Status, STAT_OCC_ERROR);
        }
    }
    callParamCallbacks();
    this->unlock();
}

void OccPortDriver::processOccDataThread()
{
    void *data;
    uint32_t length;
    uint32_t consumed;
    bool resetErrorRatelimit = false;
    DasPacketList packetsList;

    // Initialize members used in helper function calculateDataRateOut()
    epicsTimeGetCurrent(&m_dataRateOutTime);
    m_dataRateOutCount = 0;

    while (true) {
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

        if (!packetsList.reset(reinterpret_cast<uint8_t*>(data), length)) {
            // This should not happen. If it does it's certainly a code error that needs to be fixed.
            if (!resetErrorRatelimit) {
                LOG_ERROR("PluginDriver:%s ERROR failed to reset DasPacketList\n", __func__);
                resetErrorRatelimit = true;
            }
            continue;
        }
        resetErrorRatelimit = false;

        // Notify everybody about new data
        sendToPlugins(REASON_OCCDATA, &packetsList);

        // Plugins have been notified, hopefully they're all non-blocking.
        // While waiting, calculate how much data can be consumed from circular buffer.
        for (const DasPacket *packet = packetsList.first(); packet != 0; packet = packetsList.next(packet)) {
#ifdef DWORD_PADDING_WORKAROUND
            consumed += packet->getAlignedLength();
#else
            consumed += packet->length();
#endif
        }

        // Calculate data processing throughput every second
        calculateDataRateOut(consumed);

        // Decrease reference counter and wait for everybody else to do the same
        packetsList.release(); // reset() set it to 1
        packetsList.waitAllReleased();

        // Nobody is using data anymore
        m_circularBuffer->consume(consumed);

        // Corrupted data check
        if (consumed == 0 && length > DasPacket::MinLength) {
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
