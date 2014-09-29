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
        ->add("timeStamp", standardField->timeStamp())
        ->add("tof", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("pixel", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sampleA", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sampleB", standardField->scalarArray(epics::pvData::pvUInt, ""))
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
    beginGroupPut();
}

void PvaNeutronData::endGroupPut()
{
    endGroupPut();
    unlock();
}

bool PvaNeutronData::init()
{
    initPVRecord();

    if (!timeStamp.attach(getPVStructure()->getSubField("timeStamp")))
        return false;

    tof = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("tof.value");
    if (tof.get() == NULL)
        return false;

    pixel = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
    if (pixel.get() == NULL)
        return false;

    sampleA = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sampleA.value");
    if (sampleA.get() == NULL)
        return false;

    sampleB = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sampleB.value");
    if (sampleB.get() == NULL)
        return false;

    return true;
}
