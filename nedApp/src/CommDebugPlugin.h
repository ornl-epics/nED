/* CommDebugPlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef COMM_DEBUG_H
#define COMM_DEBUG_H

#include "BasePlugin.h"

/**
 * Plugin to diagnose communication with modules
 *
 * It allows user to create arbitrary command packet from fields and send it
 * through optical channel. The returned response(s) are parsed into fields
 * and displayed. Multiple responses are allowed.
 *
 * The upgraded feature is when the plugin is intercepting commands from other
 * plugins destined for modules. This acts like a sniffer in both directions
 * and must be enabled specifically as it will override user populated fields.
 */
class CommDebugPlugin : public BasePlugin {
    private: // variables
        /**
         * Structure describing single received packet.
         */
        struct PacketDesc {
            uint32_t data[256];
            uint32_t length;
        };

        uint32_t m_buffer[8] = { 0 };           //!< Cached packet data to be sent out
        const uint8_t m_zeroes[1024] = { 0 };   //!< Buffer with all zeroes
        DasCmdPacket *m_packet = nullptr;
        std::list<PacketDesc> m_recvQue;
        std::list<PacketDesc> m_sendQue;
        epicsTime m_liveTimeout;        //!< How long to receive packets after manually send it (not in sniffer mode)

    public: // structures and defines
        /**
         * Constructor for CommDebugPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins list of plugins to connect to.
         */
        CommDebugPlugin(const char *portName, const char *parentPlugins);

        /**
         * Overloaded function to handle writing strings and byte arrays.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded function to handle writing integers.
         */
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Receive and process data packets.
         */
         void recvDownstream(const DasDataPacketList &packets);

        /**
         * Receive and process command packets.
         */
        void recvDownstream(const DasCmdPacketList &packets);

        /**
         * Receive and process RTDL packets.
         */
        void recvDownstream(const DasRtdlPacketList &packets);

        /**
         * Receive and process error packets.
         */
        void recvDownstream(const ErrorPacketList &packets);

        /**
         * Receive packets from plugins to modules.
         */
        void recvUpstream(const DasCmdPacketList &packets);

    private:
        /**
         * Send a command request to the currently selected module.
         */
        void request(const DasCmdPacket::CommandType command);

        /**
         * Process command response from currently selected module.
         */
        void savePacket(const Packet *packet, std::list<PacketDesc> &que, int maxQueSize);

        /**
         * Generate raw packet in cache from high level (ReqCmd, ReqIsDsp, etc.) parameters
         */
        void generatePacket(bool fromRawPvs);

        /**
         * Send cached raw packet
         */
        void sendPacket();

        /**
         * Change currently selected packet and update all related PVs.
         *
         * @param[in] index New index, 0 means current, negative values are converted to positive counterparts.
         */
        void selectRecvPacket(int index);

        /**
         * Populate PVs from packet obtained from sent queue in position defined by index.
         *
         * Negative index value displays first packet. Index over the end of the
         * queue shows last packet.
         *
         * @param[in] index of packet to be displayed.
         */
        void showSentPacket(int index);

        /**
         * Populate PVs from packet.
         *
         * @param[in] packet to be used.
         * @param[in] index to be displayed
         */
        void showSentPacket(const DasCmdPacket *packet, int index=0);

        /**
         * Show packet from receive queue in position defined by index.
         *
         * Negative index value displays first packet. Index over the end of the
         * queue shows last packet.
         *
         * @param[in] index of packet to be displayed.
         */
        void showRecvPacket(int index);

        /**
         * Populate PVs from packet.
         *
         * @param[in] packet to be used.
         * @param[in] index to be displayed
         */
        void showRecvPacket(const Packet *packet, int index=0);

    protected:
        int ReqSend;        //!< Send cached packet
        int Sniffer;        //!< Enables listening to other plugins messages
        int FilterPktType;  //!< Enables filtering by packet type
        int FilterCmd;      //!< Enables filtering by command
        int FilterModule;   //!< Enables filtering by hardware id
        int ResetQues;      //!< Clears packet FIFOs

