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
#include "Timer.h"
#include "ValueConvert.h"

#include <fstream>
#include <map>
#include <memory>

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
 *
 * Some modules implement channel functionality in dedicated FPGAs. In such
 * case each FPGA keeps it's own status and configuration registers. They can
 * be addresses with the same command but with extra channel information in the
 * header. BaseModulePlugin aggregates all channels into a single trigger.
 * It uses internal sequence to send multiple packets to the module, addressing
 * all channels. It does so sequentially, any error stops sequence. Derived
 * class must use setNumChannel() function for this functionality to get
 * enabled.
 */
class BaseModulePlugin : public BasePlugin {
    public: // structures and defines

        /**
         * Available features.
         */
        enum class ModuleFeatures : uint32_t {
            CONFIG                  = 1,    //!< Module supports configuration registers
            STATUS                  = 2,    //!< Module supports status registers
            TEMPERATURE             = 4,    //!< Module supports temperature registers
            COUNTERS                = 8,    //!< Module supports counters registers
            UPGRADE                 = 16,   //!< Module supports remote upgrade
            TIME_SYNC               = 32,   //!< Module supports time synchronization
        };

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
         * Structure describing the status parameters obtained from modules.
         */
        struct ParamDesc {
            uint32_t offset;        //!< An 4-byte offset within the payload
            uint32_t shift;         //!< Position of the field bits within 32 bits dword at given offset
            uint32_t width;         //!< Number of bits used for the value
            uint8_t section;        //!< Section name, valid values [0x0..0xF] (configuration params only)
            uint8_t channel;        //!< Channel number in range [1..8], 0 means global configuration (configuration params only)
            int32_t initVal;        //!< Initial value after object is created or configuration reset is being requested (configuration params only)
            std::shared_ptr<const BaseConvert> convert; //!< Selected from/to raw value converter
        };

        /**
         * Structure describing parameter table.
         */
        struct ParamTable {
            std::map<int, ParamDesc> mapping;   //!< Mapping table, int index is asyn parameter index
            bool sections;                      //!< Are parameters split into sections
            std::map<int, uint32_t> sizes;      //!< Section sizes in bytes
            std::map<int, uint32_t> offsets;    //!< Section offsets in bytes
            bool readonly;                      //!< Flag whether all parameters in this group are read-only
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
        static const float RESET_NO_RESPONSE_TIMEOUT;   //!< Number of seconds to wait for module RESET response
        static const float CONN_CLOSE_TIMEOUT;          //!< Number of seconds to wait after response or timeout before closing connection to parent plugins

    public: // variables
        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynOctetMask | asynFloat64Mask;
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynOctetMask | asynFloat64Mask;
        static const UnsignConvert *CONV_UNSIGN;
        static const Sign2sComplementConvert *CONV_SIGN_2COMP;
        static const SignMagnitudeConvert *CONV_SIGN_MAGN;

    protected: // variables
        uint32_t m_features;
        uint32_t m_hardwareId;                          //!< Hardware ID which this plugin is connected to
        DasCmdPacket::ModuleType m_hardwareType;        //!< Hardware type
        std::map<std::string, ParamTable> m_params;     //!< Maps of exported parameters
        DasCmdPacket::CommandType m_waitingResponse;    //!< Expected response code while waiting for response or timeout event, 0 otherwise
        uint8_t m_expectedChannel;                      //!< Channel to be configured or read config next, 0 means global config, resets to 0 when reaches 8
        uint32_t m_numChannels;                         //!< Maximum number of channels supported by module
        uint8_t m_cfgSectionCnt;                        //!< Used with sending channels configuration, tells number of times this section succeeded for previous channels

