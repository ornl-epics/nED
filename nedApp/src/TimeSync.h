/* TimeSync.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <epicsTime.h>
#include <memory>
#include <vector>

class BaseModulePlugin;
class SyncRequestPacket;
class SyncResponsePacket;

class TimeSync {
    private:
        enum {
            STATE_DISABLED,
            STATE_UNSYNCED,
            STATE_SYNCED,
            STATE_ESTIMATING,
        };
        struct TimeRecord {
            double commDly;
            epicsTime localTime;
            epicsTime remoteTime;
            epicsTime gpsTime;
            uint8_t packetSeq;
        };

        BaseModulePlugin* m_parent;     //!< Parent asynPortDriver class, used for PV communication
        uint8_t m_packetSeq = 0;        //!< Current packet sequence id
        Timer m_timer;                  //!< Periodic timer to trigger new sync packet
        std::vector<TimeRecord> m_records;
        std::unique_ptr<uint8_t> m_buffer;
        SyncRequestPacket* m_outPacket;
        int m_reg = 0x1FFFFFFF;
        double m_lastError = 0.0;
        double m_intError = 0.0;

    public:
        TimeSync(BaseModulePlugin* parent);

        //External events taken by this state machine
        bool setParam(int param, int value);
        bool rspTimeSync(const DasCmdPacket* packet);

    private:
        float timerCb();
        bool sendPacket();
        bool sendPacket(epicsTime t);
        bool sendPacket(epicsTime t, int pace);
        bool sendPacket(int pace);
        double getCommDelay();
        bool calcMeanStddev(const std::vector<double>& numbers, double& mean, double& stddev);
        double getSmoothOffset(double t);
        void PIloop(const SyncResponsePacket* packet);

    private:
        int Enable;
        int State;
        int SyncForce;
        int SyncInt;
        int SyncSamples;
        int CommDly;
        int CommDlySamples;
        int NoSyncThr;
        int LocalTimeSec;
        int LocalTimeNSec;
        int RemoteTimeSec;
        int RemoteTimeNSec;
        int RemoteTimeOff;
        int RemoteTimePace;
        int GpsTimeSec;
        int GpsTimeNSec;
        int GpsTimeOff;
        int SyncInErr;
        int SyncAdjErr;
        int SyncOutErr;
        int P;
        int I;
};

#endif // TIMESYNC_H
