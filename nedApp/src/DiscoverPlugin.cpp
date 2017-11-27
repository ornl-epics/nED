/* DiscoverPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"
//#include "AcpcFemPlugin.h"
//#include "AdcRocPlugin.h"
//#include "ArocPlugin.h"
//#include "BnlRocPlugin.h"
//#include "CRocPlugin.h"
#include "DiscoverPlugin.h"
#include "DspPlugin.h"
//#include "DspWPlugin.h"
#include "FemPlugin.h"
#include "Log.h"
//#include "RocPlugin.h"

#include <algorithm>
#include <cstring>
#include <sstream>

EPICS_REGISTER_PLUGIN(DiscoverPlugin, 2, "Port name", string, "Parent plugins", string);

DiscoverPlugin::DiscoverPlugin(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, 1, asynOctetMask, asynOctetMask)
    , m_disableTimer(true)
    , m_parentPlugins(parentPlugins)
{
    createParam("Trigger",      asynParamInt32, &Trigger);      // WRITE - Trigger discovery of modules
    createParam("Format",       asynParamInt32, &Format);       // READ - Modules found formatted in ASCII table
    createParam("Discovered",   asynParamInt32, &Discovered);   // READ - Modules found formatted in ASCII table
    createParam("Verified",     asynParamInt32, &Verified);     // READ - Modules found formatted in ASCII table
    createParam("Output",       asynParamOctet, &Output, "Not initialized");    // READ - Modules found formatted in ASCII table
    createParam("OptBcast",     asynParamInt32, &OptBcast, 1);  // WRITE - Send optical broadcast packet as part of discovery
    createParam("LvdsBcast",    asynParamInt32, &LvdsBcast, 1); // WRITE - Send LVDS broadcast packet as part of discovery
    createParam("LvdsSingle",   asynParamInt32, &LvdsSingle, 1);// WRITE - Send LVDS single word packet as part of discovery
    callParamCallbacks();
}

asynStatus DiscoverPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Trigger) {
        bool optBcast = true;
        bool lvdsBcast = true;
        bool lvdsSingle = true;

        getBooleanParam(OptBcast, optBcast);
        getBooleanParam(LvdsBcast, lvdsBcast);
        getBooleanParam(LvdsSingle, lvdsSingle);

        setIntegerParam(Discovered, 0);
        setIntegerParam(Verified, 0);
        callParamCallbacks();

        // Stay connected to parent plugins for 10 seconds after discover has
        // been issued. Usually responses come in under a second.
        m_disableTimer.cancel();
        if (!isConnected()) {
            connect(m_parentPlugins, MsgDasCmd);
        }
        std::function<float(void)> disableCb = [this](){ this->lock(); disconnect(); this->unlock(); return 0; };
        m_disableTimer.schedule(disableCb, 10);

        m_discovered.clear();
        reqDiscover();
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void DiscoverPlugin::recvDownstream(DasCmdPacketList *packets)
{
    int nDiscovered;
    int nVerified;

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        const DasCmdPacket *packet = *it;

        // Silently skip packets we're not interested in
        if (!packet->response)
            continue;

        if (packet->command == DasCmdPacket::CMD_DISCOVER) {
            DasCmdPacket::ModuleType module_type = static_cast<DasCmdPacket::ModuleType>(0);
            if (packet->length >= (sizeof(DasCmdPacket)+sizeof(uint32_t)))
                module_type = static_cast<DasCmdPacket::ModuleType>(packet->payload[0]);
            m_discovered[packet->module_id].type = module_type;
            reqVersion(packet->module_id);

        } else if (packet->command == DasCmdPacket::CMD_READ_VERSION) {
            // We need to know module type as version response format differs between modules.
            // This is the main reason why reading version is not done in parallel with discovery.
            if (m_discovered.find(packet->module_id) != m_discovered.end()) {
                switch (m_discovered[packet->module_id].type) {
                case DasCmdPacket::MOD_TYPE_ACPC:
                    AcpcPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                /*
                case DasCmdPacket::MOD_TYPE_ACPCFEM:
                    AcpcFemPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                case DasCmdPacket::MOD_TYPE_ADCROC:
                    AdcRocPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                case DasCmdPacket::MOD_TYPE_AROC:
                    ArocPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                case DasCmdPacket::MOD_TYPE_BNLROC:
                    BnlRocPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                case DasCmdPacket::MOD_TYPE_CROC:
                    CRocPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                    */
                case DasCmdPacket::MOD_TYPE_DSP:
                    DspPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                case DasCmdPacket::MOD_TYPE_FEM:
                    FemPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                    /*
                case DasCmdPacket::MOD_TYPE_ROC:
                    RocPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                case DasCmdPacket::MOD_TYPE_DSPW:
                    DspWPlugin::parseVersionRsp(packet, m_discovered[packet->module_id].version);
                    break;
                */
                default:
                    break;
                }
            }
        }
    }

    // Do some statistics how many modules we have seen. Do it separately from
    // packet parsing to avoid double accounting in case more than one same packet is received.
    nDiscovered = 0;
    nVerified = 0;
    for (auto it=m_discovered.begin(); it!=m_discovered.end(); it++) {
        if (it->second.type != 0)
            nDiscovered++;
        if (it->second.version.fw_version != 0 || it->second.version.fw_revision !=0)
            nVerified++;
    }

    setIntegerParam(Discovered, nDiscovered);
    setIntegerParam(Verified,   nVerified);
    callParamCallbacks();
}

