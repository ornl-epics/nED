/* BnlRocPvaPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 */

#include "BnlRocPvaPlugin.h"
#include "BnlDataPacket.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(BnlRocPvaPlugin, 3, "port name", string, "dispatcher port", string, "PV prefix", string);

#define NUM_BNLROCPVAPLUGIN_PARAMS ((int)(&LAST_BNLROCPVAPLUGIN_PARAM - &FIRST_BNLROCPVAPLUGIN_PARAM + 1))

#define XY_FRACT_WIDTH 11 // Number of fractional bits in X and Y calculated position

const uint32_t BnlRocPvaPlugin::CACHE_SIZE = 32*1024;


BnlRocPvaPlugin::BnlRocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix, NUM_BNLROCPVAPLUGIN_PARAMS)
    , m_xyDivider(1 << 11)
{
    // Pre-allocate vectors
    reserve();

    // Select default handler
    setCallbacks(&BnlRocPvaPlugin::processTofPixelData, &BnlRocPvaPlugin::postTofPixelData);

    // Create PV parameters
    createParam("XyFractWidth",     asynParamInt32, &XyFractWidth, 11);    // WRITE - Number of fraction bits in X,Y data
    callParamCallbacks();
}

asynStatus BnlRocPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
            // Normal mode for this plugin is the tof,pixel format
            // even though the normal mode for BNL ROC is X,Y format
            setCallbacks(&BnlRocPvaPlugin::processTofPixelData, &BnlRocPvaPlugin::postTofPixelData);
            break;
        case DATA_MODE_RAW:
            setCallbacks(&BnlRocPvaPlugin::processRawData, &BnlRocPvaPlugin::postRawData);
            break;
        case DATA_MODE_EXTENDED:
            setCallbacks(&BnlRocPvaPlugin::processExtendedData, &BnlRocPvaPlugin::postExtendedData);
            break;
        case DATA_MODE_XY:
            setCallbacks(&BnlRocPvaPlugin::processNormalData, &BnlRocPvaPlugin::postNormalData);
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
        flushData();
        return asynSuccess;
    } else if (pasynUser->reason == XyFractWidth) {
        if (value < 0 || value > 15)
            return asynError;
        m_xyDivider = 1 << value;
        return asynSuccess;
    }
    return BasePvaPlugin::writeInt32(pasynUser, value);
}

void BnlRocPvaPlugin::processNormalData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(BnlDataPacket::NormalEvent) / sizeof(uint32_t));
    const BnlDataPacket::NormalEvent *events = reinterpret_cast<const BnlDataPacket::NormalEvent *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->tof);
        m_cache.position_index.push_back(events->position);
        m_cache.position_x.push_back(events->x / m_xyDivider);
        m_cache.position_y.push_back(events->y / m_xyDivider);
        m_cache.photo_sum_x.push_back(events->unused1);
        m_cache.photo_sum_y.push_back(events->unused2);
        events++;
    }
}

