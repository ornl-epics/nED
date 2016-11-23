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

const unsigned FemPlugin::NUM_FEMPLUGIN_DYNPARAMS       = 290; // MAX(`for file in FemPlugin_v3*; do grep create $file | grep Param | wc -l; done`)

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

FemPlugin::FemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, DasPacket::MOD_TYPE_FEM, true,
                       blocking, NUM_FEMPLUGIN_PARAMS + NUM_FEMPLUGIN_DYNPARAMS)
    , m_version(version)
{
    createParam("UpgradeFile",      asynParamOctet, &UpgradeFile);               // WRITE - Path to the firmware file to be programmed
    createParam("UpdataChunkSize",  asynParamInt32,&UpgradeChunkSize, 256);    // WRITE - Maximum payload size for split program file transfer
    createParam("UpgradeStatus",    asynParamInt32, &UpgradeStatus);             // READ Firmware update status
    createParam("UpgradeSize",      asynParamInt32, &UpgradeSize, 0);            // READ - Total firmware size in bytes
    createParam("UpgradePosition",  asynParamInt32, &UpgradePosition, 0);      // READ - Bytes already sent to remote porty
    createParam("UpgradeAbort",     asynParamInt32, &UpgradeCmd, 0);             // WRITE - Upgrade command

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
        remoteUpgradeInit();
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
    if (pasynUser->reason == UpgradeChunkSize) {
        // Enforce 4 bytes aligned transfers
        uint32_t newSize = ALIGN_UP(value, 4);

        if (m_remoteUpgrade.bufferSize >= newSize) {
            m_remoteUpgrade.bufferSize = newSize;
        } else {
            char *buffer = (char *)malloc(newSize);
            if (buffer == 0)
                return asynError;
            m_remoteUpgrade.buffer.reset(buffer);
            m_remoteUpgrade.bufferSize = newSize;
        }

        setIntegerParam(UpgradeChunkSize, newSize);
        callParamCallbacks();
        return asynSuccess;
    }
    if (pasynUser->reason == UpgradeCmd) {
        if (m_remoteUpgrade.status == RemoteUpgrade::NOT_SUPPORTED) {
            LOG_WARN("Remote upgrade not supported by module");
            return asynError;
        }
        if (value == 0) {
            remoteUpgradeAbort(true);
        } else {
            m_remoteUpgrade.status = RemoteUpgrade::IN_PROGRESS;
            remoteUpgradeRsp(0);
        }
        return asynSuccess;
    }
    return BaseModulePlugin::writeInt32(pasynUser, value);
}

asynStatus FemPlugin::writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
    if (pasynUser->reason == UpgradeFile) {
        remoteUpgradeLoad(std::string(value, nChars));
        *nActual = nChars;
        return asynSuccess;
    }
    return BaseModulePlugin::writeOctet(pasynUser, value, nChars, nActual);
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

void FemPlugin::remoteUpgradeInit()
{
    int chunkSize;
    getIntegerParam(UpgradeChunkSize, &chunkSize);
    chunkSize = ALIGN_UP(chunkSize, 4);

    m_remoteUpgrade.offset = 0;
    m_remoteUpgrade.lastCount = 0;
    m_remoteUpgrade.buffer.reset( (char *)malloc(chunkSize) );

    if (!m_remoteUpgrade.buffer) {
        LOG_ERROR("Not enabling remote update - failed to allocate packet");
        m_remoteUpgrade.status = RemoteUpgrade::NOT_READY;
        m_remoteUpgrade.bufferSize = 0;
    } else {
        m_remoteUpgrade.status = RemoteUpgrade::READY;
        m_remoteUpgrade.bufferSize = chunkSize;
    }

    setIntegerParam(UpgradeStatus, m_remoteUpgrade.status);
    setIntegerParam(UpgradePosition, 0);
    setIntegerParam(UpgradeSize, 0);
    callParamCallbacks();

    std::function<bool(const DasPacket *)> handler = std::bind(&FemPlugin::remoteUpgradeRsp, this, std::placeholders::_1);
    registerResponseHandler(handler);
}

