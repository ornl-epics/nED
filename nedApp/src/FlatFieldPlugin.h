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
                int32_t nPrevious;  //!< Pixel id has error bit already set
                int32_t nUnmapped;  //!< No correction table was found for given module

                TransformErrors()
                    : nPrevious(0)
                    , nUnmapped(0)
                {}

                TransformErrors &operator+=(const TransformErrors &rhs)
                {
                    // Prevent overflow, stop counting at INT32_MAX instead
                    if (rhs.nPrevious > std::numeric_limits<int32_t>::max() - nPrevious)
                        nPrevious = std::numeric_limits<int32_t>::max();
                    else
                        nPrevious += rhs.nPrevious;
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
            MAP_ERR_NONE        = 0, //!< No error
            MAP_ERR_NO_FILE     = 1, //!< Failed to find file
            MAP_ERR_PARSE       = 2, //!< Failed to parse file
            MAP_ERR_NO_MEM      = 3, //!< Failed to allocate internal buffer
        } ImportError;

        /**
         * Enable or disable mapping mode
         */
        typedef enum {
            MAP_DISABLED        = 0, //!< Don't map any events - passthru
            MAP_ENABLED         = 1, //!< Map all events
        } MapMode_t;

    public: // structures and defines
        /**
         * Constructor for FlatFieldPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] importFilePath Path to a file holding all correction tables
         * @param[in] bufSize Transformation buffer size.
         */
        FlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *importFilePath, int bufSize);

        ~FlatFieldPlugin();

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
         * Correct single event  X,Y positions and convert it to pixel id.
         *
         * no locking required, only class variable is m_tables which is const
         */
        uint32_t xyToPixel(double x, double y, const CorrTablePair_t &tables);

        /**
         * Parse the flat-field file and populate the tables.
         *
         * All correction tables are single ASCII file. Correction table file
         * starts with mandatory header and optional comments. Header and
         * comment lines start with character '#'. Header lines are key,value
         * tupples separated by ':'. Unknown header lines are treated as
         * comment. Valid header keys are:
         * * Format version
         * * Table size
         * * Table dimension
         * * Position id
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
         */
        ImportError importFile(const std::string &path);

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
         * Table size: <X>x<Y>
         * Camera id: <id>
         * Table dimension: <x or y>
         */
        bool parseHeaders(std::ifstream &infile, int &size_x, int &size_y, int &camera_id, char &table_dim);

    private: // variables
        static const unsigned TABLE_X_SIZE = 512;
        static const unsigned TABLE_Y_SIZE = 512;
        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        DasPacketList m_packetList; //!< Local list of packets that plugin populates and sends to connected plugins
        double m_maxRawX;           //!< Maximum value for X returned by camera (cached)
        double m_maxRawY;           //!< Maximum value for Y returned by camera (cached)
        double m_photosumXLow;      //!< X photo sum low threshold
        double m_photosumXHi;       //!< X photo sum high threshold
        double m_photosumYLow;      //!< Y photo sum low threshold
        double m_photosumYHi;       //!< Y photo sum high threshold
        std::map<uint32_t, CorrTablePair_t> m_corrTables; //!< One correction table per ACPC camera.

    protected:
        #define FIRST_FLATFIELDPLUGIN_PARAM FilePath
        int FilePath;       //!< Absolute path to pixel map file
        int ErrImport;      //!< Import mapping file error (see PixelMapPlugin::ImportError)
        int CntUnmap;       //!< Number of unmapped pixels
        int CntError;       //!< Number of generic error pixel ids detected
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int MapMode;        //!< Event mapping mode (see PixelMapPlugin::MapMode_t)
        int MaxRawX;        //!< Maximum value for X returned by camera
        int MaxRawY;        //!< Maximum value for Y returned by camera
        #define LAST_FLATFIELDPLUGIN_PARAM MaxRawY
};

#endif // FLAT_FIELD_PLUGIN_H
