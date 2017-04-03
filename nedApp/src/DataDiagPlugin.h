/* DataDiagPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DATA_DIAG_PLUGIN_H
#define DATA_DIAG_PLUGIN_H

#include "BasePlugin.h"
#include "ObjectPool.h"

#include <cstring>
#include <list>

/**
 * Generic module plugin is a tool to test and debug module communication.
 *
 * The plugin is mainly useful for testing. User must first select remote
 * module and command. Writing command triggers sending the packet out and
 * waiting for response. Incoming response from the configured module will
 * immediately populate all PVs.
 *
 * Plugin requires a valid hardware address of the remote module. It does not
 * send out global commands. Format of outgoing packets depends on the link
 * type. Only DSPs are connected through optics, all other modules are
 * connected to DSP through LVDS. When sending a packet through LVDS, extra
 * formatting is necessary. User must select what type of communication to use
 * through ReqIsDsp parameter.
 *
 * User can send any 8 bit OCC command. The plugin does not check command value
 * and thus allows extensibility through EPICS database.
 */
class DataDiagPlugin : public BasePlugin {
    private: // variables
        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynOctetMask;
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynOctetMask;

        typedef struct {
            std::shared_ptr<uint32_t> buffer;
            size_t size;
            size_t errorOffset;
        } ErrorDesc;
        std::list<ErrorDesc> m_errorsQue;
        ObjectPool<uint32_t> m_memoryPool;

        epicsTimeStamp m_lastNeutronTime;
        epicsTimeStamp m_lastMetadataTime;

        uint16_t m_packetSeq;
        bool m_packetSeqInit;

    public: // structures and defines
        /**
         * Constructor for DataDiagPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        DataDiagPlugin(const char *portName, const char *dispatcherPortName, int blocking=0);

        /**
         * Overloaded function to handle reading strings and byte arrays.
         */
        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

        /**
         * Overloaded function to handle writing integers.
         */
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function to process incoming OCC packets.
         */
        virtual void processData(const DasPacketList * const packetList);

    private:
        /**
         * Check whether any TOF and pixel id are a match and return dword offset to TOF.
         */
        uint32_t checkEqualTofPixel(const DasPacket *packet);

        /**
         * Check that time in data packets is incrementing.
         */
        uint32_t checkIncTime(const DasPacket *packet);

        /**
         * Check packet sequence number is monotonically increasing.
         */
        uint32_t checkPacketSeq(const DasPacket *packet);
    protected:
        #define FIRST_DATADIAGPLUGIN_PARAM Data
        int Data;           //!< Read data formatted as hex string
        int CheckEqTofPix;  //!< Enable TOF==pixid check
        int CheckIncTime;   //!< Enable incrementing time check
        int CheckPktSeq;    //!< Enable packet sequence check
        int QueIndex;       //!< Currently display packet index
        int QueSize;        //!< Number of elements in packet buffer
        int QueMaxSize;     //!< Max num of elements in packet buffer
        int Reset;          //!< Clear buffers
        #define LAST_DATADIAGPLUGIN_PARAM Reset
};

#endif // DATA_DIAG_PLUGIN_H
