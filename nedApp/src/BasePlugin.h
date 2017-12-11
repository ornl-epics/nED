/* BasePlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PLUGIN_DRIVER_H
#define PLUGIN_DRIVER_H

#include "PluginMessage.h"
#include "EpicsRegister.h"
#include "Thread.h"

#include <stdint.h>
#include <string>
#include <functional>
#include <list>
#include <memory>
#include <asynPortDriver.h>
#include <epicsMessageQueue.h>
#include <epicsThread.h>

class Timer;

typedef std::vector<DasPacket*> DasPacketList;
typedef std::vector<DasCmdPacket*> DasCmdPacketList;
typedef std::vector<DasRtdlPacket*> DasRtdlPacketList;
typedef std::vector<DasDataPacket*> DasDataPacketList;
typedef std::vector<ErrorPacket*> ErrorPacketList;

/**
 * Registers plugin with EPICS system.
 *
 * Each plugin class should call this macro somewhere in the .c/.cpp file. The macro
 * creates a C function called \<plugin name\>Configure and exports it through EPICS
 * to be used from EPICS shell (for example st.cmd).
 *
 * Registering plugin with EPICS from the C code is first step in EPICS registration.
 * User must manually add support for registered class into nedSupport.dbd file. For
 * example, append the following line:
 * registrar("ExamplePluginRegister")
 *
 * There are several macros depending on the number and type of arguments required by
 * the plugin.
 *
 * Example usage for ExamplePlugin(const char *portName, int queueSize) :
 * EPICS_REGISTER_PLUGIN(ExamplePlugin, 2, "Example port", string, "Queue size", int);
 *
 * @param[in] name Name of the plugin class (ie. ExamplePlugin), a static C function
 *            is created called ExamplePluginConfigure which creates new object of
 *            ExamplePlugin whenever called.
 * @param[in] numargs Number of plugin parameters supported by plugin constructor.
 * @param[in] ... Pairs of (argument name, argument type) parameters. For each parameter
 *            there should be exactly one such pair. Supported argument types are
 *            string, int.
 */
#define EPICS_REGISTER_PLUGIN(name, numargs, ...) EPICS_REGISTER(name, name, numargs, __VA_ARGS__)

/**
 * Abstract base plugin class.
 *
 * The class provides basis for all plugins with helper functions for creating
 * parameters and exchange of data with other plugins. Plugin connections are
 * established by child plugins subscribing to parent plugins. The usual data
 * flow is from parent plugin(s) to child plugins and is called downstream
 * flow. Downstream flow can be optimized by creating a thread handling the
 * data - also called blocking mode. The opposite direction is called upstream
 * and is executed in callers' thread.
 *
 * Parameters to be exposed to EPICS are created using asynPortDriver *param
 * mechanism. There's plenty of helper functions in this class to help derived
 * classes with managing those parameters.
 *
 * Plugin instances can be loaded at compile time or at run time. For compile
 * time inclusion simply instantiate a new object of the plugin class somewhere
 * in the code. For runtime loaded plugins, the plugin class implementation must
 * export its functionality through EPICS. New instances can then be created from
 * the EPICS shell. Use #EPICS_REGISTER_PLUGIN macro to export plugin to EPICS.
 */
class BasePlugin : public asynPortDriver {
    public:
        static const int defaultInterfaceMask = asynInt32Mask | asynGenericPointerMask | asynDrvUserMask;
        static const int defaultInterruptMask = asynInt32Mask | asynGenericPointerMask;

        /**
         * Constructor
         *
         * Initialize internal state of the class. This includes calling asynPortDriver
         * constructor, creating and setting default values for class parameters and creating
         * worker thread for received data callbacks if requested by blocking parameter.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPorts Comma separated list of remote ports to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports, this bit mask is added to default one.
         * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks), this bit mask is added to default one.
         * @param[in] queueSize Max number of inter-plugin messages
         * @param[in] asynFlags Flags when creating the asyn port driver; includes ASYN_CANBLOCK and ASYN_MULTIDEVICE.
         * @param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
         * @param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
         */
        BasePlugin(const char *portName, int blocking=0, int interfaceMask=0, int interruptMask=0,
                   int queueSize=5, int asynFlags=0, int priority=0, int stackSize=0);

