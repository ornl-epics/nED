/* RtdlPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef RTDL_PLUGIN_H
#define RTDL_PLUGIN_H

#include "BasePlugin.h"

/**
 * Gather and present statistical information of the incoming data
 */
class RtdlPlugin : public BasePlugin {
    public: // functions
        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        RtdlPlugin(const char *portName, const char *dispatcherPortName, int blocking);

        /**
         * Overloaded function to receive all OCC data.
         */
        void processData(const DasPacketList * const packetList);

    private:
        uint64_t m_receivedCount;
        uint64_t m_processedCount;
        epicsTimeStamp m_lastRtdlTime;  //!< Time from last processed RTDL packet

    private: // asyn parameters
        #define FIRST_RTDLPLUGIN_PARAM Timestamp
        int Timestamp;
        int BadPulse;
        int PulseFlavor;
        int PulseCharge;
        int BadVetoFrame;
        int BadCycleFrame;
        int Tstat;
        int PrevCycleVeto;
        int Cycle;
        int IntraPulseTime;
        int TofFullOffset;
        int FrameOffset;
        int TofFixOffset;
        int RingPeriod;
        #define LAST_RTDLPLUGIN_PARAM RingPeriod
};

#endif // RTDL_PLUGIN_H
