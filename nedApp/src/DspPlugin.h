#ifndef DSP_PLUGIN_H
#define DSP_PLUGIN_H

#include "BaseModulePlugin.h"

/**
 * Plugin for DSP module.
 *
 * General plugin parameters:
 * asyn param name      | asyn param index | asyn param type | init val | mode | Description
 * -------------------- | ---------------- | --------------- | -------- | ---- | -----------
 * HARDWARE_ID          | HardwareId       | asynParamInt32  | 0        | RO   | Hardware ID of the module connected to
 * HARDWARE_VER         | HardwareVer      | asynParamInt32  | 0        | RO   | Hardware version
 * HARDWARE_REV         | HardwareRev      | asynParamInt32  | 0        | RO   | Hardware revision
 * HARDWARE_DATE        | HardwareDate     | asynOctet       | ""       | RO   | Hardware date
 * FIRMWARE_VER         | FirmwareVer      | asynParamInt32  | 0        | RO   | Number of packets processed, to be populated by derived classes
 * STATUS               | Status           | asynParamInt32  | 0        | RO   | Status of DSP plugin (see DspPlugin::Status for available options)
 * COMMAND              | Command          | asynParamInt32  | 0        | RW   | Issue a command for this plugin (see DspPlugin::Command for available options)
 */
class DspPlugin : public BaseModulePlugin {
    private: // structures and definitions
        struct ParamDesc {
            char section;           //!< Section name
            uint32_t offset;        //!< An 4-byte offset within the section
            uint32_t mask;          //!< The mask specifies allowed range of values and position within the parameter bitfield, for example mask 0x30 allows values in range 0-3 and the actual value will be shifted by 4 when put in section parameter
            int initVal;            //!< Initial value after object is created or configuration reset is being requested
        };

        struct VersionRegister {
#ifdef BITFIELD_LSB_FIRST
            unsigned day:8;
            unsigned month:8;
            unsigned year:8;
            unsigned revision:4;
            unsigned version:4;
#else
#error Missing DspVersionRegister declaration
#endif
        };

        /**
         * Response payload to READ_VERSION command.
         */
        struct RspVersion {
            struct VersionRegister hardware;
            struct VersionRegister firmware;
            uint32_t eeprom_code;
        };

        /**
         * Valid statuses of the DspPlugin, the communication link with Dsp or the DSP itself.
         */
        enum Status {
            STAT_NOT_INITIALIZED    = 0,    //!< DspPlugin has not yet been initialized
            STAT_DSP_TIMEOUT        = 10,   //!< DSP has not respond in expected time to the last command
        };

        /**
         * Valid commands to be send through COMMAND parameter.
         */
        enum Command {
            DSP_CMD_NONE            = 0,
            DSP_CMD_INITIALIZE      = 1,    //!< Trigger DSP module initialization
            DSP_CMD_CONFIG_WRITE    = 2,    //!< Write current configuration to the DSP module
            DSP_CMD_CONFIG_READ     = 3,    //!< Read actual configuration from DPS module and populate PVs accordingly
            DSP_CMD_CONFIG_RESET    = 4,    //!< Reset configuration to default values
        };

        static const unsigned NUM_DSPPLUGIN_CONFIGPARAMS;   //!< This is used as a runtime assert check and must match number of configuration parameters
        static const unsigned NUM_DSPPLUGIN_STATUSPARAMS;   //!< This is used as a runtime assert check and must match number of status parameters
        static const double DSP_RESPONSE_TIMEOUT;           //!< Default DSP response timeout, in seconds

    public:

        /**
         * Constructor for DspPlugin
         *
         * Constructor will create and populate PVs with default values.
         *
	     * @param[in] portName asyn port name.
	     * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
	     * @param[in] hardwareId Hardware ID of the DSP module, can be in IP format (xxx.xxx.xxx.xxx) or
         *                       in hex number string in big-endian byte order (0x15FACB2D equals to IP 21.250.203.45)
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        DspPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, int blocking);

    private:
        std::map<int, struct ParamDesc> m_configParams;

        /**
         * Initialize module by requesting it's version information.
         *
         * When the module responds, populate the Hardware version and
         * Firmware parameters.
         *
         * This function is asynchronous but does not wait for response.
         */
        void reqVersionRead();

