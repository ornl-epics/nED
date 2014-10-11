/* BaseDispatcherPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASE_DISPATCHER_PLUGIN_H
#define BASE_DISPATCHER_PLUGIN_H

#include "BasePlugin.h"
#include "StateMachine.h"
#include "Timer.h"

#include <map>

#define SM_ACTION_CMD(a)        (a)
#define SM_ACTION_ACK(a)        ((a) | (0x1 << 16))
#define SM_ACTION_ERR(a)        ((a) | (0x1 << 17))
#define SM_ACTION_TIMEOUT(a)    ((a) | (0x1 << 18))

/**
 * Base dispatcher class.
 *
 * A dispatcher plugin is the one that receives some data from OCC and forwards
 * it to other plugins that are subscribed to the dispatcher. In the process it
 * may filter out some packets or it may transform packets. In either case it
 * must create new DasPacketList list to keep reference counting of the original
 * script intact. New list can point to the same data when packets are not
 * modified. Otherwise different memory must be used.
 *
 * Sending data to plugins locks the plugins which will lock themselves. Thus
 * dispatcher plugin must not hold its own lock while sending data to plugins.
 */
class BaseDispatcherPlugin : public BasePlugin {
    public: // functions

        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynGenericPointerMask;
        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynGenericPointerMask;

        /**
         * Constructor for BaseDispatcherPlugin
         *
         * @param[in] portName asyn port name.
         * @param[in] remotePortName Name of the asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        BaseDispatcherPlugin(const char *portName, const char *dispatcherPortName,
                             int blocking=0, int numParams=0,
                             int interfaceMask=BaseDispatcherPlugin::defaultInterfaceMask,
                             int interruptMask=BaseDispatcherPlugin::defaultInterruptMask);

        /**
         * Abstract destructor
         */
        virtual ~BaseDispatcherPlugin() = 0;

        /**
         * Receive packet from plugins and forward it to remote port.
         *
         * Plugins registered to this port will indirectly use this function to
         * send packet to the OCC.
         */
        asynStatus writeGenericPointer(asynUser *pasynUser, void *pointer);

        /**
         * Send a list of packets to registered plugins.
         *
         * Plugin must not be locked when calling this function. Plugins callbacks
         * are invoked in context of this thread if they're not blocking. Function
         * returns \b after all plugins completed their processing.
         *
         * Input packet list is assumed to have single reference count. Function
         * releases the single reference counter before it returns.
         *
         * @param packetList List of packet to be sent to plugins
         */
        virtual void sendToPlugins(DasPacketList *packetList);

        /**
         * Overloaded incoming data handler.
         *
         * Only unlocked method is provided to prevent dead-lock when calling
         * sendToPlugins. Developer is responsible for locking the plugin.
         *
         * @param[in] packets List of packets received in this batch.
         */
        virtual void processDataUnlocked(const DasPacketList * const packets) {}

    private:
        // Prevent overloading locked version
        void processData(const DasPacketList *) {}
};

#endif // BASE_DISPATCHER_PLUGIN_H
