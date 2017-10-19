/* EmptyPulsePlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef EMPTY_PULSE_PLUGIN_H
#define EMPTY_PULSE_PLUGIN_H

#include "BaseDispatcherPlugin.h"
#include "RtdlHeader.h"

#include <list>

/**
 * Empty pulse plugins injects empty neutron packets into stream.
 *
 * Useful with low event rates when a beam pulse might not produce
 * a single neutron event, this plugin makes sure to inject an empty
 * packets with appropriate RTDL information. This is required by
 * ADnED and SMS to do correct counting. While DSP have the same
 * capability and can do it much more efficiently, there's a use
 * case DSP doesn't handle. EQ-SANS is running so called frame-skipping
 * where a 30Hz frame contains information from 2 frames.
 */
class EmptyPulsePlugin : public BaseDispatcherPlugin {
    private: // variables
        int m_fd;           //!< File handle for an opened file, or -1
        bool m_fdIsPipe;    //!< true when opened file is a named pipe

    public: // functions
        /**
         * Constructor
         *
         * Only initialize parameters in constructor, don't open the file.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        EmptyPulsePlugin(const char *portName, const char *dispatcherPortName, int blocking);

        /**
         * Destructor
         *
         * Closes the dump file to make sure unsynced data get flushed to filesystem.
         */
        ~EmptyPulsePlugin();

        /**
         * Overloaded function to receive all OCC data.
         */
        void processDataUnlocked(const DasPacketList * const packetList);

    private: // variables
        /**
         * Describe a single allocated memory block.
         */
        typedef struct {
            DasPacket *packet;          //!< Allocated memory address
            size_t size;                //!< Allocated memory size
            bool inUse;                 //!< Currently used by someone
        } PoolEntry;

        std::list<RtdlHeader> m_rtdls;  //!< Received but not processed FIFO
        uint32_t m_nReceived;           //!< Number of received packets
        uint32_t m_nProcessed;          //!< Number of modified packets
        std::list<PoolEntry> m_pool;    //!< Pool of DasPacket allocated objects
        epicsMutex m_mutex;             //!< Mutex used solely to serialize sending data to plugins

    private: // functions
        DasPacket *allocPacket(const RtdlHeader &rtdl);
        void freePacket(DasPacket *packet);

    private: // asyn parameters
        #define FIRST_EMPTYPULSEPLUGIN_PARAM InjectEn
        int InjectEn;       //!< Enable injecting empty pulse packets
        int RtdlBufSize;    //!< Max size of RTDL queue
        int SkipPulses;     //!< Number of pulses to skip
        int PktPoolSize;    //!< Allocated packets pool size
        #define LAST_EMPTYPULSEPLUGIN_PARAM PktPoolSize
};

#endif // EMPTY_PULSE_PLUGIN_H
