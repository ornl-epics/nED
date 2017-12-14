/* StatPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Event.h"
#include "StatPlugin.h"

#include <climits>

EPICS_REGISTER_PLUGIN(StatPlugin, 3, "Port name", string, "Parent ports", string, "Blocking", int);

StatPlugin::StatPlugin(const char *portName, const char *parentPlugins, int blocking)
    : BasePlugin(portName, std::string(parentPlugins).find(',')!=std::string::npos, asynOctetMask | asynFloat64Mask, asynFloat64Mask)
    , m_receivedCount(0)
    , m_receivedBytes(0)
    , m_cmdCount(0)
    , m_cmdBytes(0)
    , m_dataCount(0)
    , m_dataBytes(0)
    , m_metaCount(0)
    , m_metaBytes(0)
    , m_rtdlCount(0)
    , m_rtdlBytes(0)
    , m_tsyncCount(0)
    , m_tsyncBytes(0)
    , m_badCount(0)
    , m_badBytes(0)
    , m_neutronPCharge(0)
    , m_rtdlPCharge(0)
    , m_pulseType(RtdlHeader::RTDL_FLAVOR_TARGET_1)
{

    createParam("Reset",        asynParamInt32, &Reset,     0); // WRITE - Reset counters

    createParam("CmdCnt",       asynParamInt32, &CmdCnt,    0); // READ - Number of command response packets
    createParam("DataCnt",      asynParamInt32, &DataCnt,   0); // READ - Number of data packets
    createParam("MetaCnt",      asynParamInt32, &MetaCnt,   0); // READ - Number of metadata packets
    createParam("RtdlCnt",      asynParamInt32, &RtdlCnt,   0); // READ - Number of RTDL packets
    createParam("TsyncCnt",     asynParamInt32, &TsyncCnt,  0); // READ - Number of TSYNC packets
    createParam("BadCnt",       asynParamInt32, &BadCnt,    0); // READ - Number of bad packets
    createParam("TotCnt",       asynParamInt32, &TotCnt,    0); // READ - Total number of packets

    createParam("CmdByte",      asynParamInt32, &CmdByte,   0); // READ - Bytes of command response packets
    createParam("DataByte",     asynParamInt32, &DataByte,  0); // READ - Bytes of data packets
    createParam("MetaByte",     asynParamInt32, &MetaByte,  0); // READ - Bytes of metadata packets
    createParam("RtdlByte",     asynParamInt32, &RtdlByte,  0); // READ - Bytes of RTDL packets
    createParam("TsyncByte",    asynParamInt32, &TsyncByte, 0); // READ - Bytes of TSYNCpackets
    createParam("BadByte",      asynParamInt32, &BadByte,   0); // READ - Bytes of bad packets
    createParam("TotByte",      asynParamInt32, &TotByte,   0); // READ - Total number of bytes received

    createParam("NeutronPCharge", asynParamFloat64, &NeutronPCharge, 0.0); // READ - Accumulated neutron proton charge since last reset
    createParam("RtdlPCharge",  asynParamFloat64, &RtdlPCharge, 0.0); // READ - Accumulated RTDL (accelerator) proton charge since last reset
    createParam("PulseType",    asynParamInt32, &PulseType, RtdlHeader::RTDL_FLAVOR_TARGET_1); // READ -Select pulse type to collect proton charge for

    static std::list<int> msgs = {MsgDasData, MsgDasCmd, MsgDasRtdl};
    BasePlugin::connect(parentPlugins, msgs);
}

void StatPlugin::recvDownstream(DasDataPacketList *packets)
{
    m_receivedCount += packets->size();

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        DasDataPacket *packet = *it;
        m_dataBytes += packet->length;
        m_receivedBytes += packet->length;

        if (packet->format == DasDataPacket::DATA_FMT_META) {
            uint32_t nEvents = 0;
            packet->getEvents<Event::Pixel>(nEvents);
            m_metaBytes += nEvents * sizeof(Event::Pixel);
            m_metaCount++;
        } else {
            uint32_t nEvents = 0;
            uint32_t eventSize;
            if (packet->format == DasDataPacket::DATA_FMT_PIXEL) {
                packet->getEvents<Event::Pixel>(nEvents);
                eventSize = sizeof(Event::Pixel);
            } else if (packet->format == DasDataPacket::DATA_FMT_ACPC_XY_PS) {
                packet->getEvents<Event::AcpcXyPs>(nEvents);
                eventSize = sizeof(Event::AcpcXyPs);
            } else if (packet->format == DasDataPacket::DATA_FMT_AROC_RAW) {
                packet->getEvents<Event::ArocRaw>(nEvents);
                eventSize = sizeof(Event::ArocRaw);
            } else {
                packet->getEvents<Event::Pixel>(nEvents);
                eventSize = sizeof(Event::Pixel);
            }
            m_dataBytes += eventSize * nEvents;
            m_dataCount++;
        }
    }

    setIntegerParam(TotCnt,   m_receivedCount % INT_MAX);
    setIntegerParam(DataCnt,  m_dataCount % INT_MAX);
    setIntegerParam(MetaCnt,  m_metaCount % INT_MAX);

    setIntegerParam(TotByte,  m_receivedBytes % INT_MAX);
    setIntegerParam(DataByte, m_dataBytes % INT_MAX);
    setIntegerParam(MetaByte, m_metaBytes % INT_MAX);

    callParamCallbacks();
}

void StatPlugin::recvDownstream(DasCmdPacketList *packets)
{
    m_receivedCount += packets->size();
    m_cmdCount += packets->size();
    for (auto it = packets->begin(); it != packets->end(); it++) {
        m_receivedBytes += (*it)->length;
        m_cmdBytes += (*it)->length;
    }
    setIntegerParam(TotCnt,   m_receivedCount % INT_MAX);
    setIntegerParam(CmdCnt,   m_cmdCount % INT_MAX);
    setIntegerParam(TotByte,  m_receivedBytes % INT_MAX);
    setIntegerParam(CmdByte,  m_cmdBytes % INT_MAX);
    callParamCallbacks();
}

void StatPlugin::recvDownstream(DasRtdlPacketList *packets)
{
    m_receivedCount += packets->size();
    m_rtdlCount += packets->size();
    for (auto it = packets->begin(); it != packets->end(); it++) {
        m_receivedBytes += (*it)->length;
        m_rtdlBytes += (*it)->length;
    }
    setIntegerParam(TotCnt,   m_receivedCount % INT_MAX);
    setIntegerParam(TotByte,  m_receivedBytes % INT_MAX);
    setIntegerParam(RtdlCnt,  m_rtdlCount % INT_MAX);
    setIntegerParam(RtdlByte, m_rtdlBytes % INT_MAX);
    callParamCallbacks();
}

asynStatus StatPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Reset) {
        m_receivedCount = m_receivedBytes = 0;
        m_cmdCount      = m_cmdBytes      = 0;
        m_dataCount     = m_dataBytes     = 0;
        m_metaCount     = m_metaBytes     = 0;
        m_rtdlCount     = m_rtdlBytes     = 0;
        m_tsyncCount    = m_tsyncBytes    = 0;
        m_badCount      = m_badBytes      = 0;
        m_neutronPCharge = 0;
        m_rtdlPCharge    = 0;
        return asynSuccess;
    } else if (pasynUser->reason == PulseType) {
        m_pulseType = (RtdlHeader::PulseFlavor)value;
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void StatPlugin::accumulatePCharge(epicsTimeStamp &lastPulseTime, const RtdlHeader *rtdl, double &pcharge)
{
    if (rtdl && rtdl->pulse.flavor == m_pulseType) {
        epicsTimeStamp time = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
        if (epicsTimeNotEqual(&time, &lastPulseTime)) {
            lastPulseTime = time;
            pcharge += 0.00000000001 * rtdl->pulse.charge;
        }
    }
}
