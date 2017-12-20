/* OccPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "OccPlugin.h"
#include "DmaCircularBuffer.h"
#include "DmaCopier.h"
#include "BasePlugin.h"
#include "Log.h"

#include <cstring> // strerror
#include <occlib.h>
#include <math.h>
#include <sstream>

EPICS_REGISTER_PLUGIN(OccPlugin, 3, "Port name", string, "OCC connection string", string, "Local buffer size", int);

/**
 * A global variable to assign unique sourceId to all packets coming from this device.
 */
static uint32_t g_sourceId = 0;

OccPlugin::OccPlugin(const char *portName, const char *devfile, uint32_t localBufferSize)
    : BasePlugin(portName, 0, asynFloat64Mask|asynOctetMask, asynFloat64Mask|asynOctetMask)
    , m_sendId(0)
    , m_recvId(0xFFFFFFFF)
    , m_sourceId(g_sourceId++)
    , m_occ(NULL)
{
    int status;

    // Register params with asyn
    //           0123456789
    createParam("Status",           asynParamInt32,     &Status,        STAT_OK);   // READ - Status of OccPlugin       (see OccPlugin::Status)
    createParam("LastErr",          asynParamInt32,     &LastErr,       0);         // READ - Last error code returned by OCC API
    createParam("HwType",           asynParamInt32,     &HwType);                   // READ - OCC board type                (1=SNS PCI-X,2=SNS PCIe,15=simulator)
    createParam("HwVer",            asynParamInt32,     &HwVer);                    // READ - OCC board hardware version
    createParam("FwVer",            asynParamInt32,     &FwVer);                    // READ - OCC board firmware version
    createParam("FwDate",           asynParamOctet,     &FwDate);                   // READ - OCC board firmware date
    createParam("ConStatus",        asynParamInt32,     &ConStatus);                // READ - Optical connection status     (0=connected,1=no SFP,2=no cable,3=laser fault)
    createParam("Command",          asynParamInt32,     &Command);                  // WRITE - Issue OccPlugin command  (see OccPlugin::Command)
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
    createParam("OldPktsEn",        asynParamInt32,     &OldPktsEn, 0);             // WRITE - Enable support for old DAS 1.0 packets

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
            std::bind(&OccPlugin::processOccDataThread, this, std::placeholders::_1),
            epicsThreadGetStackSize(epicsThreadStackMedium),
            epicsThreadPriorityHigh
        );
        m_occBufferReadThread->start();
        m_occStatusRefreshThread = new Thread(
            "OCC status",
            std::bind(&OccPlugin::refreshOccStatusThread, this, std::placeholders::_1),
            epicsThreadGetStackSize(epicsThreadStackSmall),
            epicsThreadPriorityLow
        );
        m_occStatusRefreshThread->start();
    }
}

OccPlugin::~OccPlugin()
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

void OccPlugin::refreshOccStatusThread(epicsEvent *shutdown)
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

