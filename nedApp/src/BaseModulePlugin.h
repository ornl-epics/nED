/* BaseModulePlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BASE_MODULE_PLUGIN_H
#define BASE_MODULE_PLUGIN_H

#include "BasePlugin.h"
#include "StateMachine.h"
#include "Timer.h"

#include <fstream>
#include <map>

#define SM_ACTION_CMD(a)        (a)
#define SM_ACTION_ACK(a)        ((a) | (0x1 << 16))
#define SM_ACTION_ERR(a)        ((a) | (0x1 << 17))
#define SM_ACTION_TIMEOUT(a)    ((a) | (0x1 << 18))

/**
 * Base class for all plugins working with particular module.
 *
 * The BaseModulePlugin provides functionality common to all modules
 * handler plugins. It's able to construct outgoing packet with all
 * the raw fields. It filters out incoming packets which don't originate
 * from the connected module and invokes appropriate response handler
 * for each response, which can be overloaded by derived classes.
 * It also hides the complexity of connection type and simplifies it
 * to a single "is module behind the DSP" question, which is discussed
 * in details in next section.
 *
 * Submodules connected through DSP use LVDS connection and their
 * responses contain 16-bit data fields packet into 24-bit LVDS
 * words. Extra 8-bits are used by the LVDS channel for parity,
 * start/stop bits etc. DSP on the other hand sends 32-bit data
 * fields and the CRC at the end of the packet checks the correctness
 * of all the data. When an LVDS packet is passing the DSP, DSP
 * might transform it. When sending a packet to the module behind
 * the DSP, DSP expects to received LVDS packet tagged as pass-thru.
 * OCC header destination is not used, instead the destination address
 * must be included as first two dwords in the packet's payload.
 * The actual payload follows in 16-bit words. Each dword in the
 * pass-thru OCC packet must have upper 8 bits cleared, and the LVDS
 * control bits must be calculated.
 * Receiving response from a module behind the DSP is different.
 * DSP transforms the response from submodules. It takes away
 * LVDS control bits and joins 2 LVDS 16-bit data fields into
 * single 32-bit dword field - first LVDS word in lower-part of dword
 * and second LVDS word in upper-part of dword
 *
 * BaseModulePlugin supports all common module communication, including
 * DISCOVER, READ_VERSION, READ_STATUS, READ_CONFIG, WRITE_CONFIG,
 * START, STOP. There's 2 overloadable handlers for each type, reqXY
 * sends a command to module and rspXY processes the response to that
 * command. Sending out a command recharges the timer which defaults
 * to 2 seconds. Received response cancels the timeout. No other command
 * can be issues while waiting for response.
 *
 * There's a generic status and configuration parameter description
 * functionality that all modules should use. Modules should only
 * create a table of parameters using createStatusParam() and
 * createConfigParam() functions. The BaseModulePlugin will make
 * sure to properly map those into OCC packets.
 *
 * Commands can be issued independently from each other as long as
 * response for the previous command was received or timed out.
 * Command can be issued using CmdReq PV which atomically sends
 * OCC command to the module and switches CmdRsp PV to waiting state.
 * Reading the CmdRsp PV immediately after writing CmdReq will
 * @b always give accurate last command status.
 */
class BaseModulePlugin : public BasePlugin {
    public: // structures and defines
        /**
         * Valid last command return codes
         */
        enum LastCommandResponse {
            LAST_CMD_NONE           = 0,    //!< No command issued yet
            LAST_CMD_OK             = 1,    //!< Last command response received and parsed
            LAST_CMD_WAIT           = 2,    //!< Waiting for last command response
            LAST_CMD_TIMEOUT        = 3,    //!< Did not receive response for the last command
            LAST_CMD_ERROR          = 4,    //!< Error processing last command response
            LAST_CMD_SKIPPED        = 5,    //!< Command was not issued
        };

        /**
         * Possible module verification statuses
         */
        enum TypeVersionStatus {
            ST_TYPE_VERSION_INIT    = 0,
            ST_TYPE_OK              = 1,
            ST_TYPE_ERR             = 2,
            ST_VERSION_OK           = 3,
            ST_VERSION_ERR          = 4,
            ST_TYPE_VERSION_OK      = 5,
        };

