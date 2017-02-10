/* PvaXyPlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "PvaXyPlugin.h"
#include "Log.h"

#define NUM_PVAXYPLUGIN_PARAMS ((int)(&LAST_PVAXYPLUGIN_PARAM - &FIRST_PVAXYPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(PvaXyPlugin, 3, "port name", string, "dispatcher port", string, "PV prefix", string);
const uint32_t PvaXyPlugin::CACHE_SIZE = 32*1024;

PvaXyPlugin::PvaXyPlugin(const char *portName, const char *dispatcherPortName, const char *channelName)
    : BasePlugin(portName, dispatcherPortName, NUM_PVAXYPLUGIN_PARAMS)
    , m_xyDivider(1 << 24)
    , m_photosumDivider(1 << 15)
{
    // Initialize PVA record
    m_pvRecord = createPvRecord(channelName);

    // Initialize cache
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.x.reserve(CACHE_SIZE);
    m_cache.y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);

    // UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth",  asynParamInt32, &XyFractWidth, 24); // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth",  asynParamInt32, &PsFractWidth, 15); // WRITE - Number of fraction bits in PhotoSum data
    callParamCallbacks();
}

PvaXyPlugin::PvaRecord::shared_pointer PvaXyPlugin::createPvRecord(const char *channelName)
{
    // Initialize PVA channel
    epics::pvData::FieldCreatePtr fieldCreate = epics::pvData::getFieldCreate();
    epics::pvData::StandardFieldPtr standardField = epics::pvData::getStandardField();
    epics::pvData::PVDataCreatePtr pvDataCreate = epics::pvData::getPVDataCreate();

    epics::pvData::PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()
        ->add("timeStamp",      standardField->timeStamp())
        ->add("time_of_flight", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("position_index", standardField->scalar(epics::pvData::pvDouble, ""))
        ->add("x",              standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("Y",              standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("photo_sum_x",    standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("photo_sum_y",    standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->createStructure()
    );

    PvaRecord::shared_pointer pvRecord(new PvaRecord(channelName, pvStructure));
    pvRecord->init();
    do {
        if (pvRecord->timeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp"))) {
            LOG_ERROR("Can't create 'timeStamp' field in PV");
            pvRecord.reset();
            break;
        }
        pvRecord->x = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("x.value");
        if (pvRecord->x.get() == NULL) {
            LOG_ERROR("Can't create 'x' field in PV");
            pvRecord.reset();
            break;
        }
        pvRecord->y = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("y.value");
        if (pvRecord->y.get() == NULL) {
            LOG_ERROR("Can't create 'y' field in PV");
            pvRecord.reset();
            break;
        }

        pvRecord->photo_sum_x = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("photo_sum_x.value");
        if (pvRecord->photo_sum_x.get() == NULL) {
            LOG_ERROR("Can't create 'photo_sum_x' field in PV");
            pvRecord.reset();
            break;
        }

        pvRecord->photo_sum_y = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("photo_sum_y.value");
        if (pvRecord->photo_sum_y.get() == NULL) {
            LOG_ERROR("Can't create 'photo_sum_y' field in PV");
            pvRecord.reset();
            break;
        }
    } while (0);

    return pvRecord;
}

asynStatus PvaXyPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == XyFractWidth) {
        m_xyDivider = 1 << value;
        return asynSuccess;
    } else if (pasynUser->reason == PsFractWidth) {
        m_photosumDivider = 1 << value;
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void PvaXyPlugin::processData(const DasPacketList * const packetList)
{
    int nProcessed = 0;
    int nReceived = packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        DasPacket::DataFormat dataFormat = packet->getDataFormat();
        if (dataFormat != DasPacket::DATA_FMT_XY && dataFormat != DasPacket::DATA_FMT_XY_PS)
            continue;

        uint32_t dataLen = 0;
        const uint32_t *data = packet->getData(&dataLen);

        if (dataFormat == DasPacket::DATA_FMT_XY) {
            processXyData(data, dataLen);
            nProcessed++;
        } else if (dataFormat == DasPacket::DATA_FMT_XY_PS) {
            processXyPsData(data, dataLen);
            nProcessed++;
        }
    }

    postData(m_pvRecord);

    // Update parameters
    addIntegerParam(ProcCount,  nProcessed);
    addIntegerParam(RxCount,    nReceived);
    callParamCallbacks();
}

void PvaXyPlugin::processXyPsData(const uint32_t *data, uint32_t count)
{
    struct Event {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t x;
        uint32_t y;
        uint32_t photo_sum_x;
        uint32_t photo_sum_y;
    };

    uint32_t nEvents = count / (sizeof(Event) / sizeof(uint32_t));
    const Event *event = reinterpret_cast<const Event *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(event->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(event->position_index & 0x7FFFFFFF);
        // In UQm.n format
        m_cache.x.push_back((1.0 * event->x) / m_xyDivider);
        m_cache.y.push_back((1.0 * event->y) / m_xyDivider);
        m_cache.photo_sum_x.push_back((1.0 * event->photo_sum_x) / m_photosumDivider);
        m_cache.photo_sum_y.push_back((1.0 * event->photo_sum_y) / m_photosumDivider);
        event++;
    }
}

void PvaXyPlugin::processXyData(const uint32_t *data, uint32_t count)
{
    struct Event {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t x;
        uint32_t y;
    };

    uint32_t nEvents = count / (sizeof(Event) / sizeof(uint32_t));
    const Event *event = reinterpret_cast<const Event *>(data);

    // Go through events and append to cache
    while (nEvents-- > 0) {
        m_cache.time_of_flight.push_back(event->time_of_flight & 0x000FFFFF);
        m_cache.position_index.push_back(event->position_index & 0x7FFFFFFF);
        // In UQm.n format
        m_cache.x.push_back((1.0 * event->x) / m_xyDivider);
        m_cache.y.push_back((1.0 * event->y) / m_xyDivider);
        event++;
    }
}

void PvaXyPlugin::postData(const PvaXyPlugin::PvaRecord::shared_pointer& pvRecord)
{
    pvRecord->time_of_flight->replace(freeze(m_cache.time_of_flight));
    pvRecord->position_index->replace(freeze(m_cache.position_index));
    pvRecord->x->replace(freeze(m_cache.x));
    pvRecord->y->replace(freeze(m_cache.y));
    pvRecord->photo_sum_x->replace(freeze(m_cache.photo_sum_x));
    pvRecord->photo_sum_y->replace(freeze(m_cache.photo_sum_y));

    // Reduce gradual memory reallocation by pre-allocating instead of clear()
    m_cache.time_of_flight.reserve(CACHE_SIZE);
    m_cache.position_index.reserve(CACHE_SIZE);
    m_cache.x.reserve(CACHE_SIZE);
    m_cache.y.reserve(CACHE_SIZE);
    m_cache.photo_sum_x.reserve(CACHE_SIZE);
    m_cache.photo_sum_y.reserve(CACHE_SIZE);
}
