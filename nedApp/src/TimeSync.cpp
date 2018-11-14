/* TimeSync.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

// Local includes
#include "BaseModulePlugin.h"
#include "TimeSync.h"

// EPICS includes
#include <alarm.h>

// System includes
#include <algorithm>
#include <cmath>

class SyncRequestPacket : public DasCmdPacket {
    public:
        static SyncRequestPacket *init(uint8_t *data, size_t size, uint32_t moduleId)
        {
            auto pkt = DasCmdPacket::init(data, size, moduleId, CMD_TIME_SYNC, 0x1, false, false, 0, 12);
            return reinterpret_cast<SyncRequestPacket*>(pkt);
        }

        epicsTimeStamp timestamp;
        union __attribute__ ((__packed__)) {
            struct __attribute__ ((__packed__)) {
                uint32_t pace:29;
                bool direction:1;
                bool setpace:1;
                bool settime:1;
            };
            uint32_t ctrl;
        };
};

class SyncResponsePacket : public DasCmdPacket {
    public:
        uint32_t tof;
        epicsTimeStamp gpstime;
        epicsTimeStamp remotetime;
        union __attribute__ ((__packed__)) {
            struct __attribute__ ((__packed__)) {
                uint32_t pace:29;
                bool direction:1;
                bool setpace:1;
                bool settime:1;
            };
            uint32_t ctrl;
        };
};

TimeSync::TimeSync(BaseModulePlugin* parent)
    : m_parent(parent)
    , m_timer(false)
{
    m_parent->createParam("TS:Enable",          asynParamInt32,   &Enable,          0);
    m_parent->createParam("TS:State",           asynParamInt32,   &State,           static_cast<int>(STATE_UNSYNCED));
    m_parent->createParam("TS:SyncForce",       asynParamInt32,   &SyncForce);
    m_parent->createParam("TS:SyncInt",         asynParamFloat64, &SyncInt,         0.1);
    m_parent->createParam("TS:SyncSamples",     asynParamInt32,   &SyncSamples,     10);
    m_parent->createParam("TS:CommDlySamples",  asynParamInt32,   &CommDlySamples,  3);
    m_parent->createParam("TS:CommDly",         asynParamFloat64, &CommDly,         0.0);
    m_parent->createParam("TS:NoSyncThr",       asynParamFloat64, &NoSyncThr,       10);
    m_parent->createParam("TS:LocalTimeSec",    asynParamInt32,   &LocalTimeSec,    0);
    m_parent->createParam("TS:LocalTimeNSec",   asynParamInt32,   &LocalTimeNSec,   0);
    m_parent->createParam("TS:RemoteTimeSec",   asynParamInt32,   &RemoteTimeSec,   0);
    m_parent->createParam("TS:RemoteTimeNSec",  asynParamInt32,   &RemoteTimeNSec,  0);
    m_parent->createParam("TS:RemoteTimeOff",   asynParamFloat64, &RemoteTimeOff,   0.0);
    m_parent->createParam("TS:GpsTimeSec",      asynParamInt32,   &GpsTimeSec,      0);
    m_parent->createParam("TS:GpsTimeNSec",     asynParamInt32,   &GpsTimeNSec,     0);
    m_parent->createParam("TS:GpsTimeOff",      asynParamFloat64, &GpsTimeOff,      0.0);
    m_parent->createParam("TS:SyncRawError",    asynParamFloat64, &SyncRawError,    0.0);
    m_parent->createParam("TS:SyncAdjError",    asynParamFloat64, &SyncAdjError,    0.0);
    m_parent->createParam("TS:SyncFinError",    asynParamFloat64, &SyncFinError,    0.0);
    m_parent->createParam("TS:P",               asynParamFloat64, &P,               0.0);
    m_parent->createParam("TS:I",               asynParamFloat64, &I,               0.0);

    std::function<float(void)> periodicCb = std::bind(&TimeSync::timerCb, this);
    m_timer.schedule(periodicCb, 0.1);

    m_buffer.reset(new uint8_t[sizeof(SyncRequestPacket)]);
    assert(!!m_buffer);

    m_outPacket = SyncRequestPacket::init(m_buffer.get(), sizeof(SyncRequestPacket), 0x0);
    assert(m_outPacket != nullptr);
}

bool TimeSync::setParam(int param, int value)
{
    if (param == SyncForce) {
        m_offset = 0.0;
        m_lastError = 0.0;
        m_adjError = 0.0;
        m_reg = 0x1FFFFFFF;
        if (sendPacket(epicsTime::getCurrent() + getCommDelay())) {
            m_parent->setIntegerParam(State, static_cast<int>(STATE_ESTIMATING));
            m_records.clear();
        } else {
            // LOG_ERROR
        }
        m_parent->callParamCallbacks();
        return true;
    }
    if (param == Enable) {
        m_parent->setIntegerParam(State, static_cast<int>(value ? STATE_SYNCED : STATE_DISABLED));
        m_parent->callParamCallbacks();
        return true;
    }
    return false;
}

float TimeSync::timerCb()
{
    double interval = m_parent->getDoubleParam(SyncInt);
    m_parent->lock();

    if (m_parent->getIntegerParam(State) == static_cast<int>(STATE_DISABLED)) {
        // Override interval to prevent frequenty no-ops
        interval = 1.0;
    } else {
        // By design allow only one outstanding packet - cleanup last one if not received
        while (!m_records.empty() && m_records.back().remoteTime == epicsTimeStamp{0,0}) {
            m_records.pop_back();
            m_parent->setParamAlarmStatus(RemoteTimeSec, epicsAlarmRead);
            m_parent->setParamAlarmStatus(RemoteTimeNSec, epicsAlarmRead);
            m_parent->setParamAlarmStatus(RemoteTimeOff, epicsAlarmRead);
            m_parent->setParamAlarmStatus(GpsTimeSec, epicsAlarmRead);
            m_parent->setParamAlarmStatus(GpsTimeNSec, epicsAlarmRead);
            m_parent->setParamAlarmStatus(GpsTimeOff, epicsAlarmRead);
            m_parent->setParamAlarmSeverity(RemoteTimeSec, epicsSevInvalid);
            m_parent->setParamAlarmSeverity(RemoteTimeNSec, epicsSevInvalid);
            m_parent->setParamAlarmSeverity(RemoteTimeOff, epicsSevInvalid);
            m_parent->setParamAlarmSeverity(GpsTimeSec, epicsSevInvalid);
            m_parent->setParamAlarmSeverity(GpsTimeNSec, epicsSevInvalid);
            m_parent->setParamAlarmSeverity(GpsTimeOff, epicsSevInvalid);
        }

        // Send packet and measure communication delay
        epicsTime t1, t2;
        t1 = epicsTime::getCurrent();
        (void)sendPacket();
        t2 = epicsTime::getCurrent();
        m_parent->setDoubleParam(CommDly, (t2 - t1)*1e6);

        // Save to cache
        TimeRecord rec;
        rec.localTime = t2;
        rec.commDly = t2 - t1;
        rec.packetSeq = m_packetSeq;
        m_records.push_back(std::move(rec));
    }

    epicsTimeStamp now;
    epicsTimeGetCurrent(&now);
    m_parent->setIntegerParam(LocalTimeSec, now.secPastEpoch);
    m_parent->setIntegerParam(LocalTimeNSec, now.nsec);
    m_parent->callParamCallbacks();

    m_parent->unlock();
    return interval;
}

bool TimeSync::rspTimeSync(const DasCmdPacket* packet)
{
    if (m_parent->getIntegerParam(State) == static_cast<int>(STATE_DISABLED)) {
fprintf(stderr, "TimeSync: rsp wrong state\n");
        return true;
    }
    if (!packet->isResponse() || !packet->isAcknowledge()) {
fprintf(stderr, "TimeSync: rsp not response or ack\n");
        return true;
    }
    if (packet->getCmdId() != m_packetSeq) {
fprintf(stderr, "TimeSync: rsp wrong seq expect %u got %u\n", m_packetSeq, packet->getCmdId());
        return true;
    }
    if (packet->getCommand() != DasCmdPacket::CMD_TIME_SYNC) {
fprintf(stderr, "TimeSync: rsp not sync\n");
        return true;
    }
    if (packet->getLength() != sizeof(SyncResponsePacket)) {
fprintf(stderr, "TimeSync: rsp bad length\n");
        return true;
    }
    auto syncpkt = reinterpret_cast<const SyncResponsePacket*>(packet);
//fprintf(stderr, "TimeSync: rsp GPS=%u.%09u DSP=%u.%09u 0x%08X\n", syncpkt->gpstime.secPastEpoch, syncpkt->gpstime.nsec, syncpkt->remotetime.secPastEpoch, syncpkt->remotetime.nsec, syncpkt->ctrl);
    if (m_records.empty()) {
fprintf(stderr, "TimeSync: rsp m_records empty\n");
        return true;
    }

    // Map response packet to the sent request
    auto& rec = m_records.back();
    if (rec.packetSeq != packet->getCmdId() || rec.remoteTime != epicsTimeStamp{0, 0})
        return true;

    rec.remoteTime = syncpkt->remotetime;
    rec.gpsTime = epicsTime(syncpkt->gpstime) + syncpkt->tof/1e7;
    double offset = (rec.remoteTime - rec.localTime)*1e6;
    m_parent->setIntegerParam(RemoteTimeSec, syncpkt->remotetime.secPastEpoch);
    m_parent->setIntegerParam(RemoteTimeNSec, syncpkt->remotetime.nsec);
    m_parent->setDoubleParam(RemoteTimeOff, offset);
    m_parent->setParamAlarmStatus(RemoteTimeSec, epicsAlarmNone);
    m_parent->setParamAlarmStatus(RemoteTimeNSec, epicsAlarmNone);
    m_parent->setParamAlarmStatus(RemoteTimeOff, epicsAlarmNone);
    m_parent->setParamAlarmSeverity(RemoteTimeSec, 0);
    m_parent->setParamAlarmSeverity(RemoteTimeNSec, 0);
    m_parent->setParamAlarmSeverity(RemoteTimeOff, 0);
    m_parent->setIntegerParam(GpsTimeSec, syncpkt->gpstime.secPastEpoch);
    m_parent->setIntegerParam(GpsTimeNSec, syncpkt->gpstime.nsec);
    m_parent->setDoubleParam(GpsTimeOff, (rec.gpsTime - rec.localTime)*1e6);
    m_parent->setParamAlarmStatus(GpsTimeSec, epicsAlarmNone);
    m_parent->setParamAlarmStatus(GpsTimeNSec, epicsAlarmNone);
    m_parent->setParamAlarmStatus(GpsTimeOff, epicsAlarmNone);
    m_parent->setParamAlarmSeverity(GpsTimeSec, 0);
    m_parent->setParamAlarmSeverity(GpsTimeNSec, 0);
    m_parent->setParamAlarmSeverity(GpsTimeOff, 0);

    PIloop(syncpkt);
    m_parent->callParamCallbacks();
    return true;
}

bool TimeSync::RegressionLoop(const SyncResponsePacket* packet) {
/*
    if (m_parent->getIntegerParam(State) == static_cast<int>(STATE_UNSYNCED))
        return true;

    double duration = m_records.back().remoteTime - m_records.front().remoteTime;
    double offset = calcOffset();
    m_parent->setDoubleParam(SyncError, offset*1e6);

    if (m_records.size() < (size_t)m_parent->getIntegerParam(SyncSamples))
        return true;

    if (std::abs(offset) > m_parent->getDoubleParam(NoSyncThr)) {
        m_parent->setIntegerParam(State, static_cast<int>(STATE_UNSYNCED));
        return true;
    }

    m_offset += offset;
//    m_parent->setDoubleParam(SyncAccError, m_offset*1e6);

    if (m_parent->getIntegerParam(State) == static_cast<int>(STATE_ESTIMATING)) {
        m_pace = (offset != 0.0 ? duration / offset : 0x1FFFFFFF);
    } else if (offset != 0.0) {
        m_pace += duration / offset;
    }

    epicsTime t1, t2;
    t1 = epicsTime::getCurrent();
    sendPacket(epicsTime::getCurrent() + getCommDelay(), m_pace);
    t2 = epicsTime::getCurrent();
    m_parent->setDoubleParam(CommDly, (t2 - t1)*1e6);

    TimeRecord rec;
    rec.localTime = t2;
    rec.commDly = t2 - t1;
    rec.packetSeq = m_packetSeq;
    m_records.clear();
    m_records.push_back(std::move(rec));

    m_parent->setIntegerParam(RemoteTimePace, m_pace);
    m_parent->setIntegerParam(State, static_cast<int>(STATE_SYNCED));
*/
    return true;
}

