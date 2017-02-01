/* CmdDispatcher.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "CmdDispatcher.h"

static const int asynMaxAddr       = 1;
static const int asynInterfaceMask = asynGenericPointerMask;
static const int asynInterruptMask = asynGenericPointerMask;
static const int asynFlags         = 0;
static const int asynAutoConnect   = 1;
static const int asynPriority      = 0;
static const int asynStackSize     = 0;

EPICS_REGISTER_PLUGIN(CmdDispatcher, 2, "Port name", string, "Dispatcher port name", string);

CmdDispatcher::CmdDispatcher(const char *portName, const char *connectPortName)
    : BaseDispatcherPlugin(portName, connectPortName, /*blocking=*/1, /*numparams=*/0,
                           asynInterfaceMask, asynInterruptMask)
    , m_nReceived(0)
    , m_nProcessed(0)
{
}

void CmdDispatcher::processDataUnlocked(const DasPacketList * const packetList)
{
    DasPacketList cmdList;

    for (auto it = packetList->cbegin(); it != packetList->cend(); it++) {
        const DasPacket *packet = *it;

        if (packet->isResponse()) {
            cmdList.push_back(packet);
        }
    }

    if (!cmdList.empty()) {
        cmdList.reserve();
        BaseDispatcherPlugin::sendToPlugins(&cmdList);
        cmdList.release();
        cmdList.waitAllReleased();
    }

    // Update parameters
    this->lock();
    m_nReceived += packetList->size();
    m_nProcessed += cmdList.size();
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
    this->unlock();
}