        /**
         * Destructor.
         */
        virtual ~BasePlugin() = 0;

        /**
         * Connect to one or many parent plugins.
         *
         * For each ports in a list connect to all messages types requested. If succesfull,
         * bi-directional communication with connectes plugins is enabled. If some plugins
         * were previously connected, they're disconnected first. Port must not be locked
         * when calling this function.
         *
         * @param[in] ports to connect to
         * @param[in] messageTypes each number must be one of the integers registers by Msg* parameters.
         * @return true if connected to remote ports, false otherwise.
         */
        bool connect(const std::list<std::string> &ports, const std::list<int> &messageTypes);

        /**
         * Connect to one or many parent plugins.
         *
         * Helper functions accepting comma-separated string of remote ports.
         */
        bool connect(const std::string &ports, const std::list<int> &messageTypes);

        /**
         * Connect to same ports and message type as last time.
         */
        bool connect();

        /**
         * Connect to one or many parent plugins.
         *
         * Helper functions accepting comma-separated string of remote ports.
         */
        bool connect(const std::string &ports, int messageType)
        {
            std::list<int> msgs;
            msgs.push_back(messageType);
            return connect(ports, msgs);
        }

        /**
         * Disconnect from all parent plugins.
         */
        bool disconnect();

        /**
         * Return true if connected to *any* plugin, false when not connected.
         */
        bool isConnected();

        /**
         * Request a custom callback function to be called at some time in the future.
         *
         * Using this function, the plugin can request asynchronous task to be
         * scheduled at some relative time.
         * All tasks are run from a background thread and before they're
         * executed, the plugin thread safety is guaranteed through its lock.
         *
         * When the timer expires, it invokes user defined function with no parameters.
         * User defined function should return delay in seconds when the
         * next invocation should occur, or 0 to stop the timer.
         *
         * @param[in] callback Function to be called after delay expires.
         * @param[in] delay Delay from now when to invoke the function, in seconds.
         * @return active or inactive timer
         */
        std::shared_ptr<Timer> scheduleCallback(std::function<float(void)> &callback, double delay);

        /**
         * A callback function called by asyn upon receiving message from parent plugin.
         *
         * Evaluates generic pointer as PluginMessage. Skips messages that plugin
         * is not subscribed to. Executes recvDownstream() if plugin is in
         * non-blocking mode, otherwise it queues message for receive thread
         * to pick up and execute same function in custom thread.
         *
         * Should be private but is called from C scope and must be public.
         * It's called in the context of parent plugin thread.
         */
        void recvDownstreamCb(asynUser *pasynUser, void *ptr);

        /**
         * A worker function to process messages from parent plugins.
         *
         * This function is the first one called when a new message from parents
         * is received by current plugin. Default implementation simply invokes
         * corresponding function based on type parameter.
         */
        virtual void recvDownstream(int type, PluginMessage *msg);

        /**
         * A worker function to process DasPacket messages from parent plugins.
         */
        virtual void recvDownstream(DasPacketList *packets) {};

        /**
         * A worker function to process DasDataPacket messages from parent plugins.
         */
        virtual void recvDownstream(DasDataPacketList *packets) {};

        /**
         * A worker function to process DasCmdPacket messages from parent plugins.
         */
        virtual void recvDownstream(DasCmdPacketList *packets) {};

        /**
         * A worker function to process DasRtdlPacket messages from parent plugins.
         */
        virtual void recvDownstream(DasRtdlPacketList *packets) {};

