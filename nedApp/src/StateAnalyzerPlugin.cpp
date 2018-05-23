/* StateAnalyzerPlugin.cpp
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Log.h"
#include "StateAnalyzerPlugin.h"

#include <algorithm>
#include <string.h>

EPICS_REGISTER_PLUGIN(StateAnalyzerPlugin, 2, "Port name", string, "Parent plugins", string);

StateAnalyzerPlugin::StateAnalyzerPlugin(const char *portName, const char *parentPlugins)
    : BasePlugin(portName, 1, asynFloat64Mask|asynOctetMask, asynFloat64Mask|asynOctetMask)
    , m_processThread("StateAnalyzerThread", std::bind(&StateAnalyzerPlugin::processThread, this, std::placeholders::_1))
    , Devices(4)
{
    createParam("Status",       asynParamInt32, &Status, 1);                // READ - Plugin status, 0=error, 1=ok, 2=warning
    createParam("StatusText",   asynParamOctet, &StatusText, "Ready");      // READ - Text description of the error, if any
    createParam("MaxCacheLen",  asynParamInt32, &MaxCacheLen, 10);          // WRITE - Number of pulses to cache events before sending to other plugins
    createParam("Enable",       asynParamInt32, &Enable, 0);                // WRITE - Toggle processing any data by this plugin, when off all packets are passed thru
    createParam("State",        asynParamInt32, &State, 0);                 // READ - Current state
    createParam("Vetoing",      asynParamInt32, &Vetoing, 0);               // READ - Current state of vetoing
    createParam("NominalDist",  asynParamFloat64, &NominalDist, 1);         // WRITE - Nominal detector distance
    createParam("PixelBitOffset", asynParamInt32, &PixelBitOffset, 20);     // WRITE - Bit offset of the state information in pixelid
    createParam("StatePixelMask", asynParamInt32, &StatePixelMask, 0x0);    // WRITE - Bit mask for combined state events

    for (uint8_t i = 0; i < 4; i++) {
        int param;
        char name[16];

        m_devices.push_back({0,0,0,0});

        snprintf(name, sizeof(name), "Dev%uEnable", i+1);
        createParam(name, asynParamInt32, &param, 0);
        Devices[i].Enable = param;

        snprintf(name, sizeof(name), "Dev%uPixOn", i+1);
        createParam(name, asynParamInt32, &param, 0);
        Devices[i].PixelOn = param;

        snprintf(name, sizeof(name), "Dev%uPixOff", i+1);
        createParam(name, asynParamInt32, &param, 0);
        Devices[i].PixelOff = param;

        snprintf(name, sizeof(name), "Dev%uPixVetoOn", i+1);
        createParam(name, asynParamInt32, &param, 0);
        Devices[i].PixelVetoOn = param;

        snprintf(name, sizeof(name), "Dev%uPixVetoOff", i+1);
        createParam(name, asynParamInt32, &param, 0);
        Devices[i].PixelVetoOff = param;

        snprintf(name, sizeof(name), "Dev%uBitOffset", i+1);
        createParam(name, asynParamInt32, &param, 0);
        Devices[i].BitOffset = param;

        snprintf(name, sizeof(name), "Dev%uDistance", i+1);
        createParam(name, asynParamFloat64, &param, 0);
        Devices[i].Distance = param;
    }
    callParamCallbacks();

    BasePlugin::connect(parentPlugins, MsgDasData);
    m_processThread.start();
}

asynStatus StateAnalyzerPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == MaxCacheLen) {
        if (value <= 0)
            return asynError;
        m_maxCacheLen = value;
        return asynSuccess;
    }
    if (pasynUser->reason == PixelBitOffset) {
        if (value <= 0 || value >= 28)
            return asynError;
        m_bitOffset = value;
        return asynSuccess;
    }
    if (pasynUser->reason == StatePixelMask) {
        m_statePixelMask = value;
        return asynSuccess;
    }
    if (pasynUser->reason == Enable) {
        if (!m_enabled && value > 0) {
            LOG_INFO("Resetting state on enable");
            setIntegerParam(Status, 1);
            setStringParam(StatusText, "Ready");
            callParamCallbacks();
        }
        m_enabled = (value > 0);
        return asynSuccess;
    }
    for (size_t i = 0; i < Devices.size(); i++) {
        if (pasynUser->reason == Devices[i].Enable) {
            m_devices[i].enabled = (value > 0);
            return asynSuccess;
        }
        if (pasynUser->reason == Devices[i].PixelOn) {
            m_devices[i].pixelOn = (value >= 0 ? value : 0);
            return asynSuccess;
        }
        if (pasynUser->reason == Devices[i].PixelOff) {
            m_devices[i].pixelOff = (value >= 0 ? value : 0);
            return asynSuccess;
        }
        if (pasynUser->reason == Devices[i].PixelVetoOn) {
            m_devices[i].pixelVetoOn = (value >= 0 ? value : 0);
            return asynSuccess;
        }
        if (pasynUser->reason == Devices[i].PixelVetoOff) {
            m_devices[i].pixelVetoOff = (value >= 0 ? value : 0);
            return asynSuccess;
        }
        if (pasynUser->reason == Devices[i].BitOffset) {
            m_devices[i].bitOffset = (value >= 0 ? value : 0);
            return asynSuccess;
        }
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus StateAnalyzerPlugin::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    if (pasynUser->reason == NominalDist) {
        if (value <= 0)
            return asynError;
        m_distance = value;
        return asynSuccess;
    }
    for (size_t i = 0; i < Devices.size(); i++) {
        if (pasynUser->reason == Devices[i].Distance) {
            m_devices[i].distance = (value >= 0 ? value : 0);
            return asynSuccess;
        }
    }
    return BasePlugin::writeFloat64(pasynUser, value);
}

void StateAnalyzerPlugin::recvDownstream(const DasDataPacketList &packets)
{
    if (!m_enabled) {
        sendDownstream(packets);
        if (m_cache.size() > 0) {
            LOG_WARN("Discarding %zu frames worth of data due to disabling this functionality", m_cache.size());
            m_cache.clear();
        }
        return;
    }

    DasDataPacketList sendPackets;
    for (const auto &packet: packets) {
        // This is assuming that packet timestamps are monotonically
        // increasing or same. It uses that knowledge to optimize
        // cache access to always insert at the beginning for
        // low number of iterations required when searching, and poping
        // from the back when cache is full.
        epicsTime timestamp = packet->getTimeStamp();
        auto cached = m_cache.begin();
        for (; cached != m_cache.end(); cached++) {
            if (cached->timestamp == timestamp)
                break;
        }
        if (cached == m_cache.end()) {
            m_cache.push_front(PulseEvents(timestamp, m_devices.size()));
            cached = m_cache.begin();
        }

        // Potential mismatch when plugin connected to multiple parents and one
        // is mapped and the other one is not
        if (packet->getEventsMapped() == true && cached->mapped == false)
            cached->mapped = true;

        if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_PIXEL) {
            auto event = packet->getEvents<Event::Pixel>();
            uint32_t nEvents = packet->getNumEvents();
            while (nEvents-- > 0) {
                cached->pixel_neutrons.sortedInsert(*event);
                event++;
            }
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_BNL_DIAG) {
            auto event = packet->getEvents<Event::BNL::Diag>();
            uint32_t nEvents = packet->getNumEvents();
            while (nEvents-- > 0) {
                cached->bnl_neutrons.sortedInsert(*event);
                event++;
            }
        } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_META) {
            // Meta events are used here but not changed - will not send again
            sendPackets.push_back(packet);

            // Normalize TOF to nominal distance and put in sorted array
            const Event::Pixel *event = packet->getEvents<Event::Pixel>();
            uint32_t nEvents = packet->getNumEvents();
            while (nEvents-- > 0) {
                uint32_t pixelid = event->pixelid; // optimization: put event->pixelid to local CPU register
                for (size_t i = 0; i < m_devices.size(); i++) {
                    if (!m_devices[i].enabled)
                        continue;

                    if (pixelid == m_devices[i].pixelOn || pixelid == m_devices[i].pixelOff) {

                        Event::Pixel state_event;
                        state_event.tof = lround(event->tof * (m_distance / m_devices[i].distance));
                        state_event.pixelid = ((pixelid == m_devices[i].pixelOn ? 0x1 : 0x0) << 28);
                        state_event.pixelid |= i;

                        cached->states.sortedInsert(state_event);
                        break;
                    }
                    if (pixelid == m_devices[i].pixelVetoOn || pixelid == m_devices[i].pixelVetoOff) {

                        Event::Pixel state_event;
                        state_event.tof = lround(event->tof * (m_distance / m_devices[i].distance));
                        state_event.pixelid = ((pixelid == m_devices[i].pixelVetoOn ? 0x3 : 0x2) << 28);
                        state_event.pixelid |= i;

                        cached->states.sortedInsert(state_event);
                        break;
                    }
                }
                event++;
            }
        } else {
            // Pass other events un-processed
            sendPackets.push_back(packet);

            static bool logged = false;
            if (!logged) {
                LOG_WARN("StateAnalyzerPlugin setup but some events in this environment are not supported!");
                logged = true;
            }
        }

        // Send events from the back of the queue to processing thread
        // which in the end sends events to subscribed plugins.
        uint8_t retries = 0;
        while (m_cache.size() > m_maxCacheLen) {
            if (m_processQue.size() < m_maxCacheLen) {
                m_processQue.enqueue(std::move(m_cache.back()));
                m_cache.pop_back();
            } else {
                if (++retries > 1000) {
                    LOG_WARN("Background processing task is taking >1s");
                    break;
                }
                this->unlock();
                epicsThreadSleep(0.001);
                this->lock();
            }
        }
    }

    if (!sendPackets.empty()) {
        sendDownstream(sendPackets);
    }
}

void StateAnalyzerPlugin::processEvents(PulseEvents &pulseEvents)
{
    // We need to make local copies for calcCombinedStates to modify it.
    // Because we need previous pulse values in tagPixelIds()
    uint32_t state = m_state;
    uint32_t vetostate = m_vetostate;

    do {
        DasDataPacketList packets;

        // Calculate combined states
        if (!pulseEvents.states.empty()) {
            auto statesPkt = m_packetsPool.getPtr( DasDataPacket::getLength(DasDataPacket::EVENT_FMT_META,  pulseEvents.states.size())   );
            if (statesPkt) {
                packets.push_back(statesPkt.get());
                statesPkt->init(  DasDataPacket::EVENT_FMT_META,  pulseEvents.timestamp, pulseEvents.states.size());
                calcCombinedStates(pulseEvents.states, statesPkt->getEvents<Event::Pixel>(), state, vetostate);
            } else {
                this->lock();
                setIntegerParam(Status, 0);
                setStringParam(StatusText, "Insufficent memory");
                callParamCallbacks();
                LOG_ERROR("Failed to allocate output packets for combined states from pool");
                this->unlock();
                break;
            }
        }

        // Tag pixel ids in regular Event::Pixel format
        if (!pulseEvents.pixel_neutrons.empty()) {
            auto pixelsPkt = m_packetsPool.getPtr( DasDataPacket::getLength(DasDataPacket::EVENT_FMT_PIXEL, pulseEvents.pixel_neutrons.size()) );
            if (pixelsPkt) {
                packets.push_back(pixelsPkt.get());
                pixelsPkt->init(DasDataPacket::EVENT_FMT_PIXEL, pulseEvents.timestamp, pulseEvents.pixel_neutrons.size());
                pixelsPkt->setEventsMapped(pulseEvents.mapped);
                tagPixelIds<Event::Pixel>(pulseEvents.pixel_neutrons, pulseEvents.states, pixelsPkt->getEvents<Event::Pixel>());
            } else {
                this->lock();
                setIntegerParam(Status, 0);
                setStringParam(StatusText, "Insufficent memory");
                callParamCallbacks();
                LOG_ERROR("Failed to allocate output packets for neutron events from pool");
                this->unlock();
                break;
            }
        }

        // Tag pixel ids in Event::BNL::Diag format
        if (!pulseEvents.bnl_neutrons.empty()) {
            auto bnlPkt = m_packetsPool.getPtr( DasDataPacket::getLength(DasDataPacket::EVENT_FMT_BNL_DIAG, pulseEvents.bnl_neutrons.size()) );
            if (bnlPkt) {
                packets.push_back(bnlPkt.get());
                bnlPkt->init(DasDataPacket::EVENT_FMT_BNL_DIAG, pulseEvents.timestamp, pulseEvents.bnl_neutrons.size());
                bnlPkt->setEventsMapped(pulseEvents.mapped);
                tagPixelIds<Event::BNL::Diag>(pulseEvents.bnl_neutrons, pulseEvents.states, bnlPkt->getEvents<Event::BNL::Diag>());
            } else {
                this->lock();
                setIntegerParam(Status, 0);
                setStringParam(StatusText, "Insufficent memory");
                callParamCallbacks();
                LOG_ERROR("Failed to allocate output packets for neutron events from pool");
                this->unlock();
                break;
            }
        }

        // Now we can change those to be used in the next run round.
        m_state = state;
        m_vetostate = vetostate;

        // Send packets and update status
        this->lock();
        if (!packets.empty())
            sendDownstream(packets);
        setIntegerParam(State, state);
        setIntegerParam(Vetoing, vetostate != 0x0);
        callParamCallbacks();
        this->unlock();
    } while (false);
}

void StateAnalyzerPlugin::calcCombinedStates(EventList<Event::Pixel> &states, Event::Pixel *outEvents, uint32_t &state, uint32_t &vetostate) {

    for (auto state_event = states.begin(); state_event != states.end(); state_event++) {
        uint32_t i = state_event->pixelid & 0xFF;
        assert(i < m_devices.size());

        switch (state_event->pixelid >> 28) {
        case 0x0: // state OFF
            state &= ~(1 << m_devices[i].bitOffset);
            break;
        case 0x1: // state ON
            state |= (1 << m_devices[i].bitOffset);
            break;
        case 0x2: // veto OFF
            vetostate &= ~(1 << m_devices[i].bitOffset);
            break;
        case 0x3: // veto ON
            vetostate |= (1 << m_devices[i].bitOffset);
            break;
        default:
            break;
        }

        state_event->pixelid = m_statePixelMask | state;
        if (vetostate != 0x0)
            state_event->pixelid |= 0x100;

        // Copy to output array
        *outEvents = *state_event;
        outEvents++;
    }
}

template <typename T>
void StateAnalyzerPlugin::tagPixelIds(EventList<T> &events, const EventList<Event::Pixel> &states, T *outEvents)
{
    uint32_t state = m_state;
    uint32_t veto = (m_vetostate != 0x0 ? PIXEL_STATE_VETO_MASK : 0x0);

    // Double loop but the complexity of this entire function is really just O(Nevents+Nstates)
    // If Nevents is a huge number, the outer loop can be split between multiple threads
    auto state_event = states.begin();
    for (auto event = events.begin(); event != events.end(); event++) {
        // Project this event time of flight to the nominal distance domain
        // so that we only have to calculate it once per event.
        uint32_t tof = lround(m_distance * (event->tof / getPixelDistance(event->pixelid)));

        while (state_event != states.end()) {
            if (state_event->tof > tof) {
                break;
            }
            state = state_event->pixelid & 0xFF;
            veto = ((state_event->pixelid & 0x100) ? PIXEL_STATE_VETO_MASK : 0x0);
            state_event++;
        }
        event->pixelid |= ((state | veto) << m_bitOffset);

        // Copy to output array
        *outEvents = *event;
        outEvents++;
    }
}

double StateAnalyzerPlugin::getPixelDistance(uint32_t pixelid)
{
    return m_distance;
}

void StateAnalyzerPlugin::processThread(epicsEvent *shutdown)
{
    LOG_DEBUG("Processing thread started");
    while (shutdown->tryWait() == false) {
        PulseEvents pulseEvents(epicsTimeStamp(), 0);
        if (m_processQue.deque(pulseEvents, 0.1)) {
            processEvents(pulseEvents);
        }
    }
    LOG_DEBUG("Processing thread exiting");
}

template <typename T>
void StateAnalyzerPlugin::EventList<T>::sortedInsert(const T &event) {
    for (auto it = this->rbegin(); it != this->rend(); it++) {
        if (event.tof >= it->tof) {
            this->insert(it.base(), event);
            return;
        }
    }
    this->push_front(event);
}
