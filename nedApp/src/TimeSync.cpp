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
    m_parent->createParam("TS:ForceSync",       asynParamInt32,   &ForceSync);
    m_parent->createParam("TS:SyncInt",         asynParamFloat64, &SyncInt,         0.1);
    m_parent->createParam("TS:SyncSamples",     asynParamInt32,   &SyncSamples,     10);
    m_parent->createParam("TS:SyncOffAvg",      asynParamFloat64, &SyncOffAvg,      0.0);
    m_parent->createParam("TS:SyncOffStd",      asynParamFloat64, &SyncOffStd,      0.0);
    m_parent->createParam("TS:GpsOffAvg",       asynParamFloat64, &GpsOffAvg,       0.0);
    m_parent->createParam("TS:GpsOffStd",       asynParamFloat64, &GpsOffStd,       0.0);
    m_parent->createParam("TS:CommDlySamples",  asynParamInt32,   &CommDlySamples,  3);
    m_parent->createParam("TS:CommDlyAvg",      asynParamFloat64, &CommDlyAvg,      0.0);
    m_parent->createParam("TS:CommDlyStd",      asynParamFloat64, &CommDlyStd,      0.0);
    m_parent->createParam("TS:NoSyncThr",       asynParamFloat64, &NoSyncThr,       10);
    m_parent->createParam("TS:AdjThr",          asynParamFloat64, &AdjThr,          0.1);
    m_parent->createParam("TS:RemoteTimeSec",   asynParamInt32,   &RemoteTimeSec,   0);
    m_parent->createParam("TS:RemoteTimeNSec",  asynParamInt32,   &RemoteTimeNSec,  0);
    m_parent->createParam("TS:LocalTimeSec",    asynParamInt32,   &LocalTimeSec,    0);
    m_parent->createParam("TS:LocalTimeNSec",   asynParamInt32,   &LocalTimeNSec,   0);
    m_parent->createParam("TS:GpsTimeSec",      asynParamInt32,   &GpsTimeSec,      0);
    m_parent->createParam("TS:GpsTimeNSec",     asynParamInt32,   &GpsTimeNSec,     0);
    m_parent->createParam("TS:RemotePace",      asynParamInt32,   &RemotePace,      0);

    m_parent->createParam("TS:SetPace",         asynParamInt32,   &SetPace);

    std::function<float(void)> periodicCb = std::bind(&TimeSync::timerCb, this);
    m_timer.schedule(periodicCb, 0.1);

    m_buffer.reset(new uint8_t[sizeof(SyncRequestPacket)]);
    assert(!!m_buffer);

    m_outPacket = SyncRequestPacket::init(m_buffer.get(), sizeof(SyncRequestPacket), 0x0);
    assert(m_outPacket != nullptr);
}