void OccPlugin::refreshOccStatus(bool basic_status)
{
    // This one can take long time to execute, don't lock the driver while it's executing
    int ret = occ_status(m_occ, &m_occStatusCache, basic_status ? OCC_STATUS_FAST : OCC_STATUS_FULL);

    this->lock();

    if (ret != 0) {
        setIntegerParam(LastErr, -ret);
        LOG_ERROR("Failed to query OCC status: %s(%d)", strerror(-ret), ret);
    } else {
        char buf[20];
        snprintf(buf, sizeof(buf), "%jX", m_occStatusCache.fpga_serial_number);
        setStringParam(FpgaSn,          buf);
        snprintf(buf, sizeof(buf), "%04X/%02X/%02X",  m_occStatusCache.firmware_date        & 0xFFFF,
                                                     (m_occStatusCache.firmware_date >> 24) & 0xFF,
                                                     (m_occStatusCache.firmware_date >> 16) & 0xFF);
        setStringParam(FwDate,          buf);

        setIntegerParam(HwType,         m_occStatusCache.board);
        setIntegerParam(HwVer,          m_occStatusCache.hardware_ver);
        setIntegerParam(FwVer,          m_occStatusCache.firmware_ver);
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

asynStatus OccPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
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

    } else if (pasynUser->reason == OldPktsEn) {
        // Note that OCC required RX disabled during this operation. OCC library
        // is enforcing it so we don't have to do it here.
        if ((ret = occ_enable_old_packets(m_occ, value > 0)) != 0) {
            LOG_ERROR("Unable to %s old packets - %s(%d)", (value > 0 ? "enable" : "disable"), strerror(-ret), ret);
            setIntegerParam(LastErr, -ret);
            callParamCallbacks();
            return asynError;
        }
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

asynStatus OccPlugin::readInt32(asynUser *pasynUser, epicsInt32 *value)
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

void OccPlugin::recvUpstream(DasCmdPacketList *packets)
{
    if (m_occ == NULL) {
        LOG_ERROR("OCC device not initialized");
        return;
    }

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        DasCmdPacket *packet = *it;
        packet->sequence = (++m_sendId % 255);
        int ret = occ_send(m_occ, reinterpret_cast<const void *>(packet), ALIGN_UP(packet->length, 4));
        if (ret != 0) {
            setIntegerParam(LastErr, -ret);
            setIntegerParam(Status, STAT_OCC_ERROR);
            callParamCallbacks();
            LOG_ERROR("Unable to send data to OCC - %s(%d)\n", strerror(-ret), ret);
            break;
        }
    }
}

void OccPlugin::recvUpstream(DasPacketList *packets)
{
    if (m_occ == NULL) {
        LOG_ERROR("OCC device not initialized");
        return;
    }

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        DasPacket *packet = *it;
        int ret = occ_send(m_occ, reinterpret_cast<const void *>(packet), ALIGN_UP(packet->getLength(), 4));
        if (ret != 0) {
            setIntegerParam(LastErr, -ret);
            setIntegerParam(Status, STAT_OCC_ERROR);
            callParamCallbacks();
            LOG_ERROR("Unable to send data to OCC - %s(%d)\n", strerror(-ret), ret);
            break;
        }
    }
}

