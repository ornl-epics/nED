/* AcpcPvaPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPvaPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(AcpcPvaPlugin, 3, "port name", string, "dispatcher port", string, "PV prefix", string);
const uint32_t AcpcPvaPlugin::CACHE_SIZE = 32*1024;

AcpcPvaPlugin::AcpcPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix)
{
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.position_x.reserve(CACHE_SIZE);
    m_cache.position_y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);
}

asynStatus AcpcPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
            setCallbacks(&AcpcPvaPlugin::processNormalData, &AcpcPvaPlugin::postNormalData);
            break;
        case DATA_MODE_RAW:
        case DATA_MODE_VERBOSE:
            // TODO
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
    }
    return BasePvaPlugin::writeInt32(pasynUser, value);
}

void AcpcPvaPlugin::processNormalData(const uint32_t *data, uint32_t count)
{
    // Structure describing normal data from ACPC.
    struct AcpcNormalData {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t position_x;
        uint32_t position_y;
        uint32_t photo_sum_x;
        uint32_t photo_sum_y;
    };

    uint32_t nEvents = count / (sizeof(AcpcNormalData) / sizeof(uint32_t));
    const AcpcNormalData *events = reinterpret_cast<const AcpcNormalData *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(events->position_index);
        m_cache.position_x.push_back(events->position_x);
        m_cache.position_y.push_back(events->position_y);
        m_cache.photo_sum_x.push_back(events->photo_sum_x);
        m_cache.photo_sum_y.push_back(events->photo_sum_y);
        events++;
    }
}

void AcpcPvaPlugin::postNormalData(const PvaNeutronData::shared_pointer& pvRecord)
{
    pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
    pvRecord->position_index->replace(freeze(m_cache.position_index));
    pvRecord->position_x->replace(freeze(m_cache.position_x));
    pvRecord->position_y->replace(freeze(m_cache.position_y));
    pvRecord->photo_sum_x->replace(freeze(m_cache.photo_sum_x));
    pvRecord->photo_sum_y->replace(freeze(m_cache.photo_sum_y));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.position_x.reserve(CACHE_SIZE);
    m_cache.position_y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);
}
