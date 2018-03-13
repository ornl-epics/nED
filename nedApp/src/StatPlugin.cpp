/* StatPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Event.h"
#include "Log.h"
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
    createParam("PChargeRtdl",  asynParamFloat64, &PChargeRtdl,  0.0); // READ - Proton charge updated with every RTDL
    createParam("PChargeData",  asynParamFloat64, &PChargeData,  0.0); // READ - Proton charge updated with every acquisition frame
    createParam("RtdlCacheSize",asynParamInt32,   &RtdlCacheSize, 10); // WRITE - Number of RTDL data to be cached

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

    for (const auto &packet: packets) {
        uint32_t nEvents = packet->getNumEvents();
        totBytes += packet->getLength();

        if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_META) {
            metaBytes += nEvents * packet->getEventsSize();
            metaCnts += nEvents;
            if (isTimestampUnique(packet->getTimeStamp(), m_metaTimes))
                metaTimes += 1;

        } else {
            neutronBytes += nEvents * packet->getEventsSize();
            neutronCnts += nEvents;
            if (isTimestampUnique(packet->getTimeStamp(), m_neutronTimes))
                neutronTimes += 1;
        }

        double pcharge = getDataProtonCharge(packet->getTimeStamp());
        if (pcharge >= 0.0) {
            setDoubleParam(PChargeData, pcharge);
            // We need to do callbacks now in case multiple frames
            // packets are being processed.
            callParamCallbacks();
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
        cmdBytes += (*it)->getLength();
        totBytes += (*it)->getLength();
    }

    setDoubleParam(CmdPkts,         cmdPkts % LLONG_MAX);
    setDoubleParam(CmdBytes,        cmdBytes % LLONG_MAX);
    setDoubleParam(TotBytes,        totBytes % LLONG_MAX);
    callParamCallbacks();
}

void StatPlugin::recvDownstream(const RtdlPacketList &packets)
{
    uint64_t rtdlTimes  = getDoubleParam(RtdlTimes);
    uint64_t rtdlBytes  = getDoubleParam(RtdlBytes);
    uint64_t rtdlPkts   = getDoubleParam(RtdlPkts) + packets.size();
    uint64_t totBytes   = getDoubleParam(TotBytes);

    for (const auto &packet: packets) {
        rtdlBytes += packet->getLength();
        totBytes += packet->getLength();
        if (cacheRtdl(packet) == true) {
            rtdlTimes += 1;
        }

        double pcharge = getDataProtonCharge(packet->getTimeStamp());
        if (pcharge >= 0.0) {
            setDoubleParam(PChargeData, pcharge);
            // We need to do callbacks now in case multiple frames
            // packets are being processed.
            callParamCallbacks();
        }

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

    for (const auto &packet: packets) {
        totBytes += packet->getLength();
    }

    setDoubleParam(ErrorPkts,       errorPkts % LLONG_MAX);
    setDoubleParam(TotBytes,        totBytes % LLONG_MAX);
    callParamCallbacks();
}

bool StatPlugin::isTimestampUnique(const epicsTimeStamp &timestamp, std::list<epicsTime> &que)
{
    size_t cacheSize = getIntegerParam(RtdlCacheSize);
    epicsTime t(timestamp);

    bool found = (std::find(que.begin(), que.end(), t) != que.end());
    if (!found) {
        while (que.size() >= cacheSize)
            que.pop_back();
        que.push_front(t);
    }
    return !found;
}

bool StatPlugin::cacheRtdl(const RtdlPacket *packet)
{
    size_t cacheSize = getIntegerParam(RtdlCacheSize);
    epicsTime t(packet->getTimeStamp());

    for (auto it = m_dataPcharge.begin(); it != m_dataPcharge.end(); it++) {
        if (std::get<0>(*it) == t) {
            // Add proton charge to the previous RTDL if still in queue
            if (++it != m_dataPcharge.end()) {
                std::get<1>(*it) = std::max(0.0, packet->getProtonCharge());
            } else {
                LOG_ERROR("RTDL cached proton charge queue too small");
            }
            return false;
        }
    }

    m_dataPcharge.push_front(std::make_tuple(t, -1.0, false));
    while (m_dataPcharge.size() >= cacheSize)
        m_dataPcharge.pop_back();
    return true;
}

double StatPlugin::getDataProtonCharge(const epicsTimeStamp &timestamp)
{
    epicsTime t(timestamp);
    for (auto it = m_dataPcharge.begin(); it != m_dataPcharge.end(); it++) {
        if (std::get<0>(*it) == t) {
            if (std::get<1>(*it) < 0.0 || std::get<2>(*it) == true)
                return -1.0;
            std::get<2>(*it) = true;
            return std::get<1>(*it);
        }
    }
    LOG_WARN("No RTDL information cached for %u.%09u", timestamp.secPastEpoch, timestamp.nsec);
    return -1.0;
}
