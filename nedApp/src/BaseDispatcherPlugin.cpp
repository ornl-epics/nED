/* BaseDispatcherPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseDispatcherPlugin.h"
#include "Log.h"

BaseDispatcherPlugin::BaseDispatcherPlugin(const char *portName, const char *dispatcherPortName,
                                           int blocking, int numParams, int interfaceMask, int interruptMask)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, numParams, 1,
                 interfaceMask | BaseDispatcherPlugin::defaultInterfaceMask, interruptMask | BaseDispatcherPlugin::defaultInterruptMask)
{
}

BaseDispatcherPlugin::~BaseDispatcherPlugin()
{}

asynStatus BaseDispatcherPlugin::writeGenericPointer(asynUser *pasynUser, void *pointer)
{
    if (pasynUser->reason == REASON_OCCDATA) {
        asynInterface *interface = pasynManager->findInterface(m_pasynuser, asynGenericPointerType, 1);
        if (!interface) {
            LOG_ERROR("ERROR: Cannot find writeGenericPointer interface on array port %s", m_dispatcherPortName.c_str());
            return asynError;
        }

        asynGenericPointer *asynGenericPointerInterface = reinterpret_cast<asynGenericPointer *>(interface->pinterface);
        void *ptr = reinterpret_cast<void *>(reinterpret_cast<DasPacketList *>(pointer));
        asynGenericPointerInterface->write(interface->drvPvt, m_pasynuser, ptr);
    }
    return asynSuccess;
}

void BaseDispatcherPlugin::sendToPlugins(const DasPacketList *packetList)
{
    void *ptr = const_cast<void *>(reinterpret_cast<const void *>(packetList));
    doCallbacksGenericPointer(ptr, REASON_OCCDATA, 0);
}
