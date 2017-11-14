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
    static std::list<int> msgs = { MsgOldDas, MsgDasCmd, MsgDasRtdl };
    BasePlugin::connect(parentPlugins, MsgOldDas);
}

void Das1Compatibility::recvDownstream(int type, PluginMessage *msg)
{
    if (type != MsgOldDas) {
        sendDownstream(type, msg);
    } else {
        DasCmdPacketList cmds;
        DasRtdlPacketList rtdls;

        const DasPacketList *packets = dynamic_cast<const DasPacketList*>(msg);

        for (auto it = packets->cbegin(); it != packets->cend(); it++) {
            const DasPacket *packet = *it;
            if (packet->isRtdl() && packet->isCommand()) {
                rtdls.push_back(old2new_rtdl(packet));
            } else if (packet->isCommand() && !packet->isRtdl()) {
                cmds.push_back(old2new_cmd(packet));
            } else if (packet->isData() && !packet->isRtdl()) {
                // TODO!!!
            } else {
                // Discard other packets
            }
        }

        if (!rtdls.empty()) {
            rtdls.claim();
            BasePlugin::sendDownstream(&rtdls);
            rtdls.release();
            rtdls.waitAllReleased();
            for (auto it = rtdls.begin(); it != rtdls.end(); it++) {
                free(*it);
            }
        }

        if (!cmds.empty()) {
            cmds.claim();
            BasePlugin::sendDownstream(&cmds);
            cmds.release();
            cmds.waitAllReleased();
            for (auto it = cmds.begin(); it != cmds.end(); it++) {
                free(*it);
            }
        }
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
    return DasCmdPacket::create(packet->source,
                                packet->destination,
                                static_cast<DasCmdPacket::CommandType>(packet->getCommandType()),
                                (packet->cmdinfo.command != DasPacket::RSP_NACK),
                                packet->cmdinfo.is_response,
                                (packet->cmdinfo.is_channel ? packet->cmdinfo.channel + 1 : 0),
                                packet->getPayload(),
                                packet->getPayloadLength());
}

DasPacket *Das1Compatibility::new2old_cmd(const DasCmdPacket *packet, bool lvds)
{
    size_t payload_length = packet->length - sizeof(DasCmdPacket);
    DasPacket::CommandType command = static_cast<DasPacket::CommandType>(packet->command);
    if (lvds) {
        return DasPacket::createLvds(packet->source, packet->destination, command, packet->channel, payload_length, packet->payload);
    } else {
        return DasPacket::createOcc(packet->source, packet->destination, command, packet->channel, payload_length, packet->payload);
    }
}
