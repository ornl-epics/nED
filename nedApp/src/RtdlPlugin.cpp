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
    createParam("BadPulse",         asynParamInt32, &BadPulse);     // READ - Bad pulse indicator (0=bad pulse, 1=good pulse)
    createParam("PulseFlavor",      asynParamInt32, &PulseFlavor);  // READ - Pulse flavor
    createParam("PulseCharge",      asynParamInt32, &PulseCharge);  // READ - Pulse charge
    createParam("BadVetoFrame",     asynParamInt32, &BadVetoFrame); // READ - Bad veto frame
    createParam("BadCycleFrame",    asynParamInt32, &BadCycleFrame);// READ - Bad cycle frame
    createParam("Tstat",            asynParamInt32, &Tstat);        // READ - TSTAT
    createParam("PrevCycleVeto",    asynParamInt32, &PrevCycleVeto);// READ - Previous pulse veto
    createParam("Cycle",            asynParamInt32, &Cycle);        // READ - Cycle frame
    createParam("TsyncPeriod",      asynParamInt32, &TsyncPeriod);  // READ - Number of ns between reference pulses
    createParam("TofFullOffset",    asynParamInt32, &TofFullOffset);// READ - TOF full offset
    createParam("FrameOffset",      asynParamInt32, &FrameOffset);  // READ - Frame offset
    createParam("TofFixOffset",     asynParamInt32, &TofFixOffset); // READ - TOF fixed offset
    createParam("RingPeriod",       asynParamInt32, &RingPeriod);   // READ - Ring revolution period
    createParam("ErrorsFutureTime", asynParamInt32, &ErrorsFutureTime, -1); // READ - Number of errors when time jumps in the future
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

    BasePlugin::connect(parentPlugins, MsgDasRtdl);
}