    private: // variables
        uint8_t m_wordSize;                             //!< Register word size.
        uint8_t m_cmdVer = 0;                           //!< Commands protocol version
        std::map<int, uint32_t> m_configSectionSizes;   //!< Configuration section sizes, in words (word=2B for submodules, =4B for DSPs)
        std::map<int, uint32_t> m_configSectionOffsets; //!< Status response payload size, in words (word=2B for submodules, =4B for DSPs)
        Timer m_timeoutTimer;                           //!< Currently running timer for response timeout handling
        std::list<std::function<bool(const DasCmdPacket *)> > m_stateMachines; //!< Active internal state machines
        epicsTime m_connStaleTime;                      //!< Time when connection becomes candidate to close, used for book-keeping the connection
        Timer m_connTimer;                              //!< Periodic timer to check whether connection can be closed.
        std::string m_parentPlugins;                    //!< Parent plugins to connect to
        std::string m_configDir;                        //!< Directory where to save all configurations

    public: // functions

        /**
         * Constructor for BaseModulePlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] configDir Folder to store module configurations.
         * @param[in] hardwareType Type of hardware module.
         * @param[in] wordSize Number of bytes describing single register word
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         * @param[in] numParams The number of parameters that the derived class supports.
         * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports.
         * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks)
         */
        BaseModulePlugin(const char *portName, const char *parentPlugins, const char *configDir,
                         DasCmdPacket::ModuleType hardwareType, uint8_t wordSize,
                         int interfaceMask=0, int interruptMask=0);

        /**
         * Abstract destructor
         */
        virtual ~BaseModulePlugin() = 0;

        /**
         * Write octet parameter value.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Set number of channels supported by module.
         *
         * When module firmware implements its channels as individual register
         * sets, this function can be used to specify number of channels
         * supported. It must be called exactly once, before createConfigParam()
         * or createStatusParam() calls.
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
        void sendUpstream(DasCmdPacket::CommandType command, uint8_t channel=0, uint32_t *payload=0, uint32_t length=0);

        /**
         * Send packet to parent plugins.
         *
         * @param[in] packet to be sent
         */
        void sendUpstream(const DasCmdPacket* packet);

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
        virtual void recvDownstream(const DasCmdPacketList &packets);

        /**
         * Check response packets before handling their content.
         *
         * First it checks if there's any internal state machine expecting this
         * response. Each state machine must register its entry function with
         * m_stateMachines and must return true if it handled the response, in
         * which case all further processing of the response is stopped.
         *
         * Enforces several checks on response packets. When a command is sent
         * out, expected response must be received or timeout occurred before
         * another command can be issued. This function checks both conditions
         * and skips the response if either not expected or received after
         * timeout.
         * After all checks have passed it invokes handleResponse() for actual
         * response processing which can be overloaded.
         *
         * Function supports channel specific status and config responses.
         *
         * Override this function when either the command was not sent from
         * handleRequest() or when you want to handle response checks
         * yourself.
         *
         * @param[in] packet to be processed.
         * @return true if packet has been processed, false otherwise
         */
        virtual bool processResponse(const DasCmdPacket *packet);

        /**
         * Send a request to the module.
         *
         * For every down-stream request the module is expected to reply back.
         * A state around waiting for the response is handled by this function,
         * including setting up timeout counter, setting the expected response
         * and updating related status PV to let user know about the progress.
         * A single outstanding request/response is allowed at any given time.
         * If correlating response is not received in given time, the request
         * is canceled.
         *
         * @param[in] command to be sent
         * @return true if request has been handled, false otherwise
         */
        virtual bool processRequest(DasCmdPacket::CommandType command);

        /**
         * Handle common command requests.
         *
         * Command request is triggered from EPICS records and is handled by
         * writeInt32 CmdReq parameter. Function is called form writeInt32()
         * handler to send out corresponding command. Default implementation
         * supports these commands:
         * - DasCmdPacket::CMD_READ_VERSION
         * - DasCmdPacket::CMD_READ_CONFIG
         * - DasCmdPacket::CMD_READ_STATUS
         * - DasCmdPacket::CMD_READ_TEMPERATURE
         * - DasCmdPacket::CMD_READ_STATUS_COUNTERS
         * - DasCmdPacket::CMD_RESET_STATUS_COUNTERS
         * - DasCmdPacket::CMD_WRITE_CONFIG
         * - DasCmdPacket::CMD_DISCOVER
         * - DasCmdPacket::CMD_RESET
         * - DasCmdPacket::CMD_START
         * - DasCmdPacket::CMD_STOP
         * - DasCmdPacket::CMD_UPGRADE
         * Detector specific commands can be issued by overloading this function.
         *
         * @param[in] command requested.
         * @param[out] timeout before giving up waiting for response, default is 2.0
         * @return Response to be waited for.
         */
        virtual DasCmdPacket::CommandType handleRequest(DasCmdPacket::CommandType command, double &timeout);