bool TimeSync::PIloop(const SyncResponsePacket* packet) {

    double P_ = m_parent->getDoubleParam(P);
    double I_ = m_parent->getDoubleParam(I);

    if (m_records.size() < (size_t)m_parent->getIntegerParam(SyncSamples))
        return true;

    // Calc offset between remote and local clock
    double error = m_records.back().remoteTime - m_records.back().localTime;
    m_parent->setDoubleParam(SyncRawError, error*1e6);

    if (m_parent->getIntegerParam(State) == static_cast<int>(STATE_ESTIMATING)) {
        double duration = (m_records.back().localTime - m_records.front().localTime);
        m_offset = error;
        m_lastError = error;
        m_parent->setDoubleParam(SyncFinError, m_lastError*1e6);
        m_parent->setDoubleParam(SyncRawError, error*1e6);
        double Error = error * P_ + m_adjError * I_;
        m_parent->setDoubleParam(SyncAdjError, Error*1e6);
        m_reg = duration / error;

        sendPacket(epicsTime::getCurrent() + getCommDelay(), m_reg);
        m_records.clear();

        m_parent->setIntegerParam(State, static_cast<int>(STATE_SYNCED));
        return true;
    }

    // Something bad happened - not tracking any more
    if (std::abs(error) > (m_parent->getDoubleParam(NoSyncThr)/1e6)) {
        m_parent->setIntegerParam(State, static_cast<int>(STATE_UNSYNCED));
        return true;
    }

    m_adjError += error;

    double Error = error * P_ + m_adjError * I_;
    m_parent->setDoubleParam(SyncAdjError, Error*1e6);

    m_lastError += Error;
    m_parent->setDoubleParam(SyncFinError, m_lastError*1e6);

    double duration = m_records.back().localTime - m_records.front().localTime;
    if (m_lastError != 0.0)
        m_reg = duration/m_lastError;

    m_records.clear();
    return true;
}

