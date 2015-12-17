/* FlatFieldPlugin.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef FLAT_FIELD_PLUGIN_H
#define FLAT_FIELD_PLUGIN_H

#include "BaseDispatcherPlugin.h"
#include "FlatFieldTable.h"

#include <limits>
#include <sstream>

/**
 * FlatFieldPlugin applies flat-field correction to pre-calculated X,Y events.
 *
 * The plugin corrects X,Y events pre-calculated by ACPC. It corrects events
 * according to pre-calibrated correction factors, checks photo sum correctness
 * and converts X,Y event into pixel id that can be used by other plugins or
 * external parties.
 *
 * Internally the algorithm corrects X,Y events using 2 correction tables per
 * postion (one ACPC), one table for each dimension. Those are two-dimensional
 * tables containing correction factors. Algorithm also uses 1 photo sum limits
 * table for X dimension. All tables must be of the same size, usually 512x512
 * elements.
 *
 * Correction tables are used as lookup tables for getting X and Y correction
 * factors. X and Y are first scaled to match tables range. Scaled values
 * are used to locate cell within the table. Using the scaled X and Y, a value
 * from photo sum limits table is obtained and is used to filter out invalid
 * events based on photo sum value. The same scaled X and Y are then used on
 * X correction table as index to get X correction factor. Similarly
 * for Y dimension. X and Y correction factors are applied to raw X and Y
 * accordingly. Finally position id, corrected X and Y tripplet is converted
 * to 32 bit unsigned pixel id using this formula:
 * pixel = (<position id> & 0x3FFF << 18) | (<corrected X> & 0x1FF << 9) | (<corrected Y> & 0x1FF)
 *
 * Sliding photo sum X value is checked to be within pre-calibrated thresholds.
 * Single X photo sum lookup table is used to determine threshold for given
 * X,Y event. Events outside the threshold are eliminated. CntPhotoSum counter
 * is incremented for every eliminated event.
 *
 * Every ACPC should be assigned a unique position id. m_corrTables variables
 * is an array of all positions configured. Each position has at most 4 tables,
 * but at least X correction, Y correction and X photo sum tables.
 *
 * Plugin supports 3 modes of operation (see FlatFieldPlugin::FfMode_t):
 * - In pass-thru mode it forwards all packets intact. This includes also
 *   command responses and other non-neutron data packets.
 * - In flatten mode it applies flat-field correction and photo sum elimination
 *   as described above.
 * - In convert only mode it doesn't apply flat-field correction or phhoto sum
 *   elimination, it only converts X,Y event into TOF,pixel id format that many
 *   other plugins understand.
 */
class FlatFieldPlugin : public BaseDispatcherPlugin {
    private: // structures & typedefs

        /**
         * Contain all tables for a give detector position
         *
         * Structure used in m_tables variable. Not all tables need to be
         * present -> use std::shared_ptr.
         */
        struct PositionTables {
            bool enabled;
            std::shared_ptr<FlatFieldTable> corrX;  //!< Pointer to X correction table
            std::shared_ptr<FlatFieldTable> corrY;  //!< Pointer to Y correction table
            std::shared_ptr<FlatFieldTable> psLowX; //!< Pointer to lower X photosum table
            std::shared_ptr<FlatFieldTable> psUpX;  //!< Pointer to upper X photosum table
        };

