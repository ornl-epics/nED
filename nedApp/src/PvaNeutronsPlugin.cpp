/* PvaNeutronsPlugin.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Event.h"
#include "Log.h"
#include "PvaNeutronsPlugin.h"

// EPICS base includes
#include <alarm.h>

// EPICS v4 includes
#include <pv/pvDatabase.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>


/**
 * PVRecord for Neutrons channel.
 */
class PvaNeutronsPlugin::PvaRecordPixel : public epics::pvDatabase::PVRecord {
    private:
        /**
         * C'tor.
         */
        PvaRecordPixel(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure)
            , m_sequence(0)
        {}

        template <typename T>
        std::tuple<uint32_t, epics::pvData::PVUIntArray::svector, epics::pvData::PVUIntArray::svector> getTofPixels(const DasDataPacket *packet)
        {
            uint32_t nEvents = packet->getNumEvents();
            const T *events = packet->getEvents<T>();

            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            for (uint32_t i = 0; i < nEvents; i++) {
                tofs[i]   = events[i].tof;
                pixels[i] = events[i].pixelid;
            }
            return std::make_tuple(nEvents, tofs, pixels);
        }

        template <typename T>
        std::tuple<uint32_t, epics::pvData::PVUIntArray::svector, epics::pvData::PVUIntArray::svector> getTofPixelsMapped(const DasDataPacket *packet)
        {
            uint32_t nEvents = packet->getNumEvents();
            const T *events = packet->getEvents<T>();

            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            for (uint32_t i = 0; i < nEvents; i++) {
                tofs[i]   = events[i].tof;
                pixels[i] = events[i].mapped_pixelid;
            }
            return std::make_tuple(nEvents, tofs, pixels);
        }

    public:
        POINTER_DEFINITIONS(PvaRecordPixel);

        /**
         * Allocate and initialize PvaRecordPixel and attach to all fields.
         */
        static PvaRecordPixel::shared_pointer create(const std::string &recordName)
        {
            using namespace epics::pvData;

            StandardFieldPtr standardField = getStandardField();
            FieldCreatePtr fieldCreate     = getFieldCreate();
            PVDataCreatePtr pvDataCreate   = getPVDataCreate();

            PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
                fieldCreate->createFieldBuilder()->
                    add("timeStamp",        standardField->timeStamp())->
                    add("logical",          standardField->scalar(pvBoolean, ""))->
                    add("num_events",       standardField->scalar(pvUInt, ""))->
                    add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("pixel",            standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    // proton_charge field is here temporarily until ADnED can use RtdlPlugin instead
                    add("proton_charge",    standardField->scalar(pvDouble, "display"))->
                    createStructure()
            );

            PvaRecordPixel::shared_pointer pvRecord(new PvaRecordPixel(recordName, pvStructure));
            if (!pvRecord)
                return PvaRecordPixel::shared_pointer();

            pvRecord->initPVRecord();

            if (!pvRecord->pvTimeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp")))
                return PvaRecordPixel::shared_pointer();

            pvRecord->pvLogical = pvRecord->getPVStructure()->getSubField<epics::pvData::PVBoolean>("logical.value");
            if (pvRecord->pvLogical.get() == NULL)
                return PvaRecordPixel::shared_pointer();

            pvRecord->pvNumEvents = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
            if (pvRecord->pvNumEvents.get() == NULL)
                return PvaRecordPixel::shared_pointer();

            pvRecord->pvTimeOfFlight = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
            if (pvRecord->pvTimeOfFlight.get() == NULL)
                return PvaRecordPixel::shared_pointer();

            pvRecord->pvPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
            if (pvRecord->pvPixel.get() == NULL)
                return PvaRecordPixel::shared_pointer();

            // Again, temporarily here
            pvRecord->pvProtonCharge = pvRecord->getPVStructure()->getSubField<epics::pvData::PVDouble>("proton_charge.value");
            if (pvRecord->pvProtonCharge.get() == NULL)
                return PvaRecordPixel::shared_pointer();

            return pvRecord;
        }

        /**
         * Publish a single atomic update of the PV, take values from packet.
         */
        bool update(const DasDataPacket *packet, uint32_t &nEvents, double pCharge)
        {
            bool posted = false;

            // 31 bit sequence number is good for around 9 months.
            // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
            // In worst case client will skip one packet on rollover and then recover
            // the sequence.
            epicsTimeStamp ts = packet->getTimeStamp();
            epics::pvData::TimeStamp timestamp(
                epics::pvData::posixEpochAtEpicsEpoch + ts.secPastEpoch,
                ts.nsec,
                m_sequence++ % 0x7FFFFFFF
            );

            // Now extract events from packet
            epics::pvData::PVUIntArray::svector tofs;
            epics::pvData::PVUIntArray::svector pixels;
            bool mapped = false;
            switch (packet->getEventsFormat()) {
                case DasDataPacket::EVENT_FMT_PIXEL:
                    std::tie(nEvents, tofs, pixels) = getTofPixels<Event::Pixel>(packet);
                    break;
                case DasDataPacket::EVENT_FMT_PIXEL_MAPPED:
                    mapped = true;
                    std::tie(nEvents, tofs, pixels) = getTofPixels<Event::Pixel>(packet);
                    break;
                case DasDataPacket::EVENT_FMT_LPSD_VERBOSE:
                    std::tie(nEvents, tofs, pixels) = getTofPixels<Event::LPSD::Verbose>(packet);
                    break;
                case DasDataPacket::EVENT_FMT_LPSD_DIAG:
                    mapped = true;
                    std::tie(nEvents, tofs, pixels) = getTofPixelsMapped<Event::LPSD::Diag>(packet);
                    break;
                case DasDataPacket::EVENT_FMT_BNL_VERBOSE:
                    std::tie(nEvents, tofs, pixels) = getTofPixels<Event::BNL::Verbose>(packet);
                    break;
                case DasDataPacket::EVENT_FMT_BNL_DIAG:
                    mapped = true;
                    std::tie(nEvents, tofs, pixels) = getTofPixelsMapped<Event::BNL::Diag>(packet);
                    break;
                case DasDataPacket::EVENT_FMT_TIME_CALIB:
                    mapped = true;
                    nEvents = 0;
                    break;
                default:
                    nEvents = 0;
                    return false;
            }

            lock();
            try {
                beginGroupPut();

                pvTimeStamp.set(timestamp);
                pvLogical->put(mapped);
                pvNumEvents->put(nEvents);
                pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
                pvPixel->replace(epics::pvData::freeze(pixels));

                endGroupPut();
                posted = true;
            } catch (...) {
            }
            unlock();

            return posted;
        }

    private:
        uint32_t m_sequence;
        epics::pvData::PVTimeStamp      pvTimeStamp;    //!< Time stamp common to all events
        epics::pvData::PVBooleanPtr     pvLogical;      //!< Flags whether pixels are mapped to logical ids
        epics::pvData::PVUIntPtr        pvNumEvents;    //!< Number of events in tof,pixelid arrays
        epics::pvData::PVUIntArrayPtr   pvTimeOfFlight; //!< Time offset relative to time stamp
        epics::pvData::PVUIntArrayPtr   pvPixel;        //!< Pixel ID
        epics::pvData::PVDoublePtr      pvProtonCharge; //!< Proton charge in for this pulse

};

/**
 * PVRecord for LPSD channel.
 */
class PvaNeutronsPlugin::PvaRecordLpsd : public epics::pvDatabase::PVRecord {
    private:
        /**
         * C'tor.
         */
        PvaRecordLpsd(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure)
            , m_sequence(0)
        {}

        template <typename T>
        std::tuple<uint32_t, epics::pvData::PVUIntArray::svector, epics::pvData::PVUIntArray::svector> getTofPixels(const DasDataPacket *packet)
        {
            uint32_t nEvents = packet->getNumEvents();
            const T *events = packet->getEvents<T>();

            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            for (uint32_t i = 0; i < nEvents; i++) {
                tofs[i]   = events[i].tof;
                pixels[i] = events[i].pixelid;
            }
            return std::make_tuple(nEvents, tofs, pixels);
        }

    public:
        POINTER_DEFINITIONS(PvaRecordLpsd);

