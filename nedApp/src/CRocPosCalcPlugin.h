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
 * CRocPosCalcPlugin canverts detector raw data into detector normal data
 *
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
            uint32_t position;
            uint16_t timeRange1;
            uint16_t timeRange2;
            uint16_t timeRange3;
            uint16_t timeRange4;

            uint32_t xMin;          //!< Min number of counts on each x sample
            uint32_t xCntMax;       //!< Max number of valid x samples
            uint32_t yMin;          //!< Min number of counts on each y sample
            uint32_t yCntMax;       //!< Max number of valid y samples

            uint32_t gMin;
            uint32_t gMin2;
            uint32_t gGapMin1;
            uint32_t gGapMin2;

            enum {
                ENCODE_MULTI_GAP_REQ,
                ENCODE_V3,
            } mapMode;
        };

        /**
         * Calculation parameters shared accross all detectors.
         */
        struct CalcParams {
            bool passVetoes;
            bool inExtMode;
            bool outExtMode;

            bool checkAdjTube;          //!< Switch to toggle checking for adjacent tubes
            float gNongapMaxRatio;
            bool efficiencyBoost;   //!< Switch for efficiency boost calculation
        };

        /**
         * Structure used for returning event counters from processPacket() function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class Stats {
            public:
                int32_t nTotal;         //!< Total number of events in packet
                int32_t counters[20];   //!< Array of different veto counters

                #define VETO2INT(a) (((a) & ~0x80000000) >> 16)

                Stats()
                {
                    reset();
                }

                void reset()
                {
                    nTotal = 0;
                    for (size_t i=0; i<sizeof(counters)/sizeof(int32_t); i++) {
                        counters[i] = 0;
                    }
                }

                Stats &operator+=(const Stats &rhs)
                {
                    nTotal += rhs.nTotal;
                    for (size_t i=0; i<sizeof(counters)/sizeof(int32_t); i++) {
                        counters[i] += rhs.counters[i];
                    }
                    if (nTotal > std::numeric_limits<int32_t>::max()) {
                        reset();
                    }
                    return *this;
                }

                void increment(CRocDataPacket::VetoType type)
                {
                    nTotal++;
                    counters[VETO2INT(type)]++;
                }

                int32_t get(CRocDataPacket::VetoType type)
                {
                    return counters[VETO2INT(type)];
                }
        };

    private: // variables

        epicsMutex m_paramsMutex;       //!< Mutex protecting detector and calculation parameters structures

        std::map<std::string, CRocParams*> m_detParamsByName;           //!< Map of per-CROC parameter structures
        std::unordered_map<uint32_t, CRocParams*> m_detParamsByPosition;//!< Fast lookup of the parameter structures based on position

        CalcParams m_calcParams;    //!< Parameters common to all CROCs

        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        DasPacketList m_packetList; //!< Local list of packets that plugin populates and sends to connected plugins
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
        CRocDataPacket::VetoType calculatePixel(const CRocDataPacket::RawEvent *event, const CRocParams *params, uint32_t &pixel);

        CRocDataPacket::VetoType calculateYPosition(const CRocDataPacket::RawEvent *event, const CRocParams *params, double &y);

        CRocDataPacket::VetoType calculateXPosition(const CRocDataPacket::RawEvent *event, const CRocParams *params, double &x);

        CRocDataPacket::VetoType calculateGPositionMultiGapReq(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t xIndex, uint8_t &gIndex);

        CRocDataPacket::VetoType calculateGPositionNencode(const CRocDataPacket::RawEvent *event, const CRocParams *detParams, uint8_t &xIndex, uint8_t &gIndex);

        /**
         * Find the maximum three indexes in the array.
         *
         * @param[in] values array
         * @param[in] size of array
         * @param[out] max1 index of maximum value
         * @param[out] max2 index of second max value
         * @param[out] max3 index of third max value
         */
        static void findMaxIndexes(const uint8_t *values, size_t size, uint8_t &max1, uint8_t &max2, uint8_t &max3);

        /**
         * Save a single detector parameter into cache
         *
         * Find CRocParams structure based on detector name. Create new one if
         * not found. Then assign new value to corresponding parameter in the
         * structure.
         * When setting position parameter, function will also connect the found
         * CRocParameter to the lookup-by-position table.
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
        int PassVetoes;     //!< Allow vetoes in output stream
        #define LAST_CROCPOSCALCPLUGIN_PARAM PassVetoes
};

#endif // CROC_POS_CALC_PLUGIN_H
