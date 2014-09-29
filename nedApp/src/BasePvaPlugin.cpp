/* BasePvaPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BasePvaPlugin.h"
#include "Log.h"

#define NUM_BASEPVAPLUGIN_PARAMS 0 // ((int)(&LAST_BASEPVAPLUGIN_PARAM - &FIRST_BASEPVAPLUGIN_PARAM + 1))

BasePvaPlugin::BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, 0, NUM_BASEPVAPLUGIN_PARAMS)
    , m_pvRecord(PvaNeutronData::create(std::string(pvPrefix) + "Neutrons"))
{
}

BasePvaPlugin::~BasePvaPlugin()
{}

asynStatus BasePvaPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        if (value > 0 && !m_pvRecord) {
            LOG_ERROR("Can't enable plugin, PV record not initialized");
            return asynError;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void BasePvaPlugin::processData(const DasPacketList * const packetList)
{
    switch (getDataMode()) {
    case DATA_MODE_NORMAL:
        processDataNormal(packetList);
        break;
    default:
        break;
    }
}
