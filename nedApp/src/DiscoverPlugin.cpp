/* DiscoverPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DiscoverPlugin.h"
#include "DspPlugin.h"
#include "Log.h"
#include "RocPlugin.h"

#include <cstring>
#include <sstream>

#define NUM_DISCOVERPLUGIN_PARAMS ((int)(&LAST_DISCOVERPLUGIN_PARAM - &FIRST_DISCOVERPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(DiscoverPlugin, 2, "Port name", string, "Dispatcher port name", string);

DiscoverPlugin::DiscoverPlugin(const char *portName, const char *dispatcherPortName)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, 1, NUM_DISCOVERPLUGIN_PARAMS, 0,
                 BasePlugin::defaultInterfaceMask | asynOctetMask, BasePlugin::defaultInterruptMask | asynOctetMask)
{
    createParam("Trigger",      asynParamInt32, &Trigger);      // WRITE - Trigger discovery of modules
    createParam("Format",       asynParamInt32, &Format);       // READ - Modules found formatted in ASCII table
    createParam("Discovered",   asynParamInt32, &Discovered);   // READ - Modules found formatted in ASCII table
    createParam("Verified",     asynParamInt32, &Verified);     // READ - Modules found formatted in ASCII table
    createParam("Output",       asynParamOctet, &Output, "Not initialized");    // READ - Modules found formatted in ASCII table
    callParamCallbacks();
}

asynStatus DiscoverPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Trigger) {
        setIntegerParam(Discovered, 0);
        setIntegerParam(Verified, 0);
        callParamCallbacks();
        
        m_discovered.clear();
        reqDiscover(DasPacket::HWID_BROADCAST);
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void DiscoverPlugin::processData(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nDiscovered = 0;
    int nVerified = 0;
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(Discovered, &nDiscovered);
    getIntegerParam(Verified,   &nVerified);

    nReceived += packetList->size();

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        // Silently skip packets we're not interested in
        if (!packet->isResponse())
            continue;

        if (packet->cmdinfo.command == DasPacket::CMD_DISCOVER) {
            if (packet->cmdinfo.module_type == DasPacket::MOD_TYPE_DSP) {
                // DSP responds with a list of modules it knows about in the payload.
                // It appears that only DSPs will respond to a broadcast address and from
                // their responses all their submodules can be observed. Since we're
                // also interested in module types, we'll do a p2p discover to every module.
                m_discovered[packet->source].type = DasPacket::MOD_TYPE_DSP;
                reqVersion(packet->getSourceAddress());

                nDiscovered++;
            
                // The global LVDS discover packet should address all modules connected
                // through LVDS. For some unidentified reason, ROC boards connected directly
                // to DSP don't respond, whereas ROCs behind FEM do.
                // So we do P2P to each module.
                for (uint32_t i=0; i<packet->payload_length/sizeof(uint32_t); i++) {
                    nDiscovered++;
                    m_discovered[packet->payload[i]].parent = packet->getRouterAddress();
                    reqLvdsDiscover(packet->payload[i]);
                    reqLvdsVersion(packet->payload[i]);
                }
            } else if (packet->cmdinfo.is_passthru) {
                // Source hardware id belongs to the DSP, the actual module id is in payload
                m_discovered[packet->getSourceAddress()].type = packet->cmdinfo.module_type;
            } else {
                m_discovered[packet->getSourceAddress()].type = packet->cmdinfo.module_type;
            }
            nProcessed++;
        } else if (packet->cmdinfo.command == DasPacket::CMD_READ_VERSION) {
            uint32_t source = packet->getSourceAddress();
            if (m_discovered.find(source) != m_discovered.end()) {
                switch (m_discovered[source].type) {
                case DasPacket::MOD_TYPE_DSP:
                    DspPlugin::parseVersionRsp(packet, m_discovered[source].version);
                    break;
                case DasPacket::MOD_TYPE_ROC:
                    RocPlugin::parseVersionRsp(packet, m_discovered[source].version);
                    break;
                default:
                    break;
                }
            }
            nVerified++;
            nProcessed++;
        }
    }

    setIntegerParam(RxCount,    nReceived);
    setIntegerParam(ProcCount,  nProcessed);
    setIntegerParam(Discovered, nDiscovered);
    setIntegerParam(Verified,   nVerified);
    callParamCallbacks();
}

uint32_t DiscoverPlugin::formatOutput(char *buffer, uint32_t size)
{
    int ret;
    int length = size;
    uint32_t i = 1;

    ret = snprintf(buffer, length, "Discovered modules:\n");
    if (ret > length)
        return 0;
    length -= ret;
    buffer += ret;

    for (std::map<uint32_t, ModuleDesc>::iterator it = m_discovered.begin(); it != m_discovered.end(); it++, i++) {
        std::string moduleId(BaseModulePlugin::addr2ip(it->first));
        std::string parentId(BaseModulePlugin::addr2ip(it->second.parent));
        const char *type;
        BaseModulePlugin::Version version = it->second.version;

        switch (it->second.type) {
            case DasPacket::MOD_TYPE_ACPC:      type = "ACPC";      break;
            case DasPacket::MOD_TYPE_ACPCFEM:   type = "ACPC FEM";  break;
            case DasPacket::MOD_TYPE_AROC:      type = "AROC";      break;
            case DasPacket::MOD_TYPE_BIDIMROC:  type = "BIDIMROC";  break;
            case DasPacket::MOD_TYPE_BLNROC:    type = "BLNROC";    break;
            case DasPacket::MOD_TYPE_CROC:      type = "CROC";      break;
            case DasPacket::MOD_TYPE_DSP:       type = "DSP";       break;
            case DasPacket::MOD_TYPE_FFC:       type = "FFC";       break;
            case DasPacket::MOD_TYPE_FEM:       type = "FEM";       break;
            case DasPacket::MOD_TYPE_HROC:      type = "HROC";      break;
            case DasPacket::MOD_TYPE_IROC:      type = "IROC";      break;
            case DasPacket::MOD_TYPE_ROC:       type = "ROC";       break;
            case DasPacket::MOD_TYPE_SANSROC:   type = "SANSROC";   break;
            default:                            type = "unknown";
        }

        if (it->second.parent != 0) {
            ret = snprintf(buffer, length,
                           "  %3u %-8s: %-15s ver %d.%d/%d.%d date %.04d/%.02d/%.02d (DSP=%s)\n",
                           i, type, moduleId.c_str(), version.hw_version, version.hw_revision,
                           version.fw_version, version.fw_revision, version.fw_year,
                           version.fw_month, version.fw_day, parentId.c_str());
        } else {
            ret = snprintf(buffer, length,
                           "  %3u %-8s: %-15s ver %d.%d/%d.%d date %.04d/%.02d/%.02d\n",
                           i, type, moduleId.c_str(), version.hw_version, version.hw_revision,
                           version.fw_version, version.fw_revision, version.fw_year,
                           version.fw_month, version.fw_day);
        }
        if (ret == -1 || ret > length)
            break;

        length -= ret;
        buffer += ret;
    }
    return (size - length);
}

uint32_t DiscoverPlugin::formatSubstitution(char *buffer, uint32_t size)
{
    int ret;
    int length = size;
    uint32_t i = 1;
    
    std::map<std::string, uint32_t> ids;

    for (std::map<uint32_t, ModuleDesc>::iterator it = m_discovered.begin(); it != m_discovered.end(); it++, i++) {
        std::string moduleId(BaseModulePlugin::addr2ip(it->first));
        const char *plugin;
        const char *type;
        BaseModulePlugin::Version version = it->second.version;

        switch (it->second.type) {
            case DasPacket::MOD_TYPE_ACPCFEM:   plugin = "AcpcFemPlugin";   type = "afem";    break;
            case DasPacket::MOD_TYPE_ACPC:      plugin = "AcpcPlugin";      type = "acpc";    break;
            case DasPacket::MOD_TYPE_DSP:       plugin = "DspPlugin";       type = "dsp";     break;
            case DasPacket::MOD_TYPE_FEM:       plugin = "FemPlugin";       type = "fem";     break;
            case DasPacket::MOD_TYPE_ROC:       plugin = "RocPlugin";       type = "roc";     break;
/*
 * These are not yet supported
            case DasPacket::MOD_TYPE_AROC:      plugin = "ArocPlugin";      type = "aroc";    break;
            case DasPacket::MOD_TYPE_BIDIMROC:  plugin = "BidimRocPlugin";  type = "Broc";    break;
            case DasPacket::MOD_TYPE_BLNROC:    plugin = "BnlRocPlugin";    type = "broc";    break;
            case DasPacket::MOD_TYPE_CROC:      plugin = "CrocPlugin";      type = "croc";    break;
            case DasPacket::MOD_TYPE_FFC:       plugin = "FfcPlugin";       type = "ffc";     break;
            case DasPacket::MOD_TYPE_HROC:      plugin = "HrocPlugin";      type = "hroc";    break;
            case DasPacket::MOD_TYPE_IROC:      plugin = "IrocPlugin";      type = "iroc";    break;
            case DasPacket::MOD_TYPE_SANSROC:   plugin = "SansRocPlugin";   type = "sroc";    break;
*/
            default:                            plugin = "Unknown";         type = "unkn";    break;
        }
        
        if (ids.find(type) == ids.end())
            ids.insert(std::pair<std::string, uint32_t>(type, 1));
        std::stringstream id;
        id << type << ids[type]++;
        
        ret = snprintf(buffer, length, "{ PLUGIN=%s, ID=%s, IP=%s, VER=%d%d }\n",
                       plugin, id.str().c_str(), moduleId.c_str(), version.fw_version, version.fw_revision);
        if (ret == -1 || ret > length)
            break;

        length -= ret;
        buffer += ret;
    }
    return (size - length);
}