        /**
         * Valid remote upgrade status values.
         */
        enum RemoteUpgradeStatus {
            UPGRADE_NOT_SUPPORTED   = 0, //!< Remote upgrade not yet started
            UPGRADE_NOT_STARTED     = 1, //!< Remote upgrade not yet started
            UPGRADE_IN_PROGRESS     = 2, //!< Remote upgrade currently in progress
            UPGRADE_DONE            = 3, //!< All data sent and acknowledged by remote party
            UPGRADE_CANCELED        = 4, //!< Canceled due to packet timeout or nack
            UPGRADE_INIT_FAILED     = 5, //!< Failed to initialize
            UPGRADE_USER_ABORT      = 6, //!< Aborted by user
        };

        /**
         * Structure describing the status parameters obtained from modules.
         */
        struct ParamDesc {
            uint32_t offset;        //!< An 4-byte offset within the payload
            uint32_t shift;         //!< Position of the field bits within 32 bits dword at given offset
            uint32_t width;         //!< Number of bits used for the value
            uint8_t section;        //!< Section name, valid values [0x0..0xF] (configuration params only)
            uint8_t channel;        //!< Channel number in range [1..8], 0 means global configuration (configuration params only)
            int32_t initVal;        //!< Initial value after object is created or configuration reset is being requested (configuration params only)
        };

        struct Version {
            uint8_t hw_version;
            uint8_t hw_revision;
            uint16_t hw_year;
            uint8_t hw_month;
            uint8_t hw_day;
            uint8_t fw_version;
            uint8_t fw_revision;
            uint16_t fw_year;
            uint8_t fw_month;
            uint8_t fw_day;

            Version()
                : hw_version(0)
                , hw_revision(0)
                , hw_year(0)
                , hw_month(0)
                , hw_day(0)
                , fw_version(0)
                , fw_revision(0)
                , fw_year(0)
                , fw_month(0)
                , fw_day(0)
            {}
        };

        static const float NO_RESPONSE_TIMEOUT;         //!< Number of seconds to wait for module response

    public: // variables
        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynOctetMask;
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynOctetMask;

    protected: // variables
        uint32_t m_hardwareId;                          //!< Hardware ID which this plugin is connected to
        DasPacket::ModuleType m_hardwareType;           //!< Hardware type
        std::vector<uint32_t> m_statusPayloadLengths;   //!< Size in bytes of the READ_STATUS request/response payload, calculated dynamically by createStatusParam()
        uint32_t m_countersPayloadLength;               //!< Size in bytes of the READ_STATUS_COUNTERS request/response payload, calculated dynamically by createCounterParam()
        uint32_t m_upgradePayloadLength;                //!< Size in bytes of the PROGRAM response payload, calculated dynamically by linkUpgradeParam()
        uint32_t m_temperaturePayloadLength;            //!< Size in bytes of the READ_TEMPERATURE response payload, calculated dynamically by createTempParam()
        std::map<int, ParamDesc> m_statusParams;        //!< Map of exported status parameters
        std::map<int, ParamDesc> m_counterParams;       //!< Map of exported status counter parameters
        std::map<int, ParamDesc> m_configParams;        //!< Map of exported config parameters
        std::map<int, ParamDesc> m_upgradeParams;       //!< Map of exported remote upgrade parameters
        std::map<int, ParamDesc> m_temperatureParams;   //!< Map of exported temperature parameters
        StateMachine<TypeVersionStatus, int> m_verifySM;//!< State machine for verification status
        DasPacket::CommandType m_waitingResponse;       //!< Expected response code while waiting for response or timeout event, 0 otherwise
        bool m_configInitialized;                       //!< Configuration sections sizes and offsets are calculated
        uint8_t m_expectedChannel;                      //!< Channel to be configured or read config next, 0 means global config, resets to 0 when reaches 8
        uint32_t m_numChannels;                         //!< Maximum number of channels supported by module
        uint8_t m_cfgSectionCnt;                        //!< Used with sending channels configuration, tells number of times this section succeeded for previous channels

    private: // variables
        bool m_behindDsp;
        std::map<int, uint32_t> m_configSectionSizes;   //!< Configuration section sizes, in words (word=2B for submodules, =4B for DSPs)
        std::map<int, uint32_t> m_configSectionOffsets; //!< Status response payload size, in words (word=2B for submodules, =4B for DSPs)
        std::shared_ptr<Timer> m_timeoutTimer;          //!< Currently running timer for response timeout handling
        struct {
            bool inProgress;        //!< Remote upgrade currently in progress
            std::ifstream file;     //!< Firmware image file path, stays opened while in progress
            uint32_t *buffer;       //!< Buffer to read data into
            int bufferSize;         //!< Buffer size in bytes, (re)allocated when inProgress transitions to true
            int position;           //!< Current file position, used as progress
        } m_remoteUpgrade;          //!< Remote upgrade context