        /**
         * Structure used for returning error counters from transform function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class TransformErrors {
            public:
                int32_t nErrors;    //!< Pixel id has error bit already set
                int32_t nUnmapped;  //!< No correction table was found for given module
                int32_t nPhotoSum;  //!< Number of photo sum rejected pixels

                TransformErrors()
                    : nErrors(0)
                    , nUnmapped(0)
                    , nPhotoSum(0)
                {}

                TransformErrors &operator+=(const TransformErrors &rhs)
                {
                    // Prevent overflow, stop counting at INT32_MAX instead
                    if (rhs.nErrors > std::numeric_limits<int32_t>::max() - nErrors)
                        nErrors = std::numeric_limits<int32_t>::max();
                    else
                        nErrors += rhs.nErrors;
                    if (rhs.nUnmapped > std::numeric_limits<int32_t>::max() - nUnmapped)
                        nUnmapped = std::numeric_limits<int32_t>::max();
                    else
                        nUnmapped += rhs.nUnmapped;
                    if (rhs.nPhotoSum > std::numeric_limits<int32_t>::max() - nPhotoSum)
                        nPhotoSum = std::numeric_limits<int32_t>::max();
                    else
                        nPhotoSum += rhs.nPhotoSum;
                    return *this;
                }
        };

        /**
         * Possible errors
         */
        typedef enum {
            FF_ERR_NONE         = 0, //!< No error
            FF_ERR_NO_FILE      = 1, //!< Failed to find file
            FF_ERR_PARSE        = 2, //!< Failed to parse file
            FF_ERR_NO_MEM       = 3, //!< Failed to allocate internal buffer
            FF_ERR_EXIST        = 4, //!< Correction table for this position already imported
            FF_ERR_BAD_FORMAT   = 5, //!< Unrecognized file format
            FF_ERR_BAD_SIZE     = 6, //!< One or more tables dimension size mismatched
            FF_ERR_INCOMPLETE   = 7, //!< One or more tables missing
        } ImportError;

        /**
         * Enable or disable mapping mode
         */
        typedef enum {
            FF_PASS_THRU        = 0, //!< Passthru mode
            FF_FLATTEN          = 1, //!< Transform neutron events
            FF_CONVERT_ONLY     = 2, //!< Convert X,Y event into TOF,pixel id only,
                                     //!< no flat-field correction or photo sum elimination is applied
        } FfMode_t;

