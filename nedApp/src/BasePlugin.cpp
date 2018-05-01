/* BasePlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "BasePlugin.h"
#include "Log.h"
#include "Timer.h"

#include <algorithm>
#include <epicsThread.h>
#include <string>

#define MESSAGE_QUEUE_SIZE 5   //!< Size of the message queue for callbacks. Increased to 5 to allow multiple parent plugins.

/* Helper C functions for asyn/EPICS registration
 */
extern "C" {
    static void recvDownstreamCb(void *drvPvt, asynUser *pasynUser, void *genericPointer)
    {
        BasePlugin *plugin = reinterpret_cast<BasePlugin *>(drvPvt);
        plugin->recvDownstreamCb(pasynUser, genericPointer);
    }
}

/**
 * Structure used for exchanging parameters.
 */
struct ParamsExch {
    std::string portName;
    std::string paramName;
};


BasePlugin::BasePlugin(const char *portName, int blocking, int interfaceMask, int interruptMask,
                       int queueSize, int asynFlags, int priority, int stackSize)
    : asynPortDriver(portName, /*maxAddr=*/0, interfaceMask | defaultInterfaceMask,
                     interruptMask | defaultInterruptMask, asynFlags, 1, priority, stackSize)
    , m_portName(portName)
    , m_messageQueue(queueSize, sizeof(void*))
    , m_thread(0)
    , m_shutdown(false)
{
    createParam("MsgOldDas",    asynParamGenericPointer,    &MsgOldDas);
    createParam("MsgError",     asynParamGenericPointer,    &MsgError);
    createParam("MsgDasData",   asynParamGenericPointer,    &MsgDasData);
    createParam("MsgDasCmd",    asynParamGenericPointer,    &MsgDasCmd);
    createParam("MsgDasRtdl",   asynParamGenericPointer,    &MsgDasRtdl);
    createParam("MsgParamExch", asynParamGenericPointer,    &MsgParamExch);

    if (blocking) {
        std::string threadName = m_portName + "_Thread";
        m_thread = new Thread(
            threadName.c_str(),
            std::bind(&BasePlugin::recvDownstreamThread, this, std::placeholders::_1),
            epicsThreadGetStackSize(epicsThreadStackMedium),
            epicsThreadPriorityMedium
        );
        if (m_thread)
            m_thread->start();
    }
}

BasePlugin::~BasePlugin()
{
    if (m_thread) {
        // Wake-up processing thread by sending a dummy message. The thread
        // will then exit based on the changed m_shutdown param.
        m_shutdown = true;
        m_messageQueue.send(0, 0);
        m_thread->stop();
        delete m_thread;
    }

    disconnect();
}

bool BasePlugin::connect(const std::list<std::string> &plugins, const std::list<int> &messageTypes)
{

    for (auto it=plugins.begin(); it!=plugins.end(); it++) {
        for (auto jt=messageTypes.begin(); jt!= messageTypes.end(); jt++) {

            // Check if already connected
            bool connected = false;
            for (auto kt=m_connectedPorts.begin(); kt != m_connectedPorts.end(); kt++) {
                if (kt->pluginName == *it && kt->messageType == *jt) {
                    connected = true;
                    break;
                }
            }
            if (connected) {
                LOG_WARN("Already connected to this plugin %s with message type %d", it->c_str(), *jt);
                continue;
            }

            asynUser *pasynuser = pasynManager->createAsynUser(0, 0);
            if (pasynuser == 0) {
                LOG_ERROR("Failed to create asyn user interface for %s", it->c_str());
                disconnect();
                return false;
            }
            pasynuser->userPvt = this;
            pasynuser->reason = *jt;

            asynStatus status = pasynManager->connectDevice(pasynuser, it->c_str(), 0);
            if (status != asynSuccess) {
                LOG_ERROR("Failed calling pasynManager->connectDevice to port %s (status=%d)",
                          it->c_str(), status);
                pasynManager->freeAsynUser(pasynuser);
                disconnect();
                return false;
            }

            asynInterface *interface = pasynManager->findInterface(pasynuser, asynGenericPointerType, 1);
            if (!interface) {
                LOG_ERROR("Can't find asynGenericPointer interface on remote plugin %s", it->c_str());
                pasynManager->freeAsynUser(pasynuser);
                disconnect();
                return false;
            }

            asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
            void *asynGenericPointerInterrupt = NULL;
            status = asynGenericPointerInterface->registerInterruptUser(
                        interface->drvPvt, pasynuser,
                        ::recvDownstreamCb, this, &asynGenericPointerInterrupt);
            if (status != asynSuccess) {
                LOG_ERROR("Can't enable interrupt callbacks: %s", pasynuser->errorMessage);
                pasynManager->freeAsynUser(pasynuser);
                disconnect();
                return false;
            }

            RemotePort port;
            port.pluginName = *it;
            port.messageType = *jt;
            port.pasynuser = pasynuser;
            port.asynGenericPointerInterrupt = asynGenericPointerInterrupt;
            m_connectedPorts.push_back(port);
        }
    }

    LOG_DEBUG("Connected to parent plugins: %s", std::accumulate(plugins.begin(), plugins.end(), std::string(",")).substr(1).c_str());
    return true;
}

