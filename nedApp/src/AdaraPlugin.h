/* AdaraPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef ADARA_PLUGIN_H
#define ADARA_PLUGIN_H

#include "BaseSocketPlugin.h"
#include "RtdlHeader.h"

#include <list>
#include <vector>

/**
 * Plugin that forwards Neutron Event data to ADARA SMS over TCP/IP.
 *
 * The plugin registers to receive all packets, but will only process and
 * forward neutron event data packets which have RTDL header attached and
 * the RTDL packets.
 *
 * After setting at least LISTEN_ADDR parameter, the plugin will start listening
 * for incoming TCP/IP connections. LISTEN_ADDR and LISTEN_PORT can be changed
 * at any time, which will reconfigure listening socket and also disconnect
 * ADARA SMS client, if connected.
 *
 * When enabled, processing of packets will only occur if there's a client
 * which accepts data. In this case, the Neutron Event data packets and RTDL
 * packets are transformed into ADARA format and sent over socket, one packet
 * at a time. Client socket is disconnected on any error. ADARA is supposed
 * to reconnect immediately.
 */
class AdaraPlugin : public BaseSocketPlugin {
    private:
        epicsTimeStamp m_lastDataTimestamp{0,0}; //!< Timestamp of last event data sent to Adara for SMS book-keeping
        uint16_t m_packetSeq{0};

        struct DataInfo {
            uint16_t sourceId{0};
            uint16_t pulseSeq{0};
            RtdlHeader rtdl;
        };
        std::vector<uint32_t> m_buffer;

        std::list<std::pair<epicsTime, DataInfo>> m_cachedRtdl;

        bool m_eventsSel[8] = { true, true, true, true, true, true, true, true };

    public:
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dataPlugins to connect to.
         * @param[in] rtdlPlugins to connect to.
         */
        AdaraPlugin(const char *portName, const char *dataPlugins, const char *rtdlPlugins);

        /**
         * Overloaded function.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

        /**
         * Process RTDL packets.
         */
        void recvDownstream(const RtdlPacketList &packets) override;

        /**
         * Process data packets.
         */
        void recvDownstream(const DasDataPacketList &packets) override;

    private:
        /**
         * Sends ADARA heartbeat packet during event data inactivity.
         *
         * @retval true When data has been sent.
         * @retval false Socket not connected or other socket error.
         */
        bool sendHeartbeat() override;

        /**
         * When socket is connected, send RTDL to ADARA.
         *
         * @param[in] timestamp of the RTLD packet
         * @param[in] rtdl header as expected by ADARA protocol
         * @param[in] frames from RTDL packet
         * @retval true When data has been sent.
         * @retval false Socket not connected or other socket error.
         */
        bool sendRtdl(epicsTimeStamp timestamp, const RtdlHeader &rtdl, const std::vector<RtdlPacket::RtdlFrame> &frames);

        /**
         * When socket is connected, send events to ADARA.
         *
         * @param[in] timestamp of the data packet
         * @param[in] mapped flags whether events have been previously mapped
         * @param[in] events array of events, templated parameter.
         * @param[in] nEvents number of events to send.
         * @retval true When data has been sent.
         * @retval false Socket not connected or other socket error.
         */
        template <typename T>
        bool sendEvents(epicsTimeStamp &timestamp, bool mapped, const T *events, uint32_t nEvents);

        /**
         * Overloaded function resets internal state when new client connects.
         */
        void clientConnected();

        /**
         * Reset all internal logic.
         *
         * When reset, the following internal logic is reset:
         * - ADARA protocol sources' sequence numbers are reset to 0
         * - RTDL book-keeping logic is reset to beggining
         */
        void reset();

    protected:
        int Enable;
        int Reset;
        int RtdlCacheSize;
        int CntDataPkts;
        int CntRtdlPkts;
        int CntPingPkts;
        int DetEventsEn;
        int MonEventsEn;
        int SigEventsEn;
        int AdcEventsEn;
        int ChopEventsEn;
};

#endif // ADARA_PLUGIN_H
