#ifndef CALIBRATION_PLUGIN_H
#define CALIBRATION_PLUGIN_H

#include "BaseSocketPlugin.h"

#include <shareLib.h>

#include <pv/pvDatabase.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/standardPVField.h>

class RawCalDataPVRecord : public epics::pvDatabase::PVRecord
{
public:
	POINTER_DEFINITIONS(RawCalDataPVRecord);

	// factory method
	static RawCalDataPVRecord::shared_pointer
		create(std::string const & recordName);

	virtual void process();

	void update(uint posIndex,
		ushort sample1A, ushort sample1B,
		ushort sample2A, ushort sample2B);
		
	virtual bool init();

private:
	RawCalDataPVRecord(std::string const & recordName,
		epics::pvData::PVStructurePtr const & pvStructure);

	// records time of last process() call
	epics::pvData::TimeStamp	timeStamp;

	// RAW Mode ROC output data
	epics::pvData::PVTimeStamp	pvTimeStamp;
	epics::pvData::PVUIntPtr	pvPosIndex;
	epics::pvData::PVUShortPtr	pvSample1A;
	epics::pvData::PVUShortPtr	pvSample1B;
	epics::pvData::PVUShortPtr	pvSample2A;
	epics::pvData::PVUShortPtr	pvSample2B;

	// used for EXTENDED RAW format with calculated position
	epics::pvData::PVUShortPtr	pvPixelID;
};


/**
 * Plugin that forwards Raw data to calibration software over EPICS v4.
 *
 * CalibrationPlugin provides following asyn parameters:
 * asyn param    | asyn param type | init val | mode | Description
 * ------------- | --------------- | -------- | ---- | -----------
 */

class CalibrationPlugin : public BasePlugin {
    private:
        uint64_t m_nTransmitted;    //!< Number of packets sent to BASESOCKET
        uint64_t m_nProcessed;      //!< Number of processed packets
        uint64_t m_nReceived;       //!< Number of packets received from dispatcher
	RawCalDataPVRecord::shared_pointer raw_cal_record;
    public:
        /**
         * Constructor
	     *
	     * @param[in] portName asyn port name.
	     * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
	     * @param[in] blocking Should processing of callbacks block execution of caller thread or not.
         */
        CalibrationPlugin(const char *portName, const char *dispatcherPortName, int blocking);

        /**
         * Destructor
         */
        virtual ~CalibrationPlugin();

        /**
         * Send all packets directly to client socket, if connected.
         *
         * @param[in] packetList List of packets to be processed
         */
        virtual void processData(const DasPacketList * const packetList);
};


#endif // CALIBRATION_PLUGIN_H
