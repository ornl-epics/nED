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
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_STATPLUGIN_PARAMS, 1, asynOctetMask)
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
{
    createParam("CmdCnt",       asynParamInt32, &CmdCnt,    m_cmdCount);
    createParam("DataCnt",      asynParamInt32, &DataCnt,   m_dataCount);
    createParam("MetaCnt",      asynParamInt32, &MetaCnt,   m_metaCount);
    createParam("RtdlCnt",      asynParamInt32, &RtdlCnt,   m_rtdlCount);
    createParam("TsyncCnt",     asynParamInt32, &TsyncCnt,  m_tsyncCount);
    createParam("BadCnt",       asynParamInt32, &BadCnt,    m_badCount);
    createParam("TotCnt",       asynParamInt32, &TotCnt,    m_receivedCount);

    createParam("CmdByte",      asynParamInt32, &CmdByte,   m_cmdBytes);
    createParam("DataByte",     asynParamInt32, &DataByte,  m_dataBytes);
    createParam("MetaByte",     asynParamInt32, &MetaByte,  m_metaBytes);
    createParam("RtdlByte",     asynParamInt32, &RtdlByte,  m_rtdlBytes);
    createParam("TsyncByte",    asynParamInt32, &TsyncByte, m_tsyncBytes);
    createParam("BadByte",      asynParamInt32, &BadByte,   m_badBytes);
    createParam("TotByte",      asynParamInt32, &TotByte,   m_receivedBytes);
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
            m_dataCount++;
            m_dataBytes += packet->length();
        } else if (packet->isMetaData()) {
            m_metaCount++;
            m_metaBytes += packet->length();
        } else if (packet->isRtdl()) {
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

    callParamCallbacks();
}
