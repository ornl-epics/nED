/* PixelMapPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PIXEL_MAP_PLUGIN_H
#define PIXEL_MAP_PLUGIN_H

#include "BasePlugin.h"
#include "ObjectPool.h"

#include <limits>
#include <vector>

/**
 * PixelMap plugin transforms raw pixel ids according to static mapping
 *
 * Sometimes detectors pixel ids must be transformed according to some static
 * mapping table. This plugins reads such table from pixel map file and corrects
 * all pixel ids in normal event data when (not raw or extended).
 *
 * In addition to Enable switch provided by BasePlugin, pass thru mode can be
 * selected using the MapMode parameter with value 0.
 *
 * Internally the plugin pre-allocates a buffer when constructed. When processing
 * received packets from OCC, the received batch might be bigger than the
 * internal buffer. In such case the batch is split and packets processed so
 * far are sent to connected plugins. The SplitCount parameter is increased for
 * every split. Splitting batches has performance impact, ideally buffer is big
 * enough and no splits occur.
 */
class PixelMapPlugin : public BasePlugin {
    private:
        /**
         * Structure used for returning error counters from pixel mapping function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class PixelMapErrors {
            public:
                int32_t nErrors;        //!< Pixel id has error bit set and the value could not be mapped
                int32_t nUnmapped;      //!< No mapping was found and pixel id error bit was set

                PixelMapErrors()
                    : nErrors(0)
                    , nUnmapped(0)
                {}

                PixelMapErrors &operator+=(const PixelMapErrors &rhs)
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
            MAP_ERR_NONE        = 0, //!< No error
            MAP_ERR_NO_FILE     = 1, //!< Failed to find file
            MAP_ERR_PARSE       = 2, //!< Failed to parse file
            MAP_ERR_NO_MEM      = 3, //!< Failed to allocate internal buffer
        } ImportError;

        /**
         * Possible mode for dealing with bad events.
         *
         * Note: it's a bitmask
         */
        typedef enum {
            MAP_NONE            = 0, //!< Don't map any events - passthru
            MAP_GOOD            = 1, //!< Map only good events
            MAP_BAD             = 2, //!< Map only bad events
            MAP_ALL             = 3, //!< Map all events
        } MapMode_t;

    public: // functions
        /**
         * Constructor
         *
         * Initialize mapping tables from file.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins is a comma separated list of plugins to connect to
         * @param[in] pixelMapFile Input file to read mapping from
         */
        PixelMapPlugin(const char *portName, const char *parentPlugins, const char *pixelMapFile);

        /**
         * Overloaded function.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function to receive data packets.
         */
        void recvDownstream(const DasDataPacketList &packets);

    private: // functions
        /**
         * Replicate srcPacket but map pixel ids.
         *
         * Source packet header (including RTDL) is copied to the destination
         * packet. Then events are iterated and each pixel id is mapped according
         * to the pixel map table. If no mapping is found, event is vetoed and
         * not included in destination packet.
         *
         * @param[in] srcPacket Original packet from OCC
         * @param[out] destPacket Copied packet with pixel ids mapped
         * @param[in] passVetoes Should vetoed events be included in dest packet
         * @return Number of unmapped pixel ids.
         */
        PixelMapErrors packetMap(const DasDataPacket *srcPacket, DasDataPacket *destPacket, bool passVetoes);

        /**
         * Read mapping table from a file.
         *
         * @param[in] filepath Full path to 3-column text file, one pixel id mapping per line
         * @return 0 on success or error code otherwise.
         */
        ImportError importPixelMapFile(const char *filepath);

    private:
        std::vector<uint32_t> m_map; //!< Pixel mapping, index is raw pixel id, value is translated pixel id

        ObjectPool<DasDataPacket> m_packetsPool{false}; //!< Pool of packets to be used for modified data

    private: // asyn parameters
        int FilePath;       //!< Absolute path to pixel map file
        int ErrImport;      //!< Import mapping file error (see PixelMapPlugin::ImportError)
        int CntUnmap;       //!< Number of unmapped pixels
        int CntError;       //!< Number of generic error pixel ids detected
        int ResetCnt;       //!< Reset counters
        int MapEn;          //!< Toggle pixel mapping
        int VetoMode;       //!< Select what to do with veto events
};

#endif // PIXEL_MAP_PLUGIN_H
