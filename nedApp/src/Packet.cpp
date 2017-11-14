/* Packet.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Packet.h"

#include <string.h>
#include <stdlib.h>
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
    memcpy(this->payload, frames, nFrames*sizeof(uint32_t));
}

DasCmdPacket *DasCmdPacket::create(uint32_t src, uint32_t dest, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload, size_t payloadSize)
{
    assert(payloadSize < 1024*1024);
    DasCmdPacket *packet = reinterpret_cast<DasCmdPacket*>(malloc(sizeof(DasCmdPacket) + payloadSize));
    if (packet) {
        packet->length = sizeof(DasCmdPacket) + payloadSize;
        packet->init(src, dest, cmd, ack, rsp, ch, payload, payloadSize);
    }
    return packet;
}

void DasCmdPacket::init(uint32_t src, uint32_t dest, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload_, size_t payloadSize)
{
    assert(this->length >= (sizeof(DasCmdPacket) + payloadSize));
    memset(this, 0, (sizeof(DasCmdPacket) + payloadSize));

    this->version = 0x1;
    this->type = TYPE_DAS_CMD;
    this->length = sizeof(DasCmdPacket) + payloadSize;

    this->source = src;
    this->destination = dest;
    if (ch > 0)
        this->channel = ch;
    this->acknowledge = ack;
    this->response = rsp;
    this->command = cmd;
    memcpy(this->payload, payload_, payloadSize);
}