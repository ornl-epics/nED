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

#include "BasePlugin.h"
#include "Event.h"
#include "ObjectPool.h"

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
class BnlPosCalcPlugin : public BasePlugin {
    private: // definitions

        /**
         * Return codes from calculation function
         */
        typedef enum {
            CALC_SUCCESS,           //!< Success code, event was calculated
            CALC_OVERFLOW_FLAG,     //!< Error code, over-flow flag was detected in one or many raw values
            CALC_EDGE,              //!< Error code, event to close to the edge
            CALC_LOW_CHARGE,        //!< Error code, event charge below threshold
            CALC_MULTI_EVENT,       //!< Error code, multiple events detected
            CALC_BAD_CONFIG,        //!< Error code, bad or missing configuration
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
                int32_t nMultiEvent;    //!< Number of multiple events - vetoed
                int32_t nBadConfig;     //!< Number of bad configuration events

                Stats()
                    : nTotal(0)
                    , nGood(0)
                    , nOverflow(0)
                    , nEdge(0)
                    , nLowCharge(0)
                    , nMultiEvent(0)
                    , nBadConfig(0)
                {}

                Stats &operator+=(const Stats &rhs)
                {
                    nTotal += rhs.nTotal;
                    nGood += rhs.nGood;
                    nOverflow += rhs.nOverflow;
                    nEdge += rhs.nEdge;
                    nLowCharge += rhs.nLowCharge;
                    nMultiEvent += rhs.nMultiEvent;
                    nBadConfig += rhs.nBadConfig;
                    if (nTotal > std::numeric_limits<int32_t>::max()) {
                        nTotal = nGood = nOverflow = nEdge = nLowCharge = nMultiEvent = nBadConfig = 0;
                    }
                    return *this;
                }
        };

        /**
         * Structure holding all calculation params
         */
        struct CalcParams {
            float xyDivider;                  //!< Calculated Qm.n -> unsigned converter
            double correctionScale;           //!< Correction parameter
            double correctionResolution;      //!< Correction parameter
            double boundaryLowX;              //!< Calculated X low boundary
            double boundaryHighX;             //!< Calculated X high boundary
            double boundaryLowY;              //!< Calculated Y low boundary
            double boundaryHighY;             //!< Calculated Y high boundary
            int nCalcValues;                  //!< Number of values used in calculation
            bool lowChargeVetoEn;             //!< Toggle low charge rejection
            bool overflowVetoEn;              //!< Toggle overflow rejection
            bool edgeVetoEn;                  //!< Toggle edge rejection
            bool multiEventVetoEn;            //!< Toggle multi-event rejection
            int centroidMin;                  //!< Used by X,Y calculation for vetoing certain events
            double xCentroidScale;            //!< Used by X,Y calculation
            double yCentroidScale;            //!< Used by X,Y calculation
            int xScales[20];                  //!< X calculation parameter
            int yScales[17];                  //!< Y calculation parameter
            int xOffsets[20];                 //!< X calculation parameter
            int yOffsets[17];                 //!< Y calculation parameter
            int xMinThresholds[20];           //!< X thresholds after calculation
            int yMinThresholds[17];           //!< Y thresholds after calculation
            int xMinThresholdsUnscaled[20];   //!< Unscaled X thresholds calculation
            int yMinThresholdsUnscaled[17];   //!< Unscaled Y thresholds calculation
        };


    private: // variables
        CalcParams m_calcParams;    //!< Container for all calculation parameters
        Stats m_stats;              //!< Event counters
        ObjectPool<DasDataPacket> m_packetsPool{true};  //!< Pool of allocated data packets to store modified data

    public: // structures and defines

        /**
         * Constructor for BnlPosCalcPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins Name of the dispatcher asyn port to connect to.
         */
        BnlPosCalcPlugin(const char *portName, const char *parentPlugins);

    private:

        /**
         * Handle writing integer values.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

        /**
         * Overloaded function to process incoming data packets.
         */
        void recvDownstream(const DasDataPacketList &packets) override;

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
         * @return Packet from pool containing calculated events and stats structure.
         */
        std::pair<DasDataPacket *, Stats> processPacket(const DasDataPacket *srcPacket, const CalcParams &calcParams);

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
         * When CALC_ALTERNATIVE pre-processor switch is enabled, the
         * implementation of this function is different. The alternative
         * approach tries to more precisely calculate the position using more
         * than 3 raw values.
         * The calculation algorithm begins with converting 20 X and 17 Y raw
         * samples with 12 bit precision to scaled values for better precision.
         * Any converted value below the sample threshold is considered 0.
         * It then finds a single peak in each direction. If more than one
         * local maxima is observed, event is rejected as multi-event veto.
         * If no peak is found for each dimension, event is rejected as
         * low charge veto.
         * Samples are next smoothed so that they form a single triangle
         * along axis. Any smaller peaks that don't qualify as an event
         * are removed by smoothening.
         * Number of values used in calculation is defined by user but can
         * be reduced around the edges. If number is reduced to 1, event is
         * rejected as edge veto unless user requested 1 value for calculation.
         * Finally the X and Y positions are calculated based on centroid
         * finding method.
         *
         * When number of values used for calculation is low, events are pulled
         * towards the integer values and causing a sharp dip in the center
         * between two integer values.
         * It was empirically observed that 5 values in calculation already
         * give good results with flood data. 6 values was sufficiently good
         * for lab experiment but does have undesiried effect around edges.
         * Values greater than 7 don't bring further improvements.
         *
         * @param[in] raw event data
         * @param[in] calcParams params used in calculation
         * @param[out] x calculated position
         * @param[out] y calculated position
         * @return calculation status
         */
        calc_return_t calculatePosition(const Event::BNL::Raw *event, const CalcParams &calcParams, double *x, double *y);

    protected:
        int ErrMem;             //!< Error allocating buffer
        int LowChargeVetoEn;    //!< Switch for toggle low charge rejection
        int OverflowVetoEn;     //!< Switch for toggle overflow rejection
        int EdgeVetoEn;         //!< Switch for toggle edge rejection
        int MultiEventVetoEn;   //!< Switch for toggle multi-event rejection
        int CntTotalEvents;     //!< Number of total events
        int CntGoodEvents;      //!< Number of calculated events
        int CntEdgeVetos;       //!< Number of vetoed events due to close to edge
        int CntLowChargeVetos;  //!< Number of vetoed events due to low charge
        int CntOverflowVetos;   //!< Number of vetoed events due to overflow flag
        int CntMultiEventVetos; //!< Number of vetoed events due to multiple peaks
        int BadConfig;          //!< Something is wrong with configuration parameters
        int ResetCnt;           //!< Reset counters
        int CalcEn;             //!< Toggle position calculation
        int NumCalcValues;      //!< Number of values to use in calculation
        int CentroidMin;        //!< Centroid minimum parameter for X,Y calculation
        int XCentroidScale;     //!< Centroid X scale factor
        int YCentroidScale;     //!< Centroid Y scale factor
        int XScales[20];
        int YScales[17];
        int XOffsets[20];
        int YOffsets[17];
        int XMinThresholds[20];
        int YMinThresholds[17];
};

#endif // BNL_POS_CALC_PLUGIN_H
