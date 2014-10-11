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
    const DasPacket *first = 0;
    const DasPacket *last = 0;
    DasPacketList cmdList;
    uint32_t nReceived = 0;
    uint32_t nProcessed = 0;

    for (const DasPacket *packet = packetList->first(); packet != 0; packet = packetList->next(packet)) {
        nReceived++;

        if (packet->isCommand() && packet->cmdinfo.command != DasPacket::CMD_RTDL && packet->cmdinfo.command != DasPacket::CMD_TSYNC) {
            if (first == 0)
                first = packet;
            last = packet;
            nProcessed++;
        } else if (first) {
            sendToPlugins(first, last);
            first = last = 0;
        }
    }
    if (first) {
        sendToPlugins(first, last);
    }

    // Update parameters
    this->lock();
    m_nReceived += nReceived;
    m_nProcessed += nProcessed;
    setIntegerParam(ProcCount,  m_nProcessed);
    setIntegerParam(RxCount,    m_nReceived);
    callParamCallbacks();
    this->unlock();
}

void CmdDispatcher::sendToPlugins(const DasPacket *first, const DasPacket *last)
{
    DasPacketList cmdList;
    uint32_t length = (reinterpret_cast<const uint8_t*>(last) - reinterpret_cast<const uint8_t*>(first)) + last->length();

    cmdList.reset(reinterpret_cast<const uint8_t*>(first), length);
    BaseDispatcherPlugin::sendToPlugins(&cmdList);
}