bool BasePlugin::connect(const std::string &ports, const std::list<int> &messageTypes)
{
    std::vector<std::string> v = Common::split(ports, ',');
    std::list<std::string> ports_(v.begin(), v.end());
    return connect(ports_, messageTypes);
}

bool BasePlugin::disconnect()
{
    for (auto it=m_connectedPorts.begin(); it!=m_connectedPorts.end(); it++) {
        if (it->asynGenericPointerInterrupt) {

            asynInterface *interface = pasynManager->findInterface(it->pasynuser, asynGenericPointerType, 1);
            if (!interface) {
                LOG_ERROR("Can't find asynGenericPointer interface on remote plugin %s", it->pluginName.c_str());
            } else {
                asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
                asynStatus status = asynGenericPointerInterface->cancelInterruptUser(
                    interface->drvPvt,
                    it->pasynuser,
                    it->asynGenericPointerInterrupt);
                pasynManager->freeAsynUser(it->pasynuser);
                it->pasynuser = NULL;
                it->asynGenericPointerInterrupt = NULL;
                if (status != asynSuccess) {
                    LOG_ERROR("Can't disable interrupt callbacks on dispatcher port: %s", it->pasynuser->errorMessage);
                }
            }
        }
    }
    if (!m_connectedPorts.empty())
        LOG_DEBUG("Disconnected from parent plugins");
    m_connectedPorts.clear();
    return true;
}

bool BasePlugin::isConnected()
{
    return !m_connectedPorts.empty();
}

void BasePlugin::recvDownstreamCb(asynUser *pasynUser, void *ptr)
{
    int msgType = pasynUser->reason;
    PluginMessage *msg = reinterpret_cast<PluginMessage *>(ptr);
    if (msg != 0) {

        if (m_thread == 0) {
            /* In blocking mode, process the callback in calling thread. Return when
             * processing is complete.
             */
            this->lock();
            recvDownstream(msgType, msg);
            this->unlock();
        } else {
            /* Non blocking mode means the callback will be processed in our background
             * thread. Make a reservation so that it doesn't go away.
             */
            msg->claim();
            std::pair<int, PluginMessage*> *q = new std::pair<int, PluginMessage *>(msgType, msg);
            if (m_messageQueue.trySend(&q, sizeof(&q)) == -1) {
                msg->release();
                LOG_ERROR("Message queue full, discarding message");
                m_messageQueue.show();
            }
        }
    }
}

void BasePlugin::recvDownstreamThread(epicsEvent *shutdown)
{
    while (!m_shutdown) {
        std::pair<int, PluginMessage *> *q = NULL;
        if (m_messageQueue.receive(&q, sizeof(&q)) <= 0)
            continue;

        int msgType = q->first;
        PluginMessage *msg = q->second;

        this->lock();
        recvDownstream(msgType, msg);
        this->unlock();

        msg->release();
        delete q;
    }
}

