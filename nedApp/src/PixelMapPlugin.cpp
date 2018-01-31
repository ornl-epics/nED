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
#include "likely.h"
#include "Log.h"

#include <fstream>
#include <string>

#define PIXID_ERR       (1 << 31)
#define PIXID_ERR_MAP   PIXID_ERR
//#define PIXID_ERR_MAP   (1 << 27 | PIXID_ERR) // TODO: Uncomment if the error pickels should be additionally flagged by this plugin

EPICS_REGISTER_PLUGIN(PixelMapPlugin, 3, "Port name", string, "Parent plugins", string, "PixelMap file", string);

PixelMapPlugin::PixelMapPlugin(const char *portName, const char *parentPlugins, const char *pixelMapFile)
    : BasePlugin(portName, 0, asynOctetMask, asynOctetMask)
    , m_packetsPool(false)
{
    ImportError err = importPixelMapFile(pixelMapFile);

    createParam("FilePath",     asynParamOctet, &FilePath, pixelMapFile); // Path to pixel map file
    createParam("ErrImport",    asynParamInt32, &ErrImport, err); // Last mapping import error
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels
    createParam("CntError",     asynParamInt32, &CntError,  0);   // Number of unknown-error pixels
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    createParam("MapEn",        asynParamInt32, &MapEn, 0);       // Toggle pixel mapping
    createParam("VetoMode",     asynParamInt32, &VetoMode, 1);    // Toggle discarding veto events
    callParamCallbacks();

    BasePlugin::connect(parentPlugins, MsgDasData);
}

asynStatus PixelMapPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(CntUnmap, 0);
            setIntegerParam(CntError, 0);
            callParamCallbacks();
        }
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void PixelMapPlugin::recvDownstream(const DasDataPacketList &packets)
{
    bool mapEn = getBooleanParam(MapEn);
    bool passVetoes = getBooleanParam(VetoMode);
    PixelMapErrors errors;

    getIntegerParam(CntUnmap,   &errors.nUnmapped);
    getIntegerParam(CntError,   &errors.nErrors);
    if (m_map.empty())
        mapEn = false;

    // Optimize pass thru mode
    if (mapEn == false) {
        sendDownstream(packets);
    } else {
        DasDataPacketList modifiedPackets;
        std::vector<DasDataPacket *> allocatedPackets;

        for (auto it = packets.cbegin(); it != packets.cend(); it++) {
            const DasDataPacket *origPacket = *it;

            if (origPacket->format != DasDataPacket::DATA_FMT_PIXEL) {
                modifiedPackets.push_back(origPacket);
                continue;
            }

            DasDataPacket *newPacket = m_packetsPool.get(origPacket->length);
            if (!newPacket) {
                LOG_ERROR("Failed to allocate output packet");
                continue;
            }
            allocatedPackets.push_back(newPacket);

            errors += packetMap(origPacket, newPacket, passVetoes);
            modifiedPackets.push_back(newPacket);
        }
        if (!modifiedPackets.empty()) {
            sendDownstream(modifiedPackets);
        }

        for (auto it = allocatedPackets.begin(); it != allocatedPackets.end(); it++) {
            m_packetsPool.put(*it);
        }
    }

    setIntegerParam(CntUnmap,   errors.nUnmapped);
    setIntegerParam(CntError,   errors.nErrors);
    callParamCallbacks();
}

PixelMapPlugin::PixelMapErrors PixelMapPlugin::packetMap(const DasDataPacket *srcPacket, DasDataPacket *destPacket, bool passVetos)
{
    PixelMapErrors errors;
    const Event::Pixel *srcEvent= reinterpret_cast<const Event::Pixel *>(srcPacket->events);
    Event::Pixel *destEvent = reinterpret_cast<Event::Pixel *>(destPacket->events);
    uint32_t nEvents = (srcPacket->length - sizeof(DasDataPacket)) / sizeof(Event::Pixel);
    uint32_t nDestEvents = 0;

    destPacket->init(DasDataPacket::DATA_FMT_PIXEL, srcPacket->timestamp_sec, srcPacket->timestamp_nsec, 0, 0);
    destPacket->mapped = true;

    // The below code was optimized for speed and is not as elegant as could be
    // otherwise. Bitfield, condition rearranging were both tried with worse results.
    while (nEvents-- > 0) {

        if (likely((srcEvent->pixelid & PIXID_ERR) == 0 && srcEvent->pixelid < m_map.size())) {
            // Gaps in pixel map table resolve to error pixels,
            // no need to care here
            destEvent->tof = srcEvent->tof;
            destEvent->pixelid = m_map[srcEvent->pixelid];
            destEvent++;
            nDestEvents++;
        } else if ((srcEvent->pixelid & 0x70000000) != 0) {
            // This must be some fast-metadata events put into neutron packet
            // Let them through as is
            destEvent->tof = srcEvent->tof;
            destEvent->pixelid = srcEvent->pixelid;
            destEvent++;
            nDestEvents++;
        } else if (passVetos == true) {
            destEvent->tof = srcEvent->tof;
            destEvent->pixelid = srcEvent->pixelid;

            if (srcEvent->pixelid & PIXID_ERR) { // Already tagged as error
                errors.nErrors++;
            } else {
                destEvent->pixelid |= PIXID_ERR_MAP;
                errors.nUnmapped++;
            }

            destEvent++;
            nDestEvents++;
        }

        srcEvent++;
    }

    destPacket->length += nDestEvents * sizeof(Event::Pixel);

    return errors;
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
            m_map.push_back(i | PIXID_ERR_MAP);
        }

        // Insert mapping at proper position
        if (raw == m_map.size())
            m_map.push_back(mapped);
        else if (m_map[raw] == (raw | PIXID_ERR_MAP))
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
