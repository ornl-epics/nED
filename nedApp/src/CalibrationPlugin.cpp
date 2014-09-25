#include "CalibrationPlugin.h"

EPICS_REGISTER_PLUGIN(CalibrationPlugin, 3, "port name", string, "dispatcher port", string, "blocking callbacks", int);

#define NUM_CALIBRATIONPLUGIN_PARAMS      0

RawCalDataPVRecord::shared_pointer RawCalDataPVRecord::create(std::string const & recordName)
{
	epics::pvData::FieldCreatePtr fieldCreate = epics::pvData::getFieldCreate();
	epics::pvData::StandardFieldPtr standardField = epics::pvData::getStandardField();
	epics::pvData::PVDataCreatePtr pvDataCreate = epics::pvData::getPVDataCreate();

	epics::pvData::PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
		fieldCreate->createFieldBuilder()
		->add("timeStamp", standardField->timeStamp())
		->add("posIndex", standardField->scalar(epics::pvData::pvUInt, ""))
		->add("sample1A", standardField->scalar(epics::pvData::pvUShort, ""))
		->add("sample1B", standardField->scalar(epics::pvData::pvUShort, ""))
		->add("sample2A", standardField->scalar(epics::pvData::pvUShort, ""))
		->add("sample2B", standardField->scalar(epics::pvData::pvUShort, ""))
		->createStructure()
	);

	RawCalDataPVRecord *t = new RawCalDataPVRecord(recordName, pvStructure);
	RawCalDataPVRecord::shared_pointer pvRecord(t);
        //new RawCalDataPVRecord(recordName, pvStructure));

	if (!pvRecord->init())
		pvRecord.reset();
	return pvRecord;
}

bool RawCalDataPVRecord::init()
{
	initPVRecord();

	if (!pvTimeStamp.attach(getPVStructure()->getSubField("timeStamp")))
		return false;

	pvPosIndex = getPVStructure()->getUIntField("posIndex.value");
	if (pvPosIndex.get() == NULL)
		return false;

	pvSample1A = getPVStructure()->getUShortField("sample1A.value");
	if (pvSample1A.get() == NULL)
		return false;

	pvSample1B = getPVStructure()->getUShortField("sample1B.value");
	if (pvSample1B.get() == NULL)
		return false;

	pvSample2A = getPVStructure()->getUShortField("sample2A.value");
	if (pvSample2A.get() == NULL)
		return false;

	pvSample2B = getPVStructure()->getUShortField("sample2B.value");
	if (pvSample2B.get() == NULL)
		return false;

	return true;
}

RawCalDataPVRecord::RawCalDataPVRecord(std::string const & recordName,
	epics::pvData::PVStructurePtr const & pvStructure) : PVRecord(recordName, pvStructure)
{

}

void RawCalDataPVRecord::process()
{
	// update time stamp
	timeStamp.getCurrent();
	pvTimeStamp.set(timeStamp);
}

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


CalibrationPlugin::CalibrationPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_CALIBRATIONPLUGIN_PARAMS)
    , m_nTransmitted(0)
    , m_nProcessed(0)
    , m_nReceived(0)
{
	// call factory method to create a PV Record
	std::string name("raw_cal_data");
	std::cout << "Creating V4 '" << name << "' record" << std::endl;
	raw_cal_record = RawCalDataPVRecord::create(name);

	// Add record to PV database
	if (!epics::pvDatabase::PVDatabase::getMaster()->addRecord(raw_cal_record))
		std::cout << "Cannot create raw cal data record '" <<
			raw_cal_record->getRecordName() << "'" << std::endl;
}

CalibrationPlugin::~CalibrationPlugin()
{
}

void CalibrationPlugin::processData(const DasPacketList * const packetList)
{
    // Do we need to connect the client? There's no extra thread that would wait
    // for client, instead we rely on the incoming data rate to trigger this function
    // quite often.
   /* bool clientConnected = connectClient();*/

    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        m_nReceived++;

        // Don't even bother with packet inspection if there's noone interested
       /* if (!clientConnected)
            continue;
*/
	if (packet->isRtdl()) {
	    continue;
	}

	if (packet->isData()) {
	    uint32_t eventsCount;
	    const DasPacket::RawEvent *events =
		reinterpret_cast<const DasPacket::RawEvent *>
			(packet->getEventData(&eventsCount));

	    raw_cal_record->update(events->posindex, events->sample1A,
		events->sample1B, events->sample2A, events->sample2B);

            m_nTransmitted++;
	}

        m_nProcessed++;
    }

    // Update parameters
    setIntegerParam(TxCount,    m_nTransmitted);
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

