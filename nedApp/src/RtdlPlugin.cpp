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
}

void RtdlPlugin::processData(const DasPacketList * const packetList)
{
    m_receivedCount += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        if (packet->isRtdl()) {
            const RtdlHeader *rtdl = reinterpret_cast<const RtdlHeader *>(packet->getPayload());
            epicsTimeStamp rtdlTime;
            char rtdlTimeStr[64];

            // Format time
            rtdlTime.secPastEpoch = rtdl->timestamp_sec;
            rtdlTime.nsec = rtdl->timestamp_nsec;
            epicsTimeToStrftime(rtdlTimeStr, sizeof(rtdlTimeStr), TIMESTAMP_FORMAT, &rtdlTime);

            setStringParam(Timestamp,           rtdlTimeStr);
            setIntegerParam(BadPulse,           rtdl->pulse.bad);
            setIntegerParam(PulseFlavor,        rtdl->pulse.flavor);
            setIntegerParam(PulseCharge,        rtdl->pulse.charge);
            setIntegerParam(BadVetoFrame,       rtdl->bad_veto_frame);
            setIntegerParam(BadCycleFrame,      rtdl->bad_cycle_frame);
            setIntegerParam(Tstat,              rtdl->tstat);
            setIntegerParam(PrevCycleVeto,      rtdl->last_cycle_veto);
            setIntegerParam(Cycle,              rtdl->cycle);
            setIntegerParam(IntraPulseTime,     rtdl->tsync_width * 100);
            setIntegerParam(TofFullOffset,      rtdl->tof_full_offset);
            setIntegerParam(FrameOffset,        rtdl->frame_offset);
            setIntegerParam(TofFixOffset,       rtdl->tof_fixed_offset);

            m_processedCount++;
        }
    }

    setIntegerParam(RxCount, m_receivedCount % INT_MAX);
    setIntegerParam(ProcCount, m_processedCount % INT_MAX);

    callParamCallbacks();
}