        /**
         * Handles common responses.
         *
         * This function is called from processResponse() after response has
         * been verified and checked for timeout. Handling response needs
         * only verify contents of the response packet.
         *
         * Default implementation handles responses common to all modules.
         * This includes status, config, version, discover and other responses.
         * Derived plugin can overload this function to support custom
         * commands/responses.
         *
         * Overload this function when command was sent from handleRequest()
         * handler.
         *
         * @param[in] packet to be processed.
         * @return true if packet has been processed successfuly, false otherwise
         */
        virtual bool handleResponse(const DasCmdPacket *packet);

        /**
         * Send request to module to do a reset.
         */
        virtual DasCmdPacket::CommandType reqReset();

        /**
         * Handle RESET response.
         */
        virtual bool rspReset(const DasCmdPacket *packet);

        /**
         * Send request to reset LVDS chips.
         *
         * ROC boards does a full power-cycle of all LVDS chips when it receives
         * this command.
         */
        virtual DasCmdPacket::CommandType reqResetLvds();

        /**
         * Handle RESET_LVDS response.
         */
        virtual bool rspResetLvds(const DasCmdPacket *packet);

        /**
         * Send request to LVDS switch to send a long T&C SysReset signal.
         */
        virtual DasCmdPacket::CommandType reqTcReset();

        /**
         * Handle T&C_RESET response.
         */
        virtual bool rspTcReset(const DasCmdPacket *packet);

        /**
         * Send request to LVDS switch to send a short T&C SysReset signal.
         *
         * ROC boards does a full power-cycle of all LVDS chips when it receives
         * this command.
         */
        virtual DasCmdPacket::CommandType reqTcResetLvds();

        /**
         * Handle T&C_RESET_LVDS response.
         */
        virtual bool rspTcResetLvds(const DasCmdPacket *packet);

        /**
         * Called when discover request to the module should be made.
         *
         * Base implementation simply sends a DISCOVER command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqDiscover();

        /**
         * Default handler for DISCOVER response.
         *
         * Only check for timeout.
         *
         * @param[in] packet with response to DISCOVER
         * @return true if timeout has not yet expired, false otherwise.
         */
        virtual bool rspDiscover(const DasCmdPacket *packet);

        /**
         * Called when read version request to the module should be made.
         *
         * Base implementation simply sends a READ_VERSION command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqReadVersion();

        /**
         * Default handler for READ_VERSION response.
         *
         * Only check for timeout.
         *
         * @param[in] packet with response to READ_VERSION
         * @return true if timeout has not yet expired, false otherwise.
         */
        virtual bool rspReadVersion(const DasCmdPacket *packet);

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
        virtual DasCmdPacket::CommandType reqReadStatus(uint8_t channel);

        /**
         * Default handler for READ_STATUS response.
         *
         * Read the packet payload and populate status parameters.
         *
         * @param[in] packet with response to READ_STATUS
         * @param[in] channel selected
         * @return true if packet was parsed and module version verified.
         */
        virtual bool rspReadStatus(const DasCmdPacket *packet, uint8_t channel);

        /**
         * Called when read status counters request to the module should be made.
         *
         * Base implementation simply sends a READ_STATUS_COUNTERS command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqReadStatusCounters();

        /**
         * Called when reset status counters request to the module should be made.
         *
         * Base implementation simply sends a RESET_STATUS_COUNTERS command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqResetStatusCounters();

        /**
         * Default handler for READ_STATUS_COUNTERS response.
         *
         * Read the packet payload and populate counters parameters.
         *
         * @param[in] packet with response to READ_STATUS_COUNTERS
         * @return true if packet was parsed and module version verified.
         */
        virtual bool rspReadStatusCounters(const DasCmdPacket *packet);

