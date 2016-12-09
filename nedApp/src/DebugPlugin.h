/* DebugPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DEBUG_PLUGIN_H
#define DEBUG_PLUGIN_H

#include "BasePlugin.h"

#include <cstring>

/**
 * Generic module plugin is a tool to test and debug module communication.
 *
 * The plugin is mainly useful for testing. User must first select remote
 * module and command. Writing command triggers sending the packet out and
 * waiting for response. Incoming response from the configured module will
 * immediately populate all PVs.
 *
 * Plugin requires a valid hardware address of the remote module. It does not
 * send out global commands. Format of outgoing packets depends on the link
 * type. Only DSPs are connected through optics, all other modules are
 * connected to DSP through LVDS. When sending a packet through LVDS, extra
 * formatting is necessary. User must select what type of communication to use
 * through ReqIsDsp parameter.
 *
 * User can send any 8 bit OCC command. The plugin does not check command value
 * and thus allows extensibility through EPICS database.
 */
class DebugPlugin : public BasePlugin {
    private: // variables
        /**
         * Valid byte grouping modes.
         */
        enum ByteGroupingMode {
            GROUP_2_BYTES_SWAPPED   = 0,
            GROUP_2_BYTES           = 1,
            GROUP_4_BYTES           = 2,
        };

        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynOctetMask;
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynOctetMask;

        uint32_t m_rawPacket[18];   //!< Cached packet data to be sent out
        uint32_t m_payload[256];    //!< Last packet payload
        uint32_t m_payloadLen;      //!< Last packet payload length, in number of elements in m_payload

    public: // structures and defines
        /**
         * Constructor for DebugPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        DebugPlugin(const char *portName, const char *dispatcherPortName, int blocking=0);

        /**
         * Overloaded function to handle writing strings and byte arrays.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded function to handle reading strings and byte arrays.
         */
        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

        /**
         * Overloaded function to handle writing integers.
         */
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function to process incoming OCC packets.
         */
        virtual void processData(const DasPacketList * const packetList);

    private:
        /**
         * Send a command request to the currently selected module.
         */
        void request(const DasPacket::CommandType command);

        /**
         * Process command response from currently selected module.
         */
        bool parseCmd(const DasPacket *packet);

        /**
         * Generate raw packet in cache from high level (ReqCmd, ReqIsDsp, etc.) parameters
         */
        void generatePacket();

        /**
         * Send cached raw packet
         */
        void sendPacket();

    protected:
        #define FIRST_GENERICMODULEPLUGIN_PARAM ReqDest
        int ReqDest;        //!< Module address to communicate with
        int ReqCmd;         //!< Command to be sent
        int ReqIsDsp;       //!< Is the module we communicate with behinds the DSP, using LVDS link
        int ReqSend;        //!< Send cached packet
        int RspCmd;         //!< Response command, see DasPacket::CommandType
        int RspCmdAck;      //!< Response ACK/NACK
        int RspFlag;        //!< Response flag present
        int RspHwType;      //!< Hardware type, see DasPacket::ModuleType
        int RspSrc;         //!< Response source address
        int RspRouter;      //!< Response router address
        int RspDest;        //!< Response destination address
        int RspLen;         //!< Response length in bytes
        int RspDataLen;     //!< Response payload length in bytes
        int RspData;        //!< Response payload
        int ByteGrp;        //!< How many byte to group
        int Channel;        //!< Select channel to send command to (read/write config only)
        int RawPkt0;        //!< Raw packet data, dword 0
        int RawPkt1;        //!< Raw packet data, dword 1
        int RawPkt2;        //!< Raw packet data, dword 2
        int RawPkt3;        //!< Raw packet data, dword 3
        int RawPkt4;        //!< Raw packet data, dword 4
        int RawPkt5;        //!< Raw packet data, dword 5
        int RawPkt6;        //!< Raw packet data, dword 6
        int RawPkt7;        //!< Raw packet data, dword 7
        int RawPkt8;        //!< Raw packet data, dword 8
        int RawPkt9;        //!< Raw packet data, dword 9
        int RawPkt10;       //!< Raw packet data, dword 10
        int RawPkt11;       //!< Raw packet data, dword 11
        int RawPkt12;       //!< Raw packet data, dword 12
        int RawPkt13;       //!< Raw packet data, dword 13
        int RawPkt14;       //!< Raw packet data, dword 14
        int RawPkt15;       //!< Raw packet data, dword 15
        int RawPkt16;       //!< Raw packet data, dword 16
        int RawPkt17;       //!< Raw packet data, dword 17
        #define LAST_GENERICMODULEPLUGIN_PARAM RawPkt17
};

#endif // GENERIC_MODULE_PLUGIN_H
