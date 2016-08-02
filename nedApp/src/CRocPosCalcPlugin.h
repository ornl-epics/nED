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

            float gNongapMaxRatio;  //!< Percentage of the second max G comparing to max in order to qualify
            bool efficiencyBoost;       //!< Ignore min threshold if single channel response

            // For testing purposes for now, consider moving to detectors otherwise
            uint8_t timeRange1Min;      //!< Threshold for first time range bin
            uint8_t timeRange2Min;      //!< Threshold for second time range bin
            uint32_t timeRangeDelayMin; //!< Delayed event threshold
            uint32_t timeRangeSumMax;   //!< Max value for sumed time ranges
            uint32_t echoDeadTime;      //!< Min time between two space-close events in 100ns
            uint32_t echoDeadArea;      //!< Area in pixels for echo rejection
        };

        /**
         * Structure used for returning event counters from processPacket() function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class Stats {
            private:
                int32_t counters[20];   //!< Array of different veto counters
                uint64_t total;         //!< All counters combined

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
         *
         * This is the top level function to convert a raw event into common
         * neutron format. Based on run time parameters, it decides which
         * discrimination and event calculations algorithms to use. Rejected
         * events are flagged with veto masks. Dirty work is done by other
         * functions in this order:
         *  * checkTimeRange()
         *  * calculateYPosition()
         *  * calculateXPosition()
         *
         * @param[in] event input in raw format
         * @param[in] params of the detector
         * @param[in] pulseId is unique id of the pulse this event belongs to
         * @param[out] pixel calculated, upper bits describe veto flags
         * @return When event is rejected, the return veto type describes the
         *         reason for rejection. The same type is put in upper bits of
         *         pixel.
         */
        CRocDataPacket::VetoType calculatePixel(const CRocDataPacket::RawEvent *event, CRocParams *params, uint64_t pulseId, uint32_t &pixel);

        /**
         * Verify the 4 time ranges of the raw event.
         *
         * Raw event provides counts in 4 time range bins. Width of the bins
         * is configured in detector settings. The information from time ranges
         * is used to detect a real neutron event and eliminate potential gamma
         * events. Function here does only elimination, no correction.
         *
         * The discrimination involves several steps:
         *   * minimum number of bins with high enough counts
         *   * integrated number of counts must be below the max threshold
         *   * first two bins must be above the min limit
         *
         * @param[in] event input in raw format
         * @param[in] detParams settings of detector
         * @return Positive number when rejected, 0 otherwise.
         */
        CRocDataPacket::VetoType checkTimeRange(const CRocDataPacket::RawEvent *event, const CRocParams *detParams);

        /**
         * Calculate the Y position of the event
         *
         * Detector provides 7 Y positions.
         *
         * Function first does low signal and noisy signal discrimination.
         * Number of channels with counts over the minimum must not exceed
         * configurable threshold. If no channel reaches the minimum count value,
         * a second chance is given if efficiency boost is enabled. Only if two
         * adjacent events sumed together reach the minimum the event is allowed.
         *
         * The calculated Y position is simply the index of the channel with
         * highest counts value. Except when two channels have the same highest
         * value. In such case the third adjacent channel determines the final
         * Y index.
         *
         * @param[in] event input in raw format
         * @param[in] detParams settings of detector
         * @param[out] y calculated position
         * @return Positive number when rejected, 0 otherwise.
         */
        CRocDataPacket::VetoType calculateYPosition(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &y);

        /**
         * Calculate the Y position of the event
         *
         * @todo Work in progress, not used right now due to ghosting.
         */
        CRocDataPacket::VetoType calculateYPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &y);

        /**
         * Calculate the X position of the event
         *
         * Due to limited number of PMTs, X information is organized into 14
         * groups called G channels and 11 X channels for finer position within
         * the group. Each channels provides number of photon counts in that
         * channel in given acquisition time. To calculate the overall X position,
         * the process is easy in theory with a lot of corner cases to handle.
         * Find the G channel with most counts and multiply its index with 11.
         * Then find the X channel and add its index to previosuly multipled
         * value. The outcome is the absolute X position of the event.
         *
         * When X is close to 0 or 10 edge, its response could leak to the other
         * side of channels and 2 G channels will share the counts. The
         * algorithm tries to use G and X channel information to determine the
         * correct position of the event. Error to do so will move the pixel id
         * 11 or even 22 pixels to the either direction. Such an error is often
         * called ghosting in SNS. PMT physical mapping can improve the situation
         * greatly and ghosting is almost removed with mapping v3.
         *
         * Function first does low signal and noisy signal discrimination.
         * Number of channels with counts over the minimum must not exceed
         * configurable threshold. If no channel reaches the minimum count value,
         * a second chance is given if efficiency boost is enabled. Only if two
         * adjacent events sumed together reach the minimum the event is allowed.
         *
         * The rest of the function deals with properly determining the correct
         * location of the event based on selected detector mapping. It will
         * change G and/or X index when certain. When uncertain, it will reject
         * the event as ghost.
         *
         * Finally the X position is calculated as X = Gindex*11 + Xindex.
         *
         * @param[in] event input in raw format
         * @param[in] detParams settings of detector
         * @param[out] x calculated position
         * @return Positive number when rejected, 0 otherwise.
         */
        CRocDataPacket::VetoType calculateXPosition(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &x);

        /**
         * Calculate the X position of the event
         *
         * @todo Work in progress, not used right now due to ghosting.
         */
        CRocDataPacket::VetoType calculateXPositionNew(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint8_t &x);

    private:
        /**
         * Return an with of the maximum value or the first one found.
         *
         * When two or more values are the same, index of the first one is
         * returned.
         *
         * @param[in] values input table
         * @param[in] size of the input table
         * @param[out] max found index
         * @return true when found, false when all values are 0.
         */
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

        /**
         * Return a ratio of noise comparing to the max value.
         *
         * For each value from the input table determine the index distance from
         * the maxIndex. Use found distance as the index into the weights table
         * and multiply weight with the input value. Sum all together and divide
         * by value at maxIndex. Returned ratio equal to 1.0 when maxIndex is
         * the only non-zero value or >1.0 otherwise.
         *
         * @param[in] values input table
         * @param[in] size of the input table and weights table
         * @param[in] maxIndex index of the maximum value in input table
         * @param[in] weights is a table with weights
         * @return noise ratio
         */
        double calculateNoise(const uint8_t *values, size_t size, uint8_t maxIndex, const uint8_t *weights);

        /**
         * Values from the input table are sorted in descending order and the list of indeces is returned.
         *
         * @param[in] values input table
         * @param[in] size of the input table
         * @return a list indeces of the same size as input table
         */
        static inline std::vector<uint8_t> sortIndexesDesc(const uint8_t *values, size_t size);

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
        int ErrMem;             //!< Error allocating buffer
        int CntSplit;           //!< Total number of splited incoming packet lists
        int ResetCnt;           //!< Reset counters
        int CalcEn;             //!< Toggle position calculation
        int OutExtMode;         //!< Switch to toggle between normal and extended output format
        int PassVetoes;         //!< Allow vetoes in output stream
        int GNongapMaxRatio;    //!< Second max G ratio
        int EfficiencyBoost;    //!< Switch to enable efficiency boost
        int TimeRange1Min;      //!< Min counts in first time range bin
        int TimeRange2Min;      //!< Min counts in second time range bin
        int TimeRangeDelayMin;  //!< Delayed event threshold
        int TimeRangeSumMax;    //!< High threshold for integrated time ranges
        int EchoDeadTime;       //!< Time between two events in 100ns
        int EchoDeadArea;       //!< High threshold for integrated time ranges
        int ProcessMode;        //!< Select event verification algorithm

        int CntTotalEvents;    //!< Number of all events
        int CntGoodEvents;     //!< Number of good events
        int CntVetoYLow;       //!< Y signal low
        int CntVetoYNoise;     //!< Y signal noisy
        int CntVetoXLow;       //!< X signal low
        int CntVetoXNoise;     //!< X signal noisy
        int CntVetoGLow;       //!< G signal low
        int CntVetoGNoise;     //!< G signal noisy
        int CntVetoGGhost;     //!< Rejected due to ghosting
        int CntVetoGNonAdj;    //!< Non adjacent channels
        int CntVetoBadPos;     //!< Invalid detector configuration, no such position configured
        int CntVetoBadCalc;    //!< Bad configuration, no such mapping mode
        int CntVetoEcho;       //!< Event to close to previous
        int CntVetoTimeRange;  //!< Time range bins rejected
        int CntVetoDelayed;    //!< Event delayed based on time range bins
        #define LAST_CROCPOSCALCPLUGIN_PARAM CntVetoDelayed
};

#endif // CROC_POS_CALC_PLUGIN_H
