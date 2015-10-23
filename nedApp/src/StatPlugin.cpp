/* StatPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "StatPlugin.h"

#include <climits>

#define NUM_STATPLUGIN_PARAMS ((int)(&LAST_STATPLUGIN_PARAM - &FIRST_STATPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(StatPlugin, 3, "Port name", string, "Dispatcher port name", string, "Blocking", int);

StatPlugin::StatPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_STATPLUGIN_PARAMS, 1, asynOctetMask | asynFloat64Mask, asynFloat64Mask)
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
}

void StatPlugin::processData(const DasPacketList * const packetList)
{
    m_receivedCount += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        m_receivedBytes += packet->length();
        if (packet->isResponse()) {
            m_cmdCount++;
            m_cmdBytes += packet->length();
        } else if (packet->isNeutronData()) {
            accumulatePCharge(m_neutronPulseTime, packet->getRtdlHeader(), m_neutronPCharge);
            m_dataCount++;
            m_dataBytes += packet->length();
        } else if (packet->isMetaData()) {
            m_metaCount++;
            m_metaBytes += packet->length();
        } else if (packet->isRtdl()) {
            accumulatePCharge(m_rtdlPulseTime, packet->getRtdlHeader(), m_rtdlPCharge);
            m_rtdlCount++;
            m_rtdlBytes += packet->length();
        } else if (packet->cmdinfo.is_command && packet->cmdinfo.command == DasPacket::CMD_TSYNC) {
            m_tsyncCount++;
            m_tsyncBytes += packet->length();
        } else if (packet->isBadPacket()) {
            m_badCount++;
            m_badBytes += packet->length();
        }
    }

    setIntegerParam(ProcCount,m_receivedCount % INT_MAX);

    setIntegerParam(TotCnt,   m_receivedCount % INT_MAX);
    setIntegerParam(CmdCnt,   m_cmdCount % INT_MAX);
    setIntegerParam(DataCnt,  m_dataCount % INT_MAX);
    setIntegerParam(MetaCnt,  m_metaCount % INT_MAX);
    setIntegerParam(RtdlCnt,  m_rtdlCount % INT_MAX);
    setIntegerParam(TsyncCnt, m_tsyncCount % INT_MAX);
    setIntegerParam(BadCnt,   m_badCount % INT_MAX);

    setIntegerParam(TotByte,  m_receivedBytes % INT_MAX);
    setIntegerParam(CmdByte,  m_cmdBytes % INT_MAX);
    setIntegerParam(DataByte, m_dataBytes % INT_MAX);
    setIntegerParam(MetaByte, m_metaBytes % INT_MAX);
    setIntegerParam(RtdlByte, m_rtdlBytes % INT_MAX);
    setIntegerParam(TsyncByte,m_tsyncBytes % INT_MAX);
    setIntegerParam(BadByte,  m_badBytes % INT_MAX);

    setDoubleParam(NeutronPCharge, m_neutronPCharge);
    setDoubleParam(RtdlPCharge, m_rtdlPCharge);

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
