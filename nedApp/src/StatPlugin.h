/* StatPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef STAT_PLUGIN_H
#define STAT_PLUGIN_H

#include "BasePlugin.h"

/**
 * Gather and present statistical information of the incoming data
 */
class StatPlugin : public BasePlugin {
    public: // functions
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        StatPlugin(const char *portName, const char *dispatcherPortName, int blocking);

        /**
         * Overloaded function to receive all OCC data.
         */
        void processData(const DasPacketList * const packetList);

    private:
        uint64_t m_receivedCount;
        uint64_t m_receivedBytes;
        uint64_t m_cmdCount;
        uint64_t m_cmdBytes;
        uint64_t m_dataCount;
        uint64_t m_dataBytes;
        uint64_t m_metaCount;
        uint64_t m_metaBytes;
        uint64_t m_rtdlCount;
        uint64_t m_rtdlBytes;
        uint64_t m_tsyncCount;
        uint64_t m_tsyncBytes;
        uint64_t m_badCount;
        uint64_t m_badBytes;

    private: // asyn parameters
        #define FIRST_STATPLUGIN_PARAM Reset
        int Reset;              //!< Reset counters
        int CmdCnt;             //!< Number of command response packets
        int DataCnt;            //!< Number of data packets
        int MetaCnt;            //!< Number of data packets
        int RtdlCnt;            //!< Number of RTDL packets
        int TsyncCnt;           //!< Number of TSYNC packets
        int BadCnt;             //!< Number of bad packets
        int TotCnt;             //!< Total number of packets
        int CmdByte;            //!< Bytes of command response packets
        int DataByte;           //!< Bytes of data packets
        int MetaByte;           //!< Bytes of data packets
        int RtdlByte;           //!< Bytes of RTDL packets
        int TsyncByte;          //!< Bytes of TSYNC packets
        int BadByte;            //!< Bytes of bad packets
        int TotByte;            //!< Total number of packets
        #define LAST_STATPLUGIN_PARAM TotByte
};

#endif // STAT_PLUGIN_H