void BnlRocPvaPlugin::processRawData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(BnlDataPacket::RawEvent) / sizeof(uint32_t));
    const BnlDataPacket::RawEvent *events = reinterpret_cast<const BnlDataPacket::RawEvent *>(data);

    /* Pull the least significant 16bits from sample1 and sample2 and
     * package them together as sample_a1; this combines the 1-A and 2-A
     * samples.  Repeat for the B samples.  Append each event to cache.
     */
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->tof & 0x000FFFFF);
        m_cache.position_index.push_back(events->position);
        m_cache.sample_x1.push_back(events->sample_x1);
        m_cache.sample_x2.push_back(events->sample_x2);
        m_cache.sample_x3.push_back(events->sample_x3);
        m_cache.sample_x4.push_back(events->sample_x4);
        m_cache.sample_x5.push_back(events->sample_x5);
        m_cache.sample_x6.push_back(events->sample_x6);
        m_cache.sample_x7.push_back(events->sample_x7);
        m_cache.sample_x8.push_back(events->sample_x8);
        m_cache.sample_x9.push_back(events->sample_x9);
        m_cache.sample_x10.push_back(events->sample_x10);
        m_cache.sample_x11.push_back(events->sample_x11);
        m_cache.sample_x12.push_back(events->sample_x12);
        m_cache.sample_x13.push_back(events->sample_x13);
        m_cache.sample_x14.push_back(events->sample_x14);
        m_cache.sample_x15.push_back(events->sample_x15);
        m_cache.sample_x16.push_back(events->sample_x16);
        m_cache.sample_x17.push_back(events->sample_x17);
        m_cache.sample_x18.push_back(events->sample_x18);
        m_cache.sample_x19.push_back(events->sample_x19);
        m_cache.sample_x20.push_back(events->sample_x20);
        m_cache.sample_y1.push_back(events->sample_y1);
        m_cache.sample_y2.push_back(events->sample_y2);
        m_cache.sample_y3.push_back(events->sample_y3);
        m_cache.sample_y4.push_back(events->sample_y4);
        m_cache.sample_y5.push_back(events->sample_y5);
        m_cache.sample_y6.push_back(events->sample_y6);
        m_cache.sample_y7.push_back(events->sample_y7);
        m_cache.sample_y8.push_back(events->sample_y8);
        m_cache.sample_y9.push_back(events->sample_y9);
        m_cache.sample_y10.push_back(events->sample_y10);
        m_cache.sample_y11.push_back(events->sample_y11);
        m_cache.sample_y12.push_back(events->sample_y12);
        m_cache.sample_y13.push_back(events->sample_y13);
        m_cache.sample_y14.push_back(events->sample_y14);
        m_cache.sample_y15.push_back(events->sample_y15);
        m_cache.sample_y16.push_back(events->sample_y16);
        m_cache.sample_y17.push_back(events->sample_y17);
        events++;
    }
}

void BnlRocPvaPlugin::processExtendedData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(BnlDataPacket::ExtendedEvent) / sizeof(uint32_t));
    const BnlDataPacket::ExtendedEvent *events = reinterpret_cast<const BnlDataPacket::ExtendedEvent *>(data);

    /* Pull the least significant 16bits from sample1 and sample2 and
     * package them together as sample_a1; this combines the 1-A and 2-A
     * samples.  Repeat for the B samples.  Append each event to cache.
     */
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->tof & 0x000FFFFF);
        m_cache.position_index.push_back(events->position);
        m_cache.sample_x1.push_back(events->sample_x1);
        m_cache.sample_x2.push_back(events->sample_x2);
        m_cache.sample_x3.push_back(events->sample_x3);
        m_cache.sample_x4.push_back(events->sample_x4);
        m_cache.sample_x5.push_back(events->sample_x5);
        m_cache.sample_x6.push_back(events->sample_x6);
        m_cache.sample_x7.push_back(events->sample_x7);
        m_cache.sample_x8.push_back(events->sample_x8);
        m_cache.sample_x9.push_back(events->sample_x9);
        m_cache.sample_x10.push_back(events->sample_x10);
        m_cache.sample_x11.push_back(events->sample_x11);
        m_cache.sample_x12.push_back(events->sample_x12);
        m_cache.sample_x13.push_back(events->sample_x13);
        m_cache.sample_x14.push_back(events->sample_x14);
        m_cache.sample_x15.push_back(events->sample_x15);
        m_cache.sample_x16.push_back(events->sample_x16);
        m_cache.sample_x17.push_back(events->sample_x17);
        m_cache.sample_x18.push_back(events->sample_x18);
        m_cache.sample_x19.push_back(events->sample_x19);
        m_cache.sample_x20.push_back(events->sample_x20);
        m_cache.sample_y1.push_back(events->sample_y1);
        m_cache.sample_y2.push_back(events->sample_y2);
        m_cache.sample_y3.push_back(events->sample_y3);
        m_cache.sample_y4.push_back(events->sample_y4);
        m_cache.sample_y5.push_back(events->sample_y5);
        m_cache.sample_y6.push_back(events->sample_y6);
        m_cache.sample_y7.push_back(events->sample_y7);
        m_cache.sample_y8.push_back(events->sample_y8);
        m_cache.sample_y9.push_back(events->sample_y9);
        m_cache.sample_y10.push_back(events->sample_y10);
        m_cache.sample_y11.push_back(events->sample_y11);
        m_cache.sample_y12.push_back(events->sample_y12);
        m_cache.sample_y13.push_back(events->sample_y13);
        m_cache.sample_y14.push_back(events->sample_y14);
        m_cache.sample_y15.push_back(events->sample_y15);
        m_cache.sample_y16.push_back(events->sample_y16);
        m_cache.sample_y17.push_back(events->sample_y17);
        m_cache.position_x.push_back(events->x / m_xyDivider);
        m_cache.position_y.push_back(events->y / m_xyDivider);
        events++;
    }
}

