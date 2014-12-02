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

const uint32_t BasePvaPlugin::CACHE_SIZE = 32*1024;
const std::string BasePvaPlugin::PV_NEUTRONS("Neutrons");
const std::string BasePvaPlugin::PV_METADATA("Metadata");

BasePvaPlugin::BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, 0, NUM_BASEPVAPLUGIN_PARAMS)
    , m_nReceived(0)
    , m_nProcessed(0)
    , m_pulseTime({0, 0})
    , m_pulseCharge(0)
    , m_postSeq(0)
    , m_processNeutronCb(0)
    , m_postNeutronsCb(0)
{
    m_pvNeutrons = PvaNeutronData::create(pvPrefix + PV_NEUTRONS);
    if (!m_pvNeutrons)
        LOG_ERROR("Cannot create PVA record '%s%s'", pvPrefix, PV_NEUTRONS.c_str());
    else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pvNeutrons) == false) {
        LOG_ERROR("Cannot register PVA record '%s%s'", pvPrefix, PV_NEUTRONS.c_str());
        m_pvNeutrons.reset();
    }

    m_pvMetadata = PvaNeutronData::create(pvPrefix + PV_METADATA);
    if (!m_pvMetadata)
        LOG_ERROR("Cannot create PVA record '%s%s'", pvPrefix, PV_METADATA.c_str());
    else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pvMetadata) == false) {
        LOG_ERROR("Cannot register PVA record '%s%s'", pvPrefix, PV_METADATA.c_str());
        m_pvNeutrons.reset();
    }
}

BasePvaPlugin::~BasePvaPlugin()
{}

asynStatus BasePvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        if (value > 0 && (!m_pvNeutrons || !m_pvMetadata)) {
            LOG_ERROR("Can't enable plugin, PV records not initialized");
            return asynError;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void BasePvaPlugin::processData(const DasPacketList * const packetList)
{
    m_nReceived += packetList->size();

    // Plugin is not enabled if m_pvNeutrons or m_pvMetadata are not valid -
    // no need to verify

    bool haveNeutrons = false;
    bool haveMetadata = false;

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;
        uint32_t dataLen = 0;

        const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
        const uint32_t *data = packet->getData(&dataLen);

        if (packet->isData() == false || rtdl == 0 || dataLen == 0)
            continue;

        // Extract timestamp and proton charge from RTDL
        // RTDL should always be present when working with DSP-T
        epicsTimeStamp time = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
        if (epicsTimeNotEqual(&time, &m_pulseTime)) {
            postData(haveNeutrons, haveMetadata);
            haveNeutrons = haveMetadata = false;

            m_pulseTime = { time.secPastEpoch, time.nsec };
            // Convert charge from 10pC unsigned integer to C double
            m_pulseCharge = static_cast<double>(rtdl->pulse_charge * 10) * 10e-12;
        }

        if (packet->isMetaData()) {
            processMetaData(data, dataLen);
            haveMetadata = true;
        } else if (m_processNeutronCb) {
            m_processNeutronCb(this, data, dataLen);
            haveNeutrons = true;
        }

        m_nProcessed++;
    }

    postData(haveNeutrons, haveMetadata);

    // Update parameters
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

void BasePvaPlugin::postData(bool postNeutrons, bool postMetadata)
{
    if (postNeutrons && m_postNeutronsCb) {
        // EPICSv4 uses POSIX EPOCH, v3 uses EPICS EPOCH
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + m_pulseTime.secPastEpoch, m_pulseTime.nsec, m_postSeq++);

        m_pvNeutrons->beginGroupPut();
        m_pvNeutrons->timeStamp.set(time);
        m_pvNeutrons->proton_charge->put(m_pulseCharge);
        m_postNeutronsCb(this, m_pvNeutrons);
        m_pvNeutrons->endGroupPut();
    }

    if (postMetadata) {
        // EPICSv4 uses POSIX EPOCH, v3 uses EPICS EPOCH
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + m_pulseTime.secPastEpoch, m_pulseTime.nsec, m_postSeq++);

        m_pvMetadata->beginGroupPut();
        m_pvMetadata->timeStamp.set(time);
        m_pvMetadata->proton_charge->put(m_pulseCharge);
        m_pvNeutrons->time_of_flight->replace(freeze(m_cacheMeta.time_of_flight));
        m_pvNeutrons->pixel->replace(freeze(m_cacheMeta.pixel));
        m_pvMetadata->endGroupPut();

        // Reduce gradual memory reallocation by pre-allocating instead of clear()
        m_cacheMeta.time_of_flight.reserve(CACHE_SIZE);
        m_cacheMeta.pixel.reserve(CACHE_SIZE);
    }
}

void BasePvaPlugin::processMetaData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(DasPacket::Event) / sizeof(uint32_t));
    const DasPacket::Event *events = reinterpret_cast<const DasPacket::Event *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cacheMeta.time_of_flight.push_back(events->tof);
        m_cacheMeta.pixel.push_back(events->pixelid);
        events++;
    }
}

void BasePvaPlugin::setCallbacks(ProcessDataCb procCb, PostDataCb postCb) {
    m_processNeutronCb = procCb;
    m_postNeutronsCb = postCb;
}