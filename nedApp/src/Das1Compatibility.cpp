/* Das1Compatibility.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Das1Compatibility.h"
#include "Packet.h"

EPICS_REGISTER_PLUGIN(Das1Compatibility, 2, "Port name", string, "Parent port name(s)", string);

Das1Compatibility::Das1Compatibility(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, std::string(parentPlugins).find(',')!=std::string::npos)
{
    // Permanently connect to all parent plugins
    BasePlugin::connect(parentPlugins, MsgOldDas);
}

void Das1Compatibility::recvDownstream(DasCmdPacketList *packets)
{
    unlock();
    sendDownstream(packets);
    lock();
}

void Das1Compatibility::recvDownstream(DasRtdlPacketList *packets)
{
    unlock();
    sendDownstream(packets);
    lock();
}

void Das1Compatibility::recvDownstream(ErrorPacketList *packets)
{
    unlock();
    sendDownstream(packets);
    lock();
}

void Das1Compatibility::recvDownstream(DasPacketList *packets)
{
    DasCmdPacketList cmds;
    DasRtdlPacketList rtdls;

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        const DasPacket *packet = *it;
        if (packet->isRtdl()) {
            // Eliminate data flavor of RTDL packets - they're the same
            if (packet->isCommand()) {
                rtdls.push_back(old2new_rtdl(packet));
            }
        } else if (packet->isCommand()) {
            // RTDL command case has been handled, additionally filter out
            // some command pretenders
            if (packet->getCommandType() != DasPacket::CMD_TSYNC) {
                cmds.push_back(old2new_cmd(packet));
            }
        } else if (packet->isData()) {
            // TODO!!!
        } else {
            // Discard other packets
        }
    }

    this->unlock();
    std::vector< std::shared_ptr<PluginMessage> > messages;
    if (!rtdls.empty())
        messages.push_back(BasePlugin::sendDownstream(&rtdls, false));
    if (!cmds.empty())
        messages.push_back(BasePlugin::sendDownstream(&cmds, false));

    for (auto it = messages.begin(); it != messages.end(); it++) {
        if (!!(*it)) {
            (*it)->waitAllReleased();
        }
    }
    this->lock();

    for (auto it = rtdls.begin(); it != rtdls.end(); it++) {
        free(*it);
    }
    for (auto it = cmds.begin(); it != cmds.end(); it++) {
        free(*it);
    }
}

void Das1Compatibility::recvUpstream(DasCmdPacketList *packets)
{
    DasPacketList das1Packets;

    for (auto it = packets->cbegin(); it != packets->cend(); it++) {
        DasPacket *packet;

        // Since we don't know whether target is DSP or other module, we send
        // out both flavors of the command.
        packet = new2old_cmd(*it, DAS1_OPTICAL);
        if (packet)
            das1Packets.push_back(packet);
        packet = new2old_cmd(*it, DAS1_LVDS);
        if (packet)
            das1Packets.push_back(packet);

        // DSP-T prior to 6.5 will intercept global discover command and do his
        // own discovery. To work around that we need to send a single word
        // LVDS command.
        if ((*it)->command == DasCmdPacket::CMD_DISCOVER && (*it)->module_id == DasCmdPacket::BROADCAST_ID) {
            packet = new2old_cmd(*it, DAS1_LVDS_SINGLE_WORD);
            if (packet)
                das1Packets.push_back(packet);
        }
    }
    sendUpstream(&das1Packets);

    for (auto it = das1Packets.begin(); it != das1Packets.end(); it++) {
        delete *it;
    }
}

DasRtdlPacket *Das1Compatibility::old2new_rtdl(const DasPacket *packet)
{
    const uint32_t *frames = (packet->payload + sizeof(RtdlHeader)/sizeof(uint32_t));
    size_t nFrames = (packet->payload_length - sizeof(RtdlHeader))/sizeof(uint32_t);
    uint32_t sourceId = 0;
    bool found = false;

    // Map 32-bit DSP-T address to unique source id number
    for (sourceId = 0; sourceId < m_rtdlSources.size(); sourceId++) {
        if (m_rtdlSources[sourceId] == packet->source) {
            found = true;
            break;
        }
    }
    if (!found && m_rtdlSources.size() < 255) { // 255 comes from size of DasRtdlPacket::source
        sourceId = m_rtdlSources.size();
        m_rtdlSources.push_back(packet->source);
    }

    return DasRtdlPacket::create(sourceId,
                                 packet->getRtdlHeader(),
                                 frames,
                                 nFrames);
}

DasCmdPacket *Das1Compatibility::old2new_cmd(const DasPacket *packet)
{
    if (packet->getCommandType() == DasPacket::CMD_DISCOVER) {
        uint32_t module_type = static_cast<uint32_t>(packet->cmdinfo.module_type);
        return DasCmdPacket::create(packet->getSourceAddress(),
                                    DasCmdPacket::CMD_DISCOVER,
                                    true,
                                    packet->cmdinfo.is_response,
                                    0,
                                    &module_type,
                                    sizeof(module_type));
    } else {
        return DasCmdPacket::create(packet->getSourceAddress(),
                                    static_cast<DasCmdPacket::CommandType>(packet->getCommandType()),
                                    (packet->cmdinfo.command != DasPacket::RSP_NACK),
                                    packet->cmdinfo.is_response,
                                    (packet->cmdinfo.is_channel ? packet->cmdinfo.channel + 1 : 0),
                                    packet->getPayload(),
                                    packet->getPayloadLength());
    }
}

DasPacket *Das1Compatibility::new2old_cmd(const DasCmdPacket *packet, enum Das1PacketType mode)
{
    size_t payload_length = packet->cmd_length - packet->getHeaderLen();
    DasPacket::CommandType command = static_cast<DasPacket::CommandType>(packet->command);
    switch (mode) {
    case DAS1_LVDS_SINGLE_WORD:
        return DasPacket::createLvds(DasCmdPacket::OCC_ID, DasPacket::HWID_BROADCAST_SW, command, packet->cmd_sequence, payload_length, packet->payload);
    case DAS1_LVDS:
        return DasPacket::createLvds(DasCmdPacket::OCC_ID, packet->module_id, command, packet->cmd_sequence, payload_length, packet->payload);
    case DAS1_OPTICAL:
    default:
        return DasPacket::createOcc(DasCmdPacket::OCC_ID, packet->module_id, command, packet->cmd_sequence, payload_length, packet->payload);
    }
}
