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
        ->add("proton_charge",  standardField->scalar(epics::pvData::pvDouble, ""))
        ->add("time_of_flight", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("pixel",          standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_a1",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_a2",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_a8",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_a19",     standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_a48",     standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_b1",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_b8",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_b12",     standardField->scalarArray(epics::pvData::pvUInt, ""))
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

bool PvaNeutronData::init()
{
    initPVRecord();

    if (!timeStamp.attach(getPVStructure()->getSubField("timeStamp")))
        return false;

    proton_charge = getPVStructure()->getDoubleField("proton_charge.value");
    if (proton_charge.get() == NULL)
        return false;

    time_of_flight = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
    if (time_of_flight.get() == NULL)
        return false;

    pixel = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
    if (pixel.get() == NULL)
        return false;

    sample_a1 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a1.value");
    if (sample_a1.get() == NULL)
        return false;

    sample_a2 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a2.value");
    if (sample_a2.get() == NULL)
        return false;

    sample_a8 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a8.value");
    if (sample_a8.get() == NULL)
        return false;

    sample_a19 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a19.value");
    if (sample_a19.get() == NULL)
        return false;

    sample_a48 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a48.value");
    if (sample_a48.get() == NULL)
        return false;

    sample_b1 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b1.value");
    if (sample_b1.get() == NULL)
        return false;

    sample_b8 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b8.value");
    if (sample_b8.get() == NULL)
        return false;

    sample_b12 = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b12.value");
    if (sample_b12.get() == NULL)
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