void BnlRocPvaPlugin::postNormalData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->position_x->replace(freeze(m_cache.position_x));
    m_pvNeutrons->position_y->replace(freeze(m_cache.position_y));
    m_pvNeutrons->photo_sum_x->replace(freeze(m_cache.photo_sum_x));
    m_pvNeutrons->photo_sum_y->replace(freeze(m_cache.photo_sum_y));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.position_x.reserve(CACHE_SIZE);
    m_cache.position_y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);
}

void BnlRocPvaPlugin::postRawData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));

    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->sample_x1->replace(freeze(m_cache.sample_x1));
    m_pvNeutrons->sample_x2->replace(freeze(m_cache.sample_x2));
    m_pvNeutrons->sample_x3->replace(freeze(m_cache.sample_x3));
    m_pvNeutrons->sample_x4->replace(freeze(m_cache.sample_x4));
    m_pvNeutrons->sample_x5->replace(freeze(m_cache.sample_x5));
    m_pvNeutrons->sample_x6->replace(freeze(m_cache.sample_x6));
    m_pvNeutrons->sample_x7->replace(freeze(m_cache.sample_x7));
    m_pvNeutrons->sample_x8->replace(freeze(m_cache.sample_x8));
    m_pvNeutrons->sample_x9->replace(freeze(m_cache.sample_x9));
    m_pvNeutrons->sample_x10->replace(freeze(m_cache.sample_x10));
    m_pvNeutrons->sample_x11->replace(freeze(m_cache.sample_x11));
    m_pvNeutrons->sample_x12->replace(freeze(m_cache.sample_x12));
    m_pvNeutrons->sample_x13->replace(freeze(m_cache.sample_x13));
    m_pvNeutrons->sample_x14->replace(freeze(m_cache.sample_x14));
    m_pvNeutrons->sample_x15->replace(freeze(m_cache.sample_x15));
    m_pvNeutrons->sample_x16->replace(freeze(m_cache.sample_x16));
    m_pvNeutrons->sample_x17->replace(freeze(m_cache.sample_x17));
    m_pvNeutrons->sample_x18->replace(freeze(m_cache.sample_x18));
    m_pvNeutrons->sample_x19->replace(freeze(m_cache.sample_x19));
    m_pvNeutrons->sample_x20->replace(freeze(m_cache.sample_x20));
    m_pvNeutrons->sample_y1->replace(freeze(m_cache.sample_y1));
    m_pvNeutrons->sample_y2->replace(freeze(m_cache.sample_y2));
    m_pvNeutrons->sample_y3->replace(freeze(m_cache.sample_y3));
    m_pvNeutrons->sample_y4->replace(freeze(m_cache.sample_y4));
    m_pvNeutrons->sample_y5->replace(freeze(m_cache.sample_y5));
    m_pvNeutrons->sample_y6->replace(freeze(m_cache.sample_y6));
    m_pvNeutrons->sample_y7->replace(freeze(m_cache.sample_y7));
    m_pvNeutrons->sample_y8->replace(freeze(m_cache.sample_y8));
    m_pvNeutrons->sample_y9->replace(freeze(m_cache.sample_y9));
    m_pvNeutrons->sample_y10->replace(freeze(m_cache.sample_y10));
    m_pvNeutrons->sample_y11->replace(freeze(m_cache.sample_y11));
    m_pvNeutrons->sample_y12->replace(freeze(m_cache.sample_y12));
    m_pvNeutrons->sample_y13->replace(freeze(m_cache.sample_y13));
    m_pvNeutrons->sample_y14->replace(freeze(m_cache.sample_y14));
    m_pvNeutrons->sample_y15->replace(freeze(m_cache.sample_y15));
    m_pvNeutrons->sample_y16->replace(freeze(m_cache.sample_y16));
    m_pvNeutrons->sample_y17->replace(freeze(m_cache.sample_y17));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    reserve();
}

