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

#include "BaseDispatcherPlugin.h"
#include "DasPacketList.h"

#include <limits>
#include <vector>

/**
 * PixelMap plugin transforms raw pixel ids according to static mapping
 *
 * Sometimes detectors pixel ids must be transformed according to some static
 * mapping table. This plugins reads such table from pixel map file and corrects
 * all pixel ids in normal event data when (not raw or extended).
 *
 * In addition to Enable switch provided by BasePlugin, PixelMap also provides
 * PassThru switch. When enabled, data will be passed thru intact.
 *
 * Internally the plugin pre-allocates a buffer when constructed. When processing
 * received packets from OCC, the received batch might be bigger than the
 * internal buffer. In such case the batch is split and packets processed so
 * far are sent to connected plugins. The SplitCount parameter is increased for
 * every split. Splitting batches has performance impact, ideally buffer is big
 * enough and no splits occur.
 *
 * Available PixelMapPlugin parameters (in addition to ones from BasePlugin):
 * asyn param    | asyn param type | init val | mode | Description                   |
 * ------------- | --------------- | -------- | ---- | ------------------------------
 * MapErr        | asynParamInt32  | 0        | RO   | Last mapping error
 * PassThru      | asynParamInt32  | 0        | RW   | Skip remapping pixels
 * UnmapCount    | asynParamInt32  | 0        | RO   | Number of unmapped pixels
 * SplitCount    | asynParamInt32  | 0        | RO   | Number of splits occurred
 */
class PixelMapPlugin : public BaseDispatcherPlugin {
    private:
        /**
         * Structure used for returning error counters from pixel mapping function.
         * It's easier to parallelize and more effiecient than passing arguments
         * by reference.
         */
        class PixelMapErrors {
            public:
                int32_t nErrBound;      //!< Pixel id has error bit set and the value could not be mapped
                int32_t nErrOther;      //!< Pixel id has error bit set but could be mapped otherwise
                int32_t nUnmapped;      //!< No mapping was found and pixel id error bit was set

                PixelMapErrors()
                    : nErrBound(0)
                    , nErrOther(0)
                    , nUnmapped(0)
                {}

                PixelMapErrors &operator+=(const PixelMapErrors &rhs)
                {
                    // Don't care about overflow here
                    nErrBound += rhs.nErrBound;
                    nErrOther += rhs.nErrOther;
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

    public: // functions
        /**
         * Constructor
         *
         * Initialize mapping tables from file.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pixelMapFile Input file to read mapping from
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        PixelMapPlugin(const char *portName, const char *dispatcherPortName, int blocking, const char *pixelMapFile, int bufSize);

        /**
         * Destructor
         *
         * Closes the dump file to make sure unsynced data gets flushed to filesystem.
         */
        ~PixelMapPlugin();

        /**
         * Overloaded function to receive all OCC data.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

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
         * @return Number of unmapped pixel ids.
         */
        PixelMapErrors packetMap(const DasPacket *srcPacket, DasPacket *destPacket);

        /**
         * Read mapping table from a file.
         *
         * @param[in] filepath Full path to 3-column text file, one pixel id mapping per line
         * @return 0 on success or error code otherwise.
         */
        ImportError importPixelMapFile(const char *filepath);

    private:
        uint8_t *m_buffer;          //!< Buffer used to copy OCC data into, modify it and send it on to plugins
        uint32_t m_bufferSize;      //!< Size of buffer
        DasPacketList m_packetList; //!< Local list of packets that plugin populates and sends to connected plugins

        std::vector<uint32_t> m_map; //!< Pixel mapping, index is raw pixel id, value is translated pixel id

    private: // asyn parameters
        #define FIRST_PIXELMAPPLUGIN_PARAM MapErr
        int MapErr;         //!< Mapping error (see PixelMapPlugin::ImportError)
        int PassThru;       //!< Should the plugin do the pixel map conversion
        int CntUnmap;       //!< Number of unmapped pixels
        int CntErrOthr;     //!< Number of generic error pixel ids detected
        int CntErrBnd;      //!< Number of error pixel ids outside range detected
        int SplitCount;     //!< Total number of splited incoming packet lists
        #define LAST_PIXELMAPPLUGIN_PARAM SplitCount
};

#endif // PIXEL_MAP_PLUGIN_H
