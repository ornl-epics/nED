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

#define NUM_ACPCPVAPLUGIN_PARAMS ((int)(&LAST_ACPCPVAPLUGIN_PARAM - &FIRST_ACPCPVAPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(AcpcPvaPlugin, 3, "port name", string, "dispatcher port", string, "PV prefix", string);
const uint32_t AcpcPvaPlugin::CACHE_SIZE = 32*1024;

AcpcPvaPlugin::AcpcPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix, NUM_ACPCPVAPLUGIN_PARAMS)
    , m_xyDivider(1 << 24)
    , m_photosumDivider(1 << 15)
{
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.position_x.reserve(CACHE_SIZE);
    m_cache.position_y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);

    // UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth",  asynParamInt32, &XyFractWidth, 24); // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth",  asynParamInt32, &PsFractWidth, 15); // WRITE - Number of fraction bits in PhotoSum data
    callParamCallbacks();
}

asynStatus AcpcPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
            setCallbacks(&AcpcPvaPlugin::processTofPixelData, &AcpcPvaPlugin::postTofPixelData);
            break;
        case DATA_MODE_RAW:
            setCallbacks(&AcpcPvaPlugin::processRawData, &AcpcPvaPlugin::postRawData);
            break;
        case DATA_MODE_VERBOSE:
            // TODO
            break;
        case DATA_MODE_XY:
            setCallbacks(&AcpcPvaPlugin::processNormalData, &AcpcPvaPlugin::postNormalData);
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
        //flushData();
    } else if (pasynUser->reason == XyFractWidth) {
        m_xyDivider = 1 << value;
        return asynSuccess;
    } else if (pasynUser->reason == PsFractWidth) {
        m_photosumDivider = 1 << value;
        return asynSuccess;
    }
    return BasePvaPlugin::writeInt32(pasynUser, value);
}