bool TimeSync::setParam(int param, int value)
{
    if (param == ForceSync) {
        if (m_parent->getIntegerParam(State) == static_cast<int>(STATE_UNSYNCED)) {
            double offset;
            double stddev;
            if (!calcCommDelay(offset, stddev))
                offset = measCommDelay();
            if (sendPacket(epicsTime::getCurrent() + offset)) {
                m_parent->setIntegerParam(State, static_cast<int>(STATE_SYNCED));
                m_parent->callParamCallbacks();
                m_records.clear();
            } else {
                // LOG_ERROR
            }
        }
        return true;
    }
    if (param == SetPace) {
        m_outPacket->setModuleId(m_parent->getHardwareId());
        m_packetSeq = (m_packetSeq + 1) & 0x1F;
        m_outPacket->setCmdId(m_packetSeq);
        m_pace = (value & 0x7FFFFFFF);
        m_outPacket->ctrl = m_pace;
        m_parent->sendUpstream(m_outPacket);
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
    double interval = 1.0; // Default value when we're disabled
    m_parent->lock();

    if (m_parent->getIntegerParam(State) != static_cast<int>(STATE_DISABLED)) {
        // Only keep requested number of samples
        unsigned nSamples = m_parent->getIntegerParam(SyncSamples);
        while (m_records.size() > nSamples) {
            m_records.erase(m_records.begin());
        }

        // Check for lost communication
        size_t noRspCnt = 0;
        for (auto it = m_records.rbegin(); it != m_records.rend(); it++) {
            if (it->remoteTime != epicsTimeStamp{0,0})
                break;
            noRspCnt++;
        }
        if (noRspCnt > 0) {
            m_parent->setParamAlarmStatus(GpsTimeSec, epicsAlarmNone);
            m_parent->setParamAlarmStatus(GpsTimeNSec, epicsAlarmNone);
            m_parent->setParamAlarmSeverity(GpsTimeSec, 0);
            m_parent->setParamAlarmSeverity(GpsTimeNSec, 0);
            m_parent->setParamAlarmStatus(RemoteTimeSec, epicsAlarmNone);
            m_parent->setParamAlarmStatus(RemoteTimeNSec, epicsAlarmNone);
            m_parent->setParamAlarmSeverity(RemoteTimeSec, 0);
            m_parent->setParamAlarmSeverity(RemoteTimeNSec, 0);
            if (noRspCnt > (m_records.size() / 2)) {
                m_parent->setIntegerParam(State, static_cast<int>(STATE_UNSYNCED));
            }
        }

        epicsTime t1, t2;
        t1 = epicsTime::getCurrent();
        (void)sendPacket();
        t2 = epicsTime::getCurrent();

        TimeRecord rec;
        rec.localTime = t2;
        rec.commDly = t2 - t1;
        rec.packetSeq = m_packetSeq;
        m_records.push_back(std::move(rec));

        interval = m_parent->getDoubleParam(SyncInt);
    }

    epicsTimeStamp now;
    epicsTimeGetCurrent(&now);
    m_parent->setIntegerParam(LocalTimeSec, now.secPastEpoch);
    m_parent->setIntegerParam(LocalTimeNSec, now.nsec);
    updateParams();

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

    // Map response packet to the sent request
    auto syncpkt = reinterpret_cast<const SyncResponsePacket*>(packet);
fprintf(stderr, "TimeSync: rsp GPS=%u.%09u DSP=%u.%09u 0x%08X\n", syncpkt->gpstime.secPastEpoch, syncpkt->gpstime.nsec, syncpkt->remotetime.secPastEpoch, syncpkt->remotetime.nsec, syncpkt->ctrl);
    for (auto it = m_records.rbegin(); it != m_records.rend(); it++) {
        if (it->packetSeq == packet->getCmdId() && it->remoteTime == epicsTimeStamp{0, 0}) {
            it->remoteTime = syncpkt->remotetime;
            epicsTime gps = syncpkt->gpstime;
            gps += syncpkt->tof/10000000.0;
            it->gpsTime = gps;
            m_parent->setIntegerParam(RemoteTimeSec, syncpkt->remotetime.secPastEpoch);
            m_parent->setIntegerParam(RemoteTimeNSec, syncpkt->remotetime.nsec);
            m_parent->setParamAlarmStatus(RemoteTimeSec, epicsAlarmNone);
            m_parent->setParamAlarmStatus(RemoteTimeNSec, epicsAlarmNone);
            m_parent->setParamAlarmSeverity(RemoteTimeSec, 0);
            m_parent->setParamAlarmSeverity(RemoteTimeNSec, 0);
            m_parent->setIntegerParam(GpsTimeSec, syncpkt->gpstime.secPastEpoch);
            m_parent->setIntegerParam(GpsTimeNSec, syncpkt->gpstime.nsec);
            m_parent->setParamAlarmStatus(GpsTimeSec, epicsAlarmNone);
            m_parent->setParamAlarmStatus(GpsTimeNSec, epicsAlarmNone);
            m_parent->setParamAlarmSeverity(GpsTimeSec, 0);
            m_parent->setParamAlarmSeverity(GpsTimeNSec, 0);
            m_parent->callParamCallbacks();
            break;
        }
    }

    if (m_records.size() < m_parent->getIntegerParam(SyncSamples))
        return true;

    // Measure clocks offset, take actions if necessary
    double offset, stddev;
    calcSyncOffset(offset, stddev);

    if (std::abs(offset) > (m_parent->getDoubleParam(NoSyncThr)/1000)) {
        // This is an easy one - hopefully is never happens
fprintf(stderr, "TimeSync: =>UNSYNC offset=%.06f (stddev=%.06f n=%zu)\n", offset, stddev, m_records.size());
        m_parent->setIntegerParam(State, static_cast<int>(STATE_UNSYNCED));
        m_parent->callParamCallbacks();
        return true;
    }
fprintf(stderr, "TimeSync: offset=%f max=%f\n", offset, m_parent->getDoubleParam(AdjThr)/1000);
    if (std::abs(offset) > (m_parent->getDoubleParam(AdjThr)/1000)) {
        // Idea is to adjust the remote clock pace, and while sending packet also set time

        // Pace is calculated as Duration/Offset
        auto first = m_records.begin();
        for (; first != m_records.end(); first++) {
            if (first->remoteTime != epicsTimeStamp{0,0}) {
                break;
            }
        }
        auto last = m_records.rbegin();
        for (; last != m_records.rend(); last++) {
            if (last->remoteTime != epicsTimeStamp{0,0}) {
                break;
            }
        }
        if (first != m_records.end() && last != m_records.rend()) {
            double timeDiff = last->remoteTime - first->remoteTime;
            double offsetDiff = (last->remoteTime - last->localTime) - (first->remoteTime - first->localTime);
fprintf(stderr, "TimeSync: timeDiff=%.08f offsetDiff=%.08f\n", timeDiff, offsetDiff);
            int pace = timeDiff / offsetDiff;

            double delay;
            if (!calcCommDelay(delay, stddev))
                delay = measCommDelay();

fprintf(stderr, "TimeSync: sendPacket(t, pace=%d)\n", pace);
            sendPacket(epicsTime::getCurrent() + delay, pace);
            m_records.clear();

            m_parent->setIntegerParam(RemotePace, pace);
            m_parent->callParamCallbacks();
        }
        return true;
    }

    return true;
}

double TimeSync::measCommDelay()
{
    const unsigned nSamples = m_parent->getIntegerParam(CommDlySamples);

    uint32_t moduleId = m_parent->getHardwareId();
    moduleId ^= 0x80000000; // Try to select non-existing module id

    // Generate valid packet, size is what matters most
    std::array<uint8_t, sizeof(SyncRequestPacket)> buffer;
    SyncRequestPacket *packet = SyncRequestPacket::init(buffer.data(), buffer.size(), moduleId);
    if (packet == nullptr)
        return -1.0;

    // Do a round of time measurements and calculate average value
    double avg = 0.0;
    epicsTime t1, t2;
    for (auto i = nSamples; i > 0; i--) {
        t1 = epicsTime::getCurrent();
        m_parent->sendUpstream(packet);
        t2 = epicsTime::getCurrent();
        avg += (t2 - t1);
    }
    avg /= nSamples;

    return avg;
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
    m_outPacket->direction = (m_pace & 0x20000000);
    m_outPacket->pace = (m_pace & 0x1FFFFFFF);
    m_parent->sendUpstream(m_outPacket);
    return true;
}

bool TimeSync::sendPacket(epicsTime t)
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
    m_outPacket->direction = (m_pace & 0x20000000);
    m_outPacket->pace = (m_pace & 0x1FFFFFFF);
    m_outPacket->ctrl = m_pace;
    m_outPacket->settime = true;
    m_parent->sendUpstream(m_outPacket);
fprintf(stderr, "Set time seqId=%u\n", m_packetSeq);
    return true;
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
    m_pace = m_outPacket->ctrl;
    m_parent->sendUpstream(m_outPacket);
    return true;
}

void TimeSync::updateParams()
{
    double mean;
    double stddev;

    if (calcSyncOffset(mean, stddev)) {
        m_parent->setDoubleParam(SyncOffAvg, mean*1000000);
        m_parent->setDoubleParam(SyncOffStd, stddev*1000000);
    }

    if (calcGpsOffset(mean, stddev)) {
        m_parent->setDoubleParam(GpsOffAvg, mean*1000000);
        m_parent->setDoubleParam(GpsOffStd, stddev*1000000);
    }

    if (calcCommDelay(mean, stddev)) {
        m_parent->setDoubleParam(CommDlyAvg, mean*1000000);
        m_parent->setDoubleParam(CommDlyStd, stddev*1000000);
    }

    m_parent->callParamCallbacks();
}

bool TimeSync::calcCommDelay(double& mean, double& stddev)
{
    std::vector<double> v;
    for (auto it: m_records) {
        v.push_back(it.commDly);
    }
    return calcMeanStddev(v, mean, stddev);
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
