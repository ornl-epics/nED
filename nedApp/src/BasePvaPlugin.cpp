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

#include <limits>

#define NUM_BASEPVAPLUGIN_PARAMS 0 // ((int)(&LAST_BASEPVAPLUGIN_PARAM - &FIRST_BASEPVAPLUGIN_PARAM + 1))

BasePvaPlugin::BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, 0, NUM_BASEPVAPLUGIN_PARAMS)
    , m_userTagCounter(0)
    , m_nReceived(0)
    , m_nProcessed(0)
    , m_currentPulseTime(epicsTimeStamp({0, 0}))
    , m_currentPulseCharge(0)
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

    if (!!m_pvRecord) {
        bool dataToPost = false;

        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;

            m_nReceived++;

            const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();

            // TODO: what about metadata? In tof,pixel format?
            if (packet->isNeutronData() == false || rtdl == 0)
                continue;

            // Extract timestamp and proton charge from RTDL
            // RTDL should always be present when working with DSP-T
            epicsTimeStamp time = {rtdl->timestamp_sec, rtdl->timestamp_nsec};
            if (epicsTimeNotEqual(&time, &m_currentPulseTime)) {
                // new pulse detected, post any outstanding packets, go to next
                if (dataToPost == true) {
                    postData(m_currentPulseTime, m_currentPulseCharge);
                    dataToPost = false;
                }
                m_currentPulseTime = time;
                m_currentPulseCharge = rtdl->charge;
            }

            // Switch with virtual function is faster than std::function
            switch (getDataMode()) {
            case DATA_MODE_NORMAL:
                processNormalPacket(packet);
                break;
            case DATA_MODE_RAW:
                processRawPacket(packet);
                break;
            case DATA_MODE_EXTENDED:
                processExtendedPacket(packet);
                break;
            }

            m_nProcessed++;
            dataToPost = true;
        }

        if (dataToPost > 0) {
            postData(m_currentPulseTime, m_currentPulseCharge);
        }
    }

    // Update parameters
    setIntegerParam(ProcCount,  m_nProcessed % std::numeric_limits<int32_t>::max());
    setIntegerParam(RxCount,    m_nReceived  % std::numeric_limits<int32_t>::max());
    callParamCallbacks();
}

void BasePvaPlugin::postData(const epicsTimeStamp &pulseTime, uint32_t pulseCharge) {
    switch (getDataMode()) {
    case DATA_MODE_NORMAL:
        postNormalData(pulseTime, pulseCharge, m_userTagCounter++);
        break;
    case DATA_MODE_RAW:
        postRawData(pulseTime, pulseCharge, m_userTagCounter++);
        break;
    case DATA_MODE_EXTENDED:
        postExtendedData(pulseTime, pulseCharge, m_userTagCounter++);
        break;
    }
}
