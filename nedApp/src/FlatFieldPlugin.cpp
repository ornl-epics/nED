/* FlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Bits.h"
#include "Common.h"
#include "Event.h"
#include "FlatFieldPlugin.h"
#include "FlatFieldTable.h"
#include "likely.h"
#include "Log.h"

#include <dirent.h>
#include <cmath>
#include <cstring> // strerror()

#include <fstream>
#include <limits>
#include <string>
#include <sstream>

#if defined(WIN32) || defined(_WIN32)
#   define PATH_SEPARATOR '\\'
#else
#   define PATH_SEPARATOR '/'
#endif

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 3, "Port name", string, "Parent plugins", string, "Positions", string);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *parentPlugins, const char *positions)
    : BasePlugin(portName, 1, asynOctetMask | asynFloat64Mask | asynInt32ArrayMask, asynOctetMask | asynFloat64Mask)
    , m_parentPlugins(parentPlugins)
{
    createParam("ImportReport", asynParamOctet, &ImportReport);         // Generate textual file import report
    createParam("ImportStatus", asynParamInt32, &ImportStatus, IMPORT_STATUS_NONE); // Import status
    createParam("ImportDir",    asynParamOctet, &ImportDir);            // Path to correction tables directory
    createParam("NumPositions", asynParamInt32, &NumPositions, 0);      // READ - Number of configured positions
    createParam("Positions",    asynParamInt32Array, &Positions);       // READ - All configured positions
    createParam("CntGoodEvents",asynParamInt32, &CntGoodEvents, 0);     // Number of calculated events
    createParam("CntInhVetos",  asynParamInt32, &CntInhVetos, 0);       // Number of vetos inherited from others
    createParam("CntPosVetos",  asynParamInt32, &CntPosVetos, 0);       // Number of bad position vetos
    createParam("CntRangeVetos",asynParamInt32, &CntRangeVetos, 0);     // Number of bad X,Y range vetos
    createParam("CntPosCfgVetos",asynParamInt32, &CntPosCfgVetos, 0);   // Number of position configuration vetos
    createParam("CntPsVetos",   asynParamInt32, &CntPsVetos, 0);        // Number of photosum discriminated events
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);             // Reset counters
    // Next two params are in UQm.n format, n is fraction bits, http://en.wikipedia.org/wiki/Q_%28number_format%29
    createParam("XyFractWidth", asynParamInt32, &XyFractWidth, 24);     // WRITE - Number of fraction bits in X,Y data
    createParam("PsFractWidth", asynParamInt32, &PsFractWidth, 15);     // WRITE - Number of fraction bits in PhotoSum data
    createParam("XMaxIn",       asynParamFloat64, &XMaxIn, 158.0);      // WRITE - Defines input X range
    createParam("YMaxIn",       asynParamFloat64, &YMaxIn, 158.0);      // WRITE - Defines input Y range
    createParam("XMaxOut",      asynParamInt32, &XMaxOut, 511);         // WRITE - Converted max X value
    createParam("YMaxOut",      asynParamInt32, &YMaxOut, 511);         // WRITE - Converted max Y value
    createParam("TablesSizeX",  asynParamInt32, &TablesSizeX, 0);       // READ - All tables X size
    createParam("TablesSizeY",  asynParamInt32, &TablesSizeY, 0);       // READ - All tables Y size

    std::vector<std::string> positions_ = Common::split(positions, ',');
    for (auto it=positions_.begin(); it!=positions_.end(); it++) {
        int position_id = stoi(*it);
        PosEnable[position_id] = -1;

        int param;
        char name[16];

        snprintf(name, sizeof(name), "Pos%u:Enable", position_id);
        createParam(name, asynParamInt32, &param, 0);
        PosEnable[position_id] = param;

        snprintf(name, sizeof(name), "Pos%u:Id", position_id);
        createParam(name, asynParamInt32, &param, position_id);
        PosId[position_id] = param;

        snprintf(name, sizeof(name), "Pos%u:CorrX", position_id);
        createParam(name, asynParamInt32, &param, 0);
        PosCorrX[position_id] = param;

        snprintf(name, sizeof(name), "Pos%u:CorrY", position_id);
        createParam(name, asynParamInt32, &param, 0);
        PosCorrY[position_id] = param;

        snprintf(name, sizeof(name), "Pos%u:PsUpX", position_id);
        createParam(name, asynParamInt32, &param, 0);
        PosPsUpX[position_id] = param;

        snprintf(name, sizeof(name), "Pos%u:PsLowX", position_id);
        createParam(name, asynParamInt32, &param, 0);
        PosPsLowX[position_id] = param;
    }

    setIntegerParam(NumPositions, positions_.size());
    callParamCallbacks();

    BasePlugin::connect(parentPlugins, MsgDasData);
}

asynStatus FlatFieldPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(CntGoodEvents, 0);
            setIntegerParam(CntPosVetos, 0);
            setIntegerParam(CntRangeVetos, 0);
            setIntegerParam(CntPosCfgVetos, 0);
            setIntegerParam(CntPsVetos, 0);
            callParamCallbacks();
            m_counters.reset();
        }
        return asynSuccess;
    } else if (pasynUser->reason == XyFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason == PsFractWidth) {
        if (value < 1 || value > 30)
            return asynError;
    } else if (pasynUser->reason ==  XMaxOut || pasynUser->reason == YMaxOut) {
        if (value < 1 || value >= 1024)
            return asynError;
    } else {
        for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
            if (pasynUser->reason == PosEnable[it->second.position_id]) {
                asynStatus ret = asynSuccess;
                if (value == 1 && (it->second.corrX == 0 || it->second.corrY == 0)) {
                    LOG_ERROR("Failed to enable position %u, missing correction tables", it->second.position_id);
                    value = 0;
                    ret = asynError;
                } else {
                    it->second.enabled = (value != 0);
                }
                setIntegerParam(PosEnable[it->second.position_id], value);
                callParamCallbacks();
                return ret;
            }
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus FlatFieldPlugin::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    if (pasynUser->reason == XMaxIn || pasynUser->reason == YMaxIn) {
        if (value <= 0.0)
            return asynError;
    }

    return BasePlugin::writeFloat64(pasynUser, value);
}

asynStatus FlatFieldPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == ImportReport) {
        *nActual = snprintf(value, nChars, "%s", m_importReport.c_str());
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

asynStatus FlatFieldPlugin::readInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn)
{
    if (pasynUser->reason == Positions) {
        size_t i = 0;
        for (auto it = PosId.begin(); it != PosId.end() && i < nElements; it++, i++) {
            value[i] = it->first;
        }
        *nIn = i;
        return asynSuccess;
    }
    return BasePlugin::readInt32Array(pasynUser, value, nElements, nIn);
}

asynStatus FlatFieldPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == ImportDir) {
        *nActual = nChars;
        if (nChars >0) {
            if (value[nChars-1] == 0)
                nChars--;
            std::string path(value, nChars);

            DIR *dir = opendir(path.c_str());
            if (dir == NULL) {
                LOG_ERROR("ImportDir '%s' is not directory", path.c_str());
                setIntegerParam(ImportStatus, IMPORT_STATUS_ERROR);
                callParamCallbacks();
                return asynError;
            } else if (m_importTimer.get() != 0) {
                LOG_ERROR("Importing files in progress");
                return asynError;
            } else {
                std::function<float(void)> importCb = std::bind(&FlatFieldPlugin::importFilesCb, this, path);
                m_importTimer = scheduleCallback(importCb, 0.1);
                if (m_importTimer.get() == 0) {
                    LOG_ERROR("Failed to schedule importing directory");
                    setIntegerParam(ImportStatus, IMPORT_STATUS_ERROR);
                    callParamCallbacks();
                    return asynError;
                }
            }
            setIntegerParam(ImportStatus, IMPORT_STATUS_BUSY);
            callParamCallbacks();
        }
        return asynSuccess;
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

void FlatFieldPlugin::recvDownstream(const DasDataPacketList &packets)
{
    double xMaxIn, yMaxIn;
    int xMaxOut, yMaxOut;

    // Parametrise calculating algorithms based on detector.
    int xyFractWidth = 0;
    int psFractWidth = 0;

    getIntegerParam(XyFractWidth,   &xyFractWidth);
    getIntegerParam(PsFractWidth,   &psFractWidth);
    getDoubleParam(XMaxIn,          &xMaxIn);
    getDoubleParam(YMaxIn,          &yMaxIn);
    getIntegerParam(XMaxOut,        &xMaxOut);
    getIntegerParam(YMaxOut,        &yMaxOut);

    // This is a trick to avoid locking access to member variables. Since
    // plugin design ensures a single instance of processDataUnlocked()
    // function at any time, we read in values and store them as class members.
    // This makes member variables const for the duration of this function.
    m_psScale = 1.0 / (1 << psFractWidth);
    m_xScaleIn = 1.0 * (m_tableSizeX - 1) / ((1 << xyFractWidth) * xMaxIn);
    m_yScaleIn = 1.0 * (m_tableSizeY - 1) / ((1 << xyFractWidth) * yMaxIn);
    m_xScaleOut = 1.0 * Bits::roundUpPower2(yMaxOut) * xMaxOut / (m_tableSizeX - 1);
    m_yScaleOut = 1.0 *                                yMaxOut / (m_tableSizeY - 1);
    m_xMaskOut = (Bits::roundUpPower2(xMaxOut) - 1) * Bits::roundUpPower2(yMaxOut);
    m_yMaskOut = (Bits::roundUpPower2(yMaxOut) - 1);

    DasDataPacketList outPackets;
    std::vector<DasDataPacket *> pooledPackets;

    for (const auto &packet: packets) {
        epicsTimeStamp timestamp = packet->getTimeStamp();
        uint32_t nEvents = packet->getNumEvents();

        std::pair<DasDataPacket*, Counters> res;
        if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_BNL_DIAG) {
            res = processEvents(timestamp, packet->getEvents<Event::BNL::Diag>(), nEvents);
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_ACPC_XY_PS) {
            res = processEvents(timestamp, packet->getEvents<Event::ACPC::Normal>(), nEvents);
        } else {
            res = std::make_pair(const_cast<DasDataPacket*>(packet), Counters());
        }

        if (res.first == packet) {
            // Probably not in right data format for this plugin
            outPackets.push_back(res.first);
        } else if (res.first != nullptr) {
            // We got a newly allocated packet with modified events in it
            outPackets.push_back(res.first);
            pooledPackets.push_back(res.first);
            m_counters += res.second;
        } else {
            // Can't allocate packet
            LOG_ERROR("Failed to allocate packet from pool");
        }
    }

    // Send to subscribed plugins and wait they complete processing
    sendDownstream(outPackets);

    // Return allocated packets back to pool
    for (auto &packet: pooledPackets) {
        m_packetsPool.put(packet);
    }

    setIntegerParam(CntGoodEvents,  m_counters[VETO_NO]           % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntInhVetos,    m_counters[VETO_INHERITED]    % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntPosVetos,    m_counters[VETO_POSITION]     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntRangeVetos,  m_counters[VETO_RANGE]        % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntPosCfgVetos, m_counters[VETO_POSITION_CFG] % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntPsVetos,     m_counters[VETO_PHOTOSUM]     % std::numeric_limits<int32_t>::max());
    callParamCallbacks();
}

// Only implement processEvents() for known input events.
// This could be solved with constexpr from std=c++17

std::pair<DasDataPacket *, FlatFieldPlugin::Counters> FlatFieldPlugin::processEvents(const epicsTimeStamp &timestamp, const Event::BNL::Diag *srcEvents, uint32_t nEvents) {
    Counters counters;
    DasDataPacket *packet = m_packetsPool.get(DasDataPacket::getLength(DasDataPacket::EVENT_FMT_BNL_DIAG, nEvents));
    if (packet != nullptr) {
        packet->init(DasDataPacket::EVENT_FMT_BNL_DIAG, timestamp, nEvents, srcEvents);
        packet->setEventsCorrected(true);
        Event::BNL::Diag *events = packet->getEvents<Event::BNL::Diag>();

        while (nEvents-- > 0) {
            // events were already (efficiently) copied by packet->init()
            // Only populate the changes.
            events->pixelid &= Event::Pixel::VETO_MASK;
            events->pixelid |= srcEvents->position;

            VetoType veto = VETO_NO;
            if (srcEvents->x < 0 || srcEvents->y < 0) {
                // Probably previously tagged veto
                veto = VETO_INHERITED;
            } else {
                events->corrected_x = srcEvents->x * m_xScaleIn;
                events->corrected_y = srcEvents->y * m_yScaleIn;

                veto = correctPosition(events->corrected_x, events->corrected_y, srcEvents->position);

                events->pixelid |= (std::lround(events->corrected_x * m_xScaleOut) & m_xMaskOut);
                events->pixelid |= (std::lround(events->corrected_y * m_yScaleOut) & m_yMaskOut);
            }
            counters[veto]++;

            if (veto != VETO_NO) {
                events->corrected_x = -1;
                events->corrected_y = -1;
                events->pixelid |= Event::Pixel::VETO_MASK;
            }

            srcEvents++;
            events++;
        }
    }
    return std::make_pair(packet, std::move(counters));
}

std::pair<DasDataPacket *, FlatFieldPlugin::Counters> FlatFieldPlugin::processEvents(const epicsTimeStamp &timestamp, const Event::ACPC::Normal *srcEvents, uint32_t nEvents) {
    Counters counters;
    DasDataPacket *packet = m_packetsPool.get(DasDataPacket::getLength(DasDataPacket::EVENT_FMT_ACPC_DIAG, nEvents));
    if (packet != nullptr) {
        packet->init(DasDataPacket::EVENT_FMT_ACPC_DIAG, timestamp, nEvents);
        packet->setEventsCorrected(true);
        Event::ACPC::Diag *events = packet->getEvents<Event::ACPC::Diag>();

        while (nEvents-- > 0) {
            // Changing format requires event-by-event copy
            *events = *srcEvents;
            events->pixelid &= Event::Pixel::VETO_MASK;
            events->pixelid |= srcEvents->position;

            VetoType veto = VETO_NO;
            if (srcEvents->x < 0 || srcEvents->y < 0) {
                // Probably previously tagged veto - don't double count
                veto = VETO_INHERITED;
            } else {
                events->corrected_x = srcEvents->x * m_xScaleIn;
                events->corrected_y = srcEvents->y * m_yScaleIn;
                double photo_sum_x = srcEvents->photo_sum_x * m_psScale;

                // Check photo sum first
                veto = checkPhotoSumLimits(events->corrected_x, events->corrected_y, photo_sum_x, srcEvents->position);
                if (veto == VETO_NO) {
                    // Apply flat-field correction
                    veto = correctPosition(events->corrected_x, events->corrected_y, srcEvents->position);
                }

                // Calculate pixelid
                events->pixelid |= (std::lround(events->corrected_x * m_xScaleOut) & m_xMaskOut);
                events->pixelid |= (std::lround(events->corrected_y * m_yScaleOut) & m_yMaskOut);
            }
            counters[veto]++;

            if (veto != VETO_NO) {
                events->corrected_x = -1;
                events->corrected_y = -1;
                events->pixelid |= Event::Pixel::VETO_MASK;
            }

            srcEvents++;
            events++;
        }
    }
    return std::make_pair(packet, counters);
}

FlatFieldPlugin::VetoType FlatFieldPlugin::correctPosition(double &x, double &y, uint32_t position)
{
    std::shared_ptr<FlatFieldTable> xtable = m_tables[position].corrX;
    std::shared_ptr<FlatFieldTable> ytable = m_tables[position].corrY;
    if (!xtable || !ytable || m_tables[position].enabled == false)
        return VETO_POSITION;

    unsigned xp = x;
    unsigned yp = y;

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= (xtable->sizeX-1) || y < 0.0 || yp >= (xtable->sizeY-1))
        return VETO_RANGE;

    // All checks passed - do the correction
    double dx = (xp == 0 || xp == (xtable->sizeX - 1)) ? 0 : x - xp;
    double dy = (yp == 0 || yp == (ytable->sizeY - 1)) ? 0 : y - yp;

    x -= (dx * xtable->data[xp+1][yp]) + ((1 - dx) * xtable->data[xp][yp]);
    y -= (dy * ytable->data[xp][yp+1]) + ((1 - dy) * ytable->data[xp][yp]);

    return VETO_NO;
}

FlatFieldPlugin::VetoType FlatFieldPlugin::checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position)
{
    std::shared_ptr<FlatFieldTable> upperLimits = m_tables[position].psUpX;
    std::shared_ptr<FlatFieldTable> lowerLimits = m_tables[position].psLowX;
    if (!upperLimits || !lowerLimits || m_tables[position].enabled == false)
        return VETO_POSITION;

    unsigned xp = nearbyint(x);
    unsigned yp = nearbyint(y);

    // All tables of the same size, safe to compare against just one
    if (x < 0.0 || xp >= (upperLimits->sizeX-1) || y < 0.0 || yp >= (upperLimits->sizeY-1))
        return VETO_RANGE;

    double upperLimit = upperLimits->data[xp][yp];
    double lowerLimit = lowerLimits->data[xp][yp];

    if (lowerLimit <= photosum_x && photosum_x <= upperLimit)
        return VETO_NO;
    else
        return VETO_PHOTOSUM;
}

float FlatFieldPlugin::importFilesCb(const std::string &path)
{
    // Importing takes quite some time, don't starve other plugins in the mean time.
    // So let's unsubscribe temporarily from receiving any data
    bool enabled = this->isConnected();
    if (enabled) {
        this->disconnect();
    }
    importFiles(path);
    if (enabled) {
        this->connect(m_parentPlugins, MsgDasData);
    }
    m_importTimer.reset();
    return 0.0;
}

void FlatFieldPlugin::importFiles(const std::string &path_)
{
    struct dirent entry, *result;
    bool foundCorrTables = false;
    bool foundPsTables = false;
    std::ostringstream importReport;
    std::string path(path_);

    if (path.empty()) {
        LOG_ERROR("Invalid import directory");
        m_importReport = "none (invalid directory)";
        setIntegerParam(ImportStatus, IMPORT_STATUS_ERROR);
        callParamCallbacks();
        return;
    } else if (path[path.length()] != PATH_SEPARATOR) {
        path += PATH_SEPARATOR;
    }

    LOG_INFO("Importing files from %s", path.c_str());
    importReport << "Importing files from " << path << ":" << std::endl;

    DIR *dir = opendir(path.c_str());
    if (dir == NULL) {
        LOG_ERROR("Failed to read import directory '%s': %s", path.c_str(), strerror(errno));
        m_importReport = "none (failed to read directory)";
        setIntegerParam(ImportStatus, IMPORT_STATUS_ERROR);
        callParamCallbacks();
        return;
    }

    this->unlock();
    std::map<uint32_t, PositionTables> tables;
    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
        std::string filename(entry.d_name);
        std::string filepath(path + filename);
        if (!Common::isDir(filepath)) {
            std::shared_ptr<FlatFieldTable> table(new FlatFieldTable());
            if (table.get() == 0) {
                LOG_ERROR("Failed to initialize table");
                importReport << " * " << filename << ": error - failed to allocate table" << std::endl;
                continue;
            }
            if (table->import(filepath) == false) {
                LOG_ERROR("Failed to import file '%s': %s", filename.c_str(), table->getImportError().c_str());
                importReport << " * " << filename << ": error - " << table->getImportError() << std::endl;
                continue;
            }

            // Initialize the common table size variable
            if (m_tableSizeX == 0) {
                m_tableSizeX = table->sizeX;
                m_tableSizeY = table->sizeY;
            }

            // Ensure all tables are of the same size
            if (m_tableSizeX != table->sizeX || m_tableSizeY != table->sizeY) {
                LOG_ERROR("Table size mismatch");
                importReport << " * " << filename << ": error - table size mismatch" << std::endl;
                continue;
            }

            // Push table to tables container
            if (table->type == FlatFieldTable::TYPE_X_CORR) {
                if (tables[table->pixel_offset].corrX.get() == 0) {
                    tables[table->pixel_offset].corrX = table;
                    tables[table->pixel_offset].position_id = table->position_id;
                    tables[table->pixel_offset].nTables++;
                    foundCorrTables = true;
                    setIntegerParam(PosCorrX[table->position_id], 1);
                } else {
                    LOG_ERROR("Correction X table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    setIntegerParam(PosCorrX[table->position_id], 0);
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_Y_CORR) {
                if (tables[table->pixel_offset].corrY.get() == 0) {
                    tables[table->pixel_offset].corrY = table;
                    tables[table->pixel_offset].position_id = table->position_id;
                    tables[table->pixel_offset].nTables++;
                    foundCorrTables = true;
                    setIntegerParam(PosCorrY[table->position_id], 1);
                } else {
                    LOG_ERROR("Correction Y table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    setIntegerParam(PosCorrY[table->position_id], 0);
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_LOW) {
                if (tables[table->pixel_offset].psLowX.get() == 0) {
                    tables[table->pixel_offset].psLowX = table;
                    tables[table->pixel_offset].position_id = table->position_id;
                    tables[table->pixel_offset].nTables++;
                    foundPsTables = true;
                    setIntegerParam(PosPsLowX[table->position_id], 1);
                } else {
                    LOG_ERROR("Photosum low X table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    setIntegerParam(PosPsLowX[table->position_id], 0);
                    continue;
                }
            } else if (table->type == FlatFieldTable::TYPE_X_PS_UP) {
                if (tables[table->pixel_offset].psUpX.get() == 0) {
                    tables[table->pixel_offset].psUpX = table;
                    tables[table->pixel_offset].position_id = table->position_id;
                    tables[table->pixel_offset].nTables++;
                    foundPsTables = true;
                    setIntegerParam(PosPsUpX[table->position_id], 1);
                } else {
                    LOG_ERROR("Photosum upper X table already loaded for position %u (pixel offset=%u)", table->position_id, table->pixel_offset);
                    importReport << " * " << filename << ": error - table for this position already loaded" << std::endl;
                    setIntegerParam(PosPsUpX[table->position_id], 0);
                    continue;
                }
            }

            LOG_INFO("Imported file '%s'", filename.c_str());
            importReport << " * " << filename << ": imported" << std::endl;
        }
    }
    closedir(dir);

    this->lock();
    m_tables = tables;
    m_importReport = importReport.str();

    // Check tables constraints for each position:
    // * all tables of the same size - already checked when tables created
    // * X and Y correction tables loaded if using correction (foundCorrTables==true)
    // * Lower and upper X photosum tables loaded if using photosum (foundPsTable==true)
    bool nPositions = 0;
    for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {

        if (it->second.corrX.get() == 0 && it->second.corrY.get() == 0 && it->second.psLowX.get() == 0 && it->second.psUpX.get() == 0)
            continue;

        nPositions++;

        it->second.enabled = true;
        if (foundCorrTables == true) {
            if (it->second.corrX.get() == 0) {
                LOG_ERROR("Missing X correction table for position %u", it->second.position_id);
                it->second.enabled = false;
            }
            if (it->second.corrY.get() == 0) {
                LOG_ERROR("Missing Y correction table for position %u", it->second.position_id);
                it->second.enabled = false;
            }
        }

        if (foundPsTables == true) {
            if (it->second.psLowX.get() == 0) {
                LOG_ERROR("Missing lower X photosum correction table for position %u", it->second.position_id);
                it->second.enabled = false;
            }
            if (it->second.psUpX.get() == 0) {
                LOG_ERROR("Missing upper X photosum correction table for position %u", it->second.position_id);
                it->second.enabled = false;
            }
        }
        setIntegerParam(PosEnable[it->second.position_id], it->second.enabled);
    }
    setIntegerParam(ImportStatus, IMPORT_STATUS_DONE);
    setIntegerParam(TablesSizeX, (int)m_tableSizeX);
    setIntegerParam(TablesSizeY, (int)m_tableSizeY);
    callParamCallbacks();
}

std::string FlatFieldPlugin::generatePositionsReport()
{
    std::ostringstream report;
    for (auto it=m_tables.begin(); it!=m_tables.end(); it++) {
        // Skip sparse vector entries
        if (it->second.nTables == 0)
            continue;

        report << "Position " << it->second.position_id << (it->second.enabled == false ? " NOT " : " ") << "enabled:" << std::endl;
        report << " * upper X photosum table " << (it->second.psUpX.get()  != 0 ? "loaded" : "MISSING") << std::endl;
        report << " * lower X photosum table " << (it->second.psLowX.get() != 0 ? "loaded" : "MISSING") << std::endl;
        report << " * X correction table " << (it->second.corrX.get()  != 0 ? "loaded" : "MISSING") << std::endl;
        report << " * Y correction table " << (it->second.corrY.get()  != 0 ? "loaded" : "MISSING") << std::endl;
    }
    return report.str();
}

void FlatFieldPlugin::report(FILE *fp, int details)
{
    std::string positionsReport = generatePositionsReport();
    fwrite(positionsReport.c_str(), 1, positionsReport.length(), fp);
    fwrite(m_importReport.c_str(), 1, m_importReport.length(), fp);
    return BasePlugin::report(fp, details);
}

FlatFieldPlugin::Counters::Counters()
{
    reset();
}

void FlatFieldPlugin::Counters::reset()
{
    m_map.clear();
    m_map[VETO_NO]              = 0;
    m_map[VETO_INHERITED]       = 0;
    m_map[VETO_POSITION]        = 0;
    m_map[VETO_RANGE]           = 0;
    m_map[VETO_POSITION_CFG]    = 0;
    m_map[VETO_PHOTOSUM]        = 0;
}

FlatFieldPlugin::Counters &FlatFieldPlugin::Counters::operator+=(const FlatFieldPlugin::Counters &rhs)
{
    for (auto it = rhs.m_map.begin(); it != rhs.m_map.end(); it++) {
        m_map[it->first] += it->second;
    }
    return *this;
}

uint32_t &FlatFieldPlugin::Counters::operator[](VetoType index)
{
    return m_map[index];
}
