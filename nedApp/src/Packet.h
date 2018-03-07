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

#include <cassert>
#include <epicsTime.h>
#include <stdint.h>
#include <stdexcept>
#include <cstddef>
#include <vector>

/**
 * The packet structure describes any given packet exchanged between plugins.
 */
class Packet {
    public: /* Variables */
        typedef enum {
            TYPE_LEGACY     = 0x0,
            TYPE_ERROR      = 0x1,
            TYPE_RTDL       = 0x6,
            TYPE_DAS_DATA   = 0x7,
            TYPE_DAS_CMD    = 0x8,
            TYPE_ACC_TIME   = 0x10,
            TYPE_OLD_RTDL   = 0xFF, // Software only, hopefully such packet doesn't get defined
        } Type;

        using ParseError = std::runtime_error;

    protected:

        struct __attribute__ ((__packed__)) {
            unsigned sequence:8;    //!< Packet sequence number, incremented by sender for each sent packet
            bool priority:1;        //!< Flag to denote high priority handling, used by hardware to optimize interrupt handling
            unsigned __reserved1:11;
            Type type:8;            //!< Packet type
            unsigned version:4;     //<! Packet version
        };

        uint32_t length;            //!< Total number of bytes for this packet

    public: /* Functions */

        /**
         * Default constructor.
         */
        Packet(uint8_t version, Type type, uint32_t length);

        /**
         * Copy constructor, makes byte-by-byte copy of original.
         */
        Packet(const Packet *orig);

        /**
         * Cast raw pointer to Packet pointer.
         * 
         * The function tries to interpret raw data as a valid Packet,
         * performing several checks including but not limited to:
         * - checking minimum/maximum packet size requirements
         * - checking enough memory is allocated for a packet
         *
         * @return Casted valid packet, throws otherwise
         */
        static const Packet *cast(const uint8_t *data, size_t size) throw(ParseError);

        /**
         * Decode and return packet version as defined in header.
         *
         * @return packet version.
         */
        uint8_t getVersion() const {
            return version;
        }
        uint8_t getVersion() {
            return version;
        }

        /**
         * Set packet version.
         */
        void setVersion(uint8_t ver) {
            version = ver & 0xF;
        }

        /**
         * Decode and return packet type as defined in header.
         *
         * @return packet type.
         */
        Type getType() const {
            return type;
        }
        Type getType() {
            return type;
        }

        /**
         * Return packet length in bytes.
         *
         */
        uint32_t getLength() const {
            return length;
        }
        uint32_t getLength() {
            return length;
        }

        /**
         * Return packet sequence number as set by the sender.
         *
         * Number 0 means either value rolled over or sender does not support
         * setting sequence number.
         */
        uint8_t getSequenceId() const {
            return sequence;
        }
        uint8_t getSequenceId() {
            return sequence;
        }

        /**
         * Set new sequence id of the packet.
         */
        void setSequenceId(uint8_t sequence_) {
            sequence = sequence_;
        }

        /**
         * Is this a priority packet?
         */
        bool isPriority() const {
            return priority;
        }
        bool isPriority() {
            return priority;
        }

        /**
         * Set priority flag to this packet.
         */
        void setPriority(bool enable) {
            priority = enable;
        }
};

/**
 * Error packet is produced by the receiver hardware when incoming packet is not valid.
 */
class ErrorPacket : public Packet {
    public:
        typedef enum {
            TYPE_NO_ERROR   = 0x0,
            TYPE_ERR_FRAME  = 0x1,
            TYPE_ERR_LENGTH = 0x2,
            TYPE_ERR_CRC    = 0x3,
        } ErrorCode;

        struct __attribute__ ((__packed__)) {
            unsigned __err_rsv1:8;
            ErrorCode code:4;       //!< Type of error detected
            unsigned __err_rsv2:20;
        };
        uint32_t frame_count;       //!< Number of frame errors
        uint32_t length_count;      //!< Number of length errors
        uint32_t crc_count;         //!< Number of CRC errors
        uint32_t orig[0];           //!< Recovered data, dynamic length defined by packet length field

        /**
         * Up-cast Packet to ErrorPacket if packet type allows so.
         */
        static const ErrorPacket *cast(const Packet *packet) {
            if (packet->getType() == Packet::TYPE_ERROR) {
                return reinterpret_cast<const ErrorPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to ErrorPacket");
            }
        }
        /**
         * Up-cast Packet to ErrorPacket if packet type allows so.
         */
        static ErrorPacket *cast(Packet *packet) {
            if (packet->getType() == Packet::TYPE_ERROR) {
                return reinterpret_cast<ErrorPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to ErrorPacket");
            }
        }
};

