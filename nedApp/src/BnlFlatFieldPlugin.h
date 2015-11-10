/* BnlFlatFieldPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BNL_FLAT_FIELD_PLUGIN_H
#define BNL_FLAT_FIELD_PLUGIN_H

#include "BaseDispatcherPlugin.h"

/**
 * BnlFlatFieldPlugin applies BNL specific flat-field transformation
 *
 * Code is heavily based on dcomserver code. Especially X,Y calculation is
 * almost exact copy. The plugins' input are two files .prmcalc and .val in
 * the format dcomserver understands and FlatFieldGen tool generates.
 *
 * As of 10/21/2015 the BNL ROC in normal mode is outing X,Y with 9 bits
 * precision, while the flat-field code uses 8byte floating precision.
 * dcomserver never uses BNL normal mode. This plugin works with BNL ROC
 * raw mode and it does the X,Y calculation itself when in raw mode.
 */
class BnlFlatFieldPlugin : public BaseDispatcherPlugin {
    private: // variables

        /**
         * Possible errors
         */
        typedef enum {
            MAP_ERR_NONE        = 0, //!< No error
            MAP_ERR_VALUES      = 1, //!< Failed to load correction values file
            MAP_ERR_PARAMS      = 2, //!< Failed to load parameters file
            MAP_ERR_NO_MEM      = 3, //!< Failed to allocate internal buffer
        } ImportError;

        /**
         * Processing mode
         */
        typedef enum {
            MODE_PASSTHRU       = 0, //!< Don't correct any events
            MODE_CONVERT        = 1, //!< Convert to generic normal data format
            MODE_CORRECT        = 2, //!< Apply flat-field correction
            MODE_CORRECT_CONVERT= 3, //!< Apply correction and convert data format
        } ProcessMode_t;

        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        DasPacketList m_packetList; //!< Local list of packets that plugin populates and sends to connected plugins

        float m_xyDivider;          //!< Fixed point Qm.n -> double converter
        double m_correctionScale;   //!< Correction parameter
        double m_correctionResolution;  //!< Correction parameter
        double m_boundaryLowX;      //!< Calculated X low boundary
        double m_boundaryHighX;     //!< Calculated X high boundary
        double m_boundaryLowY;      //!< Calculated Y low boundary
        double m_boundaryHighY;     //!< Calculated Y high boundary
        int m_centroidMin;          //!< Used by X,Y calculation for vetoing certain events
        double m_xCentroidScale;    //!< Used by X,Y calculation
        double m_yCentroidScale;    //!< Used by X,Y calculation
        uint8_t m_pixelRes;         //!< How many bits to use for X,Y resolution
        int m_xScales[20];          //!< X calculation parameter
        int m_yScales[17];          //!< Y calculation parameter
        int m_xOffsets[20];         //!< X calculation parameter
        int m_yOffsets[17];         //!< Y calculation parameter

        /**
         * Table describes single dimension correction, populated from input files.
         */
        struct CorrectionTable {
            bool initialized;
            uint32_t nX;
            uint32_t nY;
            uint32_t method;
            double xScale;
            double yScale;
            std::vector<double> rangeLow;
            std::vector<double> rangeHigh;
            std::vector<double> rangeCenter;
            std::vector<double> values;
        } m_tableX, m_tableY;

