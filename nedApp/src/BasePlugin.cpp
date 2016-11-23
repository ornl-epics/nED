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
#include "DasPacketList.h"
#include "Log.h"
#include "Timer.h"

#include <epicsThread.h>
#include <string>

#define NUM_BASEPLUGIN_PARAMS ((int)(&LAST_BASEPLUGIN_PARAM - &FIRST_BASEPLUGIN_PARAM + 1))

#define MESSAGE_QUEUE_SIZE 1   //!< Size of the message queue for callbacks. Since using the synchronous callbacks, 1 should be enough.

/* Helper C functions for asyn/EPICS registration
 */
extern "C" {
    static void dispatcherCallback(void *drvPvt, asynUser *pasynUser, void *genericPointer)
    {
        BasePlugin *plugin = reinterpret_cast<BasePlugin *>(drvPvt);
        plugin->dispatcherCallback(pasynUser, genericPointer);
    }
}

BasePlugin::BasePlugin(const char *portName, const char *dispatcherPortNames, int reason, int blocking,
                       int numParams, int maxAddr, int interfaceMask,
                       int interruptMask, int asynFlags, int autoConnect,
                       int priority, int stackSize)
    : asynPortDriver(portName, maxAddr, NUM_BASEPLUGIN_PARAMS + numParams, interfaceMask | defaultInterfaceMask,
                     interruptMask | defaultInterruptMask, asynFlags, autoConnect, priority, stackSize)
    , m_portName(portName)
    , m_messageQueue(MESSAGE_QUEUE_SIZE, sizeof(void*))
    , m_thread(0)
    , m_shutdown(false)
{
    int status;

    createParam("Enable",       asynParamInt32,     &Enable);       // WRITE - Enable or disable plugin
    createParam("ProcCount",    asynParamInt32,     &ProcCount, 0); // READ - Number processed packets
    createParam("RxCount",      asynParamInt32,     &RxCount,   0); // READ - Number packets received from OCC
    createParam("TxCount",      asynParamInt32,     &TxCount,   0); // READ - Number packets sent to OCC
    createParam("DataMode",     asynParamInt32,     &DataModeP, DATA_MODE_NORMAL); // WRITE - Data format mode (see BasePlugin::DataMode)

    std::vector<std::string> portNames = split(dispatcherPortNames, ',');
    for (auto it=portNames.begin(); it!=portNames.end(); it++) {
        asynUser *pasynuser = pasynManager->createAsynUser(0, 0);
        if (pasynuser == 0) {
            LOG_ERROR("Failed to create asyn user interface for %s", it->c_str());
            continue;
        }
        pasynuser->userPvt = this;
        pasynuser->reason = reason;

        // Connect to dispatcher port permanently. Don't allow connecting to different port at runtime.
        // Callbacks need to be enabled separately in order to actually get triggered from dispatcher.
        status = pasynManager->connectDevice(pasynuser, it->c_str(), 0);
        if (status != asynSuccess) {
            LOG_ERROR("Failed calling pasynManager->connectDevice to port %s (status=%d, error=%s)",
                      it->c_str(), status, m_remotePorts[*it].pasynuser->errorMessage);
            pasynManager->freeAsynUser(pasynuser);
            continue;
        }

        m_remotePorts[*it].pasynuser = pasynuser;
        m_remotePorts[*it].asynGenericPointerInterrupt = NULL;
    }

    if (m_remotePorts.size() > 1) {
        // Enforce blocking mode to avoid serialization of calling threads
        blocking = 1;
    }

    if (blocking) {
        std::string threadName = m_portName + "_Thread";
        m_thread = new Thread(
            threadName.c_str(),
            std::bind(&BasePlugin::processDataThread, this, std::placeholders::_1),
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

    // Make sure to disconnect from asyn ports
    (void)enableCallbacks(false);

    for (auto it=m_remotePorts.begin(); it!=m_remotePorts.end(); it++) {
        pasynManager->disconnect(it->second.pasynuser);
        pasynManager->freeAsynUser(it->second.pasynuser);
        it->second.pasynuser = NULL;
    }
}

void BasePlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    this->lock();
    processData(packetList);
    this->unlock();
}

asynStatus BasePlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == Enable) {
        this->unlock();
        bool status = enableCallbacks(value > 0);
        this->lock();
        if (status == false)
            return asynError;
    } else if (pasynUser->reason == DataModeP) {
        switch (value) {
        case DATA_MODE_NORMAL:
        case DATA_MODE_RAW:
        case DATA_MODE_VERBOSE:
            m_dataMode = static_cast<DataMode>(value);
            break;
        default:
            LOG_ERROR("Ignoring invalid output mode %d", value);
            return asynError;
        }
    }

    return asynPortDriver::writeInt32(pasynUser, value);
}