void BasePlugin::recvDownstream(int type, PluginMessage *msg)
{
    if (msg) {
        if (type == MsgOldDas) {
            recvDownstream( *msg->get<const DasPacketList>() );
        } else if (type == MsgDasCmd) {
            recvDownstream( *msg->get<const DasCmdPacketList>() );
        } else if (type == MsgDasRtdl) {
            recvDownstream( *msg->get<const RtdlPacketList>() );
        } else if (type == MsgDasData) {
            recvDownstream( *msg->get<const DasDataPacketList>() );
        } else if (type == MsgError) {
            recvDownstream( *msg->get<const ErrorPacketList>() );
        } else if (type == MsgParamExch) {

        } else {
            LOG_ERROR("Ignoring not supported message type '%d'", type);
        }
    }
}

std::unique_ptr<PluginMessage> BasePlugin::sendDownstream(int type, const void *data, bool wait)
{
    std::unique_ptr<PluginMessage> msg(new PluginMessage(data));
    if (msg) {
        msg->claim();
        void *ptr = reinterpret_cast<void *>(msg.get());
        doCallbacksGenericPointer(ptr, type, 0);
        msg->release();
        if (wait) {
            msg->waitAllReleased();
            msg.reset();
        }
    }
    return msg;
}

asynStatus BasePlugin::writeGenericPointer(asynUser *pasynUser, void *ptr)
{
    int msgType = pasynUser->reason;
    PluginMessage *msg = reinterpret_cast<PluginMessage *>(ptr);
    if (msg == 0)
        return asynError;

    recvUpstream(msgType, msg);
    return asynSuccess;
}

asynStatus BasePlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == MsgParamExch) {
        ParamsExch *p = reinterpret_cast<ParamsExch *>(pasynUser->userData);
        return this->recvParam(p->portName, p->paramName, value);
    }
    return asynPortDriver::writeInt32(pasynUser, value);
}

void BasePlugin::recvUpstream(int type, PluginMessage *msg)
{
    if (type == MsgOldDas) {
        recvUpstream( *msg->get<const DasPacketList>() );
    } else if (type == MsgDasCmd) {
        recvUpstream( *msg->get<const DasCmdPacketList>() );
    } else {
        LOG_ERROR("Skipping sending unsupported message upstream");
    }
}

void BasePlugin::sendUpstream(int type, const void *data)
{
    std::unique_ptr<PluginMessage> msg(new PluginMessage(data));
    if (msg) {
        msg->claim();

        for (auto it=m_connectedPorts.begin(); it!=m_connectedPorts.end(); it++) {
            if (it->pasynuser->reason == type) {
                asynInterface *interface = pasynManager->findInterface(it->pasynuser, asynGenericPointerType, 1);
                if (!interface) {
                    LOG_ERROR("Can't find %s interface on array port %s", asynGenericPointerType, it->pluginName.c_str());
                    continue;
                }

                asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
                void *ptr = reinterpret_cast<void *>(msg.get());
                asynGenericPointerInterface->write(interface->drvPvt, it->pasynuser, ptr);
            }
        }

        msg->release();
        msg->waitAllReleased();
    }
}

std::shared_ptr<Timer> BasePlugin::scheduleCallback(std::function<float(void)> &callback, double delay)
{
    std::shared_ptr<Timer> timer(new Timer(true));
    if (timer) {
        std::function<float(void)> timerCb = std::bind(&BasePlugin::timerExpire, this, timer, callback);
        if (!timer->schedule(timerCb, delay))
           LOG_WARN("Failed to schedule callback");
    }
    return timer;
}

float BasePlugin::timerExpire(std::shared_ptr<Timer> &timer, std::function<float(void)> callback)
{
    float delay = 0.0;
    this->lock();
    if (timer->isActive()) {
        delay = callback();
    }
    this->unlock();
    return delay;
}

// ===== Parameter helper functions =====
const std::string BasePlugin::getParamName(int index)
{
    const char *name = "<error>";
    asynPortDriver::getParamName(index, &name);
    return name;
}

