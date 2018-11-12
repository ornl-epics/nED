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

class TimeSync {
    private:
        enum {
            STATE_DISABLED,
            STATE_UNSYNCED,
            STATE_SYNCED,
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
        uint32_t m_pace{0x3FFFFFFF};

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
        double measCommDelay();
        void updateParams();
        bool calcCommDelay(double& mean, double& stddev);
        bool calcSyncOffset(double& mean, double& stddev);
        bool calcGpsOffset(double& mean, double& stddev);
        bool calcMeanStddev(const std::vector<double>& numbers, double& mean, double& stddev);

    private:
        int Enable;
        int State;
        int ForceSync;
        int SyncInt;
        int SyncSamples;
        int SyncOffAvg;
        int SyncOffStd;
        int GpsOffAvg;
        int GpsOffStd;
        int CommDlyAvg;
        int CommDlyStd;
        int CommDlySamples;
        int NoSyncThr;
        int AdjThr;
        int LocalTimeSec;
        int LocalTimeNSec;
        int RemoteTimeSec;
        int RemoteTimeNSec;
        int GpsTimeSec;
        int GpsTimeNSec;
        int RemotePace;

        int SetPace; // TODO: remove
};

#endif // TIMESYNC_H
