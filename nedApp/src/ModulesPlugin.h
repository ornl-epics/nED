/* ModulesPlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef MODULES_PLUGIN_H
#define MODULES_PLUGIN_H

#include "BasePlugin.h"
#include "BaseModulePlugin.h"

#include <pv/pvDatabase.h>
#include <pv/standardPVField.h>

#include <map>

/**
 * Plugin to discover detector modules.
 *
 * The plugin will discover all modules and remember them internally. asyn report
 * can be used to print details of all discovered details.
 */
class ModulesPlugin : public BasePlugin {

    private:
        /**
         * Module description.
         */
        struct ModuleDesc {
            DasCmdPacket::ModuleType type;
            uint32_t parent;
            BaseModulePlugin::Version version;
            bool verified;

            ModuleDesc()
                : type(static_cast<DasCmdPacket::ModuleType>(0))
                , parent(0)
                , verified(false)
            {}
        };

        /**
         * Record for a structure with a single string array field.
         */
        class Record : public epics::pvDatabase::PVRecord {
            public:
                POINTER_DEFINITIONS(Record);

                /**
                 * Allocate and initialize ModulesPlugin and store it as g_modulesListRecord.
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

        static const uint32_t BUFFER_SIZE = 100000;
        char *m_bufferTxt;                              //!< Buffer for output discover text
        ModulesPlugin::Record::shared_pointer m_record; //!< PVA record for registered modules list
        std::map<uint32_t, ModuleDesc> m_discovered;    //!< Map of modules discovered, key is module's hardware id
        Timer m_disableTimer;                           //!< Timer to trigger disabling the module after inactivity timeout
        std::string m_parentPlugins;                    //!< Parent plugins to connect to

    public:
        /**
         * Constructor for ModulesPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] listPv defines a name of PVAccess PV to export modules name thru
         */
        ModulesPlugin(const char *portName, const char *parentPlugins, const char *listPv);

    private:
        /**
         * Overloaded function called by asynPortDriver when the PV should change value.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process incoming command packets.
         */
        void recvDownstream(const DasCmdPacketList &packetList);

        /**
         * Overloaded function to print details about discovered devices.
         */
        void report(FILE *fp, int details);

        /**
         * Send out a broadcast DISCOVER command to all modules.
         */
        void reqDiscover(uint32_t moduleId=DasCmdPacket::BROADCAST_ID);

        /**
         * Send a READ_VERSION command to a particular DSP.
         */
        void reqVersion(uint32_t moduleId);

        /**
         * Send a READ_VERSION command to a particular module through LVDS.
         */
        void reqLvdsVersion(uint32_t moduleId);

        /**
         * Print discovered modules in human format
         */
        uint32_t formatTxt(char *buffer, uint32_t size);

    private:
        int Discover;           //!< Trigger discovery of modules
        int TxtDisplay;
        int Discovered;         //!< Number of discovered modules
        int Verified;           //!< Number of detectors with version verified
        int RefreshPvaList;     //!< Refresh list of modules in PVA record
        int DbPath;             //!< Path to detectors substitution file
};

#endif // MODULES_PLUGIN_H
