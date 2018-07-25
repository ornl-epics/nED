/* ModulesPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"
#include "AcpcFemPlugin.h"
#include "AdcRocPlugin.h"
#include "ArocPlugin.h"
#include "BnlRocPlugin.h"
#include "CRocPlugin.h"
#include "ModulesPlugin.h"
#include "DspPlugin.h"
#include "DspWPlugin.h"
#include "FemPlugin.h"
#include "Log.h"
#include "RocPlugin.h"

#include <algorithm>
#include <cstring>
#include <sstream>

EPICS_REGISTER_PLUGIN(ModulesPlugin, 3, "Port name", string, "Parent plugins", string, "PVA name", string);

ModulesPlugin::ModulesPlugin(const char *portName, const char *parentPlugins, const char *pvName)
    : BasePlugin(portName, 1, asynOctetMask, asynOctetMask)
    , m_disableTimer(true)
    , m_parentPlugins(parentPlugins)
{
    // Allocate and initialize text buffers
    m_bufferTxt = (char *)malloc(BUFFER_SIZE);
    if (m_bufferTxt == 0) {
        LOG_ERROR("Failed to allocate text buffer");
        assert(m_bufferTxt != 0);
    }

    createParam("Discover",     asynParamInt32, &Discover);         // WRITE - Trigger discovery of modules
    createParam("RefreshPvaList",asynParamInt32,&RefreshPvaList);   // WRITE - Refresh modules list in PVA record
    createParam("Discovered",   asynParamInt32, &Discovered, 0);    // READ - Modules found formatted in ASCII table
    createParam("Verified",     asynParamInt32, &Verified, 0);      // READ - Modules found formatted in ASCII table
    createParam("TxtDisplay",   asynParamOctet, &TxtDisplay);       // READ - Text in human readable format

    if (pvName == 0 || strlen(pvName) == 0) {
        LOG_ERROR("Missing PVA record name");
    } else {
        m_record = Record::create(pvName);
        if (!m_record)
            LOG_ERROR("Failed to create PVA record '%s'", pvName);
        else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_record) == false)
            LOG_ERROR("Failed to register PVA record '%s'", pvName);
    }

    // Check parent plugins exist, force saving connect information
    connect(parentPlugins, MsgDasCmd);
    disconnect();

    callParamCallbacks();
}

asynStatus ModulesPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Discover) {
        setIntegerParam(Discovered, 0);
        setIntegerParam(Verified, 0);
        setStringParam(TxtDisplay, "");
        callParamCallbacks();

        // Stay connected to parent plugins for 10 seconds after discover has
        // been issued. Usually responses come back in under a second.
        m_disableTimer.cancel();
        if (!isConnected()) {
            connect(m_parentPlugins, {MsgDasCmd});
        }
        std::function<float(void)> disableCb = [this](){ this->lock(); disconnect(); this->unlock(); return 0; };
        m_disableTimer.schedule(disableCb, 10);

        m_discovered.clear();
        reqDiscover();
        return asynSuccess;
    } else if (pasynUser->reason == RefreshPvaList) {
        std::list<std::string> modules;
        BaseModulePlugin::getModuleNames(modules);
        return m_record->update(modules) ? asynSuccess : asynError;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void ModulesPlugin::recvDownstream(const DasCmdPacketList &packets)
{
    int nDiscovered;
    int nVerified;

    for (auto it = packets.cbegin(); it != packets.cend(); it++) {
        const DasCmdPacket *packet = *it;

        // Silently skip packets we're not interested in
        if (!packet->isResponse())
            continue;

        if (packet->getCommand() == DasCmdPacket::CMD_DISCOVER) {
            DasCmdPacket::ModuleType module_type = static_cast<DasCmdPacket::ModuleType>(0);
            if (packet->getLength() >= (sizeof(DasCmdPacket)+sizeof(uint32_t)))
                module_type = static_cast<DasCmdPacket::ModuleType>(packet->getCmdPayload()[0]);
            m_discovered[packet->getModuleId()].type = module_type;
            reqVersion(packet->getModuleId());

        } else if (packet->getCommand() == DasCmdPacket::CMD_READ_VERSION) {
            // We need to know module type as version response format differs between modules.
            // This is the main reason why reading version is not done in parallel with discovery.
            if (m_discovered.find(packet->getModuleId()) != m_discovered.end()) {
                switch (m_discovered[packet->getModuleId()].type) {
                case DasCmdPacket::MOD_TYPE_ACPC:
                    AcpcPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_ACPCFEM:
                    AcpcFemPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_ADCROC:
                    AdcRocPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_AROC:
                    ArocPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_BNLROC:
                    BnlRocPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_CROC:
                    CRocPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_DSP:
                    DspPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_FEM:
                    FemPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_ROC:
                    RocPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
                case DasCmdPacket::MOD_TYPE_DSPW:
                    DspWPlugin::parseVersionRsp(packet, m_discovered[packet->getModuleId()].version);
                    break;
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
    bool updateText = false;
    for (auto it=m_discovered.begin(); it!=m_discovered.end(); it++) {
        if (it->second.type != 0) {
            nDiscovered++;
            updateText = true;
        }
        if (it->second.version.fw_version != 0 || it->second.version.fw_revision !=0) {
            nVerified++;
            updateText = true;
        }
    }

    if (updateText) {
        if (formatTxt(m_bufferTxt, BUFFER_SIZE) > 0) {
            setStringParam(TxtDisplay, m_bufferTxt);
        }
    }

    setIntegerParam(Discovered, nDiscovered);
    setIntegerParam(Verified,   nVerified);
    callParamCallbacks();
}

uint32_t ModulesPlugin::formatTxt(char *buffer, uint32_t size)
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
        BaseModulePlugin::Version version = it->second.version;
        char line[128];

        std::stringstream id;
        if (!name.empty()) {
            id << name;
        } else {
            std::string type;
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
                case DasCmdPacket::MOD_TYPE_ADCROC:    type = "adcroc";    break;
                case DasCmdPacket::MOD_TYPE_SANSROC:   type = "sansroc";   break;
                default:                               type = "invalid";
            }
            
            if (ids.find(type) == ids.end())
                ids.insert(std::pair<std::string, uint32_t>(type, 1));
            id << "UNKWN " << type << ids[type]++;
        }

        if (it->second.parent != 0) {
            ret = snprintf(line, sizeof(line),
                           "%-20s: %-15s ver %d.%d/%d.%d date %.04d/%.02d/%.02d (DSP=%s)\n",
                           id.str().c_str(), moduleId.c_str(), version.hw_version, version.hw_revision,
                           version.fw_version, version.fw_revision, version.fw_year,
                           version.fw_month, version.fw_day, parentId.c_str());
        } else {
            ret = snprintf(line, sizeof(line),
                           "%-20s: %-15s ver %d.%d/%d.%d date %.04d/%.02d/%.02d\n",
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

void ModulesPlugin::report(FILE *fp, int details)
{
    uint32_t length = formatTxt(m_bufferTxt, BUFFER_SIZE);
    fwrite(m_bufferTxt, 1, length, fp);
    return BasePlugin::report(fp, details);
}

void ModulesPlugin::reqDiscover(uint32_t moduleId)
{
    std::array<uint8_t, 1024> buffer;
    DasCmdPacket *packet = DasCmdPacket::init(buffer.data(), buffer.size(), moduleId, DasCmdPacket::CMD_DISCOVER, 0);
    if (!packet) {
        LOG_ERROR("Failed to allocate DISCOVER packet");
        return;
    }
    sendUpstream(packet);
}

void ModulesPlugin::reqVersion(uint32_t moduleId)
{
    std::array<uint8_t, 1024> buffer;
    DasCmdPacket *packet = DasCmdPacket::init(buffer.data(), buffer.size(), moduleId, DasCmdPacket::CMD_READ_VERSION, 0);
    if (!packet) {
        LOG_ERROR("Failed to allocate READ_VERSION packet");
        return;
    }
    sendUpstream(packet);
}

ModulesPlugin::Record::Record(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
    : epics::pvDatabase::PVRecord(recordName, pvStructure)
{
}

/**
 * Allocate and initialize ModulesPlugin::Record.
 */
