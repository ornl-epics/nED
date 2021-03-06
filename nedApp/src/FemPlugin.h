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
#include "Timer.h"

#include <memory>

/**
 * Plugin for FEM module.
 */
class FemPlugin : public BaseModulePlugin {
    private: // variables
        struct RemoteUpgrade {
            /**
             * Valid remote update statuses
             */
            enum Status {
                NOT_SUPPORTED       = 0, //!< Remote update not supported by this modules
                NOT_READY           = 1, //!< Supported but failed to initialize internal structures
                LOADED              = 2, //!< Firmware file loaded and ready to start
                ERASING             = 3, //!< Flash is erasing, must wait
                IN_PROGRESS         = 4, //!< Some packets already sent, some left to be sent
                WAITING             = 5, //!< Waiting for module to become ready
                FINALIZING          = 6, //!< Final check of status, disable remote upgrade
                DONE                = 7, //!< Upgrade successful
                ERROR               = 8, //!< Upgrade failed
                ABORTED             = 9, //!< User aborted update
            } status;

            /**
             * Valid state machine actions/state transitions.
             */
            typedef enum {
                INIT,
                LOAD_FILE,
                START,
                PROCESS_RESPONSE,
                ABORT,
                TIMEOUT,
            } Action;

            /**
             * Type of check to perform in remoteUpgradeCheck function.
             */
            typedef enum {
                WAIT_ERASED,
                WAIT_READY,
                WAIT_PROGRAMMED,
            } CheckType;

            McsFile file;           //!< Firmware image file path, stays opened while in progress
            std::shared_ptr<char> buffer;
            uint32_t bufferSize;
            uint32_t pktPayloadSize;//!< Size of allocated space in packet payload
            std::vector<char> sendBuf;
            uint32_t offset;        //!< Current data position, used as progress
            uint32_t lastCount;     //!< Number of bytes sent in previous chunk
            Timer responseTimer{false};//!< Currently running timer for response timeout handling
            Timer statusTimer{false};//!< Currently running timer for status refresh
            epicsTimeStamp eraseStartTime; //!< Timestamp of erase command sent out
            epicsTimeStamp lastReadyTime; //!< Timestamp of last 'ready' response
            epicsTimeStamp disableTime; //!< Timestamp when upgrade was disabled
            uint8_t expectedWrCfg;  //!< Counter how many write config responses to silently ignore
            uint32_t expectedSeqId; //!< Packet sequence number for verifying remote end received packet
            uint32_t seqRetries;    //!< Consequtive failed responses
            uint32_t totRetries;    //!< Total number of retries during one upgrade session
        } m_remoteUpgrade;          //!< Remote upgrade context

    public: // functions

        /**
         * Constructor for FemPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] parentPlugins to connect to.
         * @param[in] version FEM HW&SW version, ie. V10_50
         * @param[in] pvaParamsName Name of PVA pv to export parameters lists
         */
        FemPlugin(const char *portName, const char *parentPlugins, const char *version, const char *pvaParamsName=nullptr, const char *configDir=nullptr);

        /**
         * Handler for CMD_UPGRADE command.
         *
         * Sends out a CMD_UPGRADE request packet, with payload copied from m_sendBuf if any.
         */
        bool reqUpgrade();

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
         * Create and register all FEM9 v310 parameters to be exposed to EPICS.
         */
        void createParams_v310();

        /**
         * Create and register all FEM9 v38 parameters to be exposed to EPICS.
         */
        void createParams_v320();

        /**
         * Remote upgrade state machine function.
         *
         * This state machine is FEM specific, although it could be generilized
         * if modules follow similar approach. The state machine operates
         * asynchronously in response-processing thread upon receiving response
         * from the module.
         *
         * @image html images/RemoteUpgrade.svg
         */
        bool remoteUpgradeSM(RemoteUpgrade::Action action, const DasCmdPacket *packet=0);

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
        FemPlugin::RemoteUpgrade::Status remoteUpgradeCheck(RemoteUpgrade::CheckType type);

        /**
         * Remote upgrade timeout handler.
         */
        float remoteUpgradeTimeout();

        /**
         * Periodic timer to check upgrade status.
         */
        float remoteUpgradeStatusRefresh();

        /**
         * Check whether the timer expired
         *
         * Retrieve asyn parameter value as timeout and check whether more than
         * that time has elapsed since the timer was reset.
         *
         * @param[in] timer previously reset, starting point
         * @param[in] timeoutParam asyn float64 param to retrieve timeout value
         * @return true if more than timeout time has elapsedsince timer was set/reset, false otherwise
         */
        bool timerExpired(epicsTimeStamp *timer, int timeoutParam);

    protected:
        int UpgradeFile;     //!< New firmware file to be programed
        int UpgradeChunkSize;//!< Number of bytes to push in one transfer
        int UpgradeStatus;   //!< Remote upgrade status
        int UpgradeErrorStr; //!< Error description
        int UpgradeSize;     //!< Total firmware size in bytes
        int UpgradePosition; //!< Bytes already sent to remote party
        int UpgradeCmd;      //!< Command to send to update process (start, abort)
        int UpgradeEraseTimeout;    //!< Max time to wait for erased flag
        int UpgradeBusyTimeout;     //!< Max time to wait for ready flag
        int UpgradeProgramTimeout;  //!< Max time to wait for programmed flag
        int UpgradeNoRspTimeout;    //!< Max time to wait for response
        int UpgradeNoRspMaxRetries; //!< Max times to retry when no response received
        int UpgradeNoRspSeqRetries; //!< Number of consecutive retries
        int UpgradeNoRspTotRetries; //!< Number of all retries in one upgrade session
};

#endif // DSP_PLUGIN_H