double TimeSync::getSmoothOffset(double t)
{
    if (m_records.empty())
        return 0.0;

    // Do linear regression with least-square method
    double timesMean = 0.0;
    double offsetsMean = 0.0;
    for (auto rec: m_records) {
        timesMean += rec.localTime - m_records.front().localTime;
        offsetsMean += rec.remoteTime - rec.localTime;
    }
    timesMean /= m_records.size();
    offsetsMean /= m_records.size();

    double denominator = 0.0;
    double numerator = 0.0;
    for (auto rec: m_records) {
        double a = (rec.localTime - m_records.front().localTime) - timesMean;
        double b = (rec.localTime - rec.localTime) - offsetsMean;
        denominator += a*a;
        numerator += a*b;
    }
    double slope = numerator / denominator;
    double intercept = offsetsMean - slope * timesMean;

    return intercept + slope * t;
}

double TimeSync::getCommDelay()
{
    double delay = 0.0;
    double stddev;

    // First check if we can calculate delay from existing samples
    std::vector<double> v;
    for (auto it: m_records) {
        v.push_back(it.commDly);
    }
    if (calcMeanStddev(v, delay, stddev))
        return delay;

    // We have to measure it
    const unsigned nSamples = m_parent->getIntegerParam(CommDlySamples);

    uint32_t moduleId = m_parent->getHardwareId();
    moduleId ^= 0x80000000; // Try to select non-existing module id

    // Generate valid packet, size is what matters most
    std::array<uint8_t, sizeof(SyncRequestPacket)> buffer;
    SyncRequestPacket *packet = SyncRequestPacket::init(buffer.data(), buffer.size(), moduleId);
    if (packet == nullptr)
        return -1.0;

    // Do a round of time measurements and calculate average value
    delay = 0.0;
    epicsTime t1, t2;
    for (auto i = nSamples; i > 0; i--) {
        t1 = epicsTime::getCurrent();
        m_parent->sendUpstream(packet);
        t2 = epicsTime::getCurrent();
        delay += (t2 - t1);
    }
    delay /= nSamples;

    return delay;
}

