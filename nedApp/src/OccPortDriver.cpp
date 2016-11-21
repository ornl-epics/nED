/* OccPortDriver.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "OccPortDriver.h"
#include "DmaCircularBuffer.h"
#include "DmaCopier.h"
#include "EpicsRegister.h"
#include "BasePlugin.h"
#include "Log.h"

#include <algorithm> // std::min
#include <cstring> // strerror
#include <cstddef>
#include <errno.h>
#include <occlib.h>
#include <math.h>

#include "DspPlugin.h"

static const int asynMaxAddr       = 1;
static const int asynInterfaceMask = asynInt32Mask | asynOctetMask | asynGenericPointerMask | asynDrvUserMask | asynFloat64Mask; // don't remove DrvUserMask or you'll break callback's reasons
static const int asynInterruptMask = asynInt32Mask | asynOctetMask | asynGenericPointerMask | asynFloat64Mask;
static const int asynFlags         = 0;
static const int asynAutoConnect   = 1;
static const int asynPriority      = 0;
static const int asynStackSize     = 0;

#define NUM_OCCPORTDRIVER_PARAMS ((int)(&LAST_OCCPORTDRIVER_PARAM - &FIRST_OCCPORTDRIVER_PARAM + 1))

EPICS_REGISTER(Occ, OccPortDriver, 3, "Port name", string, "OCC connection string", string, "Local buffer size", int);

OccPortDriver::OccPortDriver(const char *portName, const char *devfile, uint32_t localBufferSize)
    : asynPortDriver(portName, asynMaxAddr, NUM_OCCPORTDRIVER_PARAMS, asynInterfaceMask,
                     asynInterruptMask, asynFlags, asynAutoConnect, asynPriority, asynStackSize)
    , m_occ(NULL)
{
    int status;

    // Register params with asyn
    //           0123456789
    createParam("Status",           asynParamInt32,     &Status,        STAT_OK);   // READ - Status of OccPortDriver       (see OccPortDriver::Status)
    createParam("LastErr",          asynParamInt32,     &LastErr,       0);         // READ - Last error code returned by OCC API
    createParam("HwType",           asynParamInt32,     &HwType);                   // READ - OCC board type                (1=SNS PCI-X,2=SNS PCIe,15=simulator)
    createParam("HwVer",            asynParamInt32,     &HwVer);                    // READ - OCC board hardware version
    createParam("FwVer",            asynParamInt32,     &FwVer);                    // READ - OCC board firmware version
    createParam("FwDate",           asynParamInt32,     &FwDate);                   // READ - OCC board firmware date
    createParam("ConStatus",        asynParamInt32,     &ConStatus);                // READ - Optical connection status     (0=connected,1=no SFP,2=no cable,3=laser fault)
    createParam("Command",          asynParamInt32,     &Command);                  // WRITE - Issue OccPortDriver command  (see OccPortDriver::Command)
    createParam("FpgaSn",           asynParamOctet,     &FpgaSn);                   // READ - FPGA serial number in hex str
    createParam("FpgaTemp",         asynParamFloat64,   &FpgaTemp);                 // READ - FPGA temperature in Celsius
    createParam("FpgaCoreV",        asynParamFloat64,   &FpgaCoreV);                // READ - FPGA core voltage in Volts
    createParam("FpgaAuxV",         asynParamFloat64,   &FpgaAuxV);                 // READ - FPGA aux voltage in Volts
    createParam("ErrCrc",           asynParamInt32,     &ErrCrc);                   // READ - Number of CRC errors detected by OCC
    createParam("ErrLength",        asynParamInt32,     &ErrLength);                // READ - Number of length errors detected by OCC
    createParam("ErrFrame",         asynParamInt32,     &ErrFrame);                 // READ - Number of frame errors detected by OCC
    createParam("SfpSerNo",         asynParamOctet,     &SfpSerNo);                 // READ - SFP serial number string
    createParam("SfpPartNo",        asynParamOctet,     &SfpPartNo);                // READ - SFP part number string
    createParam("SfpType",          asynParamInt32,     &SfpType);                  // READ - SFP type
    createParam("SfpTemp",          asynParamFloat64,   &SfpTemp);                  // READ - SFP temperature in Celsius
    createParam("SfpRxPower",       asynParamFloat64,   &SfpRxPower);               // READ - SFP RX power in uW
    createParam("SfpTxPower",       asynParamFloat64,   &SfpTxPower);               // READ - SFP TX power in uW
    createParam("SfpVccPow",        asynParamFloat64,   &SfpVccPow);                // READ - SFP VCC power in Volts
    createParam("SfpTxBiasC",       asynParamFloat64,   &SfpTxBiasC);               // READ - SFP TX bias current in uA
    createParam("StatusInt",        asynParamInt32,     &StatusInt,     5);         // WRITE - OCC status refresh interval in s
    createParam("ExtStatInt",       asynParamInt32,     &ExtStatInt,    60);        // WRITE - OCC extended status refresh interval in s
    createParam("DmaUsed",          asynParamInt32,     &DmaUsed);                  // READ - DMA memory used space
    createParam("DmaSize",          asynParamInt32,     &DmaSize);                  // READ - DMA memory size
    createParam("BufUsed",          asynParamInt32,     &BufUsed);                  // READ - Virtual buffer used space
    createParam("BufSize",          asynParamInt32,     &BufSize);                  // READ - Virtual buffer size
    createParam("RecvRate",         asynParamInt32,     &RecvRate);                 // READ - OCC receiving throughput in B/s
    createParam("CopyRate",         asynParamInt32,     &CopyRate);                 // READ - Copy to internal buffer throughput in B/s
    createParam("ProcRate",         asynParamInt32,     &ProcRate);                 // READ - Data processing throughput in B/s
    createParam("RxEn",             asynParamInt32,     &RxEn);                     // WRITE - Enable incoming data          (0=disable,1=enable)
    createParam("RxEnRb",           asynParamInt32,     &RxEnRb);                   // READ - Incoming data enabled         (0=disabled,1=enabled)
    createParam("ErrPktEn",         asynParamInt32,     &ErrPktEn);                 // WRITE - Error packets output switch   (0=disable,1=enable)
    createParam("ErrPktEnRb",       asynParamInt32,     &ErrPktEnRb);               // READ - Error packets enabled         (0=disabled,1=enabled)
    createParam("MaxPktSize",       asynParamInt32,     &MaxPktSize, 4000);         // WRITE - Maximum size of DAS packet payload

    occ_interface_type occtype = OCC_INTERFACE_OPTICAL;
    if (strchr(devfile, ':') != 0)
        occtype = OCC_INTERFACE_SOCKET;

    // Initialize OCC library
    status = occ_open(devfile, occtype, &m_occ);
    setIntegerParam(LastErr, -status);
    if (status != 0) {
        setIntegerParam(Status, STAT_OCC_NOT_INIT);
        setIntegerParam(LastErr, -status);
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "Unable to open OCC device - %s(%d)\n", strerror(-status), status);
        m_occ = NULL;
    }

    // Initialize some parameters values
    setIntegerParam(BufSize, localBufferSize);

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

        // Do a full status check the first time before the update thread starts
        refreshOccStatus(false);

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
    epicsTimeStamp now;
    epicsTimeStamp lastExtStatusUpdate = { 0, 0 };

    epicsTimeGetCurrent(&lastExtStatusUpdate);
    while (shutdown->tryWait() == false) {
        int basicInterval;
        int extendedInterval;

        // Determine refresh intervals
        this->lock();
        getIntegerParam(StatusInt, &basicInterval);
        if (basicInterval < 1) // prevent querying to often
            basicInterval = 1;
        getIntegerParam(ExtStatInt, &extendedInterval);
        if (extendedInterval < 1) // prevent querying to often
            extendedInterval = 1;
        this->unlock();

        // If signal is caught, maybe we need to shutdown
        if (m_statusEvent.wait(basicInterval) == true)
            continue;

        // Determine whether full status should be retrieved - slow
        epicsTimeGetCurrent(&now);
        if (epicsTimeDiffInSeconds(&now, &lastExtStatusUpdate) >= extendedInterval) {
            refreshOccStatus(false);
            epicsTimeGetCurrent(&lastExtStatusUpdate);
        } else {
            refreshOccStatus(true);
        }
    }
}

void OccPortDriver::refreshOccStatus(bool basic_status)
{
    // This one can take long time to execute, don't lock the driver while it's executing
    int ret = occ_status(m_occ, &m_occStatusCache, basic_status ? OCC_STATUS_FAST : OCC_STATUS_FULL);

    this->lock();

    if (ret != 0) {
        setIntegerParam(LastErr, -ret);
        LOG_ERROR("Failed to query OCC status: %s(%d)", strerror(-ret), ret);
    } else {
        char sn[20];
        snprintf(sn, sizeof(sn), "%jX", m_occStatusCache.fpga_serial_number);
        setStringParam(FpgaSn,          sn);

        setIntegerParam(HwType,         m_occStatusCache.board);
        setIntegerParam(HwVer,          m_occStatusCache.hardware_ver);
        setIntegerParam(FwVer,          m_occStatusCache.firmware_ver);
        setIntegerParam(FwDate,         m_occStatusCache.firmware_date);
        setIntegerParam(ConStatus,      m_occStatusCache.optical_signal);
        setIntegerParam(RxEnRb,         m_occStatusCache.rx_enabled);
        setIntegerParam(ErrPktEnRb,     m_occStatusCache.err_packets_enabled);
        setDoubleParam(FpgaTemp,        m_occStatusCache.fpga_temp);
        setDoubleParam(FpgaCoreV,       m_occStatusCache.fpga_core_volt);
        setDoubleParam(FpgaAuxV,        m_occStatusCache.fpga_aux_volt);
        setIntegerParam(ErrCrc,         m_occStatusCache.err_crc);
        setIntegerParam(ErrLength,      m_occStatusCache.err_length);
        setIntegerParam(ErrFrame,       m_occStatusCache.err_frame);
        setStringParam(SfpSerNo,        m_occStatusCache.sfp_serial_number);
        setStringParam(SfpPartNo,       m_occStatusCache.sfp_part_number);
        setIntegerParam(SfpType,        m_occStatusCache.sfp_type);
        setDoubleParam(SfpTemp,         m_occStatusCache.sfp_temp);
        setDoubleParam(SfpRxPower,      m_occStatusCache.sfp_rx_power);
        setDoubleParam(SfpTxPower,      m_occStatusCache.sfp_tx_power);
        setDoubleParam(SfpVccPow,       m_occStatusCache.sfp_vcc_power);
        setDoubleParam(SfpTxBiasC,      m_occStatusCache.sfp_tx_bias_cur);

        setIntegerParam(DmaUsed,        m_occStatusCache.dma_used);
        setIntegerParam(DmaSize,        m_occStatusCache.dma_size);
        setIntegerParam(RecvRate,       m_occStatusCache.rx_rate);

        if (m_occStatusCache.stalled)
            setIntegerParam(Status,     STAT_OCC_STALL);
        else if (m_occStatusCache.overflowed)
            setIntegerParam(Status,     STAT_OCC_FIFO_FULL);
    }

    callParamCallbacks();

    this->unlock();
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

        if (value == 1) {
            // RX could be switched off in the middle of the incoming packet.
            // Second half of that packet would show up in the queue next time
            // enabled. Calling occ_reset() to avoid it.
            this->unlock();
            reset();
            this->lock();
        }

        if ((ret = occ_enable_rx(m_occ, value > 0)) != 0) {
            LOG_ERROR("Unable to %s optical link - %s(%d)", (value > 0 ? "enable" : "disable"), strerror(-ret), ret);
            setIntegerParam(LastErr, -ret);
            callParamCallbacks();
            return asynError;
        }

        setIntegerParam(RxEn, value);
        // There's a thread to refresh OCC status, including RX enabled
        m_statusEvent.signal();

    } else if (pasynUser->reason == ErrPktEn) {
        if ((ret = occ_enable_error_packets(m_occ, value > 0)) != 0) {
            LOG_ERROR("Unable to %s error packets output - %s(%d)", (value > 0 ? "enable" : "disable"), strerror(-ret), ret);
            setIntegerParam(LastErr, -ret);
            callParamCallbacks();
            return asynError;
        }

        // There's a thread to refresh OCC status, including error packets enabled
        m_statusEvent.signal();
    } else if (pasynUser->reason == MaxPktSize) {
        DasPacket::setMaxLength(value);
        return asynSuccess;
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

asynStatus OccPortDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
    if (m_occ == NULL) {
        // OCC not initialized, nothing that we can do here
        LOG_ERROR("OCC device not initialized");
        return asynError;
    }

    if (pasynUser->reason == BufUsed) {
        *value = m_circularBuffer->used();
        return asynSuccess;
    }
    if (pasynUser->reason == ProcRate) {
        *value = m_circularBuffer->getReadRate();
        return asynSuccess;
    }
    if (pasynUser->reason == CopyRate) {
        // is 0 when no copy buffer is used
        *value = m_circularBuffer->getPushRate();
        return asynSuccess;
    }
    return asynPortDriver::readInt32(pasynUser, value);
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
            int ret = occ_send(m_occ, reinterpret_cast<const void *>(packet), ALIGN_UP(packet->getLength(), 4));
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
    this->lock();

    if (ret == -EBADMSG) { // Corrupted data based on length check
        setIntegerParam(Status, STAT_BAD_DATA);

    } else if (ret == -ERANGE) { // Not enough data for packet
        setIntegerParam(Status, STAT_PARTIAL_DATA);

    } else if (ret == -EOVERFLOW) { // OCC FIFO overflow
        setIntegerParam(LastErr, EOVERFLOW);
        setIntegerParam(Status, STAT_OCC_FIFO_FULL);

    } else if (ret == -ENOSPC) { // OCC DMA full
        setIntegerParam(LastErr, ENOSPC);
        setIntegerParam(Status, STAT_OCC_STALL);

    } else if (ret == -ENODATA) { // DmaCopier full
        setIntegerParam(Status, STAT_BUFFER_FULL);

    } else {
        setIntegerParam(LastErr, -ret);
        setIntegerParam(Status, STAT_OCC_ERROR);
    }

    callParamCallbacks();
    this->unlock();
}

void OccPortDriver::dump(const char *data, uint32_t len)
{
    char buffer[64] = { 0 };
    char *ptr = buffer;
    const uint32_t maxlen = 4096;

    LOG_DEBUG("Dump %u bytes of raw data at offset %p", len, data);

    // OCC library is always 4-byte aligned, so the buffer should
    // be good for 4-byte reads even if the packet is not 4-byte
    // aligned.
    for (uint32_t i = 0; i < std::min(len, maxlen)/4; i++) {
        if ((i % 4) == 0 && i > 0) {
            LOG_DEBUG("%s", buffer);
            buffer[0] = 0;
            ptr = buffer;
        }
        snprintf(ptr, buffer + sizeof(buffer) - ptr, "0x%08X ", *(uint32_t *)(data + i*4));
        ptr += 11;
    }
    if (ptr > 0) {
        LOG_DEBUG("%s", buffer);
    }
    if (len > maxlen) {
        LOG_DEBUG("... truncated to %u bytes", maxlen);
    }
}

void OccPortDriver::processOccDataThread(epicsEvent *shutdown)
{
    void *data;
    uint32_t length;
    uint32_t consumed;
    DasPacketList packetsList;
    uint32_t retryCounter = 0;

    while (shutdown->tryWait() == false) {
        consumed = 0;

        // Wait for data, use a timeout for data rate out calculation
        int ret = m_circularBuffer->wait(&data, &length, 1.0);
        if (ret == -ETIME) {
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

            // Decrease reference counter and wait for everybody else to do the same
            packetsList.release(); // reset() set it to 1
            packetsList.waitAllReleased();

            // Nobody is using data anymore
            m_circularBuffer->consume(consumed);

            retryCounter = 0;

        } else {
            packetsList.release();

            if (retryCounter < 7) {
                uint32_t packetLen = 0;
                if (length >= DasPacket::getMinLength())
                    packetLen = reinterpret_cast<DasPacket *>(data)->getLength();
                LOG_DEBUG("Consumed %u of %u (expecting %u), retry %u/6", consumed, length, packetLen, retryCounter);
                // Exponentially sleep up to ~1.1s, first pass doesn't sleep
                epicsThreadSleep(1e-5 * pow(10, retryCounter++));
                continue;
            }

            // OCC still doesn't have enough data, check what's going on
            DasPacket *packet = reinterpret_cast<DasPacket *>(data);
            if (length < DasPacket::getMinLength()) {
                LOG_ERROR("Aborting processing thread, not enough input data to describe packet");
            } else if (packet->getLength() > packet->getMaxLength()) {
                LOG_ERROR("Aborting processing thread, packet size %u bigger than supported %u", packet->getLength(), packet->getMaxLength());
            } else if (packet->getLength() > length) {
                LOG_ERROR("Aborting processing thread, expecting %u bytes but only have %u", packet->getLength(), length);
            } else {
                LOG_ERROR("Aborting processing thread, undetermined error");
            }
            dump((char *)data + consumed, length - consumed);
            handleRecvError(-ERANGE);
            break;
        }
    }
}

void OccPortDriver::sendToPlugins(int messageType, const DasPacketList *packetList)
{
    const void *addr = reinterpret_cast<const void *>(packetList);
    doCallbacksGenericPointer(const_cast<void *>(addr), messageType, 0);
}
