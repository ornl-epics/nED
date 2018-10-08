/* PixelMapPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Event.h"
#include "PixelMapPlugin.h"
#include "Log.h"

#include <fstream>

EPICS_REGISTER_PLUGIN(PixelMapPlugin, 3, "Port name", string, "Parent plugins", string, "PixelMap file", string);

PixelMapPlugin::PixelMapPlugin(const char *portName, const char *parentPlugins, const char *pixelMapFile)
    : BasePlugin(portName, 0, asynOctetMask, asynOctetMask)
{
    ImportError err = importPixelMapFile(pixelMapFile);

    createParam("FilePath",     asynParamOctet, &FilePath, pixelMapFile); // Path to pixel map file
    createParam("ErrImport",    asynParamInt32, &ErrImport, err); // Last mapping import error
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    createParam("MapEn",        asynParamInt32, &MapEn, 0);       // Toggle pixel mapping
    callParamCallbacks();

    BasePlugin::connect(parentPlugins, MsgDasData);
}

asynStatus PixelMapPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(CntUnmap, 0);
            callParamCallbacks();
        }
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

template <typename T>
uint32_t PixelMapPlugin::eventsMap(T *events, uint32_t nEvents)
{
    uint32_t nUnmapped = 0;

    while (nEvents-- > 0) {
        events->pixelid_raw = events->pixelid;

        if (Event::Pixel::getType(events->pixelid) == Event::Pixel::Type::NEUTRON) {
            if (events->pixelid < m_map.size()) {
                // Gaps in pixel map table resolve to error pixels,
                // no need to care here. Already vetoed pixels
                // are outside map range and are not processed here.
                events->pixelid = m_map[events->pixelid];
            } else if ((events->pixelid & Event::Pixel::VETO_MASK) == 0x0) {
                nUnmapped++;
            } else {
                events->pixelid |= Event::Pixel::VETO_MASK;
            }
        }

        events++;
    }

    return nUnmapped;
}

template <>
uint32_t PixelMapPlugin::eventsMap(Event::Pixel *events, uint32_t nEvents)
{
    uint32_t nUnmapped = 0;

    while (nEvents-- > 0) {
        if (Event::Pixel::getType(events->pixelid) == Event::Pixel::Type::NEUTRON) {
            if (events->pixelid < m_map.size()) {
                // Gaps in pixel map table resolve to error pixels,
                // no need to care here. Already vetoed pixels
                // are outside map range and are not processed here.
                events->pixelid = m_map[events->pixelid];
            } else if ((events->pixelid & Event::Pixel::VETO_MASK) == 0x0) {
                nUnmapped++;
            } else {
                events->pixelid |= Event::Pixel::VETO_MASK;
            }
        }

        events++;
    }

    return nUnmapped;
}

void PixelMapPlugin::recvDownstream(const DasDataPacketList &packets)
{
    bool mapEn = getBooleanParam(MapEn);
    int errors = 0;

    getIntegerParam(CntUnmap, &errors);
    if (m_map.empty())
        mapEn = false;

    // Optimize pass thru mode
    if (mapEn == false) {
        sendDownstream(packets);
    } else {
        DasDataPacketList outPackets;
        std::vector<DasDataPacket *> allocatedPackets;

        for (auto it = packets.cbegin(); it != packets.cend(); it++) {
            const DasDataPacket *srcPacket = *it;
            uint32_t nEvents = srcPacket->getNumEvents();

            DasDataPacket *destPacket = nullptr;
            if (srcPacket->getEventsFormat() == DasDataPacket::EVENT_FMT_PIXEL) {
                uint32_t destPacketLen = sizeof(DasDataPacket) + nEvents*sizeof(Event::Pixel);
                destPacket = m_packetsPool.get(destPacketLen);
                if (!destPacket) {
                    LOG_ERROR("Failed to allocate output packet");
                    continue;
                }
                allocatedPackets.push_back(destPacket);
                destPacket->init(DasDataPacket::EVENT_FMT_PIXEL, srcPacket->getTimeStamp(), nEvents, srcPacket->getEvents<Event::Pixel>());
                errors += eventsMap(destPacket->getEvents<Event::Pixel>(), nEvents);
                destPacket->setEventsMapped(true);
                outPackets.push_back(destPacket);
            } else if (srcPacket->getEventsFormat() == DasDataPacket::EVENT_FMT_BNL_DIAG) {
                uint32_t destPacketLen = sizeof(DasDataPacket) + nEvents*sizeof(Event::BNL::Diag);
                destPacket = m_packetsPool.get(destPacketLen);
                if (!destPacket) {
                    LOG_ERROR("Failed to allocate output packet");
                    continue;
                }
                allocatedPackets.push_back(destPacket);
                destPacket->init(DasDataPacket::EVENT_FMT_BNL_DIAG, srcPacket->getTimeStamp(), nEvents, srcPacket->getEvents<Event::BNL::Diag>());
                errors += eventsMap(destPacket->getEvents<Event::BNL::Diag>(), nEvents);
                destPacket->setEventsMapped(true);
                outPackets.push_back(destPacket);
            } else if (srcPacket->getEventsFormat() == DasDataPacket::EVENT_FMT_ACPC_DIAG) {
                uint32_t destPacketLen = sizeof(DasDataPacket) + nEvents*sizeof(Event::ACPC::Diag);
                destPacket = m_packetsPool.get(destPacketLen);
                if (!destPacket) {
                    LOG_ERROR("Failed to allocate output packet");
                    continue;
                }
                allocatedPackets.push_back(destPacket);
                destPacket->init(DasDataPacket::EVENT_FMT_ACPC_DIAG, srcPacket->getTimeStamp(), nEvents, srcPacket->getEvents<Event::ACPC::Diag>());
                errors += eventsMap(destPacket->getEvents<Event::ACPC::Diag>(), nEvents);
                destPacket->setEventsMapped(true);
                outPackets.push_back(destPacket);
            } else {
                static bool logged = false;
                if (!logged) {
                    logged = true;
                    LOG_WARN("Unsupported data format %d, skipping packet", srcPacket->getEventsFormat());
                }
                outPackets.push_back(srcPacket);
                continue;
            }
        }
        if (!outPackets.empty()) {
            sendDownstream(outPackets);
        }

        for (auto it = allocatedPackets.begin(); it != allocatedPackets.end(); it++) {
            m_packetsPool.put(*it);
        }
    }

    setIntegerParam(CntUnmap, errors);
    callParamCallbacks();
}

PixelMapPlugin::ImportError PixelMapPlugin::importPixelMapFile(const char *filepath)
{
    std::string line;
    std::ifstream file(filepath);
    uint32_t lineno = 0;
    uint32_t raw, mapped, bank;
    char trash[2];

    if (file.good() == false) {
        LOG_ERROR("Failed to open pixel map '%s' file", filepath);
        return MAP_ERR_NO_FILE;
    }

    m_map.clear();

    while (true) {
        size_t pos;
        lineno++;

        getline(file, line);
        if (file.fail())
            break;

        // Truncate comments
        pos = line.find_first_of("#");
        if (pos != std::string::npos)
            line.resize(pos);

        // Skip blank lines
        pos = line.find_first_not_of(" \t");
        if (pos == std::string::npos)
            continue;

        // Read all elements in line, but use only first two
        if (sscanf(line.c_str(), "%u %u %u %1s\n", &raw, &mapped, &bank, trash) != 3) {
            LOG_ERROR("Bad entry in pixel map '%s' file, line %d", filepath, lineno);
            m_map.clear();
            return MAP_ERR_PARSE;
        }

        // Fill gaps with invalid mappings
        for (uint32_t i = m_map.size(); i < raw; i++) {
            m_map.push_back(i | Event::Pixel::VETO_MASK);
        }

        // Insert mapping at proper position
        if (raw == m_map.size())
            m_map.push_back(mapped);
        else if (m_map[raw] == (raw | Event::Pixel::VETO_MASK))
            m_map[raw] = mapped;
        else {
            LOG_ERROR("Duplicate raw pixel id in pixel map '%s' file, line %d", filepath, lineno);
            m_map.clear();
            return MAP_ERR_PARSE;
        }
    }

    // Shrink can be enabled if C++11 is used
    //m_map.shrink_to_fit();

    return MAP_ERR_NONE;
}
