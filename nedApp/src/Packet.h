/* PacketList.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PACKET_H
#define PACKET_H

#include "RtdlHeader.h"

#include <stdint.h>
#include <cstddef>

/**
 * The packet structure describes any given packet exchanged between plugins.
 */
class Packet {
    public: /* Variables */
        enum PacketType {
            TYPE_LEGACY     = 0x0,
            TYPE_DAS_RTDL   = 0x6,
            TYPE_DAS_DATA   = 0x7,
            TYPE_DAS_CMD    = 0x8,
            TYPE_ACC_TIME   = 0x10,
        };

        struct __attribute__ ((__packed__)) {
            unsigned sequence:8;    //!< Packet sequence number, incremented by sender for each sent packet
            enum PacketType type:8; //!< Packet type
            bool priority:1;        //!< Flag to denote high priority handling, used by hardware to optimize interrupt handling
            unsigned __reserved1:11;
            unsigned version:4;     //<! Packet version
        };

        uint32_t length;            //!< Total number of bytes for this packet

    public: /* Functions */

        /**
         * Allocate and initialize a new packet.
         *
         * @param[in] size in bytes
         * @return Returns a newly created packet or 0 on error.
         */
        static Packet *create(size_t size);
};

class DasRtdlPacket : public Packet {
    public: /* Variables */
        /**
         * Pulse flavor as described in Chapter 1.3.4 of
         * SNS Timing Master Functional System description
         * document.
         */
        enum PulseFlavor {
            RTDL_FLAVOR_NO_BEAM         = 0,    //!< No Beam
            RTDL_FLAVOR_TARGET_1        = 1,    //!< Normal Beam (Target 1)
            RTDL_FLAVOR_TARGET_2        = 2,    //!< Normal Beam (Target 2)
            RTDL_FLAVOR_DIAG_10US       = 3,    //!< 10 uSecond Diagnostic Pulse (not used)
            RTDL_FLAVOR_DIAG_50US       = 4,    //!< 50 uSecond Diagnostic Pulse
            RTDL_FLAVOR_DIAG_100US      = 5,    //!< 100 uSecond Diagnostic Pulse
            RTDL_FLAVOR_PHYSICS_1       = 6,    //!< Special Physics Pulse 1
            RTDL_FLAVOR_PHYSICS_2       = 7,    //!< Special Physics Pulse 2
        };

        /**
         * Previous cycle veto status as described in Chapter 5.1.8 of
         * SNS Timing Master Functional System description
         * document.
         */
        enum CycleVeto {
            RTDL_VETO_NO_BEAM           = (1 << 0), //!< No beam was delivered on the previous pulse.
            RTDL_VETO_NOT_TARGET_1      = (1 << 1), //!< Beam was delivered to target 2 (not to target 1)
            RTDL_VETO_NOT_TARGET_2      = (1 << 2), //!< Beam was delivered to target 1 (not to target 2)
            RTDL_VETO_DIAGNOSTIC_PULSE  = (1 << 3), //!< Beam was a “reduced intensity” diagnostic pulse
            RTDL_VETO_PHYSICS_PULSE_1   = (1 << 4), //!< Beam was one of the special physics study pulses
            RTDL_VETO_PHYSICS_PULSE_2   = (1 << 5), //!< Beam was one of the special physics study pulses (the other type)
            RTDL_VETO_MPS_AUTO_RESET    = (1 << 6), //!< Beam was interrupted by an “Auto Reset” MPS trip (fast protect)
            RTDL_VETO_MPS_FAULT         = (1 << 7), //!< Beam was interrupted or not delivered because of a “Latched” MPS trip
            RTDL_VETO_EVENT_LINK_ERROR  = (1 << 8), //!< Timing system detected corruption on the event link
            RTDL_VETO_RING_RF_SYNCH     = (1 << 9), //!< Timing system has lost synch with the Ring RF signal
            RTDL_VETO_RING_RF_FREQ      = (1 << 10), //!< Measured ring RF frequency is outside acceptable range
            RTDL_VETO_60_HZ_ERROR       = (1 << 11), //!< 60 Hz line phase error is out of tolerance
        };

