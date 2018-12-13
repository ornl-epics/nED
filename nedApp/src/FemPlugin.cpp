/* FemPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "FemPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(FemPlugin, 4, "Port name", string, "Parent plugins", string, "Hw & SW version", string, "Config dir", string);

struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     // Board revision number
    unsigned hw_version:8;      // Board version number
    unsigned hw_year:16;        // Board year
    unsigned hw_day:8;          // Board day
    unsigned hw_month:8;        // Board month
    unsigned fw_revision:8;     // Firmware revision number
    unsigned fw_version:8;      // Firmware version number
    unsigned fw_year:16;        // Firmware year
    unsigned fw_day:8;          // Firmware day
    unsigned fw_month:8;        // Firmware month
#else
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};

// GCC specific - but very efficient 1 CPU cycle
#define BYTE_SWAP(a) __builtin_bswap32(a)

FemPlugin::FemPlugin(const char *portName, const char *parentPlugins, const char *version, const char *configDir)
    : BaseModulePlugin(portName, parentPlugins, configDir, DasCmdPacket::MOD_TYPE_FEM, 2)
    , m_version(version)
{
    createParam("Upg:File",     asynParamOctet, &UpgradeFile);             // WRITE - Path to the firmware file to be programmed
    createParam("Upg:ChunkSize",asynParamInt32, &UpgradeChunkSize, 256);   // WRITE - Maximum payload size for split program file transfer
    createParam("Upg:Status",   asynParamInt32, &UpgradeStatus);           // READ Firmware update status
    createParam("Upg:ErrorStr", asynParamOctet, &UpgradeErrorStr);         // READ - Error description
    createParam("Upg:Size",     asynParamInt32, &UpgradeSize, 0);          // READ - Total firmware size in bytes
    createParam("Upg:Position", asynParamInt32, &UpgradePosition, 0);      // READ - Bytes already sent to remote porty
    createParam("Upg:Cmd",      asynParamInt32, &UpgradeCmd, 0);           // WRITE - Upgrade command
    createParam("Upg:EraseTimeout",asynParamFloat64, &UpgradeEraseTimeout, 120);   // WRITE - Max time to wait for erased flag
    createParam("Upg:BusyTimeout", asynParamFloat64, &UpgradeBusyTimeout,  1);     // WRITE - Max time to wait for ready flag
    createParam("Upg:ProgramTimeout",asynParamFloat64, &UpgradeProgramTimeout, 10);// WRITE - Max time to wait for programmed flag
    createParam("Upg:NoRspTimeout",asynParamFloat64, &UpgradeNoRspTimeout, 0.2);   // WRITE - Max time to wait for response
    createParam("Upg:NoRspMaxRetries",asynParamInt32,&UpgradeNoRspMaxRetries, 5);     // WRITE - Max times to retry when no response received
    createParam("Upg:NoRspSeqRetries",asynParamInt32,&UpgradeNoRspSeqRetries, 5);  // READ - Number of consecutive retries
    createParam("Upg:NoRspTotRetries",asynParamInt32,&UpgradeNoRspTotRetries, 5);  // READ - Number of all retries in one upgrade session

    m_remoteUpgrade.status = RemoteUpgrade::NOT_SUPPORTED;

    if (m_version == "v32") {
        setIntegerParam(Supported, 1);
        createParams_v32();
        setExpectedVersion(3, 2);
    } else if (m_version == "v34") {
        setIntegerParam(Supported, 1);
        createParams_v32();
        setExpectedVersion(3, 4);
    } else if (m_version == "v35") {
        setIntegerParam(Supported, 1);
        createParams_v35();
        setExpectedVersion(3, 5);
    } else if (m_version == "v36") {
        setIntegerParam(Supported, 1);
        createParams_v36();
        setExpectedVersion(3, 6);
    } else if (m_version == "v37") {
        setIntegerParam(Supported, 1);
        createParams_v37();
        setExpectedVersion(3, 7);
    } else if (m_version == "v38") {
        setIntegerParam(Supported, 1);
        createParams_v38();
        setExpectedVersion(3, 8);
    } else if (m_version == "v39") {
        setIntegerParam(Supported, 1);
        createParams_v39();
        remoteUpgradeSM(RemoteUpgrade::INIT);
        m_cmdHandlers[DasCmdPacket::CMD_UPGRADE].first = std::bind(&FemPlugin::reqUpgrade, this);
        m_cmdHandlers[DasCmdPacket::CMD_UPGRADE].second = std::bind(&FemPlugin::remoteUpgradeSM, this, RemoteUpgrade::PROCESS_RESPONSE, std::placeholders::_1);
        setExpectedVersion(3, 9, 10, 9);
    } else if (m_version == "v310") {
        setIntegerParam(Supported, 1);
        createParams_v310();
        remoteUpgradeSM(RemoteUpgrade::INIT);
        m_cmdHandlers[DasCmdPacket::CMD_UPGRADE].first = std::bind(&FemPlugin::reqUpgrade, this);
        m_cmdHandlers[DasCmdPacket::CMD_UPGRADE].second = std::bind(&FemPlugin::remoteUpgradeSM, this, RemoteUpgrade::PROCESS_RESPONSE, std::placeholders::_1);
        setExpectedVersion(3, 10, 10, 9);
    } else if (m_version == "v320") {
        setIntegerParam(Supported, 1);
        createParams_v320();
        setExpectedVersion(3, 20);
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported FEM version '%s'", version);
        return;
    }
    setStringParam(UpgradeErrorStr, "");

    // FEMs don't support starting/stopping
    m_cmdHandlers[DasCmdPacket::CMD_START].first = std::function<bool()>();
    m_cmdHandlers[DasCmdPacket::CMD_STOP].first  = std::function<bool()>();

    initParams();
}

asynStatus FemPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == UpgradeCmd) {
        if (value == 0) {
            return remoteUpgradeSM(RemoteUpgrade::ABORT) ? asynSuccess : asynError;
        } else if (value == 1) {
            return remoteUpgradeSM(RemoteUpgrade::START) ? asynSuccess : asynError;
        } else {
            return processRequest(DasCmdPacket::CMD_UPGRADE) ? asynSuccess : asynError;
        }
    }
    return BaseModulePlugin::writeInt32(pasynUser, value);
}

asynStatus FemPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    asynStatus ret = BaseModulePlugin::writeOctet(pasynUser, value, nChars, nActual);
    if (pasynUser->reason == UpgradeFile) {
        if (!remoteUpgradeSM(RemoteUpgrade::LOAD_FILE))
            ret = asynError;
    }
    return ret;
}

bool FemPlugin::reqUpgrade()
{
    uint32_t size = m_remoteUpgrade.sendBuf.size();
    char* data = (size > 0 ? m_remoteUpgrade.sendBuf.data() : nullptr);
    sendUpstream(DasCmdPacket::CMD_UPGRADE, 0, (uint32_t*)data, size);
    return true;
}

bool FemPlugin::parseVersionRsp(const DasCmdPacket *packet, BaseModulePlugin::Version &version)
{
    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getCmdPayload());

    if (packet->getCmdPayloadLength() != sizeof(RspReadVersion)) {
        return false;
    }

    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = HEX_BYTE_TO_DEC(response->hw_year >> 8) * 100 + HEX_BYTE_TO_DEC(response->hw_year);
    version.hw_month    = HEX_BYTE_TO_DEC(response->fw_month);
    version.hw_day      = HEX_BYTE_TO_DEC(response->hw_day);
    version.fw_version  = response->fw_version;
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->fw_year >> 8) * 100 + HEX_BYTE_TO_DEC(response->fw_year);
    version.fw_month    = HEX_BYTE_TO_DEC(response->fw_month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->fw_day);
    return true;
}

bool FemPlugin::timerExpired(epicsTimeStamp *timer, int timeoutParam)
{
    double timeout;
    getDoubleParam(timeoutParam, &timeout);
    epicsTimeStamp now;
    epicsTimeGetCurrent(&now);
    return (epicsTimeDiffInSeconds(&now, timer) > timeout);
}

// Big fat state machine
bool FemPlugin::remoteUpgradeSM(RemoteUpgrade::Action action, const DasCmdPacket *packet)
{
    int chunkSize;
    bool ret = true;
    char path[128];

    switch (action) {
    case RemoteUpgrade::INIT:
        // Everyone starts as not supported, when invoked this state changes that
        if (m_remoteUpgrade.status == RemoteUpgrade::NOT_SUPPORTED) {
            m_remoteUpgrade.status = RemoteUpgrade::NOT_READY;
            m_remoteUpgrade.expectedWrCfg = 0;
            m_remoteUpgrade.seqRetries = 0;
            m_remoteUpgrade.totRetries = 0;
            setIntegerParam(UpgradePosition, 0);
            setIntegerParam(UpgradeSize, 0);
            setIntegerParam(UpgradeNoRspSeqRetries, 0);
            setIntegerParam(UpgradeNoRspTotRetries, 0);
        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::LOAD_FILE:
        if (m_remoteUpgrade.status == RemoteUpgrade::NOT_READY ||
            m_remoteUpgrade.status == RemoteUpgrade::LOADED ||
            m_remoteUpgrade.status == RemoteUpgrade::DONE ||
            m_remoteUpgrade.status == RemoteUpgrade::ABORTED ||
            m_remoteUpgrade.status == RemoteUpgrade::ERROR) {

            if (getStringParam(UpgradeFile, sizeof(path), path) != asynSuccess) {
                LOG_ERROR("Upgrade failed - no file specified");
                setStringParam(UpgradeErrorStr, "no file");
                ret = false;
            } else if (m_remoteUpgrade.file.import(path) == true) {
                m_remoteUpgrade.status = RemoteUpgrade::LOADED;
            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade failed - file '%s' can't be loaded", path);
                setStringParam(UpgradeErrorStr, "can't load file");
                ret = false;
            }
            setIntegerParam(UpgradePosition, 0);
            setIntegerParam(UpgradeSize, m_remoteUpgrade.file.getSize());
            setStringParam(UpgradeErrorStr, "");

            LOG_INFO("Upgrade - loaded file %s", path);
        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::START:
        if (m_remoteUpgrade.status == RemoteUpgrade::LOADED ||
            m_remoteUpgrade.status == RemoteUpgrade::DONE ||
            m_remoteUpgrade.status == RemoteUpgrade::ABORTED ||
            m_remoteUpgrade.status == RemoteUpgrade::ERROR) {

            if (m_remoteUpgrade.file.getSize() == 0) {
                LOG_WARN("Upgrade can't start - file not loaded");
                setStringParam(UpgradeErrorStr, "file not loaded");
                ret = false;
                break;
            }
            LOG_INFO("Upgrade - started");

            m_remoteUpgrade.offset = 0;
            m_remoteUpgrade.lastCount = 0;
            m_remoteUpgrade.expectedWrCfg = 0;
            m_remoteUpgrade.seqRetries = 0;
            m_remoteUpgrade.totRetries = 0;
            setIntegerParam(UpgradePosition, 0);
            setIntegerParam(UpgradeNoRspSeqRetries, 0);
            setIntegerParam(UpgradeNoRspTotRetries, 0);

            getIntegerParam(UpgradeChunkSize, &chunkSize);
            chunkSize = (chunkSize < 0) ? 256 : ALIGN_UP(chunkSize, 4);
            if ((uint32_t)chunkSize > m_remoteUpgrade.bufferSize) {
                m_remoteUpgrade.buffer.reset( (char *)malloc(chunkSize) );
                if (!m_remoteUpgrade.buffer) {
                    LOG_ERROR("Upgrade can't start - failed to allocate buffer");
                    setStringParam(UpgradeErrorStr, "buffer alloc failed");
                    m_remoteUpgrade.bufferSize = 0;
                    m_remoteUpgrade.status = RemoteUpgrade::LOADED;
                    ret = false;
                    break;
                }
                m_remoteUpgrade.bufferSize = chunkSize;
            }

            m_remoteUpgrade.expectedWrCfg = 0;
            // Disable and enable remote upgrade support
            // This is a simplification for the state machine, ignoring the response
            for (int i=0; i<=1; i++) {
                if (setIntegerParam("Upg:Enable", i) != asynSuccess) {
                    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                    LOG_ERROR("Upgdrade error - can't find Upg:Enable parameter, module not supported?");
                    setStringParam(UpgradeErrorStr, "not supported");
                    ret = false;
                    break;
                }
                m_remoteUpgrade.expectedWrCfg++;
                processRequest(DasCmdPacket::CMD_WRITE_CONFIG);

                epicsThreadSleep(0.1);
            }

            // Start a periodic timer
            std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeStatusRefresh, this);
            m_remoteUpgrade.statusTimer.schedule(timeoutCb, 1.0);

            epicsTimeGetCurrent(&m_remoteUpgrade.eraseStartTime);
            m_remoteUpgrade.status = RemoteUpgrade::ERASING;

            setStringParam(UpgradeErrorStr, "");

            LOG_INFO("Upgrade - erasing");
        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::PROCESS_RESPONSE:
        // Will set up new timer if necessary
        m_remoteUpgrade.responseTimer.cancel();

        if (packet == 0) {
            LOG_ERROR("Upgrade assert - packet==0");
            setStringParam(UpgradeErrorStr, "empty response");
            ret = false;
            break;
        } else if (packet->getCommand() != DasCmdPacket::CMD_UPGRADE) {
            if (packet->getCommand() == DasCmdPacket::CMD_WRITE_CONFIG && m_remoteUpgrade.expectedWrCfg > 0) {
                // Eat write config responses from enabling/disabling remote upgrade
                m_remoteUpgrade.expectedWrCfg--;
                ret = true;
            } else {
                // Let BaseModuleHandler process it
                ret = false;
            }
            break;
        } else if (rspParams(packet, "UPGRADE") == false) {
            m_remoteUpgrade.status = RemoteUpgrade::ERROR;
            setStringParam(UpgradeErrorStr, "bad response");
            ret = false;
            break;
        }

        if (m_remoteUpgrade.status == RemoteUpgrade::ERASING) {
            m_remoteUpgrade.seqRetries = 0;
            setIntegerParam(UpgradeNoRspSeqRetries, 0);

            // most likely triggered by the background periodic refresher
            int status = remoteUpgradeCheck(RemoteUpgrade::WAIT_ERASED);
            if (status == RemoteUpgrade::DONE) {
                epicsTimeGetCurrent(&m_remoteUpgrade.lastReadyTime);
                m_remoteUpgrade.status = RemoteUpgrade::IN_PROGRESS;

                m_remoteUpgrade.statusTimer.cancel();

                // Sending another status request is redundant, but keeps SM sane
                m_remoteUpgrade.sendBuf.clear();
                processRequest(DasCmdPacket::CMD_UPGRADE);
                m_remoteUpgrade.expectedSeqId = 0; // FEM resets counter to 0 every time upgrade is started

                LOG_INFO("Upgrade - sending data");
            } else if (status == RemoteUpgrade::IN_PROGRESS) {
                // Prevent waiting for 'erased' flag too long
                if (timerExpired(&m_remoteUpgrade.eraseStartTime, UpgradeEraseTimeout)) {
                    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                    LOG_ERROR("Upgrade error - giving up waiting on module 'erased' flag");
                    setStringParam(UpgradeErrorStr, "erase timeout");

                    m_remoteUpgrade.statusTimer.cancel();
                }
            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade error - response check failed");
                setStringParam(UpgradeErrorStr, "remote error");
                m_remoteUpgrade.statusTimer.cancel();
            }
            break;
        } else if (m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS ||
                   m_remoteUpgrade.status == RemoteUpgrade::WAITING) {

            // Do all flags check out?
            int status = remoteUpgradeCheck(RemoteUpgrade::WAIT_READY);
            if (status == RemoteUpgrade::DONE) {
                int seqId;
                int maxRetries;
                getIntegerParam("Upg:SeqId", seqId);
                getIntegerParam(UpgradeNoRspMaxRetries, &maxRetries);

                // Switch from potential waiting state
                m_remoteUpgrade.status = RemoteUpgrade::IN_PROGRESS;

                epicsTimeGetCurrent(&m_remoteUpgrade.lastReadyTime);

                if ((unsigned)seqId == m_remoteUpgrade.expectedSeqId) {
                    m_remoteUpgrade.offset += m_remoteUpgrade.lastCount;
                    m_remoteUpgrade.expectedSeqId = (++seqId % 256);
                    m_remoteUpgrade.seqRetries = 0;
                    setIntegerParam(UpgradeNoRspSeqRetries, 0);
                } else if (m_remoteUpgrade.seqRetries < (unsigned)maxRetries) {
                    m_remoteUpgrade.seqRetries++;
                    m_remoteUpgrade.totRetries++;
                    setIntegerParam(UpgradeNoRspSeqRetries, m_remoteUpgrade.seqRetries);
                    setIntegerParam(UpgradeNoRspTotRetries, m_remoteUpgrade.totRetries);
                } else {
                    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                    LOG_ERROR("Upgrade error - giving up sending same chunk of data after %d retries", maxRetries);
                    setStringParam(UpgradeErrorStr, "seq not incrementing");
                    ret = true;
                    break;
                }

                getIntegerParam(UpgradeChunkSize, &chunkSize);
                chunkSize = ALIGN_UP(chunkSize, 4);
                m_remoteUpgrade.lastCount = m_remoteUpgrade.file.read(m_remoteUpgrade.offset,
                                                                      m_remoteUpgrade.buffer.get(),
                                                                      chunkSize);
                if (m_remoteUpgrade.lastCount == 0) {
                    // Done with the data, transition to FINALIZING state
                    m_remoteUpgrade.status = RemoteUpgrade::FINALIZING;
                    epicsTimeGetCurrent(&m_remoteUpgrade.disableTime);

                    // Start a periodic timer
                    std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeStatusRefresh, this);
                    m_remoteUpgrade.statusTimer.schedule(timeoutCb, 1.0);

                    LOG_INFO("Upgrade - programming");
                    break;
                }

                // MCS is in big endian, convert to little endian
                uint32_t *data = (uint32_t *)m_remoteUpgrade.buffer.get();
                for (uint32_t i=0; i<m_remoteUpgrade.lastCount/4; i++) {
                    data[i] = BYTE_SWAP(data[i]);
                }

                LOG_DEBUG("Upgrade progress, sending firmware chunk size %u at offset %u", m_remoteUpgrade.lastCount, m_remoteUpgrade.offset);
                m_remoteUpgrade.sendBuf.clear();
                for (size_t i = 0; i < m_remoteUpgrade.lastCount; i++) {
                    m_remoteUpgrade.sendBuf.push_back(m_remoteUpgrade.buffer.get()[i]);
                }
                processRequest(DasCmdPacket::CMD_UPGRADE);

                // A packet was sent out, setup a timeout timer
                double timeout;
                getDoubleParam(UpgradeNoRspTimeout, &timeout);
                std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
                m_remoteUpgrade.responseTimer.schedule(timeoutCb, timeout);

            } else if (status == RemoteUpgrade::IN_PROGRESS) {
                m_remoteUpgrade.seqRetries = 0;
                setIntegerParam(UpgradeNoRspSeqRetries, 0);

                // Prevent waiting for 'ready' flag too long
                if (timerExpired(&m_remoteUpgrade.lastReadyTime, UpgradeBusyTimeout)) {
                    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                    LOG_ERROR("Upgrade error - giving up waiting on module 'ready' flag");
                    setStringParam(UpgradeErrorStr, "ready timeout");
                    ret = true;
                    break;
                }

                m_remoteUpgrade.status = RemoteUpgrade::WAITING;

                // Send a silent request to retrieve the status again
                LOG_DEBUG("Upgrade busy, sending upgrade status refresh request");
                m_remoteUpgrade.sendBuf.clear();
                processRequest(DasCmdPacket::CMD_UPGRADE);

                // A packet was sent out, setup a timeout timer
                double timeout;
                getDoubleParam(UpgradeNoRspTimeout, &timeout);
                std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
                m_remoteUpgrade.responseTimer.schedule(timeoutCb, timeout);

            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade error - response check failed");
                setStringParam(UpgradeErrorStr, "remote error");
                ret = true;
            }
        } else if (m_remoteUpgrade.status == RemoteUpgrade::FINALIZING) {
            m_remoteUpgrade.seqRetries = 0;
            setIntegerParam(UpgradeNoRspSeqRetries, 0);

            // most likely triggered by the background periodic refresher
            int status = remoteUpgradeCheck(RemoteUpgrade::WAIT_PROGRAMMED);
            if (status == RemoteUpgrade::DONE) {
                m_remoteUpgrade.status = RemoteUpgrade::DONE;

                m_remoteUpgrade.statusTimer.cancel();

                // Disable remote upgrade support
                if (setIntegerParam("Upg:Enable", 0) != asynSuccess) {
                    LOG_ERROR("Upgdrade error - can't find Upg:Enable parameter, module not supported?");
                    setStringParam(UpgradeErrorStr, "not supported");
                } else {
                    // Push new configuration, but don't refresh upgrade status
                    // as it clears most registers
                    m_remoteUpgrade.expectedWrCfg++;
                    processRequest(DasCmdPacket::CMD_WRITE_CONFIG);
                }

                LOG_INFO("Upgrade - complete");

            } else if (status == RemoteUpgrade::IN_PROGRESS) {
                // Prevent waiting for 'ready' flag too long
                if (timerExpired(&m_remoteUpgrade.disableTime, UpgradeProgramTimeout)) {
                    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                    LOG_ERROR("Upgrade error - giving up waiting on module 'programmed' flag");
                    setStringParam(UpgradeErrorStr, "program timeout");

                    m_remoteUpgrade.statusTimer.cancel();
                }
            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade error - response check failed");
                setStringParam(UpgradeErrorStr, "remote error");
                m_remoteUpgrade.statusTimer.cancel();
            }
            break;
        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            // We're not handling the packet, flag so with return false
            ret = false;
        }
        break;
    case RemoteUpgrade::ABORT:
        if (m_remoteUpgrade.status == RemoteUpgrade::ERASING ||
            m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS ||
            m_remoteUpgrade.status == RemoteUpgrade::WAITING ||
            m_remoteUpgrade.status == RemoteUpgrade::FINALIZING) {

            m_remoteUpgrade.responseTimer.cancel();
            m_remoteUpgrade.statusTimer.cancel();
            m_remoteUpgrade.status = RemoteUpgrade::ABORTED;

            LOG_INFO("Upgrade - user aborted");

        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::TIMEOUT:
        if (m_remoteUpgrade.status == RemoteUpgrade::ERASING ||
            m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS ||
            m_remoteUpgrade.status == RemoteUpgrade::WAITING ||
            m_remoteUpgrade.status == RemoteUpgrade::FINALIZING) {

            m_remoteUpgrade.responseTimer.cancel();

            int maxRetries;
            getIntegerParam(UpgradeNoRspMaxRetries, &maxRetries);

            if (m_remoteUpgrade.seqRetries >= (unsigned)maxRetries) {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade error - too many retries sending same chunk of data");
                setStringParam(UpgradeErrorStr, "too many retries");
            } else {
                // Refresh status to see what packet has been received,
                // rest of SM will take care of that
                m_remoteUpgrade.sendBuf.clear();
                processRequest(DasCmdPacket::CMD_UPGRADE);
                m_remoteUpgrade.seqRetries++;
                m_remoteUpgrade.totRetries++;
                setIntegerParam(UpgradeNoRspSeqRetries, m_remoteUpgrade.seqRetries);
                setIntegerParam(UpgradeNoRspTotRetries, m_remoteUpgrade.totRetries);

                // A packet was sent out, setup a timeout timer
                double timeout;
                getDoubleParam(UpgradeNoRspTimeout, &timeout);
                std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
                m_remoteUpgrade.responseTimer.schedule(timeoutCb, timeout);
            }

        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    }
    setIntegerParam(UpgradePosition, m_remoteUpgrade.offset);
    setIntegerParam(UpgradeStatus, m_remoteUpgrade.status);
    callParamCallbacks();
    return ret;
}

FemPlugin::RemoteUpgrade::Status FemPlugin::remoteUpgradeCheck(RemoteUpgrade::CheckType type)
{
    int param;

    // This is all very FEM 3.9 specific
    getIntegerParam("Upg:OutError",            param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutError",            param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorIdCode",      param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorErase",       param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorProgram",     param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorCrc",         param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorBlockLocked", param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorVPP",         param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorCmdSeq",      param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorTimeOut",     param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:FifoFull",            param); if (param == 1) return RemoteUpgrade::ERROR;
    if (type == RemoteUpgrade::WAIT_ERASED) {
        getIntegerParam("Upg:OutErase",        param); if (param == 0) return RemoteUpgrade::IN_PROGRESS;
    } else if (type == RemoteUpgrade::WAIT_READY) {
        getIntegerParam("Upg:Ready",           param); if (param == 0) return RemoteUpgrade::IN_PROGRESS;
        getIntegerParam("Upg:FifoProgFull",    param); if (param == 1) return RemoteUpgrade::IN_PROGRESS;
    } else if (type == RemoteUpgrade::WAIT_PROGRAMMED) {
        getIntegerParam("Upg:OutProgram",      param); if (param == 0) return RemoteUpgrade::IN_PROGRESS;
        getIntegerParam("Upg:OutVerifyOK",     param); if (param == 0) return RemoteUpgrade::IN_PROGRESS;
    }

    return RemoteUpgrade::DONE;
}

float FemPlugin::remoteUpgradeTimeout()
{
    lock();
    remoteUpgradeSM(RemoteUpgrade::TIMEOUT);
    unlock();
    return 0.0;
}

float FemPlugin::remoteUpgradeStatusRefresh()
{
    lock();

    m_remoteUpgrade.sendBuf.clear();
    processRequest(DasCmdPacket::CMD_UPGRADE);

    // A packet was sent out, setup a timeout timer
    double timeout;
    getDoubleParam(UpgradeNoRspTimeout, &timeout);
    std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
    m_remoteUpgrade.responseTimer.schedule(timeoutCb, timeout);

    unlock();

    return 1.0;
}
