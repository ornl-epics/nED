/* FemPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef FEM_PLUGIN_H
#define FEM_PLUGIN_H

#include "BaseModulePlugin.h"
#include "McsFile.h"

#include <memory>

/**
 * Plugin for FEM module.
 */
class FemPlugin : public BaseModulePlugin {
    private: // structures and definitions
        static const unsigned NUM_FEMPLUGIN_DYNPARAMS;  //!< Maximum number of asyn parameters, including the status and configuration parameters

    private: // variables
        std::string m_version;              //!< Version string as passed to constructor
        struct RemoteUpgrade {
            /**
             * Valid remote update statuses
             */
            enum Status {
                NOT_SUPPORTED       = 0, //!< Remote update not supported by this modules
                NOT_READY           = 1, //!< Supported but failed to initialize internal structures
                READY               = 2, //!< Supported and ready
                LOADED              = 3, //!< Firmware file loaded and ready to start
                IN_PROGRESS         = 4, //!< Some packets already sent, some left to be sent
                DONE                = 5, //!< Upgrade successful
                ERROR               = 6, //!< Upgrade failed
                ABORTED             = 7, //!< User aborted update
            } status;

            McsFile file;           //!< Firmware image file path, stays opened while in progress
            std::shared_ptr<char> buffer;
            uint32_t bufferSize;
            uint32_t pktPayloadSize;//!< Size of allocated space in packet payload
            uint32_t offset;        //!< Current data position, used as progress
            uint8_t busyRetries;    //!< Number of busy retries
            uint32_t lastCount;     //!< Number of bytes sent in previous chunk
            std::shared_ptr<Timer> timer; //!< Currently running timer for response timeout handling
        } m_remoteUpgrade;          //!< Remote upgrade context

    public: // functions

        /**
         * Constructor for FemPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the ROC module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] version FEM HW&SW version, ie. V10_50
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        FemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking=0);

        /**
         * Handle parameters write requests for integer type.
         *
         * When an integer parameter is written through PV, this function
         * gets called with a new value. It handles the Command parameter
         * as well as all configuration parameters.
         *
         * @param[in] pasynUser asyn handle
         * @param[in] value New value to be applied
         * @return asynSuccess on success, asynError otherwise
         */
        virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Handle writing strings.
         */
        virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Try to parse the FEM version response packet an populate the structure.
         *
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @return true if succesful, false if version response packet could not be parsed.
         */
        static bool parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version);

        /**
         * Member counterpart of parseVersionRsp().
         *
         * @see FemPlugin::parseVersionRsp()
         */
        bool parseVersionRspM(const DasPacket *packet, BaseModulePlugin::Version &version)
        {
            return parseVersionRsp(packet, version);
        }

        /**
         * Configured version must match actual.
         *
         * @return true when they match, false otherwise.
         */
        bool checkVersion(const BaseModulePlugin::Version &version);

    private: // functions
        /**
         * Create and register all FEM10 parameters to be exposed to EPICS.
         */
        void createParams_v32();

        /**
         * Create and register all FEM9 v35 parameters to be exposed to EPICS.
         */
        void createParams_v35();

        /**
         * Create and register all FEM9 v36 parameters to be exposed to EPICS.
         */
        void createParams_v36();

        /**
         * Create and register all FEM9 v37 parameters to be exposed to EPICS.
         */
        void createParams_v37();

        /**
         * Create and register all FEM9 v38 parameters to be exposed to EPICS.
         */
        void createParams_v38();

        /**
         * Create and register all FEM9 v39 parameters to be exposed to EPICS.
         */
        void createParams_v39();

        /**
         * Create and register all FEM9 v38 parameters to be exposed to EPICS.
         */
        void createParams_v320();

        /**
         * Initialize remote upgrade sequence.
         *
         * Pre-allocates DasPacket used to send each chunk of upgrade data. It
         * then sets the remote upgrade status to ready. Finally it registers
         * remote upgrade handler as internal state machine to be invoked
         * whenever a CMD_UPGRADE response is received.
         */
        void remoteUpgradeInit();

        /**
         * Load firmware file
         *
         * Opens the file and determines its length. UpgradeSize and UpgradeLen
         * PVs are updated. Next buffer for each packet is allocated based on
         * user provided value, rounded to nearest power of 2. Finally the
         * upgrade in progress flag is set, which prevents starting second
         * upgrade sequence.
         */
        void remoteUpgradeLoad(const std::string &path);

        /**
         * Abort remote upgrade currently in progress.
         *
         * @param[in] userAbort Determines the remote upgrade status, ABORTED vs. ERROR
         */
        void remoteUpgradeAbort(bool userAbort);

        /**
         * Handle remote upgrade response packet.
         *
         * This function is almost the complete state machine for upgrade process.
         * Each time it's called it checks the status returned by module to
         * determine whether to proceed with more data, wait for busy flag or
         * abort due to error. It sends one chunk of data at a time and quits,
         * expecting the response to invoke this function again when received.
         *
         * @param[in] packet response from module
         * @return true when packet has been handled
         */
        bool remoteUpgradeRsp(const DasPacket *packet);

        /**
         * Module specific function to check for remote upgrade status response.
         *
         * Function should return RemoteUpgrade::DONE when last packet was accepted
         * by module, RemoteUpgrade::IN_PROGRESS if module is busy and needs to
         * be retried, or RemoteUpgrade::ERROR in case of any error that
         * prevents this upgrade to succeed.
         * Packet was already unpacked and all PVs populated before invoking
         * this function.
         */
        FemPlugin::RemoteUpgrade::Status remoteUpgradeCheck();

        /**
         * Remote upgrade timeout handler.
         */
        float remoteUpgradeTimeout();

    protected:
        #define FIRST_FEMPLUGIN_PARAM UpgradeFile
        int UpgradeFile;     //!< New firmware file to be programed
        int UpgradeChunkSize;//!< Number of bytes to push in one transfer
        int UpgradeStatus;   //!< Remote upgrade status
        int UpgradeSize;     //!< Total firmware size in bytes
        int UpgradePosition; //!< Bytes already sent to remote party
        int UpgradeCmd;      //!< Command to send to update process (start, abort)
        #define LAST_FEMPLUGIN_PARAM UpgradeCmd
};

#endif // DSP_PLUGIN_H
