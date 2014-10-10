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

BasePvaPlugin::BasePvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvName)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, 0, NUM_BASEPVAPLUGIN_PARAMS)
{
    m_pvRecord = PvaNeutronData::create(pvName);
    if (!m_pvRecord)
        LOG_ERROR("Cannot create PVA record '%s'", pvName);
    else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pvRecord) == false)
        LOG_ERROR("Cannot register PVA record '%s'", pvName);
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
    if (!!m_pvRecord) {
        switch (getDataMode()) {
        case DATA_MODE_NORMAL:
            processDataNormal(packetList);
            break;
        case DATA_MODE_RAW:
            processDataRaw(packetList);
            break;
        case DATA_MODE_EXTENDED:
            processDataExtended(packetList);
            break;
        default:
            break;
        }
    }
}
