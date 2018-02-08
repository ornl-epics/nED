/* RtdlPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Log.h"
#include "RtdlPlugin.h"

#include <alarm.h>
#include <pv/sharedVector.h>

#include <climits>

EPICS_REGISTER_PLUGIN(RtdlPlugin, 3, "Port name", string, "Parent plugins", string, "PVA RTDL record name", string);

#define TIMESTAMP_FORMAT        "%Y/%m/%d %T.%09f"

RtdlPlugin::RtdlPlugin(const char *portName, const char *parentPlugins, const char *pvName)
    : BasePlugin(portName, std::string(parentPlugins).find(',')!=std::string::npos, asynOctetMask, asynOctetMask)
{
    createParam("Timestamp",        asynParamOctet, &Timestamp);    // READ - Timestamp string of last RTDL
    createParam("PulseFlavor",      asynParamInt32, &PulseFlavor);  // READ - Pulse flavor
    createParam("PulseCharge",      asynParamInt32, &PulseCharge);  // READ - Pulse charge
    createParam("TimingStatus",     asynParamInt32, &TimingStatus); // READ - Timing status
    createParam("LastCycleVeto",    asynParamInt32, &LastCycleVeto);// READ - Last pulse veto
    createParam("Cycle",            asynParamInt32, &Cycle);        // READ - Cycle frame
    createParam("TsyncPeriod",      asynParamInt32, &TsyncPeriod);  // READ - Number of ns between reference pulses
    createParam("TofFullOffset",    asynParamInt32, &TofFullOffset);// READ - TOF full offset
    createParam("FrameOffset",      asynParamInt32, &FrameOffset);  // READ - Frame offset
    createParam("TofFixOffset",     asynParamInt32, &TofFixOffset); // READ - TOF fixed offset
    createParam("RingPeriod",       asynParamInt32, &RingPeriod);   // READ - Ring revolution period
    createParam("ErrorsFutureTime", asynParamInt32, &ErrorsFutureTime, 0);  // READ - Number of errors when time jumps in the future
    createParam("ErrorsPastTime",   asynParamInt32, &ErrorsPastTime, 0);    // READ - Number of errors when time jumps in the past
    createParam("PvaName",          asynParamOctet, &PvaName, pvName);

    if (pvName && strlen(pvName) > 0) {
        m_record = PvaRecord::create(pvName);
        if (!m_record) {
            LOG_ERROR("Failed to create PVA record '%s'", pvName);
            setParamAlarmStatus(PvaName, epicsAlarmUDF);
            setParamAlarmSeverity(PvaName, epicsSevMinor);
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_record) == false) {
            LOG_ERROR("Failed to register PVA record '%s'", pvName);
            setParamAlarmStatus(PvaName, epicsAlarmUDF);
            setParamAlarmSeverity(PvaName, epicsSevMinor);
            m_record.reset();
        }
    }
    callParamCallbacks();

    BasePlugin::connect(parentPlugins, { MsgDasRtdl, MsgOldDas });
}

void RtdlPlugin::recvDownstream(const DasPacketList &packets)
{
    for (const DasPacket *packet: packets) {
        if (packet->isRtdl()) {
            update(packet->getTimeStamp(), *packet->getRtdlHeader(), packet->getRtdlFrames());
            m_lastOldPktTime = epicsTime::getCurrent();
            m_oldPackets = true;
        }
    }
}

void RtdlPlugin::recvDownstream(const RtdlPacketList &packets)
{
    if (m_oldPackets) {
        if ((epicsTime::getCurrent() - m_lastOldPktTime) < 1.0) {
            // Old style RTDL packets are coming in, they provide more
            // information in terms of acquisition frame, so let's use
            // those instead.
            // But for up to 1 second at startup we could do both.
            return;
        }
        m_oldPackets = false;
    }
    for (const RtdlPacket *packet: packets) {
        update(packet->getTimeStamp(), packet->getRtdlHeader(), packet->getRtdlFrames());
    }
}

void RtdlPlugin::update(const epicsTimeStamp &timestamp, const RtdlHeader &rtdl, const std::vector<RtdlPacket::RtdlFrame> &frames)
{
    // Now check in cache of already processed timestams
    epicsTime rtdlTime = timestamp;
    for (auto it = m_timesCache.begin(); it != m_timesCache.end(); it++) {
        if (*it == rtdlTime) {
            // Skip update for RTDL that was already posted
            return;
        }
    }

    // Do some time verification - allow .5 second offset from local clock - just a sanity check, nothing will break because of it
    epicsTime now{ epicsTime::getCurrent() };
    const double threshold = 0.5;
    double diff = rtdlTime - now;
    if (diff > threshold) {
        addIntegerParam(ErrorsFutureTime, 1);
    } else if (diff < (-1 * threshold)) {
        addIntegerParam(ErrorsPastTime, 1);
    }

    m_timesCache.push_front(rtdlTime);

    // Keep the cache sane, at 60Hz nominal update rate 10 samples should be plenty
    // New entries are pushed to the front, so remove entries from the end to
    // make space for new ones.
    while (m_timesCache.size() >= 10) {
        m_timesCache.pop_back();
    }

    // Ring period not part of RtdlHeader, must decode from RTDL frame 4
    uint32_t ringPeriod = 0;
    for (const auto& frame: frames) {
        if (frame.id == 4) {
            ringPeriod = frame.data;
            break;
        }
    }

    // Format time
    char rtdlTimeStr[64];
    epicsTimeToStrftime(rtdlTimeStr, sizeof(rtdlTimeStr), TIMESTAMP_FORMAT, &timestamp);

    setStringParam(Timestamp,           rtdlTimeStr);
    setIntegerParam(PulseFlavor,        rtdl.pulse.flavor);
    setIntegerParam(PulseCharge,        rtdl.pulse.charge * 10e-12);
    setIntegerParam(TimingStatus,       rtdl.timing_status);
    setIntegerParam(LastCycleVeto,      rtdl.last_cycle_veto);
    setIntegerParam(Cycle,              rtdl.cycle);
    setIntegerParam(TsyncPeriod,        rtdl.tsync_period * 100);
    setIntegerParam(TofFullOffset,      rtdl.tof_full_offset);
    setIntegerParam(FrameOffset,        rtdl.frame_offset);
    setIntegerParam(TofFixOffset,       rtdl.tof_fixed_offset * 100);
    setIntegerParam(RingPeriod,         ringPeriod);
    
    if (m_record) {
        if (m_record->update(timestamp, rtdl, frames) == false) {
            LOG_ERROR("Failed to send PVA update");
            setParamAlarmStatus(PvaName, epicsAlarmComm);
            setParamAlarmSeverity(PvaName, epicsSevMinor);
        } else {
            setParamAlarmStatus(PvaName, epicsAlarmNone);
            setParamAlarmSeverity(PvaName, epicsSevNone);
        }
    }

    callParamCallbacks();
}

/* *** PvaRecord implementation follows *** */

