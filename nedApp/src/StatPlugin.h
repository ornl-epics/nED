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
         */
        StatPlugin(const char *portName, const char *parentPlugins);

        /**
         * Overloaded function to receive data packets.
         */
        void recvDownstream(const DasDataPacketList &packets);

        /**
         * Overloaded function to receive command packets.
         */
        void recvDownstream(const DasCmdPacketList &packets);

        /**
         * Overloaded function to receive RTDL packets.
         */
        void recvDownstream(const RtdlPacketList &packets);

        /**
         * Process error packets.
         */
        void recvDownstream(const ErrorPacketList &packets);

    private: // function

        /**
         * Check to see if current timestamp is already in que, otherwise insert it.
         */
        bool isTimestampUnique(const epicsTimeStamp &timestamp, std::list<epicsTime> &que);

    private:
        static const size_t MAX_TIME_QUE_SIZE = 5; //!< Max number of entries in each time-series que
        std::list<epicsTime> m_rtdlTimes;    //!< Que of unique RTDL times
        std::list<epicsTime> m_neutronTimes; //!< Que of unique neutron times
        std::list<epicsTime> m_metaTimes;    //!< Que of unique meta data times

    private: // asyn parameters
        int CmdPkts;
        int CmdBytes;
        int NeutronCnts;
        int NeutronBytes;
        int NeutronTimes;
        int MetaCnts;
        int MetaBytes;
        int MetaTimes;
        int ErrorPkts;
        int RtdlPkts;
        int RtdlBytes;
        int RtdlTimes;
        int TotBytes;
};

#endif // STAT_PLUGIN_H
