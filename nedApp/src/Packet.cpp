/* Packet.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "Packet.h"

#include <string.h>
#include <assert.h>

DasRtdlPacket *DasRtdlPacket::create(uint8_t src, const RtdlHeader *hdr, const uint32_t *frames, size_t nFrames)
{
    assert(nFrames == 26);
    DasRtdlPacket *packet = reinterpret_cast<DasRtdlPacket*>(malloc(sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t)));
    if (packet) {
        packet->length = sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t);
        packet->init(src, hdr, frames, nFrames);
    }
    return packet;
}

void DasRtdlPacket::init(uint8_t src, const RtdlHeader *hdr, const uint32_t *frames, size_t nFrames)
{
    assert(this->length >= (sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t)));
    memset(this, 0, (sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t)));

    this->version = 0x1;
    this->type = TYPE_DAS_RTDL;
    this->length = sizeof(DasRtdlPacket) + nFrames*sizeof(uint32_t);

    this->source = src;
    this->num_frames = nFrames;

    // Use memcpy for performance reasons
    memcpy(&this->timestamp_sec, hdr, sizeof(RtdlHeader));
    memcpy(this->frames, frames, nFrames*sizeof(uint32_t));
}

DasCmdPacket *DasCmdPacket::create(uint32_t moduleId, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload, size_t payloadSize)
{
    assert(payloadSize < 1024*1024);
    DasCmdPacket *packet = reinterpret_cast<DasCmdPacket*>(malloc(sizeof(DasCmdPacket) + ALIGN_UP(payloadSize, 4)));
    if (packet) {
        packet->init(moduleId, cmd, ack, rsp, ch, payload, payloadSize);
    }
    return packet;
}

void DasCmdPacket::init(uint32_t moduleId, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload_, size_t payloadSize)
{
    uint32_t packetLength = sizeof(DasCmdPacket) + ALIGN_UP(payloadSize, 4);
    memset(this, 0, packetLength);

    this->version = 0x1;
    this->type = TYPE_DAS_CMD;
    this->length = packetLength;

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
