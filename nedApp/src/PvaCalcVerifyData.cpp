/* PvaCalcVerifyData.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "PvaCalcVerifyData.h"

#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

PvaCalcVerifyData::shared_pointer PvaCalcVerifyData::create(const std::string &recordName, uint32_t cacheSize)
{
    epics::pvData::FieldCreatePtr fieldCreate = epics::pvData::getFieldCreate();
    epics::pvData::StandardFieldPtr standardField = epics::pvData::getStandardField();
    epics::pvData::PVDataCreatePtr pvDataCreate = epics::pvData::getPVDataCreate();

    epics::pvData::PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()
        ->add("time_of_flight", standardField->scalarArray(epics::pvData::pvUInt, ""))
        ->add("firmware_x",     standardField->scalarArray(epics::pvData::pvDouble, ""))
        ->add("firmware_y",     standardField->scalarArray(epics::pvData::pvDouble, ""))
        ->add("software_x",     standardField->scalarArray(epics::pvData::pvDouble, ""))
        ->add("software_y",     standardField->scalarArray(epics::pvData::pvDouble, ""))
        ->createStructure()
    );

    PvaCalcVerifyData::shared_pointer pvRecord(new PvaCalcVerifyData(recordName, pvStructure, cacheSize));
    if (pvRecord && !pvRecord->init()) {
        pvRecord.reset();
    } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(pvRecord)) {
        pvRecord.reset();
    }

    return pvRecord;
}

bool PvaCalcVerifyData::destroy(PvaCalcVerifyData::shared_pointer &record)
{
    if (record) {
        if (!epics::pvDatabase::PVDatabase::getMaster()->removeRecord(record))
            return false;
        record.reset();
    }
    return true;
}

PvaCalcVerifyData::PvaCalcVerifyData(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure, uint32_t cacheSize)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
    , m_cacheSize(cacheSize)
{}

bool PvaCalcVerifyData::post()
{
    bool ret = false;
    if (time_of_flight.empty() == false) {
        lock();
        try {
            beginGroupPut();
            time_of_flightPtr->replace(freeze(time_of_flight));
            firmware_xPtr->replace(freeze(firmware_x));
            firmware_yPtr->replace(freeze(firmware_y));
            software_xPtr->replace(freeze(software_x));
            software_yPtr->replace(freeze(software_y));
            endGroupPut();

            // Reduce gradual memory reallocation by pre-allocating instead of clear()
            time_of_flight.reserve(m_cacheSize);
            firmware_x.reserve(m_cacheSize);
            firmware_y.reserve(m_cacheSize);
            software_x.reserve(m_cacheSize);
            software_y.reserve(m_cacheSize);

            ret = true;
        } catch (std::exception &e) {
        }
        unlock();
    }
    return ret;
}

bool PvaCalcVerifyData::init()
{
    initPVRecord();

    time_of_flightPtr = getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
    if (time_of_flightPtr.get() == NULL)
        return false;

    firmware_xPtr = getPVStructure()->getSubField<epics::pvData::PVDoubleArray>("firmware_x.value");
    if (firmware_xPtr.get() == NULL)
        return false;

    firmware_yPtr = getPVStructure()->getSubField<epics::pvData::PVDoubleArray>("firmware_y.value");
    if (firmware_yPtr.get() == NULL)
        return false;

    software_xPtr = getPVStructure()->getSubField<epics::pvData::PVDoubleArray>("software_x.value");
    if (software_xPtr.get() == NULL)
        return false;

    software_yPtr = getPVStructure()->getSubField<epics::pvData::PVDoubleArray>("software_y.value");
    if (software_yPtr.get() == NULL)
        return false;

    return true;
}
