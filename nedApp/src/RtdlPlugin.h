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
         */
        RtdlPlugin(const char *portName, const char *parentPlugins);

        /**
         * Process downstream RTDL packets
         */
        void recvDownstream(DasRtdlPacketList *packets);

    private:
        epicsTime m_lastRtdlTime;  //!< Time from last processed RTDL packet

    private: // asyn parameters
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
        int ErrorsFutureTime;
        int ErrorsPastTime;
};

#endif // RTDL_PLUGIN_H
