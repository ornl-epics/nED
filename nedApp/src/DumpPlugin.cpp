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

#define NUM_DUMPPLUGIN_PARAMS ((int)(&LAST_DUMPPLUGIN_PARAM - &FIRST_DUMPPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(DumpPlugin, 3, "Port name", string, "Dispatcher port name", string, "Blocking", int);

DumpPlugin::DumpPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_DUMPPLUGIN_PARAMS, 1, asynOctetMask, asynOctetMask)
    , m_fd(-1)
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
    callParamCallbacks();
}

DumpPlugin::~DumpPlugin()
{
    closeFile();
}

void DumpPlugin::processData(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);

    nReceived += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        if (m_fd == -1)
            continue;

        // Skip filtered-out packets
        if ( (m_rtdlEn     && packet->isRtdl()        ) ||
             (m_neutronEn  && packet->isNeutronData() ) ||
             (m_metadataEn && packet->isMetaData()    ) ||
             (m_cmdEn      && packet->isCommand()     ) ||
             (m_unknwnEn) ) {

            ssize_t ret = write(m_fd, packet, packet->length());
            if (ret != static_cast<ssize_t>(packet->length())) {
                if (ret == -1) {
                    LOG_ERROR("Abort dumping to file due to an error: %s", strerror(errno));
                    closeFile();
                    break;
                } else {
                    off_t offset = lseek(m_fd, 0, SEEK_CUR) - ret;
                    LOG_WARN("Only dumped %zuB out of %uB at offset %lu", ret, packet->length(), offset);
                    continue;
                }
            }

            nProcessed++;
        }
    }

    setIntegerParam(RxCount,    nReceived);
    setIntegerParam(ProcCount,  nProcessed);
    callParamCallbacks();
}

asynStatus DumpPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        if (value > 0) {
            char path[1024];
            if (m_fd == -1 && getStringParam(FilePath, sizeof(path), path) == asynSuccess && openFile(path) == false)
                return asynError;
        } else {
            closeFile();
        }
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
        // If plugin is enabled, switch file now, otherwise postpone until enabled
        if (getIntegerParam(Enable, &enabled) == asynSuccess && enabled == 1) {
            closeFile();
            if (openFile(std::string(value, nChars)) == false) {
                return asynError;
            }
        }
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

bool DumpPlugin::openFile(const std::string &path)
{
    m_fd = open(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (m_fd == -1) {
        LOG_ERROR("Can not open dump file '%s': %s", path.c_str(), strerror(errno));
        return false;
    }

    LOG_INFO("Switched dump file to '%s'", path.c_str());
    return true;
}

void DumpPlugin::closeFile()
{
    if (m_fd != -1) {
        (void)fcntl(m_fd, F_SETFL, O_NONBLOCK); // best we can do, if fcntl() fails, close() could stall for a while
        (void)close(m_fd);
        m_fd = -1;
    }
}