class DasDataPacket : public Packet {
    public: /* Variables */
        typedef enum {
            EVENT_FMT_INVALID        = 0,
            EVENT_FMT_META           = 1,    //!< meta data (for choppers, beam monitors, ADC sampling etc.) in tof,pixel format
            EVENT_FMT_PIXEL          = 2,    //!< neutron data in tof,pixel format
            EVENT_FMT_LPSD_RAW       = 3,    //!< LPSD raw format
            EVENT_FMT_LPSD_VERBOSE   = 4,    //!< LPSD verbose format
            EVENT_FMT_ACPC_XY_PS     = 5,    //!< ACPC XY photosum format (XY Q17.15, PS Q8.24)
            EVENT_FMT_ACPC_RAW       = 6,    //!< ACPC raw format
            EVENT_FMT_ACPC_VERBOSE   = 7,    //!< ACPC verbose format
            EVENT_FMT_AROC_RAW       = 8,    //!< AROC raw format
            EVENT_FMT_BNL_XY         = 9,    //!< BNL XY format (Q5.11)
            EVENT_FMT_BNL_RAW        = 10,   //!< BNL raw format
            EVENT_FMT_BNL_VERBOSE    = 11,   //!< BNL verbose format
            EVENT_FMT_CROC_XY        = 12,   //!< CROC XY format (TODO: Qm.n)
            EVENT_FMT_CROC_RAW       = 13,   //!< CROC raw format
            EVENT_FMT_CROC_VERBOSE   = 14,   //!< CROC verbose format
            // These are software internal only, ids can change if needed
            EVENT_FMT_PIXEL_MAPPED   = 100,  //!< neutron data in tof,pixel format, pixels were previously mapped
            EVENT_FMT_LPSD_DIAG      = 101,  //!< LPSD verbose mode + mapped pixel field
            EVENT_FMT_ACPC_DIAG      = 102,  //!< ACPC verbose mode + flat-field corrected x,y and mapped pixel
            EVENT_FMT_BNL_DIAG       = 103,  //!< BNL verbose mode + flat-field corrected x,y and mapped pixel
            EVENT_FMT_CROC_DIAG      = 104,  //!< CROC verbose mode + mapped pixel
        } EventFormat;

    protected:
        struct __attribute__ ((__packed__)) {
            uint16_t num_events;            //!< Number of events
            EventFormat event_format:8;     //!< Data format
            bool mapped:1;                  //!< Flag whether events are mapped to logical ids
            bool corrected:1;               //!< Flag whether geometrical correction has been applied
            unsigned __data_rsv2:6;
        };

        uint32_t timestamp_sec;             //!< Accelerator time (seconds) of event 39
        uint32_t timestamp_nsec;            //!< Accelerator time (nano-seconds) of event 39

        uint32_t events[0];                 //!< Placeholder for dynamic buffer of events

    public: /* Functions */

        /**
         * Use buffer as storage for new command packet,DasData populate fields.
         *
         * Uses default values. If data is defined, it's copied to current
         * packet. Otherwise just the count is applied.
         *
         * @param buffer to be used to stora new packet
         * @param size of buffer
         * @param format of data
         * @param time_sec Seconds field
         * @param time_nsec Nano-second field
         * @param count Number of bytes in data to be copied
         * @param data to be copied to new packet
         * @return Returns a newly created packet or nullptr on error.
         */
        static DasDataPacket *init(uint8_t *buffer, size_t size, EventFormat format, const epicsTimeStamp &timestamp, uint32_t count=0, const uint32_t *data=nullptr);

        /**
         * Initialize packet fields.
         *
         * Uses default values. If data is defined, it's copied to current
         * packet. Otherwise just the count is applied.
         *
         * @param format of data
         * @param time_sec Seconds field
         * @param time_nsec Nano-second field
         * @param count Number of bytes in data to be copied
         * @param data to be copied to new packet
         */
        void init(EventFormat format, const epicsTimeStamp &timestamp, uint32_t count=0, const uint32_t *data=nullptr);

        /**
         * Up-cast Packet to DasDataPacket if packet type allows so.
         */
        static const DasDataPacket *cast(const Packet *packet) {
            if (packet->getType() == Packet::TYPE_DAS_DATA) {
                return reinterpret_cast<const DasDataPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to DasDataPacket");
            }
        }
        /**
         * Up-cast Packet to DasDataPacket if packet type allows so.
         */
        static DasDataPacket *cast(Packet *packet) {
            if (packet->getType() == Packet::TYPE_DAS_DATA) {
                return reinterpret_cast<DasDataPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to DasDataPacket");
            }
        }