    public: // structures and defines
        /**
         * Constructor for BnlFlatFieldPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] paramFile Path to a .prmcalc file
         * @param[in] valFile Path to a .val file
         * @param[in] bufSize size of the internal buffer for copying packets
         */
        BnlFlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *paramFile, const char *valFile, int bufSize=0);

        /**
         * Load single BNL ROC correction parameters
         *
         * @param[in] paramFile is meta file describing correction factor table
         * @param[in] valFile is binary file with correction factors
         */
        void loadTable(const std::string &paramFile, const std::string &valFile);

        /**
         * Load correction parameters form text file
         *
         * .prmcalc is XML style file that FlatFieldGen and dcomserver use
         * for the basic FlatField algorithm. It contains limits, max values
         * and other parameters for driving correction process.
         *
         * @param[in] paramFile path to a file to be read
         * @return true on success, false otherwise
         */
        bool loadParamFile(const std::string &paramFile);

        /**
         * Load correction values from a binary file.
         *
         * This function must be called after loadParamFile() has successfully
         * returned as it uses internal variables set by the other function.
         * It allocates and reads correction values for X and Y positions.
         *
         * @param[in] valFile path to a file to be read.
         * @return true on success, false otherwise
         */
        bool loadValueFile(const std::string &valFile);

    private:
        /**
         * Read and parse single line from .prmcalc file
         *
         * @param[in] fp previously opened file descriptor
         * @param[out] key is XMLs tag name
         * @param[out] value is XMLs tag value
         * @return false if no more data to read, true when key and value are set
         */
        bool readLine(FILE *fp, std::string &key, std::string &value);

        /**
         * Read a line from .prmcalc file and check that it matches.
         *
         * Used to identify XML header.
         *
         * @param[in] fp previously opened file descriptor
         * @param[in] match is the exact string to be matched, except for any leading or trailing white character
         * @return true if line matches
         */
        bool matchLine(FILE *fp, const std::string &match);

        /**
         * Strip white characters from beggining and end of string
         *
         * White characters are identified by isspace() C function.
         *
         * @param[in] str input string to be modified
         * @return a string points to first non-white-space character
         */
        char *strip(char *str);

        /**
         * Parse comma separated string into double tokens.
         *
         * @param[in] str string to be parsed
         * @param[out] vals vector of parsed values
         */
        void toArray(const std::string &str, std::vector<double> &vals);

        /**
         * Parse comma separated string into integer tokens.
         *
         * @param[in] str string to be parsed
         * @param[out] vals vector of parsed values
         */
        void toArray(const std::string &str, std::vector<int> &vals);

        /**
         * Overloaded function to process incoming OCC packets.
         *
         * Function receives all packets and invokes processPacket*()
         * functions on non-neutron data packets. All other packets are not
         * processed. Processed and non-processed packets are then sent to
         * subscribed plugins.
         * Function updates statistical PVs.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

        /**
         * Process single neutron data packet in BNL ROC raw output format.
         *
         * Takes one raw event at a time and first calculate X,Y position from
         * raw samples. If successful, then run correction on X,Y position.
         * And finally encode it to tof,pixelid format. All events that fail
         * to produce good X,Y position or the flat-field correction fails on
         * them is tossed. Number of tossed events is returned at the end.
         * Events that got converted and corrected are put into output packet.
         * Output packet header is copied from original one and relevant field
         * (like payload length) are adjusted.
         *
         * @param[in] srcPacket Original packet to be processed
         * @param[out] destPacket output packet with all events processed.
         * @param[in] xyDivider used to convert Qm.n unsigned -> double
         * @param[in] processMode event processing mode
         * @param[out] nCorr number of corrected events
         * @param[out] nVetoed number of vetoed events
         */
        void processPacket(const DasPacket *srcPacket, DasPacket *destPacket, float xyDivider, ProcessMode_t processMode, uint32_t &nCorr, uint32_t &nVetoed);

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
        bool calculatePositionRaw(const uint32_t *data, double *x, double *y);

        /**
         * Apply flat field correction to the X,Y position
         *
         * Using the values from .prmcalc and .val file, the algorithm moves
         * X,Y position so that the histogrammed flood data becomes flat. For
         * real sample data the same transformation is applied and eliminates
         * image imperfections.
         *
         * @note Function is direct copy from dcomserver.
         *
         * @param[out] x position to be corrected
         * @param[out] y position to be corrected
         * @return true when position was corrected, false when it was rejected
         */
        bool correctPosition(double *x, double *y);

    protected:
        #define FIRST_BNLFLATFIELDPLUGIN_PARAM ParamsPath
        int ParamsPath;     //!< Absolute path to parameters file
        int ValPath;        //!< Absolute path to correction values file
        int ErrImport;      //!< Import mapping file error (see PixelMapPlugin::ImportError)
        int XyFractWidth;   //!< Number of fraction bits in X,Y Qm.n format
        int CntGoodEvents;  //!< Number of calculated events
        int CntVetoEvents;  //!< Number of vetoed events
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int ProcessMode;    //!< Event processing mode
        int PixelRes;       //!< How many bits to use for X,Y resolution
        #define LAST_BNLFLATFIELDPLUGIN_PARAM PixelRes
};

#endif // BNL_FLAT_FIELD_PLUGIN_H
