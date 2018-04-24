/* StateAnalyzerPlugin.h
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef STATE_ANALYZER_PLUGIN_H
#define STATE_ANALYZER_PLUGIN_H

#include "BasePlugin.h"
#include "Event.h"
#include "Fifo.h"
#include "ObjectPool.h"

#include <list>
#include <vector>

/**
 * State Analyzer Plugin
 * 
 * Takes raw trigger inputs from data stream or through PV and converts
 * them to states. This is done efficiently by mapping each trigger to
 * a particular bit in a state id. If needed, states can be later remapped.
 * 
 * Triggers from data stream are essentially individual pixel ids, one for
 * ON and one for OFF state. When trigger state resolves to ON, bit in 
 * combined state id is set, otherwise it's cleared.
 * Additionally 2 more pixel ids can be defined for VETO ON and VETO OFF.
 * Any event in between VETO OFF and VETO ON triggers is rejected. This
 * ensures clean state transitions when so requested.
 * 
 * PV triggers are mostly useful for slow changing states. They operate
 * the same way as data triggers except their resolution is in ms.
 */
class epicsShareFunc StateAnalyzerPlugin : public BasePlugin {
    private:
        /**
         * Upgraded std::list holding Events, always sorted if sortedInsert() used.
         */
        class EventList : public std::list<Event::Pixel> {
            public:
                /**
                 * Insert event to the sorted list.
                 */
                void sortedInsert(const Event::Pixel &event);
        };

        struct DeviceInfo {
            uint32_t pixelOn;
            uint32_t pixelOff;
            uint32_t pixelVetoOn;
            uint32_t pixelVetoOff;
            uint8_t bitOffset;
            bool enabled;
            double distance;
        };
        struct PulseEvents {
            epicsTime timestamp;
            EventList neutrons;
            std::vector<EventList> signals;
            std::list<Event::Pixel> others;
            PulseEvents(epicsTime timestamp_, uint32_t nSignals)
                : timestamp(timestamp_)
                , signals(nSignals) {}
        };
        std::list<PulseEvents> m_cache;
        std::vector<DeviceInfo> m_devices;
        ObjectPool<DasDataPacket> m_packetsPool{true};
        bool m_enabled{false};
        uint32_t m_state{0};
        uint32_t m_vetostate{0};
        uint32_t m_maxCacheLen{10};
        double m_distance{1.0};

        Fifo<PulseEvents> m_processQue;
        Thread m_processThread;

    public:
        /**
         * Constructor
         *
         * @param[in] portName Name of the asyn port to which plugins can connect
         * @param[in] parentPlugins to connect to.
         * @param[in] numTriggers
         */
        StateAnalyzerPlugin(const char *portName, const char *parentPlugins, uint8_t numTriggers);

        /**
         * Handle this plugin specific asynInt32 parameters
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Handle this plugin specific asynFloat64 parameters
         */
        asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

        /**
         * Process incoming command packets, only packets with EVENT_FMT_PIXEL and EVENT_FMT_META.
         */
        void recvDownstream(const DasDataPacketList &packetList);

    private:

        void sendDownstream(const PulseEvents &pulseEvents);
        using BasePlugin::sendDownstream;

        void flagNeutrons(PulseEvents &pulseEvents);

        /**
         * Return pixel id distance from moderator.
         * 
         * For now always return the nominal value, but in the future we could
         * easily replace implementation with reading IDF definition instead.
         * Guaranteed not to return 0.
         */
        double getEventDistance(uint32_t pixelid);

        void processThread(epicsEvent *shutdown);

    private:
        int Status;
        int StatusText;
        int MaxCacheLen;        // Number of pulses to cache events before processing them, also the size of processing queue
        int State;
        int Vetoing;
        int Enable;
        int NominalDist;
        struct DeviceParams {
            int PixelOn;
            int PixelOff;
            int PixelVetoOn;
            int PixelVetoOff;
            int BitOffset;
            int Distance;
        };
        std::vector<DeviceParams> Devices;
};

#endif // STATE_ANALYZER_PLUGIN_H
