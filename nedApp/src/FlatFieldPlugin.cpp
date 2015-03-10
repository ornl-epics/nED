/* FlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FlatFieldPlugin.h"
#include "Log.h"

#include <fstream>
#include <limits>
#include <string>

#define NUM_FLATFIELDPLUGIN_PARAMS ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1))

#define PIXID_ERR       (1 << 31)

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 4, "Port name", string, "Dispatcher port name", string, "Correction tables file", string, "Buffer size", int);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importFilePath, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_FLATFIELDPLUGIN_PARAMS, asynOctetMask | asynFloat64Mask, asynOctetMask | asynFloat64Mask)
{
    ImportError err = MAP_ERR_NONE;

    if (bufSize > 0) {
        m_bufferSize = bufSize;
    } else {
        m_bufferSize = 4*1024*1024;
        LOG_ERROR("Invalid buffer size requested, setting default %u bytes", m_bufferSize);
    }
    m_buffer = reinterpret_cast<uint8_t *>(malloc(m_bufferSize));
    if (m_buffer == 0) {
        m_bufferSize = 0;
        err = MAP_ERR_NO_MEM;
    }

    if (err == MAP_ERR_NONE)
        err = importFile(importFilePath);

    createParam("FilePath",     asynParamOctet, &FilePath, importFilePath); // Path to correction tables file
    createParam("ErrImport",    asynParamInt32, &ErrImport, err); // Last mapping import error
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels, probably due to missing correction tables
    createParam("CntError",     asynParamInt32, &CntError,  0);   // Number of unknown-error pixels
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);   // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    createParam("MapMode",      asynParamInt32, &MapMode, MAP_DISABLED); // Event map mode (see FlatFieldPlugin::MapMode_t)
    createParam("MaxRawX",      asynParamFloat64, &MaxRawX, 158.0);  // Maximum value for X returned by camera
    createParam("MaxRawY",      asynParamFloat64, &MaxRawY, 158.0);  // Maximum value for X returned by camera
    callParamCallbacks();
}

FlatFieldPlugin::~FlatFieldPlugin()
{
    if (m_buffer != 0)
        free(m_buffer);
}

void FlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    TransformErrors errors;
    int nSplits = 0;
    int mapMode = MAP_DISABLED;

    if (m_buffer == 0) {
        LOG_ERROR("Flat field correction disabled due to import error");
        return;
    }

    this->lock();
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(CntUnmap,   &errors.nUnmapped);
    getIntegerParam(CntError,   &errors.nPrevious);
    getIntegerParam(CntSplit,   &nSplits);
    getIntegerParam(MapMode,    &mapMode);
    if (getDataMode() != BasePlugin::DATA_MODE_NORMAL)
        mapMode = MAP_DISABLED;
    // This is a trick to avoid locking. Since plugin design ensures a single
    // instance of processDataUnlocked() function at any time, we read in
    // values and store them as class members. This makes member variables
    // const for the duration of this function.
    getDoubleParam(MaxRawX,     &m_maxRawX);
    getDoubleParam(MaxRawY,     &m_maxRawY);
    this->unlock();

    if (mapMode == MAP_DISABLED) {
        // TODO: sendToPlugins should not call waitAllReleased() automatically.
        //       Neither should we. All we have to do is send packetlist
        //       to clients and return. Reference counters will be okay.
        m_packetList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&m_packetList);
        m_packetList.release();
        m_packetList.waitAllReleased();
        m_packetList.clear();
        nProcessed += packetList->size();
    } else {
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;

            m_packetList.clear(); // Doesn't resize the vector, just tosses elements
            m_packetList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < packet->length()) {
                    nSplits++;
                    break;
                }

                nProcessed++;

                // Reuse the original packet if nothing to map
                if (packet->isNeutronData() == false) {
                    m_packetList.push_back(packet);
                    continue;
                }

                // Reserve part of buffer for this packet, it may shrink from original but never grow
                DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                m_packetList.push_back(newPacket);
                bufferOffset += packet->length();

                // Do the X,Y -> pixid transformation - this could be parallelized
                errors += transformPacket(packet, newPacket);
            }

            sendToPlugins(&m_packetList);
            m_packetList.release();
            m_packetList.waitAllReleased();
        }
    }

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    callParamCallbacks();
    this->unlock();
}

FlatFieldPlugin::TransformErrors FlatFieldPlugin::transformPacket(const DasPacket *srcPacket, DasPacket *destPacket)
{
    TransformErrors errors;
    uint32_t pixelOffset = 4; // TODO: parameter from AcpcResolution**2

    struct AcpcNormalData {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t position_x;
        uint32_t position_y;
        uint32_t photo_sum_x;
        uint32_t photo_sum_y;
    };

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents, nDestEvents;
    const AcpcNormalData *srcEvent= reinterpret_cast<const AcpcNormalData *>(srcPacket->getData(&nEvents));
    DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(destPacket->getData(&nDestEvents));
    nEvents /= (sizeof(AcpcNormalData) / sizeof(uint32_t));
    nDestEvents = 0;

    while (nEvents-- > 0) {

        // Convert raw values into real ones
        double x = (1.0 * srcEvent->position_x) / (1 << 24);
        double y = (1.0 * srcEvent->position_y) / (1 << 24);
        double photosum_x = (1.0 * srcEvent->photo_sum_x) / (1 << 15);
        double photosum_y = (1.0 * srcEvent->photo_sum_y) / (1 << 15);
        uint32_t position_id = srcEvent->position_index & 0x7fff0000;

        // Find appropriate correction table
        auto corrTable = m_corrTables.find(position_id);
        if (corrTable == m_corrTables.end()) {
            errors.nUnmapped++;
            srcEvent++;
            continue;
        }

        // Skip error pixels
        if (srcEvent->position_index & PIXID_ERR) {
            errors.nPrevious++;
            srcEvent++;
            continue;;
        }

        // TODO: missing photo sum correction and thresholds

        // Form TOF,pixelid tupple
        destEvent->tof = srcEvent->time_of_flight;
        destEvent->pixelid = (position_id * pixelOffset) | xyToPixel(x, y, corrTable->second);

        srcEvent++;
        destEvent++;
        nDestEvents++;
    }

    destPacket->payload_length += nDestEvents * sizeof(DasPacket::Event);

    return errors;
}

uint16_t FlatFieldPlugin::xyToPixel(double x, double y, const FlatFieldPlugin::CorrTablePair_t &tables)
{
    x *= tables.first.size() / m_maxRawX;
    y *= tables.second.size() / m_maxRawY;

    if (x >= 0 && x <= tables.first.size() &&
        y >= 0 && y <= tables.second.size()) {

        int xp = (int)x;
        int yp = (int)y;

        double dx = (xp > 0 && xp < 511) ? x - xp : 0;
        double dy = (yp > 0 && yp < 511) ? y - yp : 0;

        x -= (dx * tables.first[xp+1][yp])  + ((1 - dx) * tables.first[xp][yp]);
        y -= (dy * tables.second[xp][yp+1]) + ((1 - dy) * tables.second[xp][yp]);
    }

    return (tables.first.size() * (int)x) + (int)y;cal
}

bool FlatFieldPlugin::parseHeader(std::ifstream &infile, std::string &key, std::string &value)
{
    std::string line;

    while (std::getline(infile, line)) {
        // Skip leading blanks
        size_t pos = line.find_first_not_of(' ');

        // Eat empty lines
        if (pos == std::string::npos)
            continue;

        line = line.substr(pos);

        // Put line back when not a header line
        if (line.at(0) != '#') {
            infile.seekg(-1 * (line.length() + 1), std::ifstream::cur);
            break;
        }

        // Skip # and blanks
        pos = line.find_first_not_of(" \t#");

        // Empty comment line
        if (pos == std::string::npos)
            continue;

        // Parse comment line, stripped text before ':' is key, value is after ':'
        pos = line.find_first_of(':');
        if (pos == std::string::npos) {
            key = line;
            value = "";
        } else {
            key = line.substr(0, line.find_last_not_of(" \t", pos));
            pos = line.find_first_not_of(" \t:", pos);
            if (pos == std::string::npos) {
                value = "";
            } else {
                value = line.substr(pos);
            }
        }
        return true;
    }

    return false;
}

bool FlatFieldPlugin::parseHeaders(std::ifstream &infile, int &size_x, int &size_y, int &position_id, char &table_dim)
{
    size_x = 0;
    size_y = 0;
    position_id = -1;
    table_dim = ' ';
    int version = 0;

    std::string key, value;

    while (parseHeader(infile, key, value)) {
        if (key == "Format version") {
            if (sscanf(value.c_str(), "%d", &version) != 2) {
                LOG_ERROR("Failed to read file format version");
                return false;
            }
        }
        if (key == "Table size") {
            if (sscanf(value.c_str(), "%dx%d", &size_x, &size_y) != 2) {
                LOG_ERROR("Failed to read table size");
                return false;
            }
        }
        if (key == "Table dimension") {
            if (value == "X" || value == "x")
                table_dim = 'x';
            else if (value == "Y" || value == "y")
                table_dim = 'y';
            else {
                LOG_ERROR("Failed to read table dimension");
                return false;
            }
        }
        if (key == "Position id") {
            if (sscanf(value.c_str(), "%d", &position_id) != 1) {
                LOG_ERROR("Failed to read camera id");
                return false;
            }
        }
    }

    // Make sure the header defines table size
    if (version == 0) {
        LOG_ERROR("Invalid file format, missing version");
        return false;
    }
    if (version != 1) {
        LOG_ERROR("Invalid file format, unsupported format version '%d'", version);
        return false;
    }
    if (size_x == 0 && size_y == 0) {
        LOG_ERROR("Invalid file format, missing table size header");
        return false;
    }
    if (table_dim == ' ') {
        LOG_ERROR("Invalid file format, missing camera id header");
        return false;
    }
    if (position_id == -1) {
        LOG_ERROR("Invalid file format, missing camera id header");
        return false;
    }

    return true;
}

FlatFieldPlugin::ImportError FlatFieldPlugin::importFile(const std::string &path)
{
    // TODO: this function needs to read a file containing multiple tables

    std::ifstream infile(path.c_str());

    // Return empty table on any failure
    if (infile.fail()) {
        LOG_ERROR("Failed to open input file");
        return MAP_ERR_NO_FILE;
    }

    while (!infile.eof()) {
        int size_x, size_y, position_id;
        char table_id;
        CorrTable_t table;

        if (!parseHeaders(infile, size_x, size_y, position_id, table_id)) {
            break;
        }

        // Correction factors should follow as doubles, X rows and Y columns.
        // Table size must match the one defined in header.
        table.reserve(size_x);
        for (int i = 0; i < size_x; i++) {
            table[i].reserve(size_y);
            for (int j = 0; j < size_y; j++) {
                double value;
                infile >> value;
                table[i][j] = value;

                if (infile.eof()) {
                    LOG_ERROR("Premature end of file");
                    return MAP_ERR_PARSE;
                }
            }
        }

        if (table_id == 'x')
            m_corrTables[position_id].first = table;
        else
            m_corrTables[position_id].second = table;
    }

    bool done = infile.eof();
    infile.close();

    return (done ? MAP_ERR_NONE : MAP_ERR_PARSE);
}