void AcpcPvaPlugin::processNormalData(const uint32_t *data, uint32_t count)
{
    // Structure describing normal data from ACPC.
    struct NormalEvent {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t position_x;
        uint32_t position_y;
        uint32_t photo_sum_x;
        uint32_t photo_sum_y;
    };

    uint32_t nEvents = count / (sizeof(NormalEvent) / sizeof(uint32_t));
    const NormalEvent *event = reinterpret_cast<const NormalEvent *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(event->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(event->position_index & 0x7FFFFFFF);
        // In UQm.n format
        m_cache.position_x.push_back((1.0 * event->position_x) / m_xyDivider);
        m_cache.position_y.push_back((1.0 * event->position_y) / m_xyDivider);
        m_cache.photo_sum_x.push_back((1.0 * event->photo_sum_x) / m_photosumDivider);
        m_cache.photo_sum_y.push_back((1.0 * event->photo_sum_y) / m_photosumDivider);
        event++;
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

void AcpcPvaPlugin::processRawData(const uint32_t *data, uint32_t count)
{
    // Structure describing normal data from ACPC.
    struct RawEvent {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t sample_a[8];
        uint32_t sample_b[8];
    };

    uint32_t nEvents = count / (sizeof(RawEvent) / sizeof(uint32_t));
    const RawEvent *events = reinterpret_cast<const RawEvent *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(events->position_index & 0x7FFFFFFF);

        if (events->position_index & 0x10) {
            m_cache.sample_a1.push_back(events->sample_a[0]);
            m_cache.sample_a2.push_back(events->sample_a[1]);
            m_cache.sample_a3.push_back(events->sample_a[2]);
            m_cache.sample_a4.push_back(events->sample_a[3]);
            m_cache.sample_a5.push_back(events->sample_a[4]);
            m_cache.sample_a6.push_back(events->sample_a[5]);
            m_cache.sample_a7.push_back(events->sample_a[6]);
            m_cache.sample_a8.push_back(events->sample_a[7]);
        } else {
            m_cache.sample_a1.push_back(events->sample_a[7]);
            m_cache.sample_a2.push_back(events->sample_a[6]);
            m_cache.sample_a3.push_back(events->sample_a[5]);
            m_cache.sample_a4.push_back(events->sample_a[4]);
            m_cache.sample_a5.push_back(events->sample_a[3]);
            m_cache.sample_a6.push_back(events->sample_a[2]);
            m_cache.sample_a7.push_back(events->sample_a[1]);
            m_cache.sample_a8.push_back(events->sample_a[0]);
        }

        if (events->position_index & 0x20) {
            m_cache.sample_b1.push_back(events->sample_b[0]);
            m_cache.sample_b2.push_back(events->sample_b[1]);
            m_cache.sample_b3.push_back(events->sample_b[2]);
            m_cache.sample_b4.push_back(events->sample_b[3]);
            m_cache.sample_b5.push_back(events->sample_b[4]);
            m_cache.sample_b6.push_back(events->sample_b[5]);
            m_cache.sample_b7.push_back(events->sample_b[6]);
            m_cache.sample_b8.push_back(events->sample_b[7]);
        } else {
            m_cache.sample_b1.push_back(events->sample_b[7]);
            m_cache.sample_b2.push_back(events->sample_b[6]);
            m_cache.sample_b3.push_back(events->sample_b[5]);
            m_cache.sample_b4.push_back(events->sample_b[4]);
            m_cache.sample_b5.push_back(events->sample_b[3]);
            m_cache.sample_b6.push_back(events->sample_b[2]);
            m_cache.sample_b7.push_back(events->sample_b[1]);
            m_cache.sample_b8.push_back(events->sample_b[0]);
        }

        events++;
    }
}

void AcpcPvaPlugin::postRawData(const PvaNeutronData::shared_pointer& pvRecord)
{
    pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
    pvRecord->position_index->replace(freeze(m_cache.position_index));
    pvRecord->sample_a1->replace(freeze(m_cache.sample_a1));
    pvRecord->sample_a2->replace(freeze(m_cache.sample_a2));
    pvRecord->sample_a3->replace(freeze(m_cache.sample_a3));
    pvRecord->sample_a4->replace(freeze(m_cache.sample_a4));
    pvRecord->sample_a5->replace(freeze(m_cache.sample_a5));
    pvRecord->sample_a6->replace(freeze(m_cache.sample_a6));
    pvRecord->sample_a7->replace(freeze(m_cache.sample_a7));
    pvRecord->sample_a8->replace(freeze(m_cache.sample_a8));
    pvRecord->sample_b1->replace(freeze(m_cache.sample_b1));
    pvRecord->sample_b2->replace(freeze(m_cache.sample_b2));
    pvRecord->sample_b3->replace(freeze(m_cache.sample_b3));
    pvRecord->sample_b4->replace(freeze(m_cache.sample_b4));
    pvRecord->sample_b5->replace(freeze(m_cache.sample_b5));
    pvRecord->sample_b6->replace(freeze(m_cache.sample_b6));
    pvRecord->sample_b7->replace(freeze(m_cache.sample_b7));
    pvRecord->sample_b8->replace(freeze(m_cache.sample_b8));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.sample_a1.reserve(CACHE_SIZE);
    m_cache.sample_a2.reserve(CACHE_SIZE);
    m_cache.sample_a3.reserve(CACHE_SIZE);
    m_cache.sample_a4.reserve(CACHE_SIZE);
    m_cache.sample_a5.reserve(CACHE_SIZE);
    m_cache.sample_a6.reserve(CACHE_SIZE);
    m_cache.sample_a7.reserve(CACHE_SIZE);
    m_cache.sample_a8.reserve(CACHE_SIZE);
    m_cache.sample_b1.reserve(CACHE_SIZE);
    m_cache.sample_b2.reserve(CACHE_SIZE);
    m_cache.sample_b3.reserve(CACHE_SIZE);
    m_cache.sample_b4.reserve(CACHE_SIZE);
    m_cache.sample_b5.reserve(CACHE_SIZE);
    m_cache.sample_b6.reserve(CACHE_SIZE);
    m_cache.sample_b7.reserve(CACHE_SIZE);
    m_cache.sample_b8.reserve(CACHE_SIZE);
}

void AcpcPvaPlugin::flushData()
{
    m_cache.time_of_flight.clear();
    m_cache.position_index.clear();
    m_cache.position_x.clear();
    m_cache.position_y.clear();
    m_cache.photo_sum_x.clear();
    m_cache.photo_sum_y.clear();

    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.position_x.reserve(CACHE_SIZE);
    m_cache.position_y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);

    BasePvaPlugin::flushData();
}
