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

#define NUM_RTDLPLUGIN_PARAMS ((int)(&LAST_RTDLPLUGIN_PARAM - &FIRST_RTDLPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(RtdlPlugin, 3, "Port name", string, "Dispatcher port name", string, "Blocking", int);

#define TIMESTAMP_FORMAT        "%Y/%m/%d %T.%09f"

RtdlPlugin::RtdlPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_RTDLPLUGIN_PARAMS, 1, asynOctetMask, asynOctetMask)
    , m_receivedCount(0)
    , m_processedCount(0)
{
    createParam("Timestamp",        asynParamOctet, &Timestamp);    // READ - Timestamp string of last RTDL
    createParam("BadPulse",         asynParamInt32, &BadPulse);     // READ - Bad pulse indicator (0=bad pulse, 1=good pulse)
    createParam("PulseFlav",        asynParamInt32, &PulseFlav);    // READ - Pulse flavor
    createParam("PulseCharg",       asynParamInt32, &PulseCharg);   // READ - Pulse charge
    createParam("BadVetoFr",        asynParamInt32, &BadVetoFr);    // READ - Bad veto frame
    createParam("BadCycleFr",       asynParamInt32, &BadCycleFr);   // READ - Bad cycle frame
    createParam("Tstat",            asynParamInt32, &Tstat);        // READ - TSTAT
    createParam("Veto",             asynParamInt32, &Veto);         // READ - Veto frame
    createParam("Cycle",            asynParamInt32, &Cycle);        // READ - Cycle frame
    createParam("IntraPTime",       asynParamInt32, &IntraPTime);   // READ - Number of ns between reference pulses
    createParam("TofFullOff",       asynParamInt32, &TofFullOff);   // READ - TOF full offset
    createParam("FrameOff",         asynParamInt32, &FrameOff);     // READ - Frame offset
    createParam("TofFixOff",        asynParamInt32, &TofFixOff);    // READ - TOF fixed offset
}

void RtdlPlugin::processData(const DasPacketList * const packetList)
{
    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        m_receivedCount++;
        if (packet->isRtdl()) {
            const DasPacket::RtdlHeader *rtdl = reinterpret_cast<const DasPacket::RtdlHeader *>(packet->getPayload());
            epicsTimeStamp rtdlTime;
            char rtdlTimeStr[64];

            // Format time
            rtdlTime.secPastEpoch = rtdl->timestamp_sec;
            rtdlTime.nsec = rtdl->timestamp_nsec;
            epicsTimeToStrftime(rtdlTimeStr, sizeof(rtdlTimeStr), TIMESTAMP_FORMAT, &rtdlTime);

            setStringParam(Timestamp,           rtdlTimeStr);
            setIntegerParam(BadPulse,           rtdl->bad_pulse);
            setIntegerParam(PulseFlav,          rtdl->pulse_flavor);
            setIntegerParam(PulseCharg,         rtdl->pulse_charge);
            setIntegerParam(BadVetoFr,          rtdl->bad_veto_frame);
            setIntegerParam(BadCycleFr,         rtdl->bad_cycle_frame);
            setIntegerParam(Tstat,              rtdl->tstat);
            setIntegerParam(Veto,               rtdl->veto);
            setIntegerParam(Cycle,              rtdl->cycle);
            setIntegerParam(IntraPTime,         rtdl->tsync_width * 100);
            setIntegerParam(TofFullOff,         rtdl->tof_full_offset);
            setIntegerParam(FrameOff,           rtdl->frame_offset);
            setIntegerParam(TofFixOff,          rtdl->tof_fixed_offset);

            m_processedCount++;
        }
    }

    setIntegerParam(RxCount, m_receivedCount % INT_MAX);
    setIntegerParam(ProcCount, m_processedCount % INT_MAX);

    callParamCallbacks();
}