asynStatus DiscoverPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == Output) {
        int format = 0;
        getIntegerParam(Format, &format);
        
        if (format == 0)
            *nActual = formatOutput(value, nChars);
        else
            *nActual = formatSubstitution(value, nChars);
        if (eomReason) *eomReason |= ASYN_EOM_EOS;
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void DiscoverPlugin::report(FILE *fp, int details)
{
    char buffer[16*1024]; // Should be sufficient for about 230 modules
    uint32_t length = formatOutput(buffer, sizeof(buffer));
    fwrite(buffer, 1, length, fp);
    return BasePlugin::report(fp, details);
}

void DiscoverPlugin::reqDiscover(uint32_t hardwareId)
{
    DasPacket *packet = DasPacket::createOcc(DasPacket::HWID_SELF, hardwareId, DasPacket::CMD_DISCOVER, 0, 0);
    if (!packet) {
        LOG_ERROR("Failed to allocate DISCOVER packet");
        return;
    }
    sendToDispatcher(packet);
    delete packet;
}

void DiscoverPlugin::reqLvdsDiscover(uint32_t hardwareId)
{
    DasPacket *packet = DasPacket::createLvds(DasPacket::HWID_SELF, hardwareId, DasPacket::CMD_DISCOVER, 0, 0);
    if (!packet) {
        LOG_ERROR("Failed to allocate DISCOVER LVDS packet");
        return;
    }
    sendToDispatcher(packet);
    delete packet;
}

void DiscoverPlugin::reqVersion(uint32_t hardwareId)
{
    DasPacket *packet = DasPacket::createOcc(DasPacket::HWID_SELF, hardwareId, DasPacket::CMD_READ_VERSION, 0, 0);
    if (!packet) {
        LOG_ERROR("Failed to allocate READ_VERSION packet");
        return;
    }
    sendToDispatcher(packet);
    delete packet;
}

void DiscoverPlugin::reqLvdsVersion(uint32_t hardwareId)
{
    DasPacket *packet = DasPacket::createLvds(DasPacket::HWID_SELF, hardwareId, DasPacket::CMD_READ_VERSION, 0, 0);
    if (!packet) {
        LOG_ERROR("Failed to allocate READ_VERSION LVDS packet");
        return;
    }
    sendToDispatcher(packet);
    delete packet;
}
