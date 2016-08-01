/* DasPacket.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef CROC_DATA_PACKET_HPP
#define CROC_DATA_PACKET_HPP

#include <DasPacket.h>

#include <cinttypes>

class CRocDataPacket {
    public:
        /**
         * Structure representing NORMAL mode data packet
         */
        typedef DasPacket::Event NormalEvent;

        /**
          * Structure representing XY mode data packet - software only
          */
        typedef struct __attribute((__packed__)) {
            uint32_t tof;       // Time of flight
            uint32_t position;  // CROC position index
            uint32_t x;         // Interpolated X position in Q8.24 format
            uint32_t y;         // Interpolated Y position in Q8.24 format
        } XyEvent;

        /**
         * Structure representing RAW mode data packet
         */
        typedef struct __attribute((__packed__)) {
            uint32_t tof;
            uint32_t position;
#ifdef BITFIELD_LSB_FIRST
            uint16_t time_range[4];
            uint8_t photon_count_y[7];
            uint8_t photon_count_x[11];
            uint8_t photon_count_g[14];
#endif
        } RawEvent;

        /**
         * Structure representing Extended mode data packet
         */
        typedef struct __attribute((__packed__)) {
            uint32_t tof;
            uint32_t position;
#ifdef BITFIELD_LSB_FIRST
            uint16_t time_range[4];
            uint8_t photon_count_y[7];
            uint8_t photon_count_x[11];
            uint8_t photon_count_g[14];
#endif
            uint32_t pixelid;
        } ExtendedEvent;

        typedef enum {
            VETO_NO                 = 0,
            VETO_OUT_OF_RANGE       = (1  << 22) | 0x80000000,
            VETO_TIMERANGE_OVERFLOW = (2  << 22) | 0x80000000,//!< Error code, sum of time ranges over threshold
            VETO_TIMERANGE_COUNT    = (3  << 22) | 0x80000000,   //!< Error code, time range count
            VETO_Y_LOW_SIGNAL       = (4  << 22) | 0x80000000,      //!< Error code, low signal on Y
            VETO_Y_HIGH_SIGNAL      = (5  << 22) | 0x80000000,     //!< Error code, high signal on Y
            VETO_Y_NON_ADJACENT     = (6  << 22) | 0x80000000,
            VETO_X_LOW_SIGNAL       = (7  << 22) | 0x80000000,      //!< Error code, low signal on X
            VETO_X_HIGH_SIGNAL      = (8  << 22) | 0x80000000,     //!< Error code, high signal on X
            VETO_G_NON_ADJACENT     = (9  << 22) | 0x80000000,
            VETO_G_HIGH_SIGNAL      = (10 << 22) | 0x80000000,           //!< More than one signal in single event
            VETO_G_GHOST            = (11 << 22) | 0x80000000,
            VETO_G_LOW_SIGNAL       = (12 << 22) | 0x80000000,
            VETO_INVALID_POSITION   = (13 << 22) | 0x80000000, //!< Unknown position
            VETO_INVALID_CALC       = (14 << 22) | 0x80000000, //!< Unknown calculation mode
            VETO_ECHO               = (15 << 22) | 0x80000000, //!< Echo event detected
            VETO_TIMERANGE_BAD      = (16 << 22) | 0x80000000, //!< Time range is odd
            VETO_TIMERANGE_DELAYED  = (17 << 22) | 0x80000000, //!< Delayed event based on time range
        } VetoType;
};

#endif // CROC_DATA_PACKET_HPP