uint32_t DiscoverPlugin::formatOutput(char *buffer, uint32_t size)
{
    int ret;
    size_t length = size;
    uint32_t i = 1;
    std::map<std::string, uint32_t> ids;
    std::vector<std::string> lines;

    ret = snprintf(buffer, length, "Discovered modules:\n");
    if (ret > (int)length)
        return 0;
    length -= ret;
    buffer += ret;

    for (std::map<uint32_t, ModuleDesc>::iterator it = m_discovered.begin(); it != m_discovered.end(); it++, i++) {
        std::string moduleId(BaseModulePlugin::addr2ip(it->first));
        std::string parentId(BaseModulePlugin::addr2ip(it->second.parent));
        std::string name(BaseModulePlugin::getModuleName(it->first));
        const char *type;
        BaseModulePlugin::Version version = it->second.version;
        char line[128];

        switch (it->second.type) {
            case DasCmdPacket::MOD_TYPE_ACPC:      type = "acpc";      break;
            case DasCmdPacket::MOD_TYPE_ACPCFEM:   type = "acpcfem";   break;
            case DasCmdPacket::MOD_TYPE_AROC:      type = "aroc";      break;
            case DasCmdPacket::MOD_TYPE_BIDIMROC:  type = "bidimroc";  break;
            case DasCmdPacket::MOD_TYPE_BNLROC:    type = "bnlroc";    break;
            case DasCmdPacket::MOD_TYPE_CROC:      type = "croc";      break;
            case DasCmdPacket::MOD_TYPE_DSP:       type = "dsp";       break;
            case DasCmdPacket::MOD_TYPE_DSPW:      type = "dsp-w";     break;
            case DasCmdPacket::MOD_TYPE_FFC:       type = "ffc";       break;
            case DasCmdPacket::MOD_TYPE_FEM:       type = "fem";       break;
            case DasCmdPacket::MOD_TYPE_HROC:      type = "hroc";      break;
            case DasCmdPacket::MOD_TYPE_IROC:      type = "iroc";      break;
            case DasCmdPacket::MOD_TYPE_ROC:       type = "roc";       break;
            case DasCmdPacket::MOD_TYPE_ADCROC:    type = "adcrod";    break;
            case DasCmdPacket::MOD_TYPE_SANSROC:   type = "sansroc";   break;
            default:                            type = "unknown";
        }

        std::stringstream id;
        if (name.empty()) {
            if (ids.find(type) == ids.end())
                ids.insert(std::pair<std::string, uint32_t>(type, 1));
            id << type << ids[type]++;
        } else {
            id << name;
        }

        if (it->second.parent != 0) {
            ret = snprintf(line, sizeof(line),
                           "%-12s: %-15s ver %d.%d/%d.%d date %.04d/%.02d/%.02d (DSP=%s)\n",
                           id.str().c_str(), moduleId.c_str(), version.hw_version, version.hw_revision,
                           version.fw_version, version.fw_revision, version.fw_year,
                           version.fw_month, version.fw_day, parentId.c_str());
        } else {
            ret = snprintf(line, sizeof(line),
                           "%-12s: %-15s ver %d.%d/%d.%d date %.04d/%.02d/%.02d\n",
                           id.str().c_str(), moduleId.c_str(), version.hw_version, version.hw_revision,
                           version.fw_version, version.fw_revision, version.fw_year,
                           version.fw_month, version.fw_day);
        }
        if (ret == -1) {
            LOG_WARN("String exceeds limit of %u bytes", (unsigned)sizeof(line));
        } else {
            lines.push_back(line);
        }
    }

    std::sort(lines.begin(), lines.end());
    for (auto line = lines.begin(); line != lines.end(); line++) {
        if (line->length() >= length) {
            LOG_WARN("Truncating output, buffer to short");
            break;
        }

        strncpy(buffer, line->c_str(), line->length());
        length -= line->length();
        buffer += line->length();
    }
    return (size - length);
}