        /**
         * Handle READ_VERSION from DSP.
         *
         * @param[in] packet DAS packet with verified READ_VERSION response.
         */
        void rspVersionRead(const DasPacket *packet);

        /**
         * Construct configuration data and send it to module.
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
         * This function is asynchronous but does not wait for response.
         */
        void reqCfgWrite();

        /**
         * Read configuration from DSP module and populate PV values.
         *
         * It will only send a configuration read command to the DSP module,
         * but not wait for response.
         *
         * This function is asynchronous but does not wait for response.
         */
        void reqCfgRead();

        /**
         * Handle READ_CONFIG response from DSP.
         *
         * Populate all PVs with the values from current DSP configuration.
         *
         * @param[in] packet DAS packet with verified READ_CONFIG response.
         */
        void rspCfgRead(const DasPacket *packet);

        /**
         * Reset configuration PVs to initial values.
         */
        void cfgReset();

        /**
         * Overloaded function called by asynPortDriver when the PV should change value.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded function called by asynPortDriver to get PV value.
         */
        asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);

        /**
         * Process incoming packets.
         */
        void processData(const DasPacketList * const packetList);

        /**
         * Create and register all configuration parameters to be exposed to EPICS.
         */
        void createConfigParams();

        /**
         * Create and register single integer configuration parameter.
         */
        void createConfigParam(const char *name, char section, uint32_t offset, uint32_t nBits, uint32_t shift, int value);
        using asynPortDriver::createParam;

        /**
         * Create and register all status parameters to be exposed to EPICS.
         */
        void createStatusParams();

        /**
         * Setup the expected next response and a timeout routine.
         *
         * When the next command is received, it must be the one specified in
         * command parameter. In that case the callback is invoked.
         * If the received command does not match expected one, error is
         * reported and next expected command is cleared.
         * If no command is received during the specified time, the
         * noRespondeCleanup() function is called.
         *
         * @param[in] cmd Command to wait for.
         * @param[in] cb Callback to run when command is received.
         * @param[in] timeout The time to wait for response before calling the cleanup function.
         */
        void expectResponse(DasPacket::CommandType cmd, std::function<void(const DasPacket *)> &cb, double timeout=DSP_RESPONSE_TIMEOUT);

        /**
         * Cleanup function called from Timer.
         *
         * It gets called in any case, whether the response ... TODO
         */
        void noResponseCleanup(DasPacket::CommandType cmd);

        /**
         * Construct particular section of configuration data.
         */
        uint32_t configureSection(char section, uint32_t *data, uint32_t count);

        /**
         * Return size of particular configuration section.
         *
         * @param[in] section Requested section
         * @return Size of the section in number of records.
         */
        uint32_t getCfgSectionSize(char section);

        /**
         * Return the absolute offset of the configuration section within all configuration structure.
         */
        uint32_t getCfgSectionOffset(char section);

    private:
        #define FIRST_DSPPLUGIN_PARAM Status
        int Status;         //!< Status of the DSP plugin
        int Command;        //!< Command to plugin, like initialize the module, read configuration, verify module version etc.
        int HardwareId;     //!< Hardare ID that this object is controlling
        int HardwareVer;    //!< Module hardware version
        int HardwareRev;    //!< Module hardware revision
        int HardwareDate;   //!< Module hardware date
        int FirmwareVer;    //!< Module firmware version
        int FirmwareRev;    //!< Module firmware revision
        int FirmwareDate;   //!< Module firmware date
        #define LAST_DSPPLUGIN_PARAM FirmwareDate

};

#endif // DSP_PLUGIN_H