        /**
         * Send PluginMessage to any connected child plugins.
         *
         * PluginMessage must be unique and not previously used. Use with care
         * and only when really necessary. Use specialized sendDownstream()
         * interfaces instead.
         *
         * Plugin must be unlocked when sending messages.
         *
         * When in wait mode, the function will return only after all subscribed
         * plugins have received and processed the message.
         *
         * @param[in] type of message to be sent
         * @param[in] msg to be sent
         * @param[in] wait for plugins to process message before returning
         */
        void sendDownstream(int type, std::shared_ptr<PluginMessage> &msg, bool wait=true);

        /**
         * Send DasPackets to any connected child plugins.
         */
        std::shared_ptr<PluginMessage> sendDownstream(DasPacketList *packets, bool wait=true);

        /**
         * Send DasDataPackets to any connected child plugins.
         */
        std::shared_ptr<PluginMessage> sendDownstream(DasDataPacketList *packets, bool wait=true);

        /**
         * Send DasCmdPackets to any connected child plugins.
         */
        std::shared_ptr<PluginMessage> sendDownstream(DasCmdPacketList *packets, bool wait=true);

        /**
         * Send DasRtdlPackets to any connected child plugins.
         */
        std::shared_ptr<PluginMessage> sendDownstream(DasRtdlPacketList *packets, bool wait=true);

        /**
         * Send ErrorPackets to any connected child plugins.
         */
        std::shared_ptr<PluginMessage> sendDownstream(ErrorPacketList *packets, bool wait=true);

        /**
         * A callback function called upon receiving message from child plugin.
         *
         * The default operation is to call function corresponding to supported
         * message type.
         */
        virtual void recvUpstream(int type, PluginMessage *msg);

        /**
         * Receive DasPacket list from children plugins.
         */
        virtual void recvUpstream(DasPacketList *packets) {};

        /**
         * Receive DasCmdPacket list from children plugins.
         */
        virtual void recvUpstream(DasCmdPacketList *packets) {};

        /**
         * Send message to parent plugins.
         *
         * Function find connected parent plugins and sends message only if
         * message type is also subscribed to.
         * Function returns immediately and doesn't wait for receiver to process
         * packets.
         */
        virtual void sendUpstream(int type, std::shared_ptr<PluginMessage> &msg);

        /**
         * Send DasPacketList to parent plugins.
         *
         * Waits until receiver processes the packet.
         */
        virtual void sendUpstream(DasPacketList *packet);

        /**
         * Send single DasPacket to parent plugins.
         *
         * Waits until receiver processes the packet.
         */
        virtual void sendUpstream(DasPacket *packet);

        /**
         * Send single DasCmdPacketList to parent plugins.
         *
         * Waits until receiver processes the packet.
         */
        virtual void sendUpstream(DasCmdPacketList *packet);

        /**
         * Send single DasCmdPacket to parent plugins.
         *
         * Waits until receiver processes the packet.
         */
        virtual void sendUpstream(DasCmdPacket *packet);

        /**
         * Return the name of the asyn parameter.
         *
         * @param[in] index asyn parameter index
         * @return Name of the parameter used when parameter was registered.
         */
        const std::string getParamName(int index);

        /**
         * Helper function to create output asynPortDriver param with initial value set.
         */
        asynStatus createParam(const std::string &name, asynParamType type, int *index, int initValue);

        /**
         * Helper function to create output asynPortDriver param with initial value set.
         */
        asynStatus createParam(const std::string &name, asynParamType type, int *index, double initValue);
        /**
         * Helper function to create output asynPortDriver param with initial value set.
         */
        asynStatus createParam(const std::string &name, asynParamType type, int *index, const std::string &initValue);

        using asynPortDriver::createParam;

        /**
         * Wrapper around getIntegerParam to retrieve parameter as boolean value
         *
         * A positive value is converted to true, false otherwise.
         */
        asynStatus getBooleanParam(int index, bool &value);

        /**
         * Convenient function, return false on error which can be ambigous.
         */
        int getBooleanParam(int index)
        {
            int val;
            if (getIntegerParam(index, &val) == asynSuccess)
                return (val > 0);
            return false;
        }

