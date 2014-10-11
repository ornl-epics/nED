/* PixelMapPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "PixelMapPlugin.h"
#include "likely.h"
#include "Log.h"

#include <climits>
#include <cstring>

#define NUM_PIXELMAPPLUGIN_PARAMS ((int)(&LAST_PIXELMAPPLUGIN_PARAM - &FIRST_PIXELMAPPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(PixelMapPlugin, 5, "Port name", string, "Dispatcher port name", string, "Blocking", int, "PixelMap file", string, "Buffer size", int);

PixelMapPlugin::PixelMapPlugin(const char *portName, const char *dispatcherPortName, int blocking, const char *pixelMapFile, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, blocking, NUM_PIXELMAPPLUGIN_PARAMS)
{
    MapError err = MAP_ERR_NONE;

    if (bufSize > 0) {
        m_bufferSize = bufSize;
        m_buffer = reinterpret_cast<uint8_t *>(malloc(m_bufferSize));
        if (m_buffer == 0) {
            m_bufferSize = 0;
            err = MAP_ERR_NO_MEM;
        }
    }

    if (err == MAP_ERR_NONE)
        err = importPixelMapFile(pixelMapFile);

    createParam("MapErr",       asynParamInt32, &MapErr,        err);
    createParam("PassThru",     asynParamInt32, &PassThru,      0);
    createParam("UnmapCount",   asynParamInt32, &UnmapCount,    0);
    createParam("SplitCount",   asynParamInt32, &SplitCount,    0);
    callParamCallbacks();
}

PixelMapPlugin::~PixelMapPlugin()
{
    if (m_buffer != 0)
        free(m_buffer);
}

void PixelMapPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nUnmapped = 0;
    int passthru = 0;
    int nSplits = 0;

    this->lock();
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(UnmapCount, &nUnmapped);
    getIntegerParam(SplitCount, &nSplits);
    getIntegerParam(PassThru,   &passthru);
    if (getDataMode() != BasePlugin::DATA_MODE_NORMAL || m_map.empty())
        passthru = 1;
    this->unlock();

    DasPacketList newPacketList;

    if (passthru != 0) {
        newPacketList.reset(packetList);
        for (const DasPacket *packet = newPacketList.first(); packet != 0; packet = newPacketList.next(packet)) {
            nReceived++;
        }
        sendToPlugins(&newPacketList);
        newPacketList.release();
        newPacketList.waitAllReleased();
    } else {

        uint32_t bufferOffset = 0;

        for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
            nReceived++;

            uint32_t remain = m_bufferSize - bufferOffset;
            if (remain < packet->length()) {
                newPacketList.reset(m_buffer, bufferOffset);
                sendToPlugins(&newPacketList);
                newPacketList.release();
                newPacketList.waitAllReleased();
                bufferOffset = 0;
                nSplits++;
            }

            // Reserve part of buffer for this packet, it may shrink from original but never grow
            DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
            bufferOffset += packet->length();

            if (packet->isNeutronData() == true) {
                // Do the pixel id mapping - this can be parallelized
                nUnmapped += packetMap(packet, newPacket);
            } else {
                packet->copy(newPacket, packet->length());
            }

            nProcessed++;
        }

        if (bufferOffset > 0) {
            DasPacketList newPacketList;
            newPacketList.reset(m_buffer, bufferOffset);
            sendToPlugins(&newPacketList);
            newPacketList.release();
            newPacketList.waitAllReleased();
        }
    }

    this->lock();
    setIntegerParam(RxCount,    nReceived % INT_MAX);
    setIntegerParam(ProcCount,  nProcessed % INT_MAX);
    setIntegerParam(UnmapCount, nUnmapped % INT_MAX);
    setIntegerParam(SplitCount, nSplits % INT_MAX);
    callParamCallbacks();
    this->unlock();
}

uint32_t PixelMapPlugin::packetMap(const DasPacket *srcPacket, DasPacket *destPacket)
{
    uint32_t nUnmapped = 0;

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents, nDestEvents;
    DasPacket::Event *srcEvent= const_cast<DasPacket::Event *>(srcPacket->getEventData(&nEvents));
    DasPacket::Event *destEvent= const_cast<DasPacket::Event *>(destPacket->getEventData(&nDestEvents));

    while (nEvents-- > 0) {
        if (likely(srcEvent->pixelid < m_map.size())) {
            destEvent->tof = srcEvent->tof;
            destEvent->pixelid = m_map[srcEvent->pixelid];
            destPacket->payload_length += sizeof(DasPacket::Event);
            srcEvent++;
            destEvent++;
        } else {
            nUnmapped++;
            srcEvent++;
        }
    }

    return nUnmapped;
}

PixelMapPlugin::MapError PixelMapPlugin::importPixelMapFile(const char *filepath)
{
    char line[128];
    FILE *fp;
    uint32_t rawPixelId, mapPixelId, bankId;

    m_map.clear();

    fp = fopen(filepath, "r");
    if (fp == 0) {
        LOG_ERROR("Failed to open pixel map '%s' file", filepath);
        return MAP_ERR_NO_FILE;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%u %u %u", &rawPixelId, &mapPixelId, &bankId) == 3) {
            if (m_map.size() != rawPixelId) {
                LOG_ERROR("Cannot parse pixel map file - raw pixel id gap found");
                m_map.clear();
                break;
            }
            m_map.push_back(mapPixelId);
        }
    }
    fclose(fp);

    if (m_map.size() == 0)
        return MAP_ERR_PARSE;

    return MAP_ERR_NONE;
}
