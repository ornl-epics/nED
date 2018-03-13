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

        /**
         * Save selected RTDL info to cache.
         *
         * Only first packet with unique RTDL data (based on timestamp) is
         * saved to cache.
         * Number of cached RTDLs is controlled through CacheSize PV.
         *
         * @param[in] packet to be saved
         * @return true if data was cached, false otherwise.
         */
        bool cacheRtdl(const RtdlPacket *packet);

        /**
         * Return valid proton charge only once for a given acquisition frame.
         *
         * Based on the timestamp, this function will lookup the proton
         * charge from RTDL and return it only once for each acquisition frame.
         * If called again for the same frame, it returns negative value.
         * Negative value is also returned when proton charge is not yet
         * available. The reason for that might be that proton charge is
         * announced by accelerator only after it has been measured, which
         * is in the next accelerator cycle from where the extraction
         * time was sent. The RTDL is sent few ms later than the extraction
         * which introduces a gap where potential neutrons are detected
         * but the proton charge information is not available yet.
         * More likely events are detected much later possibly even in
         * next accelerator cycle depending on neutron speed and detector
         * distance from sample.
         * To cover both cases, this function needs to be called for
         * event packets and RTDL packets.
         *
         * @param[in] timestamp of the data acquisition frame
         * @return proton charge or -1.0 if previously called for this frame
         *         or no proton charge info yet
         */
        double getDataProtonCharge(const epicsTimeStamp &timestamp);

    private:
        std::list<std::tuple<epicsTime,double,bool>> m_dataPcharge; //!< Cache of data acq frames pcharge
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
        int PChargeRtdl;
        int PChargeData;
        int RtdlCacheSize;
};

#endif // STAT_PLUGIN_H
