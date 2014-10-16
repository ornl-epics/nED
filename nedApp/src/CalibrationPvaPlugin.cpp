#include "CalibrationPvaPlugin.h"

EPICS_REGISTER_PLUGIN(CalibrationPvaPlugin, 3, "port name", string, "dispatcher port", string, "raw_cal_data", string);

/*#define NUM_CALIBRATIONPLUGIN_PARAMS      0*/


/*
void RawCalDataPVRecord::update(uint posIndex, ushort sample1A,
	ushort sample1B, ushort sample2A, ushort sample2B)
{
	lock();
	try
	{
		beginGroupPut();

		// perform updates
		pvPosIndex->put(posIndex);
		pvSample1A->put(sample1A);
		pvSample1B->put(sample1B);
		pvSample2A->put(sample2A);
		pvSample2B->put(sample2B);

		endGroupPut();
	}
	catch(...)
	{
		unlock();
		throw;
	}
	unlock();
}
*/

CalibrationPvaPlugin::CalibrationPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix)
    , m_nTransmitted(0)
    , m_nProcessed(0)
    , m_nReceived(0)
{
    uint32_t maxNormalEventsPerPacket = (DasPacket::MaxLength/4) / 6;

    if (!!m_pvRecord) {
        // Guestimate container size and force memory pre-allocation, will automatically extend if needed
        m_pvRecord->cache.time_of_flight.reserve(maxNormalEventsPerPacket); // normal mode is the shortest => max counts in packet
        m_pvRecord->cache.position_index.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.sample_a1.reserve(maxNormalEventsPerPacket);
        m_pvRecord->cache.sample_b1.reserve(maxNormalEventsPerPacket);
    }
}

void CalibrationPvaPlugin::processDataRaw(const DasPacketList * const packetList)
{
    /** 
     * Structure representing RAW mode data packet
     */
    struct RawEvent {
        uint32_t time_of_flight;
	uint32_t position_index;
        uint32_t sample1;
        uint32_t sample2;
    };

    // Sanity check, BasePvaPlugin should prevent it anyway
    if (!m_pvRecord)
        return;

    for (const DasPacket *packet = packetList->first(); packet != 0; 
            packet = packetList->next(packet)) {
        m_nReceived++;

	const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();

        // TODO: what about metadata? In tof,pixel format?
        if (packet->isNeutronData() == false || rtdl == 0)
            continue;

	m_nProcessed++;

	// Extract timestamp and proton charge from RTDL
        // RTDL should always be present when working with DSP-T
        epics::pvData::TimeStamp time(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        if (time != m_pvRecord->cache.timeStamp) {
            // Different pulse detected, post what we have and start over
            m_pvRecord->postCached();
        }
        m_pvRecord->cache.timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        m_pvRecord->cache.proton_charge = rtdl->charge;
/*
	uint32_t eventsCount;
	const RawEvent *events = reinterpret_cast<const RawEvent *>
            (packet->getEventData(&eventsCount));

	m_pvRecord->update(events->posindex, events->sample1A,
		events->sample1B, events->sample2A, events->sample2B);
*/

	// Data itself does not contain format information. Simple length
        // check is the best we can do.
        uint32_t dataLength;
        const struct RawEvent *data = reinterpret_cast<const RawEvent *>(packet->getData(&dataLength));
        if (data == 0 || dataLength == 0)
            continue;
        dataLength *= sizeof(uint32_t);
        if (dataLength % sizeof(RawEvent) != 0)
            continue;
        dataLength /= sizeof(RawEvent);


	// GSG: I need to pull the least significant 16bits from sample1 and sample2 and
	// package them together as sample_a1; this combines the 1-A and 2-A samples.
 	// Do same thing with the B samples.

        // Go through events and append to cache
        while (dataLength-- > 0) {
            m_pvRecord->cache.time_of_flight.push_back(data->time_of_flight);
            m_pvRecord->cache.position_index.push_back(data->position_index);
            m_pvRecord->cache.sample_a1.push_back( 
                (data->sample1&0xffff) | ((data->sample2&0xffff)<<16) );
            m_pvRecord->cache.sample_b1.push_back( 
                ((data->sample1&0xffff0000) >> 16) | (data->sample2&0xffff0000));
            data++;
        }
        m_pvRecord->postCached();

        m_nTransmitted++;
    }

    // Update parameters
    setIntegerParam(TxCount,    m_nTransmitted);
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

