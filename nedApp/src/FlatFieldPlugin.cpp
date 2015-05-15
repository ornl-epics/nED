/* FlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FlatFieldPlugin.h"
#include "likely.h"
#include "Log.h"

#include <dirent.h>

#include <fstream>
#include <limits>
#include <string>

#define NUM_FLATFIELDPLUGIN_PARAMS ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1))

#define PIXID_ERR       (1 << 31)
#define MAX_PIXEL_SIZE  512

#if defined(WIN32) || defined(_WIN32)
#   define PATH_SEPARATOR "\\"
#else
#   define PATH_SEPARATOR "/"
#endif

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 4, "Port name", string, "Dispatcher port name", string, "Correction tables file", string, "Buffer size", int);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importDir, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_FLATFIELDPLUGIN_PARAMS, asynOctetMask | asynFloat64Mask, asynOctetMask | asynFloat64Mask)
{
    ImportError err = FF_ERR_NONE;

    if (bufSize > 0) {
        m_bufferSize = bufSize;
    } else {
        m_bufferSize = 4*1024*1024;
        LOG_ERROR("Invalid buffer size requested, setting default %u bytes", m_bufferSize);
    }
    m_buffer = reinterpret_cast<uint8_t *>(malloc(m_bufferSize));
    if (m_buffer == 0) {
        m_bufferSize = 0;
        err = FF_ERR_NO_MEM;
    }

    if (err == FF_ERR_NONE)
        err = importFiles(importDir);

    createParam("ImportReport", asynParamOctet, &ImportReport);   // Generate textual file import report
    createParam("ImportDir",    asynParamOctet, &ImportDir, importDir); // Path to correction tables directory
    createParam("ErrImport",    asynParamInt32, &ErrImport, err); // Last mapping import error
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels, probably due to missing correction tables
    createParam("CntError",     asynParamInt32, &CntError,  0);   // Number of unknown-error pixels
    createParam("CntPhotoSum",  asynParamInt32, &CntPhotoSum, 0); // Number of photo sum eliminated pixels
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);   // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    createParam("FfMode",       asynParamInt32, &FfMode, FF_PASS_THRU); // FlatField transformation mode (see FlatFieldPlugin::FfMode_t)
    // Next two params are in UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth",  asynParamInt32, &XyFractWidth, 24); // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth",  asynParamInt32, &PsFractWidth, 15); // WRITE - Number of fraction bits in PhotoSum data
    createParam("XyRange",      asynParamFloat64, &XyRange, 158.0); // WRITE - Maximum X and Y values from detector
    createParam("PsLowDecBase", asynParamFloat64, &PsLowDecBase, 300.0); // WRITE - PhotoSum lower decrement base
    createParam("PsLowDecLim",  asynParamFloat64, &PsLowDecLim, 400.0); // WRITE - PhotoSum lower decrement limit
    callParamCallbacks();
}

FlatFieldPlugin::~FlatFieldPlugin()
{
    if (m_buffer != 0)
        free(m_buffer);
}

asynStatus FlatFieldPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(CntUnmap, 0);
            setIntegerParam(CntError, 0);
            setIntegerParam(CntSplit, 0);
            callParamCallbacks();
        }
        return asynSuccess;
    } else if (pasynUser->reason == XyFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason == PsFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason == FfMode) {
        if (value != FF_PASS_THRU && value != FF_FLATTEN && value != FF_CONVERT_ONLY)
            return asynError;
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

asynStatus FlatFieldPlugin::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    if (pasynUser->reason == XyRange) {
        if (value <= 0.0 || value >= MAX_PIXEL_SIZE)
            return asynError;
    }
    return BaseDispatcherPlugin::writeFloat64(pasynUser, value);
}

asynStatus FlatFieldPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == ImportReport) {

        const char *importErrMap[] = {
            "imported",                     // FF_ERR_NONE
            "no such file",                 // FF_ERR_NO_FILE
            "parse error",                  // FF_ERR_PARSE
            "no memory",                    // FF_ERR_NO_MEM
            "duplicate data for position",  // FF_ERR_EXIST
            "unrecognized file format",     // FF_ERR_BAD_FORMAT
            "table size mismatch",          // FF_ERR_BAD_SIZE
        };

        *nActual = 0;
        for (auto it = m_filesStatus.begin(); it != m_filesStatus.end(); it++) {
            int ret = snprintf(value, nChars, "%-20s - %s\n", it->first.c_str(), importErrMap[it->second]);
            if (ret == -1 || (size_t)ret > nChars)
                break;

            nChars -= ret;
            value += ret;
            *nActual += ret;
        }
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void FlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    TransformErrors errors;
    int nSplits = 0;
    DasPacketList newPacketList;

    // Parametrise calculating algorithms based on detector.
    double xyRange = 0.0;
    int xyFractWidth = 0;
    int psFractWidth = 0;

    if (m_buffer == 0) {
        LOG_ERROR("Flat field correction disabled due to import error");
        return;
    }

    this->lock();
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(CntUnmap,   &errors.nUnmapped);
    getIntegerParam(CntError,   &errors.nErrors);
    getIntegerParam(CntPhotoSum,&errors.nPhotoSum);
    getIntegerParam(CntSplit,   &nSplits);
    getIntegerParam(FfMode,     &m_ffMode);
    if (getDataMode() != BasePlugin::DATA_MODE_NORMAL)
        m_ffMode = FF_PASS_THRU;
    getIntegerParam(XyFractWidth, &xyFractWidth);
    getIntegerParam(PsFractWidth, &psFractWidth);
    getDoubleParam(XyRange,     &xyRange);

    // This is a trick to avoid locking access to member variables. Since
    // plugin design ensures a single instance of processDataUnlocked()
    // function at any time, we read in values and store them as class members.
    // This makes member variables const for the duration of this function.
    m_xyScale = 1.0 * m_tablesSize / ((1 << xyFractWidth) * xyRange);
    m_psScale = 1.0 / (1 << psFractWidth);
    getDoubleParam(PsLowDecBase,&m_psLowDecBase);
    getDoubleParam(PsLowDecLim, &m_psLowDecLim);

    this->unlock();

    if (m_ffMode == FF_PASS_THRU) {
        sendToPlugins(packetList);
        nProcessed += packetList->size();
    } else {
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;

            newPacketList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < packet->length()) {
                    nSplits++;
                    break;
                }

                // Reuse the original packet if nothing to map
                if (packet->isNeutronData() == false) {
                    newPacketList.push_back(packet);
                    continue;
                }

                nProcessed++;

                // Reserve part of buffer for this packet, it may shrink from original but never grow
                DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                newPacketList.push_back(newPacket);
                bufferOffset += packet->length();

                // Do the X,Y -> pixid transformation - this could be parallelized
                errors += transformPacket(packet, newPacket);
            }

            sendToPlugins(&newPacketList);
            newPacketList.release();
            newPacketList.waitAllReleased();
        }
    }

    nReceived += packetList->size();

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntUnmap,   errors.nUnmapped);
    setIntegerParam(CntPhotoSum,errors.nPhotoSum);
    setIntegerParam(CntError,   errors.nErrors);
    callParamCallbacks();
    this->unlock();
}

FlatFieldPlugin::TransformErrors FlatFieldPlugin::transformPacket(const DasPacket *srcPacket, DasPacket *destPacket)
{
    TransformErrors errors;

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
        // Skip error pixels
        if (srcEvent->position_index & PIXID_ERR) {
            errors.nErrors++;
            srcEvent++;
            continue;;
        }

        // Convert raw values into ones used by calculations
        uint32_t tof = srcEvent->time_of_flight;
        uint32_t position_id = srcEvent->position_index & 0x3FFF;
        double x = srcEvent->position_x * m_xyScale;
        double y = srcEvent->position_y * m_xyScale;
        double photosum_x = srcEvent->photo_sum_x * m_psScale;
        // unused srcEvent->photo_sum_y
        srcEvent++;

        // Check photo sum
        if (checkPhotoSumLimits(x, y, photosum_x, position_id) == false) {
            errors.nPhotoSum++;
            continue;
        }

        // Form TOF,pixelid tupple
        destEvent->tof = tof;
        destEvent->pixelid  = position_id << 18;
        destEvent->pixelid |= xyToPixel(x, y, position_id);

        if ((destEvent->pixelid & 0x3FFFF) == 0) {
            errors.nUnmapped++;
            continue;
        }

        destEvent++;
        nDestEvents++;
    }

    destPacket->payload_length += nDestEvents * sizeof(DasPacket::Event);

    return errors;
}

std::shared_ptr<FlatFieldPlugin::CorrTable_t> FlatFieldPlugin::findTable(uint32_t position_id, TableType_t type)
{
    if (m_corrTables.size() > position_id && m_corrTables[position_id].size() > (unsigned)type)
        return m_corrTables[position_id][type];

    return std::shared_ptr<CorrTable_t>();
}

uint32_t FlatFieldPlugin::xyToPixel(double x, double y, uint32_t position_id)
{
    if (likely(m_ffMode == FF_FLATTEN)) {
        std::shared_ptr<CorrTable_t> xtable = findTable(position_id, TABLE_X_CORRECTION);
        std::shared_ptr<CorrTable_t> ytable = findTable(position_id, TABLE_Y_CORRECTION);

        if (xtable.get() == 0 || ytable.get() == 0)
            return 0;

        unsigned xp = x;
        unsigned yp = y;

        // Tables are guaranteed to be MAX_PIXEL_SIZE elements in any direction
        if (x < 0.0 || xp >= xtable->size() || y < 0.0 || yp >= ytable->size())
            return 0;

        // All checks passed - do the correction

        double dx = (xp == 0 || xp == (xtable->size()  - 1)) ? 0 : x - xp;
        double dy = (yp == 0 || yp == (ytable->size() - 1)) ? 0 : y - yp;

        x -= (dx * (*xtable)[xp+1][yp]) + ((1 - dx) * (*xtable)[xp][yp]);
        y -= (dy * (*ytable)[xp][yp+1]) + ((1 - dy) * (*ytable)[xp][yp]);
    }

    return (MAX_PIXEL_SIZE * (int)x) + (int)y;
}

bool FlatFieldPlugin::checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position_id)
{
    if (unlikely(m_ffMode != FF_FLATTEN))
        return true;

    std::shared_ptr<CorrTable_t> upperLimits = findTable(position_id, TABLE_X_PHOTOSUM_UPPER);
    std::shared_ptr<CorrTable_t> lowerLimits = findTable(position_id, TABLE_X_PHOTOSUM_LOWER);

    if (upperLimits.get() == 0 || lowerLimits.get() == 0)
        return false;

    unsigned xp = x;
    unsigned yp = y;

    // Tables are guaranteed to be MAX_PIXEL_SIZE elements in any direction
    if (x < 0.0 || xp >= upperLimits->size() || y < 0.0 || yp >= upperLimits->size())
        return 0;

    double upperLimit = (*upperLimits)[xp][yp];
    double lowerLimit = (*lowerLimits)[xp][yp];

    return (lowerLimit <= photosum_x && photosum_x <= upperLimit);
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

        line = line.substr(pos);

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

FlatFieldPlugin::ImportError FlatFieldPlugin::parseHeaders(
    std::ifstream &infile, uint32_t &size_x, uint32_t &size_y, uint32_t &position_id,
    FlatFieldPlugin::TableType_t &type, const std::string &path)
{
    size_x = 0;
    size_y = 0;
    position_id = 0xFFFFFFFF;
    type = TABLE_UNKNOWN;
    int version = 0;
    uint32_t nHeaders = 0;

    std::string key, value;

    while (parseHeader(infile, key, value)) {
        if (key == "Format version") {
            if (sscanf(value.c_str(), "%d", &version) != 1) {
                LOG_ERROR("Failed to read file format version: %s", path.c_str());
                return FF_ERR_PARSE;
            }
            nHeaders++;
        }
        if (key == "Table size") {
            if (sscanf(value.c_str(), "%ux%u", &size_x, &size_y) != 2) {
                LOG_ERROR("Failed to read table size: %s", path.c_str());
                return FF_ERR_PARSE;
            }
            nHeaders++;
        }
        if (key == "Table type") {
            char dim;
            if (sscanf(value.c_str(), "Photo Sum %c lower", &dim) == 1) {
                if (dim == 'x' || dim == 'X') {
                    type = TABLE_X_PHOTOSUM_LOWER;
                } else if (dim == 'y' || dim == 'Y') {
                    LOG_WARN("Y photo sum table not implemented: %s", path.c_str());
                } else {
                    LOG_ERROR("Unknown photo sum dimension: %s", path.c_str());
                    return FF_ERR_PARSE;
                }
            } else if (sscanf(value.c_str(), "Photo Sum %c upper", &dim) == 1) {
                if (dim == 'x' || dim == 'X') {
                    type = TABLE_X_PHOTOSUM_UPPER;
                } else if (dim == 'y' || dim == 'Y') {
                    LOG_WARN("Y photo sum table not implemented: %s", path.c_str());
                } else {
                    LOG_ERROR("Unknown photo sum dimension: %s", path.c_str());
                    return FF_ERR_PARSE;
                }
            } else if (sscanf(value.c_str(), "Correction %c", &dim) == 1) {
                if (dim == 'x' || dim == 'X') {
                    type = TABLE_X_CORRECTION;
                } else if (dim == 'y' || dim == 'Y') {
                    type = TABLE_Y_CORRECTION;
                } else {
                    LOG_ERROR("Failed to parse table type: %s", path.c_str());
                    return FF_ERR_PARSE;
                }
            } else {
                LOG_ERROR("Failed to read table type: %s", path.c_str());
                return FF_ERR_PARSE;
            }
            nHeaders++;
        }
        if (key == "Position id") {
            if (sscanf(value.c_str(), "%u", &position_id) != 1) {
                LOG_ERROR("Failed to read position id: %s", path.c_str());
                return FF_ERR_PARSE;
            } else if (position_id & ~0x7FFFFFFF) {
                LOG_ERROR("Invalid position id %u: %s", position_id, path.c_str());
                return FF_ERR_PARSE;
            }
            nHeaders++;
        }
    }

    if (nHeaders == 0) {
        LOG_WARN("Unknown file format: %s", path.c_str());
        return FF_ERR_BAD_FORMAT;
    }

    // Make sure we got all required headers
    if (version == 0) {
        LOG_ERROR("Invalid file format, missing version: %s", path.c_str());
        return FF_ERR_PARSE;
    }
    if (version != 1) {
        LOG_ERROR("Invalid file format, unsupported format version '%d': %s", version, path.c_str());
        return FF_ERR_PARSE;
    }
    if (size_x == 0 || size_y == 0 || size_x > MAX_PIXEL_SIZE || size_y > MAX_PIXEL_SIZE) {
        LOG_ERROR("Invalid file format, missing table size header: %s", path.c_str());
        return FF_ERR_PARSE;
    }
    if (type == TABLE_UNKNOWN) {
        LOG_ERROR("Invalid file format, missing table type header: %s", path.c_str());
        return FF_ERR_PARSE;
    }
    if (position_id == 0xFFFFFFFF) {
        LOG_ERROR("Invalid file format, missing position id header: %s", path.c_str());
        return FF_ERR_PARSE;
    }

    return FF_ERR_NONE;
}

FlatFieldPlugin::ImportError FlatFieldPlugin::importFile(const std::string &path, uint32_t &tableSize)
{
    ImportError err = FF_ERR_NONE;
    std::ifstream infile(path.c_str());

    if (infile.fail()) {
        LOG_ERROR("Failed to open input file");
        return FF_ERR_NO_FILE;
    }

    while (!infile.eof()) {
        uint32_t size_x, size_y, position_id;
        TableType_t type;
        std::shared_ptr<CorrTable_t> table(new CorrTable_t);

        if (table.get() == 0) {
            err = FF_ERR_NO_MEM;
            break;
        }

        err = parseHeaders(infile, size_x, size_y, position_id, type, path);
        if (err != FF_ERR_NONE)
            break;

        if (tableSize == 0)
            tableSize = size_x;
        if (tableSize == 0 || size_x != tableSize || size_y != tableSize) {
            err = FF_ERR_BAD_SIZE;
            break;
        }

        // Correction factors should follow as doubles, X rows and Y columns.
        // Table size must match the one defined in header.
        table->resize(size_x, std::vector<double>(size_y, 0.0));
        for (uint32_t i = 0; i < size_x; i++) {
            for (uint32_t j = 0; j < size_y; j++) {
                double value;
                infile >> value;
                (*table)[i][j] = value;

                if (infile.eof()) {
                    LOG_ERROR("Premature end of file");
                    err = FF_ERR_PARSE;
                    break;
                }
            }
        }

        // Make sure there's enough place in the m_corrTables
        if (m_corrTables.size() <= position_id)
            m_corrTables.resize(position_id + 1);
        if (m_corrTables[position_id].size() < TABLE_UNKNOWN)
            m_corrTables[position_id].resize(TABLE_UNKNOWN);

        if (m_corrTables[position_id][type].get() != 0) {
            err = FF_ERR_EXIST;
        } else {
            m_corrTables[position_id][type] = table;
        }

        break;
    }

    infile.close();

    return err;
}

FlatFieldPlugin::ImportError FlatFieldPlugin::importFiles(const std::string &path)
{
    struct dirent entry, *result;
    DIR *dir = opendir(path.c_str());
    uint32_t nImported = 0;
    uint32_t nErrors = 0;

    m_tablesSize = 0;

    if (dir == NULL)
        return FF_ERR_NO_FILE;

    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
        std::string filename = entry.d_name;
        if (filename != ".." && filename != ".") {
            ImportError err = importFile(path + PATH_SEPARATOR + filename, m_tablesSize);
            m_filesStatus[filename] = err;

            if (err == FF_ERR_NONE)
                nImported++;
            else if (err != FF_ERR_BAD_FORMAT)
                nErrors++;
        }
    }

    closedir(dir);

    // Check that each position has X,Y and X photosum table at least
    for (auto it = m_corrTables.begin(); it != m_corrTables.end(); it++) {
        if (it->size() > 0) {
            if ((*it)[TABLE_X_CORRECTION].get() == 0 ||
                (*it)[TABLE_Y_CORRECTION].get() == 0 ||
                (*it)[TABLE_X_PHOTOSUM_LOWER].get() == 0 ||
                (*it)[TABLE_X_PHOTOSUM_UPPER].get() == 0) {

                m_corrTables.clear();
                return FF_ERR_INCOMPLETE;
            }
        }
    }

    if (nErrors > 0) {
        m_corrTables.clear();
        return FF_ERR_PARSE;
    }

    if (nImported == 0)
        return FF_ERR_NO_FILE;

    return FF_ERR_NONE;
}