        /**
         * Default handler for RESET_STATUS_COUNTERS response.
         *
         * @param[in] packet with response to READ_STATUS_COUNTERS
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspResetStatusCounters(const DasCmdPacket *packet);

        /**
         * Send request to module to read configuration request.
         *
         * Send a DAS packet requesting all configuration registers for given
         * channel.
         *
         * Some modules provide channel specific registers. For those a
         * modified read config command must be used to address particular
         * channel. 0 always selects main/control part of the module,
         * positive numbers select specific channel.
         *
         * @param[in] channel to be selected, 0 selects main/control part.
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqReadConfig(uint8_t channel);

        /**
         * Default handler for READ_CONFIG response.
         *
         * Read the packet payload and populate status parameters.
         *
         * @param[in] packet with response to READ_STATUS
         * @param[in] channel selected
         * @return true if packet was parsed and module version verified.
         */
        virtual bool rspReadConfig(const DasCmdPacket *packet, uint8_t channel);

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
         * modified write config command must be used to address particular
         * channel. 0 always selects main/control part of the module,
         * positive numbers select specific channel.
         *
         * @param[in] section to be selected, 0 selects all.
         * @param[in] channel to be selected, 0 selects main/control part.
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqWriteConfig(uint8_t section=0, uint8_t channel=0);

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
        virtual bool rspWriteConfig(const DasCmdPacket *packet, uint8_t channel);

        /**
         * Send START command to module.
         *
         * This function is asynchronous and does not wait for response.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqStart();

        /**
         * Default handler for START response.
         *
         * @param[in] packet with response to START
         * @retval true Timeout has not yet occurred
         * @retval false Timeout has occurred and response is invalid.
         */
        virtual bool rspStart(const DasCmdPacket *packet);

        /**
         * Send STOP command to module.
         *
         * This function is asynchronous and does not wait for response.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqStop();

        /**
         * Default handler for STOP response.
         *
         * @param[in] packet with response to STOP
         * @retval true Received command ACK in time.
         * @retval false Timeout has occurred or NACK received.
         */
        virtual bool rspStop(const DasCmdPacket *packet);

        /**
         * Send firmware upgrade packet.
         *
         * Data is sent as is to the remote side. Caller is responsible for
         * formatting, endianess etc.
         * When size is zero, modules are expected to return the current status.
         *
         * @param[in] data to be sent
         * @param[in] size of data in bytes
         * @return DasCmdPacket::CMD_UPGRADE or 0 when no packet was sent for some reason.
         */
        virtual DasCmdPacket::CommandType reqUpgrade(const char *data=0, uint32_t size=0);

        /**
         * Default handler for CMD_UPGRADE response.
         *
         * Unpacks upgrade registers.
         *
         * @retval true Response was succesfully unpacked.
         * @retval false Could not process response.
         */
        virtual bool rspUpgrade(const DasCmdPacket *packet);

        /**
         * Called when read temperature request to the module should be made.
         *
         * Base implementation simply sends a READ_TEMP command and sets up
         * timeout callback.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqReadTemperature();

        /**
         * Default handler for READ_TEMP response.
         *
         * Read the packet payload and populate temperature parameters.
         *
         * @param[in] packet with response to READ_TEMP
         * @return true if packet was parsed and temperature extracted.
         */
        virtual bool rspReadTemperature(const DasCmdPacket *packet);

        /**
         * Called when time syncronization request to the module should be made.
         *
         * Base implementation doesn't do anything.
         *
         * @return Response to wait for.
         */
        virtual DasCmdPacket::CommandType reqTimeSync() { return static_cast<DasCmdPacket::CommandType>(0); }

        /**
         * Default handler for time sync response.
         *
         * Only check for timeout.
         *
         * @param[in] packet with response to time sync
         * @return true if timeout has not yet expired, false otherwise.
         */
        virtual bool rspTimeSync(const DasCmdPacket *packet) { return false; }

