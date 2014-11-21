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
    , m_processDataCb(0)
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

    if (!!m_pvRecord && m_processDataCb != 0 && m_postDataCb != 0) {
        bool hasData = false;

        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;
            uint32_t dataLen = 0;

            const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
            const uint32_t *data = packet->getData(&dataLen);

            // TODO: what about metadata? In tof,pixel format?
            if (packet->isNeutronData() == false || rtdl == 0 || dataLen == 0)
                continue;

            // Extract timestamp and proton charge from RTDL
            // RTDL should always be present when working with DSP-T
            epicsTimeStamp time = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
            if (epicsTimeNotEqual(&time, &m_pulseTime)) {
                if (hasData == true) {
                    postData();
                    hasData = false;
                }
                m_pulseTime = { time.secPastEpoch, time.nsec };
                // Convert charge from 10pC unsigned integer to C double
                m_pulseCharge = static_cast<double>(rtdl->pulse_charge) * 10e-12;
            }

            m_processDataCb(this, data, dataLen);

            m_nProcessed++;
            hasData = true;
        }

        if (hasData == true) {
            postData();
        }
    }

    // Update parameters
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

void BasePvaPlugin::postData()
{
    if (m_postDataCb && !!m_pvRecord) {
        // EPICSv4 uses POSIX EPOCH, v3 uses EPICS EPOCH
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + m_pulseTime.secPastEpoch, m_pulseTime.nsec, m_postSeq++);

        m_pvRecord->beginGroupPut();
        m_pvRecord->timeStamp.set(time);
        m_pvRecord->proton_charge->put(m_pulseCharge);
        m_postDataCb(this, m_pvRecord);
        m_pvRecord->endGroupPut();
    }
}

void BasePvaPlugin::setCallbacks(ProcessDataCb procCb, PostDataCb postCb) {
    m_processDataCb = procCb;
    m_postDataCb = postCb;
}