        struct __attribute__ ((__packed__)) {
            uint8_t num_frames;             //!< Number of RTDL frames included
            uint8_t source;                 //!< Unique source id number
            uint16_t __rtdl_rsv1;
        };

        uint32_t timestamp_sec;             //!< Accelerator time (seconds) of event 39
        uint32_t timestamp_nsec;            //!< Accelerator time (nano-seconds) of event 39

        struct __attribute__ ((__packed__)) {
            unsigned charge:24;         //!< Pulse charge in 10 pC unit
            enum PulseFlavor flavor:6;  //!< Pulse flavor of the next cycle
            unsigned bad:1;             //!< Bad pulse flavor frame
            unsigned unused31:1;        //!< not used

            unsigned cycle:10;          //!< Cycle number
            unsigned last_cycle_veto:12;//!< Last cycle veto
            unsigned tstat:8;           //!< TSTAT
            unsigned bad_cycle_frame:1; //!< Bad cycle frame
            unsigned bad_veto_frame:1;  //!< Bad last cycle veto frame
        } pulse;

        struct __attribute__ ((__packed__)) {
            uint32_t tsync_period;
            struct __attribute__ ((__packed__)) {
                unsigned tof_fixed_offset:24; //!< TOF fixed offset
                unsigned frame_offset:4;    //!< RTDL frame offset
                unsigned unused28:3;        //!< "000"
                unsigned tof_full_offset:1; //!< TOF full offset enabled
            };
        } correction;

        uint32_t frames[0];                 //!< Placeholder for dynamic buffer of RTDL frame data

    public: /* Functions */
        /**
         * Allocates a new RTDL packet for selected payload size.
         *
         * Packet is zeroed out before returned except for the length field.
         *
         * @param[in] size payload size only
         * @return Returns a newly created packet or 0 on error.
         */
        static DasRtdlPacket *create(uint8_t src, const RtdlHeader *hdr, const uint32_t *frames, size_t nFrames);

        /**
         * Initialize packet fields.
         */
        void init(uint8_t src, const RtdlHeader *hdr, const uint32_t *frames, size_t nFrames);

};

class DasCmdPacket : public Packet {
    public: /* Variables */

        static const uint32_t BROADCAST_ID = 0x0;
        static const uint32_t OCC_ID       = 0x0CC;

        /**
         * Module types as returned in discover response.
         */
        enum ModuleType {
            MOD_TYPE_ROC                = 0x20,   //!< ROC (or LPSD) module
            MOD_TYPE_AROC               = 0x21,   //!< AROC
            MOD_TYPE_HROC               = 0x22,
            MOD_TYPE_BNLROC             = 0x25,
            MOD_TYPE_CROC               = 0x29,
            MOD_TYPE_IROC               = 0x2A,
            MOD_TYPE_BIDIMROC           = 0x2B,
            MOD_TYPE_ADCROC             = 0x2D,
            MOD_TYPE_DSP                = 0x30,
            MOD_TYPE_DSPW               = 0x31,
            MOD_TYPE_SANSROC            = 0x40,
            MOD_TYPE_ACPC               = 0xA0,
            MOD_TYPE_ACPCFEM            = 0xA1,
            MOD_TYPE_FFC                = 0xA2,
            MOD_TYPE_FEM                = 0xAA,
        };

