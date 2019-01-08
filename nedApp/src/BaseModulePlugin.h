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
 * Base class providing communication to one module.
 *
 * This class provides functionality common to most if not all DAS
 * modules like DSP, FEM, ROCs, ACPC etc. It handles command based
 * comminication with the modules, provides status and configuration
 * PVs and does some book-keeping. It's an abstract class that must
 * be extended with module specific implementation. One instance per
 * module must be created.
 *
 * Starting with DSP7 there's new optical protocol. DSP7 converts
 * legacy protocol to new one on the fly so the software can use
 * only new protocol. If DSP prior to DSP7 is used, the
 * OccPortDriver will do the conversion.
 *
 * Basic process flow is for user (through CmdReq PV) or derived
 * class (from code) to send a request using a processRequest()
 * function. Using the table of handlers, a request handler is
 * found and executed. Derived classes can install their own
 * handlers by populating m_cmdHandlers table for a given command.
 * The handler must generate a packet, send it using
 * sendDownstream() and return true. processRequest() will then
 * install timer for the corresponding response and change CmdRsp
 * PV value to waiting. Once response is received, processResponse()
 * is invoked which finds the response handler based on command
 * from the packet and invokes it. It then changes CmdRsp to either
 * success or error depending on return code of the response handler.
 * If no response is received in given time, CmdRsp changes to
 * timeout.
 *
 * Multiple requests are queued and executed in order scheduled as
 * soon as previous response is received or timeout occurs. Connection
 * to parent modules is automatically managed. It connects when
 * sending a packet and disconnects after certain timeout. Keeping
 * connection closed saves CPU resources and provides faster operation.
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
            const BaseConvert* convert; //!< Selected from/to raw value converter
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

    public: // variables
        static const int defaultInterfaceMask = BasePlugin::defaultInterfaceMask | asynOctetMask | asynFloat64Mask;
        static const int defaultInterruptMask = BasePlugin::defaultInterruptMask | asynOctetMask | asynFloat64Mask;
        static const UnsignConvert *CONV_UNSIGN;
        static const Sign2sComplementConvert *CONV_SIGN_2COMP;
        static const SignMagnitudeConvert *CONV_SIGN_MAGN;
        static const Hex2DecConvert *CONV_HEX2DEC;
        static const Hex2DecConvert2K *CONV_HEX2DEC2K;

    protected: // variables
        uint32_t m_features;
        uint32_t m_hardwareId;                          //!< Hardware ID which this plugin is connected to
        std::map<std::string, ParamTable> m_params;     //!< Maps of exported parameters
        DasCmdPacket::CommandType m_waitingResponse;    //!< Expected response code while waiting for response or timeout event, 0 otherwise
        uint8_t m_cfgSectionCnt;                        //!< Used with sending channels configuration, tells number of times this section succeeded for previous channels
        uint8_t m_cmdQueueSize{10};                     //!< Max size of m_cmdQueue

        typedef std::function<bool()> RequestHandler;
        typedef std::function<bool(const DasCmdPacket*)> ResponseHandler;
        std::vector<std::pair<RequestHandler,ResponseHandler>> m_cmdHandlers; //!< 256 command/response handlers

    private: // variables
        uint8_t m_wordSize;                             //!< Register word size.
        uint8_t m_cmdVer = 0;                           //!< Commands protocol version
        std::map<int, uint32_t> m_configSectionSizes;   //!< Configuration section sizes, in words (word=2B for submodules, =4B for DSPs)
        std::map<int, uint32_t> m_configSectionOffsets; //!< Status response payload size, in words (word=2B for submodules, =4B for DSPs)
        Timer m_noRspTimer{true};                       //!< Currently running timer for response timeout handling
        epicsTime m_connLastActive;                     //!< Time when last request has been sent out, for connection book-keeping purposes
        Timer m_connTimer{true};                        //!< Periodic timer to check whether connection can be closed.
        std::string m_parentPlugins;                    //!< Parent plugins to connect to
        std::string m_configDir{""};                    //!< Directory where to save all configurations
        std::list<DasCmdPacket::CommandType> m_cmdQueue;//!< FIFO queue of commands, max 10 in queue

    public: // functions

        /**
         * Constructor for BaseModulePlugin
         *
         * Constructor will create and populate PVs with default values.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] configDir Folder to store module configurations.
         * @param[in] wordSize Number of bytes describing single register word
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         * @param[in] numParams The number of parameters that the derived class supports.
         * @param[in] interfaceMask Bit mask defining the asyn interfaces that this driver supports.
         * @param[in] interruptMask Bit mask definining the asyn interfaces that can generate interrupts (callbacks)
         */
        BaseModulePlugin(const char *portName, const char *parentPlugins, const char *configDir,
                         uint8_t wordSize, int interfaceMask=0, int interruptMask=0);

        /**
         * Abstract destructor
         */
        virtual ~BaseModulePlugin() = 0;

        /**
         * Write octet parameter value.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual) override;

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
         * Function iterates through received packets and silently skipping
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
         *
         * Function supports channel specific status and config responses.
         *
         * @param[in] packet to be processed.
         * @return true if packet has been processed, false otherwise
         */
        virtual bool processResponse(const DasCmdPacket *packet);

        /**
         * Schedule a new request to the module.
         *
         * Plugins should always use this function.
         * It ensures multiple requests are serialized and tracked, with a proper
         * clean up action.
         *
         * @param[in] command to be sent
         * @return true if request has been handled, false otherwise
         */
         bool processRequest(DasCmdPacket::CommandType command);

        /**
         * Send as many requests as possible
         *
         * Send requests from queue. Stop on first request that requires
         * a response.
         * Connection to parent plugin(s) is automatically established.
         * Update command related PVs such as CmdReq and CmdRsp.
         * This is invoked from processRequest() on new request,
         * and processResponse() and noResponseTimeout() to check if there's
         * any queued requests to be made.
         */
        void processQueuedRequests();

        /**
         * Generic request handler.
         *
         * This generic handler is invoked when command doesn't have any specific
         * payload to be sent along. It simply creates a new request packet for
         * a given command and sends it out.
         */
        bool reqSimple(DasCmdPacket::CommandType command);

        /**
         * Generic response handler.
         *
         * This generic handler is invoked when response packet is received and
         * no particular action is required except to check the ACKNOWLEDGE flag.
         */
        bool rspSimple(const DasCmdPacket *packet);

        /**
         * Generic request handler with payload from parameters.
         *
         * This generic handler is invoked when command wants to send registered
         * parameter as payload. It creates a new request packet for a given command,
         * selects the registers and packs them into the payload. Finally it send
         * the command out.
         */
        bool reqParams(DasCmdPacket::CommandType command, const std::string& params);

        /**
         * Generic response handler with payload from parameters.
         *
         * This generic handler is invoked when response packet is received with
         * new parameter values in payload. Parameters are extracted from payload
         * and their PVs are populated unless packet is NOT ACKNOWLEDGE.
         */
        bool rspParams(const DasCmdPacket *packet, const std::string& params);

        /**
         * Upon succesful response it populates ConfigApplied PV.
         */
        bool rspWriteConfig(const DasCmdPacket* packet);

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
        void createStatusParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift);

        /**
         * Create and register single integer status counter parameter.
         *
         * Status counters provide a way to diagnose communication problems.
         */
        void createCounterParam(const char *name, uint32_t offset, uint32_t nBits, uint32_t shift);

        /**
         * Create and register single integer config parameter.
         */
        void createConfigParam(const char *name, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value, const BaseConvert *conv=CONV_UNSIGN);

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
         * @return true if the timeout function did the cleanup, that is received was *not* received.
         */
        virtual float noResponseCleanup();

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
        bool scheduleTimeoutCallback(double delay);

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
        size_t packRegParams(const std::string& group, uint32_t *payload, size_t size, uint8_t channel=0, uint8_t section=0x0);

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
        void unpackRegParams(const std::string& group, const uint32_t *payload, size_t size, uint8_t channel=0);

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
         * Convenience function that removes channel and section parameters from its signature.
         *
         * @param[in] group of registers, like CONFIG, STATUS etc.
         * @param[in] name of the register in software table
         * @param[in] readonly flags whether the register value can be modified in software
         * @param[in] offset is a word offset from the beginning of this group, word being 2 or 4 depending on the device
         * @param[in] nBits tells number of bits used by device register
         * @param[in] shift tells bit offset within word
         * @param[in] value represents initial value for writable registers
         * @param[in] conv selects from/to raw value converter
         */
        void createRegParam(const char *group, const char *name, bool readonly, uint16_t offset, uint8_t nBits, uint8_t shift, uint32_t value=0, const BaseConvert *conv=CONV_UNSIGN)
        {
            createRegParam(group, name, readonly, 0, 0, offset, nBits, shift, value, conv);
        }

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
        void initParams(uint32_t nChannels=0);

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
        int CfgSection;     //!< Selected configuration section to be written
        int CfgChannel;     //!< Selected channel to be configured
        int NoRspTimeout;   //!< Time to wait for response
        int ConnValidDly;   //!< Time to wait before disconnecting from parent plugin
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
