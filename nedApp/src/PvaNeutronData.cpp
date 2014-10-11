/* PvaNeutronData.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "PvaNeutronData.h"

#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

PvaNeutronData::shared_pointer PvaNeutronData::create(const std::string &recordName)
{
    epics::pvData::FieldCreatePtr fieldCreate = epics::pvData::getFieldCreate();
    epics::pvData::StandardFieldPtr standardField = epics::pvData::getStandardField();
    epics::pvData::PVDataCreatePtr pvDataCreate = epics::pvData::getPVDataCreate();

    epics::pvData::PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()
        ->add("timeStamp",      standardField->timeStamp())
        ->add("proton_charge",  epics::pvData::pvDouble)
        ->add("time_of_flight", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("pixel",          standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("position_index", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("position_x",     standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("position_y",     standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("photo_sum_x",    standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("photo_sum_y",    standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->createStructure()
    );

    PvaNeutronData::shared_pointer pvRecord(new PvaNeutronData(recordName, pvStructure));
    if (pvRecord && !pvRecord->init())
        pvRecord.reset();

    return pvRecord;
}

PvaNeutronData::PvaNeutronData(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
{}

void PvaNeutronData::beginGroupPut()
{
    lock();
    epics::pvDatabase::PVRecord::beginGroupPut();
}

void PvaNeutronData::endGroupPut()
{
    epics::pvDatabase::PVRecord::endGroupPut();
    unlock();
}

void PvaNeutronData::postCachedUnlocked()
{
    // TODO: Will EPICSv4 recognize not updated fields and optimize them away?
    epics::pvDatabase::PVRecord::beginGroupPut();
    time_of_flight->replace(freeze(cache.time_of_flight));
    proton_charge->put(cache.proton_charge);
    pixel->replace(freeze(cache.pixel));
    position_index->replace(freeze(cache.position_index));
    position_x->replace(freeze(cache.position_x));
    position_y->replace(freeze(cache.position_y));
    photo_sum_x->replace(freeze(cache.photo_sum_x));
    photo_sum_y->replace(freeze(cache.photo_sum_y));
    epics::pvDatabase::PVRecord::endGroupPut();

    cache.time_of_flight.clear();
    cache.pixel.clear();
    cache.position_index.clear();
    cache.position_x.clear();
    cache.position_y.clear();
    cache.photo_sum_x.clear();
    cache.photo_sum_y.clear();
}

void PvaNeutronData::postCached()
{
    lock();
    postCachedUnlocked();
    unlock();
}

bool PvaNeutronData::init()
{
    initPVRecord();

    if (!timeStamp.attach(getPVStructure()->getSubField("timeStamp")))
        return false;

    proton_charge = getPVStructure()->getSubField<epics::pvData::PVDouble>("proton_charge");
    if (proton_charge.get() == NULL)
        return false;

    time_of_flight = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
    if (time_of_flight.get() == NULL)
        return false;

    pixel = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
    if (pixel.get() == NULL)
        return false;

    position_index = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position_index.value");
    if (position_index.get() == NULL)
        return false;

    position_x = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position_x.value");
    if (position_x.get() == NULL)
        return false;

    position_y = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position_y.value");
    if (position_y.get() == NULL)
        return false;

    photo_sum_x = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("photo_sum_x.value");
    if (photo_sum_x.get() == NULL)
        return false;

    photo_sum_y = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("photo_sum_y.value");
    if (photo_sum_y.get() == NULL)
        return false;

    return true;
}
