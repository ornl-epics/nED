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

AcpcPvaPlugin::AcpcPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName)
    : BasePvaPlugin(portName, dispatcherPortName, pvName)
{
    // normal mode is the shortest => max counts in packet
    uint32_t maxNormalEventsPerPacket = (DasPacket::MaxLength/4) / 6;

    // Guestimate container size and force memory pre-allocation, will automatically extend if needed
    m_cache.time_of_flight.reserve(maxNormalEventsPerPacket);
    m_cache.position_index.reserve(maxNormalEventsPerPacket);
    m_cache.position_x.reserve(maxNormalEventsPerPacket);
    m_cache.position_y.reserve(maxNormalEventsPerPacket);
    m_cache.photo_sum_x.reserve(maxNormalEventsPerPacket);
    m_cache.photo_sum_y.reserve(maxNormalEventsPerPacket);
}

void AcpcPvaPlugin::processNormalPacket(const DasPacket * const packet)
{
    // This function is assuming exclusive access to m_pvRecord.

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

    uint32_t nEvents;
    const struct AcpcNormalData *data = reinterpret_cast<const AcpcNormalData *>(packet->getEventData(&nEvents));
    if (data == 0 || nEvents == 0)
        return;
    nEvents *= sizeof(DasPacket::Event);
    if (nEvents % sizeof(AcpcNormalData) != 0)
        return;
    nEvents /= sizeof(AcpcNormalData);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(data->time_of_flight);
        m_cache.position_index.push_back(data->position_index);
        m_cache.position_x.push_back(data->position_x);
        m_cache.position_y.push_back(data->position_y);
        m_cache.photo_sum_x.push_back(data->photo_sum_x);
        m_cache.photo_sum_y.push_back(data->photo_sum_y);
        data++;
    }
}

void AcpcPvaPlugin::postNormalData(const epicsTimeStamp &pulseTime, uint32_t pulseCharge, uint32_t postSeq)
{
    epics::pvData::TimeStamp time(pulseTime.secPastEpoch, pulseTime.nsec, postSeq);

    m_pvRecord->beginGroupPut();
    m_pvRecord->timeStamp.set(time);
    m_pvRecord->proton_charge->put(pulseCharge);
    m_pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvRecord->position_index->replace(freeze(m_cache.position_index));
    m_pvRecord->position_x->replace(freeze(m_cache.position_x));
    m_pvRecord->position_y->replace(freeze(m_cache.position_y));
    m_pvRecord->photo_sum_x->replace(freeze(m_cache.photo_sum_x));
    m_pvRecord->photo_sum_y->replace(freeze(m_cache.photo_sum_y));
    m_pvRecord->endGroupPut();

    m_cache.time_of_flight.clear();
    m_cache.position_index.clear();
    m_cache.position_x.clear();
    m_cache.position_y.clear();
    m_cache.photo_sum_x.clear();
    m_cache.photo_sum_y.clear();
}