void BnlRocPvaPlugin::postExtendedData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->sample_x1->replace(freeze(m_cache.sample_x1));
    m_pvNeutrons->sample_x2->replace(freeze(m_cache.sample_x2));
    m_pvNeutrons->sample_x3->replace(freeze(m_cache.sample_x3));
    m_pvNeutrons->sample_x4->replace(freeze(m_cache.sample_x4));
    m_pvNeutrons->sample_x5->replace(freeze(m_cache.sample_x5));
    m_pvNeutrons->sample_x6->replace(freeze(m_cache.sample_x6));
    m_pvNeutrons->sample_x7->replace(freeze(m_cache.sample_x7));
    m_pvNeutrons->sample_x8->replace(freeze(m_cache.sample_x8));
    m_pvNeutrons->sample_x9->replace(freeze(m_cache.sample_x9));
    m_pvNeutrons->sample_x10->replace(freeze(m_cache.sample_x10));
    m_pvNeutrons->sample_x11->replace(freeze(m_cache.sample_x11));
    m_pvNeutrons->sample_x12->replace(freeze(m_cache.sample_x12));
    m_pvNeutrons->sample_x13->replace(freeze(m_cache.sample_x13));
    m_pvNeutrons->sample_x14->replace(freeze(m_cache.sample_x14));
    m_pvNeutrons->sample_x15->replace(freeze(m_cache.sample_x15));
    m_pvNeutrons->sample_x16->replace(freeze(m_cache.sample_x16));
    m_pvNeutrons->sample_x17->replace(freeze(m_cache.sample_x17));
    m_pvNeutrons->sample_x18->replace(freeze(m_cache.sample_x18));
    m_pvNeutrons->sample_x19->replace(freeze(m_cache.sample_x19));
    m_pvNeutrons->sample_x20->replace(freeze(m_cache.sample_x20));
    m_pvNeutrons->sample_y1->replace(freeze(m_cache.sample_y1));
    m_pvNeutrons->sample_y2->replace(freeze(m_cache.sample_y2));
    m_pvNeutrons->sample_y3->replace(freeze(m_cache.sample_y3));
    m_pvNeutrons->sample_y4->replace(freeze(m_cache.sample_y4));
    m_pvNeutrons->sample_y5->replace(freeze(m_cache.sample_y5));
    m_pvNeutrons->sample_y6->replace(freeze(m_cache.sample_y6));
    m_pvNeutrons->sample_y7->replace(freeze(m_cache.sample_y7));
    m_pvNeutrons->sample_y8->replace(freeze(m_cache.sample_y8));
    m_pvNeutrons->sample_y9->replace(freeze(m_cache.sample_y9));
    m_pvNeutrons->sample_y10->replace(freeze(m_cache.sample_y10));
    m_pvNeutrons->sample_y11->replace(freeze(m_cache.sample_y11));
    m_pvNeutrons->sample_y12->replace(freeze(m_cache.sample_y12));
    m_pvNeutrons->sample_y13->replace(freeze(m_cache.sample_y13));
    m_pvNeutrons->sample_y14->replace(freeze(m_cache.sample_y14));
    m_pvNeutrons->sample_y15->replace(freeze(m_cache.sample_y15));
    m_pvNeutrons->sample_y16->replace(freeze(m_cache.sample_y16));
    m_pvNeutrons->sample_y17->replace(freeze(m_cache.sample_y17));
    m_pvNeutrons->position_x->replace(freeze(m_cache.position_x));
    m_pvNeutrons->position_y->replace(freeze(m_cache.position_y));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    reserve();
}

