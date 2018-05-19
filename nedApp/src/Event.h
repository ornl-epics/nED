/* Event.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

namespace Event {

    /**
     * Structure representing single TOF,pixel event
     */
    struct Pixel {
        static const uint32_t VETO_MASK = 0x80000000;

        uint32_t tof;
        uint32_t pixelid;

        static const uint32_t VETO_MASK = 0x80000000;

        enum class Type {
            NEUTRON      = 0x0,
            UNUSED1      = 0x1,
            UNUSED2      = 0x2,
            UNUSED3      = 0x3,
            BEAM_MONITOR = 0x4,
            SIGNAL       = 0x5,
            ADC          = 0x6,
            CHOPPER      = 0x7,
            VETO_NEUTRON = 0x8,
        };

        Type getType() const {
            return getType(this->pixelid);
        }

        static Type getType(uint32_t pixelid_) {
            return static_cast<Type>((pixelid_ >> 28) & 0x7);
        }
    };

    /**
     * LPSD ROC event formats.
     */
    namespace LPSD {
        /**
         * Structure representing NORMAL mode data packet
         */
        struct Normal : public Event::Pixel {};

        /**
         * Structure representing RAW mode data packet
         */
        struct __attribute((__packed__)) Raw {
            uint32_t tof;       // Regular format
            uint32_t position;  // LPSD ROC position index
            uint16_t sample_a1;
            uint16_t sample_b1;
            uint16_t sample_b2;
            uint16_t sample_a2;
        };

        /**
         * Structure representing RAW mode data packet
         */
        struct __attribute((__packed__)) Verbose {
            uint32_t tof;       // Regular format
            uint32_t position;  // LPSD ROC position index
            uint16_t sample_a1;
            uint16_t sample_b1;
            uint16_t sample_b2;
            uint16_t sample_a2;
            uint32_t pixelid;
        };

        /**
         * Event structure that includes everything about event, generated and used by software only.
         */
        struct Diag : public Verbose {
            uint32_t pixelid_raw; // Pixel as received from detector when mapping enabled, same as pixelid otherwise.

            /**
             * Copy fields from Raw event.
             */
            Diag& operator=(const Raw &raw);
        };
    };

    /**
     * BNL ROC event formats.
     */
    namespace BNL {
        /**
         * Structure representing NORMAL mode data packet
         */
        typedef struct {
            uint32_t tof;       // Regular format
            uint32_t position;  // BNL ROC position index
            uint32_t x;         // Interpolated X position in Q5.11 format
            uint32_t y;         // Interpolated Y position in Q5.11 format
        } Normal;

        /**
         * Structure representing RAW mode data packet
         */
        struct __attribute((__packed__)) Raw {
            uint32_t tof;
            uint32_t position;
#ifdef BITFIELD_LSB_FIRST
            uint16_t sample_x1;
            uint16_t sample_y1;
            uint16_t sample_x2;
            uint16_t sample_y2;
            uint16_t sample_x3;
            uint16_t sample_y3;
            uint16_t sample_x4;
            uint16_t sample_y4;
            uint16_t sample_x5;
            uint16_t sample_y5;
            uint16_t sample_x6;
            uint16_t sample_y6;
            uint16_t sample_x7;
            uint16_t sample_y7;
            uint16_t sample_x8;
            uint16_t sample_y8;
            uint16_t sample_x9;
            uint16_t sample_y9;
            uint16_t sample_x10;
            uint16_t sample_y10;
            uint16_t sample_x11;
            uint16_t sample_y11;
            uint16_t sample_x12;
            uint16_t sample_y12;
            uint16_t sample_x13;
            uint16_t sample_y13;
            uint16_t sample_x14;
            uint16_t sample_y14;
            uint16_t sample_x15;
            uint16_t sample_y15;
            uint16_t sample_x16;
            uint16_t sample_y16;
            uint16_t sample_x17;
            uint16_t sample_y17;
            uint16_t sample_x18;
            uint16_t sample_x19;
            uint16_t sample_x20;
            uint16_t _padding;
#endif
        };

        /**
         * Structure representing Extended mode data packet
         */
        struct __attribute((__packed__)) Verbose {
            uint32_t tof;
            uint32_t pixelid;
#ifdef BITFIELD_LSB_FIRST
            uint16_t sample_x1;
            uint16_t sample_y1;
            uint16_t sample_x2;
            uint16_t sample_y2;
            uint16_t sample_x3;
            uint16_t sample_y3;
            uint16_t sample_x4;
            uint16_t sample_y4;
            uint16_t sample_x5;
            uint16_t sample_y5;
            uint16_t sample_x6;
            uint16_t sample_y6;
            uint16_t sample_x7;
            uint16_t sample_y7;
            uint16_t sample_x8;
            uint16_t sample_y8;
            uint16_t sample_x9;
            uint16_t sample_y9;
            uint16_t sample_x10;
            uint16_t sample_y10;
            uint16_t sample_x11;
            uint16_t sample_y11;
            uint16_t sample_x12;
            uint16_t sample_y12;
            uint16_t sample_x13;
            uint16_t sample_y13;
            uint16_t sample_x14;
            uint16_t sample_y14;
            uint16_t sample_x15;
            uint16_t sample_y15;
            uint16_t sample_x16;
            uint16_t sample_y16;
            uint16_t sample_x17;
            uint16_t sample_y17;
            uint16_t sample_x18;
            uint16_t sample_x19;
            uint16_t sample_x20;
            uint16_t _padding;
#endif
            uint32_t x;             // Interpolated X position in Q5.11 format
            uint32_t y;             // Interpolated Y position in Q5.11 format
        };

        /**
         * Event structure that includes everything about event, generated and used by software only.
         */
        struct Diag : public Verbose {
            double corrected_x;     // Interpolated and corrected X position
            double corrected_y;     // Interpolated and corrected Y position
            uint32_t position;      // Detector id
            uint32_t pixelid_raw;   // Pixel as received from detector when mapping enabled, same as pixelid otherwise.

            /**
             * Copy fields from Raw event.
             */
            Diag& operator=(const Raw &raw);
        };
    }

    /**
     * CROC event formats.
     */
    namespace CROC {
        typedef enum {
            VETO_NO                 = 0,
            VETO_OUT_OF_RANGE       = (1  << 22) | 0x80000000,
            VETO_TIMERANGE_OVERFLOW = (2  << 22) | 0x80000000, //!< Error code, sum of time ranges over threshold
            VETO_TIMERANGE_COUNT    = (3  << 22) | 0x80000000, //!< Error code, time range count
            VETO_Y_LOW_SIGNAL       = (4  << 22) | 0x80000000, //!< Error code, low signal on Y
            VETO_Y_HIGH_SIGNAL      = (5  << 22) | 0x80000000, //!< Error code, high signal on Y
            VETO_Y_NON_ADJACENT     = (6  << 22) | 0x80000000,
            VETO_X_LOW_SIGNAL       = (7  << 22) | 0x80000000, //!< Error code, low signal on X
            VETO_X_HIGH_SIGNAL      = (8  << 22) | 0x80000000, //!< Error code, high signal on X
            VETO_G_NON_ADJACENT     = (9  << 22) | 0x80000000,
            VETO_G_HIGH_SIGNAL      = (10 << 22) | 0x80000000, //!< More than one signal in single event
            VETO_G_GHOST            = (11 << 22) | 0x80000000,
            VETO_G_LOW_SIGNAL       = (12 << 22) | 0x80000000,
            VETO_INVALID_POSITION   = (13 << 22) | 0x80000000, //!< Unknown position
            VETO_INVALID_CALC       = (14 << 22) | 0x80000000, //!< Unknown calculation mode
            VETO_ECHO               = (15 << 22) | 0x80000000, //!< Echo event detected
            VETO_TIMERANGE_BAD      = (16 << 22) | 0x80000000, //!< Time range is odd
            VETO_TIMERANGE_DELAYED  = (17 << 22) | 0x80000000, //!< Delayed event based on time range
        } VetoType;

        /**
         * Structure representing RAW mode data packet
         */
        struct __attribute((__packed__)) Raw {
            uint32_t tof;
            uint32_t position;
#ifdef BITFIELD_LSB_FIRST
            uint16_t time_range[4];
            uint8_t photon_count_y[7];
            uint8_t photon_count_x[11];
            uint8_t photon_count_g[14];
#endif
        };

        /**
         * Structure representing VERBOSE mode data packet
         */
        struct __attribute((__packed__)) Verbose {
            uint32_t tof;
            uint32_t position;
#ifdef BITFIELD_LSB_FIRST
            uint16_t time_range[4];
            uint8_t photon_count_y[7];
            uint8_t photon_count_x[11];
            uint8_t photon_count_g[14];
#endif
            uint32_t pixelid;
        };

        /**
         * Structure representing Diagnostic mode data packet
         */
        struct Diag : public Verbose {
            double x;               // Interpolated X position
            double y;               // Interpolated Y position
            double corrected_x;     // Interpolated and corrected X position
            double corrected_y;     // Interpolated and corrected Y position
            uint32_t pixelid_raw;   // Mapped pixel id

            /**
             * Copy fields from Raw event.
             */
            Diag& operator=(const Raw &raw);
        };
    };

    /**
     * ACPC event formats.
     */
    namespace ACPC {
        /**
         * Structure representing NORMAL mode data packet (by ACPC)
         */
        struct __attribute((__packed__)) Normal {
            uint32_t tof;
            uint32_t position;
            uint32_t x;             // Interpolated X position in Q8.24 format
            uint32_t y;             // Interpolated Y position in Q8.24 format
            uint32_t photo_sum_x;   // X photo sum in Q17.15 format
            uint32_t photo_sum_y;   // Y photo sum in Q17.15 format
        };

        // TODO: verbose mode from ACPC

        /**
         * Structure representing Diagnostic mode data packet
         */
        struct Diag : public Normal {
            double corrected_x;     // Interpolated and corrected X position
            double corrected_y;     // Interpolated and corrected Y position
            uint32_t pixelid;       // Pixel id
            uint32_t pixelid_raw;   // Mapped pixel id

            /**
             * Copy fields from Raw event.
             */
            Diag& operator=(const Normal &normal);
        };
    };

    namespace AROC {
        /**
         * Structure representing AROC RAW mode data packet
         */
        struct __attribute((__packed__)) Raw {
            uint32_t tof;
            uint32_t position;
            uint32_t sample_a[8];
            uint32_t sample_b[8];
        };
    };
};

#endif // EVENT_H