void FemPlugin::remoteUpgradeLoad(const std::string &path)
{
    switch (m_remoteUpgrade.status) {
    case RemoteUpgrade::READY:
    case RemoteUpgrade::LOADED:
    case RemoteUpgrade::DONE:
    case RemoteUpgrade::ERROR:
    case RemoteUpgrade::ABORTED:
        if (m_remoteUpgrade.file.import(path) == true) {
            m_remoteUpgrade.status = RemoteUpgrade::LOADED;
        } else {
            m_remoteUpgrade.status = RemoteUpgrade::ERROR;
        }
        setIntegerParam(UpgradeStatus, m_remoteUpgrade.status);
        setIntegerParam(UpgradePosition, 0);
        setIntegerParam(UpgradeSize, m_remoteUpgrade.file.getSize());
        callParamCallbacks();
        break;
    default:
        break;
    }
}

bool FemPlugin::remoteUpgradeRsp(const DasPacket *packet)
{
    if (packet && packet->getResponseType() != DasPacket::CMD_UPGRADE)
        return false;
    if (!packet && m_remoteUpgrade.status == RemoteUpgrade::LOADED)
        m_remoteUpgrade.status = RemoteUpgrade::IN_PROGRESS;
    if (m_remoteUpgrade.status != RemoteUpgrade::IN_PROGRESS)
        return false;

    // Will set up new timer if necessary
    if (m_remoteUpgrade.timer) {
        m_remoteUpgrade.timer->cancel();
        m_remoteUpgrade.timer.reset();
    }

    // do..while() trick used as exit point
    do {
        if (packet) {
            // Unpack registers
            if (rspUpgrade(packet) == false) {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                break;
            }

            // Do all flags check out?
            int status = remoteUpgradeCheck();
            if (status == RemoteUpgrade::DONE) {
                m_remoteUpgrade.offset += m_remoteUpgrade.lastCount;
                m_remoteUpgrade.busyRetries = 0;
            } else if (status == RemoteUpgrade::IN_PROGRESS) {
                // TODO: need to stop at some point
                m_remoteUpgrade.busyRetries++;
                // Send an empty packet to retrieve the status again
                // TODO: delay?
                sendToDispatcher(DasPacket::CMD_UPGRADE);
                break;
            } else {
                m_remoteUpgrade.status = RemoteUpgrade::ERROR;
                LOG_ERROR("Upgrade error - response check failed");
                break;
            }
        }

        m_remoteUpgrade.lastCount = m_remoteUpgrade.file.read(m_remoteUpgrade.offset,
                                                              m_remoteUpgrade.buffer.get(),
                                                              m_remoteUpgrade.bufferSize);
        if (m_remoteUpgrade.lastCount > 0) {
            sendToDispatcher(DasPacket::CMD_UPGRADE, 0, (uint32_t*)m_remoteUpgrade.buffer.get(), m_remoteUpgrade.lastCount);
            double timeout;
            getDoubleParam(NoRspTimeout, &timeout);
            std::function<float(void)> timeoutCb = std::bind(&FemPlugin::remoteUpgradeTimeout, this);
            m_remoteUpgrade.timer = scheduleCallback(timeoutCb, timeout);
        } else {
            m_remoteUpgrade.status = RemoteUpgrade::DONE;
        }
    } while (0);

    setIntegerParam(UpgradeStatus, m_remoteUpgrade.status);
    setIntegerParam(UpgradePosition, m_remoteUpgrade.offset);
    callParamCallbacks();

    return true;
}
FemPlugin::RemoteUpgrade::Status FemPlugin::remoteUpgradeCheck()
{
    int param;

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
    getIntegerParam("Upg:Busy",                &param); if (param == 1) return RemoteUpgrade::IN_PROGRESS;

    return RemoteUpgrade::DONE;
}

void FemPlugin::remoteUpgradeAbort(bool userAbort)
{
    if (m_remoteUpgrade.status == RemoteUpgrade::IN_PROGRESS) {
        if (userAbort)
            m_remoteUpgrade.status = RemoteUpgrade::ABORTED;
        else
            m_remoteUpgrade.status = RemoteUpgrade::ERROR;

        setIntegerParam(UpgradeStatus, m_remoteUpgrade.status);
        callParamCallbacks();
    }
}

float FemPlugin::remoteUpgradeTimeout()
{
    m_remoteUpgrade.status = RemoteUpgrade::ERROR;
    setIntegerParam(UpgradeStatus, m_remoteUpgrade.status);
    callParamCallbacks();
    LOG_ERROR("Remote upgrade failed - didn't receive response in time");
    return 0.0;
}
