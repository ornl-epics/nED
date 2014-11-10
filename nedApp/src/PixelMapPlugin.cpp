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

#include <fstream>
#include <string>

#define NUM_PIXELMAPPLUGIN_PARAMS ((int)(&LAST_PIXELMAPPLUGIN_PARAM - &FIRST_PIXELMAPPLUGIN_PARAM + 1))

#define PIXID_ERR       (1 << 31)
#define PIXID_ERR_MAP   PIXID_ERR
//#define PIXID_ERR_MAP   (1 << 27 | PIXID_ERR) // TODO: Uncomment if the error pickels should be additionally flagged by this plugin

EPICS_REGISTER_PLUGIN(PixelMapPlugin, 5, "Port name", string, "Dispatcher port name", string, "Blocking", int, "PixelMap file", string, "Buffer size", int);

PixelMapPlugin::PixelMapPlugin(const char *portName, const char *dispatcherPortName, int blocking, const char *pixelMapFile, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, blocking, NUM_PIXELMAPPLUGIN_PARAMS)
    , m_buffer(0)
    , m_bufferSize(0)
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
        err = importPixelMapFile(pixelMapFile);

    createParam("MapErr",       asynParamInt32, &MapErr,    err); // Last mapping import error
    createParam("PassThru",     asynParamInt32, &PassThru,  0);   // Skip remapping pixels
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels
    createParam("CntError",     asynParamInt32, &CntError,  0);   // Number of unknown-error pixels
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);   // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    callParamCallbacks();
}

PixelMapPlugin::~PixelMapPlugin()
{
    if (m_buffer != 0)
        free(m_buffer);
}

asynStatus PixelMapPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        if (value > 0) {
            setIntegerParam(CntUnmap, 0);
            setIntegerParam(CntError, 0);
            setIntegerParam(CntSplit, 0);
            callParamCallbacks();
        }
        return asynSuccess;
    }
    return BaseDispatcherPlugin::writeInt32(pasynUser, value);
}

void PixelMapPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int passthru = 0;
    int nSplits = 0;
    PixelMapErrors errors;

    if (m_buffer == 0) {
        LOG_ERROR("No buffer allocated, skipping pixel mapping");
        return;
    }

    this->lock();
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(CntUnmap,   &errors.nUnmapped);
    getIntegerParam(CntError,   &errors.nErrors);
    getIntegerParam(CntSplit,   &nSplits);
    getIntegerParam(PassThru,   &passthru);
    if (getDataMode() != BasePlugin::DATA_MODE_NORMAL || m_map.empty())
        passthru = 1;
    this->unlock();

    nReceived += packetList->size();

    // Software design ensures single instance of this function running
    // at any given time. We must ensure the same for our clients - that is
    // wait until they're done processing before sending them some more data.

    if (passthru != 0) {
        m_packetList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&m_packetList);
        m_packetList.release();
        m_packetList.waitAllReleased();
        m_packetList.clear();
    } else {
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {

            uint32_t bufferOffset = 0;
            m_packetList.clear();
            m_packetList.reserve();

            // Limit number of threads as working payload is not that big.
            // More threads showed more synchronization and bigger runtime overall
            #pragma omp parallel num_threads(4)
            #pragma omp single
            {
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
                        m_packetList.push_back(packet);
                        continue;
                    }

                    // Reserve part of buffer for this packet, it may shrink from original but never grow
                    DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                    m_packetList.push_back(newPacket);
                    bufferOffset += packet->length();

                    // Do the pixel id mapping - this can be parallelized
                    #pragma omp task firstprivate(packet, newPacket) shared(errors)
                    errors += packetMap(packet, newPacket);

                    nProcessed++;
                }

                // Synchronize threads
                #pragma omp taskwait

                sendToPlugins(&m_packetList);
                m_packetList.release();
                m_packetList.waitAllReleased();
            }
        }
    }

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntUnmap,   errors.nUnmapped);
    setIntegerParam(CntError,   errors.nErrors);
    callParamCallbacks();
    this->unlock();
}

PixelMapPlugin::PixelMapErrors PixelMapPlugin::packetMap(const DasPacket *srcPacket, DasPacket *destPacket)
{
    PixelMapErrors errors;

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents, nDestEvents;
    const DasPacket::Event *srcEvent= reinterpret_cast<const DasPacket::Event *>(srcPacket->getData(&nEvents));
    DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(destPacket->getData(&nDestEvents));
    nEvents /= (sizeof(DasPacket::Event) / sizeof(uint32_t));

    // The below code was optimized for speed and is not as elegant as could be
    // otherwise. Bitfield, condition rearranging were both tried with worse results.
    destPacket->payload_length += nEvents * sizeof(DasPacket::Event);
    while (nEvents-- > 0) {

        if (likely((srcEvent->pixelid & PIXID_ERR) == 0 && srcEvent->pixelid < m_map.size())) {
            // Gaps in pixel map table resolve to error pixels,
            // no need to care here
            destEvent->pixelid = m_map[srcEvent->pixelid];
        } else if (srcEvent->pixelid & PIXID_ERR) { // Already tagged as error
            destEvent->pixelid = srcEvent->pixelid;
            errors.nErrors++;
        } else {
            destEvent->pixelid = srcEvent->pixelid | PIXID_ERR_MAP;
            errors.nUnmapped++;
        }

        destEvent->tof = srcEvent->tof;
        srcEvent++;
        destEvent++;
    }

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
