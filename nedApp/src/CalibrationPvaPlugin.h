#ifndef CALIBRATION_PVA_PLUGIN_H
#define CALIBRATION_PVA_PLUGIN_H

#include "BasePvaPlugin.h"

/**
 * Plugin that forwards Raw data to calibration software over EPICS v4.
 *
 * CalibrationPlugin provides following asyn parameters:
 * asyn param    | asyn param type | init val | mode | Description
 * ------------- | --------------- | -------- | ---- | -----------
 */
class CalibrationPvaPlugin : public BasePvaPlugin {
    private:
        uint64_t m_nTransmitted;    //!< Number of packets sent to BASESOCKET
        uint64_t m_nProcessed;      //!< Number of processed packets
        uint64_t m_nReceived;       //!< Number of packets received from dispatcher
    public:
        /**
         * Constructor
	     *
	     * @param[in] portName            asyn port name.
	     * @param[in] dispatcherPortName  Name of the dispatcher asyn port to connect to.
	     * @param[in] pvPrefix            Prefix for the PV Record
         */
        CalibrationPvaPlugin(const char *portName, const char *dispatcherPortName, const char *pvPrefix);

	/**
         * Process incoming data as raw ACPC neutron data.
         *
	 * @param[in] packetList List of packets to be processed
         */
        virtual void processDataRaw(const DasPacketList * const packetList);
};

#endif // CALIBRATION_PVA_PLUGIN_H