void OccPlugin::reset() {
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

void OccPlugin::handleRecvError(int ret)
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

void OccPlugin::dump(const char *data, uint32_t len)
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

void OccPlugin::processOccDataThread(epicsEvent *shutdown)
{
    uint32_t retryCounter = 0;

    while (shutdown->tryWait() == false) {
        void *data;
        uint32_t length;

        // Wait for data, use a timeout for data rate out calculation
        int ret = m_circularBuffer->wait(&data, &length, 1.0);
        if (ret == -ETIME || ret == -ECONNRESET) {
            continue;
        } else if (ret != 0) {
            handleRecvError(ret);
            LOG_ERROR("Unable to receive data from OCC, stopped - %s(%d)\n", strerror(-ret), ret);
            break;
        }

        try {
            uint32_t left = processOccData(reinterpret_cast<uint8_t*>(data), length);
            m_circularBuffer->consume(length - left);
            retryCounter = 0;
        } catch (std::runtime_error &e) {
            if (retryCounter < 7) {
                LOG_DEBUG("Partial data in buffer, waiting for more (retry %u/6)", retryCounter);
                // Exponentially sleep up to ~1.1s, first pass doesn't sleep
                epicsThreadSleep(1e-5 * pow(10, retryCounter++));
                continue;
            }

            // OCC still doesn't have enough data, abort thread
            LOG_ERROR("Aborting processing thread: %s", e.what());
            dump(reinterpret_cast<const char *>(data), length);
            handleRecvError(-ERANGE);
            break;
        }
    }
}

uint32_t OccPlugin::processOccData(uint8_t *ptr, uint32_t size)
{
    int maxPktSize = getIntegerParam(MaxPktSize);
    bool forceOldPkts = getBooleanParam(OldPktsEn);
    bool first = true;

    DasPacketList oldDas;
    DasDataPacketList dasData;
    DasCmdPacketList dasCmd;
    DasRtdlPacketList dasRtdl;
    ErrorPacketList errors;

    uint8_t *end = ptr + size;
    while (ptr < end) {
        uint32_t bytesLeft = (end - ptr);
        uint32_t bytesProcessed = 0;

        // We don't know what we're receiving. It could be old DAS packet or
        // a new DAS header. They differ in the most significant 4 bits of the
        // first 32 bits received.
        uint32_t version = (*reinterpret_cast<uint32_t *>(ptr) >> 28);

        try {
            if (version == 0 || forceOldPkts) {
                // Old DAS packet
                DasPacket *packet = reinterpret_cast<DasPacket *>(ptr);

                if (bytesLeft < sizeof(DasPacket)) {
                    std::ostringstream error;
                    error << "Not enough data to describe old DAS packet header, needed " << sizeof(DasPacket) << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (bytesLeft < packet->getLength()) {
                    std::ostringstream error;
                    error << "Not enough data to describe old DAS packet, needed " << packet->getLength() << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (static_cast<uint32_t>(maxPktSize) < packet->getLength()) {
                    std::ostringstream error;
                    error << "Old DAS packet of " << packet->getLength() << " bytes exceeds " << maxPktSize << " bytes threshold";
                    throw std::range_error(error.str());
                }

                oldDas.push_back(packet);
                bytesProcessed = packet->getLength();
                if (bytesProcessed == 0) {
                    throw std::range_error("failed to parse old DAS packet");
                }

            } else if (version == 1) {
                // New SNS packet header
                Packet *packet = reinterpret_cast<Packet *>(ptr);

                if (bytesLeft < sizeof(Packet)) {
                    std::ostringstream error;
                    error << "Not enough data to describe DAS packet header, needed " << sizeof(Packet) << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (bytesLeft < packet->length) {
                    std::ostringstream error;
                    error << "Not enough data to describe DAS packet, needed " << packet->length << " got " << bytesLeft << " bytes";
                    throw std::range_error(error.str());
                }
                if (static_cast<uint32_t>(maxPktSize) < packet->length) {
                    std::ostringstream error;
                    error << "DAS packet of " << packet->length << " bytes exceeds " << maxPktSize << " bytes threshold";
                    throw std::range_error(error.str());
                }

                if (m_recvId != 0xFFFFFFFF && packet->sequence != ((m_recvId+1) % 255)) {
                    LOG_ERROR("Expecting packet with sequence number %u, got %u", (m_recvId+1)%255, packet->sequence);
                }
                m_recvId = packet->sequence;

                if (packet->type == Packet::TYPE_DAS_DATA) {
                    reinterpret_cast<DasDataPacket *>(packet)->source = m_sourceId;
                    dasData.push_back(reinterpret_cast<DasDataPacket *>(packet));
                } else if (packet->type == Packet::TYPE_DAS_RTDL) {
                    reinterpret_cast<DasRtdlPacket *>(packet)->source = m_sourceId;
                    dasRtdl.push_back(reinterpret_cast<DasRtdlPacket *>(packet));
                } else if (packet->type == Packet::TYPE_DAS_CMD) {
                    dasCmd.push_back(reinterpret_cast<DasCmdPacket *>(packet));
                } else if (packet->type == Packet::TYPE_ERROR) {
                    reinterpret_cast<ErrorPacket *>(packet)->source = m_sourceId;
                    dasRtdl.push_back(reinterpret_cast<DasRtdlPacket *>(packet));
                }

                bytesProcessed = packet->length;

            } else {
                throw std::runtime_error("Unsupported packet received");
            }
        } catch (std::exception &e) {
            if (first)
                throw;
            else
                break;
        }

        ptr += bytesProcessed;
        first = false;
    }

    // Publish all packets in parallel ..
    std::vector< std::shared_ptr<PluginMessage> > messages;
    if (!oldDas.empty())
        messages.push_back(sendDownstream(&oldDas, false));
    if (!dasCmd.empty())
        messages.push_back(sendDownstream(&dasCmd, false));
    if (!dasData.empty())
        messages.push_back(sendDownstream(&dasData, false));
    if (!dasRtdl.empty())
        messages.push_back(sendDownstream(&dasRtdl, false));
    if (!errors.empty())
        messages.push_back(sendDownstream(&errors, false));

    // .. and wait for all of them to get released
    for (auto it = messages.begin(); it != messages.end(); it++) {
        if (!!(*it)) {
            (*it)->waitAllReleased();
        }
    }

    return (end - ptr);
}