        /**
         * Sets all parameters corresponding to command with the same alarm.
         */
        void setParamsAlarm(DasCmdPacket::CommandType command, int alarm);

        /**
         * Sets all parameters in section with the same alarm.
         */
        void setParamsAlarm(const std::string &section, int alarm);

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
         * Convenience function for modules that don't split status for channels.
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
        void createChanConfigParam(const char *name, uint8_t channel, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value, const BaseConvert *conv=CONV_UNSIGN);

        /**
         * Convenience function for modules that don't split configuration for channels.
         */
        void createConfigParam(const char *name, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value, const BaseConvert *conv=CONV_UNSIGN)
        {
            createChanConfigParam(name, 0, section, offset, nBits, shift, value, conv);
        }

        /**
         * Create meta-parameter
         *
         * Meta parameter is a module parameter that doesn't have corresponding
         * register. It's value is cached in software only and possibly sent to
         * connected PVs.
         */
        void createMetaConfigParam(const char *name, uint32_t nBits, int value, const BaseConvert *conv=CONV_UNSIGN);

        /**
         * Create and register single integer temperature parameter.
         *
         * Temperature values are returned in READ_TEMPERATURE response payload.
         */
        void createTempParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, const BaseConvert *conv=CONV_UNSIGN);

        /**
         * Create and register single integer upgrade parameter.
         *
         * Upgrade values are returned in CMD_UPGRATE response payload.
         */
        void createUpgradeParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift, const BaseConvert *conv=CONV_UNSIGN);

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
        virtual float noResponseCleanup(DasCmdPacket::CommandType command);

        /**
         * Schedule noResponseTimeout() function execution.
         *
         * Function expects the Plugin to be locked.
         *
         * @param[in] command Expected command response
         * @param[in] delay Delay from now when to invoke the function, in seconds.
         * @retval true if callback was scheduled
         * @retval false if callback was not scheduled
         */
        bool scheduleTimeoutCallback(DasCmdPacket::CommandType command, double delay);

        /**
         * Cancel any pending timeout callback.
         *
         * Function expects the Plugin to be locked.
         *
         * @retval true if future callback was canceled
         * @retval false if callback was already invoked and it hasn't been canceled
         */
        bool cancelTimeoutCallback();

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
        virtual bool parseVersionRspM(const DasCmdPacket *packet, BaseModulePlugin::Version &version) = 0;

        /**
         * Compare version returned from the module to the expected one.
         *
         * Only non-zero fields in expected version are compared.
         *
         * @return true when they match, false otherwise.
         */
        virtual bool checkVersion(const BaseModulePlugin::Version &version);

        /**
         * Sets the version to be matched against one returned from the module.
         *
         * Only non-zero fields in expected version are compared.
         *
         * @param[in] fw_version Firmware version number
         * @param[in] fw_revision Firmware revision number
         * @param[in] hw_version Hardware version number
         * @param[in] hw_revision Hardware revision number
         */
        virtual void setExpectedVersion(uint8_t fw_version, uint8_t fw_revision, uint8_t hw_version=0, uint8_t hw_revision=0);

        /**
         * Changes the LVDS command protocol format that this module is using.
         *
         * New LVDS protocol standardizes responses. It makes ACK/NACK responses
         * use a bit-flag and changes DISCOVER response to put module type
         * in the payload.
         *
         * @param version For now only values 0 & 1 are supported.
         */
        void setCmdVersion(uint8_t version)
        {
            m_cmdVer = version;
        }

        /**
         * Pack parameters into raw format
         *
         * Cached values of parameters are transformed into raw format suitable
         * to be sent to detector.
         *
         * @param[in] group Parameters group
         * @param[out] payload buffer to be populated
         * @param[in] size of the payload buffer in bytes
         * @param[in] channel
         * @param[in] section
         * @return Number of bytes pushed to payload or 0 on error.
         */
        size_t packRegParams(const char *group, uint32_t *payload, size_t size, uint8_t channel=0, uint8_t section=0x0);

        /**
         * Method parses packet payload and extracts parameter values.
         *
         * This generic method works for any group of parameters. For every
         * parameter in the group table, it finds the matching
         * value in the packet payload and assign it as new parameter value.
         *
         * @param[in] group of registers, like CONFIG, STATUS etc.
         * @param[in] payload to be parsed
         * @param[in] size of the payload
         * @param[in] channel expected
         */
        void unpackRegParams(const char *group, const uint32_t *payload, size_t size, uint8_t channel=0);

        /**
         * Create a generic register parameter in specified group table.
         *
         * Device registers are mapped into software cached counterpars.
         * Device register is an entity of variable bit width that is
         * connected to particular functionality. Bit widths up to 32 bits
         * are supported. No alignment is enforced.
         *
         * This function creates a mapping for one device register into
         * software table of registers.
         *
         * @param[in] group of registers, like CONFIG, STATUS etc.
         * @param[in] name of the register in software table
         * @param[in] readonly flags whether the register value can be modified in software
         * @param[in] channel addresses specific sub-fpga on device
         * @param[in] section is an offset in the register table when supported by command (only config commands)
         * @param[in] offset is a word offset from the start address, word being 2 or 4 depending on the device
         * @param[in] nBits tells number of bits used by device register
         * @param[in] shift tells bit offset within word
         * @param[in] value represents initial value for writable registers
         * @param[in] conv selects from/to raw value converter
         */
        void createRegParam(const char *group, const char *name, bool readonly, uint8_t channel, uint8_t section, uint16_t offset, uint8_t nBits, uint8_t shift, uint32_t value=0, const BaseConvert *conv=CONV_UNSIGN);

        /**
         * Link existing parameter to upgrade parameters table.
         *
         * Useful when two hardware registers in two different response types
         * have the same meaning and need to be merged into one plugin parameter.
         * When either of the responses is received, only one parameter gets
         * updated.
         *
         * @param[in] group of registers, like CONFIG, STATUS etc.
         * @param[in] name of the register in software table
         * @param[in] readonly flags whether the register value can be modified in software
         * @param[in] channel addresses specific sub-fpga on device
         * @param[in] section is an offset in the register table when supported by command (only config commands)
         * @param[in] offset is a word offset from the start address, word being 2 or 4 depending on the device
         * @param[in] nBits tells number of bits used by device register
         * @param[in] shift tells bit offset within word
         */
        void linkRegParam(const char *group, const char *name, bool readonly, uint8_t channel, uint8_t section, uint16_t offset, uint8_t nBits, uint8_t shift);

        /**
         * Initialize parameters tables.
         *
         * Calculates offsets and sizes of the sections that are needed when
         * receiving and sending packets. Must be called after any call to
         * create*Param but before any packet can be received or sent. The
         * safest is to put it in plugin constructor.
         *
         * It also populates Features parameter from detected features or
         * set by derived class and finally invoked callParamCallbacks().
         *
         * Every derived class should call this function from its constructor
         * just before return.
         */
        void initParams();

        /**
         * Convert command into a string.
         *
         * @parma[in] command to be converted.
         * @return String describing the command.
         */
        const char *cmd2str(DasCmdPacket::CommandType command);

        /**
         * Register callback function for the handling responses internally.
         *
         * Internal response handling allows arbitrary function to get invoked
         * before handling the response the regular way. All communication to
         * the connected module is transparent to the rest of the BaseModulePlugin
         * functionality and is complete responsibility of the registered
         * handler. However, this allows custom state machines to be implemented
         * in nED infrastructure. That can be more efficient that external
         * state machine using sequencer for example, and more importantly it
         * can be easily parallelized with asynchronoucity of nED.
         *
         * The state machine needs to register a single entry function that is
         * invoked whenever a response from the module is received. Function
         * needs to return true if it handled the response and further processing
         * of response stops. In other words, if response was expected by the
         * state machine noone else should process it. Response handler functions
         * are called in the order they're registered. There's no mechanism to
         * prevent overlap in case - it's strictly first come first served.
         */
        void registerResponseHandler(std::function<bool(const DasCmdPacket *)> &callback);

        /**
         * Returns a name of the module from the global database.
         *
         * Whenever a module is registered, it makes a record in a global table
         * that maps its hardwareId to the name.
         * There's the implied thread safety guarantee that the table is only
         * populated on startup when modules are created.
         *
         * @param[in] hardwareId unique id.
         * @return Name of the module or empty string.
         */
        static std::string getModuleName(uint32_t hardwareId);

        /**
         * Return a list of all registered module names.
         */
        static void getModuleNames(std::list<std::string> &modules);

        /**
         * Returns current hardware id.
         */
        uint32_t getHardwareId() { return m_hardwareId; }

    private: // functions
        /**
         * Trigger calculating the configuration parameter offsets.
         */
        void recalculateConfigParams();

        /**
         * Check if connection to parent plugins can be closed and close it.
         *
         * The idea behind automatic connect/disconnect to parent plugins is to
         * not process packets (and save CPU time) while we're not expecting them.
         * This is particularly important for installations with large number
         * of modules.
         *
         * Function is called periodically. To minimize establishing
         * connection to parent plugins during busy periods, function waits 0.5s
         * longer than either response is received or no-response timer expires.
         * This gives external sequences enough time to issue new command and
         * not disconnect from parent plugins.
         */
        float checkConnection();

        /**
         * Save all configuration PVs to a file.
         *
         * Filename is based on the name parameter, with a '.sav' suffix. File is
         * saved to a pre-defined folder.
         */
        bool saveConfig(const std::string &name);

        /**
         * Load configuration PVs from a file and populate '_Saved' counterparts.
         *
         * Filename is based on the name parameter, with a '.sav' suffix. File is
         * saved to a pre-defined folder.
         */
        bool loadConfig(const std::string &name);

        /**
         * Copies configuration PV values from '_Saved' counterparts.
         *
         */
        void copyConfig();

        /**
         * Determine configuration file path.
         *
         * If existing is true, file must exist. Returns empty string if path doesn't exist.
         */
        std::string getConfigPath(const std::string &name, bool existing);

        /**
         * Parse filename (not path) and return configuration name.
         *
         * Return empty string if filename can not be parsed or is not for this module.
         */
        std::string parseConfigName(const std::string &filename);

        /**
         * Return list of valid configurations for this module.
         *
         * List is sorted in order of creation, newest first.
         */
        std::list<std::string> getListConfigs();

    protected:
        int Enable;         //!< Enable this module
        int Features;       //!< Bit-mask of supported features (see BaseModulePlugin::Features)
        int CmdReq;         //!< Command to plugin, like initialize the module, read configuration, verify module version etc.
        int CmdRsp;         //!< Last command response status
        int HwId;           //!< Hw ID that this object is controlling
        int HwType;         //!< Configured module type
        int HwVer;          //!< Module hardware version
        int HwRev;          //!< Module hardware revision
        int HwExpectVer;    //!< Module hardware version
        int HwExpectRev;    //!< Module hardware revision
        int HwDate;         //!< Module hardware date
        int FwVer;          //!< Module firmware version
        int FwRev;          //!< Module firmware revision
        int FwExpectVer;    //!< Module firmware version
        int FwExpectRev;    //!< Module firmware revision
        int FwDate;         //!< Module firmware date
        int Supported;      //!< Flag whether module is supported
        int Verified;       //!< Hardware id, version and type all verified
        int CfgSection;     //!< Selected configuration section to be written
        int CfgChannel;     //!< Selected channel to be configured
        int NoRspTimeout;   //!< Time to wait for response
        int SaveConfig;     //!< Save configuration to file
        int LoadConfig;     //!< Load configuration from file
        int CopyConfig;     //!< Copy configuration from saved
        int Config1;        //!< Name of newest config
        int Config2;        //!< Name of 2nd newest config
        int Config3;        //!< Name of 3rd newest config
        int Config4;        //!< Name of 4th newest config
        int Config5;        //!< Name of 5th newest config
        int ConfigSaved;    //!< Flag =1 when all the PVs are in config control
        int ConfigApplied;  //!< Flag =1 when all PVs are synchronized to module
};

#endif // BASE_MODULE_PLUGIN_H