bool TimeSync::sendPacket()
{
    auto moduleId = m_parent->getHardwareId();
    if (moduleId == 0)
        return false;

    m_outPacket->setModuleId(moduleId);
    m_packetSeq = (m_packetSeq + 1) & 0x1F;
    m_outPacket->setCmdId(m_packetSeq);
    m_outPacket->settime = false;
    m_outPacket->setpace = true;
    m_outPacket->pace = (std::abs(m_reg) & 0x1FFFFFFF);
    m_outPacket->direction = (m_reg < 0);
    m_parent->sendUpstream(m_outPacket);

    return true;
}

bool TimeSync::sendPacket(epicsTime t)
{
    return sendPacket(t, 0x1FFFFFFF);
}

bool TimeSync::sendPacket(epicsTime t, int pace)
{
    auto moduleId = m_parent->getHardwareId();
    if (moduleId == 0)
        return false;

    m_outPacket->setModuleId(moduleId);
    m_packetSeq = (m_packetSeq + 1) & 0x1F;
    m_outPacket->setCmdId(m_packetSeq);
    m_outPacket->timestamp = t;
    m_outPacket->settime = true;
    m_outPacket->setpace = true;
    m_outPacket->pace = (std::abs(pace) & 0x1FFFFFFF);
    m_outPacket->direction = (pace < 0);
    m_parent->sendUpstream(m_outPacket);

epicsTimeStamp ts;
epicsTimeGetCurrent(&ts);
fprintf(stderr, "TimeSync: req T=%d.%09d 0x%08X\n", ts.secPastEpoch, ts.nsec, m_outPacket->ctrl);
    return true;
}


