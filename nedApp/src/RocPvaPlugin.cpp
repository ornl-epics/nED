#include "RocPvaPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(RocPvaPlugin, 3, "port name", string, "dispatcher port", string, "raw_cal_data", string);

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
    uint32_t maxNormalEventsPerPacket = (DasPacket::MaxLength/4) / 6;

    if (!!m_pvRecord) {
        // Guestimate container size and force memory pre-allocation,
        // will automatically extend if needed
        m_cache.time_of_flight.reserve(maxNormalEventsPerPacket);
	    m_cache.pixel.reserve(maxNormalEventsPerPacket);
        m_cache.position_index.reserve(maxNormalEventsPerPacket);
        m_cache.sample_a1.reserve(maxNormalEventsPerPacket);
        m_cache.sample_b1.reserve(maxNormalEventsPerPacket);
    }

    setCallbacks(&RocPvaPlugin::processNormalPacket, &RocPvaPlugin::postNormalData);
}

asynStatus RocPvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
            setCallbacks(&RocPvaPlugin::processNormalPacket,
                &RocPvaPlugin::postNormalData);
            break;
        case DATA_MODE_RAW:
            setCallbacks(&RocPvaPlugin::processRawPacket,
                &RocPvaPlugin::postRawData);
            break;
        case DATA_MODE_EXTENDED:
            setCallbacks(&RocPvaPlugin::processExtendedPacket,
                &RocPvaPlugin::postExtendedData);
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void RocPvaPlugin::processNormalPacket(const DasPacket * const packet)
{
    uint32_t nEvents;
    const struct NormalEvent *data =
        reinterpret_cast<const NormalEvent *>(packet->getData(&nEvents));
    nEvents /= sizeof(NormalEvent) / sizeof(uint32_t);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(data->timeStamp);
        m_cache.pixel.push_back(data->pixelID);
        data++;
    }
}

void RocPvaPlugin::processRawPacket(const DasPacket * const packet)
{
    uint32_t nEvents;
    const struct RawEvent *data =
        reinterpret_cast<const RawEvent *>(packet->getData(&nEvents));
    nEvents /= sizeof(RawEvent) / sizeof(uint32_t);

    /* Pull the least significant 16bits from sample1 and sample2 and
     * package them together as sample_a1; this combines the 1-A and 2-A
     * samples.  Repeat for the B samples.  Append each event to cache.
     */
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(data->time_of_flight);
        m_cache.position_index.push_back(data->position_index);
        m_cache.sample_a1.push_back(
            (data->sample1&0xffff) | ((data->sample2&0xffff)<<16) );
        m_cache.sample_b1.push_back(
            ((data->sample1&0xffff0000) >> 16) | (data->sample2&0xffff0000));
        data++;
    }
}

void RocPvaPlugin::processExtendedPacket(const DasPacket * const packet)
{
    uint32_t nEvents;
    const struct ExtendedEvent *data =
        reinterpret_cast<const ExtendedEvent *>(packet->getData(&nEvents));
    nEvents /= sizeof(ExtendedEvent) / sizeof(uint32_t);

    /* Pull the least significant 16bits from sample1 and sample2 and
     * package them together as sample_a1; this combines the 1-A and 2-A
     * samples.  Repeat for the B samples.  Append each event to cache.
     */
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(data->time_of_flight);
        m_cache.position_index.push_back(data->position_index);
        m_cache.sample_a1.push_back(
            (data->sample1&0xffff) | ((data->sample2&0xffff)<<16) );
        m_cache.sample_b1.push_back(
            ((data->sample1&0xffff0000) >> 16) | (data->sample2&0xffff0000));
        m_cache.pixel.push_back(data->pixelID);
        data++;
    }
}


void RocPvaPlugin::postNormalData(const epicsTimeStamp &pulseTime,
    double pulseCharge, uint32_t pulseSeq)
{
    if (!!m_pvRecord) {
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + pulseTime.secPastEpoch, pulseTime.nsec,
            pulseSeq);
        m_pvRecord->beginGroupPut();
        m_pvRecord->timeStamp.set(time);
        m_pvRecord->proton_charge->putFrom(pulseCharge);
        m_pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
        m_pvRecord->pixel->replace(freeze(m_cache.pixel));
        m_pvRecord->endGroupPut();
    } else {
        LOG_WARN("No PV record, skipping posting update");
    }

    m_cache.time_of_flight.clear();
    m_cache.pixel.clear();
}

void RocPvaPlugin::postRawData(const epicsTimeStamp &pulseTime,
    double pulseCharge, uint32_t pulseSeq)
{
    if (!!m_pvRecord) {
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + pulseTime.secPastEpoch, pulseTime.nsec,
            pulseSeq);
        m_pvRecord->beginGroupPut();
        m_pvRecord->timeStamp.set(time);
        m_pvRecord->proton_charge->putFrom(pulseCharge);
        m_pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
        m_pvRecord->position_index->replace(freeze(m_cache.position_index));
        m_pvRecord->sample_a1->replace(freeze(m_cache.sample_a1));
        m_pvRecord->sample_b1->replace(freeze(m_cache.sample_b1));
        m_pvRecord->endGroupPut();
    } else {
        LOG_WARN("No PV record, skipping posting update");
    }

    m_cache.time_of_flight.clear();
    m_cache.position_index.clear();
    m_cache.sample_a1.clear();
    m_cache.sample_b1.clear();
}

void RocPvaPlugin::postExtendedData(const epicsTimeStamp &pulseTime,
    double pulseCharge, uint32_t pulseSeq)
{
    if (!!m_pvRecord) {
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + pulseTime.secPastEpoch, pulseTime.nsec,
            pulseSeq);
        m_pvRecord->beginGroupPut();
        m_pvRecord->timeStamp.set(time);
        m_pvRecord->proton_charge->putFrom(pulseCharge);
        m_pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
        m_pvRecord->position_index->replace(freeze(m_cache.position_index));
        m_pvRecord->sample_a1->replace(freeze(m_cache.sample_a1));
        m_pvRecord->sample_b1->replace(freeze(m_cache.sample_b1));
        m_pvRecord->pixel->replace(freeze(m_cache.pixel));
        m_pvRecord->endGroupPut();
    } else {
        LOG_WARN("No PV record, skipping posting update");
    }

    m_cache.time_of_flight.clear();
    m_cache.position_index.clear();
    m_cache.sample_a1.clear();
    m_cache.sample_b1.clear();
    m_cache.pixel.clear();
}
