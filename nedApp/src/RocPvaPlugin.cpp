#include "RocPvaPlugin.h"

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

RocPvaPlugin::RocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix)
    , m_nTransmitted(0)
    , m_nProcessed(0)
    , m_nReceived(0)
{
    uint32_t maxNormalEventsPerPacket = (DasPacket::MaxLength/4) / 6;

    if (!!m_pvRecord) {
        // Guestimate container size and force memory pre-allocation, will automatically extend if needed
        m_pvRecord->cache.time_of_flight.reserve(maxNormalEventsPerPacket); // normal mode is the shortest => max counts in packet
	m_pvRecord->cache.pixel.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.position_index.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.sample_a1.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.sample_b1.reserve(maxNormalEventsPerPacket);
    }
}

void RocPvaPlugin::processDataNormal(const DasPacketList * const packetList)
{
    static int userTagCounter=0;

    // Sanity check, BasePvaPlugin should prevent it anyway
    if (!m_pvRecord)
        return;

    int goodPacketCount = 0;

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;
        m_nReceived++;

	const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();

        // TODO: what about metadata? In tof,pixel format?
        if (packet->isNeutronData() == false || rtdl == 0)
            continue;

        if (packet->isRtdl() && packet->isData())
            continue;

	m_nProcessed++;

	// Extract timestamp and proton charge from RTDL
        // RTDL should always be present when working with DSP-T
        epics::pvData::TimeStamp time(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        if (time != m_pvRecord->cache.timeStamp) {
            // new pulse detected, post any outstanding packets, go to next
            if (goodPacketCount > 0) {
             	m_pvRecord->cache.timeStamp.setUserTag(userTagCounter++);
                m_pvRecord->postCached();
                goodPacketCount = 0;
            }
            m_pvRecord->cache.timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
            m_pvRecord->cache.proton_charge = rtdl->charge;
        }

	// Data itself does not contain format information. Simple length
        // check is the best we can do.
        uint32_t dataLength;
        const struct NormalEvent *data = 
            reinterpret_cast<const NormalEvent *>(packet->getEventData(&dataLength));
        if (data == 0 || dataLength == 0)
            continue;
        dataLength *= sizeof(DasPacket::Event);
        if (dataLength % sizeof(NormalEvent) != 0)
            continue;
        dataLength /= sizeof(NormalEvent);

        // Go through events and append to cache
        while (dataLength-- > 0) {
            m_pvRecord->cache.time_of_flight.push_back(data->timeStamp);
            m_pvRecord->cache.pixel.push_back(data->pixelID);
            data++;
        }

        m_nTransmitted++;
        goodPacketCount++;
    }

    if (goodPacketCount > 0) {
	m_pvRecord->cache.timeStamp.setUserTag(userTagCounter++);
        m_pvRecord->postCached();
    }

    // Update parameters
    setIntegerParam(TxCount,    m_nTransmitted);
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

void RocPvaPlugin::processDataRaw(const DasPacketList * const packetList)
{
    static int userTagCounter=0;
   
    // Sanity check, BasePvaPlugin should prevent it anyway
    if (!m_pvRecord)
        return;

    int goodPacketCount = 0;

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;
        m_nReceived++;

	const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();

        // TODO: what about metadata? In tof,pixel format?
        if (packet->isNeutronData() == false || rtdl == 0)
            continue;

        if (packet->isRtdl() && packet->isData())
            continue;

	m_nProcessed++;

	// Extract timestamp and proton charge from RTDL
        // RTDL should always be present when working with DSP-T
        epics::pvData::TimeStamp time(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        if (time != m_pvRecord->cache.timeStamp) {
            // new pulse detected, post any outstanding packets, go to next
            if (goodPacketCount > 0) {
             	m_pvRecord->cache.timeStamp.setUserTag(userTagCounter++);
                m_pvRecord->postCached();
                goodPacketCount = 0;
            }
            m_pvRecord->cache.timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
            m_pvRecord->cache.proton_charge = rtdl->charge;
        }

	// Data itself does not contain format information. Simple length
        // check is the best we can do.
        uint32_t dataLength;
        const struct RawEvent *data = 
            reinterpret_cast<const RawEvent *>(packet->getEventData(&dataLength));
        if (data == 0 || dataLength == 0)
            continue;
        dataLength *= sizeof(DasPacket::Event);
        if (dataLength % sizeof(RawEvent) != 0)
            continue;
        dataLength /= sizeof(RawEvent);

	/* Pull the least significant 16bits from sample1 and sample2 and 
         * package them together as sample_a1; this combines the 1-A and 2-A
         * samples.  Repeat for the B samples.  Append each event to cache.
         */
        while (dataLength-- > 0) {
            m_pvRecord->cache.time_of_flight.push_back(data->time_of_flight);
            m_pvRecord->cache.position_index.push_back(data->position_index);
            m_pvRecord->cache.sample_a1.push_back(
                (data->sample1&0xffff) | ((data->sample2&0xffff)<<16) );
            m_pvRecord->cache.sample_b1.push_back(
                ((data->sample1&0xffff0000) >> 16) | (data->sample2&0xffff0000));
            data++;
        }

        m_nTransmitted++;
        goodPacketCount++;
    }

    if (goodPacketCount > 0) {
	m_pvRecord->cache.timeStamp.setUserTag(userTagCounter++);
        m_pvRecord->postCached();
    }

    // Update parameters
    setIntegerParam(TxCount,    m_nTransmitted);
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

