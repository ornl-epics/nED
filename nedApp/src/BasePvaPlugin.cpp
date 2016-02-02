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

#define NUM_BASEPVAPLUGIN_PARAMS ((int)(&LAST_BASEPVAPLUGIN_PARAM - &FIRST_BASEPVAPLUGIN_PARAM + 1))

const uint32_t BasePvaPlugin::CACHE_SIZE = 32*1024;
const std::string BasePvaPlugin::PV_NEUTRONS("Neutrons");
const std::string BasePvaPlugin::PV_METADATA("Metadata");

const int BasePvaPlugin::interfaceMask = asynOctetMask | BasePlugin::defaultInterfaceMask;
const int BasePvaPlugin::interruptMask = asynOctetMask | BasePlugin::defaultInterfaceMask;

BasePvaPlugin::BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix, int numParams)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, /*blocking=*/0,
                 numParams + NUM_BASEPVAPLUGIN_PARAMS,
                 /*maxAddr=*/0, interfaceMask, interruptMask)
    , m_nReceived(0)
    , m_nProcessed(0)
    , m_pulseTime({0, 0})
    , m_pulseCharge(0)
    , m_neutronsPostSeq(0)
    , m_metadataPostSeq(0)
    , m_neutronsEn(true)
    , m_metadataEn(true)
    , m_processNeutronsCb(0)
    , m_postNeutronsCb(0)
{
    std::string pvNameNeutrons = pvPrefix + PV_NEUTRONS;
    m_pvNeutrons = PvaNeutronData::create(pvNameNeutrons);
    if (!m_pvNeutrons)
        LOG_ERROR("Cannot create PVA record '%s%s'", pvPrefix, PV_NEUTRONS.c_str());
    else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pvNeutrons) == false) {
        LOG_ERROR("Cannot register PVA record '%s%s'", pvPrefix, PV_NEUTRONS.c_str());
        m_pvNeutrons.reset();
    }

    std::string pvNameMetadata = pvPrefix + PV_METADATA;
    m_pvMetadata = PvaMetaData::create(pvNameMetadata);
    if (!m_pvMetadata)
        LOG_ERROR("Cannot create PVA record '%s%s'", pvPrefix, PV_METADATA.c_str());
    else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pvMetadata) == false) {
        LOG_ERROR("Cannot register PVA record '%s%s'", pvPrefix, PV_METADATA.c_str());
        m_pvNeutrons.reset();
    }

    createParam("PvNeutronsName", asynParamOctet, &PvNeutronsName, pvNameNeutrons.c_str()); // READ - Name of Neutrons PV
    createParam("PvMetadataName", asynParamOctet, &PvMetadataName, pvNameMetadata.c_str()); // READ - Name of Metadata PV
    createParam("PvNeutronsEn",   asynParamInt32, &PvNeutronsEn, 1);                        // WRITE - Enable exporting Neutrons PV
    createParam("PvMetadataEn",   asynParamInt32, &PvMetadataEn, 1);                        // WRITE - Enable exporting Metadata PV
    callParamCallbacks();
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
    } else if (pasynUser->reason == PvNeutronsEn) {
        m_neutronsEn = (value > 0);
    } else if (pasynUser->reason == PvMetadataEn) {
        m_metadataEn = (value > 0);
    } else if (pasynUser->reason == DataModeP) {
        flushData();
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

        const RtdlHeader *rtdl = packet->getRtdlHeader();
        const uint32_t *data = packet->getData(&dataLen);

        if (packet->isData() == false || rtdl == 0)
            continue;

        // Extract timestamp and proton charge from RTDL
        // RTDL should always be present when working with DSP-T
        epicsTimeStamp time = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
        if (epicsTimeNotEqual(&time, &m_pulseTime)) {
            postData(haveNeutrons, haveMetadata);
            haveNeutrons = haveMetadata = false;

            m_pulseTime = { time.secPastEpoch, time.nsec };
            // Convert charge from 10pC unsigned integer to C double
            m_pulseCharge = static_cast<double>(rtdl->pulse.charge * 10) * 1e-12;
        }

        if (packet->isMetaData() && m_metadataEn) {
            processMetaData(data, dataLen);
            haveMetadata = true;
            m_nProcessed++;
        } else if (packet->isNeutronData() && m_processNeutronsCb && m_neutronsEn) {
            m_processNeutronsCb(this, data, dataLen);
            haveNeutrons = true;
            m_nProcessed++;
        }
    }

    postData(haveNeutrons, haveMetadata);

    // Update parameters
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
}