void BnlRocPvaPlugin::flushData()
{
    m_cache.time_of_flight.clear();
    m_cache.position_index.clear();
    m_cache.position_x.clear();
    m_cache.position_y.clear();
    m_cache.photo_sum_x.clear();
    m_cache.photo_sum_y.clear();
    m_cache.sample_x1.clear();
    m_cache.sample_x2.clear();
    m_cache.sample_x3.clear();
    m_cache.sample_x4.clear();
    m_cache.sample_x5.clear();
    m_cache.sample_x6.clear();
    m_cache.sample_x7.clear();
    m_cache.sample_x8.clear();
    m_cache.sample_x9.clear();
    m_cache.sample_x10.clear();
    m_cache.sample_x11.clear();
    m_cache.sample_x12.clear();
    m_cache.sample_x13.clear();
    m_cache.sample_x14.clear();
    
    m_cache.sample_x15.clear();
    m_cache.sample_x16.clear();
    m_cache.sample_x17.clear();
    m_cache.sample_x18.clear();
    m_cache.sample_x19.clear();
    m_cache.sample_x20.clear();
    m_cache.sample_y1.clear();
    m_cache.sample_y2.clear();
    m_cache.sample_y3.clear();
    m_cache.sample_y4.clear();
    m_cache.sample_y5.clear();
    m_cache.sample_y6.clear();
    m_cache.sample_y7.clear();
    m_cache.sample_y8.clear();
    m_cache.sample_y9.clear();
    m_cache.sample_y10.clear();
    m_cache.sample_y11.clear();
    m_cache.sample_y12.clear();
    m_cache.sample_y13.clear();
    m_cache.sample_y14.clear();
    m_cache.sample_y15.clear();
    m_cache.sample_y16.clear();
    m_cache.sample_y17.clear();

    reserve();

    BasePvaPlugin::flushData();
}

void BnlRocPvaPlugin::reserve()
{
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.position_x.reserve(CACHE_SIZE);
    m_cache.position_y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);
    m_cache.sample_x1.reserve(CACHE_SIZE);
    m_cache.sample_x2.reserve(CACHE_SIZE);
    m_cache.sample_x3.reserve(CACHE_SIZE);
    m_cache.sample_x4.reserve(CACHE_SIZE);
    m_cache.sample_x5.reserve(CACHE_SIZE);
    m_cache.sample_x6.reserve(CACHE_SIZE);
    m_cache.sample_x7.reserve(CACHE_SIZE);
    m_cache.sample_x8.reserve(CACHE_SIZE);
    m_cache.sample_x9.reserve(CACHE_SIZE);
    m_cache.sample_x10.reserve(CACHE_SIZE);
    m_cache.sample_x11.reserve(CACHE_SIZE);
    m_cache.sample_x12.reserve(CACHE_SIZE);
    m_cache.sample_x13.reserve(CACHE_SIZE);
    m_cache.sample_x14.reserve(CACHE_SIZE);
    m_cache.sample_x15.reserve(CACHE_SIZE);
    m_cache.sample_x16.reserve(CACHE_SIZE);
    m_cache.sample_x17.reserve(CACHE_SIZE);
    m_cache.sample_x18.reserve(CACHE_SIZE);
    m_cache.sample_x19.reserve(CACHE_SIZE);
    m_cache.sample_x20.reserve(CACHE_SIZE);
    m_cache.sample_y1.reserve(CACHE_SIZE);
    m_cache.sample_y2.reserve(CACHE_SIZE);
    m_cache.sample_y3.reserve(CACHE_SIZE);
    m_cache.sample_y4.reserve(CACHE_SIZE);
    m_cache.sample_y5.reserve(CACHE_SIZE);
    m_cache.sample_y6.reserve(CACHE_SIZE);
    m_cache.sample_y7.reserve(CACHE_SIZE);
    m_cache.sample_y8.reserve(CACHE_SIZE);
    m_cache.sample_y9.reserve(CACHE_SIZE);
    m_cache.sample_y10.reserve(CACHE_SIZE);
    m_cache.sample_y11.reserve(CACHE_SIZE);
    m_cache.sample_y12.reserve(CACHE_SIZE);
    m_cache.sample_y13.reserve(CACHE_SIZE);
    m_cache.sample_y14.reserve(CACHE_SIZE);
    m_cache.sample_y15.reserve(CACHE_SIZE);
    m_cache.sample_y16.reserve(CACHE_SIZE);
    m_cache.sample_y17.reserve(CACHE_SIZE);
}
