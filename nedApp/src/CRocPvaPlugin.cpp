/* CRocPvaPlugin.cpp
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 */

#include "CRocPvaPlugin.h"
#include "CRocDataPacket.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(CRocPvaPlugin, 3, "port name", string, "dispatcher port", string, "PV prefix", string);

const uint32_t CRocPvaPlugin::CACHE_SIZE = 32*1024;


CRocPvaPlugin::CRocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix, 0)
{
    // Pre-allocate vectors
    reserve();

    // Select default handler
    setCallbacks(&CRocPvaPlugin::processTofPixelData, &CRocPvaPlugin::postTofPixelData);
}

asynStatus CRocPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
            setCallbacks(&CRocPvaPlugin::processTofPixelData, &CRocPvaPlugin::postTofPixelData);
            break;
        case DATA_MODE_RAW:
            setCallbacks(&CRocPvaPlugin::processRawData, &CRocPvaPlugin::postRawData);
            break;
        case DATA_MODE_EXTENDED:
            setCallbacks(&CRocPvaPlugin::processExtendedData, &CRocPvaPlugin::postExtendedData);
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
        flushData();
        return asynSuccess;
   }
    return BasePvaPlugin::writeInt32(pasynUser, value);
}

void CRocPvaPlugin::processRawData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(CRocDataPacket::RawEvent) / sizeof(uint32_t));
    const CRocDataPacket::RawEvent *events = reinterpret_cast<const CRocDataPacket::RawEvent *>(data);

    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->tof & 0x000FFFFF);
        m_cache.position_index.push_back(events->position);
        for (int i=0; i<14; i++)
            m_cache.photon_count_g[i].push_back(events->photon_count_g[i]);
        for (int i=0; i<11; i++)
            m_cache.photon_count_x[i].push_back(events->photon_count_x[i]);
        for (int i=0; i<7; i++)
            m_cache.photon_count_y[i].push_back(events->photon_count_y[i]);
        for (int i=0; i<4; i++)
            m_cache.time_range[i].push_back(events->time_range[i]);
        events++;
    }
}

void CRocPvaPlugin::processExtendedData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(CRocDataPacket::ExtendedEvent) / sizeof(uint32_t));
    const CRocDataPacket::ExtendedEvent *events = reinterpret_cast<const CRocDataPacket::ExtendedEvent *>(data);

    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->tof & 0x000FFFFF);
        m_cache.position_index.push_back(events->position);
        m_cache.pixel.push_back(events->pixelid);
        for (int i=0; i<14; i++) {
            m_cache.photon_count_g[i].push_back(events->photon_count_g[i]);
        }
        for (int i=0; i<11; i++) {
            m_cache.photon_count_x[i].push_back(events->photon_count_x[i]);
        }
        for (int i=0; i<7; i++) {
            m_cache.photon_count_y[i].push_back(events->photon_count_y[i]);
        }
        for (int i=0; i<4; i++) {
            m_cache.time_range[i].push_back(events->time_range[i]);
        }
        events++;
    }
}

void CRocPvaPlugin::postRawData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->sample_g1->replace(freeze(m_cache.photon_count_g[0]));
    m_pvNeutrons->sample_g2->replace(freeze(m_cache.photon_count_g[1]));
    m_pvNeutrons->sample_g3->replace(freeze(m_cache.photon_count_g[2]));
    m_pvNeutrons->sample_g4->replace(freeze(m_cache.photon_count_g[3]));
    m_pvNeutrons->sample_g5->replace(freeze(m_cache.photon_count_g[4]));
    m_pvNeutrons->sample_g6->replace(freeze(m_cache.photon_count_g[5]));
    m_pvNeutrons->sample_g7->replace(freeze(m_cache.photon_count_g[6]));
    m_pvNeutrons->sample_g8->replace(freeze(m_cache.photon_count_g[7]));
    m_pvNeutrons->sample_g9->replace(freeze(m_cache.photon_count_g[8]));
    m_pvNeutrons->sample_g10->replace(freeze(m_cache.photon_count_g[9]));
    m_pvNeutrons->sample_g11->replace(freeze(m_cache.photon_count_g[10]));
    m_pvNeutrons->sample_g12->replace(freeze(m_cache.photon_count_g[11]));
    m_pvNeutrons->sample_g13->replace(freeze(m_cache.photon_count_g[12]));
    m_pvNeutrons->sample_g14->replace(freeze(m_cache.photon_count_g[13]));
    m_pvNeutrons->sample_x1->replace(freeze(m_cache.photon_count_x[0]));
    m_pvNeutrons->sample_x2->replace(freeze(m_cache.photon_count_x[1]));
    m_pvNeutrons->sample_x3->replace(freeze(m_cache.photon_count_x[2]));
    m_pvNeutrons->sample_x4->replace(freeze(m_cache.photon_count_x[3]));
    m_pvNeutrons->sample_x5->replace(freeze(m_cache.photon_count_x[4]));
    m_pvNeutrons->sample_x6->replace(freeze(m_cache.photon_count_x[5]));
    m_pvNeutrons->sample_x7->replace(freeze(m_cache.photon_count_x[6]));
    m_pvNeutrons->sample_x8->replace(freeze(m_cache.photon_count_x[7]));
    m_pvNeutrons->sample_x9->replace(freeze(m_cache.photon_count_x[8]));
    m_pvNeutrons->sample_x10->replace(freeze(m_cache.photon_count_x[9]));
    m_pvNeutrons->sample_x11->replace(freeze(m_cache.photon_count_x[10]));
    m_pvNeutrons->sample_y1->replace(freeze(m_cache.photon_count_y[0]));
    m_pvNeutrons->sample_y2->replace(freeze(m_cache.photon_count_y[1]));
    m_pvNeutrons->sample_y3->replace(freeze(m_cache.photon_count_y[2]));
    m_pvNeutrons->sample_y4->replace(freeze(m_cache.photon_count_y[3]));
    m_pvNeutrons->sample_y5->replace(freeze(m_cache.photon_count_y[4]));
    m_pvNeutrons->sample_y6->replace(freeze(m_cache.photon_count_y[5]));
    m_pvNeutrons->sample_y7->replace(freeze(m_cache.photon_count_y[6]));
    m_pvNeutrons->time_range1->replace(freeze(m_cache.time_range[0]));
    m_pvNeutrons->time_range2->replace(freeze(m_cache.time_range[1]));
    m_pvNeutrons->time_range3->replace(freeze(m_cache.time_range[2]));
    m_pvNeutrons->time_range4->replace(freeze(m_cache.time_range[3]));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    reserve();
}

