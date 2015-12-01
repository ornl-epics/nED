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

    createParam("FlatFieldEn",   asynParamInt32, &FlatFieldEn, 0); // WRITE - Normal data has been flat-field corrected
    // UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth",  asynParamInt32, &XyFractWidth, 24); // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth",  asynParamInt32, &PsFractWidth, 15); // WRITE - Number of fraction bits in PhotoSum data
    callParamCallbacks();
}

asynStatus AcpcPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        int flatfieldEn = 0;
        getIntegerParam(FlatFieldEn, &flatfieldEn);

        switch (value) {
        case DATA_MODE_NORMAL:
            if (flatfieldEn == 1) {
                setCallbacks(&AcpcPvaPlugin::processTofPixelData, &AcpcPvaPlugin::postTofPixelData);
            } else {
                setCallbacks(&AcpcPvaPlugin::processNormalData, &AcpcPvaPlugin::postNormalData);
            }
            break;
        case DATA_MODE_RAW:
            setCallbacks(&AcpcPvaPlugin::processRawData, &AcpcPvaPlugin::postRawData);
            break;
        case DATA_MODE_VERBOSE:
            // TODO
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
        flushData();
    } else if (pasynUser->reason == FlatFieldEn) {
        int dataMode = DATA_MODE_NORMAL;
        getIntegerParam(DataModeP, &dataMode);

        if (dataMode == DATA_MODE_NORMAL) {
            if (value == 1) {
                setCallbacks(&AcpcPvaPlugin::processTofPixelData, &AcpcPvaPlugin::postTofPixelData);
            } else {
                setCallbacks(&AcpcPvaPlugin::processNormalData, &AcpcPvaPlugin::postNormalData);
            }
        }
        flushData();
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
            for (int i = 0; i < 8; i++)
                m_cache.sample_a8.push_back(events->sample_a[i]);
        } else {
            for (int i = 7; i >= 0; i--)
                m_cache.sample_a8.push_back(events->sample_a[i]);
        }

        if (events->position_index & 0x20) {
            for (int i = 0; i < 8; i++)
                m_cache.sample_b8.push_back(events->sample_b[i]);
        } else {
            for (int i = 7; i >= 0; i--)
                m_cache.sample_b8.push_back(events->sample_b[i]);
        }

        events++;
    }
}

void AcpcPvaPlugin::postRawData(const PvaNeutronData::shared_pointer& pvRecord)
{
    pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
    pvRecord->position_index->replace(freeze(m_cache.position_index));
    pvRecord->sample_a8->replace(freeze(m_cache.sample_a8));
    pvRecord->sample_b8->replace(freeze(m_cache.sample_b8));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.sample_a8.reserve(8 * CACHE_SIZE);
    m_cache.sample_b8.reserve(8 * CACHE_SIZE);
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