asynStatus BasePlugin::createParam(const char *name, asynParamType type, int *index, int initValue)
{
    asynStatus status = asynPortDriver::createParam(name, type, index);
    if (status == asynSuccess && type == asynParamInt32)
        status = setIntegerParam(*index, initValue);
    return status;
}

asynStatus BasePlugin::createParam(const char *name, asynParamType type, int *index, double initValue)
{
    asynStatus status = asynPortDriver::createParam(name, type, index);
    if (status == asynSuccess && type == asynParamFloat64)
        status = setDoubleParam(*index, initValue);
    return status;
}

asynStatus BasePlugin::createParam(const char *name, asynParamType type, int *index, const char *initValue)
{
    asynStatus status = asynPortDriver::createParam(name, type, index);
    if (status == asynSuccess && type == asynParamOctet)
        status = setStringParam(*index, initValue);
    return status;
}

asynStatus BasePlugin::getBooleanParam(int index, bool *value)
{
    int val;
    asynStatus ret = getIntegerParam(index, &val);
    *value = (val > 0);
    return ret;
}

void BasePlugin::dispatcherCallback(asynUser *pasynUser, void *genericPointer)
{
    DasPacketList *packetList = reinterpret_cast<DasPacketList *>(genericPointer);

    if (packetList == 0)
        return;

    if (m_thread == 0) {
        /* In blocking mode, process the callback in calling thread. Return when
         * processing is complete.
         */
        processDataUnlocked(packetList);
    } else {
        /* Non blocking mode means the callback will be processed in our background
         * thread. Make a reservation so that it doesn't go away.
         */
        packetList->reserve();
        if (m_messageQueue.trySend(&packetList, sizeof(&packetList)) == -1) {
            packetList->release();
            LOG_ERROR("Message queue full");
        }
    }
}

void BasePlugin::sendToDispatcher(const DasPacket *packet)
{
    for (auto it=m_remotePorts.begin(); it!=m_remotePorts.end(); it++) {
        asynInterface *interface = pasynManager->findInterface(it->second.pasynuser, asynGenericPointerType, 1);
        if (!interface) {
            LOG_ERROR("Can't find %s interface on array port %s", asynGenericPointerType, it->first.c_str());
            continue;
        }

        DasPacketList packetsList;
        packetsList.reset(packet);
        asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
        void *ptr = reinterpret_cast<void *>(const_cast<DasPacketList *>(&packetsList));
        asynGenericPointerInterface->write(interface->drvPvt, it->second.pasynuser, ptr);
        packetsList.release();
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

const char *BasePlugin::getParamName(int index)
{
    const char *name = "<error>";
    asynPortDriver::getParamName(index, &name);
    return name;
}

void BasePlugin::processDataThread(epicsEvent *shutdown)
{
    while (!m_shutdown) {
        DasPacketList *packetList;

        m_messageQueue.receive(&packetList, sizeof(&packetList));
        if (packetList == 0)
            continue;

        processDataUnlocked(packetList);

        packetList->release();
    }
}

bool BasePlugin::enableCallbacks(bool enable)
{
    asynStatus status = asynSuccess;

    for (auto it=m_remotePorts.begin(); it!=m_remotePorts.end(); it++) {
        asynInterface *interface = pasynManager->findInterface(it->second.pasynuser, asynGenericPointerType, 1);
        if (!interface) {
            LOG_ERROR("Can't find asynGenericPointer interface on array port %s", it->first.c_str());
            return false;
        }

        if (enable && !it->second.asynGenericPointerInterrupt) {
            asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
            status = asynGenericPointerInterface->registerInterruptUser(
                        interface->drvPvt, it->second.pasynuser,
                        ::dispatcherCallback, this, &it->second.asynGenericPointerInterrupt);
            if (status != asynSuccess) {
                LOG_ERROR("Can't enable interrupt callbacks on dispatcher port: %s", it->second.pasynuser->errorMessage);
            }
        }
        if (!enable && it->second.asynGenericPointerInterrupt) {
            asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
            status = asynGenericPointerInterface->cancelInterruptUser(
                interface->drvPvt,
                it->second.pasynuser,
                it->second.asynGenericPointerInterrupt);
            it->second.asynGenericPointerInterrupt = NULL;
            if (status != asynSuccess) {
                LOG_ERROR("Can't disable interrupt callbacks on dispatcher port: %s", it->second.pasynuser->errorMessage);
            }
        }
    }

    return (status == asynSuccess);
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

bool BasePlugin::sendParam(const std::string &remotePort, const std::string &paramName, epicsInt32 value)
{
    ParamsExch p = {m_portName, paramName};

    asynUser *asynuser = pasynManager->createAsynUser(0, 0);
    asynuser->userPvt = this;
    asynuser->reason = REASON_PARAMS_EXCH;
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

asynStatus BasePlugin::getIntegerParam(const char *name, int *value)
{
    int param;
    asynStatus ret = asynPortDriver::findParam(name, &param);
    if (ret == asynSuccess)
        ret = getIntegerParam(param, value);
    return ret;
}
