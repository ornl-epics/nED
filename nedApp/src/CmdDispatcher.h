/* CmdDispatcher.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef CMD_DISPATCHER_H
#define CMD_DISPATCHER_H

#include "BaseDispatcherPlugin.h"

/**
 * Plugin for receiving all packets but dispatching only commands to connected plugins.
 *
 * This plugin will register to receive all packets and will forward only those
 * that are packets. Plugin tries to minimize number of callbacks forwarded by
 * grouping successive command packets. It will dispatch the callbacks under the
 * same asyn reason as OccPortDriver does, allowing rest of plugins to choose
 * who to subscribe to.
 *
 * The main purpose of this class is to allow processing of all command packets
 * in one thread, while the rest of the data can be processed in different thread.
 * Plugins handling specific commands can be left non-blocking and while connected
 * to this plugin, their processing will be done independently of rest of data in
 * separate thread.
 *
 * Plugin is always non-blocking, thus each instance creates a processing thread.
 */
class CmdDispatcher : public BaseDispatcherPlugin {
    public:
        /**
         * Constructor
         */
        CmdDispatcher(const char *portName, const char *connectPortName);

    private:
        uint32_t m_nReceived;
        uint32_t m_nProcessed;

        /**
         * Process only command data and send it to connected plugins.
         */
        void processDataUnlocked(const DasPacketList * const packetList);
};

#endif // CMD_DISPATCHER_H
