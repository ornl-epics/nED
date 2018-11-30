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

const Packet *Packet::cast(const uint8_t *data, size_t size)
{
    if (size < sizeof(Packet)) {
        std::ostringstream error;
        error << "Not enough data to describe packet header, needed " << sizeof(Packet) << " have " << size << " bytes";
        throw ParseError(error.str());
    }

    const Packet *packet = reinterpret_cast<const Packet *>(data);

    if (size < packet->length) {
        std::ostringstream error;
        error << "Not enough data to describe packet, needed " << packet->length << " have " << size << " bytes";
        throw std::runtime_error(error.str());
    }

    if (ALIGN_UP(packet->length, 4) % 4 != 0) {
        throw std::runtime_error("Invalid packet length");
    }

    if (packet->length > 0xFFFFFF) {
        throw ParseError("Packet length out of range");
    }

    return packet;
}

/* ******************************* */
/* *** RtdlPacket functions *** */
/* ******************************* */

RtdlPacket *RtdlPacket::init(uint8_t *buffer, size_t size, const std::vector<RtdlFrame> &frames)
{
    RtdlPacket *packet = nullptr;
    uint32_t length = sizeof(RtdlPacket) + (frames.size() * sizeof(RtdlFrame));
    if (size >= length) {
        packet = reinterpret_cast<RtdlPacket *>(buffer);
        packet->init(frames);
    }

    return packet;
}

void RtdlPacket::init(const std::vector<RtdlFrame> &frames)
{
    memset(this, 0, (sizeof(RtdlPacket) + frames.size()*sizeof(RtdlFrame)));

    this->version = 0x1;
    this->type = TYPE_RTDL;
    this->length = sizeof(RtdlPacket) + frames.size()*sizeof(RtdlFrame);

    this->num_frames = frames.size();
    for (size_t i = 0; i < frames.size(); i++) {
        this->frames[i].raw = frames[i].raw;
    }
}

bool RtdlPacket::checkIntegrity() const
{
    if (this->length < sizeof(RtdlPacket))
        return false;
    if (this->length < RtdlPacket::getLength(num_frames))
        return false;
    if (getTimeStamp().nsec > 1000000000)
        return false;
    return true;
}

epicsTimeStamp RtdlPacket::getTimeStamp() const
{
    uint32_t secPastEpoch = 0;
    uint32_t nsec = 0;
    unsigned check = 0;

    for (uint32_t i = 0; i < this->num_frames; i++) {
        switch (this->frames[i].id) {
        case 1:
            secPastEpoch |= (this->frames[i].data << 8);
            check += 1;
            break;
        case 2:
            secPastEpoch |= (this->frames[i].data & 0xFF);
            nsec |= ((this->frames[i].data >> 16) & 0xFF);
            check += 2;
            break;
        case 3:
            nsec |= (this->frames[i].data << 8);
            check += 4;
            break;
        default:
            break;
        }
    }
    if (check != 7) {
        secPastEpoch = nsec = 0;
    }
    return { secPastEpoch, nsec };
}

RtdlHeader RtdlPacket::getRtdlHeader() const
{
    RtdlHeader hdr{0};

    epicsTimeStamp timestamp = getTimeStamp();
    hdr.timestamp_sec = timestamp.secPastEpoch;
    hdr.timestamp_nsec = timestamp.nsec;

    for (const auto& frame: getRtdlFrames()) {
        switch (frame.id) {
        case 2:
            hdr.timing_status = (frame.data >> 8) & 0xFF;
            break;
        case 17:
            hdr.pulse.flavor = static_cast<RtdlHeader::PulseFlavor>(frame.data & 0x3F);
            break;
        case 24:
            hdr.last_cycle_veto = (frame.data & 0xFFF);
            break;
        case 25:
            hdr.cycle = (frame.data & 0x3FF);
            break;
        case 35:
            hdr.pulse.charge = (frame.data & 0xFFFFFF);
            break;
        default:
            break;
        }
    }
    return hdr;
}

double RtdlPacket::getProtonCharge() const
{
    for (const auto& frame: getRtdlFrames()) {
        if (frame.id == 35)
            return (frame.data & 0xFFFFFF) * 10e-12;
    }
    return -1.0;
}

/* ******************************* */
/* *** DasCmdPacket functions  *** */
/* ******************************* */

DasCmdPacket *DasCmdPacket::init(uint8_t *buffer, size_t size, uint32_t moduleId, CommandType cmd, uint8_t cmd_ver, bool ack, bool rsp, uint8_t ch, size_t payloadSize, const uint32_t *payload_)
{
    DasCmdPacket *packet = nullptr;
    uint32_t packetLength = sizeof(DasCmdPacket) + ALIGN_UP(payloadSize, 4);
    if (size >= packetLength) {
        packet = reinterpret_cast<DasCmdPacket *>(buffer);
        packet->init(moduleId, cmd, cmd_ver, ack, rsp, ch, payloadSize, payload_);
    }
    return packet;
}

