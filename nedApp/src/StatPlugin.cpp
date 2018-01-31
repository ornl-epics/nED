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

#include <algorithm>
#include <climits>

EPICS_REGISTER_PLUGIN(StatPlugin, 2, "Port name", string, "Parent ports", string);

StatPlugin::StatPlugin(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, true, asynFloat64Mask, asynFloat64Mask)
{
    createParam("CmdPkts",      asynParamFloat64, &CmdPkts,      0.0); // READ - Number of command response packets
    createParam("CmdBytes",     asynParamFloat64, &CmdBytes,     0.0); // READ - Command response packets bytes
    createParam("NeutronCnts",  asynParamFloat64, &NeutronCnts,  0.0); // READ - Number of data packets
    createParam("NeutronBytes", asynParamFloat64, &NeutronBytes, 0.0); // READ - Data packets bytes
    createParam("NeutronTimes", asynParamFloat64, &NeutronTimes, 0.0); // READ - Unique neutron data timestamps
    createParam("MetaCnts",     asynParamFloat64, &MetaCnts,     0.0); // READ - Number of metadata packets
    createParam("MetaBytes",    asynParamFloat64, &MetaBytes,    0.0); // READ - Metadata packets bytes
    createParam("MetaTimes",    asynParamFloat64, &MetaTimes,    0.0); // READ - Unique meta data timestamps
    createParam("ErrorPkts",    asynParamFloat64, &ErrorPkts,    0.0); // READ - Number of bad packets
    createParam("RtdlPkts",     asynParamFloat64, &RtdlPkts,     0.0); // READ - Number of RTDL packets
    createParam("RtdlBytes",    asynParamFloat64, &RtdlBytes,    0.0); // READ - RTDL packets bytes
    createParam("RtdlTimes",    asynParamFloat64, &RtdlTimes,    0.0); // READ - Unique RTDL timestamps
    createParam("TotBytes",     asynParamFloat64, &TotBytes,     0.0); // READ - Total number of bytes received

    BasePlugin::connect(parentPlugins, {MsgDasData, MsgDasCmd, MsgDasRtdl, MsgError});
}

void StatPlugin::recvDownstream(const DasDataPacketList &packets)
{
    uint64_t neutronCnts  = getDoubleParam(NeutronCnts);
    uint64_t neutronBytes = getDoubleParam(NeutronBytes);
    uint64_t neutronTimes = getDoubleParam(NeutronTimes);
    uint64_t metaCnts     = getDoubleParam(MetaCnts);
    uint64_t metaBytes    = getDoubleParam(MetaBytes);
    uint64_t metaTimes    = getDoubleParam(MetaTimes);
    uint64_t totBytes     = getDoubleParam(TotBytes);

    for (auto it = packets.cbegin(); it != packets.cend(); it++) {
        const DasDataPacket *packet = *it;
        uint32_t nEvents = packet->getNumEvents();
        totBytes += packet->length;

        if (packet->format == DasDataPacket::DATA_FMT_META) {
            metaBytes += nEvents * packet->getEventsSize();
            metaCnts += nEvents;
            if (isTimestampUnique((*it)->timestamp_sec, (*it)->timestamp_nsec, m_metaTimes))
                metaTimes += 1;

        } else {
            neutronBytes += nEvents * packet->getEventsSize();
            neutronCnts += nEvents;
            if (isTimestampUnique((*it)->timestamp_sec, (*it)->timestamp_nsec, m_neutronTimes))
                neutronTimes += 1;
        }
    }

    setDoubleParam(NeutronCnts,     neutronCnts % LLONG_MAX);
    setDoubleParam(NeutronBytes,    neutronBytes % LLONG_MAX);
    setDoubleParam(NeutronTimes,    neutronTimes % LLONG_MAX);
    setDoubleParam(MetaCnts,        metaCnts % LLONG_MAX);
    setDoubleParam(MetaBytes,       metaBytes % LLONG_MAX);
    setDoubleParam(MetaTimes,       metaTimes % LLONG_MAX);
    setDoubleParam(TotBytes,        totBytes % LLONG_MAX);
    callParamCallbacks();
}

void StatPlugin::recvDownstream(const DasCmdPacketList &packets)
{
    uint64_t cmdPkts  = getDoubleParam(CmdPkts) + packets.size();
    uint64_t cmdBytes = getDoubleParam(CmdBytes);
    uint64_t totBytes = getDoubleParam(TotBytes);

    for (auto it = packets.begin(); it != packets.end(); it++) {
        cmdBytes += (*it)->length;
        totBytes += (*it)->length;
    }

    setDoubleParam(CmdPkts,         cmdPkts % LLONG_MAX);
    setDoubleParam(CmdBytes,        cmdBytes % LLONG_MAX);
    setDoubleParam(TotBytes,        totBytes % LLONG_MAX);
    callParamCallbacks();
}

void StatPlugin::recvDownstream(const DasRtdlPacketList &packets)
{
    uint64_t rtdlTimes  = getDoubleParam(RtdlTimes);
    uint64_t rtdlBytes  = getDoubleParam(RtdlBytes);
    uint64_t rtdlPkts   = getDoubleParam(RtdlPkts) + packets.size();
    uint64_t totBytes   = getDoubleParam(TotBytes);

    for (auto it = packets.begin(); it != packets.end(); it++) {
        rtdlBytes += (*it)->length;
        totBytes += (*it)->length;
        if (isTimestampUnique((*it)->timestamp_sec, (*it)->timestamp_nsec, m_rtdlTimes))
            rtdlTimes += 1;
    }

    setDoubleParam(RtdlTimes,       rtdlTimes % LLONG_MAX);
    setDoubleParam(RtdlBytes,       rtdlBytes % LLONG_MAX);
    setDoubleParam(RtdlPkts,        rtdlPkts % LLONG_MAX);
    setDoubleParam(TotBytes,        totBytes % LLONG_MAX);
    callParamCallbacks();
}

void StatPlugin::recvDownstream(const ErrorPacketList &packets)
{
    uint64_t errorPkts  = getDoubleParam(ErrorPkts) + packets.size();
    uint64_t totBytes   = getDoubleParam(TotBytes);

    for (auto it = packets.begin(); it != packets.end(); it++) {
        totBytes += (*it)->length;
    }

    setDoubleParam(ErrorPkts,       errorPkts % LLONG_MAX);
    setDoubleParam(TotBytes,        totBytes % LLONG_MAX);
    callParamCallbacks();
}

bool StatPlugin::isTimestampUnique(uint32_t sec, uint32_t nsec, std::list<epicsTime> &que)
{
    epicsTimeStamp timestamp = { sec, nsec };
    epicsTime t(timestamp);

    bool found = (std::find(que.begin(), que.end(), t) != que.end());
    if (!found) {
        while (que.size() > MAX_TIME_QUE_SIZE)
            que.pop_back();
    que.push_front(t);
    }
    return !found;
}
