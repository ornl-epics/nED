/* DumpPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DumpPlugin.h"
#include "Log.h"

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

EPICS_REGISTER_PLUGIN(DumpPlugin, 2, "Port name", string, "Parent plugins", string);

DumpPlugin::DumpPlugin(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, 1, asynOctetMask, asynOctetMask)
{
    createParam("Enable",           asynParamInt32, &Enable, 0);         // WRITE - Enable saving data - master switch
    createParam("FilePath",         asynParamOctet, &FilePath);          // WRITE - Path to file where to save all received data
    createParam("ErrorPktsEn",      asynParamInt32, &ErrorPktsEn, 1);    // WRITE - Enable saving error packets
    createParam("RtdlPktsEn",       asynParamInt32, &RtdlPktsEn, 1);     // WRITE - Enable saving DAS RTDL packets
    createParam("DataPktsEn",       asynParamInt32, &DataPktsEn, 1);     // WRITE - Enable saving DAS data packets
    createParam("CmdPktsEn",        asynParamInt32, &CmdPktsEn, 1);      // WRITE - Enable saving DAS command packets
    createParam("OldPktsEn",        asynParamInt32, &OldPktsEn, 0);      // WRITE - Enable saving DAS command packets
    createParam("SavedCount",       asynParamInt32, &SavedCount, 0);     // READ - Num saved packets to file
    createParam("NotSavedCount",    asynParamInt32, &NotSavedCount, 0);  // READ - Num not saved packets due error
    createParam("Overwrite",        asynParamInt32, &Overwrite, 0);      // WRITE - Overwrite existing file
    createParam("DataType",         asynParamInt32, &DataType, 0);       // WRITE - Data type packets to save
    createParam("CmdType",          asynParamInt32, &CmdType, 0);        // WRITE - Command type packets to save
    callParamCallbacks();

    // Let connect the first time, helps diagnose start-up problems
    connect(parentPlugins, {MsgDasData, MsgDasCmd, MsgDasRtdl, MsgError, MsgOldDas});
    disconnect();
}

DumpPlugin::~DumpPlugin()
{
    closeFile();
}

void DumpPlugin::recvDownstream(const DasDataPacketList &packets)
{
    if (getBooleanParam(DataPktsEn)) {
        int saved = 0;
        int failed = 0;
        int dataType = getIntegerParam(DataType);

        for (const auto& packet: packets) {
            if (dataType == 0 || packet->getEventsFormat() == dataType) {
                if (writeToFile(packet, packet->getLength()))
                    saved++;
                else
                    failed++;
            }
        }

        addIntegerParam(SavedCount, saved);
        addIntegerParam(NotSavedCount, failed);
        callParamCallbacks();
    }
}

void DumpPlugin::recvDownstream(const RtdlPacketList &packets)
{
    if (getBooleanParam(RtdlPktsEn)) {
        int saved = 0;
        int total = packets.size();

        for (const auto& packet: packets) {
            if (writeToFile(packet, packet->getLength()))
                saved++;
        }

        setIntegerParam(SavedCount, getIntegerParam(SavedCount) + saved);
        setIntegerParam(NotSavedCount, getIntegerParam(NotSavedCount) + (total - saved));
        callParamCallbacks();
    }
}

void DumpPlugin::recvDownstream(const DasCmdPacketList &packets)
{
    if (getBooleanParam(CmdPktsEn)) {
        int saved = 0;
        int total = packets.size();
        int cmdType = getIntegerParam(CmdType);

        for (const auto& packet: packets) {
            if (cmdType == 0 || packet->getCommand() == cmdType) {
                if (writeToFile(packet, packet->getLength()))
                    saved++;
            }
        }

        setIntegerParam(SavedCount, getIntegerParam(SavedCount) + saved);
        setIntegerParam(NotSavedCount, getIntegerParam(NotSavedCount) + (total - saved));
        callParamCallbacks();
    }
}

void DumpPlugin::recvDownstream(const ErrorPacketList &packets)
{
    if (getBooleanParam(ErrorPktsEn)) {
        int saved = 0;
        int total = packets.size();

        for (const auto& packet: packets) {
            if (writeToFile(packet, packet->getLength()))
                saved++;
        }

        setIntegerParam(SavedCount, getIntegerParam(SavedCount) + saved);
        setIntegerParam(NotSavedCount, getIntegerParam(NotSavedCount) + (total - saved));
        callParamCallbacks();
    }
}

void DumpPlugin::recvDownstream(const DasPacketList &packets)
{
    if (getBooleanParam(OldPktsEn)) {
        int saved = 0;
        int total = packets.size();

        for (const auto& packet: packets) {
            if (writeToFile(packet, packet->getLength()))
                saved++;
        }

        setIntegerParam(SavedCount, getIntegerParam(SavedCount) + saved);
        setIntegerParam(NotSavedCount, getIntegerParam(NotSavedCount) + (total - saved));
        callParamCallbacks();
    }
}

bool DumpPlugin::writeToFile(const void *data, uint32_t len)
{
    if (m_fd == -1)
        return false;

    // m_fd is non-blocking, might fail when system buffers are full
    ssize_t ret = write(m_fd, data, len);
    if (ret == static_cast<ssize_t>(len))
        return true;

    if (ret == -1) {
        LOG_WARN("Failed to save packet to file: %s", strerror(errno));
    } else if (m_fdIsPipe) {
        // Nothing we can do about it
        char path[1024];
        getStringParam(FilePath, sizeof(path), path);
        LOG_ERROR("Wrote %zd/%d bytes to pipe %s - reader will be confused", ret, len, path);
    } else if (lseek(m_fd, -1 * ret, SEEK_CUR) != 0) {
        // Too bad but lseek() failed - very unlikely
        off_t offset = lseek(m_fd, 0, SEEK_CUR) - ret;
        char path[1024];
        getStringParam(FilePath, sizeof(path), path);
        LOG_ERROR("Wrote %zd/%d bytes to %s at offset %lu", ret, len, path, offset);
    } else {
        LOG_WARN("Failed to save packet to file");
    }
    return false;
}

asynStatus DumpPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        if (value == 0) {
            closeFile();
            disconnect();
        } else {
            char path[1024];

            setIntegerParam(SavedCount, 0);
            setIntegerParam(NotSavedCount, 0);
            callParamCallbacks();

            if (getStringParam(FilePath, sizeof(path), path) == asynSuccess) {
                if (!openFile(path, getBooleanParam(Overwrite)))
                    return asynError;
                connect();
            }
        }
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

bool DumpPlugin::openFile(const std::string &path, bool overwrite)
{
    struct stat statBuf;

    if (stat(path.c_str(), &statBuf) == 0 && overwrite == false) {
        LOG_WARN("Not overwriting file '%s'", path.c_str());
        return false;
    }

    // Create new file if necessary, truncate existing file, works with named pipes
    m_fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, 0644);
    if (m_fd == -1) {
        LOG_ERROR("Can not open dump file '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    m_fdIsPipe = ((statBuf.st_mode & S_IFIFO) == S_IFIFO);

    LOG_INFO("Switched dump to %s '%s'", (m_fdIsPipe ? "named pipe" : "regular file"), path.c_str());
    return true;
}

void DumpPlugin::closeFile()
{
    if (m_fd != -1) {
        (void)close(m_fd);
        m_fd = -1;
    }
    m_fdIsPipe = false;
}