    public: // structures and defines
        /**
         * Constructor for FlatFieldPlugin
         *
         * Constructor will create and populate PVs with default values.
         * It will also try to parse all files it finds in importDir as X and Y
         * flat-field correction tables. See FlatFieldPlugin::importFile() for
         * file format details.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] importDir Directory path where all correction tables are.
         * @param[in] bufSize Transformation buffer size.
         */
        FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importFilePath, int bufSize);

        /**
         * Destructor deinitializes members.
         */
        ~FlatFieldPlugin();

        /**
         * Handle writing plugin integer parameters from PV.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Handle writing plugin double parameters from PV.
         */
        asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

        /**
         * Handle writing array of Int8
         */
        asynStatus writeInt8Array(asynUser *pasynUser, epicsInt8 *values, size_t nElements);

        /**
         * Handle reading array of Int8
         */
        asynStatus readInt8Array(asynUser *pasynUser, epicsInt8 *values, size_t nElements, size_t *nIn);

        /**
         * Handle reading octets
         */
        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

        /**
         * Overloaded function to process incoming OCC packets.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

        /**
         * Reports all import errors when asynReport() is called.
         */
        void report(FILE *fp, int details);

    private:
        /**
         * Transform source X,Y packet into tof,pixid output packet.
         *
         * normal data only
         * no locking required, only class variable is m_tables which is const
         */
        TransformErrors transformPacket(const DasPacket *srcPacket, DasPacket *destPacket);

        /**
         * Correct single event X,Y positions and convert it to pixel id.
         *
         * Each position is adjusted using the corresponding correction table.
         * Pixel id is calculated and returned. It's filling in lower 18 bits.
         * Correction tables up to size 512x512 are supported.
         *
         * @param[in] x Calculated position X, in range [0.0 .. X table size)
         * @param[in] y Calculated position Y, in range [0.0 .. Y table size)
         * @param[in] position Detector position id to find corresponding correction tables.
         * @return Calculated pixel id, 0 when X,Y out of range or no table found.
         */
        uint32_t xyToPixel(double x, double y, uint32_t position);

        /**
         * Determine whether the X,Y position is within photo sum limits.
         *
         * For now only uses X photosum table, according to Miljko both X and Y
         * should be used.
         *
         * @param[in] x Calculate position X, in range [0.0 .. X table size)
         * @param[in] y Calculate position Y, in range [0.0 .. X table size)
         * @param[in] photosum_x Photo sum X value
         * @param[in] position Detector position id to find corresponding correction tables.
         * @return true when X,Y position is within photosum checks, false otherwise
         */
        bool checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position);

        /**
         * Try to import all files in given directory.
         *
         * Walk through all files in the specified directory and try to import
         * them. All errors are printed to the log.
         *
         * Several checks are performed to ensure tables integrity before table is
         * accepted:
         * - file must be parsed correctly
         * - all tables must be of the same size
         * - all detected positions mut have the same set of files
         *   - X and Y correction if at least one detector has those
         *   - lower and upper X photosum limits if at least one detector has those
         *
         * All tables are kept in m_tables vector. std::vector was used
         * for performance reasons, as up to 4 tables needs to be found for
         * each event processed. Configured positions should start at 0 and
         * be contigues but that is not the requirement.
         *
         * @note Should work on WIN32 as well, but not tested.
         * @param[in] dir Relative or absolute path to a directory with
         *                correction table files.
         */
        void importFiles(const std::string &dir);

    private: // variables
        int m_tablesErr;            //!< Flag whether all required tables are ready
        int m_psEn;                 //!< Switch to toggle photosum elimination process
        int m_corrEn;               //!< Switch to toggle applying flat field correction
        int m_convEn;               //!< Switch to toggle converting data to pixel id format
        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        uint32_t m_tableSizeX;      //!< X dimension size of all tables
        uint32_t m_tableSizeY;      //!< Y dimension size of all tables
        std::vector<PositionTables> m_tables;    //!< Array of lookup tables, detector position is index
        std::ostringstream m_reportText; //!< Text to be printed when asynReport() is called

        // Following member variables must be carefully set since they're used un-locked
        double m_xScale;            //!< Scaling factor to transform raw X range to [0 .. m_tableSizeX)
        double m_yScale;            //!< Scaling factor to transform raw Y range to [0 .. m_tableSizeY)
        double m_psScale;           //!< Scaling factor to convert unsigned UQm.n 32 bit value into double
        double m_psLowDecBase;      //!< PhotoSum lower decrement base
        double m_psLowDecLim;       //!< PhotoSum lower decrement limit
        int m_ffMode;               //!< Mode of operation

    protected:
        #define FIRST_FLATFIELDPLUGIN_PARAM ImportReport
        int ImportReport;   //!< Generate textual file import report
        int ImportDir;      //!< Absolute path to pixel map file
        int BufferSize;     //!< Size of allocated buffer, 0 means alocation error
        int Positions;      //!< Array of configured positions
        int TablesErr;      //!< At least one table was not imported properly, check for reason with asynReport()
        int PsEn;           //!< Switch to toggle photosum elimination
        int CorrEn;         //!< Switch to toggle applying flat field correction
        int ConvEn;         //!< Switch to toggle converting data to pixel id format
        int CntUnmap;       //!< Number of unmapped pixels
        int CntError;       //!< Number of generic error pixel ids detected
        int CntPhotoSum;    //!< Number of photo sum eliminated pixels
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int FfMode;         //!< Flat-field transformation mode (see FlatFieldPlugin::FfMode_t)
        int XyFractWidth;   //!< X,Y is in UQm.n format, n is fraction width
        int XRange;         //!< Maximum X values from detector
        int YRange;         //!< Maximum Y values from detector
        int PsFractWidth;   //!< Photo sum is in UQm.n format, n is fraction width
        int PsLowDecBase;   //!< PhotoSum lower decrement base
        int PsLowDecLim;    //!< PhotoSum lower decrement limit
        #define LAST_FLATFIELDPLUGIN_PARAM PsLowDecLim
};

#endif // FLAT_FIELD_PLUGIN_H