    public: // functions

        /**
         * Constructor for BaseModulePlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] hardwareId Hardware ID of the module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] hardwareType Type of hardware module.
         * @param[in] behindDsp Is this module behind the DSP which transforms some of the packets?
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         * @param[in] numParams The number of parameters that the derived class supports.
         * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports.
         * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks)
         */
        BaseModulePlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId,
                         DasPacket::ModuleType hardwareType, bool behindDsp, int blocking=0, int numParams=0,
                         int interfaceMask=BaseModulePlugin::defaultInterfaceMask,
                         int interruptMask=BaseModulePlugin::defaultInterruptMask);

        /**
         * Abstract destructor
         */
        virtual ~BaseModulePlugin() = 0;

        /**
         * Set number of channels supported by module.
         *
         * When module firmware implements its channels as individual
         * configuration register sets, this function can be used to specify
         * number of channels supported. It must be called before
         * createConfigParam().
         *
         * Most of the modules have their channel configuration as part of
         * global configuration and they don't need to call this function.
         *
         * @param[in] n Number of individually configured channels, max 16.
         */
        void setNumChannels(uint32_t n);

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
         * Create a packet and send it through dispatcher to the OCC board
         *
         * Either optical or LVDS packet is created based on the connection type this
         * module uses. It then sends the packet to the dispatcher in order to be delivered
         * through OCC optical link to the module.
         *
         * Note: The length parameter is in bytes, although the payload is an array of
         * 4-byte unsigned integers. OCC packets are always 4 byte aligned, but the LVDS
         * data in them might be 2 byte aligned. The payload should always point to an
         * array of 4 byte unsigned integers. The length should be dividable by 2.
         *
         * @param[in] command A command of the packet to be sent out.
         * @param[in] channel Select a target channel, 0 means no specific channel.
         * @param[in] payload Payload to be sent out, can be NULL if length is also 0.
         * @param[in] length Payload length in bytes.
         */
        void sendToDispatcher(DasPacket::CommandType command, uint8_t channel=0, uint32_t *payload=0, uint32_t length=0);

        /**
         * Overloaded incoming data handler.
         *
         * Function iterates through receives packets and silently skipping
         * the ones that their destination does not match connected module
         * or the packet is not response to a command.
         * Packets to be processed are handed to processResponse() function
         * for real work.
         * Function also updates statistical parameters like number of received
         * and processed packets.
         *
         * @param[in] packets List of packets received in this batch.
         */
        virtual void processData(const DasPacketList * const packets);

        /**
         * General response from modules handler.
         *
         * This generic response handler recognizes all well-known responses and calls corresponding
         * handlers.
         *
         * @param[in] packet to be processed.
         * @return true if packet has been processed, false otherwise
         */
        virtual bool processResponse(const DasPacket *packet);

        /**
         * Called when discover request to the module should be made.
         *
         * Base implementation simply sends a DISCOVER command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqDiscover();

        /**
         * Default handler for DISCOVER response.
         *
         * Only check for timeout.
         *
         * @param[in] packet with response to DISCOVER
         * @return true if timeout has not yet expired, false otherwise.
         */
        virtual bool rspDiscover(const DasPacket *packet);

        /**
         * Called when read version request to the module should be made.
         *
         * Base implementation simply sends a READ_VERSION command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqReadVersion();

        /**
         * Default handler for READ_VERSION response.
         *
         * Only check for timeout.
         *
         * @param[in] packet with response to READ_VERSION
         * @return true if timeout has not yet expired, false otherwise.
         */
        virtual bool rspReadVersion(const DasPacket *packet);

        /**
         * Send request to module to read all status registers.
         *
         * Base implementation simply sends a READ_STATUS command and sets up
         * timeout callback.
         *
         * Some modules provide channel specific registers. For those a
         * modified read status command must be used to address particular
         * channel. 0 always selects main/control part of the module,
         * positive numbers select specific channel.
         *
         * @param[in] channel to be selected, 0 selects main/control part.
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqReadStatus(uint8_t channel);

        /**
         * Default handler for READ_STATUS response.
         *
         * Read the packet payload and populate status parameters.
         *
         * @param[in] packet with response to READ_STATUS
         * @param[in] channel selected
         * @return true if packet was parsed and module version verified.
         */
        virtual bool rspReadStatus(const DasPacket *packet, uint8_t channel);

        /**
         * Called when read status counters request to the module should be made.
         *
         * Base implementation simply sends a READ_STATUS_COUNTERS command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqReadStatusCounters();

        /**
         * Called when reset status counters request to the module should be made.
         *
         * Base implementation simply sends a RESET_STATUS_COUNTERS command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqResetStatusCounters();

        /**
         * Default handler for READ_STATUS_COUNTERS response.
         *
         * Read the packet payload and populate counters parameters.
         *
         * @param[in] packet with response to READ_STATUS_COUNTERS
         * @return true if packet was parsed and module version verified.
         */
        virtual bool rspReadStatusCounters(const DasPacket *packet);

        /**
         * Default handler for RESET_STATUS_COUNTERS response.
         *
         * @param[in] packet with response to READ_STATUS_COUNTERS
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspResetStatusCounters(const DasPacket *packet);

        /**
         * Send request to module to read configuration request.
         *
         * Send a DAS packet requesting all configuration registers for given
         * channel.
         *
         * Some modules provide channel specific registers. For those a
         * modified read status command must be used to address particular
         * channel. 0 always selects main/control part of the module,
         * positive numbers select specific channel.
         *
         * @param[in] channel to be selected, 0 selects main/control part.
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqReadConfig(uint8_t channel);

        /**
         * Default handler for READ_CONFIG response.
         *
         * Read the packet payload and populate status parameters.
         *
         * @param[in] packet with response to READ_STATUS
         * @param[in] channel selected
         * @return true if packet was parsed and module version verified.
         */
        virtual bool rspReadConfig(const DasPacket *packet, uint8_t channel);

        /**
         * Construct WRITE_CONFIG payload and send it to module.
         *
         * Configuration data is gathered from configuration parameters
         * and their current values. Configuration packet is created with
         * configuration data attached and sent to OCC, but not waited for
         * response.
         * Access to this function should be locked before calling it or
         * otherwise configuration data inconsistencies can occur. Luckily,
         * this function gets triggered from parameter update, which is
         * already locked.
         *
         * This function is asynchronous and does not wait for response.
         *
         * Most modules split their registers into sections. It's up to the
         * module implementantion how these are defined. Often different
         * firmware versions reorganize registers in sections. Registers
         * within a specific section can be written individually. All sections
         * can be written when module is not acquiring data, sections 0xA-0xF
         * can be written any time. Selecting section 0 writes all sections.
         *
         * Some modules provide channel specific registers. For those a
         * modified read status command must be used to address particular
         * channel. 0 always selects main/control part of the module,
         * positive numbers select specific channel.
         *
         * @param[in] section to be selected, 0 selects all.
         * @param[in] channel to be selected, 0 selects main/control part.
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqWriteConfig(uint8_t section, uint8_t channel);

        /**
         * Default handler for READ_CONFIG response.
         *
         * Default implementation checks whether timeout callbacks has already
         * kicked in and cancels still pending timeout timer.
         *
         * @param[in] packet with response to READ_STATUS
         * @param[in] channel selected
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspWriteConfig(const DasPacket *packet, uint8_t channel);

        /**
         * Send START command to module.
         *
         * This function is asynchronous and does not wait for response.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqStart();

        /**
         * Request one pulse for Pulsed Magnet.
         *
         * This function is asynchronous and does not wait for response.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqPulse();

        /**
         * Clears one pulse request for Pulsed Magnet.
         *
         * This function is asynchronous and does not wait for response.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqPulseClear();

        /**
         * Default handler for START response.
         *
         * @param[in] packet with response to START
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspStart(const DasPacket *packet);

        /**
         * Send STOP command to module.
         *
         * This function is asynchronous and does not wait for response.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqStop();

        /**
         * Default handler for STOP response.
         *
         * @param[in] packet with response to STOP
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspStop(const DasPacket *packet);

        /**
         * Default handler for CMD_PM_PULSE_RQST_ON response.
         *
         * @param[in] packet with response to pulse request
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspPulse(const DasPacket *packet);

        /**
         * Default handler for CMD_PM_PULSE_RQST_OFF response.
         *
         * @param[in] packet with response to pulse clear
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspPulseClear(const DasPacket *packet);

        /**
         * Send part of the new firmware image as one packet.
         *
         * The firmware image is split into packets based on UpgradePktSize
         * parameter. On first packet, the remote upgrade sequence is
         * initialized.
         * Updates the UpgradeStatus parameter for every relevant change.
         *
         * @image html Remote_Upgrade_SM.png
         * @image latex Remote_Upgrade_SM.png width=6in
         * @return DasPacket::CMD_UPGRADE or 0 when no packet was sent for some reason.
         */
        virtual DasPacket::CommandType reqUpgrade();

        /**
         * Default handler for CMD_UPGRADE response.
         *
         * Verifies that the remote module accepted partial firmware packet.
         * On last packet it the remote upgrade sequence is stopped.
         * Updates the UpgradeStatus parameter for every relevant change.
         *
         * @retval true Remote module acknowledged reception.
         * @retval false Timeout has occurred or remote module refused packet.
         */
        virtual bool rspUpgrade(const DasPacket *packet);

        /**
         * Called when read temperature request to the module should be made.
         *
         * Base implementation simply sends a READ_TEMP command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasPacket::CommandType reqReadTemperature();

        /**
         * Default handler for READ_TEMP response.
         *
         * Read the packet payload and populate temperature parameters.
         *
         * @param[in] packet with response to READ_TEMP
         * @return true if packet was parsed and temperature extracted.
         */
        virtual bool rspReadTemperature(const DasPacket *packet);
        /**
         * Create and register single integer status parameter.
         *
         * Status parameter is an individual status entity exported by module.
         * It can be a flag that some event occured or it can be a value like
         * number of errors. The createStatusParam() function covers them all.
         * Parameters don't exceed 32 bits. However, they can span
         * over 32bit boundary if they're shifted.
         * This function recognizes whether it's working with submodule and
         * calculates the real offset in the response. The offset parameter
         * should thus be specified in format used by the module (word 7 on LVDS
         * should be specified as offset 0x7 and dword 7 on DSP should also be
         * specified as 0x7).
         *
         * @param[in] name Parameter name must be unique within the plugin scope.
         * @param[in] channel Selected channel
         * @param[in] offset word/dword offset within the payload.
         * @param[in] nBits Width of the parameter in number of bits.
         * @param[in] shift Starting bit position within the word/dword.
         */
        void createStatusParam(const char *name, uint8_t channel, uint32_t offset, uint32_t nBits, uint32_t shift);

        /**
         * Convenience function for modules that don't split configuration for channels.
         */
        void createStatusParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift)
        {
            createStatusParam(name, 0, offset, nBits, shift);
        }

        /**
         * Create and register single integer status counter parameter.
         *
         * Status counters provide a way to diagnose communication problems.
         */
        void createCounterParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift);

        /**
         * Create and register single integer config parameter.
         */
        void createConfigParam(const char *name, uint8_t channel, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value);

        /**
         * Convenience function for modules that don't split configuration for channels.
         */
        void createConfigParam(const char *name, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value)
        {
            createConfigParam(name, 0, section, offset, nBits, shift, value);
        }

        /**
         * Create and register single integer temperature parameter.
         *
         * Temperature values are returned in READ_TEMPERATURE response payload.
         */
        void createTempParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift);

        /**
         * Link existing parameter to upgrade parameters table.
         *
         * Useful when two hardware registers in two different response types
         * have the same meaning and need to be merged into one plugin parameter.
         * When either of the responses is received, only one parameter gets
         * updated.
         *
         * @param[in] name must be any previously created parameter.
         * @param[in] offset word/dword offset within the payload.
         * @param[in] nBits Width of the parameter in number of bits.
         * @param[in] shift Starting bit position within the word/dword.
         */
        void linkUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift);

        /**
         * Convert IP or hex string into 4 byte hardware address.
         *
         * Recognized string formats:
         * - "21.250.118.223"
         * - "0x15FA76DF"
         *
         * @param[in] text to be parsed
         * @return Parsed hardware ID or 0 on error.
         */
        static uint32_t ip2addr(const std::string &text);

        /**
         * Convert 4 byte hardware address into IP like string.
         *
         * @param[in] address to be converter
         * @return IP like string
         */
        static std::string addr2ip(uint32_t address);

        /**
         * A no-response cleanup function.
         *
         * The timeout is canceled if the response is received before timeout
         * expires.
         *
         * @param[in] command Command sent to module for which response should be received.
         * @return true if the timeout function did the cleanup, that is received was *not* received.
         */
        virtual float noResponseCleanup(DasPacket::CommandType command);

        /**
         * Request a custom callback function to be called at some time in the future.
         *
         * Uses BasePlugin::scheduleCallback() for scheduling the BaseModulePlugin::noResponseTimeout()
         * function and stores the timer as a member variable.
         *
         * Function expects the Plugin to be locked.
         *
         * @param[in] command Expected command response
         * @param[in] delay Delay from now when to invoke the function, in seconds.
         * @retval true if callback was scheduled
         * @retval false if callback was not scheduled
         */
        bool scheduleTimeoutCallback(DasPacket::CommandType command, double delay);

        /**
         * Cancel any pending timeout callback and release the timer.
         *
         * Function expects the Plugin to be locked.
         *
         * @retval true if future callback was canceled
         * @retval false if callback was already invoked and it hasn't been canceled
         */
        bool cancelTimeoutCallback();

        /**
         * Initialize remote upgrade sequence.
         *
         * Opens the file and determines its length. UpgradeSize and UpgradeLen
         * PVs are updated. Next buffer for each packet is allocated based on
         * user provided value, rounded to nearest power of 2. Finally the
         * upgrade in progress flag is set, which prevents starting second
         * upgrade sequence.
         *
         * @return true when initialized, false on any error.
         */
        bool remoteUpgradeStart();

        /**
         * Read some data from file and send it to remote party.
         *
         * Read data into previously allocated buffer and ship it.
         *
         * @return false when no more data to send.
         */
        bool remoteUpgradeSend();

        /**
         * Test for more data to send.
         *
         * @return true when some data available, false otherwise.
         */
        bool remoteUpgradeDone();

        /**
         * Stop current remote upgrade sequence.
         *
         * Must be called before initiating new sequence.
         */
        void remoteUpgradeStop();

        /**
         * Check whether remote upgrade sequence is active.
         *
         * @retval true active
         * @retval false not active
         */
        bool remoteUpgradeInProgress();

        /**
         * Try to parse READ_VERSION packet payload into a Version structure.
         *
         * There's no common version response payload. Every module uses
         * different format. It's up to the derived class to provide proper
         * parsing.
         * Derived classes implement this function as static so it can be used
         * withouth object context (ie. in DiscoverPlugin). However C++ prohibits
         * overloading static functions, so this member function has an extra M
         * at the end.
         *
         * @param[in] packet to be parsed
         * @param[out] version structure to be populated
         * @return true if succesful, false if version response packet could not be parsed.
         */
        virtual bool parseVersionRspM(const DasPacket *packet, BaseModulePlugin::Version &version) = 0;

        /**
         * Configured version must match actual.
         *
         * @return true when they match, false otherwise.
         */
        virtual bool checkVersion(const BaseModulePlugin::Version &version) = 0;

    private: // functions
        /**
         * Trigger calculating the configuration parameter offsets.
         */
        void recalculateConfigParams();

        /**
         * Method parses packet payload and extracts parameter values.
         *
         * This generic method works for status, counter and configuration
         * parameters. For every parameter in the table, it finds the matching
         * value in the packet payload and assign it as new parameter value.
         * When section offsets table is given, the parameter offset is
         * considered relative to the section it belongs.
         *
         * @param[in] packet to be parsed
         * @param[in] table of paramaters to be matched
         * @param[in] sectOffsets is a able of section offsets.
         */
        void extractParams(const DasPacket *packet,
                           const std::map<int, ParamDesc> &table,
                           const std::map<int, uint32_t> &sectOffsets=std::map<int, uint32_t>());

    protected:
        #define FIRST_BASEMODULEPLUGIN_PARAM CmdReq
        int CmdReq;         //!< Command to plugin, like initialize the module, read configuration, verify module version etc.
        int CmdRsp;         //!< Last command response status
        int HwId;           //!< Hw ID that this object is controlling
        int HwType;         //!< Configured module type
        int HwVer;          //!< Module hardware version
        int HwRev;          //!< Module hardware revision
        int HwDate;         //!< Module hardware date
        int FwVer;          //!< Module firmware version
        int FwRev;          //!< Module firmware revision
        int FwDate;         //!< Module firmware date
        int Supported;      //!< Flag whether module is supported
        int Verified;       //!< Hardware id, version and type all verified
        int CfgSection;     //!< Selected configuration section to be written
        int CfgChannel;     //!< Selected channel to be configured
        int UpgradeFile;    //!< New firmware file to be programed
        int UpgradePktSize; //!< Max payload size for split transfer
        int UpgradeStatus;  //!< Remote upgrade status
        int UpgradeSize;    //!< Total firmware size in bytes
        int UpgradePos;     //!< Bytes already sent to remote party
        int UpgradeAbort;   //!< Abort current upgrade sequence
        #define LAST_BASEMODULEPLUGIN_PARAM UpgradeAbort
};

#endif // BASE_MODULE_PLUGIN_H
