/* CRocPosCalcPlugin.h
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef CROC_POS_CALC_PLUGIN_H
#define CROC_POS_CALC_PLUGIN_H

#include "BaseDispatcherPlugin.h"
#include "CRocDataPacket.h"

#include <epicsThread.h>

#include <limits>
#include <map>
#include <unordered_map>

/**
 * Analyze raw CROC data and convert it to tof,pixel format.
 *
 * When CROC is in raw data format, it performs no data validation on its own.
 * Software is responsible for validation and bad event rejection. This plugin
 * will rake raw CROC data, evaluate it and convert it to tof,pixel format.
 */
class CRocPosCalcPlugin : public BaseDispatcherPlugin {
    private: // definitions

        /**
         * Structure with CROC parameters used in calculation.
         *
         * Structure contains single CROC parameters. It gets populated when
         * CRocPlugin sends its parameters.
         */
        struct CRocParams {
            // Cached parameters values
            uint32_t position;
            uint16_t timeRange1;
            uint16_t timeRange2;
            uint16_t timeRange3;
            uint16_t timeRange4;

            uint32_t xMin;          //!< Min number of counts on each x sample
            uint32_t xCntMax;       //!< Max number of valid x samples
            uint32_t yMin;          //!< Min number of counts on each y sample
            uint32_t yCntMax;       //!< Max number of valid y samples

            uint16_t timeRangeMin;  //!< Min threshold for time range bins
            uint8_t timeRangeMinCnt;//!< Min number of valid time range bins

            uint32_t gMin;
            uint32_t gMin2;
            uint32_t gGapMin1;
            uint32_t gGapMin2;

            typedef enum {
                FIBER_CODING_V2,
                FIBER_CODING_V3,
            } FiberCoding;
            FiberCoding fiberCoding;

            uint8_t gWeights[14];       //!< G weights for noise calculation
            uint8_t xWeights[11];       //!< X weights for noise calculation
            uint8_t yWeights[7];        //!< Y weights for noise calculation
            uint32_t gNoiseThreshold;   //!< G noise threshold
            uint32_t xNoiseThreshold;   //!< X noise threshold
            uint32_t yNoiseThreshold;   //!< Y noise threshold

            // Run-time variables follow
            uint32_t lastTof;           //!< Last processed time of flight for this detector
            uint32_t lastPixelId;       //!< Last calculated pixel id
            uint64_t pulseId;           //!< Last good event pulse id
        };

        /**
         * Calculation parameters shared accross all detectors.
         */
        struct CalcParams {
            bool passVetoes;
            bool inExtMode;
            bool outExtMode;
            bool processModeNew;

            bool checkAdjTube;          //!< Switch to toggle checking for adjacent tubes
            float gNongapMaxRatio;
            bool efficiencyBoost;       //!< Ignore min threshold if single channel response

            uint8_t timeRange1Min;      //!< Threshold for first time range bin
            uint8_t timeRange2Min;      //!< Threshold for second time range bin
            uint32_t timeRangeDelayMin; //!< Delayed event threshold
            uint32_t tofResolution;     //!< Min time between two events in 100ns
        };

        /**
         * Structure used for returning event counters from processPacket() function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class Stats {
            private:
                int32_t counters[20];   //!< Array of different veto counters

            public:
                /**
                 * Constructor initializes all internal counters.
                 */
                Stats();

                /**
                 * Reset counters
                 */
                void reset();

                /**
                 * Sum counters from two objects
                 */
                Stats &operator+=(const Stats &rhs);

                /**
                 * Increase number of particular veto counts by 1
                 */
                void increment(CRocDataPacket::VetoType type);

                /**
                 * Get number of selected vetoed events.
                 */
                int32_t get(CRocDataPacket::VetoType type) const;

