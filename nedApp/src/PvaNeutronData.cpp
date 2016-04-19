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
        ->add("sample_g1",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g2",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g3",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g4",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g5",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g6",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g7",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g8",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g9",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g10",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g11",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g12",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g13",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g14",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g15",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g16",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g17",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g18",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g19",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_g20",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x1",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x2",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x3",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x4",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x5",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x6",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x7",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x8",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x9",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x10",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x11",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x12",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x13",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x14",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x15",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x16",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x17",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x18",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x19",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_x20",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y1",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y2",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y3",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y4",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y5",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y6",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y7",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y8",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y9",      standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y10",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y11",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y12",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y13",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y14",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y15",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y16",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_y17",     standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("sample_a48",     standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_b1",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_b8",      standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("sample_b12",     standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("position_index", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("position_x",     standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("position_y",     standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("photo_sum_x",    standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("photo_sum_y",    standardField->scalarArray(epics::pvData::pvFloat, ""))
        ->add("time_range1",    standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("time_range2",    standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("time_range3",    standardField->scalarArray(epics::pvData::pvUShort, ""))
        ->add("time_range4",    standardField->scalarArray(epics::pvData::pvUShort, ""))
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

    proton_charge = getPVStructure()->getSubField<epics::pvData::PVDouble>("proton_charge.value");
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

    sample_g1 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g1.value");
    if (sample_g1.get() == NULL)
        return false;

    sample_g2 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g2.value");
    if (sample_g2.get() == NULL)
        return false;

    sample_g3 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g3.value");
    if (sample_g3.get() == NULL)
        return false;

    sample_g4 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g4.value");
    if (sample_g4.get() == NULL)
        return false;

    sample_g5 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g5.value");
    if (sample_g5.get() == NULL)
        return false;

    sample_g6 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g6.value");
    if (sample_g6.get() == NULL)
        return false;

    sample_g7 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g7.value");
    if (sample_g7.get() == NULL)
        return false;

    sample_g8 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g8.value");
    if (sample_g8.get() == NULL)
        return false;

    sample_g9 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g9.value");
    if (sample_g9.get() == NULL)
        return false;

    sample_g10 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g10.value");
    if (sample_g10.get() == NULL)
        return false;

    sample_g11 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g11.value");
    if (sample_g11.get() == NULL)
        return false;

    sample_g12 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g12.value");
    if (sample_g12.get() == NULL)
        return false;

    sample_g13 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g13.value");
    if (sample_g13.get() == NULL)
        return false;

    sample_g14 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g14.value");
    if (sample_g14.get() == NULL)
        return false;

    sample_g15 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g15.value");
    if (sample_g15.get() == NULL)
        return false;

    sample_g16 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g16.value");
    if (sample_g16.get() == NULL)
        return false;

    sample_g17 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g17.value");
    if (sample_g17.get() == NULL)
        return false;

    sample_g18 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g18.value");
    if (sample_g18.get() == NULL)
        return false;

    sample_g19 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g19.value");
    if (sample_g19.get() == NULL)
        return false;

    sample_g20 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_g20.value");
    if (sample_g20.get() == NULL)
        return false;

    sample_x1 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x1.value");
    if (sample_x1.get() == NULL)
        return false;

    sample_x2 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x2.value");
    if (sample_x2.get() == NULL)
        return false;

    sample_x3 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x3.value");
    if (sample_x3.get() == NULL)
        return false;

    sample_x4 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x4.value");
    if (sample_x4.get() == NULL)
        return false;

    sample_x5 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x5.value");
    if (sample_x5.get() == NULL)
        return false;

    sample_x6 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x6.value");
    if (sample_x6.get() == NULL)
        return false;

    sample_x7 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x7.value");
    if (sample_x7.get() == NULL)
        return false;

    sample_x8 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x8.value");
    if (sample_x8.get() == NULL)
        return false;

    sample_x9 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x9.value");
    if (sample_x9.get() == NULL)
        return false;

    sample_x10 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x10.value");
    if (sample_x10.get() == NULL)
        return false;

    sample_x11 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x11.value");
    if (sample_x11.get() == NULL)
        return false;

    sample_x12 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x12.value");
    if (sample_x12.get() == NULL)
        return false;

    sample_x13 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x13.value");
    if (sample_x13.get() == NULL)
        return false;

    sample_x14 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x14.value");
    if (sample_x14.get() == NULL)
        return false;

    sample_x15 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x15.value");
    if (sample_x15.get() == NULL)
        return false;

    sample_x16 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x16.value");
    if (sample_x16.get() == NULL)
        return false;

    sample_x17 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x17.value");
    if (sample_x17.get() == NULL)
        return false;

    sample_x18 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x18.value");
    if (sample_x18.get() == NULL)
        return false;

    sample_x19 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x19.value");
    if (sample_x19.get() == NULL)
        return false;

    sample_x20 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x20.value");
    if (sample_x20.get() == NULL)
        return false;

    sample_y1 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y1.value");
    if (sample_y1.get() == NULL)
        return false;

    sample_y2 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y2.value");
    if (sample_y2.get() == NULL)
        return false;

    sample_y3 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y3.value");
    if (sample_y3.get() == NULL)
        return false;

    sample_y4 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y4.value");
    if (sample_y4.get() == NULL)
        return false;

    sample_y5 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y5.value");
    if (sample_y5.get() == NULL)
        return false;

    sample_y6 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y6.value");
    if (sample_y6.get() == NULL)
        return false;

    sample_y7 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y7.value");
    if (sample_y7.get() == NULL)
        return false;

    sample_y8 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y8.value");
    if (sample_y8.get() == NULL)
        return false;

    sample_y9 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y9.value");
    if (sample_y9.get() == NULL)
        return false;

    sample_y10 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y10.value");
    if (sample_y10.get() == NULL)
        return false;

    sample_y11 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y11.value");
    if (sample_y11.get() == NULL)
        return false;

    sample_y12 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y12.value");
    if (sample_y12.get() == NULL)
        return false;

    sample_y13 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y13.value");
    if (sample_y13.get() == NULL)
        return false;

    sample_y14 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y14.value");
    if (sample_y14.get() == NULL)
        return false;

    sample_y15 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y15.value");
    if (sample_y15.get() == NULL)
        return false;

    sample_y16 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y16.value");
    if (sample_y16.get() == NULL)
        return false;

    sample_y17 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y17.value");
    if (sample_y17.get() == NULL)
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

    position_x = getPVStructure()->getSubField<epics::pvData::PVFloatArray>("position_x.value");
    if (position_x.get() == NULL)
        return false;

    position_y = getPVStructure()->getSubField<epics::pvData::PVFloatArray>("position_y.value");
    if (position_y.get() == NULL)
        return false;

    photo_sum_x = getPVStructure()->getSubField<epics::pvData::PVFloatArray>("photo_sum_x.value");
    if (photo_sum_x.get() == NULL)
        return false;

    photo_sum_y = getPVStructure()->getSubField<epics::pvData::PVFloatArray>("photo_sum_y.value");
    if (photo_sum_y.get() == NULL)
        return false;

    time_range1 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range1.value");
    if (time_range1.get() == NULL)
        return false;

    time_range2 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range2.value");
    if (time_range2.get() == NULL)
        return false;

    time_range3 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range3.value");
    if (time_range3.get() == NULL)
        return false;

    time_range4 = getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range4.value");
    if (time_range4.get() == NULL)
        return false;

    return true;
}
