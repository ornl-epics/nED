/* BasePvaPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASE_PVA_PLUGIN_H
#define BASE_PVA_PLUGIN_H

#include "BasePlugin.h"
#include "PvaNeutronData.h"

/**
 * Abstract plugin for exporting data through EPICSv4 pvaccess
 *
 * The base PVA plugin provides functionality common to all plugins that need
 * to export data through EPICSv4 pvaccess. All detectors share the same EPICSv4
 * PVRecord structure definition. This class creates 1 instance of that PVRecord.
 * BasePvaPlugin class guarantees that the m_pvRecord is created automatically
 * and is always available to the specialized plugin, or the plugin will be
 * disabled from start on regardless the attempt to enable it through PV.
 *
 * The base plugin also ensures a common output mode switching interface for
 * all specialized plugins.
 * Specialization plugins only need to implement one or many OCC data processing
 * functions to extract detector specific data and push it to m_pvRecord object.
 * There's no field in DasPacket that describes the payload. We rely on external
 * party to flag current mode through Mode parameter. Based on that appropriate
 * processData*() function is invoked.
 *
 * Available BasePvaPlugin parameters (in addition to ones from BasePlugin):
 * asyn param    | asyn param type | init val | mode | Description                   |
 * ------------- | --------------- | -------- | ---- | ------------------------------
 */
class BasePvaPlugin : public BasePlugin {
    public:
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvName Name for the EPICSv4 PV serving the data.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName);

        /**
         * Abstract destructor.
         */
        virtual ~BasePvaPlugin() = 0;

        /**
         * Overloaded writeInt32 handler.
         *
         * Intercept Enable parameter to disallow enabling the plugin if
         * pvRecord initialization failed.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function to receive all OCC data.
         *
         * Specialized plugins are advised to use one of more specific
         * functions.
         */
        virtual void processData(const DasPacketList * const packetList);

        /**
         * Process incoming data as normal detector data.
         */
        virtual void processDataNormal(const DasPacketList * const packetList) {};

        /**
         * Process incoming data as raw detector data.
         */
        virtual void processDataRaw(const DasPacketList * const packetList) {};

        /**
         * Process incoming data as extended detector data.
         */
        virtual void processDataExtended(const DasPacketList * const packetList) {};

    protected:
        PvaNeutronData::shared_pointer m_pvRecord;

    private: // asyn parameters
        #define FIRST_BASEPVAPLUGIN_PARAM 0
        #define LAST_BASEPVAPLUGIN_PARAM 0
};

#endif // BASE_PVA_PLUGIN_H
