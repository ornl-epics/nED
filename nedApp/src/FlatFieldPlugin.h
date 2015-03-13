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
 * FlatFieldPlugin produces tof,pixelid stream out of x,y positions produces by some detectors.
 */
class FlatFieldPlugin : public BaseDispatcherPlugin {
    private: // structures & typedefs
        /**
         * Single correction table, need one for X and one fo Y
         */
        typedef std::vector< std::vector<double> > CorrTable_t;

        /**
         * Pair of X,Y correction tables.
         */
        typedef std::pair<CorrTable_t, CorrTable_t> CorrTablePair_t;

        /**
         * Structure used for returning error counters from transform function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class TransformErrors {
            public:
                int32_t nErrors;    //!< Pixel id has error bit already set
                int32_t nUnmapped;  //!< No correction table was found for given module

                TransformErrors()
                    : nErrors(0)
                    , nUnmapped(0)
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
        } ImportError;

        /**
         * Enable or disable mapping mode
         */
        typedef enum {
            FF_DISABLED         = 0, //!< Passthru mode
            FF_ENABLED          = 1, //!< Transform neutron events
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
         * @param[in] x Calculated position X in range [0.0 .. X table size)
         * @param[in] y Calculated position Y in range [0.0 .. Y table size)
         * @param[in] xtable X correction factor table
         * @param[in] ytable Y correction factor table
         * @return Calculated pixel id, 0 when X,Y out of range.
         */
        uint32_t xyToPixel(double x, double y, const CorrTable_t &xtable, const CorrTable_t &ytable);

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
         * - Table dimension: <x or y>
         * - Position id: <id>
         *
         * @param[in] infile Opened file to parse headers from.
         * @param[out] size_x X dimension size
         * @param[out] size_y Y dimension size
         * @param[out] position_id Camera position id
         * @param[out] dimension Correction table dimension, 'X' or 'Y'
         * @retval MAP_ERR_BAD_FORMAT Unrecognized file format
         * @retval MAP_ERR_PARSE File seems to be expected format but parse error.
         * @retval MAP_ERR_NONE Header parsed.
         */
        ImportError parseHeaders(std::ifstream &infile, uint32_t &size_x, uint32_t &size_y, uint32_t &position_id, char &dimension);

    private: // variables
        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        double m_photosumXLow;      //!< X photo sum low threshold
        double m_photosumXHi;       //!< X photo sum high threshold
        double m_photosumYLow;      //!< Y photo sum low threshold
        double m_photosumYHi;       //!< Y photo sum high threshold
        std::map<uint32_t, CorrTablePair_t> m_corrTables; //!< One correction table per ACPC camera.
        uint32_t m_tablesSize;       //!< X (or Y) dimension of all tables, all tables must be the same size, X and Y dimension must equal
        std::map<std::string, ImportError> m_filesStatus; //!< Import file status

        // Following member variables must be carefully used since they're used un-locked
        double m_xyScale;           //!< Scaling factor to transform raw X,Y range to [0.0 .. m_tablesSize)
        double m_psScale;           //!< Scaling factor to transform raw photo sum range to [0.0 .. 512.0)

    protected:
        #define FIRST_FLATFIELDPLUGIN_PARAM ImportReport
        int ImportReport;   //!< Generate textual file import report
        int ImportDir;      //!< Absolute path to pixel map file
        int ErrImport;      //!< Import mapping file error (see PixelMapPlugin::ImportError)
        int CntUnmap;       //!< Number of unmapped pixels
        int CntError;       //!< Number of generic error pixel ids detected
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int FfMode;         //!< Flat-field transformation mode (see FlatFieldPlugin::FfMode_t)
        int XyFractWidth;   //!< X,Y is in UQm.n format, n is fraction width
        int PsFractWidth;   //!< Photo sum is in UQm.n format, n is fraction width
        int XyRange;        //!< Maximum X and Y values from detector
        #define LAST_FLATFIELDPLUGIN_PARAM XyRange
};

#endif // FLAT_FIELD_PLUGIN_H
