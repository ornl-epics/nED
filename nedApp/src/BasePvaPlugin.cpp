/* BasePvaPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BasePvaPlugin.h"
#include "Log.h"

#define NUM_BASEPVAPLUGIN_PARAMS 0 // ((int)(&LAST_BASEPVAPLUGIN_PARAM - &FIRST_BASEPVAPLUGIN_PARAM + 1))

BasePvaPlugin::BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, 0, NUM_BASEPVAPLUGIN_PARAMS)
    , m_nReceived(0)
    , m_nProcessed(0)
    , m_pulseTime({0, 0})
    , m_pulseCharge(0)
    , m_postSeq(0)
    , m_processPacketCb(0)
    , m_postDataCb(0)
{
    m_pvRecord = PvaNeutronData::create(pvName);
    if (!m_pvRecord)
        LOG_ERROR("Cannot create PVA record '%s'", pvName);
    else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pvRecord) == false)
        LOG_ERROR("Cannot register PVA record '%s'", pvName);
}

BasePvaPlugin::~BasePvaPlugin()
{}

asynStatus BasePvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        if (value > 0 && !m_pvRecord) {
            LOG_ERROR("Can't enable plugin, PV record not initialized");
            return asynError;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void BasePvaPlugin::processData(const DasPacketList * const packetList)
{
    m_nReceived += packetList->size();

    if (!!m_pvRecord && m_processPacketCb != 0 && m_postDataCb != 0) {
        bool hasData = false;

        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;

            const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();

            // TODO: what about metadata? In tof,pixel format?
            if (packet->isNeutronData() == false || rtdl == 0)
                continue;

            // Extract timestamp and proton charge from RTDL
            // RTDL should always be present when working with DSP-T
            epicsTimeStamp time = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
            if (epicsTimeNotEqual(&time, &m_pulseTime)) {
                if (hasData == true) {
                    m_postDataCb(this, m_pulseTime, m_pulseCharge, m_postSeq++);
                    hasData = false;
                }
                m_pulseTime = { time.secPastEpoch, time.nsec };
                m_pulseCharge = rtdl->charge;
            }

            m_processPacketCb(this, packet);

            m_nProcessed++;
            hasData = true;
        }

        if (hasData) {
            m_postDataCb(this, m_pulseTime, m_pulseCharge, m_postSeq++);
        }
    }

    // Update parameters
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

void BasePvaPlugin::setCallbacks(ProcessPacketCb procCb, PostDataCb postCb) {
    m_processPacketCb = procCb;
    m_postDataCb = postCb;
}