uint32_t DiscoverPlugin::formatSubstitution(char *buffer, uint32_t size)
{
    int ret;
    size_t length = size;
    uint32_t i = 1;

    std::vector<std::string> lines;
    std::map<std::string, uint32_t> ids;

    for (std::map<uint32_t, ModuleDesc>::iterator it = m_discovered.begin(); it != m_discovered.end(); it++, i++) {
        std::string moduleId(BaseModulePlugin::addr2ip(it->first));
        std::string name(BaseModulePlugin::getModuleName(it->first));
        const char *plugin;
        const char *type;
        BaseModulePlugin::Version version = it->second.version;
        char line[128];

        switch (it->second.type) {
            case DasCmdPacket::MOD_TYPE_ACPCFEM:   plugin = "AcpcFemPlugin";   type = "afem";    break;
            case DasCmdPacket::MOD_TYPE_ACPC:      plugin = "AcpcPlugin";      type = "acpc";    break;
            case DasCmdPacket::MOD_TYPE_ADCROC:    plugin = "AdcRocPlugin";    type = "adcroc";  break;
            case DasCmdPacket::MOD_TYPE_AROC:      plugin = "ArocPlugin";      type = "aroc";    break;
            case DasCmdPacket::MOD_TYPE_BNLROC:    plugin = "BnlRocPlugin";    type = "broc";    break;
            case DasCmdPacket::MOD_TYPE_CROC:      plugin = "CRocPlugin";      type = "croc";    break;
            case DasCmdPacket::MOD_TYPE_DSP:       plugin = "DspPlugin";       type = "dsp";     break;
            case DasCmdPacket::MOD_TYPE_DSPW:      plugin = "DspWPlugin";      type = "dspw";    break;
            case DasCmdPacket::MOD_TYPE_FEM:       plugin = "FemPlugin";       type = "fem";     break;
            case DasCmdPacket::MOD_TYPE_ROC:       plugin = "RocPlugin";       type = "roc";     break;
/*
 * These are not yet supported
            case DasCmdPacket::MOD_TYPE_BIDIMROC:  plugin = "BidimRocPlugin";  type = "Broc";    break;
            case DasCmdPacket::MOD_TYPE_FFC:       plugin = "FfcPlugin";       type = "ffc";     break;
            case DasCmdPacket::MOD_TYPE_HROC:      plugin = "HrocPlugin";      type = "hroc";    break;
            case DasCmdPacket::MOD_TYPE_IROC:      plugin = "IrocPlugin";      type = "iroc";    break;
            case DasCmdPacket::MOD_TYPE_SANSROC:   plugin = "SansRocPlugin";   type = "sroc";    break;
*/
            default:                            plugin = "Unknown";         type = "unkn";    break;
        }

        std::stringstream id;
        if (name.empty()) {
            if (ids.find(type) == ids.end())
                ids.insert(std::pair<std::string, uint32_t>(type, 1));
            id << type << ids[type]++;
        } else {
            id << name;
        }

        ret = snprintf(line, sizeof(line), "{ PLUGIN=%s, ID=%s, IP=%s, VER=%d%d }\n",
                       plugin, id.str().c_str(), moduleId.c_str(), version.fw_version, version.fw_revision);
        if (ret == -1) {
            LOG_WARN("String exceeds limit of %u bytes", (unsigned)sizeof(line));
        } else {
            lines.push_back(line);
        }
    }

    std::sort(lines.begin(), lines.end());
    for (auto line = lines.begin(); line != lines.end(); line++) {
        if (line->length() >= length) {
            LOG_WARN("Truncating output, buffer to short");
            break;
        }

        strncpy(buffer, line->c_str(), line->length());
        length -= line->length();
        buffer += line->length();
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
    size_t size = 100000; // Should be enough for about 1000 modules
    char *buffer = (char *)malloc(size);
    if (buffer) {
        uint32_t length = formatOutput(buffer, size);
        fwrite(buffer, 1, length, fp);
        free(buffer);
    }
    return BasePlugin::report(fp, details);
}

void DiscoverPlugin::reqDiscover(uint32_t moduleId)
{
    DasCmdPacket *packet = DasCmdPacket::create(moduleId, DasCmdPacket::CMD_DISCOVER);
    if (!packet) {
        LOG_ERROR("Failed to allocate DISCOVER packet");
        return;
    }
    sendUpstream(packet);
    delete packet;
}

void DiscoverPlugin::reqVersion(uint32_t moduleId)
{
    DasCmdPacket *packet = DasCmdPacket::create(moduleId, DasCmdPacket::CMD_READ_VERSION);
    if (!packet) {
        LOG_ERROR("Failed to allocate READ_VERSION packet");
        return;
    }
    sendUpstream(packet);
    delete packet;
}
