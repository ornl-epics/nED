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
 * mapping table. This plugin reads such table from pixel map file and maps
 * neutron pixel ids according to the table or flags the event as veto.
 * Non neutron events are passed through intact.
 */
class PixelMapPlugin : public BasePlugin {
    private:
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
         * @param[in] parentPlugins is a comma separated list of plugins to connect to
         * @param[in] pixelMapFile Input file to read mapping from
         */
        PixelMapPlugin(const char *portName, const char *parentPlugins, const char *pixelMapFile);

        /**
         * Overloaded function.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

        /**
         * Overloaded function to receive data packets.
         */
        void recvDownstream(const DasDataPacketList &packets) override;

    private: // functions
        /**
         * Maps neutron pixels according to mapping table.
         *
         * Neutron pixels not in mapping table are vetoed. Non-neutron
         * events are left intact. Since this function is templated, 
         * it works with any Event type.
         *
         * Modifies events array in place. It's expected that array
         * was copied from original packet elsewhere, hopefully more
         * efficiently than field-by-field assignment.
         *
         * For diagnostic type of events, this function will copy original
         * pixelid to pixelid_raw field and modify pixelid in place.
         * For Event::Pixel events it will only modify pixelid in place.
         *
         * @param[in] srcEvents Original events with pixelid field
         * @param[out] destEvents Events to be modified
         * @param[in] nEvents number of events in each array.
         * @return Number of unmapped pixel ids.
         */
        template <typename T>
        uint32_t eventsMap(T *events, uint32_t nEvents);

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
        int ResetCnt;       //!< Reset counters
        int MapEn;          //!< Toggle pixel mapping
};

#endif // PIXEL_MAP_PLUGIN_H
