/* DasPacket.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BNL_DATA_PACKET_HPP
#define BNL_DATA_PACKET_HPP

#include <cinttypes>

class BnlDataPacket {
    public:
        /**
         * Structure representing NORMAL mode data packet
         */
        typedef struct {
            uint32_t tof;       // Regular format
            uint32_t position;  // BNL ROC position index
            uint32_t x;         // Interpolated X position in Q8.24 format
            uint32_t y;         // Interpolated Y position in Q8.24 format
            uint32_t unused1;   // Placeholder for photosum, not applicable for BNL
            uint32_t unused2;   // Placeholder for photosum, not applicable for BNL
        } NormalEvent;

        /**
         * Structure representing RAW mode data packet
         */
        typedef struct __attribute((__packed__)) {
            uint32_t tof;
            uint32_t position;
#ifdef BITFIELD_LSB_FIRST
            uint16_t sample_y1;
            uint16_t sample_x1;
            uint16_t sample_y2;
            uint16_t sample_x2;
            uint16_t sample_y3;
            uint16_t sample_x3;
            uint16_t sample_y4;
            uint16_t sample_x4;
            uint16_t sample_y5;
            uint16_t sample_x5;
            uint16_t sample_y6;
            uint16_t sample_x6;
            uint16_t sample_y7;
            uint16_t sample_x7;
            uint16_t sample_y8;
            uint16_t sample_x8;
            uint16_t sample_y9;
            uint16_t sample_x9;
            uint16_t sample_y10;
            uint16_t sample_x10;
            uint16_t sample_y11;
            uint16_t sample_x11;
            uint16_t sample_y12;
            uint16_t sample_x12;
            uint16_t sample_y13;
            uint16_t sample_x13;
            uint16_t sample_y14;
            uint16_t sample_x14;
            uint16_t sample_y15;
            uint16_t sample_x15;
            uint16_t sample_y16;
            uint16_t sample_x16;
            uint16_t sample_y17;
            uint16_t sample_x17;
            uint16_t sample_x18;
            uint16_t sample_x19;
            uint16_t sample_x20;
            uint16_t _padding;
#endif
        } RawEvent;
};

#endif // BNL_DATA_PACKET_HPP
