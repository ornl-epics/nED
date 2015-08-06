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

#define NUM_DUMPPLUGIN_PARAMS ((int)(&LAST_DUMPPLUGIN_PARAM - &FIRST_DUMPPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(DumpPlugin, 3, "Port name", string, "Dispatcher port name", string, "Blocking", int);

DumpPlugin::DumpPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_DUMPPLUGIN_PARAMS, 1, asynOctetMask, asynOctetMask)
    , m_fd(-1)
    , m_fdIsPipe(false)
    , m_rtdlEn(true)
    , m_neutronEn(true)
    , m_metadataEn(true)
    , m_cmdEn(true)
    , m_unknwnEn(true)
{
    createParam("FilePath",         asynParamOctet, &FilePath);          // WRITE - Path to file where to save all received data
    createParam("RtdlPktsEn",       asynParamInt32, &RtdlPktsEn, 1);     // WRITE - Switch for RTDL packets
    createParam("NeutronPktsEn",    asynParamInt32, &NeutronPktsEn, 1);  // WRITE - Switch for neutron packets
    createParam("MetadataPktsEn",   asynParamInt32, &MetadataPktsEn, 1); // WRITE - Switch for metadata packets
    createParam("CmdPktsEn",        asynParamInt32, &CmdPktsEn, 1);      // WRITE - Switch for command packets
    createParam("UnknwnPktsEn",     asynParamInt32, &UnknwnPktsEn, 1);   // WRITE - Switch for unrecognized packets
    createParam("SavedCount",       asynParamInt32, &SavedCount, 0);     // READ - Num saved packets to file
    createParam("NotSavedCount",    asynParamInt32, &NotSavedCount, 0);  // READ - Num not saved packets due error
    createParam("CorruptOffset",    asynParamInt32, &CorruptOffset, 0);  // READ - Corrupted data absolute offset in file
    createParam("Overwrite",        asynParamInt32, &Overwrite, 0);      // WRITE - Overwrite existing file
    callParamCallbacks();
}

DumpPlugin::~DumpPlugin()
{
    closeFile();
}

void DumpPlugin::processData(const DasPacketList * const packetList)
{
    int nReceived = 0, nProcessed = 0, nSaved = 0, nNotSaved = 0, corruptOffset = 0;
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(SavedCount,     &nSaved);
    getIntegerParam(NotSavedCount,  &nNotSaved);
    getIntegerParam(CorruptOffset,  &corruptOffset);

    nReceived += packetList->size();

    if (m_fd == -1) {
        // Plugin is enabled but file was not opened - either not specified or error
        nNotSaved += packetList->size();
    } else {
        for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
            const DasPacket *packet = *it;

            // Skip filtered-out packets
            if ( (m_rtdlEn     && packet->isRtdl()        ) ||
                 (m_neutronEn  && packet->isNeutronData() ) ||
                 (m_metadataEn && packet->isMetaData()    ) ||
                 (m_cmdEn      && packet->isCommand()     ) ||
                 (m_unknwnEn) ) {

                nProcessed++;

                // m_fd is non-blocking, might fail when system buffers are full
                ssize_t ret = write(m_fd, packet, packet->length());
                if (ret == static_cast<ssize_t>(packet->length())) {
                    nSaved++;
                } else {
                    nNotSaved++;
                    if (ret == -1) {
                        LOG_WARN("Failed to save packet to file: %s", strerror(errno));
                    } else if (m_fdIsPipe) {
                        // Nothing we can do about it
                        char path[1024];
                        getStringParam(FilePath, sizeof(path), path);
                        LOG_ERROR("Wrote %zd/%d bytes to pipe %s - reader will be confused", ret, packet->length(), path);
                        if (corruptOffset == 0) {
                            corruptOffset = lseek(m_fd, 0, SEEK_CUR) - ret;
                        }
                    } else if (lseek(m_fd, -1 * ret, SEEK_CUR) != 0) {
                        // Too bad but lseek() failed - very unlikely
                        off_t offset = lseek(m_fd, 0, SEEK_CUR) - ret;
                        char path[1024];
                        getStringParam(FilePath, sizeof(path), path);
                        LOG_ERROR("Wrote %zd/%d bytes to %s at offset %lu", ret, packet->length(), path, offset);
                        if (corruptOffset == 0) {
                            corruptOffset = offset;
                        }
                    } else {
                        LOG_WARN("Failed to save packet to file");
                    }
                }
            }
        }
    }

    setIntegerParam(RxCount,        nReceived);
    setIntegerParam(ProcCount,      nProcessed);
    setIntegerParam(SavedCount,     nSaved);
    setIntegerParam(NotSavedCount,  nNotSaved);
    setIntegerParam(CorruptOffset,  corruptOffset);
    callParamCallbacks();
}

asynStatus DumpPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        // Prevent reporting false positives by disabling plugin early.
        // Disabling plugin might temporarily release a lock and let some
        // packets come through.
        if (BasePlugin::writeInt32(pasynUser, value) != asynSuccess)
            return asynError;

        closeFile();

        if (value > 0) {
            char path[1024];

            setIntegerParam(SavedCount, 0);
            setIntegerParam(NotSavedCount, 0);
            setIntegerParam(CorruptOffset, 0);
            callParamCallbacks();

            if (getStringParam(FilePath, sizeof(path), path) == asynSuccess) {
                int overwrite = 0;
                getIntegerParam(Overwrite, &overwrite);
                // Ignore errors on opening file, there's NotSavedCount
                // that will increment
                (void)openFile(path, overwrite != 0);
            }
        }
        return asynSuccess;
    } else if (pasynUser->reason == RtdlPktsEn) {
        m_rtdlEn = (value > 0);
    } else if (pasynUser->reason == NeutronPktsEn) {
        m_neutronEn = (value > 0);
    } else if (pasynUser->reason == MetadataPktsEn) {
        m_metadataEn = (value > 0);
    } else if (pasynUser->reason == CmdPktsEn) {
        m_cmdEn = (value > 0);
    } else if (pasynUser->reason == UnknwnPktsEn) {
        m_unknwnEn = (value > 0);
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus DumpPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == FilePath) {
        int enabled = 0;
        *nActual = nChars;
        // If plugin is enabled, switch file now, otherwise postpone until enabled
        if (getIntegerParam(Enable, &enabled) == asynSuccess && enabled == 1) {
            closeFile();

            setIntegerParam(SavedCount, 0);
            setIntegerParam(NotSavedCount, 0);
            setIntegerParam(CorruptOffset, 0);
            callParamCallbacks();

            int overwrite = 0;
            getIntegerParam(Overwrite, &overwrite);

            // Ignore errors on opening file, there's NotSavedCount
            // that will increment
            (void)openFile(std::string(value, nChars), overwrite != 0);
        }
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
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