                /**
                 * Get number of all events combined.
                 */
                int32_t getTotal() const;
        };

    private: // variables

        epicsMutex m_paramsMutex;       //!< Mutex protecting detector and calculation parameters structures

        std::map<std::string, CRocParams*> m_detParamsByName;           //!< Map of per-CROC parameter structures
        std::unordered_map<uint32_t, CRocParams*> m_detParamsByPosition;//!< Fast lookup of the parameter structures based on position

        CalcParams m_calcParams;    //!< Parameters common to all CROCs

        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        DasPacketList m_packetList; //!< Local list of packets that plugin populates and sends to connected plugins, TODO: get rid of this one
        Stats m_stats;              //!< Event counters

    public: // structures and defines

        /**
         * Constructor for CRocPosCalcPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] bufSize size of the internal buffer for copying packets
         */
        CRocPosCalcPlugin(const char *portName, const char *dispatcherPortName, int bufSize=0);

        /**
         * Destructor
         *
         * Cleanup parameters cache and free memory
         */
        ~CRocPosCalcPlugin();

    protected:

        /**
         * Handle writing integer values, including parameters sent from CRocPlugin.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function to process incoming OCC packets.
         *
         * Function receives all packets and invokes processPacket()
         * function on neutron data packets. All other packets are not
         * processed. Processed and non-processed packets are then sent to
         * subscribed plugins.
         * Function updates statistical PVs.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

        /**
         * Process single neutron data packet in CROC raw output format.
         *
         * The input data format is expected to be either raw or extended.
         * Each raw event is processed using the calculatePixel() function
         * and converted to extended event (adding pixel to the end of raw).
         * Depending on the outExtMode parameter, the output is either
         * tof,pixel event format or extended CROC events.
         *
         * @param[in] inPacket Original packet to be processed
         * @param[out] outPacket output packet with all events processed.
         * @return event counters structure
         */
        Stats processPacket(const DasPacket *inPacket, DasPacket *outPacket);

        /**
         * Calculates pixel id from raw event data.
         */
        CRocDataPacket::VetoType calculatePixel(const CRocDataPacket::RawEvent *event, CRocParams *params, uint32_t &pixel, uint64_t pulseId);

        /**
         * Verify the time spectrum range of the event.
         *
         * The 4 time range bins are used to validate the time shape of the neutron
         * event and catch non-neutron events like gamma.
         */
        CRocDataPacket::VetoType checkTimeRange(const CRocDataPacket::RawEvent *event, const CRocParams *detParams);

        CRocDataPacket::VetoType calculateYPosition(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &y);
        CRocDataPacket::VetoType calculateYPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &y);

        CRocDataPacket::VetoType calculateXPosition(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &x);
        CRocDataPacket::VetoType calculateXPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &x);

        CRocDataPacket::VetoType calculateGPositionMultiGapReq(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t xIndex, uint8_t &gIndex);

        CRocDataPacket::VetoType calculateGPositionNencode(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &xIndex, uint8_t &gIndex);

    private:
        bool findMaxIndex(const uint8_t *values, size_t size, uint8_t &max);

        /**
         * Find out whether the event is more to the right or left around the max.
         *
         * Based on the 1st neighbours of the maximum index, calculate a normalized
         * number that represents left or right side of the maximum where the
         * event might be. Number is between -1.0 and 1.0. 0 means there is no
         * information on which side to look. Closer to -1.0 means more probably
         * the event is on the left edge of max G and closer to 1.0 means right
         * edge.
         *
         * @param[in] values Array of values to consider
         * @param[in] size of the array
         * @param[in] maxIndex Index of the maximum value within array
         * @return Normalized floating point number in range [-1.0, 1.0]
         */
        float findDirection(const uint8_t *values, size_t size, uint8_t maxIndex);

        double calculateGNoise(const uint8_t *values, uint8_t maxIndex, const uint8_t weights[14]);
        double calculateXNoise(const uint8_t *values, uint8_t maxIndex, const uint8_t weights[11]);
        double calculateYNoise(const uint8_t *values, uint8_t maxIndex, const uint8_t weights[7]);

        /**
         * Find the maximum three indexes in the array.
         *
         * @param[in] values array
         * @param[in] size of array
         * @param[out] max1 index of maximum value
         * @param[out] max2 index of second max value
         * @param[out] max3 index of third max value
         * @return number of non-zero values in array.
         */
        static uint8_t findMaxIndexes(const uint8_t *values, size_t size, uint8_t &max1, uint8_t &max2, uint8_t &max3);

        /**
         * Save a single detector parameter into cache
         *
         * Find CRocParams structure based on detector name. Create new one if
         * not found. Then assign new value to corresponding parameter in the
         * structure.
         * When setting position parameter, function will also connect the found
         * CRocParameter to the lookup-by-position table.
         * Must be thread protected from the outside.
         *
         * @param[in] detector to which parameter belongs to
         * @param[in] param name of the parameter
         * @param[in] value of parameter
         */
        void saveDetectorParam(const std::string &detector, const std::string &param, epicsInt32 value);

    protected:
        #define FIRST_CROCPOSCALCPLUGIN_PARAM ErrMem
        int ErrMem;         //!< Error allocating buffer
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int CalcEn;         //!< Toggle position calculation
        int OutExtMode;     //!< Switch to toggle between normal and extended output format
        int PassVetoes;     //!< Allow vetoes in output stream
        int GNongapMaxRatio;//!< Second max G ratio
        int EfficiencyBoost;//!< Switch to enable efficiency boost
        int TimeRange1Min;  //!< Min counts in first time range bin
        int TimeRange2Min;  //!< Min counts in second time range bin
        int TimeRangeDelayMin; //!< Delayed event threshold
        int TofResolution;  //!< Time between two events in 100ns
        int ProcessMode;    //!< Select event verification algorithm

        int CntTotalEvents;    //!< Number of all events
        int CntGoodEvents;     //!< Number of good events
        int CntVetoYLow;       //!< Y signal low
        int CntVetoYNoise;     //!< Y signal noisy
        int CntVetoXLow;       //!< X signal low
        int CntVetoXNoise;     //!< X signal noisy
        int CntVetoGLow;       //!< G signal low
        int CntVetoGNoise;     //!< G signal noisy
        int CntVetoGGhost;     //!< TODO
        int CntVetoGNonAdj;    //!< TODO
        int CntVetoBadPos;     //!< Invalid detector configuration, no such position configured
        int CntVetoBadCalc;    //!< Bad configuration, no such mapping mode
        int CntVetoEcho;       //!< Event to close to previous
        int CntVetoTimeRange;  //!< Time range bins rejected
        int CntVetoDelayed;    //!< Event delayed based on time range bins

        int GWeights;
        int XWeights;
        int YWeights;
        #define LAST_CROCPOSCALCPLUGIN_PARAM YWeights
};

#endif // CROC_POS_CALC_PLUGIN_H