asynStatus BasePlugin::createParam(const std::string &name, asynParamType type, int *index, int initValue)
{
    asynStatus status = asynPortDriver::createParam(name.c_str(), type, index);
    if (status == asynSuccess && type == asynParamInt32) {
        status = setIntegerParam(*index, initValue);
    }
    return status;
}

asynStatus BasePlugin::createParam(const std::string &name, asynParamType type, int *index, double initValue)
{
    asynStatus status = asynPortDriver::createParam(name.c_str(), type, index);
    if (status == asynSuccess && type == asynParamFloat64) {
        status = setDoubleParam(*index, initValue);
    }
    return status;
}

asynStatus BasePlugin::createParam(const std::string &name, asynParamType type, int *index, const std::string &initValue)
{
    asynStatus status = asynPortDriver::createParam(name.c_str(), type, index);
    if (status == asynSuccess && type == asynParamOctet) {
        status = setStringParam(*index, initValue.c_str());
    }
    return status;
}

asynStatus BasePlugin::getBooleanParam(int index, bool &value)
{
    int val;
    asynStatus ret = getIntegerParam(index, &val);
    value = (val > 0);
    return ret;
}

bool BasePlugin::sendParam(const std::string &remotePort, const std::string &paramName, epicsInt32 value)
{
    ParamsExch p;
    p.portName = m_portName;
    p.paramName = paramName;

    asynUser *asynuser = pasynManager->createAsynUser(0, 0);
    asynuser->userPvt = this;
    asynuser->reason = MsgParamExch;
    asynuser->userData = reinterpret_cast<void *>(&p);

    asynStatus status = pasynManager->connectDevice(asynuser, remotePort.c_str(), 0);
    if (status != asynSuccess) {
        LOG_ERROR("Failed to connect to remote port %s (status=%d, error=%s)",
                  remotePort.c_str(), status, asynuser->errorMessage);
        return false;
    }

    bool ret;
    asynInterface *iface = pasynManager->findInterface(asynuser, asynInt32Type, 1);
    if (!iface) {
        LOG_ERROR("Can't find %s interface on array port %s", asynInt32Type, remotePort.c_str());
        ret = false;
    } else {
        asynInt32 *ifaceInt32 = reinterpret_cast<asynInt32 *>(iface->pinterface);
        ifaceInt32->write(iface->drvPvt, asynuser, value);
        ret = true;
    }

    pasynManager->disconnect(asynuser);
    pasynManager->freeAsynUser(asynuser);

    return ret;
}

asynStatus BasePlugin::getDoubleParam(const std::string &name, double &value)
{
    int param;
    asynStatus ret = asynPortDriver::findParam(name.c_str(), &param);
    if (ret == asynSuccess) {
        double val;
        ret = getDoubleParam(param, &val);
        if (ret == asynSuccess)
            value = val;
    }
    return ret;
}

asynStatus BasePlugin::setDoubleParam(const std::string &name, double value)
{
    int param;
    asynStatus ret = asynPortDriver::findParam(name.c_str(), &param);
    if (ret == asynSuccess)
        ret = setDoubleParam(param, value);
    return ret;
}

asynStatus BasePlugin::getIntegerParam(const std::string &name, int &value)
{
    int param;
    asynStatus ret = asynPortDriver::findParam(name.c_str(), &param);
    if (ret == asynSuccess) {
        int val;
        ret = getIntegerParam(param, &val);
        if (ret == asynSuccess)
            value = val;
    }
    return ret;
}

asynStatus BasePlugin::setIntegerParam(const std::string &name, int value)
{
    int param;
    asynStatus ret = asynPortDriver::findParam(name.c_str(), &param);
    if (ret == asynSuccess)
        ret = setIntegerParam(param, value);
    return ret;
}

asynStatus BasePlugin::addIntegerParam(const std::string &name, int increment) {
    int tmp;
    getIntegerParam(name, tmp);
    return setIntegerParam(name, tmp+increment);
};

asynStatus BasePlugin::addIntegerParam(int param, int increment) {
    epicsInt32 tmp;
    asynPortDriver::getIntegerParam(param, &tmp);
    return asynPortDriver::setIntegerParam(param, tmp+increment);
};