void BasePvaPlugin::postData(bool postNeutrons, bool postMetadata)
{
    // 32 bit sequence number is good for around 18 months.
    // (based 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
    // In worst case client will skip one packet on rollover and then recover
    // the sequence.

    if (postNeutrons && m_postNeutronsCb) {
        // EPICSv4 uses POSIX EPOCH, v3 uses EPICS EPOCH
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + m_pulseTime.secPastEpoch, m_pulseTime.nsec, m_neutronsPostSeq++);

        m_pvNeutrons->lock();
        try {
            m_pvNeutrons->beginGroupPut();
            m_pvNeutrons->timeStamp.set(time);
            m_pvNeutrons->proton_charge->put(m_pulseCharge);
            m_postNeutronsCb(this, m_pvNeutrons);
            m_pvNeutrons->endGroupPut();
        } catch (std::exception &e) {
            LOG_ERROR("Exception caught in BasePvaPlugin::postData, postNeutrons: %s.", e.what());
        }
        m_pvNeutrons->unlock();
    }

    if (postMetadata) {
        // EPICSv4 uses POSIX EPOCH, v3 uses EPICS EPOCH
        epics::pvData::TimeStamp time(epics::pvData::posixEpochAtEpicsEpoch + m_pulseTime.secPastEpoch, m_pulseTime.nsec, m_metadataPostSeq++);

        m_pvMetadata->lock();
        try {
            m_pvMetadata->beginGroupPut();
            m_pvMetadata->timeStamp.set(time);
            m_pvMetadata->proton_charge->put(m_pulseCharge);
            m_pvMetadata->time_of_flight->replace(freeze(m_cacheMeta.time_of_flight));
            m_pvMetadata->pixel->replace(freeze(m_cacheMeta.pixel));
            m_pvMetadata->endGroupPut();
        } catch (std::exception &e) {
            LOG_ERROR("Exception caught in BasePvaPlugin::postData, portMetadata: %s.", e.what());
            m_cacheMeta.time_of_flight.clear();
            m_cacheMeta.pixel.clear();
        }
        m_pvMetadata->unlock();

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
        // Only lower 20 bits of TOF are relevant
        m_cacheMeta.time_of_flight.push_back(events->tof & 0x000FFFFF);
        m_cacheMeta.pixel.push_back(events->pixelid);
        events++;
    }
}

void BasePvaPlugin::processTofPixelData(const uint32_t *data, uint32_t count)
{
    uint32_t nEvents = count / (sizeof(DasPacket::Event) / sizeof(uint32_t));
    const DasPacket::Event *events = reinterpret_cast<const DasPacket::Event *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cacheTofPixel.time_of_flight.push_back(events->tof);
        m_cacheTofPixel.pixel.push_back(events->pixelid);
        events++;
    }
}

void BasePvaPlugin::postTofPixelData(const PvaNeutronData::shared_pointer& pvRecord)
{
    m_pvNeutrons->time_of_flight->replace(freeze(m_cacheTofPixel.time_of_flight));
    m_pvNeutrons->pixel->replace(freeze(m_cacheTofPixel.pixel));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cacheTofPixel.time_of_flight.reserve(CACHE_SIZE);
    m_cacheTofPixel.pixel.reserve(CACHE_SIZE);
}

void BasePvaPlugin::flushData()
{
    epics::pvData::PVUIntArray::svector emptyIntArray;
    epics::pvData::PVFloatArray::svector emptyFloatArray;
    epics::pvData::PVUShortArray::svector emptyUShortArray;
    epics::pvData::TimeStamp time;

    if (m_pvNeutrons) {

        m_pvNeutrons->lock();

        // Keep time, update user tag
        m_pvNeutrons->timeStamp.get(time);
        time.setUserTag(m_neutronsPostSeq++);

        try {
            m_pvNeutrons->beginGroupPut();
            m_pvNeutrons->timeStamp.set(time);
            m_pvNeutrons->proton_charge->put(0);
            m_pvNeutrons->time_of_flight->replace(freeze(emptyIntArray));
            m_pvNeutrons->pixel->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_a1->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_a2->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_a8->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_x1->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x2->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x3->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x4->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x5->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x6->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x7->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x8->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x9->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x10->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x11->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x12->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x13->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x14->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x15->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x16->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x17->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x18->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x19->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_x20->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y1->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y2->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y3->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y4->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y5->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y6->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y7->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y8->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y9->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y10->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y11->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y12->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y13->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y14->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y15->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y16->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_y17->replace(freeze(emptyUShortArray));
            m_pvNeutrons->sample_a48->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_b1->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_b8->replace(freeze(emptyIntArray));
            m_pvNeutrons->sample_b12->replace(freeze(emptyIntArray));
            m_pvNeutrons->position_index->replace(freeze(emptyIntArray));
            m_pvNeutrons->position_x->replace(freeze(emptyFloatArray));
            m_pvNeutrons->position_y->replace(freeze(emptyFloatArray));
            m_pvNeutrons->photo_sum_x->replace(freeze(emptyFloatArray));
            m_pvNeutrons->photo_sum_y->replace(freeze(emptyFloatArray));
            m_pvNeutrons->endGroupPut();
        } catch (std::exception &e) {
            LOG_ERROR("Exception caught in BasePvaPlugin::postData, postNeutrons: %s.", e.what());
        }
        m_pvNeutrons->unlock();
    }

    if (m_pvMetadata) {
        m_pvMetadata->lock();

        // Keep time, update user tag
        m_pvMetadata->timeStamp.get(time);
        time.setUserTag(m_metadataPostSeq++);

        try {
            m_pvMetadata->timeStamp.set(time);
            m_pvMetadata->proton_charge->put(0);
            m_pvMetadata->time_of_flight->replace(freeze(emptyIntArray));
            m_pvMetadata->pixel->replace(freeze(emptyIntArray));
            m_pvMetadata->endGroupPut();
        } catch (std::exception &e) {
            LOG_ERROR("Exception caught in BasePvaPlugin::postData, postNeutrons: %s.", e.what());
        }
        m_pvMetadata->unlock();
    }
}

void BasePvaPlugin::setCallbacks(ProcessDataCb procCb, PostDataCb postCb)
{
    m_processNeutronsCb = procCb;
    m_postNeutronsCb = postCb;
}