ModulesPlugin::Record::shared_pointer ModulesPlugin::Record::create(const std::string &recordName)
{
    using namespace epics::pvData;

    StandardFieldPtr standardField = getStandardField();
    FieldCreatePtr fieldCreate     = getFieldCreate();
    PVDataCreatePtr pvDataCreate   = getPVDataCreate();

    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        fieldCreate->createFieldBuilder()->
            add("modules", standardField->scalarArray(epics::pvData::pvString, ""))->
            createStructure()
    );

    Record::shared_pointer pvRecord(new ModulesPlugin::Record(recordName, pvStructure));
    if (pvRecord && !pvRecord->init()) {
        pvRecord.reset();
    }

    return pvRecord;
}

/**
 * Attach all PV structures.
 */
bool ModulesPlugin::Record::init()
{
    initPVRecord();

    pvModules = getPVStructure()->getSubField<epics::pvData::PVStringArray>("modules.value");
    if (pvModules.get() == NULL)
        return false;

    return true;
}

/**
 * Publish a single atomic update of the PV, take values from packet.
 */
bool ModulesPlugin::Record::update(const std::list<std::string> &modules)
{
    bool updated = true;

    epics::pvData::PVStringArray::svector names;
    for (auto it = modules.begin(); it != modules.end(); it++) {
        names.push_back(*it);
    }

    lock();
    try {
        beginGroupPut();
        pvModules->replace(epics::pvData::freeze(names));
        endGroupPut();
    } catch (...) {
        updated = false;
    }
    unlock();

    return updated;
}
