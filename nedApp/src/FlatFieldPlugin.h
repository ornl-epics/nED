/* FlatFieldPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef FLAT_FIELD_PLUGIN_H
#define FLAT_FIELD_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * FlatFieldPlugin produces tof,pixelid stream out of x,y positions produces by some detectors.
 *
 * General plugin parameters:
 * asyn param    | asyn param type | init val | mode | Description
 * ------------- | --------------- | -------- | ---- | -----------
 * ReqDest       | asynParamOctet  | ""       | RW   | Module address to communicate with
 * ReqCmd        | asynParamInt32  | 0        | RW   | Command to be sent
 * ReqIsDsp      | asynParamInt32  | 0        | RW   | Is the module we communicate with behinds the DSP, using LVDS link
 * RspCmd        | asynParamInt32  | 0        | RO   | Response command, see DasPacket::CommandType
 * RspCmdAck     | asynParamInt32  | 0        | RO   | Response ACK/NACK
 * RspHwType     | asynParamInt32  | 0        | RO   | Hardware type, see DasPacket::ModuleType
 * RspSrc        | asynParamOctet  | 0        | RO   | Response source address
 * RspRouter     | asynParamOctet  | 0        | RO   | Response router address
 * RspDest       | asynParamOctet  | 0        | RO   | Response destination address
 * RspLen        | asynParamInt32  | 0        | RO   | Response length in bytes
 * RspDataLen    | asynParamInt32  | 0        | RO   | Response payload length in bytes
 * RspData       | asynParamInt32  | 0        | RO   | Response payload
 */
class FlatFieldPlugin : public BasePlugin {
    private: // variables
        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynInt32ArrayMask;
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynInt32ArrayMask;
        static const unsigned TABLE_X_SIZE = 512;
        static const unsigned TABLE_Y_SIZE = 512;

    public: // structures and defines
        /**
         * Constructor for FlatFieldPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        FlatFieldPlugin(const char *portName, const char *dispatcherPortName, int blocking=0);

        ~FlatFieldPlugin();

        /**
         * Overloaded function to handle writing strings and byte arrays.
         */
//        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded function to handle reading strings and byte arrays.
         */
//        asynStatus readOctet(asynUser *pasynUser, char *value, size_t nChars, size_t *nActual, int *eomReason);

        /**
         * Overloaded function to handle writing integers.
         */
        asynStatus writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements);

        /**
         * Overloaded function to process incoming OCC packets.
         */
//        virtual void processData(const DasPacketList * const packetList);

    private:
        void initTable();

        bool quadFit2D(unsigned int *data, int startindex, int endindex, double *coef);

    protected:
        #define FIRST_FLATFIELDPLUGIN_PARAM RefTable
        int RefTable;
        #define LAST_FLATFIELDPLUGIN_PARAM RefTable
};

#endif // FLAT_FIELD_PLUGIN_H
