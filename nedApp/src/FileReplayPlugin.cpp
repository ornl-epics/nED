/* TcpClientPlugin.cpp
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Log.h"
#include "FileCircularBuffer.h"
#include "FileReplayPlugin.h"

#include <alarm.h> // EPICS alarm and severity enumerations
#include <cstring> // strerror

EPICS_REGISTER_PLUGIN(FileReplayPlugin, 1, "Port name", string);

FileReplayPlugin::FileReplayPlugin(const char *portName)
    : BasePortPlugin(portName, 0, asynOctetMask, asynOctetMask)
{
    createParam("Status",           asynParamInt32,     &Status, 0);                // READ - Status of FileReplayPlugin
    createParam("StatusText",       asynParamOctet,     &StatusText, "no file");    // READ - Current error text if any
    createParam("FilePath",         asynParamOctet,     &FilePath, "");             // WRITE - Remote hostname or IP to connect to
    createParam("Control",          asynParamInt32,     &Control, 0);               // WRITE - Control, 0 - pause, 1 - continue, 2 - reset
    createParam("Speed",            asynParamFloat64,   &Speed, 1.0);               // WRITE - Replay speed, 1.0 is real time, <1 is slower, >1 faster
    createParam("MaxPackets",       asynParamInt32,     &MaxPackets, 100);          // WRITE - Maximum number of packets to be sent in one Plugin message
    createParam("Running",          asynParamInt32,     &Running, 0);               // READ - Running status, 0 stopped, 1 running
    callParamCallbacks();

    m_circularBuffer = &m_file;

    if (!m_processThread) {
        LOG_ERROR("Failed to create processing thread");
    } else {
        m_processThread->start();
    }
}

FileReplayPlugin::~FileReplayPlugin()
{
    if (m_processThread) m_processThread->stop();
}

asynStatus FileReplayPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == FilePath) {
        *nActual = nChars;

        std::string path(value, nChars);
        std::string error;
        if (!m_file.open(path, error)) {
            LOG_ERROR("Failed to open file '%s' - %s", path.c_str(), error.c_str());
            setIntegerParam(Status, 0);
            setStringParam(StatusText, error);
            callParamCallbacks();
            return asynError;
        } else {
            LOG_INFO("Opened file '%s'", path.c_str());
            setIntegerParam(Status, 1);
            setStringParam(StatusText, "opened");
            callParamCallbacks();
            return asynSuccess;
        }
    }
    return BasePlugin::writeOctet(pasynUser, value, nChars, nActual);
}

asynStatus FileReplayPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Control) {
        if (value == 0) {
            m_file.stop();
        } else if (value == 1) {
            m_file.setOldPacketType(getBooleanParam(OldPktsEn));
            m_file.start();
        } else if (value == 2) {
            m_file.setOldPacketType(getBooleanParam(OldPktsEn));
            m_file.reset();
        }
        setIntegerParam(Running, m_file.isRunning());
        callParamCallbacks();
        return asynSuccess;
    } else if (pasynUser->reason == Speed) {
        m_file.setSpeed(value / 10.0);
        return asynSuccess;
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

bool FileReplayPlugin::send(const uint8_t *data, size_t len)
{
    // Silently discard outgoing data
    return true;
}

void FileReplayPlugin::handleRecvError(int ret)
{
    this->lock();
    setIntegerParam(Status, 0);
    setStringParam(StatusText, "read error");
    callParamCallbacks();
    this->unlock();
}
