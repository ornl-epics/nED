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
         * @param[in] parentPlugins coma separated list of parent plugins
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        StatPlugin(const char *portName, const char *parentPlugins, int blocking);

        /**
         * Overloaded function to receive DAS 1.0 packets.
         */
        void recvDownstream(DasPacketList *packets);

        /**
         * Overloaded function to receive command packets.
         */
        void recvDownstream(DasCmdPacketList *packets);

        /**
         * Overloaded function to receive RTDL packets.
         */
        void recvDownstream(DasRtdlPacketList *packets);

        /**
         * Overloaded function
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    private: // function

        /**
         * Calculate accumulated proton charge for selected source.
         *
         * @param[out] pcharge Variable to add current proton charge to
         * @param[in] lastPulseTime Timestamp of last proton charge added, used to determine new pulse
         * @param[in] rtdl header from packet
         */
        void accumulatePCharge(epicsTimeStamp &lastPulseTime, const RtdlHeader *rtdl, double &pcharge);

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
        double m_neutronPCharge;
        double m_rtdlPCharge;
        epicsTimeStamp m_neutronPulseTime;
        epicsTimeStamp m_rtdlPulseTime;
        RtdlHeader::PulseFlavor m_pulseType;

    private: // asyn parameters
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
        int NeutronPCharge;     //!< Accumulated neutron proton charge since last reset
        int RtdlPCharge;        //!< Accumulated RTDL (accelerator) proton charge since last reset
        int PulseType;          //!< Select pulse type to collect proton charge for
};

#endif // STAT_PLUGIN_H
