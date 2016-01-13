/* BnlPosCalcPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BNL_POS_CALC_PLUGIN_H
#define BNL_POS_CALC_PLUGIN_H

#include "BaseDispatcherPlugin.h"
#include "BnlDataPacket.h"
#include "PvaCalcVerifyData.h"

#include <limits>

/**
 * BnlPosCalcPlugin canverts detector raw data into detector normal data
 *
 * Converting raw data to normal data is usually necessary when debugging
 * firmware doing the calculation or when firmware is not doing it at all.
 *
 * Such case is BNL ROC 2.0 implementation where the normal mode was done
 * in a LPSD ROC compatible way but was later found not to be precise
 * enough for required flat-field correction. Calculation was probably
 * already implemented in the software so they stopped using normal mode
 * altogether and dcomserver only used raw mode. There is an effort to
 * update firmware and do the calculation there.
 *
 * In extended data format mode, the plugin can calculate X,Y position
 * the same way as in raw mode. It will push firmware calculated and
 * software calculated X,Y positions to PVA channel.
 */
class BnlPosCalcPlugin : public BaseDispatcherPlugin {
    private: // definitions

        /**
         * Return codes from calculation function
         */
        typedef enum {
            CALC_SUCCESS,           //!< Success code, event was calculated
            CALC_OVERFLOW_FLAG,     //!< Error code, over-flow flag was detected in one or many raw values
            CALC_EDGE,              //!< Error code, event to close to the edge
            CALC_LOW_CHARGE,        //!< Error code, event charge below threshold
        } calc_return_t;

        /**
         * Structure used for returning event counters from processPacket() function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class Stats {
            public:
                int32_t nTotal;         //!< Total number of events in packet
                int32_t nGood;          //!< Number of good events in packet
                int32_t nOverflow;      //!< Number of over-flow flagged events - vetoed
                int32_t nEdge;          //!< Number of events on the edge - vetoed
                int32_t nLowCharge;     //!< Number of events with low charge - vetoed

                Stats()
                    : nTotal(0)
                    , nGood(0)
                    , nOverflow(0)
                    , nEdge(0)
                    , nLowCharge(0)
                {}

                Stats &operator+=(const Stats &rhs)
                {
                    nTotal += rhs.nTotal;
                    nGood += rhs.nGood;
                    nOverflow += rhs.nOverflow;
                    nEdge += rhs.nEdge;
                    nLowCharge += rhs.nLowCharge;
                    if (nTotal > std::numeric_limits<int32_t>::max()) {
                        nTotal = nGood = nOverflow = nEdge = nLowCharge = 0;
                    }
                    return *this;
                }
        };


    private: // variables

        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        DasPacketList m_packetList; //!< Local list of packets that plugin populates and sends to connected plugins

        float m_xyDivider;          //!< Calculated Qm.n -> unsigned converter
        double m_correctionScale;   //!< Correction parameter
        double m_correctionResolution;  //!< Correction parameter
        double m_boundaryLowX;      //!< Calculated X low boundary
        double m_boundaryHighX;     //!< Calculated X high boundary
        double m_boundaryLowY;      //!< Calculated Y low boundary
        double m_boundaryHighY;     //!< Calculated Y high boundary
        int m_centroidMin;          //!< Used by X,Y calculation for vetoing certain events
        double m_xCentroidScale;    //!< Used by X,Y calculation
        double m_yCentroidScale;    //!< Used by X,Y calculation
        int m_xScales[20];          //!< X calculation parameter
        int m_yScales[17];          //!< Y calculation parameter
        int m_xOffsets[20];         //!< X calculation parameter
        int m_yOffsets[17];         //!< Y calculation parameter
        int m_xMinThresholds[20];   //!< X thresholds after calculation
        int m_yMinThresholds[17];   //!< Y thresholds after calculation
        Stats m_stats;              //!< Event counters

        PvaCalcVerifyData::shared_pointer m_pva;    //!< PVA channel for position calculated data verification

    public: // structures and defines

        /**
         * Constructor for BnlPosCalcPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] bufSize size of the internal buffer for copying packets
         */
        BnlPosCalcPlugin(const char *portName, const char *dispatcherPortName, int bufSize=0);

    private:

        /**
         * Handle writing integer values.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Register to asyn string writes.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded function to process incoming OCC packets.
         *
         * Function receives all packets and invokes processPacket*()
         * functions on neutron data packets. All other packets are not
         * processed. Processed and non-processed packets are then sent to
         * subscribed plugins.
         * Function updates statistical PVs.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

        /**
         * Process single neutron data packet in BNL ROC raw output format.
         *
         * Allocate output packet to hold all calculated positions. Size of
         * the destination packet is always less than source packet based on
         * structure sizes for different data formats. Header is byte copied.
         * Run X,Y position calculation function on all events in a packet
         * except those marked with special flag and put them into destination
         * packet. Return number of calculated packets and number of vetoes.
         *
         * If m_pva is ready and extendedMode is true, also push data into PVA
         * channel.
         *
         * @param[in] srcPacket Original packet to be processed
         * @param[out] destPacket output packet with all events processed.
         * @param[in] extendedMode Data is in extended mode.
         * @return event counters structure
         */
        Stats processPacket(const DasPacket *srcPacket, DasPacket *destPacket, bool extendedMode);

        /**
         * Calculate X,Y position from raw samples.
         *
         * The algorithm is based on dcomserver and what is described in BNLROC
         * Operations document. It first converts raw values to effective values
         * using the individual scale and offset parameters. It then finds the
         * index of X and Y maximum samples. It refuses out corner indexes.
         * If the calculation denominator is below the centroid minimum threshold,
         * event is rejected.
         * X and Y positions are finally calculated in double precision.
         *
         * @param[in] raw event data
         * @param[out] x calculated position
         * @param[out] y calculated position
         * @return true if X,Y was calculated, false when event was rejected
         */
        calc_return_t calculatePosition(const BnlDataPacket::RawEvent *event, double *x, double *y);

    protected:
        #define FIRST_DATACONVERTPLUGIN_PARAM ErrMem
        int ErrMem;         //!< Error allocating buffer
        int XyFractWidth;   //!< Number of fraction bits in X,Y Qm.n format
        int CntTotalEvents; //!< Number of total events
        int CntGoodEvents;  //!< Number of calculated events
        int CntEdgeVetos;   //!< Number of vetoed events due to close to edge
        int CntLowChargeVetos; //!< Number of vetoed events due to low charge
        int CntOverflowVetos;  //!< Number of vetoed events due to overflow flag
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int CalcEn;         //!< Toggle position calculation
        int CentroidMin;    //!< Centroid minimum parameter for X,Y calculation
        int XCentroidScale; //!< Centroid X scale factor
        int YCentroidScale; //!< Centroid Y scale factor
        int PvaName;        //!< Name of PVA channel for position calculation data
        #define LAST_DATACONVERTPLUGIN_PARAM PvaName
        int XScales[20];
        int YScales[17];
        int XOffsets[20];
        int YOffsets[17];
        int XMinThresholds[20];
        int YMinThresholds[17];
};

#endif // BNL_POS_CALC_PLUGIN_H