        /**
         * Templated function to cast generic packet events to the format of callers' preference.
         *
         * Function does a simple cast and does not check for format integrity.
         * As a convenience it also returns number of events based on the
         * requested event format.
         */
        template<typename T>
        const T *getEvents() const
        {
            return reinterpret_cast<const T *>(this->events);
        }
        template<typename T>
        T *getEvents()
        {
            return reinterpret_cast<T *>(this->events);
        }

        /**
         * Return number of events in data packet.
         *
         * The size is calculated based on payload length and the
         * DasDataPacket::format field. In case of DATA_FMT_RESERVED the event
         * size used is 8 bytes.
         *
         * @return Number of events in data packet.
         */
        uint32_t getNumEvents() const
        {
            return num_events;
        }

        /**
         * Return size of every event in data packet.
         *
         * The size is determined based on DasDataPacket::event_format field.
         */
        uint32_t getEventsSize() const
        {
            return getEventsSize(this->event_format);
        }

        /**
         * Return events size based on format.
         *
         * In case of DATA_FMT_RESERVED the event size used is 8 bytes.
         * For unknown event format the returned value is 1.
         */
        static uint32_t getEventsSize(DasDataPacket::EventFormat format);

        /**
         * Return type of events in this data packet.
         */
        EventFormat getEventsFormat() const
        {
            return event_format;
        }

        /**
         * Get acquisition frame start time (EPICS epoch timestamp)
         */
        epicsTimeStamp getTimeStamp() const {
            return { this->timestamp_sec, this->timestamp_nsec };
        }

        /**
         * Have all events in packet been mapped?
         * @return true/false
         */
        bool getEventsMapped() const {
            return mapped;
        }

        /**
         * Set or clear mapped flag.
         * @param m true/false
         */
        void setEventsMapped(bool m) {
            mapped = m;
        }
};

class RtdlPacket : public Packet {
    public:
        struct __attribute__ ((__packed__)) RtdlFrame {
            union __attribute__ ((__packed__)) {
                struct __attribute__ ((__packed__)) {
                    unsigned data:24;               //!< RTDL frame data
                    uint8_t id;                     //!< RTDL frame identifier
                };
                uint32_t raw;                       //!< Non decoded RTDL frame
            };
            RtdlFrame(uint32_t raw_)
            : raw(raw_) {}
            RtdlFrame(uint8_t id_, uint32_t data_)
            : data(data_ & 0xFFFFFF)
            , id(id_) {}
        };

    protected: /* Variables */

        struct __attribute__ ((__packed__)) {
            uint8_t num_frames;             //!< Number of 4 byte RTDL frames in this packet
            unsigned __reserved2:24;
        };

        RtdlFrame frames[0];                //!< Placeholder for dynamic buffer of RTDL frame data

    public: /* Functions */
        /**
         * Use buffer as storage for new RTDL packet, populate fields.
         *
         * @param buffer to be used to stora new packet
         * @param size of buffer
         * @param frames RTDL frames data
         * @return Returns a newly created packet or nullptr on error.
         */
        static RtdlPacket *init(uint8_t *buffer, size_t size, const std::vector<RtdlFrame> &frames);

        /**
         * Populate fields.
         *
         * @param frames RTDL frames data
         * @return Returns a newly created packet or nullptr on error.
         */
        void init(const std::vector<RtdlFrame> &frames);

        /**
         * Up-cast Packet to RtdlPacket if packet type allows so.
         */
        static const RtdlPacket *cast(const Packet *packet) {
            if (packet->getType() == Packet::TYPE_RTDL) {
                return reinterpret_cast<const RtdlPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to RtdlPacket");
            }
        }
        /**
         * Up-cast Packet to RtdlPacket if packet type allows so.
         */
        static RtdlPacket *cast(Packet *packet) {
            if (packet->getType() == Packet::TYPE_RTDL) {
                return reinterpret_cast<RtdlPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to RtdlPacket");
            }
        }

        /**
         * Get acquisition frame start time (EPICS epoch timestamp)
         *
         * Returns epicsTimeStamp{0,0} when timestamp can't be decoded due to missing RTDL frames.
         */
        epicsTimeStamp getTimeStamp() const;

