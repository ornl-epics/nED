#include "RocPvaPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(RocPvaPlugin, 3, "port name", string, "dispatcher port", string, "PV prefix", string);

const uint32_t RocPvaPlugin::CACHE_SIZE = 32*1024;

/**
 * Structure representing RAW mode data packet
 */
struct RawEvent {
    uint32_t time_of_flight;
    uint32_t position_index;
    uint32_t sample1;
    uint32_t sample2;
};

/**
 * Structure representing normal mode data packet
 */
struct NormalEvent {
    uint32_t timeStamp;
    uint32_t pixelID;
};

/**
 * Structure representing extended mode data packet
 */
struct ExtendedEvent {
    uint32_t time_of_flight;
    uint32_t position_index;
    uint32_t sample1;
    uint32_t sample2;
    uint32_t pixelID;
};

RocPvaPlugin::RocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix)
{
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.pixel.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.sample_a1.reserve(CACHE_SIZE);
    m_cache.sample_b1.reserve(CACHE_SIZE);
    m_cache.meta_time_of_flight.reserve(CACHE_SIZE);
    m_cache.meta_pixel.reserve(CACHE_SIZE);

    setCallbacks(&RocPvaPlugin::processNormalData, &RocPvaPlugin::postNormalData);
}

asynStatus RocPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
            setCallbacks(&RocPvaPlugin::processNormalData, &RocPvaPlugin::postNormalData);
            break;
        case DATA_MODE_RAW:
            setCallbacks(&RocPvaPlugin::processRawData, &RocPvaPlugin::postRawData);
            break;
        case DATA_MODE_EXTENDED:
            setCallbacks(&RocPvaPlugin::processExtendedData, &RocPvaPlugin::postExtendedData);
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
    }
    return BasePvaPlugin::writeInt32(pasynUser, value);
}

void RocPvaPlugin::processNormalData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(NormalEvent) / sizeof(uint32_t));
    const NormalEvent *events = reinterpret_cast<const NormalEvent *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->timeStamp);
        m_cache.pixel.push_back(events->pixelID);
        events++;
    }
}

void RocPvaPlugin::processRawData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(RawEvent) / sizeof(uint32_t));
    const RawEvent *events = reinterpret_cast<const RawEvent *>(data);

    /* Pull the least significant 16bits from sample1 and sample2 and
     * package them together as sample_a1; this combines the 1-A and 2-A
     * samples.  Repeat for the B samples.  Append each event to cache.
     */
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(events->position_index);
        m_cache.sample_a1.push_back(
            (events->sample1&0xffff) | ((events->sample2&0xffff)<<16) );
        m_cache.sample_b1.push_back(
            ((events->sample1&0xffff0000) >> 16) | (events->sample2&0xffff0000));
        events++;
    }
}

void RocPvaPlugin::processExtendedData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(ExtendedEvent) / sizeof(uint32_t));
    const ExtendedEvent *events = reinterpret_cast<const ExtendedEvent *>(data);

    /* Pull the least significant 16bits from sample1 and sample2 and
     * package them together as sample_a1; this combines the 1-A and 2-A
     * samples.  Repeat for the B samples.  Append each event to cache.
     */
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(events->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(events->position_index);
        m_cache.sample_a1.push_back(
            (events->sample1&0xffff) | ((events->sample2&0xffff)<<16) );
        m_cache.sample_b1.push_back(
            ((events->sample1&0xffff0000) >> 16) | (events->sample2&0xffff0000));
        m_cache.pixel.push_back(events->pixelID);
        events++;
    }
}

void RocPvaPlugin::processMetaData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(NormalEvent) / sizeof(uint32_t));
    const NormalEvent *events = reinterpret_cast<const NormalEvent *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.meta_time_of_flight.push_back(events->timeStamp & 0x000FFFFF );
        m_cache.meta_pixel.push_back(events->pixelID);
        events++;
    }
}

void RocPvaPlugin::postNormalData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->pixel->replace(freeze(m_cache.pixel));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.pixel.reserve(CACHE_SIZE);
}

void RocPvaPlugin::postRawData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->sample_a1->replace(freeze(m_cache.sample_a1));
    m_pvNeutrons->sample_b1->replace(freeze(m_cache.sample_b1));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.sample_a1.reserve(CACHE_SIZE);
    m_cache.sample_b1.reserve(CACHE_SIZE);
}

void RocPvaPlugin::postExtendedData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cache.time_of_flight));
    m_pvNeutrons->position_index->replace(freeze(m_cache.position_index));
    m_pvNeutrons->sample_a1->replace(freeze(m_cache.sample_a1));
    m_pvNeutrons->sample_b1->replace(freeze(m_cache.sample_b1));
    m_pvNeutrons->pixel->replace(freeze(m_cache.pixel));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.sample_a1.reserve(CACHE_SIZE);
    m_cache.sample_b1.reserve(CACHE_SIZE);
    m_cache.pixel.reserve(CACHE_SIZE);
}

void RocPvaPlugin::postMetaData()
{
    if (m_cache.meta_time_of_flight.size() > 0) {
        m_pvMetadata->time_of_flight->replace(freeze(m_cache.meta_time_of_flight));
        m_pvMetadata->pixel->replace(freeze(m_cache.meta_pixel));
    }

    m_cache.meta_time_of_flight.clear();
    m_cache.meta_pixel.clear();
    m_cache.meta_time_of_flight.reserve(CACHE_SIZE);
    m_cache.meta_pixel.reserve(CACHE_SIZE);
}
