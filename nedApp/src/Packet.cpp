/* Packet.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "Event.h"
#include "Packet.h"

#include <string.h>
#include <assert.h>
#include <sstream>

Packet::Packet(uint8_t version_, Type type_, uint32_t length_)
: sequence(0)
, priority(false)
, type(type_)
, version(version_)
, length(length_ < sizeof(Packet) ? sizeof(Packet) : length_)
{}

Packet::Packet(const Packet *orig)
{
    memcpy(this, orig, orig->length);
}

const Packet *Packet::cast(const uint8_t *data, size_t size) throw(ParseError)
{
    if (size < sizeof(Packet)) {
        std::ostringstream error;
        error << "Not enough data to describe packet header, needed " << sizeof(Packet) << " have " << size << " bytes";
        throw ParseError(error.str());
    }

    const Packet *packet = reinterpret_cast<const Packet *>(data);

    if (packet->length == ALIGN_UP(packet->length, 4)) {
        throw ParseError("Invalid packet length");
    }

    if (packet->length > 0xFFFFFF) {
        throw ParseError("Packet length out of range");
    }

    return packet;
}

DasRtdlPacket *DasRtdlPacket::init(uint8_t *buffer, size_t size, const RtdlHeader *hdr, const uint32_t *frames, size_t nFrames)
{
    DasRtdlPacket *packet = nullptr;
    uint32_t length = sizeof(DasRtdlPacket) + (nFrames * sizeof(uint32_t));
    if (size >= length) {
        packet = reinterpret_cast<DasRtdlPacket *>(buffer);
        packet->init(hdr, frames, nFrames);
    }
    
    return packet;
}

void DasRtdlPacket::init(const RtdlHeader *hdr, const uint32_t *frames, size_t nFrames)
{
    memset(this, 0, (sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t)));

    this->version = 0x1;
    this->type = TYPE_DAS_RTDL;
    this->length = sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t);

    this->num_frames = nFrames;

    // Use memcpy for performance reasons
    memcpy(&this->timestamp_sec, hdr, sizeof(RtdlHeader));
    memcpy(this->frames, frames, nFrames*sizeof(uint32_t));
}

DasCmdPacket *DasCmdPacket::init(uint8_t *buffer, size_t size, uint32_t moduleId, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload_, size_t payloadSize)
{
    DasCmdPacket *packet = nullptr;
    uint32_t packetLength = sizeof(DasCmdPacket) + ALIGN_UP(payloadSize, 4);
    if (size > packetLength) {
        packet = reinterpret_cast<DasCmdPacket *>(buffer);
        packet->init(moduleId, cmd, ack, rsp, ch, payload_, payloadSize);
    }
    return packet;
}

void DasCmdPacket::init(uint32_t moduleId, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload_, size_t payloadSize)
{
    memset(this, 0, sizeof(DasCmdPacket));

    this->version = 0x1;
    this->type = TYPE_DAS_CMD;
    this->length = sizeof(DasCmdPacket) + ALIGN_UP(payloadSize, 4);

    this->module_id = moduleId;
    if (ch > 0)
        this->cmd_sequence = ch;
    this->acknowledge = ack;
    this->response = rsp;
    this->command = cmd;
    this->cmd_length = payloadSize + 6; // command length is considered payload + 32 bit address + 16 bit command description
    this->lvds_version = 1;
    memcpy(this->payload, payload_, payloadSize);
}

uint32_t DasCmdPacket::getPayloadLength() const
{
    if (this->cmd_length < (sizeof(DasCmdPacket) - 6))
        return 0;
    if (this->length < (sizeof(DasCmdPacket) - 6 + this->cmd_length))
        return 0;

    return this->cmd_length - 6;
}

DasDataPacket *DasDataPacket::init(uint8_t *buffer, size_t size, DataFormat format, uint32_t time_sec, uint32_t time_nsec, const uint32_t *data, uint32_t count)
{
    DasDataPacket *packet = nullptr;
    uint32_t packetLength = sizeof(DasDataPacket) + count*4;
    if (size >= packetLength) {
        packet = reinterpret_cast<DasDataPacket *>(buffer);
        packet->init(format, time_sec, time_nsec, data, count);
    }
    return packet;
}

void DasDataPacket::init(DataFormat format, uint32_t time_sec, uint32_t time_nsec, const uint32_t *data, uint32_t count)
{
    memset(this, 0, sizeof(DasDataPacket));

    this->version = 0x1;
    this->type = TYPE_DAS_DATA;
    this->length = sizeof(DasDataPacket) + count*4;

    this->format = format;
    this->timestamp_sec = time_sec;
    this->timestamp_nsec = time_nsec;
    memcpy(this->events, data, count*4);
}

uint32_t DasDataPacket::getNumEvents() const
{
    uint32_t payloadLen = this->length - sizeof(DasDataPacket);
    uint32_t eventSize = getEventsSize();
    if (eventSize == 0)
        return 0;
    return payloadLen / eventSize;
}

uint32_t DasDataPacket::getEventsSize() const
{
    switch (this->format) {
        case DATA_FMT_RESERVED:     return sizeof(Event::Pixel);
        case DATA_FMT_META:         return sizeof(Event::Pixel);
        case DATA_FMT_PIXEL:        return sizeof(Event::Pixel);
        case DATA_FMT_LPSD_RAW:     return 0; // TODO
        case DATA_FMT_LPSD_VERBOSE: return 0; // TODO
        case DATA_FMT_ACPC_XY_PS:   return 0; // TODO
        case DATA_FMT_ACPC_RAW:     return 0; // TODO
        case DATA_FMT_ACPC_VERBOSE: return 0; // TODO
        case DATA_FMT_AROC_RAW:     return 0; // TODO
        case DATA_FMT_BNL_XY:       return 0; // TODO
        case DATA_FMT_BNL_RAW:      return 0; // TODO
        case DATA_FMT_BNL_VERBOSE:  return 0; // TODO
        case DATA_FMT_CROC_RAW:     return 0; // TODO
        case DATA_FMT_CROC_VERBOSE: return 0; // TODO
        default:                    return 0;
    }
}
