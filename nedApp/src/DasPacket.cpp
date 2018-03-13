/* DasPacket.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DasPacket.h"
#include "Common.h"

#include <stdexcept>
#include <string.h>
#include <sstream>

// Static members initilization
uint32_t DasPacket::MinLength = sizeof(DasPacket);
uint32_t DasPacket::MaxLength = sizeof(DasPacket) + 4000; // DSP-T is hardcoded to 3600 total but can leak and do slightly more, 4024 should be safe for DSP-T

DasPacket::DasPacket(uint32_t source_, uint32_t destination_, CommandInfo cmdinfo_, uint32_t payload_length_, uint32_t *payload_)
    : destination(destination_)
    , source(source_)
    , cmdinfo(cmdinfo_)
    , payload_length(payload_length_)
    , reserved1(0)
    , reserved2(0)
{
    if (payload_) {
        memcpy(payload, payload_, payload_length);
    } else {
        memset(payload, 0, payload_length);
    }
}

DasPacket *DasPacket::initOptical(uint8_t *buffer, size_t size, uint32_t source, uint32_t destination, CommandType command, uint8_t channel, uint32_t payload_length, const uint32_t *payload)
{
    DasPacket *packet = nullptr;
    CommandInfo cmdinfo;

    if (size > (sizeof(DasPacket) + ALIGN_UP(payload_length, 4))) {
        memset(&cmdinfo, 0, sizeof(cmdinfo));

        payload_length = (payload_length + 3) & ~3;
        cmdinfo.is_command = true;
        cmdinfo.command = command;
        if (channel > 0) {
            cmdinfo.channel = (channel - 1) & 0xF;
            cmdinfo.is_channel = 1;
        }

        packet = new (buffer) DasPacket(source, destination, cmdinfo, payload_length, const_cast<uint32_t *>(payload));
    }
    return packet;
}

DasPacket *DasPacket::initOptical(uint8_t *buffer, size_t size, const DasCmdPacket *orig)
{
    return initOptical(buffer,
                       size,
                       DasPacket::HWID_SELF,
                       orig->getModuleId(),
                       static_cast<DasPacket::CommandType>(orig->getCommand()),
                       orig->getSequenceId(),
                       orig->getCmdPayloadLength(),
                       orig->getCmdPayload());
}

DasPacket *DasPacket::initLvds(uint8_t *buffer, size_t size, uint32_t source, uint32_t destination, CommandType command, uint8_t channel, uint32_t payload_length, const uint32_t *payload)
{
    DasPacket *packet = nullptr;

    // Real payload is put in the DasPacket header
    uint32_t real_payload_length;
    real_payload_length = ALIGN_UP(payload_length, 2); // 2-byte aligned data for LVDS stuff
    real_payload_length *= 2; // 32 bits are devided into two dwords
    real_payload_length += (destination != HWID_BROADCAST_SW ? 2*sizeof(uint32_t) : 0); // First 2 dwords of the payload represent LVDS address for non-global commands

    if (size >= (sizeof(DasPacket) + ALIGN_UP(real_payload_length, 4))) {

        CommandInfo cmdinfo;
        memset(&cmdinfo, 0, sizeof(cmdinfo));
        cmdinfo.is_command = true;
        cmdinfo.is_passthru = true;
        cmdinfo.command = command;
        if (channel > 0) {
            cmdinfo.channel = (channel - 1) & 0xF;
            cmdinfo.is_channel = 1;
        }

        uint32_t offset = 0;

        // Cmdinfo from the OCC header is the first dword DSP passes thru
        // It must have all the LVDS flags
        cmdinfo.lvds_cmd = true;
        cmdinfo.lvds_start = true;
        cmdinfo.lvds_parity = lvdsParity(*reinterpret_cast<uint32_t *>(&cmdinfo) & 0xFFFFFF);

        // Destination in OCC header is always 0 for LVDS packets
        // Don't copy the payload
        packet = new (buffer) DasPacket(source, 0, cmdinfo, real_payload_length, 0);

        // Add 2 dwords for destination address, both LVDS pass-thru
        if (destination != HWID_BROADCAST_SW) {
            packet->payload[offset] = destination & 0xFFFF;
            packet->payload[offset] |= (lvdsParity(packet->payload[offset]) << 16);
            offset++;
            packet->payload[offset] = (destination >> 16 ) & 0xFFFF;
            packet->payload[offset] |= (lvdsParity(packet->payload[offset]) << 16);
            offset++;
        }

        // Split each 32 bits from the payload into two 16 bit parts and put them into separate dwords;
        // lower 16 bits into first dword, upper 16 bits into second dword
        // Add LVDS parity check to each word
        // Process 4-byte aligned data first, than optional 2-bytes at the end
        for (uint32_t i=0; i<payload_length/4; i++) {
            packet->payload[offset] = payload[i] & 0xFFFF;
            packet->payload[offset] |= lvdsParity(packet->payload[offset]) << 16;
            offset++;
            packet->payload[offset] = (payload[i] >> 16) & 0xFFFF;
            packet->payload[offset] |= lvdsParity(packet->payload[offset]) << 16;
            offset++;
        }
        if (payload_length % 4 != 0) {
            uint32_t i = payload_length / 4;
            packet->payload[offset] = payload[i] & 0xFFFF;
            packet->payload[offset] |= lvdsParity(packet->payload[offset]) << 16;
            offset++;
            packet->payload[offset] = 0;
            // don't increment offset
        }

        // Finalize LVDS packet - last dword must have the stop flag
        if (offset > 0) {
            offset--;
            packet->payload[offset] |= (0x1 << 17); // Last word flag...
            packet->payload[offset] ^= (0x1 << 16); // ... which flips parity
        } else {
            packet->cmdinfo.lvds_stop = true;       // Same here
            packet->cmdinfo.lvds_parity ^= 0x1;
        }
    }
    return packet;
}

DasPacket *DasPacket::initLvds(uint8_t *buffer, size_t size, const DasCmdPacket *orig, bool single)
{
    return initLvds(buffer,
                    size,
                    DasPacket::HWID_SELF,
                    (single ? HWID_BROADCAST_SW : orig->getModuleId()),
                    static_cast<DasPacket::CommandType>(orig->getCommand()),
                    orig->getSequenceId(),
                    orig->getCmdPayloadLength(),
                    orig->getCmdPayload());
}

bool DasPacket::isValid() const
{
    uint32_t length = getLength();
    return (length >= MinLength && length <= MaxLength);
}

uint32_t DasPacket::getLength() const
{
    // All incoming packets are 4-byte aligned by DSP
    // Outgoing commands for LVDS modules might be 2-byte aligned
    uint32_t packet_length = sizeof(DasPacket) + payload_length;
    if (packet_length != ALIGN_UP(packet_length, 4))
        packet_length = 0;
    else if (cmdinfo.is_command && cmdinfo.is_passthru && !cmdinfo.is_response && packet_length != ALIGN_UP(packet_length, 2))
        packet_length = 0;
    return packet_length;
}

uint32_t DasPacket::getMinLength()
{
    return DasPacket::MinLength;
}

uint32_t DasPacket::getMaxLength()
{
    return DasPacket::MaxLength;
}

uint32_t DasPacket::setMaxLength(uint32_t size)
{
    DasPacket::MaxLength = ALIGN_UP(size, 4);
    return DasPacket::MaxLength;
}

bool DasPacket::isCommand() const
{
    return (cmdinfo.is_command);
}

bool DasPacket::isResponse() const
{
    return (cmdinfo.is_command && cmdinfo.is_response);
}

void DasPacket::setResponse()
{
    cmdinfo.is_command = 1;
    cmdinfo.is_response = 1;
}

bool DasPacket::isData() const
{
    return (!cmdinfo.is_command);
}

bool DasPacket::isBad() const
{
    return (cmdinfo.command == BAD_PACKET);
}

bool DasPacket::isNeutronData() const
{
    // info == 0x0C
    return (!datainfo.is_command && datainfo.only_neutron_data && datainfo.format_code == 0x0);
}

bool DasPacket::isMetaData() const
{
    // info == 0x08
    return (!datainfo.is_command && !datainfo.only_neutron_data && datainfo.format_code == 0x0);
}

bool DasPacket::isRtdl() const
{
    if (cmdinfo.is_command) {
        return (cmdinfo.command == DasPacket::CommandType::CMD_RTDL);
    } else {
        // Data version of the RTDL command (0x85)
        return (info == 0x200000FF);
    }
}

const RtdlHeader *DasPacket::getRtdlHeader() const
{
    // RTDL packets always come from DSP only, so the RtdlHeader is at the start of payload
    if (cmdinfo.is_command && cmdinfo.command == DasPacket::CommandType::CMD_RTDL)
        return (RtdlHeader *)payload;
    if (!datainfo.is_command && datainfo.rtdl_present && payload_length >= sizeof(RtdlHeader))
        return (RtdlHeader *)payload;
    return 0;
};

const uint32_t *DasPacket::getData(uint32_t *count) const
{
    // DSP aggregates detectors data into data packets.
    // DSP-T includes RTDL information in all data packets immediately
    // after DAS header.
    const uint8_t *start = 0;
    *count = 0;
    if (!datainfo.is_command) {
        start = reinterpret_cast<const uint8_t *>(payload);
        if (datainfo.rtdl_present && payload_length >= sizeof(RtdlHeader))
            start += sizeof(RtdlHeader);
        *count = (payload_length - (start - reinterpret_cast<const uint8_t*>(payload)));
        *count /= sizeof(uint32_t);
    }
    return reinterpret_cast<const uint32_t *>(start);
}

uint32_t DasPacket::getDataLength() const
{
    if (cmdinfo.is_command)
        return 0;

    uint32_t length = payload_length;
    if (datainfo.rtdl_present) {
        if (payload_length < sizeof(RtdlHeader))
            return 0;

        length -= sizeof(RtdlHeader);
    }

    return length;
}

DasPacket::CommandType DasPacket::getCommandType() const
{
    CommandType command = static_cast<CommandType>(0);
    if (cmdinfo.is_command) {
        if (cmdinfo.command == RSP_ACK || cmdinfo.command == RSP_NACK) {
            if (cmdinfo.is_passthru) {
                command = reinterpret_cast<const CommandInfo *>(&payload[1])->command;
            } else {
                command = reinterpret_cast<const CommandInfo *>(&payload[0])->command;
            }
        } else {
            command = cmdinfo.command;
        }
    }
    return command;
}

DasPacket::CommandType DasPacket::getResponseType() const
{
    CommandType command = getCommandType();
    if (command != static_cast<CommandType>(0) && !cmdinfo.is_response)
        command = static_cast<CommandType>(0);
    return command;
}

uint32_t DasPacket::getSourceAddress() const
{
    if (cmdinfo.is_command && cmdinfo.is_passthru)
        return payload[0];
    else
        return source;
}

uint32_t DasPacket::getRouterAddress() const
{
    if (cmdinfo.is_command && cmdinfo.is_passthru)
        return source;
    else
        return 0;
}

const uint32_t *DasPacket::getPayload() const
{
    if (cmdinfo.is_command && cmdinfo.is_passthru) {
        if (cmdinfo.command == RSP_ACK || cmdinfo.command == RSP_NACK) {
            return &payload[2];
        } else {
            return &payload[1];
        }
    } else {
        return payload;
    }
}

uint32_t DasPacket::getPayloadLength() const
{
    if (cmdinfo.is_command && cmdinfo.is_passthru) {
        if (cmdinfo.command == RSP_ACK || cmdinfo.command == RSP_NACK) {
            return payload_length - 8;
        } else {
            return payload_length - 4;
        }
    } else {
        return payload_length;
    }
}

bool DasPacket::lvdsParity(int number)
{
    // even parity
    int temp = 0;
    while (number != 0) {
        temp = temp ^ (number & 0x1);
        number >>= 1;
    }
    return temp;
}

bool DasPacket::copyHeader(DasPacket *dest, uint32_t destSize) const
{
    uint32_t copySize = sizeof(DasPacket);
    uint32_t payload_length = 0;
    if (getRtdlHeader() != 0) {
        copySize += sizeof(RtdlHeader);
        payload_length += sizeof(RtdlHeader);
    }

    if (destSize < copySize)
        return false;

    memcpy(dest, this, copySize);
    dest->payload_length = payload_length;
    return true;
}

DasPacket::DataTypeLegacy DasPacket::getDataTypeLegacy() const
{
    // Can't make C union smaller than 1 byte, the intent was:
    // struct DataInfo {
    //   unsigned subpacket_start:1;     //!< The first packet in the train of subpackets
    //   unsigned subpacket_end:1;       //!< Last packet in the train
    //   union {
    //     struct {
    //       unsigned only_neutron_data:1;   //!< Only neutron data, if 0 some metadata is included
    //       unsigned rtdl_present:1;        //!< Is RTDL 6-words data included right after the header? Should be always 1 for newer DSPs
    //     };
    //     DataTypeLegacy data_type_legacy;
    //   };
    //   unsigned unused4:1;             //!< Always zero?
    int type = (datainfo.rtdl_present << 1 | datainfo.only_neutron_data);
    return static_cast<DataTypeLegacy>(type);
}

DasPacket::DataFormat DasPacket::getDataFormat() const
{
    return (isData() ? datainfo.data_format : DATA_FMT_LEGACY);
}

DasDataPacket::EventFormat DasPacket::getEventsFormat() const
{
    switch (getDataFormat()) {
    case DATA_FMT_META:             return DasDataPacket::EVENT_FMT_META;
    case DATA_FMT_PIXEL:            return DasDataPacket::EVENT_FMT_PIXEL;
    case DATA_FMT_LPSD_RAW:         return DasDataPacket::EVENT_FMT_LPSD_RAW;
    case DATA_FMT_LPSD_VERBOSE:     return DasDataPacket::EVENT_FMT_LPSD_VERBOSE;
    case DATA_FMT_AROC_RAW:         return DasDataPacket::EVENT_FMT_AROC_RAW;
    case DATA_FMT_BNL_RAW:          return DasDataPacket::EVENT_FMT_BNL_RAW;
    case DATA_FMT_BNL_VERBOSE:      return DasDataPacket::EVENT_FMT_BNL_VERBOSE;
    case DATA_FMT_CROC_RAW:         return DasDataPacket::EVENT_FMT_CROC_RAW;
    case DATA_FMT_CROC_VERBOSE:     return DasDataPacket::EVENT_FMT_CROC_VERBOSE;
    case DATA_FMT_ACPC_VERBOSE:     return DasDataPacket::EVENT_FMT_ACPC_VERBOSE;
    default:                        return DasDataPacket::EVENT_FMT_INVALID;
    }
}

const DasPacket *DasPacket::cast(const uint8_t *data, size_t size) throw(std::runtime_error)
{
    if (size < sizeof(DasPacket)) {
        std::ostringstream error;
        error << "Not enough data to describe packet header, needed " << sizeof(DasPacket) << " have " << size << " bytes";
        throw std::runtime_error(error.str());
    }

    const DasPacket *packet = reinterpret_cast<const DasPacket *>(data);

    uint32_t packetLen = sizeof(DasPacket) + packet->payload_length;
    if (size < packetLen) {
        std::ostringstream error;
        error << "Not enough data to describe packet, needed " << packetLen << " have " << size << " bytes";
        throw std::runtime_error(error.str());
    }

    if (ALIGN_UP(packet->payload_length, 4) % 4 != 0) {
        throw std::runtime_error("Invalid packet length");
    }

    if (packet->payload_length > 32768) {
        throw std::runtime_error("Packet length out of range");
    }

    return packet;
}

epicsTimeStamp DasPacket::getTimeStamp() const
{
    epicsTimeStamp timestamp{0, 0};
    if (isRtdl()) {
        const RtdlHeader *rtdl = getRtdlHeader();
        if (rtdl) {
            timestamp.secPastEpoch = rtdl->timestamp_sec;
            timestamp.nsec = rtdl->timestamp_nsec;
        }
    }
    return timestamp;
}

std::vector<RtdlPacket::RtdlFrame> DasPacket::getRtdlFrames() const
{
    std::vector<RtdlPacket::RtdlFrame> frames;
    if (isRtdl()) {
        const uint32_t *f = getPayload() + sizeof(RtdlHeader)/sizeof(uint32_t);
        uint32_t nFrames = (getPayloadLength() - sizeof(RtdlHeader)) / sizeof(uint32_t);
        for (uint32_t i = 0; i < nFrames; i++) {
            RtdlPacket::RtdlFrame frame(f[i]);
            frames.push_back(frame);
        }
    }
    return frames;
}

Packet *DasPacket::convert(uint8_t *buffer, size_t size, DasDataPacket::EventFormat eventFormat) const
{
    if (isRtdl()) {
        // Kill data flavor (duplicate) of RTDL packets sent by DSP for daisy-chaining purposes
        if (!isCommand())
            return nullptr;

        auto frames = getRtdlFrames();
        auto t = getTimeStamp();
        auto hdr = getRtdlHeader();
        // Remove frames 1-3
        for (auto it = frames.begin(); it != frames.end(); ) {
            if (it->id >= 1 && it->id <= 3) {
                it = frames.erase(it);
            } else {
                it++;
            }
        }
        frames.insert(frames.begin(), RtdlPacket::RtdlFrame{3, (t.nsec >> 8) & 0xFFFFFF});
        frames.insert(frames.begin(), RtdlPacket::RtdlFrame{2, (t.secPastEpoch & 0xFF) | (hdr->timing_status << 8) | (t.nsec & 0xFF) << 16});
        frames.insert(frames.begin(), RtdlPacket::RtdlFrame{1, (t.secPastEpoch >> 8) & 0xFFFFFF});
        return RtdlPacket::init(buffer, size, frames);

    } else if (isCommand()) {
        // Kill TSYNC commands, we don't need them in new system
        if (getCommandType() == DasPacket::CMD_TSYNC)
            return nullptr;

        if (getCommandType() == DasPacket::CMD_DISCOVER) {
            uint32_t module_type = static_cast<uint32_t>(cmdinfo.module_type);
            return DasCmdPacket::init(buffer,
                                      size,
                                      getSourceAddress(),
                                      DasCmdPacket::CMD_DISCOVER,
                                      0,
                                      true,
                                      cmdinfo.is_response,
                                      0,
                                      sizeof(module_type),
                                      &module_type);
        } else {
            return DasCmdPacket::init(buffer,
                                      size,
                                      getSourceAddress(),
                                      static_cast<DasCmdPacket::CommandType>(getCommandType()),
                                      0,
                                      (cmdinfo.command != DasPacket::RSP_NACK),
                                      cmdinfo.is_response,
                                      (cmdinfo.is_channel ? cmdinfo.channel + 1 : 0),
                                      getPayloadLength(),
                                      getPayload());
        }

    } else if (isData()) {
        DasDataPacket::EventFormat format = getEventsFormat();
        if (format == DasDataPacket::EVENT_FMT_INVALID) {
            format = eventFormat;
        }
        if (format != DasDataPacket::EVENT_FMT_INVALID) {
            epicsTimeStamp timestamp{0, 0};
            const RtdlHeader *rtdl = getRtdlHeader();
            if (rtdl) {
                timestamp = { rtdl->timestamp_sec, rtdl->timestamp_nsec };
            }
            uint32_t bytes = 0;
            const uint32_t *data = getData(&bytes);
            bytes *= sizeof(uint32_t);

            uint32_t count = bytes/DasDataPacket::getEventsSize(format);
            return DasDataPacket::init(buffer, size, format, timestamp, count, data);
        }
    }

    return nullptr;
}
