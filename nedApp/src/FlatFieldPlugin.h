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

#include <limits>

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
         * Single correction table, need one for X and one fo Y
         */
        typedef std::vector< std::vector<double> > CorrTable_t;

        /**
         * Type of the table being imported, also index in m_corrTables.
         */
        typedef enum {
            TABLE_X_CORRECTION      = 0,
            TABLE_Y_CORRECTION      = 1,
            TABLE_X_PHOTOSUM_LOWER  = 2,
            TABLE_X_PHOTOSUM_UPPER  = 3,
            TABLE_UNKNOWN, // This element must be the last one
        } TableType_t;

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
         * Handle reading octet (string) data from plugin.
         */
        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

        /**
         * Overloaded function to process incoming OCC packets.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

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
         * @param[in] position_id Detector position id to find corresponding correction tables.
         * @return Calculated pixel id, 0 when X,Y out of range or no table found.
         */
        uint32_t xyToPixel(double x, double y, uint32_t position_id);

        /**
         * Determine whether the X,Y position is within photo sum limits.
         *
         * For now only uses X photosum table, according to Miljko both X and Y
         * should be used.
         *
         * @param[in] x Calculate position X, in range [0.0 .. X table size)
         * @param[in] y Calculate position Y, in range [0.0 .. X table size)
         * @param[in] photosum_x Photo sum X value
         * @param[in] position_id Detector position id to find corresponding correction tables.
         * @return true when X,Y position is within photosum checks, false otherwise
         */
        bool checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position_id);

        /**
         * Parse the flat-field file and populate the tables.
         *
         * All correction tables are single ASCII file. Correction table file
         * starts with mandatory header and optional comments. Header and
         * comment lines start with character '#'. Header lines are key,value
         * tupples separated by ':'. Unknown header lines are treated as
         * comment. Valid header keys are:
         * - Format version
         * - Table size
         * - Table dimension
         * - Position id
         *
         * ASCII file has several advantages over binary file. It's cross
         * platform independent, it's easy to read and debug, it can be saved
         * to version control repository more efficiently due to compression.
         *
         * Example file:
         * @code {.txt}
         * # FlatField correction table: X correction
         * # File created on 2015-02-09 09:28:06
         * # Format version: 1
         * # Table size: 3x3
         * # Table dimension: X
         * # Position id: 17
         *
         * -0.12 +6.81 +3.55
         * +0.44 +5.09 +12.11
         * +0.79 -9.10 -3.61
         * @endcode
         *
         * @param[in] path Relative or absolute path to a file to parse.
         * @param[out] tableSize Both table dimensions must match this one,
         *                       initialized to size_x if 0.
         * @retval MAP_ERR_NO_FILE No such file or file can not be opened.
         * @retval MAP_ERR_BAD_FORMAT File doesn't look like valid correction
         *         table file.
         * @retval MAP_ERR_PARSE File seems to be valid correction table but
         *         there was problem parsing it.
         * @retval MAP_ERR_EXIST Data for this position already imported.
         * @retval MAP_ERR_NONE No error, file was successfully imported.
         */
        ImportError importFile(const std::string &path, uint32_t &tableSize);

        /**
         * Try to import all files in given directory.
         *
         * Function calls importFile() function for every file it finds in
         * specified directory. Member variables m_corrTables is populated
         * with one pair of X,Y correction tables per detector (position).
         * In case of any error m_corrTables is emptied.
         *
         * It also populates m_tablesSize variable which represents a single
         * dimension size for any given correction table.
         *
         * @note Should work on WIN32 as well, but not tested.
         * @param[in] dir Relative or absolute path to a directory with
         *                correction table files.
         * @retval FF_ERR_NONE All files imported succesfully.
         * @retval FF_ERR_NO_FILE No files found to import.
         * @retval FF_ERR_PARSE Failed to parse some files.
         */
        ImportError importFiles(const std::string &dir);

        /**
         * Parse single header from the current file position.
         *
         * @param[in] infile Opened file
         * @param[out] key Text up to the first ':' character in line
         * @param[out] value Text after the first ':' character in line
         * @return True when parsed, false otherwise.
         */
        bool parseHeader(std::ifstream &infile, std::string &key, std::string &value);

        /**
         * Parse all headers from the current file position.
         *
         * Process all lines starting with # character and tries to parse them
         * to obtain required fields. Blank lines are truncated. Header line
         * can be of any. Function succeeds only if it finds all required
         * headers and parses them. Required header lines:
         * - Format version: <ver>
         * - Table size: <X>x<Y>
         * - Table type: Correction <x or y>
         * - Position id: <id>
         *
         * @param[in] infile Opened file to parse headers from.
         * @param[out] size_x X dimension size
         * @param[out] size_y Y dimension size
         * @param[out] position_id Camera position id
         * @param[out] type Type of imported table
         * @param[in] path File path name used for error reporting
         * @retval MAP_ERR_BAD_FORMAT Unrecognized file format
         * @retval MAP_ERR_PARSE File seems to be expected format but parse error.
         * @retval MAP_ERR_NONE Header parsed.
         */
        ImportError parseHeaders(std::ifstream &infile, uint32_t &size_x, uint32_t &size_y, uint32_t &position_id, FlatFieldPlugin::TableType_t &type, const std::string &path);

        /**
         * Find correction table.
         *
         * @param[in] position_id Detector position
         * @param[in] type Table type.
         * @return Pointer to table or empty pointer if not found.
         */
        std::shared_ptr<CorrTable_t> findTable(uint32_t position_id, TableType_t type);

    private: // variables
        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        std::vector< std::vector<std::shared_ptr<CorrTable_t> > > m_corrTables; //!< First dimension is detector, second dimension is table type (X,Y, photosum X,Y = 4 tables per detector)
        uint32_t m_tablesSize;       //!< X (or Y) dimension of all tables, all tables must be the same size, X and Y dimension must equal
        std::map<std::string, ImportError> m_filesStatus; //!< Import file status

        // Following member variables must be carefully set since they're used un-locked
        double m_xyScale;           //!< Scaling factor to transform raw X,Y range to [0.0 .. m_tablesSize)
        double m_psScale;           //!< Scaling factor to convert unsigned UQm.n 32 bit value into double
        double m_psLowDecBase;      //!< PhotoSum lower decrement base
        double m_psLowDecLim;       //!< PhotoSum lower decrement limit
        int m_ffMode;               //!< Mode of operation

    protected:
        #define FIRST_FLATFIELDPLUGIN_PARAM ImportReport
        int ImportReport;   //!< Generate textual file import report
        int ImportDir;      //!< Absolute path to pixel map file
        int ErrImport;      //!< Import mapping file error (see PixelMapPlugin::ImportError)
        int CntUnmap;       //!< Number of unmapped pixels
        int CntError;       //!< Number of generic error pixel ids detected
        int CntPhotoSum;    //!< Number of photo sum eliminated pixels
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int FfMode;         //!< Flat-field transformation mode (see FlatFieldPlugin::FfMode_t)
        int XyFractWidth;   //!< X,Y is in UQm.n format, n is fraction width
        int XyRange;        //!< Maximum X and Y values from detector
        int PsFractWidth;   //!< Photo sum is in UQm.n format, n is fraction width
        int PsLowDecBase;   //!< PhotoSum lower decrement base
        int PsLowDecLim;    //!< PhotoSum lower decrement limit
        #define LAST_FLATFIELDPLUGIN_PARAM PsLowDecLim
};

#endif // FLAT_FIELD_PLUGIN_H
