/* DiscoverPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DISCOVER_PLUGIN_H
#define DISCOVER_PLUGIN_H

#include "BasePlugin.h"
#include "BaseModulePlugin.h"

#include <map>

/**
 * Plugin to discover detector modules.
 *
 * The plugin will discover all modules and remember them internally. asyn report
 * can be used to print details of all discovered details.
 */
class DiscoverPlugin : public BasePlugin {

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

        std::map<uint32_t, ModuleDesc> m_discovered;    //!< Map of modules discovered, key is module's hardware id
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynOctetMask;

    public:
        /**
         * Constructor for DiscoverPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         */
        DiscoverPlugin(const char *portName, const char *parentPlugins);

    private:
        /**
         * Overloaded function called by asynPortDriver when the PV should change value.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process incoming command packets.
         */
        void recvDownstream(DasCmdPacketList *packetList);

        /**
         * Overloaded method to handle reading the output string.
         */
        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

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
        uint32_t formatOutput(char *buffer, uint32_t size);

        /**
         * Print discovered modules in substitution format
         */
        uint32_t formatSubstitution(char *buffer, uint32_t size);

    private:
        int Trigger;            //!< Trigger discovery of modules
        int Output;
        int Format;             //!< Output format
        int Discovered;         //!< Number of discovered modules
        int Verified;           //!< Number of detectors with version verified
        int OptBcast;           //!< Send optical broadcast packet as part of discover
        int LvdsBcast;          //!< Send LVDS broadcast packet as part of discover
        int LvdsSingle;         //!< Send LVDS single word packet as part of discover
};

#endif // DISCOVER_PLUGIN_H
