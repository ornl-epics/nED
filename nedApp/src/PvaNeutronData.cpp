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
