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

#define ADARA_MAX_NUM_DSPS  10  //!< Maximum number of DSPs supported by a single AdaraPlugin instance

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
        uint64_t m_nTransmitted;    //!< Number of packets sent to BASESOCKET
        uint64_t m_nProcessed;      //!< Number of processed packets
        uint64_t m_nReceived;       //!< Number of packets received from dispatcher
        epicsTimeStamp m_lastSentTimestamp; //!< Timestamp of last packet sent to Adara
        epicsTimeStamp m_lastRtdlTimestamp; //!< Timestamp of last RTDL packet sent to Adara
        bool m_heartbeatActive;     //!< Allow to temporarily disable heartbeat to prevent interleaved writes on socket
        uint32_t m_dataPktType;     //!< Type of data packets, raw or mapped pixel data

        /**
         * Structure describing output packets sequence for a given source.
         */
        struct SourceSequence {
            uint32_t sourceId;          //!< Source id for output packets
            RtdlHeader rtdl;            //!< RTDL header of the current pulse
            uint32_t pulseSeq;          //!< Packet sequence number within one pulse
            uint32_t totalSeq;          //!< Overall packet sequence number
            SourceSequence()
                : pulseSeq(0)
                , totalSeq(0)
            {
                reset();
            }
            void reset()
            {
                rtdl.timestamp_sec  = 0;
                rtdl.timestamp_nsec = 0;
            }
        };
        SourceSequence m_neutronSeq;    //!< Neutrons data sequence
        SourceSequence m_metadataSeq;   //!< Metadata data sequence

    public:
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Should processing of callbacks block execution of caller thread or not.
         * @param[in] numDsps Number of DSPs that will be sending data
         */
        AdaraPlugin(const char *portName, const char *dispatcherPortName, int blocking, int numDsps);

        /**
         * Destructor
         */
        ~AdaraPlugin();

        /**
         * Overloaded function.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process RTDL and NeutronData packets only, skip rest.
         *
         * @param[in] packetList List of packets to be processed
         */
        void processData(const DasPacketList * const packetList);

        /**
         * Overloaded periodic function to send ADARA Heartbeat packet.
         *
         * Send Heartbeat packet and call base BaseSocketPlugin::checkClient()
         * function for timer upkeep.
         *
         * @return Number returned from BaseSocketPlugin::checkClient()
         */
        float checkClient();

    protected:
        #define FIRST_ADARAPLUGIN_PARAM PixelsMapped
        int PixelsMapped;
        int NeutronsEn;
        int MetadataEn;
        int Reset;;
        #define LAST_ADARAPLUGIN_PARAM Reset
};

#endif // ADARA_PLUGIN_H