void RtdlPlugin::recvDownstream(DasRtdlPacketList *packets)
{
    if (m_record) {
        setParamAlarmStatus(PvaName, epicsAlarmNone);
        setParamAlarmSeverity(PvaName, epicsSevNone);
    }

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        const DasRtdlPacket *packet = *it;

        epicsTimeStamp t;
        t.secPastEpoch = packet->timestamp_sec;
        t.nsec         = packet->timestamp_nsec;

        // Format time
        char rtdlTimeStr[64];
        epicsTimeToStrftime(rtdlTimeStr, sizeof(rtdlTimeStr), TIMESTAMP_FORMAT, &t);

        // Account for any errors
        epicsTime rtdlTime = t;
        if (rtdlTime < m_lastRtdlTime) {
            // RTDL time going backwards?
            // Occasionally a glitch has been observed where the time jumps
            // way in the future, but then no other packets would get processed.
            // So let's pretend the future one was bogus and this is a good one.
            addIntegerParam(ErrorsPastTime, 1);
        } else if (rtdlTime == m_lastRtdlTime) {
            // Silently drop duplicate RTDLs - hopefully the packet content is
            // the same as previous one with same timestamp.
            continue;
        } else {
            // RTDL packets are guaranteed 60Hz => when time is more than 16.6ms
            // in the future, we may have skipped one or more packets.
            if ((rtdlTime - 0.017) > m_lastRtdlTime) {
                addIntegerParam(ErrorsFutureTime, 1);
            }
        }
        m_lastRtdlTime = rtdlTime;

        // Usually frame 4 follows RTLD header, but let's make it generic
        uint32_t ringPeriod = 0;
        for (uint32_t i=0; i<packet->num_frames; i++) {
             if ((packet->frames[i]>>24) == 4) {
                 ringPeriod = packet->frames[i] & 0xFFFFFF;
                 break;
             }
         }

        setStringParam(Timestamp,           rtdlTimeStr);
        setIntegerParam(BadPulse,           packet->pulse.bad);
        setIntegerParam(PulseFlavor,        packet->pulse.flavor);
        setIntegerParam(PulseCharge,        packet->pulse.charge * 10e-12);
        setIntegerParam(BadVetoFrame,       packet->pulse.bad_veto_frame);
        setIntegerParam(BadCycleFrame,      packet->pulse.bad_cycle_frame);
        setIntegerParam(Tstat,              packet->pulse.tstat);
        setIntegerParam(PrevCycleVeto,      packet->pulse.last_cycle_veto);
        setIntegerParam(Cycle,              packet->pulse.cycle);
        setIntegerParam(TsyncPeriod,        packet->correction.tsync_period * 100);
        setIntegerParam(TofFullOffset,      packet->correction.tof_full_offset);
        setIntegerParam(FrameOffset,        packet->correction.frame_offset);
        setIntegerParam(TofFixOffset,       packet->correction.tof_fixed_offset * 100);
        setIntegerParam(RingPeriod,         ringPeriod);

        if (m_record && m_record->update(*it) == false) {
            LOG_ERROR("Failed to send PVA update");
            setParamAlarmStatus(PvaName, epicsAlarmComm);
            setParamAlarmSeverity(PvaName, epicsSevMinor);
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
            add("bad_pulse",      standardField->scalar(pvBoolean, ""))->
            add("pulse_flavor",   standardField->enumerated(""))->
            add("bad_veto_frame", standardField->scalar(pvBoolean, ""))->
            add("bad_cycle_frame",standardField->scalar(pvBoolean, ""))->
            add("tstat",          standardField->scalar(pvUByte, ""))->
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

    pvBadPulse = getPVStructure()->getSubField<epics::pvData::PVBoolean>("bad_pulse.value");
    if (pvBadPulse.get() == NULL)
        return false;

    if (!pvPulseFlavor.attach(getPVStructure()->getSubField("pulse_flavor.value")))
        return false;
    std::vector<std::string> flavors = {
        "no beam", "target 1", "target 2", "diag 10us", "diag 50us",
        "diag 100us", "physics 1", "physics 2"
    };
    pvPulseFlavor.setChoices(flavors);

    pvBadVetoFrame = getPVStructure()->getSubField<epics::pvData::PVBoolean>("bad_veto_frame.value");
    if (pvBadVetoFrame.get() == NULL)
        return false;

    pvBadCycleFrame = getPVStructure()->getSubField<epics::pvData::PVBoolean>("bad_cycle_frame.value");
    if (pvBadCycleFrame.get() == NULL)
        return false;

    pvTstat = getPVStructure()->getSubField<epics::pvData::PVUByte>("tstat.value");
    if (pvTstat.get() == NULL)
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

bool RtdlPlugin::PvaRecord::update(DasRtdlPacket *packet)
{
    bool posted = true;

    // 31 bit sequence number is good for around 9 months.
    // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
    // In worst case client will skip one packet on rollover and then recover
    // the sequence.
    epics::pvData::TimeStamp timestamp(
        epics::pvData::posixEpochAtEpicsEpoch + packet->timestamp_sec,
        packet->timestamp_nsec,
        m_sequence++ % 0x7FFFFFFF
    );

    epics::pvData::PVUIntArray::svector frames(packet->num_frames);
    for (uint32_t i = 0; i < packet->num_frames; i++) {
        frames[i] = packet->frames[i];
    }

    epics::pvData::Display display;

    lock();
    try {
        beginGroupPut();

        pvTimeStamp.set(timestamp);
        pvProtonCharge->put(packet->pulse.charge * 10e-12);
        pvBadPulse->put(packet->pulse.bad);
        pvPulseFlavor.setIndex(packet->pulse.flavor);
        pvBadVetoFrame->put(packet->pulse.bad_veto_frame);
        pvBadCycleFrame->put(packet->pulse.bad_cycle_frame);
        pvTstat->put(packet->pulse.tstat);
        pvLastCycleVeto->put(packet->pulse.last_cycle_veto);
        pvCycle->put(packet->pulse.cycle);
        pvTsyncPeriod->put(packet->correction.tsync_period * 100);
        pvTofOffset->put(packet->correction.tof_fixed_offset * 100);
        pvFrameOffset->put(packet->correction.frame_offset * 100);
        pvOffsetEnabled->put(packet->correction.tof_full_offset);
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
