/* TimingPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef TIMING_PLUGIN_H
#define TIMING_PLUGIN_H

#include "BaseDispatcherPlugin.h"

/**
 * Class injects timing information when not available from DSP-T.
 *
 * In order to support nED operation on legacy DSP, TimingPlugin
 * injects timing related information in the right locations. This includes
 * creating and sending to plugins full RTDL packets every 16.6ms. Data packets
 * are modified with RTDL header following DasPacket header. Train of
 * sub-packets share the same RTDL header information regardless if some
 * of them are received after new RTDL data is available.
 *
 * Plugin functionality is split into two parts. First one takes care of
 * periodically updating RTDL data and sending out RTDL packets to registered
 * plugins. It's running in it's own thread through EPICS timers.
 * Second part is processing incoming packets and injecting RTDL header
 * into them before sending them to plugins.
 *
 * Plugin does not discard any original packets or their parts. If RTDL packets
 * are already in the inbound stream, plugin passes them on but it also
 * creates new RTDL packets with potentialy different time and certanly
 * non-synchronized cycle numbers. Don't use this plugin with DSP-T.
 *
 * RTDL data can be obtained from the network coming from ETC. In that case plugin
 * receives RTDL data messages from ETC and replaces cached RTDL data immediately.
 *
 * Plugin can also fake RTDL data. In that case most of the information
 * is static with a few exceptions. Timestamp is taken from system clock and
 * binned into 60 bins. Pulse charge changes randomly. Cycle number correctly
 * counts to 600 and then rolls over to 0. Every 600th pulse gets vetoed
 * as physics pulse.
 *
 * Regardless the source of RTDL data, whenever it updates the plugin will
 * send an RTDL packet to plugins. For faked data the frequency is
 * guaranteed to be 60Hz. For RTDL data from the network the update
 * happens as soon as network packet is received.
 *
 * Plugin should be connected directly to the source and all other plugins
 * must be behind it in order to get properly timed packets. Plugin works
 * also in DSP-T environment but it's un-necessary there and should not be
 * used for performance reasons.
 */
class TimingPlugin : public BaseDispatcherPlugin {
    public:
        /**
         * Constructor
         */
        TimingPlugin(const char *portName, const char *connectPortName);

    private:
        uint32_t m_nReceived;           //!< Number of received packets
        uint32_t m_nProcessed;          //!< Number of modified packets
        DasPacket *m_rtdlPacket;        //!< Last RTDL packet sent out
        RtdlHeader m_neutronsRtdl;      //!< RTDL data for neutron packets, if sub-packets train goes after new RTDL data
        RtdlHeader m_metaRtdl;          //!< RTDL data for metadata packets, if sub-packets train goes after new RTDL data
        std::list<DasPacket *> m_pool;  //!< Pool of DasPacket allocated objects, all of the same size
        static const uint32_t PACKET_SIZE;  //!< Size of every packet allocated
        epicsMutex m_mutex;             //!< Mutex used solely to serialize sending data to plugins
        Timer m_timer;                  //!< Used to trigger periodic RTDL updates
        int m_socket;                   //!< Socket to remote timing server

        /**
         * Handle setting integer parameters.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process received packets.
         *
         * Loops through all packets in the list and modifies neutron and
         * metadata data packets without RTDL header. Other packets are
         * passed on unmodified.
         *
         * Function works properly even on packets with RTDL header already.
         * It simply skips them but it still takes some time to run. It's
         * un-necessary to run this plugin when packets are expected to have
         * RTDL timing header already included.
         *
         * The function relies on DSP to group pulse data into sub-packets
         * and keep subpacket_start and subpacket_end flags accurate. When
         * a new pulse is detected based on new train of sub-packets,
         * cached RTDL data is updated and an RTDL packet is sent out first.
         * All sub-packets are tagged with the same RTDL information.
         *
         * Function may alter order of the packets received.
         *
         * @param[in] packetList List of packets to be processed.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

        /**
         * Inject RTDL timing header into the data packet.
         *
         * RTDL timing header is injected between packet header and payload
         * data. Besides injecting RTDL header, packet payload and rtdl_present
         * flag of the source packet are modified in the output packet.
         *
         * Function assumes data packet.
         *
         * @param[in] src Source data packet
         * @param[in] rtdl RTDL data to be used.
         * @return Modified packet or 0 on error.
         */
        const DasPacket *timestampPacket(const DasPacket *src, const RtdlHeader *rtdl);

        /**
         * Allocate packet of at least the size requested
         *
         * Either from pool or allocate new one if pool exhausted. All packets
         * are of the same size which simplifies the implementation and
         * optimizes performance while consuming more memory. Packets are
         * recycled on regular basis so little actual allocation is expected
         * after a couple of cycles. Packet returned from this function must
         * be freed using freePacket() which puts it into the pool for reuse.
         *
         * Is not safe thread, but as long as it's used solely from processData*()
         * it's safe not to lock due to program design constraints.
         *
         * @param[in] size Size of the packet is checked not to exceed limits.
         * @return Allocated packet or 0 on error.
         */
        DasPacket *allocPacket(uint32_t size);

        /**
         * Push no longer used packet to the pool for reuse.
         *
         * Is not safe thread, but as long as it's used solely from processData*()
         * it's safe not to lock due to program design constraints.
         *
         * @param[in] packet Unused packet.
         */
        void freePacket(DasPacket *packet);

        /**
         * Worker function invoked periodically to check if RTDL should update.
         *
         * In fake mode operation the function checks if there's enough time elapsed
         * to generate a new RTDL. If so, it sends an RTDL packet to plugins.
         *
         * @return Delay in seconds before invoking again.
         */
        double updateRtdl();

        /**
         * Populate RTDL packet with fake header data.
         *
         * Updates timestamp based on system clock and binned into 60Hz bins.
         * Generates random pulse charge information. Increases cycle number by
         * one or roll over to 0 when reached 600. When cycle is 0, flavor flag
         * is set to RtdlHeader::RTDL_FLAVOR_NO_BEAM temporarily.
         *
         * No data is updated if current data is still valid based on 60Hz resolution.
         *
         * @param[out] packet RTDL packet to be updated
         * @return true when packet was updated
         */
        bool createFakeRtdl(DasPacket *packet);

        /**
         * Initialize network socket for receiving data.
         */
        bool connectEtc(uint32_t port);

        /**
         * Disconnect from remote socket.
         */
        void disconnectEtc();

        /**
         * Check if there's new RTDL message from ETC and update packet
         *
         * Only processes RTDL messages from ETC, skips the rest. If there's
         * an RTDL message waiting in queue, its data is used to update RTDL
         * packet and function returns true. Return false otherwise.
         *
         * @param[out] packet RTDL packet to be updated
         * @return true when packet was updated
         */
        bool recvRtdlFromEtc(DasPacket *packet);

    protected:
        #define FIRST_TIMINGPLUGIN_PARAM PoolSize
        int PoolSize;           //!< Number of allocated packets
        int Mode;               //!< Select RTDL data source, fake or network
        int RecvPort;           //!< Remote port
        int Connected;          //!< Flag whether connected to ETC computer
        int FakeRtdlCnt;        //!< Number of fake RTDL pulses generated
        int EtcRtdlCnt;         //!< Number of RTDL pulses received from ETC
        #define LAST_TIMINGPLUGIN_PARAM EtcRtdlCnt

};
#endif // TIMING_PLUGIN_H
