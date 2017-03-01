/* DataDiagPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DataDiagPlugin.h"
#include "Log.h"

EPICS_REGISTER_PLUGIN(DataDiagPlugin, 2, "Port name", string, "Dispatcher port name", string);

#define NUM_DATADIAGPLUGIN_PARAMS      ((int)(&LAST_DATADIAGPLUGIN_PARAM - &FIRST_DATADIAGPLUGIN_PARAM + 1))

DataDiagPlugin::DataDiagPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_DATADIAGPLUGIN_PARAMS, 1,
                 defaultInterfaceMask, defaultInterruptMask)
    , m_memoryPool(false)
{
    createParam("Data",         asynParamOctet, &Data); // Data
    createParam("CheckEqTofPix",asynParamInt32, &CheckEqTofPix, 0); // Enable TOF==pixid check
    createParam("CheckIncTime", asynParamInt32, &CheckIncTime, 0); // Enable incrementing time check
    createParam("QueIndex",     asynParamInt32, &QueIndex, 0);  // Currently selected error data
    createParam("QueSize",      asynParamInt32, &QueSize, 0);   // Number of elements in errors buffer
    createParam("QueMaxSize",   asynParamInt32, &QueMaxSize, 0); // Max num of elements in errors buffer

    callParamCallbacks();
}

asynStatus DataDiagPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    /*
    if (pasynUser->reason == ReqSend) {
    }
    */
    return BasePlugin::writeInt32(pasynUser, value);
}

asynStatus DataDiagPlugin::readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason)
{
    if (pasynUser->reason == Data) {
        int queIndex = 0;
        getIntegerParam(QueIndex, &queIndex);

        *nActual = 0;

        if (queIndex < 0)
            queIndex *= -1;
        if (queIndex < (int)m_errorsQue.size()) {
            auto data = m_errorsQue.begin();
            for (int i=0; i<queIndex; i++)
                data++;

            for (uint32_t i = 0; i < data->size; i++) {
                int len;
                char delimiter = ((i+1)%6 == 0 ? '\n' : ' ');
                uint32_t val = data->buffer.get()[i];

                if (i == data->errorOffset)
                    len = snprintf(value, nChars, "***%08X***%c", val, delimiter);
                else
                    len = snprintf(value, nChars, "%08X%c", val, delimiter);
                if (len >= static_cast<int>(nChars) || len == -1)
                    break;

                nChars -= len;
                *nActual += len;
                value += len;
            }
        }
        if (eomReason) *eomReason |= ASYN_EOM_EOS;
        return asynSuccess;
    }
    return BasePlugin::readOctet(pasynUser, value, nChars, nActual, eomReason);
}

void DataDiagPlugin::processData(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    bool equalTofPixel = false;
    bool incTime = false;
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getBooleanParam(CheckEqTofPix, &equalTofPixel);
    getBooleanParam(CheckIncTime, &incTime);
    uint32_t batchLen = 0;
    uint32_t errorOffset = 0;

    nReceived += packetList->size();

    uint32_t batchOffset = 0;
    for (auto it = packetList->cbegin(); it != packetList->cend() && errorOffset == 0; it++) {
        const DasPacket *packet = *it;
        batchLen += packet->getLength();
        bool processed = false;

        if (equalTofPixel == true && packet->isData()) {
            errorOffset = checkEqualTofPixel(packet);
            if (errorOffset != 0) {
                errorOffset += batchOffset;
                break;
            } else {
                processed = true;
            }
        }

        if (incTime == true && packet->isData()) {
            errorOffset = checkIncTime(packet);
            if (errorOffset != 0) {
                errorOffset += batchOffset;
                break;
            } else {
                processed = true;
            }
        }

        batchOffset += packet->getLength();

        if (processed)
            nProcessed++;
    }

    if (errorOffset != 0) {
        int maxQueSize = 0;
        getIntegerParam(QueMaxSize, &maxQueSize);
        if (maxQueSize > 0) {
            ErrorDesc error;
            error.buffer = m_memoryPool.getPtr(batchLen);
            error.size = batchLen / 4;
            error.errorOffset = errorOffset;

            char *buffer = reinterpret_cast<char*>(error.buffer.get());
            for (auto it = packetList->cbegin(); it != packetList->cend() && batchLen > 0; it++) {
                const DasPacket *packet = *it;
                uint32_t length = packet->getLength();
                memcpy(buffer, packet, length);
                buffer += length;
                batchLen -= length;
            }

            while ((int)m_errorsQue.size() >= maxQueSize)
                m_errorsQue.pop_back();
            m_errorsQue.push_front(error);
        }
    }

    setIntegerParam(QueSize, m_errorsQue.size());
    setIntegerParam(RxCount,    nReceived);
    setIntegerParam(ProcCount,  nProcessed);
    callParamCallbacks();
}

uint32_t DataDiagPlugin::checkIncTime(const DasPacket *packet) {
    uint32_t errorOffset = 0;
    const RtdlHeader *rtdl = packet->getRtdlHeader();
    if (rtdl != 0) {
        epicsTimeStamp t = { rtdl->timestamp_sec, rtdl->timestamp_nsec };

        if (packet->isNeutronData()) {
            if (epicsTimeLessThan(&t, &m_lastNeutronTime))
                errorOffset = 6; // TODO: use OFFSET_OF or similar
            m_lastNeutronTime = t;
        } else if (packet->isMetaData()) {
            if (epicsTimeLessThan(&t, &m_lastMetadataTime))
                errorOffset = 6; // TODO: use OFFSET_OF or similar
            t = m_lastMetadataTime;
        }
    }
    return errorOffset;
}

uint32_t DataDiagPlugin::checkEqualTofPixel(const DasPacket *packet) {
    uint32_t errorOffset = 0;
    uint32_t length = 0;
    const uint32_t *events = packet->getData(&length);

    // Treat all events as TOF,pixid pairs, 8 bytes total
    for (uint32_t i=0; i<length/8; i++) {
        uint32_t tof = *events++;
        uint32_t pixel = *events++;
        if (tof == pixel) {
            errorOffset = (events - reinterpret_cast<const uint32_t*>(packet) - 2);
            break;
        }
    }
    return errorOffset;
}
