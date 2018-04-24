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

EPICS_REGISTER_PLUGIN(StateAnalyzerPlugin, 3, "Port name", string, "Parent plugins", string, "Number of triggers", int);

StateAnalyzerPlugin::StateAnalyzerPlugin(const char *portName, const char *parentPlugins, uint8_t numTriggers)
    : BasePlugin(portName, 1, asynFloat64Mask|asynOctetMask, asynFloat64Mask|asynOctetMask)
    , m_processThread("StateAnalyzerThread", std::bind(&StateAnalyzerPlugin::processThread, this, std::placeholders::_1))
    , Devices(numTriggers)
{
    createParam("Status",       asynParamInt32, &Status, 1);        // READ - Plugin status, 0=error, 1=ok, 2=warning
    createParam("StatusText",   asynParamOctet, &StatusText, "");   // READ - Text description of the error, if any
    createParam("MaxCacheLen",  asynParamInt32, &MaxCacheLen, 10);  // WRITE - Number of pulses to cache events before sending to other plugins
    createParam("Enable",       asynParamInt32, &Enable, 0);        // WRITE - Toggle processing any data by this plugin, when off all packets are passed thru
    createParam("State",        asynParamInt32, &State, 0);         // READ - Current state
    createParam("Vetoing",      asynParamInt32, &Vetoing, 0);       // READ - Current state of vetoing
    createParam("NominalDist",  asynParamFloat64, &NominalDist, 1);   // WRITE - Nominal detector distance
    for (uint8_t i = 0; i < numTriggers; i++) {
        int param;
        char name[16];

        m_devices.push_back({0,0,0,0});

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
    if (pasynUser->reason == Enable) {
        m_enabled = (value > 0);
        return asynSuccess;
    }
    for (size_t i = 0; i < Devices.size(); i++) {
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
        return;
    }

    DasDataPacketList sendPackets;
    for (const auto &packet: packets) {
        // Pass meta and other events un-processed
        if (packet->getEventsFormat() != DasDataPacket::EVENT_FMT_PIXEL)
            sendPackets.push_back(packet);

        // Meta events are used but not changed - will not send again
        if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_PIXEL ||
            packet->getEventsFormat() == DasDataPacket::EVENT_FMT_META) {

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

            // Separate out signal events
            const Event::Pixel *event = packet->getEvents<Event::Pixel>();
            uint32_t nEvents = packet->getNumEvents();
            for (uint32_t i = 0; i < nEvents; i++) {
                uint32_t pixelid = event->pixelid; // optimization: put event->pixelid to local CPU register
                if (Event::getPixelType(pixelid) == Event::PixelType::NEUTRON) {
                    cached->neutrons.sortedInsert(*event);
                } else {
                    bool isSignal = false;
                    for (size_t i = 0; i < m_devices.size(); i++) {
                        if (pixelid == m_devices[i].pixelOn || pixelid == m_devices[i].pixelOff ||
                            pixelid == m_devices[i].pixelVetoOn || pixelid == m_devices[i].pixelVetoOff) {

                            cached->signals[i].sortedInsert(*event);
                            isSignal = true;
                            break;
                        }
                    }
                    if (!isSignal) {
                        cached->others.push_back(*event);
                    }
                }
                event++;
            }

            // Send events from the back of the queue to processing thread
            // which in the end sends only neutron events to subscribed plugins.
            while (m_cache.size() > m_maxCacheLen) {
                if (m_processQue.size() < m_maxCacheLen) {
                    m_processQue.enqueue(std::move(m_cache.back()));
                    m_cache.pop_back();
                } else {
                    LOG_ERROR("No space left in processing queue, dropping entire frame of events");
                }
            }
        }
    }

    if (!sendPackets.empty()) {
        sendDownstream(sendPackets);
    }
}

