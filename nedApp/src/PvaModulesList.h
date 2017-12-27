/* PvaModulesList.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_MODULES_LIST_H
#define PVA_MODULES_LIST_H

#include "BasePlugin.h"

#include <vector>

#include <pv/pvDatabase.h>
#include <pv/standardPVField.h>

/**
 * PVAccess PV record.
 */
class PvaModulesList : public BasePlugin {
    public:
        /**
         * C'tor
         */
        PvaModulesList(const char *portName, const char *pvName);

        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    private:
        /**
         * Record for a structure with a single string array field.
         */
        class Record : public epics::pvDatabase::PVRecord {
            public:
                POINTER_DEFINITIONS(Record);

                /**
                 * Allocate and initialize PvaModulesList and store it as g_modulesListRecord.
                 */
                static Record::shared_pointer create(const std::string &recordName);

                /**
                 * Push new list of modules to the PV.
                 */
                bool update(const std::list<std::string> &modules);

            private:
                epics::pvData::PVStringArrayPtr pvModules;      //<! List of registered modules
                std::vector<std::string> m_modules;

                /**
                 * C'tor.
                 */
                Record(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure);

                /**
                 * Attach all PV structures.
                 */
                bool init();
        };

        PvaModulesList::Record::shared_pointer m_record;

    private: // asyn Parameters
        int Reload;
};


#endif // PVA_MODULES_LIST_H