        int ReqVersion;     //!< Packet version to be sent out
        int ReqPriority;    //!< Packet priority - usually set by modules only
        int ReqType;        //!< Packet type - only works with 8
        int ReqSequence;    //!< Packet sequence number
        int ReqLength;      //!< Packet length in bytes
        int ReqCmdLen;      //!< Command payload length
        int ReqCmd;         //!< Command to be sent to module
        int ReqVerifyId;    //!< Command payload length
        int ReqAck;         //!< Command payload length
        int ReqRsp;         //!< Command payload length
        int ReqCmdVer;      //!< Command payload length
        int ReqModule;      //!< Module address to communicate with
        int ReqRaw0;        //!< Request packet raw word 0
        int ReqRaw1;        //!< Request packet raw word 1
        int ReqRaw2;        //!< Request packet raw word 2
        int ReqRaw3;        //!< Request packet raw word 3
        int ReqRaw4;        //!< Request packet raw word 4
        int ReqRaw5;        //!< Request packet raw word 5
        int ReqRaw6;        //!< Request packet raw word 6
        int ReqRaw7;        //!< Request packet raw word 7
        int ReqTimeStamp;   //!< Request receive time in msec precision

        int RspVersion;     //!< Packet version
        int RspPriority;    //!< Packet priority - usually set by modules only
        int RspType;        //!< Packet type - only works with 8
        int RspSequence;    //!< Packet sequence number
        int RspLength;      //!< Packet length in bytes
        int RspRaw0;        //!< Response packet raw word 0
        int RspRaw1;        //!< Response packet raw word 1
        int RspRaw2;        //!< Response packet raw word 2
        int RspRaw3;        //!< Response packet raw word 3
        int RspRaw4;        //!< Response packet raw word 4
        int RspRaw5;        //!< Response packet raw word 5
        int RspRaw6;        //!< Response packet raw word 6
        int RspRaw7;        //!< Response packet raw word 7
        int RspTimeStamp;   //!< Response receive time in msec precision

        // Command packet PVs
        int RspErrSourceId; //!< Error packet source id
        int RspErrCode;     //!< Error type

        // Command packet PVs
        int RspCmdLen;      //!< Command payload length
        int RspCmd;         //!< Command to be sent to module
        int RspCmdVerifyId; //!< Command verification id
        int RspCmdAck;      //!< Command acknowledge flag
        int RspCmdRsp;      //!< Command response flag
        int RspCmdVer;      //!< Command version (always 1)
        int RspCmdModule;   //!< Module address to communicate with

        // RTDL packet PVs
        int RspRtdlSourceId;//!< RTDL source id field
        int RspRtdlFrames;  //!< RTDL number of frames
        int RspRtdlTime;    //!< RTDL time, seconds converter to time string in ETC
        int RspRtdlTimeNsec;//!< RTDL time, nano-seconds
        int RspRtdlPcharge; //!< RTDL pulse proton charge
        int RspRtdlFlavor;  //!< RTDL pulse flavors
        int RspRtdlBad;     //!< RTDL bad pulse flag
        int RspRtdlCycle;   //!< RTDL pulse cycle number (0-599)
        int RspRtdlVeto;    //!< RTDL last pulse veto
        int RspRtdlTstat;   //!< RTDL tstat
        int RspRtdlBadFr;   //!< RTDL bad cycle frame
        int RspRtdlBadVeto; //!< RTDL bad last cycle veto frame
        int RspRtdlTsync;   //!< RTDL applied TSYNC
        int RspRtdlTofOff;  //!< RTDL applied TOF offset
        int RspRtdlFrOff;   //!< RTDL applied frame offset
        int RspRtdlOffEn;   //!< RTDL TOF enabled flag

        // Data packet PVs
        int RspDataSourceId;//!< Data source id field
        int RspDataFormat;  //!< Data format
        int RspDataTime;    //!< Data time, seconds converter to time string in ETC
        int RspDataTimeNsec;//!< Data time, nano-seconds

        int SendQueIndex;   //!< Currently display sent packet index
        int SendQueSize;    //!< Number of elements in send buffer
        int SendQueMaxSize; //!< Max num of elements in send buffer
        int RecvQueIndex;   //!< Currently display received packet index
        int RecvQueSize;    //!< Number of elements in receive buffer
        int RecvQueMaxSize; //!< Max num of elements in receive buffer
};

#endif // COMM_DEBUG_H