void CRocPvaPlugin::postExtendedData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->pixel->replace(freeze(m_cache.pixel));
    m_pvNeutrons->sample_g1->replace(freeze(m_cache.photon_count_g[0]));
    m_pvNeutrons->sample_g2->replace(freeze(m_cache.photon_count_g[1]));
    m_pvNeutrons->sample_g3->replace(freeze(m_cache.photon_count_g[2]));
    m_pvNeutrons->sample_g4->replace(freeze(m_cache.photon_count_g[3]));
    m_pvNeutrons->sample_g5->replace(freeze(m_cache.photon_count_g[4]));
    m_pvNeutrons->sample_g6->replace(freeze(m_cache.photon_count_g[5]));
    m_pvNeutrons->sample_g7->replace(freeze(m_cache.photon_count_g[6]));
    m_pvNeutrons->sample_g8->replace(freeze(m_cache.photon_count_g[7]));
    m_pvNeutrons->sample_g9->replace(freeze(m_cache.photon_count_g[8]));
    m_pvNeutrons->sample_g10->replace(freeze(m_cache.photon_count_g[9]));
    m_pvNeutrons->sample_g11->replace(freeze(m_cache.photon_count_g[10]));
    m_pvNeutrons->sample_g12->replace(freeze(m_cache.photon_count_g[11]));
    m_pvNeutrons->sample_g13->replace(freeze(m_cache.photon_count_g[12]));
    m_pvNeutrons->sample_g14->replace(freeze(m_cache.photon_count_g[13]));
    m_pvNeutrons->sample_x1->replace(freeze(m_cache.photon_count_x[0]));
    m_pvNeutrons->sample_x2->replace(freeze(m_cache.photon_count_x[1]));
    m_pvNeutrons->sample_x3->replace(freeze(m_cache.photon_count_x[2]));
    m_pvNeutrons->sample_x4->replace(freeze(m_cache.photon_count_x[3]));
    m_pvNeutrons->sample_x5->replace(freeze(m_cache.photon_count_x[4]));
    m_pvNeutrons->sample_x6->replace(freeze(m_cache.photon_count_x[5]));
    m_pvNeutrons->sample_x7->replace(freeze(m_cache.photon_count_x[6]));
    m_pvNeutrons->sample_x8->replace(freeze(m_cache.photon_count_x[7]));
    m_pvNeutrons->sample_x9->replace(freeze(m_cache.photon_count_x[8]));
    m_pvNeutrons->sample_x10->replace(freeze(m_cache.photon_count_x[9]));
    m_pvNeutrons->sample_x11->replace(freeze(m_cache.photon_count_x[10]));
    m_pvNeutrons->sample_y1->replace(freeze(m_cache.photon_count_y[0]));
    m_pvNeutrons->sample_y2->replace(freeze(m_cache.photon_count_y[1]));
    m_pvNeutrons->sample_y3->replace(freeze(m_cache.photon_count_y[2]));
    m_pvNeutrons->sample_y4->replace(freeze(m_cache.photon_count_y[3]));
    m_pvNeutrons->sample_y5->replace(freeze(m_cache.photon_count_y[4]));
    m_pvNeutrons->sample_y6->replace(freeze(m_cache.photon_count_y[5]));
    m_pvNeutrons->sample_y7->replace(freeze(m_cache.photon_count_y[6]));
    m_pvNeutrons->time_range1->replace(freeze(m_cache.time_range[0]));
    m_pvNeutrons->time_range2->replace(freeze(m_cache.time_range[1]));
    m_pvNeutrons->time_range3->replace(freeze(m_cache.time_range[2]));
    m_pvNeutrons->time_range4->replace(freeze(m_cache.time_range[3]));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    reserve();
}

void CRocPvaPlugin::flushData()
{
    m_cache.time_of_flight.clear();
    m_cache.position_index.clear();
    m_cache.pixel.clear();
    for (int i=0; i<14; i++)
        m_cache.photon_count_g[i].clear();
    for (int i=0; i<11; i++)
        m_cache.photon_count_x[i].clear();
    for (int i=0; i<7; i++)
        m_cache.photon_count_y[i].clear();
    for (int i=0; i<4; i++)
        m_cache.time_range[i].clear();

    reserve();

    BasePvaPlugin::flushData();
}

void CRocPvaPlugin::reserve()
{
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.pixel.reserve(CACHE_SIZE);
    for (int i=0; i<14; i++)
        m_cache.photon_count_g[i].reserve(CACHE_SIZE);
    for (int i=0; i<11; i++)
        m_cache.photon_count_x[i].reserve(CACHE_SIZE);
    for (int i=0; i<7; i++)
        m_cache.photon_count_y[i].reserve(CACHE_SIZE);
    for (int i=0; i<4; i++)
        m_cache.time_range[i].reserve(CACHE_SIZE);
}
