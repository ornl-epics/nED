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
        // Pass meta and other events un-processed
        if (packet->getEventsFormat() != DasDataPacket::EVENT_FMT_PIXEL)
            sendPackets.push_back(packet);

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

        // Meta events are used here but not changed - will not send again
        if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_PIXEL ||
            packet->getEventsFormat() == DasDataPacket::EVENT_FMT_META) {

            // Separate out signal events
            const Event::Pixel *event = packet->getEvents<Event::Pixel>();
            uint32_t nEvents = packet->getNumEvents();
            while (nEvents-- > 0) {
                if (event->getType() == Event::Pixel::Type::NEUTRON) {
                    cached->neutrons.sortedInsert(*event);
                } else {
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
                }
                event++;
            }
        }

        // Send events from the back of the queue to processing thread
        // which in the end sends events to subscribed plugins.
        uint8_t retries = 0;
        while (m_cache.size() > m_maxCacheLen) {
            if (m_processQue.size() < m_maxCacheLen) {
                if (m_cache.back().neutrons.size() > 0 || m_cache.back().states.size() >0) {
                    m_processQue.enqueue(std::move(m_cache.back()));
                }
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

void StateAnalyzerPlugin::processEvents(PulseEvents &pulseEvents, Event::Pixel *neutrons, Event::Pixel *states) {
    // We'll need these in the neutron pixel changing loop,
    // it's important to capture before m_state and m_vetostate
    // are modified by the combined state loop.
    uint8_t state = m_state;
    uint32_t veto = (m_vetostate != 0x0 ? PIXEL_STATE_VETO_MASK : 0x0);

    // First convert raw signals to combined state events.
    // Assumption: the state array is pre-sorted
    for (auto state_event = pulseEvents.states.begin(); state_event != pulseEvents.states.end(); state_event++) {
        uint32_t i = state_event->pixelid & 0xFF;
        assert(i < m_devices.size());

        switch (state_event->pixelid >> 28) {
        case 0x0: // state OFF
            m_state &= ~(1 << m_devices[i].bitOffset);
            break;
        case 0x1: // state ON
            m_state |= (1 << m_devices[i].bitOffset);
            break;
        case 0x2: // veto OFF
            m_vetostate &= ~(1 << m_devices[i].bitOffset);
            break;
        case 0x3: // veto ON
            m_vetostate |= (1 << m_devices[i].bitOffset);
            break;
        default:
            break;
        }

        state_event->pixelid = m_statePixelMask | m_state;
        if (m_vetostate != 0x0)
            state_event->pixelid |= 0x100;

        *states = *state_event;
        states++;
    }
    
    // Double loop but the complexity of this entire function is really just O(Nneutrons+Nstates)
    // If Nneutrons is a huge number, the outer loop can be split between multiple threads
    auto state_event = pulseEvents.states.begin();
    for (auto event = pulseEvents.neutrons.begin(); event != pulseEvents.neutrons.end(); event++) {
        // Project this event time of flight to the nominal distance domain
        // so that we only have to calculate it once per event.
        uint32_t tof = lround(m_distance * (event->tof / getPixelDistance(event->pixelid)));

        while (state_event != pulseEvents.states.end()) {
            if (state_event->tof > tof) {
                break;
            }
            state = state_event->pixelid & 0xFF;
            veto = ((state_event->pixelid & 0x100) ? PIXEL_STATE_VETO_MASK : 0x0);
            state_event++;
        }

        neutrons->tof = event->tof;
        neutrons->pixelid = event->pixelid | (state << m_bitOffset) | veto;
        neutrons++;
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
            // Allocate outgoing packets from pool
            auto neutronsPkt = m_packetsPool.getPtr( DasDataPacket::getLength(DasDataPacket::EVENT_FMT_PIXEL, pulseEvents.neutrons.size()) );
            auto statesPkt   = m_packetsPool.getPtr( DasDataPacket::getLength(DasDataPacket::EVENT_FMT_META,  pulseEvents.states.size())   );

            if (neutronsPkt && statesPkt) {
                neutronsPkt->init(DasDataPacket::EVENT_FMT_PIXEL, pulseEvents.timestamp, pulseEvents.neutrons.size());
                statesPkt->init(  DasDataPacket::EVENT_FMT_META,  pulseEvents.timestamp, pulseEvents.states.size());
                
                // Do the heavy lifting
                processEvents(pulseEvents, neutronsPkt->getEvents<Event::Pixel>(), statesPkt->getEvents<Event::Pixel>());

                // And finally send 2 packets out
                DasDataPacketList packets;
                packets.push_back(neutronsPkt.get());
                packets.push_back(statesPkt.get());

                this->lock();
                sendDownstream(packets);
                setIntegerParam(State, m_state);
                setIntegerParam(Vetoing, m_vetostate != 0x0);
                callParamCallbacks();
                this->unlock();
            } else {
                this->lock();
                setIntegerParam(Status, 0);
                setStringParam(StatusText, "Insufficent memory");
                callParamCallbacks();
                this->unlock();
                LOG_ERROR("Failed to allocate output packets from pool");
            }
        }
    }
    LOG_DEBUG("Processing thread exiting");
}

void StateAnalyzerPlugin::EventList::sortedInsert(const Event::Pixel &event) {
    for (auto it = this->rbegin(); it != this->rend(); it++) {
        if (event.tof >= it->tof) {
            this->insert(it.base(), event);
            return;
        }
    }
    this->push_front(event);
}