RtdlPlugin::PvaRecord::PvaRecord(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
    , m_sequence(0)
{}

RtdlPlugin::PvaRecord::shared_pointer RtdlPlugin::PvaRecord::create(const std::string &recordName)
{
    using namespace epics::pvData;

    StandardFieldPtr standardField = getStandardField();
    FieldCreatePtr fieldCreate     = getFieldCreate();
    PVDataCreatePtr pvDataCreate   = getPVDataCreate();

    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()->
            add("timeStamp",      standardField->timeStamp())->
            add("proton_charge",  standardField->scalar(pvDouble, "display"))->
            add("pulse_flavor",   standardField->enumerated(""))->
            add("timing_status",  standardField->scalar(pvUByte, ""))->
            add("last_cycle_veto",standardField->scalar(pvUShort, ""))->
            add("cycle",          standardField->scalar(pvUShort, ""))->
            add("tsync_period",   standardField->scalar(pvUInt, "display"))->
            add("tof_offset",     standardField->scalar(pvUInt, "display"))->
            add("frame_offset",   standardField->scalar(pvUInt, ""))->
            add("offset_enabled", standardField->scalar(pvBoolean, ""))->
            add("frames",         standardField->scalarArray(pvUInt, ""))->
            createStructure()
    );

    PvaRecord::shared_pointer pvRecord(new PvaRecord(recordName, pvStructure));
    if (pvRecord && !pvRecord->init()) {
        pvRecord.reset();
    }

    return pvRecord;
}

