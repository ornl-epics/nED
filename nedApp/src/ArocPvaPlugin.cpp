/* ArocPvaPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "ArocPvaPlugin.h"

EPICS_REGISTER_PLUGIN(ArocPvaPlugin, 3, "port name", string, "dispatcher port", string, "pvAccess PV record prefix", string);

#define NUM_AROCPVAPLUGIN_PARAMS 0 //((int)(&LAST_AROCPVAPLUGIN_PARAM - &FIRST_AROCPVAPLUGIN_PARAM + 1))

ArocPvaPlugin::ArocPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix)
    : BasePvaPlugin(portName, dispatcherPortName, pvPrefix)
{
}

void ArocPvaPlugin::processDataNormal(const DasPacketList * const packetList)
{
    struct OccNormalData {
        uint32_t tof;
        uint32_t position_index;
        uint32_t calculated1[8];
        uint32_t calculated2[8];
    };

    epics::pvData::TimeStamp timeStamp;

    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        if (packet->isNeutronData() == false)
            continue;

        const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
        if (rtdl) {
            timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
            m_pvRecord->timeStamp.set(timeStamp);
        }

        uint32_t dataLength;
        const struct OccNormalData *data = reinterpret_cast<const struct OccNormalData*>(packet->getEventData(&dataLength));

        if ((dataLength % sizeof(struct OccNormalData)) != 0)
            continue;

        for (uint32_t i = 0; i < dataLength; i += sizeof(struct OccNormalData)) {
            //m_pvRecord->tof.put() or m_pvRecord.tof.replace()
            data++;
        }
    }
}

void ArocPvaPlugin::processDataRaw(const DasPacketList * const packetList)
{
    struct OccRawData {
        uint32_t tof;
        uint32_t position_index;
        uint32_t sample1[8];
        uint32_t sample2[8];
    };

    epics::pvData::TimeStamp timeStamp;

    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        if (packet->isNeutronData() == false)
            continue;

        const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
        if (rtdl) {
            timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        }

        uint32_t dataLength;
        const struct OccRawData *data = reinterpret_cast<const struct OccRawData*>(packet->getEventData(&dataLength));

        if ((dataLength % sizeof(struct OccRawData)) != 0)
            continue;

        for (uint32_t i = 0; i < dataLength; i += sizeof(struct OccRawData)) {
            data++;
        }
    }
}

void ArocPvaPlugin::processDataExtended(const DasPacketList * const packetList)
{
    struct OccExtendedData {
        uint32_t tof;
        uint32_t position_index;
        uint32_t sample1[8];
        uint32_t sample2[8];
        uint32_t calculated1[8];
        uint32_t calculated2[8];
    };

    epics::pvData::TimeStamp timeStamp;

    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        if (packet->isNeutronData() == false)
            continue;

        const DasPacket::RtdlHeader *rtdl = packet->getRtdlHeader();
        if (rtdl) {
            timeStamp.put(rtdl->timestamp_sec, rtdl->timestamp_nsec);
        }

        uint32_t dataLength;
        const struct OccExtendedData *data = reinterpret_cast<const struct OccExtendedData*>(packet->getEventData(&dataLength));

        if ((dataLength % sizeof(struct OccExtendedData)) != 0)
            continue;

        for (uint32_t i = 0; i < dataLength; i += sizeof(struct OccExtendedData)) {
            data++;
        }
    }
}