        /**
         * Type of commands supported by modules.
         *
         * It's up to the module whether he implements particular command.
         */
        enum CommandType {
            CMD_READ_VERSION            = 0x20, //!< Read module version
            CMD_READ_CONFIG             = 0x21, //!< Read module configuration
            CMD_READ_STATUS             = 0x22, //!< Read module status
            CMD_READ_TEMPERATURE        = 0x23, //!< Read module temperature(s)
            CMD_READ_STATUS_COUNTERS    = 0x24, //!< Read module status counters
            CMD_RESET_STATUS_COUNTERS   = 0x25, //!< Reset module status counters
            CMD_RESET_LVDS              = 0x27, //!< Reset LVDS chips
            CMD_TC_RESET_LVDS           = 0x28, //!< Send a short T&C SysReset signal
            CMD_TC_RESET                = 0x29, //!< Send a long T&C SysReset signal
            CMD_WRITE_CONFIG            = 0x30, //!< Write module configuration
            CMD_WRITE_CONFIG_1          = 0x31, //!< Write module configuration section 1
            CMD_WRITE_CONFIG_2          = 0x32, //!< Write module configuration section 2
            CMD_WRITE_CONFIG_3          = 0x33, //!< Write module configuration section 3
            CMD_WRITE_CONFIG_4          = 0x34, //!< Write module configuration section 4
            CMD_WRITE_CONFIG_5          = 0x35, //!< Write module configuration section 5
            CMD_WRITE_CONFIG_6          = 0x36, //!< Write module configuration section 6
            CMD_WRITE_CONFIG_7          = 0x37, //!< Write module configuration section 7
            CMD_WRITE_CONFIG_8          = 0x38, //!< Write module configuration section 8
            CMD_WRITE_CONFIG_9          = 0x39, //!< Write module configuration section 9
            CMD_WRITE_CONFIG_A          = 0x3A, //!< Write module configuration section A
            CMD_WRITE_CONFIG_B          = 0x3B, //!< Write module configuration section B
            CMD_WRITE_CONFIG_C          = 0x3C, //!< Write module configuration section C
            CMD_WRITE_CONFIG_D          = 0x3D, //!< Write module configuration section D
            CMD_WRITE_CONFIG_E          = 0x3E, //!< Write module configuration section E
            CMD_WRITE_CONFIG_F          = 0x3F, //!< Write module configuration section F
            CMD_HV_SEND                 = 0x50, //!< Send data through RS232 port, HV connected to ROC
            CMD_HV_RECV                 = 0x51, //!< Receive data from RS232 port, HV connected to ROC
            CMD_UPGRADE                 = 0x6F, //!< Send chunk of new firmware data
            CMD_DISCOVER                = 0x80, //!< Discover modules
            CMD_RESET                   = 0x81, //!< Reset of all components
            CMD_START                   = 0x82, //!< Start acquisition
            CMD_STOP                    = 0x83, //!< Stop acquisition
            CMD_PM_PULSE_RQST_ON        = 0x90, //!< Request one pulse for Pulsed Magnet
            CMD_PM_PULSE_RQST_OFF       = 0x91, //!< Clears one pulse request for Pulsed Magnet
            CMD_PREAMP_TEST_CONFIG      = 0x92, //!< Send pulse settings
            CMD_PREAMP_TEST_TRIGGER     = 0x93, //!< Send a single pre-amp pulse request
        };

        struct {
            unsigned cmd_length:12;     //!< Command payload length
            unsigned __reserved2:4;
            enum CommandType command:8; //!< Type of command
            unsigned cmd_sequence:5;    //!< Command sequence id
            bool acknowledge:1;         //!< Flag whether command was succesful, only valid in response
            bool response:1;            //!< Flags this command packet as response
            unsigned lvds_version:1;    //!< LVDS protocol version,
                                        //!< hardware uses this flag ti distinguish protocol in responses
                                        //!< but doesn't use it from optical side, software will reuse
                                        //!< to distinguish DSP and other modules
        };
        uint32_t module_id;             //!< Destination address
        uint32_t payload[0];            //!< Dynamic sized command payload

    public: /* Functions */
        /**
         * Allocates a new command packet for selected payload size.
         *
         * Packet is zeroed out before returned except for the length field.
         *
         * @param[in] size payload size only
         * @return Returns a newly created packet or 0 on error.
         */
        static DasCmdPacket *create(uint32_t moduleId, CommandType cmd, bool ack=false, bool rsp=false, uint8_t ch=0, const uint32_t *payload=0, size_t payloadSize=0);

        /**
         * Initialize packet fields.
         */
        void init(uint32_t moduleId, CommandType cmd, bool ack, bool rsp, uint8_t ch, const uint32_t *payload_, size_t payloadSize);

        /**
         * Return length of inner header in bytes.
         */
        static uint32_t getHeaderLen() { return 6; }
};

#endif // PACKET_H