bool TimeSync::sendPacket(int pace)
{
    auto moduleId = m_parent->getHardwareId();
    if (moduleId == 0)
        return false;

    m_outPacket->setModuleId(moduleId);
    m_packetSeq = (m_packetSeq + 1) & 0x1F;
    m_outPacket->setCmdId(m_packetSeq);
    m_outPacket->settime = false;
    m_outPacket->setpace = true;
    m_outPacket->pace = (std::abs(pace) & 0x1FFFFFFF);
    m_outPacket->direction = (pace < 0);
    m_parent->sendUpstream(m_outPacket);

epicsTimeStamp ts;
epicsTimeGetCurrent(&ts);
fprintf(stderr, "TimeSync: req T=%d.%09d 0x%08X\n", ts.secPastEpoch, ts.nsec, m_outPacket->ctrl);
    return true;
}

bool TimeSync::calcSyncOffset(double& mean, double& stddev)
{
    std::vector<double> v;
    for (auto it: m_records) {
        if (it.remoteTime != epicsTimeStamp{0,0})
            v.push_back(it.remoteTime - it.localTime);
    }
    return calcMeanStddev(v, mean, stddev);
}

bool TimeSync::calcGpsOffset(double& mean, double& stddev)
{
    std::vector<double> v;
    for (auto it: m_records) {
        if (it.gpsTime != epicsTimeStamp{0,0})
            v.push_back(it.gpsTime - it.localTime);
    }
    return calcMeanStddev(v, mean, stddev);
}

bool TimeSync::calcMeanStddev(const std::vector<double>& numbers, double& mean, double& stddev)
{
    if (numbers.empty())
        return false;

    mean = 0.0;
    stddev = 0.0;
    for (auto value: numbers) {
        mean += value;
    }
    mean /= numbers.size();
    for (auto value: numbers) {
        stddev += std::pow(value - mean, 2);
    }
    stddev = std::sqrt(stddev / numbers.size());
    return true;
}
