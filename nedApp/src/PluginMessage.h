/* PluginMessage.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PLUGIN_MESSAGE_H
#define PLUGIN_MESSAGE_H

#include "Packet.h"
#include "DasPacket.h"

#include <epicsEvent.h>
#include <epicsMutex.h>
#include <vector>
#include <string>

/**
 * Base inter-plugin communication message.
 */
class PluginMessage
{
    public:
        /**
         * Constructor initializes internal structures.
         */
        PluginMessage();

        /**
         * Destructor, asserts when refcount != 0.
         */
        virtual ~PluginMessage();

        /**
         * Increase internal reference count.
         *
         * After making the reservation, data pointed to by this class
         * is guaranteed not to change by any plugin until a release() is called.
         */
        void claim();

        /**
         * Decrease internal reference count.
         *
         * Every call to reserve() must be followed by a call to this
         * function. In addition, reset() sets reference count to 1.
         *
         * After the client gets finished with processing DAS packets
         * described by this list, it should release the data by calling
         * this function.
         */
        void release();

        /**
         * Wait for all consumers to release the object.
         *
         * After the function returns, reference counter is guaranteed to be 0 and
         * object can not be claim()ed again.
         */
        void waitAllReleased();

        /**
         * Return true if message is not used by anybody, false otherwise.
         */
        bool released();

    private:
        unsigned long m_refcount;
        mutable epicsMutex m_lock;
        mutable epicsEvent m_event;
};

class ParamsExch : public PluginMessage {
    public:
        std::string portName;
        std::string paramName;
};

class DasPacketList : public PluginMessage, public std::vector<DasPacket*> {};
class DasCmdPacketList : public PluginMessage, public std::vector<DasCmdPacket*> {};
class DasRtdlPacketList : public PluginMessage, public std::vector<DasRtdlPacket*> {};

#endif // PLUGIN_MESSAGE_H
