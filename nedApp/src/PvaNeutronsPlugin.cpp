/* PvaNeutronsPlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Event.h"
#include "Log.h"
#include "PvaNeutronsPlugin.h"

#include <alarm.h>
#include <pv/sharedVector.h>

#include <climits>

EPICS_REGISTER_PLUGIN(PvaNeutronsPlugin, 3, "Port name", string, "Parent plugins", string, "PVA record name", string);

PvaNeutronsPlugin::PvaNeutronsPlugin(const char *portName, const char *parentPlugins, const char *pvName)
    : BasePlugin(portName, std::string(parentPlugins).find(',')!=std::string::npos, asynOctetMask, asynOctetMask)
{
    int status = STATUS_READY;

    if (pvName == 0 || strlen(pvName) == 0) {
        LOG_ERROR("Missing PVA record name");
        status = STATUS_BAD_CONFIG;
        pvName = "";
    } else {
        m_record = PvaRecord::create(pvName);
        if (!m_record) {
            LOG_ERROR("Failed to create PVA record '%s'", pvName);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_record) == false) {
            LOG_ERROR("Failed to register PVA record '%s'", pvName);
            status = STATUS_INIT_ERROR;
        }
    }

    createParam("PvaName",      asynParamOctet, &PvaName, pvName);
    createParam("Status",       asynParamInt32, &Status, status); // READ - Plugin status
    if (status != STATUS_READY) {
        setParamAlarmStatus(PvaName, epicsAlarmUDF);
        setParamAlarmSeverity(PvaName, epicsSevMajor);
    }
    callParamCallbacks();

    BasePlugin::connect(parentPlugins, MsgDasData);
}

void PvaNeutronsPlugin::recvDownstream(DasDataPacketList *packets)
{
    int status = epicsAlarmNone;
    int severity = epicsSevNone;
    if (m_record) {
        for (auto it = packets->cbegin(); it != packets->cend(); it++) {
            if ((*it)->format == DasDataPacket::DATA_FMT_PIXEL) {
                if (m_record->update(*it) == false) {
                    LOG_ERROR("Failed to send PVA update");
                    status = epicsAlarmComm;
                    severity = epicsSevMinor;
                    break;
                }
            }
        }
    }
    setIntegerParam(Status, status == epicsAlarmNone ? 0 : 3);
    setParamAlarmStatus(PvaName, status);
    setParamAlarmSeverity(PvaName, severity);
    callParamCallbacks();
}

/* *** PvaRecord implementation follows *** */

PvaNeutronsPlugin::PvaRecord::PvaRecord(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
    , m_sequence(0)
{}

PvaNeutronsPlugin::PvaRecord::shared_pointer PvaNeutronsPlugin::PvaRecord::create(const std::string &recordName)
{
    using namespace epics::pvData;

    StandardFieldPtr standardField = getStandardField();
    FieldCreatePtr fieldCreate     = getFieldCreate();
    PVDataCreatePtr pvDataCreate   = getPVDataCreate();

    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()->
            add("timeStamp",        standardField->timeStamp())->
            add("logical",          standardField->scalar(pvBoolean, ""))->
            add("num_events",       standardField->scalar(pvUInt, ""))->
            add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
            add("pixel",            standardField->scalarArray(epics::pvData::pvUInt, ""))->
            createStructure()
    );

    PvaRecord::shared_pointer pvRecord(new PvaRecord(recordName, pvStructure));
    if (pvRecord && !pvRecord->init()) {
        pvRecord.reset();
    }

    return pvRecord;
}

bool PvaNeutronsPlugin::PvaRecord::init()
{
    initPVRecord();

    if (!pvTimeStamp.attach(getPVStructure()->getSubField("timeStamp")))
        return false;

    pvLogical = getPVStructure()->getSubField<epics::pvData::PVBoolean>("logical.value");
    if (pvLogical.get() == NULL)
        return false;

    pvNumEvents = getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
    if (pvNumEvents.get() == NULL)
        return false;

    pvTimeOfFlight = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
    if (pvTimeOfFlight.get() == NULL)
        return false;

    pvPixel = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
    if (pvPixel.get() == NULL)
        return false;

    return true;
}

bool PvaNeutronsPlugin::PvaRecord::update(DasDataPacket *packet)
{
    bool posted = true;

    assert(packet->format == DasDataPacket::DATA_FMT_PIXEL);

    // 31 bit sequence number is good for around 9 months.
    // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
    // In worst case client will skip one packet on rollover and then recover
    // the sequence.
    epics::pvData::TimeStamp timestamp(
        epics::pvData::posixEpochAtEpicsEpoch + packet->timestamp_sec,
        packet->timestamp_nsec,
        m_sequence++ % 0x7FFFFFFF
    );

    uint32_t nEvents = 0;
    Event::Pixel *events = packet->getEvents<Event::Pixel>(nEvents);

    // Pre-allocate svector to minimize gradual memory allocations
    epics::pvData::PVUIntArray::svector tofs(nEvents);
    epics::pvData::PVUIntArray::svector pixels(nEvents);
    for (uint32_t i = 0; i < nEvents; i++) {
        tofs[i]   = events[i].tof;
        pixels[i] = events[i].pixelid;
        events++;
    }

    lock();
    try {
        beginGroupPut();

        pvTimeStamp.set(timestamp);
        pvLogical->put(packet->mapped);
        pvNumEvents->put(nEvents);
        pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
        pvPixel->replace(epics::pvData::freeze(pixels));

        endGroupPut();
    } catch (...) {
        posted = false;
    }
    unlock();

    return posted;
}