        /**
         * Allocate and initialize PvaRecordLpsd and attach to all fields.
         */
        static PvaRecordLpsd::shared_pointer create(const std::string &recordName)
        {
            using namespace epics::pvData;

            StandardFieldPtr standardField = getStandardField();
            FieldCreatePtr fieldCreate     = getFieldCreate();
            PVDataCreatePtr pvDataCreate   = getPVDataCreate();

            PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
                fieldCreate->createFieldBuilder()->
                    add("timeStamp",        standardField->timeStamp())->
                    add("num_events",       standardField->scalar(pvUInt, ""))->
                    add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("position",         standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("pixel",            standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("mapped_pixel",     standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a1",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a2",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b1",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b2",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    createStructure()
            );

            PvaRecordLpsd::shared_pointer pvRecord(new PvaRecordLpsd(recordName, pvStructure));
            if (!pvRecord)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->initPVRecord();

            if (!pvRecord->pvTimeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp")))
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvNumEvents = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
            if (pvRecord->pvNumEvents.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvTimeOfFlight = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
            if (pvRecord->pvTimeOfFlight.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvPosition = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position.value");
            if (pvRecord->pvPosition.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
            if (pvRecord->pvPixel.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvMappedPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("mapped_pixel.value");
            if (pvRecord->pvMappedPixel.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvSampleA1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a1.value");
            if (pvRecord->pvSampleA1.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvSampleA2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a2.value");
            if (pvRecord->pvSampleA2.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvSampleB1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b1.value");
            if (pvRecord->pvSampleB1.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            pvRecord->pvSampleB2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b2.value");
            if (pvRecord->pvSampleB2.get() == NULL)
                return PvaRecordLpsd::shared_pointer();

            return pvRecord;
        }

        /**
         * Publish a single atomic update of the PV, take values from packet.
         */
        bool update(const DasDataPacket *packet, uint32_t &nEvents)
        {
            bool posted = false;

            // 31 bit sequence number is good for around 9 months.
            // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
            // In worst case client will skip one packet on rollover and then recover
            // the sequence.
            epicsTimeStamp ts = packet->getTimeStamp();
            epics::pvData::TimeStamp timestamp(
                epics::pvData::posixEpochAtEpicsEpoch + ts.secPastEpoch,
                ts.nsec,
                m_sequence++ % 0x7FFFFFFF
            );

            // Pre-allocate svector to minimize gradual memory allocations
            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            epics::pvData::PVUIntArray::svector mapped_pixels(nEvents);
            epics::pvData::PVUIntArray::svector positions(nEvents);
            epics::pvData::PVUShortArray::svector sample_a1(nEvents);
            epics::pvData::PVUShortArray::svector sample_a2(nEvents);
            epics::pvData::PVUShortArray::svector sample_b1(nEvents);
            epics::pvData::PVUShortArray::svector sample_b2(nEvents);

            // Now extract events from packet
            if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_LPSD_RAW) {
                const Event::LPSD::Raw *events = packet->getEvents<Event::LPSD::Raw>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]   = events[i].tof;
                    positions[i] = events[i].position;
                    pixels[i] = 0;
                    mapped_pixels[i] = 0;
                    sample_a1[i]  = events[i].sample_a1;
                    sample_a2[i]  = events[i].sample_a2;
                    sample_b1[i]  = events[i].sample_b1;
                    sample_b2[i]  = events[i].sample_b2;
                }
            } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_LPSD_VERBOSE) {
                const Event::LPSD::Verbose *events = packet->getEvents<Event::LPSD::Verbose>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]   = events[i].tof;
                    positions[i] = events[i].position;
                    pixels[i] = events[i].pixelid;
                    mapped_pixels[i] = 0;
                    sample_a1[i]  = events[i].sample_a1;
                    sample_a2[i]  = events[i].sample_a2;
                    sample_b1[i]  = events[i].sample_b1;
                    sample_b2[i]  = events[i].sample_b2;
                }
            } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_LPSD_DIAG) {
                const Event::LPSD::Diag *events = packet->getEvents<Event::LPSD::Diag>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]   = events[i].tof;
                    positions[i] = events[i].position;
                    pixels[i] = events[i].pixelid;
                    mapped_pixels[i] = events[i].mapped_pixelid;
                    sample_a1[i]  = events[i].sample_a1;
                    sample_a2[i]  = events[i].sample_a2;
                    sample_b1[i]  = events[i].sample_b1;
                    sample_b2[i]  = events[i].sample_b2;
                }
            } else {
                nEvents = 0;
                return false;
            }

            lock();
            try {
                beginGroupPut();

                pvTimeStamp.set(timestamp);
                pvNumEvents->put(nEvents);
                pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
                pvPixel->replace(epics::pvData::freeze(pixels));

                endGroupPut();
                posted = true;
            } catch (...) {
            }
            unlock();

            return posted;
        }

    private:
        uint32_t m_sequence;
        epics::pvData::PVTimeStamp      pvTimeStamp;    //!< Time stamp common to all events
        epics::pvData::PVUIntPtr        pvNumEvents;    //!< Number of events in tof,pixelid arrays
        epics::pvData::PVUIntArrayPtr   pvTimeOfFlight; //!< Time offset relative to time stamp
        epics::pvData::PVUIntArrayPtr   pvPosition;     //!< Detector position
        epics::pvData::PVUIntArrayPtr   pvPixel;        //!< Pixel ID
        epics::pvData::PVUIntArrayPtr   pvMappedPixel;  //!< Mapped pixel ID
        epics::pvData::PVUIntArrayPtr   pvSampleA1;     //!< Raw sample A1
        epics::pvData::PVUIntArrayPtr   pvSampleA2;     //!< Raw sample A2
        epics::pvData::PVUIntArrayPtr   pvSampleB1;     //!< Raw sample B1
        epics::pvData::PVUIntArrayPtr   pvSampleB2;     //!< Raw sample B2
};

/**
 * PVRecord for BNL channel.
 */
class PvaNeutronsPlugin::PvaRecordBnl : public epics::pvDatabase::PVRecord {
    private:
        /**
         * C'tor.
         */
        PvaRecordBnl(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure)
            , m_sequence(0)
        {}

    public:
        POINTER_DEFINITIONS(PvaRecordBnl);

        /**
         * Allocate and initialize PvaRecordBnl.
         */
        static PvaNeutronsPlugin::PvaRecordBnl::shared_pointer create(const std::string &recordName) {
            using namespace epics::pvData;

            StandardFieldPtr standardField = getStandardField();
            FieldCreatePtr fieldCreate     = getFieldCreate();
            PVDataCreatePtr pvDataCreate   = getPVDataCreate();

            PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
                fieldCreate->createFieldBuilder()->
                    add("timeStamp",        standardField->timeStamp())->
                    add("num_events",       standardField->scalar(pvUInt, ""))->
                    add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("position",         standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("x",                standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("y",                standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("corrected_x",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("corrected_y",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("pixel",            standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("mapped_pixel",     standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_x1",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x2",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x3",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x4",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x5",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x6",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x7",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x8",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x9",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x10",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x11",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x12",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x13",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x14",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x15",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x16",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x17",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x18",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x19",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_x20",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y1",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y2",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y3",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y4",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y5",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y6",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y7",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y8",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y9",        standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y10",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y11",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y12",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y13",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y14",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y15",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y16",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("sample_y17",       standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    createStructure()
            );

            PvaRecordBnl::shared_pointer pvRecord(new PvaRecordBnl(recordName, pvStructure));
            if (!pvRecord)
                return PvaRecordBnl::shared_pointer();

            pvRecord->initPVRecord();

            if (!pvRecord->pvTimeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp")))
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvNumEvents = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
            if (pvRecord->pvNumEvents.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvTimeOfFlight = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
            if (pvRecord->pvTimeOfFlight.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvPosition = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position.value");
            if (pvRecord->pvPosition.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
            if (pvRecord->pvPixel.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvMappedPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("mapped_pixel.value");
            if (pvRecord->pvMappedPixel.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("x.value");
            if (pvRecord->pvX.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("y.value");
            if (pvRecord->pvY.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvCorrectedX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("corrected_x.value");
            if (pvRecord->pvCorrectedX.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvCorrectedY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("corrected_y.value");
            if (pvRecord->pvCorrectedY.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x1.value");
            if (pvRecord->pvSampleX1.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x2.value");
            if (pvRecord->pvSampleX2.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x3.value");
            if (pvRecord->pvSampleX3.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x4.value");
            if (pvRecord->pvSampleX4.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x5.value");
            if (pvRecord->pvSampleX5.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x6.value");
            if (pvRecord->pvSampleX6.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x7.value");
            if (pvRecord->pvSampleX7.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX8 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x8.value");
            if (pvRecord->pvSampleX8.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX9 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x9.value");
            if (pvRecord->pvSampleX9.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX10 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x10.value");
            if (pvRecord->pvSampleX10.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX11 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x11.value");
            if (pvRecord->pvSampleX11.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX12 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x12.value");
            if (pvRecord->pvSampleX12.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX13 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x13.value");
            if (pvRecord->pvSampleX13.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX14 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x14.value");
            if (pvRecord->pvSampleX14.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX15 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x15.value");
            if (pvRecord->pvSampleX15.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX16 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x16.value");
            if (pvRecord->pvSampleX16.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX17 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x17.value");
            if (pvRecord->pvSampleX17.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX18 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x18.value");
            if (pvRecord->pvSampleX18.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX19 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x19.value");
            if (pvRecord->pvSampleX19.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleX20 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_x20.value");
            if (pvRecord->pvSampleX20.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y1.value");
            if (pvRecord->pvSampleY1.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y2.value");
            if (pvRecord->pvSampleY2.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y3.value");
            if (pvRecord->pvSampleY3.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y4.value");
            if (pvRecord->pvSampleY4.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y5.value");
            if (pvRecord->pvSampleY5.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y6.value");
            if (pvRecord->pvSampleY6.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y7.value");
            if (pvRecord->pvSampleY7.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY8 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y8.value");
            if (pvRecord->pvSampleY8.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY9 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y9.value");
            if (pvRecord->pvSampleY9.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY10 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y10.value");
            if (pvRecord->pvSampleY10.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY11 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y11.value");
            if (pvRecord->pvSampleY11.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY12 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y12.value");
            if (pvRecord->pvSampleY12.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY13 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y13.value");
            if (pvRecord->pvSampleY13.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY14 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y14.value");
            if (pvRecord->pvSampleY14.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY15 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y15.value");
            if (pvRecord->pvSampleY15.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY16 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y16.value");
            if (pvRecord->pvSampleY16.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            pvRecord->pvSampleY17 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("sample_y17.value");
            if (pvRecord->pvSampleY17.get() == NULL)
                return PvaRecordBnl::shared_pointer();

            return pvRecord;

        }

        /**
         * Publish a single atomic update of the PV, take values from packet.
         */
        bool update(const DasDataPacket *packet, uint32_t &nEvents) {
            bool posted = false;
            epicsTimeStamp ts = packet->getTimeStamp();

            // 31 bit sequence number is good for around 9 months.
            // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
            // In worst case client will skip one packet on rollover and then recover
            // the sequence.
            epics::pvData::TimeStamp timestamp(
                epics::pvData::posixEpochAtEpicsEpoch + ts.secPastEpoch,
                ts.nsec,
                m_sequence++ % 0x7FFFFFFF
            );

            nEvents = packet->getNumEvents();

            // Pre-allocate svector to minimize gradual memory allocations
            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            epics::pvData::PVUIntArray::svector mapped_pixels(nEvents);
            epics::pvData::PVUIntArray::svector positions(nEvents);
            epics::pvData::PVFloatArray::svector xs(nEvents);
            epics::pvData::PVFloatArray::svector ys(nEvents);
            epics::pvData::PVFloatArray::svector corrected_xs(nEvents);
            epics::pvData::PVFloatArray::svector corrected_ys(nEvents);
            epics::pvData::PVUShortArray::svector sample_x1(nEvents);
            epics::pvData::PVUShortArray::svector sample_x2(nEvents);
            epics::pvData::PVUShortArray::svector sample_x3(nEvents);
            epics::pvData::PVUShortArray::svector sample_x4(nEvents);
            epics::pvData::PVUShortArray::svector sample_x5(nEvents);
            epics::pvData::PVUShortArray::svector sample_x6(nEvents);
            epics::pvData::PVUShortArray::svector sample_x7(nEvents);
            epics::pvData::PVUShortArray::svector sample_x8(nEvents);
            epics::pvData::PVUShortArray::svector sample_x9(nEvents);
            epics::pvData::PVUShortArray::svector sample_x10(nEvents);
            epics::pvData::PVUShortArray::svector sample_x11(nEvents);
            epics::pvData::PVUShortArray::svector sample_x12(nEvents);
            epics::pvData::PVUShortArray::svector sample_x13(nEvents);
            epics::pvData::PVUShortArray::svector sample_x14(nEvents);
            epics::pvData::PVUShortArray::svector sample_x15(nEvents);
            epics::pvData::PVUShortArray::svector sample_x16(nEvents);
            epics::pvData::PVUShortArray::svector sample_x17(nEvents);
            epics::pvData::PVUShortArray::svector sample_x18(nEvents);
            epics::pvData::PVUShortArray::svector sample_x19(nEvents);
            epics::pvData::PVUShortArray::svector sample_x20(nEvents);
            epics::pvData::PVUShortArray::svector sample_y1(nEvents);
            epics::pvData::PVUShortArray::svector sample_y2(nEvents);
            epics::pvData::PVUShortArray::svector sample_y3(nEvents);
            epics::pvData::PVUShortArray::svector sample_y4(nEvents);
            epics::pvData::PVUShortArray::svector sample_y5(nEvents);
            epics::pvData::PVUShortArray::svector sample_y6(nEvents);
            epics::pvData::PVUShortArray::svector sample_y7(nEvents);
            epics::pvData::PVUShortArray::svector sample_y8(nEvents);
            epics::pvData::PVUShortArray::svector sample_y9(nEvents);
            epics::pvData::PVUShortArray::svector sample_y10(nEvents);
            epics::pvData::PVUShortArray::svector sample_y11(nEvents);
            epics::pvData::PVUShortArray::svector sample_y12(nEvents);
            epics::pvData::PVUShortArray::svector sample_y13(nEvents);
            epics::pvData::PVUShortArray::svector sample_y14(nEvents);
            epics::pvData::PVUShortArray::svector sample_y15(nEvents);
            epics::pvData::PVUShortArray::svector sample_y16(nEvents);
            epics::pvData::PVUShortArray::svector sample_y17(nEvents);

            if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_BNL_RAW) {
                const Event::BNL::Raw *events = packet->getEvents<Event::BNL::Raw>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]   = events[i].tof;
                    positions[i] = events[i].position;
                    pixels[i] = 0;
                    mapped_pixels[i] = 0;
                    xs[i] = -1;
                    ys[i] = -1;
                    corrected_xs[i] = -1;
                    corrected_ys[i] = -1;
                    sample_x1[i]  = events[i].sample_x1;
                    sample_x2[i]  = events[i].sample_x2;
                    sample_x3[i]  = events[i].sample_x3;
                    sample_x4[i]  = events[i].sample_x4;
                    sample_x5[i]  = events[i].sample_x5;
                    sample_x6[i]  = events[i].sample_x6;
                    sample_x7[i]  = events[i].sample_x7;
                    sample_x8[i]  = events[i].sample_x8;
                    sample_x9[i]  = events[i].sample_x9;
                    sample_x10[i] = events[i].sample_x10;
                    sample_x11[i] = events[i].sample_x11;
                    sample_x12[i] = events[i].sample_x12;
                    sample_x13[i] = events[i].sample_x13;
                    sample_x14[i] = events[i].sample_x14;
                    sample_x15[i] = events[i].sample_x15;
                    sample_x16[i] = events[i].sample_x16;
                    sample_x17[i] = events[i].sample_x17;
                    sample_x18[i] = events[i].sample_x18;
                    sample_x19[i] = events[i].sample_x19;
                    sample_x20[i] = events[i].sample_x20;
                    sample_y1[i]  = events[i].sample_y1;
                    sample_y2[i]  = events[i].sample_y2;
                    sample_y3[i]  = events[i].sample_y3;
                    sample_y4[i]  = events[i].sample_y4;
                    sample_y5[i]  = events[i].sample_y5;
                    sample_y6[i]  = events[i].sample_y6;
                    sample_y7[i]  = events[i].sample_y7;
                    sample_y8[i]  = events[i].sample_y8;
                    sample_y9[i]  = events[i].sample_y9;
                    sample_y10[i] = events[i].sample_y10;
                    sample_y11[i] = events[i].sample_y11;
                    sample_y12[i] = events[i].sample_y12;
                    sample_y13[i] = events[i].sample_y13;
                    sample_y14[i] = events[i].sample_y14;
                    sample_y15[i] = events[i].sample_y15;
                    sample_y16[i] = events[i].sample_y16;
                    sample_y17[i] = events[i].sample_y17;
                }
            } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_BNL_DIAG) {
                const Event::BNL::Diag *events = packet->getEvents<Event::BNL::Diag>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]         = events[i].tof;
                    positions[i]    = events[i].position;
                    pixels[i]       = events[i].pixelid;
                    mapped_pixels[i]= events[i].mapped_pixelid;
                    xs[i]           = 1.0 * events[i].x / (1 << 11);
                    ys[i]           = 1.0 * events[i].y / (1 << 11);
                    corrected_xs[i] = 1.0 * events[i].corrected_x;
                    corrected_ys[i] = 1.0 * events[i].corrected_y;
                    sample_x1[i]    = events[i].sample_x1;
                    sample_x2[i]    = events[i].sample_x2;
                    sample_x3[i]    = events[i].sample_x3;
                    sample_x4[i]    = events[i].sample_x4;
                    sample_x5[i]    = events[i].sample_x5;
                    sample_x6[i]    = events[i].sample_x6;
                    sample_x7[i]    = events[i].sample_x7;
                    sample_x8[i]    = events[i].sample_x8;
                    sample_x9[i]    = events[i].sample_x9;
                    sample_x10[i]   = events[i].sample_x10;
                    sample_x11[i]   = events[i].sample_x11;
                    sample_x12[i]   = events[i].sample_x12;
                    sample_x13[i]   = events[i].sample_x13;
                    sample_x14[i]   = events[i].sample_x14;
                    sample_x15[i]   = events[i].sample_x15;
                    sample_x16[i]   = events[i].sample_x16;
                    sample_x17[i]   = events[i].sample_x17;
                    sample_x18[i]   = events[i].sample_x18;
                    sample_x19[i]   = events[i].sample_x19;
                    sample_x20[i]   = events[i].sample_x20;
                    sample_y1[i]    = events[i].sample_y1;
                    sample_y2[i]    = events[i].sample_y2;
                    sample_y3[i]    = events[i].sample_y3;
                    sample_y4[i]    = events[i].sample_y4;
                    sample_y5[i]    = events[i].sample_y5;
                    sample_y6[i]    = events[i].sample_y6;
                    sample_y7[i]    = events[i].sample_y7;
                    sample_y8[i]    = events[i].sample_y8;
                    sample_y9[i]    = events[i].sample_y9;
                    sample_y10[i]   = events[i].sample_y10;
                    sample_y11[i]   = events[i].sample_y11;
                    sample_y12[i]   = events[i].sample_y12;
                    sample_y13[i]   = events[i].sample_y13;
                    sample_y14[i]   = events[i].sample_y14;
                    sample_y15[i]   = events[i].sample_y15;
                    sample_y16[i]   = events[i].sample_y16;
                    sample_y17[i]   = events[i].sample_y17;
                }
            } else {
                nEvents = 0;
                return false;
            }

            lock();
            try {
                beginGroupPut();

                pvTimeStamp.set(timestamp);
                pvNumEvents->put(nEvents);
                pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
                pvPosition->replace(epics::pvData::freeze(positions));
                pvPixel->replace(epics::pvData::freeze(pixels));
                pvMappedPixel->replace(epics::pvData::freeze(mapped_pixels));
                pvX->replace(epics::pvData::freeze(xs));
                pvY->replace(epics::pvData::freeze(ys));
                pvCorrectedX->replace(epics::pvData::freeze(corrected_xs));
                pvCorrectedY->replace(epics::pvData::freeze(corrected_ys));
                pvSampleX1->replace(epics::pvData::freeze(sample_x1));
                pvSampleX2->replace(epics::pvData::freeze(sample_x2));
                pvSampleX3->replace(epics::pvData::freeze(sample_x3));
                pvSampleX4->replace(epics::pvData::freeze(sample_x4));
                pvSampleX5->replace(epics::pvData::freeze(sample_x5));
                pvSampleX6->replace(epics::pvData::freeze(sample_x6));
                pvSampleX7->replace(epics::pvData::freeze(sample_x7));
                pvSampleX8->replace(epics::pvData::freeze(sample_x8));
                pvSampleX9->replace(epics::pvData::freeze(sample_x9));
                pvSampleX10->replace(epics::pvData::freeze(sample_x10));
                pvSampleX11->replace(epics::pvData::freeze(sample_x11));
                pvSampleX12->replace(epics::pvData::freeze(sample_x12));
                pvSampleX13->replace(epics::pvData::freeze(sample_x13));
                pvSampleX14->replace(epics::pvData::freeze(sample_x14));
                pvSampleX15->replace(epics::pvData::freeze(sample_x15));
                pvSampleX16->replace(epics::pvData::freeze(sample_x16));
                pvSampleX17->replace(epics::pvData::freeze(sample_x17));
                pvSampleX18->replace(epics::pvData::freeze(sample_x18));
                pvSampleX19->replace(epics::pvData::freeze(sample_x19));
                pvSampleX20->replace(epics::pvData::freeze(sample_x20));
                pvSampleY1->replace(epics::pvData::freeze(sample_y1));
                pvSampleY2->replace(epics::pvData::freeze(sample_y2));
                pvSampleY3->replace(epics::pvData::freeze(sample_y3));
                pvSampleY4->replace(epics::pvData::freeze(sample_y4));
                pvSampleY5->replace(epics::pvData::freeze(sample_y5));
                pvSampleY6->replace(epics::pvData::freeze(sample_y6));
                pvSampleY7->replace(epics::pvData::freeze(sample_y7));
                pvSampleY8->replace(epics::pvData::freeze(sample_y8));
                pvSampleY9->replace(epics::pvData::freeze(sample_y9));
                pvSampleY10->replace(epics::pvData::freeze(sample_y10));
                pvSampleY11->replace(epics::pvData::freeze(sample_y11));
                pvSampleY12->replace(epics::pvData::freeze(sample_y12));
                pvSampleY13->replace(epics::pvData::freeze(sample_y13));
                pvSampleY14->replace(epics::pvData::freeze(sample_y14));
                pvSampleY15->replace(epics::pvData::freeze(sample_y15));
                pvSampleY16->replace(epics::pvData::freeze(sample_y16));
                pvSampleY17->replace(epics::pvData::freeze(sample_y17));

                endGroupPut();
                posted = true;
            } catch (...) {
            }
            unlock();

            return posted;
        }

    private:
        uint32_t m_sequence;
        epics::pvData::PVTimeStamp      pvTimeStamp;     //!< Time stamp common to all events
        epics::pvData::PVUIntPtr        pvNumEvents;     //!< Number of events in all arrays
        epics::pvData::PVUIntArrayPtr   pvTimeOfFlight;  //!< Time offset relative to time stamp
        epics::pvData::PVUIntArrayPtr   pvPosition;      //!< Calculated pixel ID
        epics::pvData::PVUIntArrayPtr   pvPixel;         //!< Calculated pixel ID
        epics::pvData::PVUIntArrayPtr   pvMappedPixel;   //!< Calculated pixel ID
        epics::pvData::PVFloatArrayPtr  pvX;             //!< Position X
        epics::pvData::PVFloatArrayPtr  pvY;             //!< Position Y
        epics::pvData::PVFloatArrayPtr  pvCorrectedX;    //!< Corrected position X
        epics::pvData::PVFloatArrayPtr  pvCorrectedY;    //!< Corrected position Y

        epics::pvData::PVUShortArrayPtr pvSampleX1;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX2;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX3;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX4;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX5;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX6;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX7;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX8;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX9;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX10;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX11;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX12;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX13;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX14;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX15;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX16;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX17;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX18;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX19;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleX20;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY1;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY2;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY3;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY4;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY5;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY6;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY7;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY8;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY9;      //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY10;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY11;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY12;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY13;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY14;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY15;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY16;     //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvSampleY17;     //!< Raw sample data
};

/**
 * PVRecord for CROC channel.
 */
class PvaNeutronsPlugin::PvaRecordCroc : public epics::pvDatabase::PVRecord {
    private:
        /**
         * C'tor.
         */
        PvaRecordCroc(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure)
            , m_sequence(0)
        {}

    public:
        POINTER_DEFINITIONS(PvaRecordCroc);

        /**
         * Allocate and initialize PvaRecordCroc.
         */
        static PvaNeutronsPlugin::PvaRecordCroc::shared_pointer create(const std::string &recordName) {
            using namespace epics::pvData;

            StandardFieldPtr standardField = getStandardField();
            FieldCreatePtr fieldCreate     = getFieldCreate();
            PVDataCreatePtr pvDataCreate   = getPVDataCreate();

            PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
                fieldCreate->createFieldBuilder()->
                    add("timeStamp",        standardField->timeStamp())->
                    add("num_events",       standardField->scalar(pvUInt, ""))->
                    add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("position",         standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("x",                standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("y",                standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("corrected_x",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("corrected_y",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("pixel",            standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("mapped_pixel",     standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("time_range1",      standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("time_range2",      standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("time_range3",      standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("time_range4",      standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y1",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y2",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y3",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y4",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y5",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y6",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_y7",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x1",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x2",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x3",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x4",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x5",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x6",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x7",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x8",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x9",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x10", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_x11", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g1",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g2",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g3",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g4",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g5",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g6",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g7",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g8",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g9",  standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g10", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g11", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g12", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g13", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    add("photon_count_g14", standardField->scalarArray(epics::pvData::pvUShort, ""))->
                    createStructure()
            );

            PvaRecordCroc::shared_pointer pvRecord(new PvaRecordCroc(recordName, pvStructure));
            if (!pvRecord)
                return PvaRecordCroc::shared_pointer();

            pvRecord->initPVRecord();

            if (!pvRecord->pvTimeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp")))
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvNumEvents = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
            if (pvRecord->pvNumEvents.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvTimeOfFlight = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
            if (pvRecord->pvTimeOfFlight.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPosition = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position.value");
            if (pvRecord->pvPosition.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
            if (pvRecord->pvPixel.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvMappedPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("mapped_pixel.value");
            if (pvRecord->pvMappedPixel.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("x.value");
            if (pvRecord->pvX.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("y.value");
            if (pvRecord->pvY.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvCorrectedX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("corrected_x.value");
            if (pvRecord->pvCorrectedX.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvCorrectedY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("corrected_y.value");
            if (pvRecord->pvCorrectedY.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvTimeRange1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range1.value");
            if (pvRecord->pvTimeRange1.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvTimeRange2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range2.value");
            if (pvRecord->pvTimeRange2.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvTimeRange3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range3.value");
            if (pvRecord->pvTimeRange3.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvTimeRange4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("time_range4.value");
            if (pvRecord->pvTimeRange4.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y1.value");
            if (pvRecord->pvPhotonCountY1.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y2.value");
            if (pvRecord->pvPhotonCountY2.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y3.value");
            if (pvRecord->pvPhotonCountY3.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y4.value");
            if (pvRecord->pvPhotonCountY4.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y5.value");
            if (pvRecord->pvPhotonCountY5.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y6.value");
            if (pvRecord->pvPhotonCountY6.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountY7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_y7.value");
            if (pvRecord->pvPhotonCountY7.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x1.value");
            if (pvRecord->pvPhotonCountX1.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x2.value");
            if (pvRecord->pvPhotonCountX2.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x3.value");
            if (pvRecord->pvPhotonCountX3.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x4.value");
            if (pvRecord->pvPhotonCountX4.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x5.value");
            if (pvRecord->pvPhotonCountX5.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x6.value");
            if (pvRecord->pvPhotonCountX6.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x7.value");
            if (pvRecord->pvPhotonCountX7.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX8 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x8.value");
            if (pvRecord->pvPhotonCountX8.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX9 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x9.value");
            if (pvRecord->pvPhotonCountX9.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX10 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x10.value");
            if (pvRecord->pvPhotonCountX10.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountX11 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_x11.value");
            if (pvRecord->pvPhotonCountX11.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g1.value");
            if (pvRecord->pvPhotonCountG1.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g2.value");
            if (pvRecord->pvPhotonCountG2.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g3.value");
            if (pvRecord->pvPhotonCountG3.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g4.value");
            if (pvRecord->pvPhotonCountG4.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g5.value");
            if (pvRecord->pvPhotonCountG5.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g6.value");
            if (pvRecord->pvPhotonCountG6.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g7.value");
            if (pvRecord->pvPhotonCountG7.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG8 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g8.value");
            if (pvRecord->pvPhotonCountG8.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG9 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g9.value");
            if (pvRecord->pvPhotonCountG9.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG10 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g10.value");
            if (pvRecord->pvPhotonCountG10.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG11 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g11.value");
            if (pvRecord->pvPhotonCountG11.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG12 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g12.value");
            if (pvRecord->pvPhotonCountG12.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG13 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g13.value");
            if (pvRecord->pvPhotonCountG13.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            pvRecord->pvPhotonCountG14 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUShortArray>("photon_count_g14.value");
            if (pvRecord->pvPhotonCountG14.get() == NULL)
                return PvaRecordCroc::shared_pointer();

            return pvRecord;

        }

        /**
         * Publish a single atomic update of the PV, take values from packet.
         */
        bool update(const DasDataPacket *packet, uint32_t &nEvents) {
            bool posted = false;
            epicsTimeStamp ts = packet->getTimeStamp();

            // 31 bit sequence number is good for around 9 months.
            // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
            // In worst case client will skip one packet on rollover and then recover
            // the sequence.
            epics::pvData::TimeStamp timestamp(
                epics::pvData::posixEpochAtEpicsEpoch + ts.secPastEpoch,
                ts.nsec,
                m_sequence++ % 0x7FFFFFFF
            );

            nEvents = packet->getNumEvents();

            // Pre-allocate svector to minimize gradual memory allocations
            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            epics::pvData::PVUIntArray::svector mapped_pixels(nEvents);
            epics::pvData::PVUIntArray::svector positions(nEvents);
            epics::pvData::PVFloatArray::svector xs(nEvents);
            epics::pvData::PVFloatArray::svector ys(nEvents);
            epics::pvData::PVFloatArray::svector corrected_xs(nEvents);
            epics::pvData::PVFloatArray::svector corrected_ys(nEvents);
            epics::pvData::PVUShortArray::svector time_range1(nEvents);
            epics::pvData::PVUShortArray::svector time_range2(nEvents);
            epics::pvData::PVUShortArray::svector time_range3(nEvents);
            epics::pvData::PVUShortArray::svector time_range4(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y1(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y2(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y3(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y4(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y5(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y6(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_y7(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x1(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x2(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x3(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x4(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x5(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x6(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x7(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x8(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x9(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x10(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_x11(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g1(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g2(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g3(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g4(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g5(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g6(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g7(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g8(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g9(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g10(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g11(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g12(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g13(nEvents);
            epics::pvData::PVUShortArray::svector photon_count_g14(nEvents);


            if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_CROC_RAW) {
                const Event::CROC::Raw *events = packet->getEvents<Event::CROC::Raw>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]   = events[i].tof & 0x000FFFFF;
                    positions[i] = events[i].position;
                    pixels[i] = 0;
                    mapped_pixels[i] = 0;
                    xs[i] = -1;
                    ys[i] = -1;
                    corrected_xs[i] = -1;
                    corrected_ys[i] = -1;
                    time_range1[i]      = events[i].time_range[0];
                    time_range2[i]      = events[i].time_range[1];
                    time_range3[i]      = events[i].time_range[2];
                    time_range4[i]      = events[i].time_range[3];
                    photon_count_y1[i]  = events[i].photon_count_y[0];
                    photon_count_y2[i]  = events[i].photon_count_y[1];
                    photon_count_y3[i]  = events[i].photon_count_y[2];
                    photon_count_y4[i]  = events[i].photon_count_y[3];
                    photon_count_y5[i]  = events[i].photon_count_y[4];
                    photon_count_y6[i]  = events[i].photon_count_y[5];
                    photon_count_y7[i]  = events[i].photon_count_y[6];
                    photon_count_x1[i]  = events[i].photon_count_x[0];
                    photon_count_x2[i]  = events[i].photon_count_x[1];
                    photon_count_x3[i]  = events[i].photon_count_x[2];
                    photon_count_x4[i]  = events[i].photon_count_x[3];
                    photon_count_x5[i]  = events[i].photon_count_x[4];
                    photon_count_x6[i]  = events[i].photon_count_x[5];
                    photon_count_x7[i]  = events[i].photon_count_x[6];
                    photon_count_x8[i]  = events[i].photon_count_x[7];
                    photon_count_x9[i]  = events[i].photon_count_x[8];
                    photon_count_x10[i] = events[i].photon_count_x[9];
                    photon_count_x11[i] = events[i].photon_count_x[10];
                    photon_count_g1[i]  = events[i].photon_count_g[0];
                    photon_count_g2[i]  = events[i].photon_count_g[1];
                    photon_count_g3[i]  = events[i].photon_count_g[2];
                    photon_count_g4[i]  = events[i].photon_count_g[3];
                    photon_count_g5[i]  = events[i].photon_count_g[4];
                    photon_count_g6[i]  = events[i].photon_count_g[5];
                    photon_count_g7[i]  = events[i].photon_count_g[6];
                    photon_count_g8[i]  = events[i].photon_count_g[7];
                    photon_count_g9[i]  = events[i].photon_count_g[8];
                    photon_count_g10[i] = events[i].photon_count_g[9];
                    photon_count_g11[i] = events[i].photon_count_g[10];
                    photon_count_g12[i] = events[i].photon_count_g[11];
                    photon_count_g13[i] = events[i].photon_count_g[12];
                    photon_count_g14[i] = events[i].photon_count_g[13];
                }
            } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_CROC_DIAG) {
                const Event::CROC::Diag *events = packet->getEvents<Event::CROC::Diag>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]         = events[i].tof & 0x000FFFFF;
                    positions[i]    = events[i].position;
                    pixels[i]       = events[i].pixelid;
                    mapped_pixels[i]= events[i].mapped_pixelid;
                    xs[i]           = 1.0 * events[i].x / (1 << 11);
                    ys[i]           = 1.0 * events[i].y / (1 << 11);
                    corrected_xs[i] = 1.0 * events[i].corrected_x;
                    corrected_ys[i] = 1.0 * events[i].corrected_y;
                    time_range1[i]      = events[i].time_range[0];
                    time_range2[i]      = events[i].time_range[1];
                    time_range3[i]      = events[i].time_range[2];
                    time_range4[i]      = events[i].time_range[3];
                    photon_count_y1[i]  = events[i].photon_count_y[0];
                    photon_count_y2[i]  = events[i].photon_count_y[1];
                    photon_count_y3[i]  = events[i].photon_count_y[2];
                    photon_count_y4[i]  = events[i].photon_count_y[3];
                    photon_count_y5[i]  = events[i].photon_count_y[4];
                    photon_count_y6[i]  = events[i].photon_count_y[5];
                    photon_count_y7[i]  = events[i].photon_count_y[6];
                    photon_count_x1[i]  = events[i].photon_count_x[0];
                    photon_count_x2[i]  = events[i].photon_count_x[1];
                    photon_count_x3[i]  = events[i].photon_count_x[2];
                    photon_count_x4[i]  = events[i].photon_count_x[3];
                    photon_count_x5[i]  = events[i].photon_count_x[4];
                    photon_count_x6[i]  = events[i].photon_count_x[5];
                    photon_count_x7[i]  = events[i].photon_count_x[6];
                    photon_count_x8[i]  = events[i].photon_count_x[7];
                    photon_count_x9[i]  = events[i].photon_count_x[8];
                    photon_count_x10[i] = events[i].photon_count_x[9];
                    photon_count_x11[i] = events[i].photon_count_x[10];
                    photon_count_g1[i]  = events[i].photon_count_g[0];
                    photon_count_g2[i]  = events[i].photon_count_g[1];
                    photon_count_g3[i]  = events[i].photon_count_g[2];
                    photon_count_g4[i]  = events[i].photon_count_g[3];
                    photon_count_g5[i]  = events[i].photon_count_g[4];
                    photon_count_g6[i]  = events[i].photon_count_g[5];
                    photon_count_g7[i]  = events[i].photon_count_g[6];
                    photon_count_g8[i]  = events[i].photon_count_g[7];
                    photon_count_g9[i]  = events[i].photon_count_g[8];
                    photon_count_g10[i] = events[i].photon_count_g[9];
                    photon_count_g11[i] = events[i].photon_count_g[10];
                    photon_count_g12[i] = events[i].photon_count_g[11];
                    photon_count_g13[i] = events[i].photon_count_g[12];
                    photon_count_g14[i] = events[i].photon_count_g[13];
                }
            } else {
                nEvents = 0;
                return false;
            }

            lock();
            try {
                beginGroupPut();

                pvTimeStamp.set(timestamp);
                pvNumEvents->put(nEvents);
                pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
                pvPosition->replace(epics::pvData::freeze(positions));
                pvPixel->replace(epics::pvData::freeze(pixels));
                pvMappedPixel->replace(epics::pvData::freeze(mapped_pixels));
                pvX->replace(epics::pvData::freeze(xs));
                pvY->replace(epics::pvData::freeze(ys));
                pvCorrectedX->replace(epics::pvData::freeze(corrected_xs));
                pvCorrectedY->replace(epics::pvData::freeze(corrected_ys));
                pvTimeRange1->replace(epics::pvData::freeze(time_range1));
                pvTimeRange2->replace(epics::pvData::freeze(time_range2));
                pvTimeRange3->replace(epics::pvData::freeze(time_range3));
                pvTimeRange4->replace(epics::pvData::freeze(time_range4));
                pvPhotonCountY1->replace(epics::pvData::freeze(photon_count_y1));
                pvPhotonCountY2->replace(epics::pvData::freeze(photon_count_y2));
                pvPhotonCountY3->replace(epics::pvData::freeze(photon_count_y3));
                pvPhotonCountY4->replace(epics::pvData::freeze(photon_count_y4));
                pvPhotonCountY5->replace(epics::pvData::freeze(photon_count_y5));
                pvPhotonCountY6->replace(epics::pvData::freeze(photon_count_y6));
                pvPhotonCountY7->replace(epics::pvData::freeze(photon_count_y7));
                pvPhotonCountX1->replace(epics::pvData::freeze(photon_count_x1));
                pvPhotonCountX2->replace(epics::pvData::freeze(photon_count_x2));
                pvPhotonCountX3->replace(epics::pvData::freeze(photon_count_x3));
                pvPhotonCountX4->replace(epics::pvData::freeze(photon_count_x4));
                pvPhotonCountX5->replace(epics::pvData::freeze(photon_count_x5));
                pvPhotonCountX6->replace(epics::pvData::freeze(photon_count_x6));
                pvPhotonCountX7->replace(epics::pvData::freeze(photon_count_x7));
                pvPhotonCountX8->replace(epics::pvData::freeze(photon_count_x8));
                pvPhotonCountX9->replace(epics::pvData::freeze(photon_count_x9));
                pvPhotonCountX10->replace(epics::pvData::freeze(photon_count_x10));
                pvPhotonCountX11->replace(epics::pvData::freeze(photon_count_x11));
                pvPhotonCountG1->replace(epics::pvData::freeze(photon_count_g1));
                pvPhotonCountG2->replace(epics::pvData::freeze(photon_count_g2));
                pvPhotonCountG3->replace(epics::pvData::freeze(photon_count_g3));
                pvPhotonCountG4->replace(epics::pvData::freeze(photon_count_g4));
                pvPhotonCountG5->replace(epics::pvData::freeze(photon_count_g5));
                pvPhotonCountG6->replace(epics::pvData::freeze(photon_count_g6));
                pvPhotonCountG7->replace(epics::pvData::freeze(photon_count_g7));
                pvPhotonCountG8->replace(epics::pvData::freeze(photon_count_g8));
                pvPhotonCountG9->replace(epics::pvData::freeze(photon_count_g9));
                pvPhotonCountG10->replace(epics::pvData::freeze(photon_count_g10));
                pvPhotonCountG11->replace(epics::pvData::freeze(photon_count_g11));
                pvPhotonCountG12->replace(epics::pvData::freeze(photon_count_g12));
                pvPhotonCountG13->replace(epics::pvData::freeze(photon_count_g13));
                pvPhotonCountG14->replace(epics::pvData::freeze(photon_count_g14));
                endGroupPut();
                posted = true;
            } catch (...) {
            }
            unlock();

            return posted;
        }

    private:
        uint32_t m_sequence;
        epics::pvData::PVTimeStamp      pvTimeStamp;     //!< Time stamp common to all events
        epics::pvData::PVUIntPtr        pvNumEvents;     //!< Number of events in all arrays
        epics::pvData::PVUIntArrayPtr   pvTimeOfFlight;  //!< Time offset relative to time stamp
        epics::pvData::PVUIntArrayPtr   pvPosition;      //!< Calculated pixel ID
        epics::pvData::PVUIntArrayPtr   pvPixel;         //!< Calculated pixel ID
        epics::pvData::PVUIntArrayPtr   pvMappedPixel;   //!< Calculated pixel ID
        epics::pvData::PVFloatArrayPtr  pvX;             //!< Position X
        epics::pvData::PVFloatArrayPtr  pvY;             //!< Position Y
        epics::pvData::PVFloatArrayPtr  pvCorrectedX;    //!< Corrected position X
        epics::pvData::PVFloatArrayPtr  pvCorrectedY;    //!< Corrected position Y

        epics::pvData::PVUShortArrayPtr pvTimeRange1;    //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvTimeRange2;    //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvTimeRange3;    //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvTimeRange4;    //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY1; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY2; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY3; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY4; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY5; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY6; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountY7; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX1; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX2; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX3; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX4; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX5; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX6; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX7; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX8; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX9; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX10;//!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountX11;//!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG1; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG2; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG3; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG4; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG5; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG6; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG7; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG8; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG9; //!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG10;//!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG11;//!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG12;//!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG13;//!< Raw sample data
        epics::pvData::PVUShortArrayPtr pvPhotonCountG14;//!< Raw sample data
};

/**
 * PVRecord for CROC channel.
 */
class PvaNeutronsPlugin::PvaRecordAcpc : public epics::pvDatabase::PVRecord {
    private:
        /**
         * C'tor.
         */
        PvaRecordAcpc(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure)
            , m_sequence(0)
        {}

    public:
        POINTER_DEFINITIONS(PvaRecordAcpc);

        /**
         * Allocate and initialize PvaRecordAcpc.
         */
        static PvaNeutronsPlugin::PvaRecordAcpc::shared_pointer create(const std::string &recordName) {
            using namespace epics::pvData;

            StandardFieldPtr standardField = getStandardField();
            FieldCreatePtr fieldCreate     = getFieldCreate();
            PVDataCreatePtr pvDataCreate   = getPVDataCreate();

            PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
                fieldCreate->createFieldBuilder()->
                    add("timeStamp",        standardField->timeStamp())->
                    add("num_events",       standardField->scalar(pvUInt, ""))->
                    add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("position",         standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("x",                standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("y",                standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("corrected_x",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("corrected_y",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("photo_sum_x",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("photo_sum_y",      standardField->scalarArray(epics::pvData::pvFloat, ""))->
                    add("pixel",            standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("mapped_pixel",     standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    createStructure()
            );

            PvaRecordAcpc::shared_pointer pvRecord(new PvaRecordAcpc(recordName, pvStructure));
            if (!pvRecord)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->initPVRecord();

            if (!pvRecord->pvTimeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp")))
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvNumEvents = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
            if (pvRecord->pvNumEvents.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvTimeOfFlight = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
            if (pvRecord->pvTimeOfFlight.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvPosition = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position.value");
            if (pvRecord->pvPosition.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("pixel.value");
            if (pvRecord->pvPixel.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvMappedPixel = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("mapped_pixel.value");
            if (pvRecord->pvMappedPixel.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("x.value");
            if (pvRecord->pvX.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("y.value");
            if (pvRecord->pvY.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvCorrectedX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("corrected_x.value");
            if (pvRecord->pvCorrectedX.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvCorrectedY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("corrected_y.value");
            if (pvRecord->pvCorrectedY.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvPhotoSumX = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("photo_sum_x.value");
            if (pvRecord->pvPhotoSumX.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            pvRecord->pvPhotoSumY = pvRecord->getPVStructure()->getSubField<epics::pvData::PVFloatArray>("photo_sum_y.value");
            if (pvRecord->pvPhotoSumY.get() == NULL)
                return PvaRecordAcpc::shared_pointer();

            return pvRecord;

        }

        /**
         * Publish a single atomic update of the PV, take values from packet.
         */
        bool update(const DasDataPacket *packet, uint32_t &nEvents) {
            bool posted = false;
            epicsTimeStamp ts = packet->getTimeStamp();

            // 31 bit sequence number is good for around 9 months.
            // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
            // In worst case client will skip one packet on rollover and then recover
            // the sequence.
            epics::pvData::TimeStamp timestamp(
                epics::pvData::posixEpochAtEpicsEpoch + ts.secPastEpoch,
                ts.nsec,
                m_sequence++ % 0x7FFFFFFF
            );

            nEvents = packet->getNumEvents();

            // Pre-allocate svector to minimize gradual memory allocations
            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector pixels(nEvents);
            epics::pvData::PVUIntArray::svector mapped_pixels(nEvents);
            epics::pvData::PVUIntArray::svector positions(nEvents);
            epics::pvData::PVFloatArray::svector xs(nEvents);
            epics::pvData::PVFloatArray::svector ys(nEvents);
            epics::pvData::PVFloatArray::svector corrected_xs(nEvents);
            epics::pvData::PVFloatArray::svector corrected_ys(nEvents);
            epics::pvData::PVFloatArray::svector photo_sum_x(nEvents);
            epics::pvData::PVFloatArray::svector photo_sum_y(nEvents);

            if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_ACPC_XY_PS) {
                const Event::ACPC::Normal *events = packet->getEvents<Event::ACPC::Normal>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]         = events[i].tof & 0x000FFFFF;
                    positions[i]    = events[i].position & 0x7FFFFFFF;
                    pixels[i]       = 0;
                    mapped_pixels[i]= 0;
                    xs[i]           = 1.0 * events[i].x / (1 << 24);
                    ys[i]           = 1.0 * events[i].y / (1 << 24);
                    corrected_xs[i] = -1;
                    corrected_ys[i] = -1;
                    photo_sum_x[i]  = events[i].photo_sum_x;
                    photo_sum_y[i]  = events[i].photo_sum_y;
                }
            } else if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_ACPC_DIAG) {
                const Event::ACPC::Diag *events = packet->getEvents<Event::ACPC::Diag>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]         = events[i].tof & 0x000FFFFF;
                    positions[i]    = events[i].position & 0x7FFFFFFF;
                    pixels[i]       = events[i].pixelid;
                    mapped_pixels[i]= events[i].mapped_pixelid;
                    xs[i]           = 1.0 * events[i].x / (1 << 24);
                    ys[i]           = 1.0 * events[i].y / (1 << 24);
                    corrected_xs[i] = 1.0 * events[i].corrected_x;
                    corrected_ys[i] = 1.0 * events[i].corrected_y;
                    photo_sum_x[i]  = 1.0 * events[i].photo_sum_x / (1 << 15);
                    photo_sum_y[i]  = 1.0 * events[i].photo_sum_y / (1 << 15);
                }
            } else {
                nEvents = 0;
                return false;
            }

            lock();
            try {
                beginGroupPut();

                pvTimeStamp.set(timestamp);
                pvNumEvents->put(nEvents);
                pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
                pvPosition->replace(epics::pvData::freeze(positions));
                pvPixel->replace(epics::pvData::freeze(pixels));
                pvMappedPixel->replace(epics::pvData::freeze(mapped_pixels));
                pvX->replace(epics::pvData::freeze(xs));
                pvY->replace(epics::pvData::freeze(ys));
                pvCorrectedX->replace(epics::pvData::freeze(corrected_xs));
                pvCorrectedY->replace(epics::pvData::freeze(corrected_ys));
                pvPhotoSumX->replace(epics::pvData::freeze(photo_sum_x));
                pvPhotoSumY->replace(epics::pvData::freeze(photo_sum_y));
                endGroupPut();
                posted = true;
            } catch (...) {
            }
            unlock();

            return posted;
        }

    private:
        uint32_t m_sequence;
        epics::pvData::PVTimeStamp      pvTimeStamp;     //!< Time stamp common to all events
        epics::pvData::PVUIntPtr        pvNumEvents;     //!< Number of events in all arrays
        epics::pvData::PVUIntArrayPtr   pvTimeOfFlight;  //!< Time offset relative to time stamp
        epics::pvData::PVUIntArrayPtr   pvPosition;      //!< Calculated pixel ID
        epics::pvData::PVUIntArrayPtr   pvPixel;         //!< Calculated pixel ID
        epics::pvData::PVUIntArrayPtr   pvMappedPixel;   //!< Calculated pixel ID
        epics::pvData::PVFloatArrayPtr  pvX;             //!< Position X
        epics::pvData::PVFloatArrayPtr  pvY;             //!< Position Y
        epics::pvData::PVFloatArrayPtr  pvCorrectedX;    //!< Corrected position X
        epics::pvData::PVFloatArrayPtr  pvCorrectedY;    //!< Corrected position Y
        epics::pvData::PVFloatArrayPtr  pvPhotoSumX;     //!< Photo sum X
        epics::pvData::PVFloatArrayPtr  pvPhotoSumY;     //!< Photo sum Y
};

/**
 * PVRecord for AROC channel.
 */
class PvaNeutronsPlugin::PvaRecordAroc : public epics::pvDatabase::PVRecord {
    private:
        /**
         * C'tor.
         */
        PvaRecordAroc(const std::string &recordName, const epics::pvData::PVStructurePtr &pvStructure)
            : epics::pvDatabase::PVRecord(recordName, pvStructure)
            , m_sequence(0)
        {}

    public:
        POINTER_DEFINITIONS(PvaRecordAroc);

        /**
         * Allocate and initialize PvaRecordAroc and attach to all fields.
         */
        static PvaRecordAroc::shared_pointer create(const std::string &recordName)
        {
            using namespace epics::pvData;

            StandardFieldPtr standardField = getStandardField();
            FieldCreatePtr fieldCreate     = getFieldCreate();
            PVDataCreatePtr pvDataCreate   = getPVDataCreate();

            PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
                fieldCreate->createFieldBuilder()->
                    add("timeStamp",        standardField->timeStamp())->
                    add("num_events",       standardField->scalar(pvUInt, ""))->
                    add("time_of_flight",   standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("position",         standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a1",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a2",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a3",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a4",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a5",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a6",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a7",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_a8",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b1",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b2",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b3",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b4",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b5",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b6",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b7",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    add("sample_b8",        standardField->scalarArray(epics::pvData::pvUInt, ""))->
                    createStructure()
            );

            PvaRecordAroc::shared_pointer pvRecord(new PvaRecordAroc(recordName, pvStructure));
            if (!pvRecord)
                return PvaRecordAroc::shared_pointer();

            pvRecord->initPVRecord();

            if (!pvRecord->pvTimeStamp.attach(pvRecord->getPVStructure()->getSubField("timeStamp")))
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvNumEvents = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUInt>("num_events.value");
            if (pvRecord->pvNumEvents.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvTimeOfFlight = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("time_of_flight.value");
            if (pvRecord->pvTimeOfFlight.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvPosition = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("position.value");
            if (pvRecord->pvPosition.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a1.value");
            if (pvRecord->pvSampleA1.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a2.value");
            if (pvRecord->pvSampleA2.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a3.value");
            if (pvRecord->pvSampleA3.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a4.value");
            if (pvRecord->pvSampleA4.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a5.value");
            if (pvRecord->pvSampleA5.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a6.value");
            if (pvRecord->pvSampleA6.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a7.value");
            if (pvRecord->pvSampleA7.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleA8 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_a8.value");
            if (pvRecord->pvSampleA8.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB1 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b1.value");
            if (pvRecord->pvSampleB1.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB2 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b2.value");
            if (pvRecord->pvSampleB2.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB3 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b3.value");
            if (pvRecord->pvSampleB3.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB4 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b4.value");
            if (pvRecord->pvSampleB4.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB5 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b5.value");
            if (pvRecord->pvSampleB5.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB6 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b6.value");
            if (pvRecord->pvSampleB6.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB7 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b7.value");
            if (pvRecord->pvSampleB7.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            pvRecord->pvSampleB8 = pvRecord->getPVStructure()->getSubField<epics::pvData::PVUIntArray>("sample_b8.value");
            if (pvRecord->pvSampleB8.get() == NULL)
                return PvaRecordAroc::shared_pointer();

            return pvRecord;
        }

        /**
         * Publish a single atomic update of the PV, take values from packet.
         */
        bool update(const DasDataPacket *packet, uint32_t &nEvents)
        {
            bool posted = false;

            // 31 bit sequence number is good for around 9 months.
            // (based on 5mio events/s, IRQ coallescing = 40, max OCC packet size = 3600B)
            // In worst case client will skip one packet on rollover and then recover
            // the sequence.
            epicsTimeStamp ts = packet->getTimeStamp();
            epics::pvData::TimeStamp timestamp(
                epics::pvData::posixEpochAtEpicsEpoch + ts.secPastEpoch,
                ts.nsec,
                m_sequence++ % 0x7FFFFFFF
            );

            nEvents = packet->getNumEvents();

            // Pre-allocate svector to minimize gradual memory allocations
            epics::pvData::PVUIntArray::svector tofs(nEvents);
            epics::pvData::PVUIntArray::svector positions(nEvents);
            epics::pvData::PVUIntArray::svector sample_a1(nEvents);
            epics::pvData::PVUIntArray::svector sample_a2(nEvents);
            epics::pvData::PVUIntArray::svector sample_a3(nEvents);
            epics::pvData::PVUIntArray::svector sample_a4(nEvents);
            epics::pvData::PVUIntArray::svector sample_a5(nEvents);
            epics::pvData::PVUIntArray::svector sample_a6(nEvents);
            epics::pvData::PVUIntArray::svector sample_a7(nEvents);
            epics::pvData::PVUIntArray::svector sample_a8(nEvents);
            epics::pvData::PVUIntArray::svector sample_b1(nEvents);
            epics::pvData::PVUIntArray::svector sample_b2(nEvents);
            epics::pvData::PVUIntArray::svector sample_b3(nEvents);
            epics::pvData::PVUIntArray::svector sample_b4(nEvents);
            epics::pvData::PVUIntArray::svector sample_b5(nEvents);
            epics::pvData::PVUIntArray::svector sample_b6(nEvents);
            epics::pvData::PVUIntArray::svector sample_b7(nEvents);
            epics::pvData::PVUIntArray::svector sample_b8(nEvents);

            if (packet->getEventsFormat() == DasDataPacket::EVENT_FMT_AROC_RAW) {
                const Event::AROC::Raw *events = packet->getEvents<Event::AROC::Raw>();
                for (uint32_t i = 0; i < nEvents; i++) {
                    tofs[i]   = events[i].tof & 0x000FFFFF;
                    positions[i] = events[i].position & 0x7FFFFFFF;
                    if (events[i].position & 0x10) {
                        sample_a1[i]  = events[i].sample_a[0];
                        sample_a2[i]  = events[i].sample_a[1];
                        sample_a3[i]  = events[i].sample_a[2];
                        sample_a4[i]  = events[i].sample_a[3];
                        sample_a5[i]  = events[i].sample_a[4];
                        sample_a6[i]  = events[i].sample_a[5];
                        sample_a7[i]  = events[i].sample_a[6];
                        sample_a8[i]  = events[i].sample_a[7];
                    } else {
                        sample_a1[i]  = events[i].sample_a[7];
                        sample_a2[i]  = events[i].sample_a[6];
                        sample_a3[i]  = events[i].sample_a[5];
                        sample_a4[i]  = events[i].sample_a[4];
                        sample_a5[i]  = events[i].sample_a[3];
                        sample_a6[i]  = events[i].sample_a[2];
                        sample_a7[i]  = events[i].sample_a[1];
                        sample_a8[i]  = events[i].sample_a[0];
                    }
                    if (events[i].position & 0x20) {
                        sample_b1[i]  = events[i].sample_b[0];
                        sample_b2[i]  = events[i].sample_b[1];
                        sample_b3[i]  = events[i].sample_b[2];
                        sample_b4[i]  = events[i].sample_b[3];
                        sample_b5[i]  = events[i].sample_b[4];
                        sample_b6[i]  = events[i].sample_b[5];
                        sample_b7[i]  = events[i].sample_b[6];
                        sample_b8[i]  = events[i].sample_b[7];
                    } else {
                        sample_b1[i]  = events[i].sample_b[7];
                        sample_b2[i]  = events[i].sample_b[6];
                        sample_b3[i]  = events[i].sample_b[5];
                        sample_b4[i]  = events[i].sample_b[4];
                        sample_b5[i]  = events[i].sample_b[3];
                        sample_b6[i]  = events[i].sample_b[2];
                        sample_b7[i]  = events[i].sample_b[1];
                        sample_b8[i]  = events[i].sample_b[0];
                    }
                }
            } else {
                nEvents = 0;
                return false;
            }

            lock();
            try {
                beginGroupPut();

                pvTimeStamp.set(timestamp);
                pvNumEvents->put(nEvents);
                pvTimeOfFlight->replace(epics::pvData::freeze(tofs));
                pvPosition->replace(epics::pvData::freeze(positions));
                pvSampleA1->replace(epics::pvData::freeze(sample_a1));
                pvSampleA2->replace(epics::pvData::freeze(sample_a2));
                pvSampleA3->replace(epics::pvData::freeze(sample_a3));
                pvSampleA4->replace(epics::pvData::freeze(sample_a4));
                pvSampleA5->replace(epics::pvData::freeze(sample_a5));
                pvSampleA6->replace(epics::pvData::freeze(sample_a6));
                pvSampleA7->replace(epics::pvData::freeze(sample_a7));
                pvSampleA8->replace(epics::pvData::freeze(sample_a8));
                pvSampleB1->replace(epics::pvData::freeze(sample_b1));
                pvSampleB2->replace(epics::pvData::freeze(sample_b2));
                pvSampleB3->replace(epics::pvData::freeze(sample_b3));
                pvSampleB4->replace(epics::pvData::freeze(sample_b4));
                pvSampleB5->replace(epics::pvData::freeze(sample_b5));
                pvSampleB6->replace(epics::pvData::freeze(sample_b6));
                pvSampleB7->replace(epics::pvData::freeze(sample_b7));
                pvSampleB8->replace(epics::pvData::freeze(sample_b8));

                endGroupPut();
                posted = true;
            } catch (...) {
            }
            unlock();

            return posted;
        }

    private:
        uint32_t m_sequence;
        epics::pvData::PVTimeStamp    pvTimeStamp;      //!< Time stamp common to all events
        epics::pvData::PVUIntPtr      pvNumEvents;      //!< Number of events in all arrays
        epics::pvData::PVUIntArrayPtr pvTimeOfFlight;   //!< Time offset relative to time stamp
        epics::pvData::PVUIntArrayPtr pvPosition;       //!< Calculated pixel ID

        epics::pvData::PVUIntArrayPtr pvSampleA1;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA2;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA3;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA4;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA5;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA6;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA7;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleA8;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB1;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB2;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB3;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB4;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB5;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB6;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB7;       //!< Raw sample data
        epics::pvData::PVUIntArrayPtr pvSampleB8;       //!< Raw sample data
};


EPICS_REGISTER_PLUGIN(PvaNeutronsPlugin, 4, "Port name", string, "Parent data plugins", string, "Parent RTDL plugins", string, "PVA records prefix", string);

PvaNeutronsPlugin::PvaNeutronsPlugin(const char *portName, const char *dataPlugins, const char *rtdlPlugins, const char *pvPrefix)
    : BasePlugin(portName, std::string(dataPlugins).find(',')!=std::string::npos, asynOctetMask, asynOctetMask)
{
    int status = STATUS_READY;

    if (pvPrefix == 0 || strlen(pvPrefix) == 0) {
        LOG_ERROR("Missing PVA record name");
        status = STATUS_BAD_CONFIG;
        pvPrefix = "";
    } else {
        std::string prefix(pvPrefix);

        // Regular pixel PVA record
        createParam("PixelPvaName", asynParamOctet, &PixelPvaName, prefix + "Neutrons");
        createParam("PixelNumEvents", asynParamInt32, &PixelNumEvents, 0);
        createParam("PixelEnable", asynParamInt32, &PixelEnable, 1);
        m_pixelRecord = PvaRecordPixel::create(prefix + "Neutrons");
        if (!m_pixelRecord) {
            LOG_ERROR("Failed to create pixel PVA record '%sNeutrons'", pvPrefix);
            setParamAlarmStatus(PixelPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(PixelPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_pixelRecord) == false) {
            LOG_ERROR("Failed to register pixel PVA record '%sNeutrons'", pvPrefix);
            setParamAlarmStatus(PixelPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(PixelPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }

        // Metadata PVA record
        createParam("MetaPvaName", asynParamOctet, &MetaPvaName, prefix + "Metadata");
        createParam("MetaNumEvents", asynParamInt32, &MetaNumEvents, 0);
        createParam("MetaEnable", asynParamInt32, &MetaEnable, 1);
        m_metaRecord = PvaRecordPixel::create(prefix + "Metadata");
        if (!m_metaRecord) {
            LOG_ERROR("Failed to create meta PVA record '%sMetadata'", pvPrefix);
            setParamAlarmStatus(MetaPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(MetaPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_metaRecord) == false) {
            LOG_ERROR("Failed to register meta PVA record '%sMetadata'", pvPrefix);
            setParamAlarmStatus(MetaPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(MetaPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }

        // LPSD ROC PVA record
        createParam("LpsdPvaName", asynParamOctet, &LpsdPvaName, prefix + "LPSD");
        createParam("LpsdNumEvents", asynParamInt32, &LpsdNumEvents, 0);
        createParam("LpsdEnable", asynParamInt32, &LpsdEnable, 1);
        m_lpsdRecord = PvaRecordLpsd::create(prefix + "LPSD");
        if (!m_lpsdRecord) {
            LOG_ERROR("Failed to create pixel PVA record '%sLPSD'", pvPrefix);
            setParamAlarmStatus(LpsdPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(LpsdPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_lpsdRecord) == false) {
            LOG_ERROR("Failed to register pixel PVA record '%sLPSD'", pvPrefix);
            setParamAlarmStatus(LpsdPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(LpsdPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }

        // BNL ROC PVA record
        createParam("BnlPvaName", asynParamOctet, &BnlPvaName, prefix + "BNL");
        createParam("BnlNumEvents", asynParamInt32, &BnlNumEvents, 0);
        createParam("BnlEnable", asynParamInt32, &BnlEnable, 0);
        m_bnlRecord = PvaRecordBnl::create(prefix + "BNL");
        if (!m_bnlRecord) {
            LOG_ERROR("Failed to create BNL PVA record '%sBNL'", pvPrefix);
            setParamAlarmStatus(BnlPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(BnlPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_bnlRecord) == false) {
            LOG_ERROR("Failed to register BNL PVA record '%sBNL'", pvPrefix);
            setParamAlarmStatus(BnlPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(BnlPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }

        // CROC PVA record
        createParam("CrocPvaName", asynParamOctet, &CrocPvaName, prefix + "CROC");
        createParam("CrocNumEvents", asynParamInt32, &CrocNumEvents, 0);
        createParam("CrocEnable", asynParamInt32, &CrocEnable, 0);
        m_crocRecord = PvaRecordCroc::create(prefix + "CROC");
        if (!m_crocRecord) {
            LOG_ERROR("Failed to create CROC PVA record '%sCROC'", pvPrefix);
            setParamAlarmStatus(CrocPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(CrocPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_crocRecord) == false) {
            LOG_ERROR("Failed to register CROC PVA record '%sCROC'", pvPrefix);
            setParamAlarmStatus(CrocPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(CrocPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }

        // ACPC PVA record
        createParam("AcpcPvaName", asynParamOctet, &AcpcPvaName, prefix + "ACPC");
        createParam("AcpcNumEvents", asynParamInt32, &AcpcNumEvents, 0);
        createParam("AcpcEnable", asynParamInt32, &AcpcEnable, 0);
        m_acpcRecord = PvaRecordAcpc::create(prefix + "ACPC");
        if (!m_acpcRecord) {
            LOG_ERROR("Failed to create ACPC PVA record '%sACPC'", pvPrefix);
            setParamAlarmStatus(AcpcPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(AcpcPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_acpcRecord) == false) {
            LOG_ERROR("Failed to register ACPC PVA record '%sACPC'", pvPrefix);
            setParamAlarmStatus(AcpcPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(AcpcPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }

        // AROC PVA record
        createParam("ArocPvaName", asynParamOctet, &ArocPvaName, prefix + "AROC");
        createParam("ArocNumEvents", asynParamInt32, &ArocNumEvents, 0);
        createParam("ArocEnable", asynParamInt32, &ArocEnable, 0);
        m_arocRecord = PvaRecordAroc::create(prefix + "AROC");
        if (!m_arocRecord) {
            LOG_ERROR("Failed to create AROC PVA record '%sAROC'", pvPrefix);
            setParamAlarmStatus(ArocPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(ArocPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        } else if (epics::pvDatabase::PVDatabase::getMaster()->addRecord(m_arocRecord) == false) {
            LOG_ERROR("Failed to register AROC PVA record '%sAROC'", pvPrefix);
            setParamAlarmStatus(ArocPvaName, epicsAlarmUDF);
            setParamAlarmSeverity(ArocPvaName, epicsSevMajor);
            status = STATUS_INIT_ERROR;
        }
    }

    createParam("Status", asynParamInt32, &Status, status);
    callParamCallbacks();

    BasePlugin::connect(dataPlugins, MsgDasData);
    BasePlugin::connect(rtdlPlugins, MsgDasRtdl);
}

void PvaNeutronsPlugin::recvDownstream(const DasDataPacketList &packets)
{
    bool acpcEn  = getBooleanParam(AcpcEnable);
    bool arocEn  = getBooleanParam(ArocEnable);
    bool bnlEn   = getBooleanParam(BnlEnable);
    bool lpsdEn  = getBooleanParam(LpsdEnable);
    bool pixelEn = getBooleanParam(PixelEnable);
    bool metaEn  = getBooleanParam(MetaEnable);

    bool acpcGood   = true;
    bool arocGood   = true;
    bool bnlGood    = true;
    bool lpsdGood   = true;
    bool pixelGood  = true;
    bool metaGood   = true;

    int nAcpcEvents  = -1;
    int nArocEvents  = -1;
    int nBnlEvents   = -1;
    int nLpsdEvents  = -1;
    int nPixelEvents = -1;
    int nMetaEvents  = -1;
    for (const auto &packet: packets) {
        uint32_t nEvents;
        double pCharge = getProtonCharge(packet->getTimeStamp());
        switch (packet->getEventsFormat()) {
            case DasDataPacket::EVENT_FMT_ACPC_DIAG:
                if (m_pixelRecord && pixelEn && pixelGood) {
                    pixelGood = m_pixelRecord->update(packet, nEvents, pCharge);
                    nPixelEvents = (nPixelEvents == -1 ? nEvents : nPixelEvents + nEvents);
                }
                // fall-thru
            case DasDataPacket::EVENT_FMT_ACPC_XY_PS:
                if (m_acpcRecord && acpcEn && acpcGood) {
                    acpcGood = m_acpcRecord->update(packet, nEvents);
                    nAcpcEvents = (nAcpcEvents == -1 ? nEvents : nAcpcEvents + nEvents);
                }
                break;
            case DasDataPacket::EVENT_FMT_AROC_RAW:
                if (m_arocRecord && arocEn && arocGood) {
                    arocGood = m_arocRecord->update(packet, nEvents);
                    nArocEvents = (nArocEvents == -1 ? nEvents : nArocEvents + nEvents);
                }
                break;
            case DasDataPacket::EVENT_FMT_BNL_VERBOSE:
            case DasDataPacket::EVENT_FMT_BNL_DIAG:
                if (m_pixelRecord && pixelEn && pixelGood) {
                    pixelGood = m_pixelRecord->update(packet, nEvents, pCharge);
                    nPixelEvents = (nPixelEvents == -1 ? nEvents : nPixelEvents + nEvents);
                }
                // fall-thru
            case DasDataPacket::EVENT_FMT_BNL_RAW:
                if (m_bnlRecord && bnlEn && bnlGood) {
                    bnlGood = m_bnlRecord->update(packet, nEvents);
                    nBnlEvents = (nBnlEvents == -1 ? nEvents : nBnlEvents + nEvents);
                }
                break;
            case DasDataPacket::EVENT_FMT_LPSD_VERBOSE:
            case DasDataPacket::EVENT_FMT_LPSD_DIAG:
                if (m_pixelRecord && pixelEn && pixelGood) {
                    pixelGood = m_pixelRecord->update(packet, nEvents, pCharge);
                    nPixelEvents = (nPixelEvents == -1 ? nEvents : nPixelEvents + nEvents);
                }
                // fall-thru
            case DasDataPacket::EVENT_FMT_LPSD_RAW:
                if (m_lpsdRecord && lpsdEn && lpsdGood) {
                    lpsdGood = m_lpsdRecord->update(packet, nEvents);
                    nLpsdEvents = (nLpsdEvents == -1 ? nEvents : nLpsdEvents + nEvents);
                }
                break;
            case DasDataPacket::EVENT_FMT_PIXEL:
                if (m_pixelRecord && pixelEn && pixelGood) {
                    pixelGood = m_pixelRecord->update(packet, nEvents, pCharge);
                    nPixelEvents = (nPixelEvents == -1 ? nEvents : nPixelEvents + nEvents);
                }
                break;
            case DasDataPacket::EVENT_FMT_META:
                if (m_metaRecord && metaEn && metaGood) {
                    metaGood = m_metaRecord->update(packet, nEvents, pCharge);
                    nMetaEvents = (nMetaEvents == -1 ? nEvents : nMetaEvents + nEvents);
                }
                break;
            case DasDataPacket::EVENT_FMT_TIME_CALIB:
                // Use this event type as a heartbeat signal used for proton
                // charge counting only. No events are pushed to pixel record.
                if (m_pixelRecord && pixelEn && pixelGood) {
                    pixelGood = m_pixelRecord->update(packet, nEvents, pCharge);
                    nPixelEvents = (nPixelEvents == -1 ? nEvents : nPixelEvents + nEvents);
                }
                break;
            default:
                LOG_WARN("Data type %u not supported", packet->getEventsFormat());
                break;
        }
    }

    if (nAcpcEvents >= 0) {
        addIntegerParam(AcpcNumEvents, nAcpcEvents);
        if (bnlGood) {
            setParamAlarmStatus(AcpcPvaName, epicsAlarmNone);
            setParamAlarmSeverity(AcpcPvaName, epicsSevNone);
        } else {
            LOG_ERROR("Failed to send PVA update on ACPC channel");
            setParamAlarmStatus(AcpcPvaName, epicsAlarmComm);
            setParamAlarmSeverity(AcpcPvaName, epicsSevMinor);
        }
    }

    if (nArocEvents >= 0) {
        addIntegerParam(ArocNumEvents, nArocEvents);
        if (bnlGood) {
            setParamAlarmStatus(ArocPvaName, epicsAlarmNone);
            setParamAlarmSeverity(ArocPvaName, epicsSevNone);
        } else {
            LOG_ERROR("Failed to send PVA update on AROC channel");
            setParamAlarmStatus(ArocPvaName, epicsAlarmComm);
            setParamAlarmSeverity(ArocPvaName, epicsSevMinor);
        }
    }

    if (nBnlEvents >= 0) {
        addIntegerParam(BnlNumEvents, nBnlEvents);
        if (bnlGood) {
            setParamAlarmStatus(BnlPvaName, epicsAlarmNone);
            setParamAlarmSeverity(BnlPvaName, epicsSevNone);
        } else {
            LOG_ERROR("Failed to send PVA update on BNL channel");
            setParamAlarmStatus(BnlPvaName, epicsAlarmComm);
            setParamAlarmSeverity(BnlPvaName, epicsSevMinor);
        }
    }

    if (nLpsdEvents >= 0) {
        addIntegerParam(LpsdNumEvents, nLpsdEvents);
        if (lpsdGood) {
            setParamAlarmStatus(LpsdPvaName, epicsAlarmNone);
            setParamAlarmSeverity(LpsdPvaName, epicsSevNone);
        } else {
            LOG_ERROR("Failed to send PVA update on LPSD channel");
            setParamAlarmStatus(LpsdPvaName, epicsAlarmComm);
            setParamAlarmSeverity(LpsdPvaName, epicsSevMinor);
        }
    }

    if (nPixelEvents >= 0) {
        addIntegerParam(PixelNumEvents, nPixelEvents);
        if (pixelGood) {
            setParamAlarmStatus(PixelPvaName, epicsAlarmNone);
            setParamAlarmSeverity(PixelPvaName, epicsSevNone);
        } else {
            LOG_ERROR("Failed to send PVA update on Neutrons channel");
            setParamAlarmStatus(PixelPvaName, epicsAlarmComm);
            setParamAlarmSeverity(PixelPvaName, epicsSevMinor);
        }
    }

    if (nMetaEvents >= 0) {
        addIntegerParam(MetaNumEvents, nMetaEvents);
        if (metaGood) {
            setParamAlarmStatus(MetaPvaName, epicsAlarmNone);
            setParamAlarmSeverity(MetaPvaName, epicsSevNone);
        } else {
            LOG_ERROR("Failed to send PVA update on Neutrons channel");
            setParamAlarmStatus(MetaPvaName, epicsAlarmComm);
            setParamAlarmSeverity(MetaPvaName, epicsSevMinor);
        }
    }

    callParamCallbacks();
}

void PvaNeutronsPlugin::recvDownstream(const RtdlPacketList &packets)
{
    for (auto &packet: packets) {
        bool found = false;
        for (auto it = m_pChargeFifo.begin(); it != m_pChargeFifo.end(); it++) {
            if (it->first == packet->getTimeStamp()) {
                found = true;
                break;
            }
        }
        if (!found) {
            m_pChargeFifo.push_front(std::make_pair(packet->getTimeStamp(), packet->getProtonCharge()));
            // Does queue size need to be configurable?
            if (m_pChargeFifo.size() > 20) {
                m_pChargeFifo.pop_back();
            }
        }
    }
}

double PvaNeutronsPlugin::getProtonCharge(const epicsTime &timestamp)
{
    for (auto it = m_pChargeFifo.begin(); it != m_pChargeFifo.end(); it++) {
        if (it->first == timestamp) {
            return it->second;
        }
    }
    return 0;
}
