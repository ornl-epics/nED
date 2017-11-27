/* CommDebug.h
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
class CommDebug : public BasePlugin {
    private: // variables
        /**
         * Structure describing single received packet.
         */
        struct PacketDesc {
            uint32_t data[256];
            uint32_t length;
            epicsTimeStamp timestamp;
        };

        uint32_t m_buffer[8];   //!< Cached packet data to be sent out
        DasCmdPacket *m_packet;
        std::list<PacketDesc> m_lastPacketQueue;

    public: // structures and defines
        /**
         * Constructor for CommDebug
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins list of plugins to connect to.
         */
        CommDebug(const char *portName, const char *parentPlugins);

        /**
         * Overloaded function to handle writing strings and byte arrays.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded function to handle writing integers.
         */
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Receive packets from modules.
         */
        void recvDownstream(DasCmdPacketList *packets);

        /**
         * Receive packets from plugins to modules.
         */
        void recvUpstream(DasCmdPacketList *packets);

    private:
        /**
         * Send a command request to the currently selected module.
         */
        void request(const DasCmdPacket::CommandType command);

        /**
         * Process command response from currently selected module.
         */
        bool savePacket(const DasCmdPacket *packet);

        /**
         * Generate raw packet in cache from high level (ReqCmd, ReqIsDsp, etc.) parameters
         */
        void generatePacket(bool fromRawPvs);

        /**
         * Send cached raw packet
         */
        void sendPacket();

        /**
         * Populate PVs for sent package.
         */
        void showSendPacket();

        /**
         * Change currently selected packet and update all related PVs.
         *
         * @param[in] index New index, 0 means current, negative values are converted to positive counterparts.
         */
        void selectRecvPacket(int index);

    protected:
        int ReqSend;        //!< Send cached packet
        int ReqSniffer;     //!< Enables listening to other plugins messages

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

        int RspVersion;     //!< Packet version
        int RspPriority;    //!< Packet priority - usually set by modules only
        int RspType;        //!< Packet type - only works with 8
        int RspSequence;    //!< Packet sequence number
        int RspLength;      //!< Packet length in bytes
        int RspCmdLen;      //!< Command payload length
        int RspCmd;         //!< Command to be sent to module
        int RspVerifyId;    //!< Command payload length
        int RspAck;         //!< Command payload length
        int RspRsp;         //!< Command payload length
        int RspCmdVer;      //!< Command payload length
        int RspModule;      //!< Module address to communicate with
        int RspRaw0;        //!< Response packet raw word 0
        int RspRaw1;        //!< Response packet raw word 1
        int RspRaw2;        //!< Response packet raw word 2
        int RspRaw3;        //!< Response packet raw word 3
        int RspRaw4;        //!< Response packet raw word 4
        int RspRaw5;        //!< Response packet raw word 5
        int RspRaw6;        //!< Response packet raw word 6
        int RspRaw7;        //!< Response packet raw word 7
        int RspTimeStamp;   //!< Response receive time in msec precision

        int PktQueIndex;    //!< Currently display packet index
        int PktQueSize;     //!< Number of elements in packet buffer
        int PktQueMaxSize;  //!< Max num of elements in packet buffer
};

#endif // COMM_DEBUG_H