        /**
         * Decode most relevant RTLD data.
         *
         * If RTDLs frames are missing, some of the fields will be 0.
         * RtdlHeader format is used by legacy firmware as well as ADARA
         * software.
         *
         * @return Populated structure.
         */
        RtdlHeader getRtdlHeader() const;

        /**
         * Return number of RTDL frames included in this packet.
         */
        uint8_t getNumRtdlFrames() const {
            return num_frames;
        }

        /**
         * Return RTDL frames from the packet in no particular order.
         */
        std::vector<RtdlFrame> getRtdlFrames() const {
            return std::vector<RtdlFrame>(frames, frames + num_frames);
        }
};

class DasCmdPacket : public Packet {
    public: /* Variables */

        static const uint32_t BROADCAST_ID = 0x0;
        static const uint32_t OCC_ID       = 0x0CC;

        /**
         * Module types as returned in discover response.
         */
        typedef enum {
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
        } ModuleType;

        /**
         * Type of commands supported by modules.
         *
         * It's up to the module whether he implements particular command.
         */
        typedef enum {
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
        } CommandType;

    protected:

        struct {
            unsigned cmd_length:12;     //!< Command payload length in bytes, must be multiple of 2
            unsigned __reserved2:4;
            CommandType command:8;      //!< Type of command
            unsigned cmd_id:5;          //!< Command/response verification id
            bool acknowledge:1;         //!< Flag whether command was succesful, only valid in response
            bool response:1;            //!< Flags this command packet as response
            unsigned cmd_version:1;     //!< LVDS protocol version,
                                        //!< hardware uses this flag to distinguish protocol in responses
                                        //!< but doesn't use it from optical side.
        };
        uint32_t module_id;             //!< Destination address
        uint32_t payload[0];            //!< Dynamic sized command payload, storage must be multiple of 4 bytes but actual payload can be multiple of 2

    public: /* Functions */
        /**
         * Use buffer as storage for new command packet, populate fields.
         */
        static DasCmdPacket *init(uint8_t *data, size_t size, uint32_t moduleId, CommandType cmd, uint8_t cmd_ver, bool ack=false, bool rsp=false, uint8_t ch=0, size_t payloadSize=0, const uint32_t *payload_=nullptr);

        /**
         * Populate fields.
         *
         * Uses default values. If payload is defined, it's copied to current
         * packet. Otherwise just the payloadSize is applied.
         */
        void init(uint32_t moduleId, CommandType cmd, uint8_t cmd_ver, bool ack=false, bool rsp=false, uint8_t ch=0, size_t payloadSize=0, const uint32_t *payload_=nullptr);

        /**
         * Up-cast Packet to DasCmdPacket if packet type allows so.
         */
        static const DasCmdPacket *cast(const Packet *packet) {
            if (packet->getType() == Packet::TYPE_DAS_CMD) {
                return reinterpret_cast<const DasCmdPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to DasCmdPacket");
            }
        }
        /**
         * Up-cast Packet to DasCmdPacket if packet type allows so.
         */
        static DasCmdPacket *cast(Packet *packet) {
            if (packet->getType() == Packet::TYPE_DAS_CMD) {
                return reinterpret_cast<DasCmdPacket *>(packet);
            } else {
                throw Packet::ParseError("Can't upcast to DasCmdPacket");
            }
        }

        /**
         * Return command length in bytes.
         */
        uint32_t getCmdLength() const {
            return cmd_length;
        }

        /**
         * Return number of bytes of command payload.
         */
        uint32_t getCmdPayloadLength() const;

        /**
         * Return command type.
         */
        CommandType getCommand() const {
            return command;
        }

        /**
         * Return remote module id.
         */
        uint32_t getModuleId() const {
            return module_id;
        }

        /**
         * Return remote module id as IP string.
         */
        std::string getModuleIdStr() const;

        /**
         * Return command payload.
         *
         * The last 2 bytes might be padded with 0x0 depending
         * on the actual payload length.
         */
        const uint32_t *getCmdPayload() const {
            return payload;
        }

        /**
         * Is this a command response packet?
         */
        bool isResponse() const {
            return response;
        }

        /**
         * Is this acknowledged command response packet?
         */
        bool isAcknowledge() const {
            return acknowledge;
        }

        /**
         * Return command/response verification id.
         */
        bool getCmdId() const {
            return cmd_id;
        }

        /**
         * Get LVDS protocol version.
         */
        uint8_t getCmdVer() const {
            return cmd_version;
        }
};

#endif // PACKET_H
