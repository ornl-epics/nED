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

#define NUM_FEMPLUGIN_PARAMS    ((int)(&LAST_FEMPLUGIN_PARAM - &FIRST_FEMPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(FemPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned FemPlugin::NUM_FEMPLUGIN_DYNPARAMS       = 320; // MAX(`for file in FemPlugin_v3*; do grep create $file | grep Param | wc -l; done`)

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

FemPlugin::FemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_FEM, true,
                       blocking, NUM_FEMPLUGIN_PARAMS + NUM_FEMPLUGIN_DYNPARAMS)
    , m_version(version)
{
    createParam("Upg:File",     asynParamOctet, &UpgradeFile);             // WRITE - Path to the firmware file to be programmed
    createParam("Upg:ChunkSize",asynParamInt32, &UpgradeChunkSize, 256);   // WRITE - Maximum payload size for split program file transfer
    createParam("Upg:Status",   asynParamInt32, &UpgradeStatus);           // READ Firmware update status
    createParam("Upg:Size",     asynParamInt32, &UpgradeSize, 0);          // READ - Total firmware size in bytes
    createParam("Upg:Position", asynParamInt32, &UpgradePosition, 0);      // READ - Bytes already sent to remote porty
    createParam("Upg:Cmd",      asynParamInt32, &UpgradeCmd, 0);           // WRITE - Upgrade command

    m_remoteUpgrade.status = RemoteUpgrade::NOT_SUPPORTED;

    if (m_version == "v32" || m_version == "v34") {
        setIntegerParam(Supported, 1);
        createParams_v32();
    } else if (m_version == "v35") {
        setIntegerParam(Supported, 1);
        createParams_v35();
    } else if (m_version == "v36") {
        setIntegerParam(Supported, 1);
        createParams_v36();
    } else if (m_version == "v37") {
        setIntegerParam(Supported, 1);
        createParams_v37();
    } else if (m_version == "v38") {
        setIntegerParam(Supported, 1);
        createParams_v38();
    } else if (m_version == "v39") {
        setIntegerParam(Supported, 1);
        createParams_v39();
        remoteUpgradeSM(RemoteUpgrade::INIT);
    } else if (m_version == "v320") {
        setIntegerParam(Supported, 1);
        createParams_v320();
    } else {
        setIntegerParam(Supported, 0);
        LOG_ERROR("Unsupported FEM version '%s'", version);
    }

    callParamCallbacks();
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
            return processRequest(DasPacket::CMD_UPGRADE) ? asynSuccess : asynError;
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

bool FemPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getPayload());

    if (packet->getPayloadLength() != sizeof(RspReadVersion)) {
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

bool FemPlugin::checkVersion(const BaseModulePlugin::Version &version)
{
    if ((version.hw_version == 10 && version.hw_revision == 2) ||
        (version.hw_version == 10 && version.hw_revision == 9)) {
        char ver[10];
        snprintf(ver, sizeof(ver), "v%u%u", version.fw_version, version.fw_revision);
        if (m_version == ver)
            return true;
    }

    return false;
}

bool FemPlugin::remoteUpgradeRsp(const DasPacket *packet)
{
    return remoteUpgradeSM(RemoteUpgrade::PROCESS_RESPONSE, packet);
}

// Big fat state machine
bool FemPlugin::remoteUpgradeSM(RemoteUpgrade::Action action, const DasPacket *packet)
{
    int chunkSize;
    bool ret = true;
    char path[128];

    switch (action) {
    case RemoteUpgrade::INIT:
        if (m_remoteUpgrade.status == RemoteUpgrade::NOT_SUPPORTED) {
            std::function<bool(const DasPacket *)> handler = std::bind(&FemPlugin::remoteUpgradeSM, this, RemoteUpgrade::PROCESS_RESPONSE, std::placeholders::_1);
            registerResponseHandler(handler);

            m_remoteUpgrade.status = RemoteUpgrade::NOT_READY;
            setIntegerParam(UpgradePosition, 0);
            setIntegerParam(UpgradeSize, 0);
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
                ret = false;
            } else if (m_remoteUpgrade.file.import(path) == true) {
                m_remoteUpgrade.status = RemoteUpgrade::LOADED;
            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                ret = false;
            }
            setIntegerParam(UpgradePosition, 0);
            setIntegerParam(UpgradeSize, m_remoteUpgrade.file.getSize());
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
                ret = false;
                break;
            }

            setIntegerParam(UpgradePosition, 0);
            m_remoteUpgrade.offset = 0;
            m_remoteUpgrade.lastCount = 0;

            getIntegerParam(UpgradeChunkSize, &chunkSize);
            chunkSize = (chunkSize < 0) ? 256 : ALIGN_UP(chunkSize, 4);
            if ((uint32_t)chunkSize > m_remoteUpgrade.bufferSize) {
                m_remoteUpgrade.buffer.reset( (char *)malloc(chunkSize) );
                if (!m_remoteUpgrade.buffer) {
                    LOG_ERROR("Upgrade can't start - failed to allocate buffer");
                    m_remoteUpgrade.bufferSize = 0;
                    m_remoteUpgrade.status = RemoteUpgrade::LOADED;
                    ret = false;
                    break;
                }
                m_remoteUpgrade.bufferSize = chunkSize;
            }

            m_remoteUpgrade.status = RemoteUpgrade::IN_PROGRESS;

            reqUpgrade();
            epicsTimeGetCurrent(&m_remoteUpgrade.lastReadyTime);
            LOG_INFO("Upgrade started");
        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::PROCESS_RESPONSE:
        if (m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS ||
            m_remoteUpgrade.status == RemoteUpgrade::WAITING) {

            if (packet == 0) {
                LOG_ERROR("Upgrade assert - packet==0");
                ret = false;
                break;
            } else if (packet->getResponseType() != DasPacket::CMD_UPGRADE) {
                // Silently skip other packets, too much chatter otherwise
                ret = false;
                break;
            } else if (rspUpgrade(packet) == false) {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                ret = false;
                break;
            }

            // Will set up new timer if necessary
            if (m_remoteUpgrade.timer) {
                m_remoteUpgrade.timer->cancel();
                m_remoteUpgrade.timer.reset();
            }

            // Do all flags check out?
            int status = remoteUpgradeCheck();
            if (status == RemoteUpgrade::DONE) {

                // Switch from potential waiting state
                m_remoteUpgrade.status = RemoteUpgrade::IN_PROGRESS;

                epicsTimeGetCurrent(&m_remoteUpgrade.lastReadyTime);

                m_remoteUpgrade.offset += m_remoteUpgrade.lastCount;

                getIntegerParam(UpgradeChunkSize, &chunkSize);
                chunkSize = ALIGN_UP(chunkSize, 4);
                m_remoteUpgrade.lastCount = m_remoteUpgrade.file.read(m_remoteUpgrade.offset,
                                                                      m_remoteUpgrade.buffer.get(),
                                                                      chunkSize);
                if (m_remoteUpgrade.lastCount == 0) {
                    m_remoteUpgrade.status = RemoteUpgrade::DONE;
                    break;
                }

                // MCS is in big endian, convert to little endian
                uint32_t *data = (uint32_t *)m_remoteUpgrade.buffer.get();
                for (uint32_t i=0; i<m_remoteUpgrade.lastCount/4; i++) {
                    data[i] = BYTE_SWAP(data[i]);
                }

                LOG_DEBUG("Upgrade progress, sending firmware chunk size %u at offset %u", m_remoteUpgrade.lastCount, m_remoteUpgrade.offset);
                reqUpgrade(m_remoteUpgrade.buffer.get(), m_remoteUpgrade.lastCount);

                // A packet was sent out, setup a timeout timer
                double timeout;
                getDoubleParam(NoRspTimeout, &timeout);
                std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
                m_remoteUpgrade.timer = scheduleCallback(timeoutCb, timeout);

            } else if (status == RemoteUpgrade::IN_PROGRESS) {
                // Prevent waiting for 'ready' flag too long
                epicsTimeStamp now;
                epicsTimeGetCurrent(&now);
                if (epicsTimeDiffInSeconds(&now, &m_remoteUpgrade.lastReadyTime) > 1.0) {
                    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                    LOG_ERROR("Upgrade error - giving up waiting on module 'ready' flag");
                    // Response handled, return true
                    break;
                }

                m_remoteUpgrade.status = RemoteUpgrade::WAITING;

                // Send a silent request to retrieve the status again
                LOG_DEBUG("Upgrade busy, sending upgrade status refresh request");
                reqUpgrade();

                // A packet was sent out, setup a timeout timer
                double timeout;
                getDoubleParam(NoRspTimeout, &timeout);
                std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
                m_remoteUpgrade.timer = scheduleCallback(timeoutCb, timeout);

            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade error - response check failed");
                ret = false;
            }
        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::ABORT:
        if (m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS ||
            m_remoteUpgrade.status == RemoteUpgrade::WAITING) {

            if (m_remoteUpgrade.timer) {
                m_remoteUpgrade.timer->cancel();
                m_remoteUpgrade.timer.reset();
            }
            m_remoteUpgrade.status = RemoteUpgrade::ABORTED;

        } else {
            LOG_DEBUG("Upgrade SM action ignore - state=%d, action=%d", m_remoteUpgrade.status, action);
            ret = false;
        }
        break;
    case RemoteUpgrade::TIMEOUT:
        if (m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS ||
            m_remoteUpgrade.status == RemoteUpgrade::WAITING) {

            if (m_remoteUpgrade.timer) {
                m_remoteUpgrade.timer->cancel();
                m_remoteUpgrade.timer.reset();
            }

            m_remoteUpgrade.status = RemoteUpgrade::ERROR;
            LOG_ERROR("Upgrade failed - didn't receive response in time");

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

FemPlugin::RemoteUpgrade::Status FemPlugin::remoteUpgradeCheck()
{
    int param;

    // This is all very FEM 3.9 specific
    getIntegerParam("Upg:OutError",            &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutError",            &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorIdCode",      &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorErase",       &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorProgram",     &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorCrc",         &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorBlockLocked", &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorVPP",         &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorCmdSeq",      &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:OutErrorTimeOut",     &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:FifoFull",            &param); if (param == 1) return RemoteUpgrade::ERROR;
    getIntegerParam("Upg:Ready",               &param); if (param == 0) return RemoteUpgrade::IN_PROGRESS;
    getIntegerParam("Upg:FifoProgFull",        &param); if (param == 1) return RemoteUpgrade::IN_PROGRESS;

    return RemoteUpgrade::DONE;
}

float FemPlugin::remoteUpgradeTimeout()
{
    remoteUpgradeSM(RemoteUpgrade::TIMEOUT);
    return 0.0;
}