        /**
         * Send int32 parameter to another plugin.
         *
         * @param[in] remotePort name of the remote plugin.
         * @param[in] paramName name of the parameter
         * @param[in] value of the parameter
         * @return true on success
         */
        bool sendParam(const std::string &remotePort, const std::string &paramName, epicsInt32 value);

        /**
         * Returns the value for an integer from the parameter library.
         *
         * Convenience function to look by parameter name that only works for
         * for first asynPort address plugins.
         *
         * @see asynPortDriver::getIntegerParam(int, int*)
         */
        asynStatus getIntegerParam(const std::string &name, int &value);
        using asynPortDriver::getIntegerParam;

        /**
         * Convenient function, return -1 on error which can be ambigous.
         */
        int getIntegerParam(int index)
        {
            int val;
            if (getIntegerParam(index, &val) == asynSuccess)
                return val;
            return -1;
        }

        /**
         * Convenient function, return -1 on error which can be ambigous.
         */
        int getIntegerParam(const std::string &name)
        {
            int val;
            if (getIntegerParam(name, val) == asynSuccess)
                return val;
            return -1;
        }

        /**
         * Sets new value of an integer in the parameter library.
         *
         * Convenience function to look by parameter name that only works for
         * for first asynPort address plugins.
         *
         * @see asynPortDriver::setIntegerParam(int, int)
         */
        asynStatus setIntegerParam(const std::string &name, int value);
        using asynPortDriver::setIntegerParam;

        /**
         * Convenient function, return -1 on error which can be ambigous.
         */
        std::string getStringParam(int index)
        {
            char buffer[256];
            if (getStringParam(index, sizeof(buffer), buffer) == asynSuccess)
                return std::string(buffer);
            return std::string();
        }
        using asynPortDriver::getStringParam;

        /**
         * Convenience function to increment current parameter value.
         */
        asynStatus addIntegerParam(const std::string &name, int increment);

        /**
         * Convenience function to increment current parameter value.
         */
        asynStatus addIntegerParam(int param, int increment);

        /**
         * Overloaded asynPortDriver function to receive messges from child plugins.
         *
         * To comply with name terminology in this class, the functions is a
         * simple wrapper around cbDownstream().
         */
        asynStatus writeGenericPointer(asynUser *pasynUser, void *pointer);

    private:
        /**
         * Receive threads' main function when in blocking mode.
         *
         * Thread is taking messages from internal queue and executes recvDownstream()
         * function for each message.
         *
         * Runs until the shutdown flag is not set. It monitors message queue
         * and processes every message in receive thread context. Deffers
         * the actual work to processUpstreamMsg() function.
         *
         * @param[out] shutdown Flags when the thread should stop.
         */
        void recvDownstreamThread(epicsEvent *shutdown);

        /**
         * Called from epicsTimer when timer expires.
         */
        float timerExpire(std::shared_ptr<Timer> &timer, std::function<float(void)> callback);

    private:
        /**
         * Structure to describe asyn interface.
         */
        struct RemotePort {
            std::string pluginName;                 //!< Name of connected port
            asynUser *pasynuser;                    //!< asynUser handler for asyn management
            void *asynGenericPointerInterrupt;      //!< Generic pointer interrupt handler
        };

        std::string m_portName;                     //!< Port name
        std::list<RemotePort> m_connectedPorts;     //!< List of connected remote ports.
        epicsMessageQueue m_messageQueue;           //!< Message queue for non-blocking mode
        Thread *m_thread;                           //!< Thread ID if created during constructor, 0 otherwise
        bool m_shutdown;                            //!< Flag to shutdown the thread, used in conjunction with messageQueue wakeup
        std::list<std::shared_ptr<Timer> > m_timers;//!< List of timers currently scheduled
        std::list<std::string> m_lastConnectedPlugins; //!< Names of ports used for last connection
        std::list<int> m_lastConnectedTypes;        //!< Message types used for last connection

    protected:
        int MsgOldDas;
        int MsgError;
        int MsgDasData;
        int MsgDasCmd;
        int MsgDasRtdl;
        int MsgParamExch;
};

#endif // PLUGIN_DRIVER_H