bool RtdlPlugin::PvaRecord::init()
{
    initPVRecord();

    if (!pvTimeStamp.attach(getPVStructure()->getSubField("timeStamp")))
        return false;

    pvProtonCharge = getPVStructure()->getSubField<epics::pvData::PVDouble>("proton_charge.value");
    if (pvProtonCharge.get() == NULL)
        return false;

    if (!pvProtonChargeDisplay.attach(getPVStructure()->getSubField("proton_charge.display")))
        return false;

    if (!pvPulseFlavor.attach(getPVStructure()->getSubField("pulse_flavor.value")))
        return false;
    std::vector<std::string> flavors = {
        "no beam", "target 1", "target 2", "diag 10us", "diag 50us",
        "diag 100us", "physics 1", "physics 2"
    };
    pvPulseFlavor.setChoices(flavors);

    pvTimingStatus = getPVStructure()->getSubField<epics::pvData::PVUByte>("timing_status.value");
    if (pvTimingStatus.get() == NULL)
        return false;

    pvLastCycleVeto = getPVStructure()->getSubField<epics::pvData::PVUShort>("last_cycle_veto.value");
    if (pvLastCycleVeto.get() == NULL)
        return false;

    pvCycle = getPVStructure()->getSubField<epics::pvData::PVUShort>("cycle.value");
    if (pvCycle.get() == NULL)
        return false;

    pvTsyncPeriod = getPVStructure()->getSubField<epics::pvData::PVUInt>("tsync_period.value");
    if (pvTsyncPeriod.get() == NULL)
        return false;

    if (!pvTsyncPeriodDisplay.attach(getPVStructure()->getSubField("tsync_period.display")))
        return false;

    pvTofOffset = getPVStructure()->getSubField<epics::pvData::PVUInt>("tof_offset.value");
    if (pvTofOffset.get() == NULL)
        return false;

    if (!pvTofOffsetDisplay.attach(getPVStructure()->getSubField("tof_offset.display")))
        return false;

    pvFrameOffset = getPVStructure()->getSubField<epics::pvData::PVUInt>("frame_offset.value");
    if (pvFrameOffset.get() == NULL)
        return false;

    pvOffsetEnabled = getPVStructure()->getSubField<epics::pvData::PVBoolean>("offset_enabled.value");
    if (pvOffsetEnabled.get() == NULL)
        return false;

    pvFrames = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("frames.value");
    if (pvFrames.get() == NULL)
        return false;

    return true;
}

bool RtdlPlugin::PvaRecord::update(const epicsTimeStamp &timestamp_, const RtdlHeader &rtdl, const std::vector<RtdlPacket::RtdlFrame> &frames_)
{
    bool posted = true;

    // 31 bit sequence number is good for around 9 months.
    // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
    // In worst case client will skip one packet on rollover and then recover
    // the sequence.
    epics::pvData::TimeStamp timestamp(
        epics::pvData::posixEpochAtEpicsEpoch + timestamp_.secPastEpoch,
        timestamp_.nsec,
        m_sequence++ % 0x7FFFFFFF
    );

    epics::pvData::PVUIntArray::svector frames(frames_.size());
    for (size_t i = 0; i < frames_.size(); i++) {
        frames[i] = frames_[i].raw;
    }

    epics::pvData::Display display;

    lock();
    try {
        beginGroupPut();

        pvTimeStamp.set(timestamp);
        pvProtonCharge->put(rtdl.pulse.charge * 10e-12);
        pvPulseFlavor.setIndex(rtdl.pulse.flavor);
        pvTimingStatus->put(rtdl.timing_status);
        pvLastCycleVeto->put(rtdl.last_cycle_veto);
        pvCycle->put(rtdl.cycle);
        pvTsyncPeriod->put(rtdl.tsync_period * 100);
        pvTofOffset->put(rtdl.tof_fixed_offset * 100);
        pvFrameOffset->put(rtdl.frame_offset * 100);
        pvOffsetEnabled->put(rtdl.tof_full_offset);
        pvFrames->replace(epics::pvData::freeze(frames));

        display.setUnits("C");
        pvProtonChargeDisplay.set(display);

        display.setUnits("ns");
        pvTsyncPeriodDisplay.set(display);
        pvTofOffsetDisplay.set(display);

        endGroupPut();
    } catch (...) {
        posted = false;
    }
    unlock();

    return posted;
}
