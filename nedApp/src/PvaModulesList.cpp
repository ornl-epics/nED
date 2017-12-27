/* PvaModulesList.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseModulePlugin.h"
#include "Log.h"
#include "PvaModulesList.h"

EPICS_REGISTER_PLUGIN(PvaModulesList, 2, "Port name", string, "PV name", string);

PvaModulesList::PvaModulesList(const char *portName, const char *pvName)
: BasePlugin(portName)
{
    if (pvName == 0 || strlen(pvName) == 0) {
        LOG_ERROR("Missing PVA record name");
    } else {
        m_record = Record::create(pvName);
        if (!m_record)
            LOG_ERROR("Failed to create PVA record '%s'", pvName);
        else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_record) == false)
            LOG_ERROR("Failed to register PVA record '%s'", pvName);
    }

    createParam("Reload", asynParamInt32, &Reload);
}

asynStatus PvaModulesList::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Reload) {
        std::list<std::string> modules;
        BaseModulePlugin::getModuleNames(modules);
        return m_record->update(modules) ? asynSuccess : asynError;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

PvaModulesList::Record::Record(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
{
}

/**
 * Allocate and initialize PvaModulesList::Record.
 */
PvaModulesList::Record::shared_pointer PvaModulesList::Record::create(const std::string &recordName)
{
    using namespace epics::pvData;

    StandardFieldPtr standardField = getStandardField();
    FieldCreatePtr fieldCreate     = getFieldCreate();
    PVDataCreatePtr pvDataCreate   = getPVDataCreate();

    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()->
            add("modules", standardField->scalarArray(epics::pvData::pvString, ""))->
            createStructure()
    );

    Record::shared_pointer pvRecord(new PvaModulesList::Record(recordName, pvStructure));
    if (pvRecord && !pvRecord->init()) {
        pvRecord.reset();
    }

    return pvRecord;
}

/**
 * Attach all PV structures.
 */
bool PvaModulesList::Record::init()
{
    initPVRecord();

    pvModules = getPVStructure()->getSubField<epics::pvData::PVStringArray>("modules.value");
    if (pvModules.get() == NULL)
        return false;

    return true;
}

/**
 * Publish a single atomic update of the PV, take values from packet.
 */
bool PvaModulesList::Record::update(const std::list<std::string> &modules)
{
    bool updated = true;

    epics::pvData::PVStringArray::svector names;
    for (auto it = modules.begin(); it != modules.end(); it++) {
        names.push_back(*it);
    }

    lock();
    try {
        beginGroupPut();
        pvModules->replace(epics::pvData::freeze(names));
        endGroupPut();
    } catch (...) {
        updated = false;
    }
    unlock();

    return updated;
}