void StateAnalyzerPlugin::flagNeutrons(PulseEvents &pulseEvents) {
    // Cache pointers into signal arrays.
    // That way when iterating through neutron events, we can skip
    // signal events we have already visited, since arrays are guaranteed to
    // be sorted.
    std::vector<EventList::const_iterator> arrayOffsets(pulseEvents.signals.size());
    for (size_t i = 0; i < pulseEvents.signals.size(); i++) {
        arrayOffsets[i] = pulseEvents.signals[i].begin();
    }

    // The events array could be devided into sections and processed
    // by individual threads.
    for (auto event = pulseEvents.neutrons.begin(); event != pulseEvents.neutrons.end(); event++) {
        double inv_speed = lround(event->tof / getEventDistance(event->pixelid));

        for (size_t i = 0; i < pulseEvents.signals.size(); i++) {
            // Alias values for efficiency
            uint32_t vetoOn = m_devices[i].pixelVetoOn;
            uint32_t vetoOff = m_devices[i].pixelVetoOff;
            uint32_t stateOn = m_devices[i].pixelOn;
            uint32_t stateOff = m_devices[i].pixelOff;
            uint32_t stateBit = 1 << m_devices[i].bitOffset;

            // Calculate time when neutron passed this signal
            uint32_t tof = inv_speed * m_devices[i].distance;

            // Grab state from previous packets, potentially frames
            bool veto = ((m_vetostate & stateBit) != 0x0);
            bool state = ((m_state & stateBit) != 0x0);

            for (auto signal_event = arrayOffsets[i]; signal_event != pulseEvents.signals[i].end(); signal_event++) {
                if (signal_event->tof > tof) {
                    // Skip already processed event, the next time this loop is called
                    arrayOffsets[i] = signal_event;
                    // Stop looking, all further TOFs are even bigger
                    break;
                }

                // Keep track of state and veto
                if (signal_event->pixelid == stateOn)
                    state = true;
                else if (signal_event->pixelid == stateOff)
                    state = false;
                else if (signal_event->pixelid == vetoOn)
                    veto = true;
                else if (signal_event->pixelid == vetoOff)
                    veto = false;
            }

            if (state) {
                event->pixelid |= stateBit;
                m_state |= stateBit;
            } else {
                event->pixelid &= ~stateBit;
                m_state &= ~stateBit;
            }

            if (veto) {
                m_vetostate |= stateBit;
                // Once vetoed stays vetoed, but keep adding state id
                // in case someone is interested
                event->pixelid |= Event::PIXEL_VETO_MASK;
            } else {
                m_vetostate &= ~stateBit;
            }
        }
    }
}

double StateAnalyzerPlugin::getEventDistance(uint32_t pixelid)
{
    return m_distance;
}

void StateAnalyzerPlugin::sendDownstream(const PulseEvents &pulseEvents)
{
    uint32_t nEvents = pulseEvents.neutrons.size();
    uint32_t packetLen = DasDataPacket::getLength(DasDataPacket::EVENT_FMT_PIXEL, nEvents);
    DasDataPacket *packet = m_packetsPool.get(packetLen);
    if (packet) {
        packet->init(DasDataPacket::EVENT_FMT_PIXEL, pulseEvents.timestamp, nEvents);
        Event::Pixel *event = packet->getEvents<Event::Pixel>();
        for (auto it = pulseEvents.neutrons.begin(); it != pulseEvents.neutrons.end(); it++) {
            event->tof = it->tof;
            event->pixelid = it->pixelid;
            event++;
        }

        this->lock();
        sendDownstream({packet});
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
        LOG_ERROR("Failed to allocate DasDataPacket of %u bytes from pool", packetLen);
    }
}

void StateAnalyzerPlugin::processThread(epicsEvent *shutdown)
{
    while (shutdown->tryWait() == false) {
        PulseEvents pulseEvents(epicsTimeStamp(), 0);
        if (m_processQue.deque(pulseEvents, 0.1)) {
            flagNeutrons(pulseEvents);
            sendDownstream(pulseEvents);
        }
    }
}

void StateAnalyzerPlugin::EventList::sortedInsert(const Event::Pixel &event) {
    // Optimize the most likely case
    if (this->back().tof < event.tof) {
        this->push_back(event);
        return;
    }

    // Bad luck, we have to find a spot in the middle of array.
    // Since this is unlikely case, don't bother with binary search.
    for (auto it = this->begin(); it != this->end(); it++) {
        if (event.tof < it->tof) {
            this->insert(it, event);
            return;
        }
    }
}
