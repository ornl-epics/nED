/* TofCorrectPlugin.h
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 * @data July 13, 2016
 */

#ifndef TOF_CORRECT_PLUGIN_H
#define TOF_CORRECT_PLUGIN_H

#include "BaseDispatcherPlugin.h"

/**
 * Define a number of pixels to be corrected
 */
#define TOF_MAX_PIXELS 3

/**
 * Plugin to adjust TOF in events.
 *
 * For now the plugin only adjusts selected meta-data events. It's a temporary
 * workaround for missing DSP-T functionality, while that is being in progress.
 */
class TofCorrectPlugin : public BaseDispatcherPlugin {
    public:
        /**
         * Constructor
         */
        TofCorrectPlugin(const char *portName, const char *connectPortName, int nPixels);

        /**
         * Destructor
         */
        ~TofCorrectPlugin();

    private:

        /**
         * Describe a single allocated memory block.
         */
        typedef struct {
            DasPacket *packet;          //!< Allocated memory address
            size_t size;                //!< Allocated memory size
            bool inUse;                 //!< Currently used by someone
        } PoolEntry;

        /**
         * Structure describing one pixel id correction.
         */
        typedef struct {
            bool enabled;
            int pixelId;
            int tofOffset;
            uint32_t coarseOffset;
            uint32_t fineOffset;
            int32_t nEvents;

        } CorrectionDesc;

        uint32_t m_nReceived;           //!< Number of received packets
        uint32_t m_nProcessed;          //!< Number of modified packets
        std::list<PoolEntry> m_pool;    //!< Pool of DasPacket allocated objects, all of the same size
        epicsMutex m_mutex;             //!< Mutex used solely to serialize sending data to plugins
        std::vector<CorrectionDesc> m_corrections;//!< Limit to 3 pixel ids for now, also relates to PV hookups
        uint32_t m_frameLen;            //!< Frame length in 100ns units

        /**
         * Handle writing integer pv
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Process received packets.
         *
         * Metadata packets are copied and modified, all other patkes are reused
         * without modifications. From metadata packets, select events that
         * match the configured pixel id and apply the TOF correction to those.
         * The correction includes coarse frame correction as well as fine
         * correction within the frame.
         *
         * @param[in] packetList List of packets to be processed.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

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

    protected:
        #define FIRST_TOFCORRECTPLUGIN_PARAM FrameLen
        int FrameLen;           //!< Length of a frame in nsec
        int PoolSize;           //!< Number of allocated packets
        int NPixels;            //!< Number of pixels to correct
        #define LAST_TOFCORRECTPLUGIN_PARAM NPixels
        std::vector<int> PixelIds;      //!< Selected pixel ids
        std::vector<int> TofOffsets;    //!< Offset to be applied to TOF
        std::vector<int> NCorrected;    //!< Number of corrected events
};
#endif // TOF_CORRECT_PLUGIN_H
