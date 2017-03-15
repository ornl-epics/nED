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
#include <map>
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
            unsigned nTables;                       //!< Number of loaded tables, when zero position is sparse and should be skipped
            bool enabled;                           //!< User has enabled this position - may not have other field
            uint32_t position_id;                   //!< Position human friendly id
            std::shared_ptr<FlatFieldTable> corrX;  //!< Pointer to X correction table
            std::shared_ptr<FlatFieldTable> corrY;  //!< Pointer to Y correction table
            std::shared_ptr<FlatFieldTable> psLowX; //!< Pointer to lower X photosum table
            std::shared_ptr<FlatFieldTable> psUpX;  //!< Pointer to upper X photosum table
        };

        /**
         * Event veto qualifier.
         */
        typedef enum {
            VETO_NO,                 //!< No veto
            VETO_POSITION,           //!< Invalid/unconfigured position
            VETO_RANGE,              //!< X,Y out of range
            VETO_POSITION_CFG,       //!< Position configuration error - overlaps pixel id
            VETO_PHOTOSUM,           //!< Photosum range
        } VetoType;

        /**
         * Import status enumeration.
         */
        enum {
            IMPORT_STATUS_NONE,     //!< No imports yet
            IMPORT_STATUS_ERROR,    //!< Import failed, using previous import if available
            IMPORT_STATUS_BUSY,     //!< Import in progress
            IMPORT_STATUS_DONE,     //!< Import succesfully done
        };

        /**
         * Structure used for returning event counters from processPacket() function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class EventCounters {
            public:
                int32_t nTotal;         //!< Total number of events in packet
                int32_t nGood;          //!< Number of good events in packet
                int32_t nPosition;      //!< Number of bad position - vetoed
                int32_t nRange;         //!< Number of bad X,Y range - vetoed
                int32_t nPosCfg;        //!< Number of position overlap - vetoed
                int32_t nPhotosum;      //!< Number of photosum discriminated - vetoed

                EventCounters()
                {
                    reset();
                }

                EventCounters &operator+=(const EventCounters &rhs)
                {
                    nTotal += rhs.nTotal;
                    nGood += rhs.nGood;
                    nPosition += rhs.nPosition;
                    nRange += rhs.nRange;
                    nPosCfg += rhs.nPosCfg;
                    nPhotosum += rhs.nPhotosum;
                    if (nTotal > std::numeric_limits<int32_t>::max()) {
                        nTotal = nGood = nPosition = nRange = nPosCfg = nPhotosum = 0;
                    }
                    return *this;
                }

                void reset()
                {
                    nTotal = nGood = nPosition = nRange = nPosCfg = nPhotosum = 0;
                }
        };

    public: // structures and defines
        /**
         * Constructor for FlatFieldPlugin
         *
         * Constructor will create and populate PVs with default values.
         * Based on coma delimited list of positions, it will also create 4 PVs
         * per position. Note that unless position is specified in constructor, any
         * event from that position will be rejected.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] positions Coma delimited list of position ids (not pixel offsets)
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
         * Handle writing octets
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

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
         * Process single packet, potentially thread-safe.
         *
         * Events from srcPacket are processed one by one and good events are
         * copied to destPacket. The three input parameters define the
         * processing done on each event.
         *
         * Events from srcPacket are expected to be in format compatible with
         * ACPC normal data. Raw X and Y are in Qn.m where m is defined through
         * XyFractWidth EPICS parameter. Raw values are transformed into
         * real X,Y double values and then scaled to match correction table
         * sizes. The integral part of such calculated values is used as an
         * index into all correction tables. Flat field correction and
         * photosum elimination is done using these.
         *
         * When convert mode is specified, double X,Y values must be down
         * scaled to fit into the requested number of bits (usually 8 or 9, max 10).
         * After scaling, the value is rounded to closest integer value.
         *
         * @param[in] srcPacket Original packet to be processed
         * @param[out] destPacket output packet with all events processed.
         * @param[in] xyDivider used to convert Qm.n unsigned -> double
         * @param[in] correct Toggle applying flat field correction
         * @param[in] photosum Toggle checking photosum
         * @param[in] convert Convert from native normal to common normal event
         * @param[out] counters tell how many events were good or rejected and why
         */
        void processPacket(const DasPacket *srcPacket, DasPacket *destPacket, bool correct, bool photosum, bool convert, EventCounters &counters);

        /**
         * Apply flat field correction on X,Y event
         *
         * Use X and Y correction tables to adjust x and y parameters.
         *
         * @param[in] x value to be corrected, in range [0.0 .. m_tableSizeX)
         * @param[in] y value to be corrected, in range [0.0 .. m_tableSizeY)
         * @param[in] position Detector position id to find corresponding correction tables.
         * @return VetoType
         */
        VetoType correctPosition(double &x, double &y, uint32_t position);

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
         * @return VetoType
         */
        VetoType checkPhotoSumLimits(double x, double y, double photosum_x, uint32_t position);

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
         * @return true when folder was found and some files may be imported,
         *         false when folder does not exist.
         */
        void importFiles(const std::string &dir);

        /**
         * Callback function invoked from a thread to prevent blocking other plugins.
         */
        float importFilesCb(const std::string &dir);

        /**
         * Generate printable report of loaded positions
         *
         * Walks through m_tables table and lists all known positions.
         * Position is sparse when no files have been loaded for that position
         * and user hasn't enabled it.
         *
         * @param[in] psEn toggles printing photosum table information
         * @param[in] corrEn toggles printing flat field correction table information
         * @return printable text
         */
        std::string generatePositionsReport(bool psEn, bool corrEn);

    private: // variables
        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        uint32_t m_tableSizeX;      //!< X dimension size of all tables
        uint32_t m_tableSizeY;      //!< Y dimension size of all tables
        std::map<uint32_t, PositionTables> m_tables; //!< Map of lookup tables/number of detectors is usually small so hashing should be somewhat equally fast as vector, index is pixel_offset
        std::string m_importReport; //!< Text to be printed when asynReport() is called
        std::shared_ptr<Timer> m_importTimer; //!< Timer is used as a worker thread for importing files

        // Following member variables must be carefully set since they're used un-locked
        double m_xScaleIn;          //!< Scaling factor to transform raw X range to [0 .. m_tableSizeX)
        double m_yScaleIn;          //!< Scaling factor to transform raw Y range to [0 .. m_tableSizeY)
        double m_xScaleOut;         //!< Scaling factor to convert X to pixel id format
        double m_yScaleOut;         //!< Scaling factor to convert Y to pixel id format
        uint32_t m_xMaskOut;        //!< Mask to be applied to X when converting to pixel id format
        uint32_t m_yMaskOut;        //!< Mask to be applied to Y when converting to pixel id format
        double m_psScale;           //!< Scaling factor to convert unsigned UQm.n 32 bit value into double
        EventCounters m_evCounters; //!< Global event counters

    protected:
        #define FIRST_FLATFIELDPLUGIN_PARAM ImportReport
        int ImportReport;   //!< Generate textual file import report
        int ImportStatus;   //!< Import status
        int ImportDir;      //!< Absolute path to pixel map file
        int BufferSize;     //!< Size of allocated buffer, 0 means alocation error
        int PsEn;           //!< Switch to toggle photosum elimination
        int CorrEn;         //!< Switch to toggle applying flat field correction
        int ConvEn;         //!< Switch to toggle converting data to pixel id format
        int CntGoodEvents;  //!< Number of calculated events
        int CntPosVetos;    //!< Number of bad position vetos
        int CntRangeVetos;  //!< Number of bad X,Y range vetos
        int CntPosCfgVetos; //!< Number of position config vetos
        int CntPsVetos;     //!< Number of photosum discriminated events
        int CntSplit;       //!< Total number of splited incoming packet lists
        int ResetCnt;       //!< Reset counters
        int PsFractWidth;   //!< Photo sum is in UQm.n format, n is fraction width
        int XyFractWidth;   //!< X,Y is in UQm.n format, n is fraction width
        int XMaxIn;         //!< Maximum X values from detector
        int YMaxIn;         //!< Maximum Y values from detector
        int XMaxOut;        //!< Maximum X values when converted to pixel id format
        int YMaxOut;        //!< Maximum Y values when converted to pixel id format
        int TablesSizeX;    //!< All tables size X
        int TablesSizeY;    //!< All tables size Y
        #define LAST_FLATFIELDPLUGIN_PARAM TablesSizeY

        std::map<uint32_t, int> PosEnable;
        std::map<uint32_t, int> PosId;
        std::map<uint32_t, int> PosCorrX;
        std::map<uint32_t, int> PosCorrY;
        std::map<uint32_t, int> PosPsUpX;
        std::map<uint32_t, int> PosPsLowX;
};

#endif // FLAT_FIELD_PLUGIN_H
