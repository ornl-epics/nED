/* RtdlPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "RtdlPlugin.h"

#include <climits>

EPICS_REGISTER_PLUGIN(RtdlPlugin, 2, "Port name", string, "Parent plugins", string);

#define TIMESTAMP_FORMAT        "%Y/%m/%d %T.%09f"

RtdlPlugin::RtdlPlugin(const char *portName, const char *parentPlugins)
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
    createParam("IntraPulseTime",   asynParamInt32, &IntraPulseTime); // READ - Number of ns between reference pulses
    createParam("TofFullOffset",    asynParamInt32, &TofFullOffset);// READ - TOF full offset
    createParam("FrameOffset",      asynParamInt32, &FrameOffset);  // READ - Frame offset
    createParam("TofFixOffset",     asynParamInt32, &TofFixOffset); // READ - TOF fixed offset
    createParam("RingPeriod",       asynParamInt32, &RingPeriod);   // READ - Ring revolution period
    createParam("ErrorsFutureTime", asynParamInt32, &ErrorsFutureTime, -1); // READ - Number of errors when time jumps in the future
    createParam("ErrorsPastTime",   asynParamInt32, &ErrorsPastTime, 0);    // READ - Number of errors when time jumps in the past

    BasePlugin::connect(parentPlugins, MsgDasRtdl);
}

void RtdlPlugin::recvDownstream(DasRtdlPacketList *packets)
{
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
        setIntegerParam(PulseCharge,        packet->pulse.charge);
        setIntegerParam(BadVetoFrame,       packet->pulse.bad_veto_frame);
        setIntegerParam(BadCycleFrame,      packet->pulse.bad_cycle_frame);
        setIntegerParam(Tstat,              packet->pulse.tstat);
        setIntegerParam(PrevCycleVeto,      packet->pulse.last_cycle_veto);
        setIntegerParam(Cycle,              packet->pulse.cycle);
        setIntegerParam(IntraPulseTime,     packet->correction.tsync_period * 100);
        setIntegerParam(TofFullOffset,      packet->correction.tof_full_offset);
        setIntegerParam(FrameOffset,        packet->correction.frame_offset);
        setIntegerParam(TofFixOffset,       packet->correction.tof_fixed_offset);
        setIntegerParam(RingPeriod,         ringPeriod);

    }

    callParamCallbacks();
}