void DasCmdPacket::init(uint32_t moduleId, CommandType cmd, uint8_t cmd_ver, bool ack, bool rsp, uint8_t ch, size_t payloadSize, const uint32_t *payload_)
{
    memset(this, 0, sizeof(DasCmdPacket));

    this->version = 0x1;
    this->type = TYPE_DAS_CMD;
    this->length = sizeof(DasCmdPacket) + ALIGN_UP(payloadSize, 4);

    this->module_id = moduleId;
    if (ch > 0)
        this->cmd_id = ch;
    this->acknowledge = ack;
    this->response = rsp;
    this->command = cmd;
    this->cmd_length = payloadSize + 6; // command length is considered payload + 32 bit address + 16 bit command description
    this->cmd_version = (cmd_ver > 1);
    if (payload_ != nullptr) {
        memcpy(this->payload, payload_, payloadSize);
    }
}

bool DasCmdPacket::checkIntegrity() const
{
    if (this->length < sizeof(DasCmdPacket))
        return false;
    if (this->length < (sizeof(DasCmdPacket) - 6 + cmd_length))
        return false;
    return true;
}

uint32_t DasCmdPacket::getCmdPayloadLength() const
{
    if (this->length < (sizeof(DasCmdPacket) + (this->cmd_length - 6)))
        return 0;

    return this->cmd_length - 6;
}

std::string DasCmdPacket::getModuleIdStr() const
{
    char buf[16] = "";
    snprintf(buf, sizeof(buf)-1, "%d.%d.%d.%d", (module_id >> 24) & 0xFF, (module_id >> 16) & 0xFF, (module_id >> 8) & 0xFF, module_id & 0xFF);
    buf[sizeof(buf)-1] = '\0';
    return std::string(buf);
}

/* ******************************* */
/* *** DasDataPacket functions *** */
/* ******************************* */

DasDataPacket *DasDataPacket::init(uint8_t *buffer, size_t size, EventFormat format, const epicsTimeStamp &timestamp, uint32_t count, const void *data)
{
    DasDataPacket *packet = nullptr;
    uint32_t packetLength = sizeof(DasDataPacket) + count*getEventsSize(format);
    if (size >= packetLength) {
        packet = reinterpret_cast<DasDataPacket *>(buffer);
        packet->init(format, timestamp, count, data);
    }
    return packet;
}

void DasDataPacket::init(EventFormat format, const epicsTimeStamp &timestamp, uint32_t count, const void *data)
{
    uint32_t dataBytes = count*getEventsSize(format);
    memset(this, 0, sizeof(DasDataPacket));

    this->version = 0x1;
    this->type = TYPE_DAS_DATA;
    this->length = sizeof(DasDataPacket) + dataBytes;

    this->event_format = format;
    this->num_events = count;
    this->timestamp_sec = timestamp.secPastEpoch;
    this->timestamp_nsec = timestamp.nsec;
    if (data != nullptr) {
        memcpy(this->events, data, dataBytes);
    }
}

bool DasDataPacket::checkIntegrity() const
{
    if (this->length < sizeof(DasDataPacket))
        return false;
    if (this->length < DasDataPacket::getLength(event_format, num_events))
        return false;
    if (this->getTimeStamp().nsec > 1000000000)
        return false;
    return true;
}

uint32_t DasDataPacket::getEventsSize(DasDataPacket::EventFormat format)
{
    switch (format) {
        case EVENT_FMT_META:         return sizeof(Event::Pixel);
        case EVENT_FMT_PIXEL:        return sizeof(Event::Pixel);
        case EVENT_FMT_BNL_RAW:      return sizeof(Event::BNL::Raw);
        case EVENT_FMT_BNL_VERBOSE:  return sizeof(Event::BNL::Verbose);
        case EVENT_FMT_BNL_DIAG:     return sizeof(Event::BNL::Diag);
        case EVENT_FMT_ACPC_XY_PS:   return sizeof(Event::ACPC::Normal);
        case EVENT_FMT_ACPC_DIAG:    return sizeof(Event::ACPC::Diag);
        case EVENT_FMT_TIME_CALIB:   return sizeof(Event::Pixel);
        case EVENT_FMT_AROC_RAW:     return sizeof(Event::AROC::Raw);
        case EVENT_FMT_LPSD_RAW:     return sizeof(Event::LPSD::Raw);
        case EVENT_FMT_LPSD_VERBOSE: return sizeof(Event::LPSD::Verbose);
        default:
            throw std::runtime_error("Packet::getEventsSize() not implemented for requested format");
    }
}
