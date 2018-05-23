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
 * Calculates combined state events based on devices events
 * or external PVs. Combined states events are injected into
 * the data stream as well as encoded in all neutron events
 * pixelids.
 */
class epicsShareFunc StateAnalyzerPlugin : public BasePlugin {
    private:

        static const uint32_t PIXEL_STATE_VETO_MASK = 0x08000000;

        /**
         * Upgraded std::list holding Events, always sorted if sortedInsert() used.
         */
        template <typename T>
        class EventList : public std::list<T> {
            public:
                /**
                 * Sorted insert based on event->tof field.
                 */
                void sortedInsert(const T &event);
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
            bool mapped{false};
            EventList<Event::Pixel> pixel_neutrons;
            EventList<Event::BNL::Diag> bnl_neutrons;
            EventList<Event::Pixel> states;
            PulseEvents(epicsTime timestamp_, uint32_t nSignals)
                : timestamp(timestamp_) {}
        };
        std::list<PulseEvents> m_cache;
        std::vector<DeviceInfo> m_devices;
        ObjectPool<DasDataPacket> m_packetsPool{true};
        bool m_enabled{false};
        uint32_t m_state{0};
        uint32_t m_vetostate{0};
        uint32_t m_maxCacheLen{10};
        double m_distance{1.0};
        uint32_t m_bitOffset{20};
        uint32_t m_statePixelMask{0x60320000};

        Fifo<PulseEvents> m_processQue;
        Thread m_processThread;

    public:
        /**
         * Constructor
         *
         * @param[in] portName Name of the asyn port to which plugins can connect
         * @param[in] parentPlugins to connect to.
         */
        StateAnalyzerPlugin(const char *portName, const char *parentPlugins);

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

        /**
         * Return pixel id distance from moderator.
         * 
         * For now always return the nominal value, but in the future we could
         * easily replace implementation with reading IDF definition instead.
         * Guaranteed not to return 0.
         */
        double getPixelDistance(uint32_t pixelid);

        /**
         * Events processing thread.
         *
         * Waits for all data from a single frame and then invokes
         * processEvents() function.
         * 
         * It then sends all events to subscribed plugins as two packets,
         * one for neutrons and second one with combined state events as
         * meta.
         */
        void processThread(epicsEvent *shutdown);

        /**
         * Process all events from a single frame and send them to subscribed plugins.
         *
         * Invokes calcCombinedStates() and tagPixelIds() functions. Puts
         * the results into DasDataPacket's and send them to subscribed
         * plugins. It reflects current state through PVs.
         * 
         * It can be run from a separate thread.
         */
        void processEvents(PulseEvents &pulseEvents);

        /**
         * Calculate combined state events
         *
         * Iterates through a list of signal events that were
         * previously sorted based on their time-of-flight projected to the
         * same nominal distance. It converts signal events pixel ids
         * into combined state events using a mask in m_statePixelMask.
         * Veto states are flagged with 0x100 bit.
         *
         * states array is modified in place, but for optimization reasons
         * the events are also copied to outEvents raw array which needs to
         * be pre-allocated and big enough to accomodate all events.
         *
         * state and vetostate are input and output parameters and are used
         * for keeping track of the states in between the pulses or invokations
         * to this function. When this function returns, the values reflect
         * the last calculated state.
         */
        void calcCombinedStates(EventList<Event::Pixel> &states, Event::Pixel *outEvents, uint32_t &state, uint32_t &vetostate);

        /**
         * Tag pixelid with the state information.
         *
         * This is a templated function that works for all types of events
         * that contain pixelid field. Function uses m_state and m_vetostate
         * so be careful modifying those object variables.
         *
         * Walks through all events and correlates them to the pre-calculated
         * combined states. It puts the combined state information in the
         * upper bits of each pixel id where the bit location is defined by
         * m_bitOffset. State veto is flagged in pixel id as bit 27.
         *
         * events array is modified in place, but for optimization reasons
         * the events are also copied to outEvents raw array which needs to
         * be pre-allocated and big enough to accomodate all events.
         */
        template <typename T>
        void tagPixelIds(EventList<T> &events, const EventList<Event::Pixel> &states, T *outEvents);

    private:
        int Status;             // Plugin overall status
        int StatusText;         // Plugin status text
        int MaxCacheLen;        // Number of pulses to cache events before processing them, also the size of processing queue
        int State;              // Current combined state
        int Vetoing;            // Current vetoing state
        int Enable;             // Enable state calculation, otherwise pass thru all packets
        int NominalDist;        // Distance where all events get projected to for time-ordering purposes, usually detector center distance.
        int PixelBitOffset;     // Calculated state number bit-offset in pixel id
        int StatePixelMask;     // Bit mask for combined state events
        struct DeviceParams {
            int Enable;         // Enables this device
            int PixelOn;        // State ON pixel
            int PixelOff;       // State OFF pixel
            int PixelVetoOn;    // Veto ON pixel (0x0 to disable)
            int PixelVetoOff;   // Veto OFF pixel (0x0 to disable)
            int BitOffset;      // This device bit offset in calculated state
            int Distance;       // Device distance from moderator
        };
        std::vector<DeviceParams> Devices;
};

#endif // STATE_ANALYZER_PLUGIN_H
