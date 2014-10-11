/* AcpcPvaPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPvaPlugin.h"

EPICS_REGISTER_PLUGIN(AcpcPvaPlugin, 3, "port name", string, "dispatcher port", string, "pvAccess PV record prefix", string);

AcpcPvaPlugin::AcpcPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix)
{
    uint32_t maxNormalEventsPerPacket = (DasPacket::MaxLength/4) / 6;

    if (!!m_pvRecord) {
        // Guestimate container size and force memory pre-allocation, will automatically extend if needed
        m_pvRecord->cache.time_of_flight.reserve(maxNormalEventsPerPacket); // normal mode is the shortest => max counts in packet
        m_pvRecord->cache.position_index.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.position_x.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.position_y.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.photo_sum_x.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.photo_sum_y.reserve(maxNormalEventsPerPacket);
    }
}

void AcpcPvaPlugin::processDataNormal(const DasPacketList * const packetList)
{
    // This function is assuming exclusive access to m_pvRecord.
    // Locked processData() enforces that guarantee.

    // Structure describing normal data from ACPC.
    struct AcpcNormalData {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t position_x;
        uint32_t position_y;
        uint32_t photo_sum_x;
        uint32_t photo_sum_y;
    };

    // Sanity check, BasePvaPlugin should prevent it anyway
    if (!m_pvRecord)
        return;

    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();

        // TODO: what about metadata? In tof,pixel format?
        if (packet->isNeutronData() == false || rtdl == 0)
            continue;

        // Extract timestamp and proton charge from RTDL
        // RTDL should always be present when working with DSP-T
        epics::pvData::TimeStamp time(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        if (time != m_pvRecord->cache.timeStamp) {
            // Different pulse detected, post what we have and start over
            m_pvRecord->postCached();
        }
        m_pvRecord->cache.timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        m_pvRecord->cache.proton_charge = rtdl->charge;

        // Data itself does not contain format information. Simple length
        // check is the best we can do.
        uint32_t dataLength;
        const struct AcpcNormalData *data = reinterpret_cast<const AcpcNormalData *>(packet->getData(&dataLength));
        if (data == 0 || dataLength == 0)
            continue;
        dataLength *= sizeof(uint32_t);
        if (dataLength % sizeof(AcpcNormalData) != 0)
            continue;
        dataLength /= sizeof(AcpcNormalData);

        // Go through events and append to cache
        while (dataLength-- > 0) {
            m_pvRecord->cache.time_of_flight.push_back(data->time_of_flight);
            m_pvRecord->cache.position_index.push_back(data->position_index);
            m_pvRecord->cache.position_x.push_back(data->position_x);
            m_pvRecord->cache.position_y.push_back(data->position_y);
            m_pvRecord->cache.photo_sum_x.push_back(data->photo_sum_x);
            m_pvRecord->cache.photo_sum_y.push_back(data->photo_sum_y);
            data++;
        }
        m_pvRecord->postCached();
    }
}
