/* PvaMetaData.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "PvaMetaData.h"

#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

PvaMetaData::shared_pointer PvaMetaData::create(const std::string &recordName)
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
        ->createStructure()
    );

    PvaMetaData::shared_pointer pvRecord(new PvaMetaData(recordName, pvStructure));
    if (pvRecord && !pvRecord->init())
        pvRecord.reset();

    return pvRecord;
}

PvaMetaData::PvaMetaData(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
{}

bool PvaMetaData::init()
{
    initPVRecord();

    if (!timeStamp.attach(getPVStructure()->getSubField("timeStamp")))
        return false;

    proton_charge = getPVStructure()->getSubField<epics::pvData::PVDouble>("proton_charge.value");
    if (proton_charge.get() == NULL)
        return false;

    time_of_flight = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
    if (time_of_flight.get() == NULL)
        return false;

    pixel = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
    if (pixel.get() == NULL)
        return false;

    return true;
}
