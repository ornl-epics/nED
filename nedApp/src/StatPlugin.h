#ifndef STAT_PLUGIN_H
#define STAT_PLUGIN_H

#include "BasePlugin.h"

/**
 * Gather and display statistical information of the incoming data
 *
 * Available StatPlugin parameters (in addition to ones from BasePlugin):
 * asyn param    | asyn param type | init val | mode | Description                   |
 * ------------- | --------------- | -------- | ---- | ------------------------------
 * Reset         | asynParamInt32  | 0        | RW   | Reset counters
 * CmdCnt        | asynParamInt32  | 0        | RO   | Number of command response packets
 * DataCnt       | asynParamInt32  | 0        | RO   | Number of data packets
 * MetaCnt       | asynParamInt32  | 0        | RO   | Number of data packets
 * RtdlCnt       | asynParamInt32  | 0        | RO   | Number of RTDL packets
 * TsyncCnt      | asynParamInt32  | 0        | RO   | Number of TSYNC packets
 * BadCnt        | asynParamInt32  | 0        | RO   | Number of bad packets
 * TotCnt        | asynParamInt32  | 0        | RO   | Total number of packets
 * CmdByte       | asynParamInt32  | 0        | RO   | Bytes of command response packets
 * DataByte      | asynParamInt32  | 0        | RO   | Bytes of data packets
 * MetaByte      | asynParamInt32  | 0        | RO   | Bytes of data packets
 * RtdlByte      | asynParamInt32  | 0        | RO   | Bytes of RTDL packets
 * TsyncByte     | asynParamInt32  | 0        | RO   | Bytes of TSYNC packets
 * BadByte       | asynParamInt32  | 0        | RO   | Bytes of bad packets
 * TotByte       | asynParamInt32  | 0        | RO   | Total number of packets
 */
class StatPlugin : public BasePlugin {
    public: // functions
        /**
         * Constructor
         *
	     * @param[in] portName asyn port name.
	     * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        StatPlugin(const char *portName, const char *dispatcherPortName, int blocking);

        /**
         * Overloaded function to receive all OCC data.
         */
        void processData(const DasPacketList * const packetList);

    private:
        uint64_t m_receivedCount;
        uint64_t m_receivedBytes;
        uint64_t m_cmdCount;
        uint64_t m_cmdBytes;
        uint64_t m_dataCount;
        uint64_t m_dataBytes;
        uint64_t m_metaCount;
        uint64_t m_metaBytes;
        uint64_t m_rtdlCount;
        uint64_t m_rtdlBytes;
        uint64_t m_tsyncCount;
        uint64_t m_tsyncBytes;
        uint64_t m_badCount;
        uint64_t m_badBytes;

    private: // asyn parameters
        #define FIRST_STATPLUGIN_PARAM Reset
        int Reset;              //!< Reset counters
        int CmdCnt;             //!< Number of command response packets
        int DataCnt;            //!< Number of data packets
        int MetaCnt;            //!< Number of data packets
        int RtdlCnt;            //!< Number of RTDL packets
        int TsyncCnt;           //!< Number of TSYNC packets
        int BadCnt;             //!< Number of bad packets
        int TotCnt;             //!< Total number of packets
        int CmdByte;            //!< Bytes of command response packets
        int DataByte;           //!< Bytes of data packets
        int MetaByte;           //!< Bytes of data packets
        int RtdlByte;           //!< Bytes of RTDL packets
        int TsyncByte;          //!< Bytes of TSYNC packets
        int BadByte;            //!< Bytes of bad packets
        int TotByte;            //!< Total number of packets
        #define LAST_STATPLUGIN_PARAM TotByte
};

#endif // STAT_PLUGIN_H
