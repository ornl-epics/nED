/* BnlFlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BnlDataPacket.h"
#include "BnlFlatFieldPlugin.h"
#include "Common.h"
#include "Log.h"

#include <cstring>
#include <likely.h>
#include <limits>
#include <math.h>

EPICS_REGISTER_PLUGIN(BnlFlatFieldPlugin, 5, "Port name", string, "Dispatcher port name", string, "Parameter file path", string, "Values file path", string, "Buffer size in bytes", int);

#define NUM_BNLFLATFIELDPLUGIN_PARAMS      ((int)(&LAST_BNLFLATFIELDPLUGIN_PARAM - &FIRST_BNLFLATFIELDPLUGIN_PARAM + 1)) + (20 + 17)*2

BnlFlatFieldPlugin::BnlFlatFieldPlugin(const char *portName, const char *dispatcherPortName, const char *paramFile, const char *valFile, int bufSize)
    : BaseDispatcherPlugin(portName, dispatcherPortName, 1, NUM_BNLFLATFIELDPLUGIN_PARAMS, asynOctetMask, asynOctetMask)
    , m_correctionScale(1.0)
{
    ImportError err = MAP_ERR_NONE;

    if (bufSize > 0) {
        m_bufferSize = bufSize;
    } else {
        m_bufferSize = 4*1024*1024;
        LOG_INFO("Setting default buffer size to %u bytes", m_bufferSize);
    }
    m_buffer = reinterpret_cast<uint8_t *>(malloc(m_bufferSize));
    if (m_buffer == 0) {
        m_bufferSize = 0;
        err = MAP_ERR_NO_MEM;
    }

    if (err == MAP_ERR_NONE) {
        if (loadParamFile(paramFile) == false)
            err = MAP_ERR_PARAMS;
        if (loadValueFile(valFile) == false)
            err = MAP_ERR_VALUES;
    }

    createParam("ParamsPath",   asynParamOctet, &ParamsPath, paramFile);      // Path to parameters file
    createParam("ValPath",      asynParamOctet, &ValPath, valFile);           // Path to correction values file
    createParam("ErrImport",    asynParamInt32, &ErrImport, err);             // Last mapping import error
    createParam("XyFractWidth", asynParamInt32, &XyFractWidth, 11);           // WRITE - Number of fraction bits in X,Y data
    createParam("CntVetoEvents",asynParamInt32, &CntVetoEvents, 0);           // Number of vetoed events
    createParam("CntGoodEvents",asynParamInt32, &CntGoodEvents, 0);           // Number of calculated events
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);               // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);                   // Reset counters
    createParam("ProcessMode",  asynParamInt32, &ProcessMode, MODE_PASSTHRU); // Event correction mode
    createParam("PixelRes",     asynParamInt32, &PixelRes, 8);                // How many bits to use for X,Y resolution
    callParamCallbacks();
}

asynStatus BnlFlatFieldPlugin::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    if (pasynUser->reason == ResetCnt) {
        setIntegerParam(RxCount, 0);
        setIntegerParam(ProcCount, 0);
        setIntegerParam(CntVetoEvents, 0);
        setIntegerParam(CntGoodEvents, 0);
        setIntegerParam(CntSplit, 0);
        return asynSuccess;
    }
    return BasePlugin::writeInt32(pasynUser, value);
}

void BnlFlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nSplits = 0;
    int nVeto = 0;
    int nGood = 0;
    int processMode = 0;
    float xyDivider = 1;
    int val;

    this->lock();
    getIntegerParam(RxCount,        &nReceived);
    getIntegerParam(ProcCount,      &nProcessed);
    getIntegerParam(CntVetoEvents,  &nVeto);
    getIntegerParam(CntGoodEvents,  &nGood);
    getIntegerParam(CntSplit,       &nSplits);
    getIntegerParam(ProcessMode,    &processMode);
    if (getDataMode() != BasePlugin::DATA_MODE_NORMAL || !m_tableX.initialized || !m_tableY.initialized)
        processMode = MODE_PASSTHRU;
    getIntegerParam(XyFractWidth,   &val);
    if (val < 0)       xyDivider = 1 << 0;
    else if (val > 15) xyDivider = 1 << 15;
    else               xyDivider = 1 << val;
    getIntegerParam(PixelRes,       &val);
    if (val < 0)       m_pixelRes = 8;
    else if (val > 14) m_pixelRes = 15;
    else               m_pixelRes = val;
    this->unlock();

    // Optimize pass thru mode
    if (processMode == MODE_PASSTHRU) {
        m_packetList.reset(packetList); // reset() automatically reserves
        sendToPlugins(&m_packetList);
        m_packetList.release();
        m_packetList.waitAllReleased();
        m_packetList.clear();
        nProcessed += packetList->size();
    } else {
        // Break single loop into two parts to have single point of sending data
        for (auto it = packetList->cbegin(); it != packetList->cend(); ) {
            uint32_t bufferOffset = 0;
            m_packetList.clear();
            m_packetList.reserve();

            for (; it != packetList->cend(); it++) {
                const DasPacket *packet = *it;

                // If running out of space, send this batch
                uint32_t remain = m_bufferSize - bufferOffset;
                if (remain < packet->length()) {
                    nSplits++;
                    break;
                }

                nProcessed++;

                // Reuse the original packet if nothing to map
                if (packet->isNeutronData() == false) {
                    m_packetList.push_back(packet);
                    continue;
                }

                // Reserve part of buffer for this packet, it may shrink from original but never grow
                DasPacket *newPacket = reinterpret_cast<DasPacket *>(m_buffer + bufferOffset);
                m_packetList.push_back(newPacket);
                bufferOffset += packet->length();

                // Process the packet - only raw mode supported for now
                processPacket(packet, newPacket, xyDivider, (ProcessMode_t)processMode, nGood, nVeto);
            }

            sendToPlugins(&m_packetList);
            m_packetList.release();
            m_packetList.waitAllReleased();
        }
    }

    this->lock();
    setIntegerParam(RxCount,    nReceived   % std::numeric_limits<int32_t>::max());
    setIntegerParam(ProcCount,  nProcessed  % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntSplit,   nSplits     % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntVetoEvents, nVeto    % std::numeric_limits<int32_t>::max());
    setIntegerParam(CntGoodEvents, nGood    % std::numeric_limits<int32_t>::max());
    callParamCallbacks();
    this->unlock();
}

void BnlFlatFieldPlugin::processPacket(const DasPacket *srcPacket, DasPacket *destPacket, float xyDivider, ProcessMode_t processMode, int &nGood, int &nVeto)
{
    bool convert = (processMode == MODE_CONVERT || processMode == MODE_CORRECT_CONVERT);
    bool correct = (processMode == MODE_CORRECT || processMode == MODE_CORRECT_CONVERT);

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents;
    const BnlDataPacket::NormalEvent *srcEvent= reinterpret_cast<const BnlDataPacket::NormalEvent *>(srcPacket->getData(&nEvents));
    nEvents /= (sizeof(BnlDataPacket::NormalEvent) / sizeof(uint32_t));

    uint32_t nDestEvents;
    uint32_t *newPayload = reinterpret_cast<uint32_t *>(destPacket->getData(&nDestEvents));
    nDestEvents = 0;

    uint32_t pixelResMask = (1 << m_pixelRes) - 1;

    while (nEvents-- > 0) {
        double x = srcEvent->x / xyDivider;
        double y = srcEvent->y / xyDivider;
        bool vetoed = false;

        if (likely(correct == true)) {
            vetoed = (correctPosition(&x, &y) == false);
        }

        if (vetoed == false) {
            if (likely(convert == true)) {
                DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(newPayload);

                destEvent->tof = srcEvent->tof & 0xFFFFFFF; // TODO: +timeoffset?
                destEvent->pixelid = (srcEvent->position & 0x3) << 28; // incompatible with dcomserver, he shifts for 16 or 18 bits depending on HighRes
                destEvent->pixelid |= ((uint16_t)(16 * x + 0.5) & ((1 << (m_pixelRes+1)) - 1)) << m_pixelRes;
                destEvent->pixelid |= ((uint16_t)(16 * y + 0.5) & ((1 << m_pixelRes) - 1));

                newPayload += sizeof(DasPacket::Event) / sizeof(uint32_t);
                destPacket->payload_length += sizeof(DasPacket::Event);
            } else {
                BnlDataPacket::NormalEvent *destEvent = reinterpret_cast<BnlDataPacket::NormalEvent *>(newPayload);

                destEvent->tof = srcEvent->tof & 0xFFFFFFF; // TODO: +timeoffset?
                destEvent->position = srcEvent->position;
                destEvent->x = x * xyDivider;
                destEvent->y = y * xyDivider;
                destEvent->unused1 = 0;
                destEvent->unused2 = 0;

                newPayload += sizeof(BnlDataPacket::NormalEvent) / sizeof(uint32_t);
                destPacket->payload_length += sizeof(BnlDataPacket::NormalEvent);
            }

            nGood++;
        } else {
            nVeto++;
        }
        srcEvent++;
    }
}

bool BnlFlatFieldPlugin::correctPosition(double *x, double *y)
{
    // This function is almost exact re-implementation from dcomserver. There's
    // room for optimization of the code, but only variable names are changed.
    // The notion is that we shall try to unify flat-field correction, maybe
    // use ACPCs flat-field correction which *seems* superior.

	size_t i,j,k;
	double d1,d2,d3,d4,dindex,dregion1,dregion2;
	double xcorrected,ycorrected;

	if (*x > m_boundaryHighX || *x < m_boundaryLowX || *y > m_boundaryHighY || *y< m_boundaryLowY)
	{
		*x=m_boundaryHighX+100.0;
		*y=m_boundaryHighY+100.0;
		return false;
	}
	i=(int)((*y)*m_tableX.xScale);
	if (*y > m_tableX.rangeLow[i])
	{
		while (*y > m_tableX.rangeHigh[i])
		{
			if (i==m_tableX.nX-1)
				break;
			i++;
		}
	}
	else if (i>0)
	{
		i--;
		while (*y < m_tableX.rangeLow[i])
		{
			if (i==0)
				break;
			i--;
		}
	}
//okay, got the region.
//now find the yScale
	dindex=(*x)*m_tableX.yScale;  //this trucates so we use j and j+1;
	j=(int)dindex;
	k=i*m_tableX.nY;
//find the index dude...
	if (*x > m_tableX.values[k+j])
	{
		while (*x > m_tableX.values[k+j+1])
		{
			if (j==m_tableX.nY-1)
				break;
			j++;
		}
	}
	else if (j>0)
	{
		j--;
		while (*x < m_tableX.values[k+j])
		{
			if (j==0)
				break;
			j--;
		}
	}
	d1=m_tableX.values[k+j];
	d2=m_tableX.values[k+j+1];
//
	dregion1=(*x-d1)*(d2-d1)+j;

	if ((i>0) && (i<m_tableX.nX-1))
	{
		d3=fabs(*y-m_tableX.rangeCenter[i-1]);
		d4=fabs(*y-m_tableX.rangeCenter[i+1]);
	}
	else if (i==0)
	{
		d3=1.0e9;
		d4=fabs(*y-m_tableX.rangeCenter[1]);
	}
	else
	{
		d3=fabs(*y-m_tableX.rangeCenter[m_tableX.nX-2]);
		d4=1.0e9;
	}


	if (d4 > d3)
	{
		d4=fabs(*y-m_tableX.rangeCenter[i]);
//must find j again.....
		k=(i-1)*m_tableX.nY;
		if (*x > m_tableX.values[k+j])
		{
			while (*x > m_tableX.values[k+j+1])
			{
				if (j==m_tableX.nY-1)
					break;
				j++;
			}
		}
		else if (j>0)
		{
			j--;
			while (*x < m_tableX.values[k+j])
			{
				if (j==0)
					break;
				j--;
			}
		}
		d1=m_tableX.values[k+j];
		d2=m_tableX.values[k+j+1];
	//
		dregion2=(*x-d1)*(d2-d1)+j;
		xcorrected=m_correctionResolution*(dregion1*d3+dregion2*d4)/(d3+d4);  //note when d3 is big means far from region 2
	}
	else
	{
		d3=fabs(*y-m_tableX.rangeCenter[i]);
//must find j again.....
		k=(i+1)*m_tableX.nY;
		if (*x > m_tableX.values[k+j])
		{
			while (*x > m_tableX.values[k+j+1])
			{
				if (j==m_tableX.nY-1)
					break;
				j++;
			}
		}
		else if (j>0)
		{
			j--;
			while (*x < m_tableX.values[k+j])
			{
				if (j==0)
					break;
				j--;
			}
		}
		d1=m_tableX.values[k+j];
		d2=m_tableX.values[k+j+1];
	//
		dregion2=(*x-d1)*(d2-d1)+j;
		xcorrected=m_correctionResolution*(dregion1*d4+dregion2*d3)/(d3+d4);
	}
////////////////////////
//now repeat for y....
/////////////////////////////
//////////////////////////////
	i=(int)((*x)*m_tableY.xScale);
	if (*x > m_tableY.rangeLow[i])
	{
		while (*x > m_tableY.rangeHigh[i])
		{
			if (i==m_tableY.nX-1)
				break;
			i++;
		}
	}
	else if (i>0)
	{
		i--;
		while (*x < m_tableY.rangeLow[i])
		{
			if (i==0)
				break;
			i--;
		}
	}
//okay, got the region.
//now find the yScale
	dindex=(*y)*m_tableY.yScale;  //this trucates so we use j and j+1;
	j=(int)dindex;
	k=i*m_tableY.nY;
//find the index dude...
	if (*y > m_tableY.values[k+j])
	{
		while (*y > m_tableY.values[k+j+1])
		{
			if (j==m_tableY.nY-1)
				break;
			j++;
		}
	}
	else if (j>0)
	{
		j--;
		while (*y < m_tableY.values[k+j])
		{
			if (j==0)
				break;
			j--;
		}
	}
	d1=m_tableY.values[k+j];
	d2=m_tableY.values[k+j+1];
//
	dregion1=(*y-d1)*(d2-d1)+j;

	if ((i>0) && (i<m_tableY.nX-1))
	{
		d3=fabs(*x-m_tableY.rangeCenter[i-1]);
		d4=fabs(*x-m_tableY.rangeCenter[i+1]);
	}
	else if (i==0)
	{
		d3=1.0e9;
		d4=fabs(*x-m_tableY.rangeCenter[1]);
	}
	else
	{
		d3=fabs(*x-m_tableY.rangeCenter[m_tableX.nX-2]);
		d4=1.0e9;
	}
	if (d4 > d3)
	{
		d4=fabs(*x-m_tableY.rangeCenter[i]);
//must find j again.....
		k=(i-1)*m_tableY.nY;
		if (*y > m_tableY.values[k+j])
		{
			while (*y > m_tableY.values[k+j+1])
			{
				if (j==m_tableY.nY-1)
					break;
				j++;
			}
		}
		else if (j>0)
		{
			j--;
			while (*y < m_tableY.values[k+j])
			{
				if (j==0)
					break;
				j--;
			}
		}
		d1=m_tableY.values[k+j];
		d2=m_tableY.values[k+j+1];
	//
		dregion2=(*y-d1)*(d2-d1)+j;
		ycorrected=m_correctionResolution*(dregion1*d3+dregion2*d4)/(d3+d4);
	}
	else
	{
		d3=fabs(*x-m_tableY.rangeCenter[i]);
//must find j again.....
		k=(i+1)*m_tableY.nY;
		if (*y > m_tableY.values[k+j])
		{
			while (*y > m_tableY.values[k+j+1])
			{
				if (j==m_tableY.nY-1)
					break;
				j++;
			}
		}
		else if (j>0)
		{
			j--;
			while (*y < m_tableY.values[k+j])
			{
				if (j==0)
					break;
				j--;
			}
		}
		d1=m_tableY.values[k+j];
		d2=m_tableY.values[k+j+1];
	//
		dregion2=(*y-d1)*(d2-d1)+j;
		ycorrected=m_correctionResolution*(dregion1*d4+dregion2*d3)/(d3+d4);
	}

	*x=xcorrected;
	*y=ycorrected;
	if (*x > m_boundaryHighX || *x < m_boundaryLowX || *y > m_boundaryHighY || *y< m_boundaryLowY)
	{
		*x=m_boundaryHighX+100.0;
		*y=m_boundaryHighY+100.0;
		return false;
	}

	return true;
}

bool BnlFlatFieldPlugin::loadParamFile(const std::string &paramFile)
{
    double maxX = 1;
    double maxY = 1;

    FILE *fp = fopen(paramFile.c_str(), "r");
    if (!fp) {
        LOG_ERROR("Can't open param file '%s'", paramFile.c_str());
        return false;
    }

    if (matchLine(fp, "<?xml version=\"1.0\"?>") == false) {
        LOG_ERROR("Invalid param file format: missing header");
        fclose(fp);
        return false;
    }

    if (matchLine(fp, "<FlattenConfig>") == false) {
        LOG_ERROR("Invalid param file format: no XML root");
        fclose(fp);
        return false;
    }

    std::string key, value;
    while (readLine(fp, key, value) == true) {
        if (key == "CorrectionScale") {
            m_correctionScale = stod(value);
        } else if (key == "CorrectionResolution") {
            m_correctionResolution = stod(value);
        } else if (key == "MaxValues") {
            std::vector<double> vals;
            toArray(value, vals);
            if (vals.size() == 2) {
                maxX = vals[0];
                maxY = vals[1];
            } else {
                LOG_ERROR("MaxValues must be a 2 value field");
            }
        } else if (key == "CorrectionBoundary") {
            std::vector<double> vals;
            toArray(value, vals);
            if (vals.size() == 4) {
                m_boundaryLowX  = vals[0];
                m_boundaryHighX = vals[1];
                m_boundaryLowY  = vals[2];
                m_boundaryHighY = vals[3];
            } else {
                LOG_ERROR("CorrectionBoundary must be a 4 value field");
            }
        } else if (key == "CorrectionSize") {
            CorrectionTable *table;
            if (value.at(0) == 'X')
                table = &m_tableX;
            else if (value.at(0) == 'Y')
                table = &m_tableY;
            else {
                LOG_ERROR("CorrectionSize dimension must be X or Y");
                continue;
            }

            std::vector<int> vals;
            toArray(value.erase(0, 1), vals);
            if (vals.size() == 3) {
                table->nX = vals[1];
                table->nY = vals[2];
                table->rangeLow.resize(table->nX);
                table->rangeHigh.resize(table->nX);
                table->rangeCenter.resize(table->nX);
            } else {
                LOG_ERROR("CorrectionSize must be a 4 value field");
            }
        } else if (key == "CorrectionMethod") {
            CorrectionTable *table;
            if (value.at(0) == 'X')
                table = &m_tableX;
            else if (value.at(0) == 'Y')
                table = &m_tableY;
            else {
                LOG_ERROR("CorrectionMethod dimension must be X or Y");
                continue;
            }

            std::vector<int> vals;
            toArray(value.erase(0, 1), vals);
            if (vals.size() == 2) {
                table->method = vals[1];
            } else {
                LOG_ERROR("CorrectionMethod must be a 3 value field");
            }
        } else if (key == "CorrectionRegion") {
            CorrectionTable *table;
            if (value.at(0) == 'X')
                table = &m_tableX;
            else if (value.at(0) == 'Y')
                table = &m_tableY;
            else {
                LOG_ERROR("CorrectionRegion dimension must be X or Y");
                continue;
            }

            std::vector<double> vals;
            toArray(value.erase(0, 1), vals);
            if (vals.size() == 5) {
                uint32_t row = vals[1];
                if (row < table->rangeLow.size()) {
                    table->rangeLow[row] = vals[2];
                    table->rangeHigh[row] = vals[3];
                    table->rangeCenter[row] = 0.5 * (vals[2] + vals[3]);
                } else {
                    LOG_ERROR("CorretionRegion parameter out of range");
                }
            } else {
                LOG_ERROR("CorrectionRegion must be a 6 value field");
            }
        }
    }

    fclose(fp);

    // Apply correction scale to all parameters
    m_boundaryLowX  *= m_correctionScale;
    m_boundaryHighX *= m_correctionScale;
    m_boundaryLowY  *= m_correctionScale;
    m_boundaryHighX *= m_correctionScale;
    for (size_t i = 0; i < m_tableX.rangeLow.size(); i++) {
        m_tableX.rangeLow[i] *= m_correctionScale;
        m_tableX.rangeHigh[i] *= m_correctionScale;
        m_tableX.rangeCenter[i] *= m_correctionScale;
    }
    for (size_t i = 0; i < m_tableY.rangeLow.size(); i++) {
        m_tableY.rangeLow[i] *= m_correctionScale;
        m_tableY.rangeHigh[i] *= m_correctionScale;
        m_tableY.rangeCenter[i] *= m_correctionScale;
    }

    m_tableX.xScale = m_tableX.rangeLow.size() / maxX;
    m_tableX.yScale = m_tableX.rangeLow.size() / maxY;
    m_tableY.xScale = m_tableY.rangeLow.size() / maxX;
    m_tableY.yScale = m_tableY.rangeLow.size() / maxY;

    m_tableX.initialized = true;
    m_tableY.initialized = true;

    return true;
}

bool BnlFlatFieldPlugin::loadValueFile(const std::string &valFile)
{
    uint32_t n;
    FILE *fp = fopen(valFile.c_str(), "rb");
    if (!fp) {
        LOG_ERROR("Can't open values file '%s'", valFile.c_str());
        return false;
    }

    n = m_tableX.nX * m_tableX.nY;
    m_tableX.values.resize(n);
    for (uint32_t i=0; i<n; i++) {
        double val;
        if (fread(&val, sizeof(double), 1, fp) != 1) {
            LOG_ERROR("Failed to read form val file");
            fclose(fp);
            return false;
        }
        m_tableX.values[i] = val * m_correctionScale;
    }

    n = m_tableY.nX * m_tableY.nY;
    m_tableY.values.resize(n);
    for (uint32_t i=0; i<n; i++) {
        double val;
        if (fread(&val, sizeof(double), 1, fp) != 1) {
            LOG_ERROR("Failed to read form val file");
            fclose(fp);
            return false;
        }
        m_tableY.values[i] = val * m_correctionScale;
    }

    fclose(fp);
    return true;
}

bool BnlFlatFieldPlugin::readLine(FILE *fp, std::string &key, std::string &value)
{
    char buffer[256];
    while (fp && fgets(buffer, sizeof(buffer), fp) != NULL) {
        char key1[32], key2[32], val[128];
        if (sscanf(strip(buffer), "<%[^>]>%[^<]</%[^>]>", key1, val, key2) == 3 && strcmp(key1, key2) == 0) {
            key = key1;
            value = strip(val);
            return true;
        }
    }
    return false;
}

bool BnlFlatFieldPlugin::matchLine(FILE *fp, const std::string &match)
{
    char buffer[256];

    if (!fp || fgets(buffer, sizeof(buffer), fp) == NULL)
        return false;

    return (match == strip(buffer));
}

char *BnlFlatFieldPlugin::strip(char *str)
{
    while (isspace(*str)) str++;

    if (*str == 0)
        return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    *(end+1) = 0;
    return str;
}

void BnlFlatFieldPlugin::toArray(const std::string &str, std::vector<double> &vals)
{
    std::string temp = str;
    vals.clear();

    while (true) {
        // Find first digit character
        size_t idx = temp.find_first_of("0123456789.");
        if (idx == std::string::npos)
            break;

        // Convert entire number to double, idx points to first non-translated character
        temp = temp.substr(idx);
        vals.push_back(stod(temp, &idx));

        // Erase eaten characters
        temp.erase(0, idx);
    }
}

void BnlFlatFieldPlugin::toArray(const std::string &str, std::vector<int> &vals)
{
    std::string temp = str;
    vals.clear();

    while (true) {
        // Find first digit character
        size_t idx = temp.find_first_of("0123456789");
        if (idx == std::string::npos)
            break;

        // Convert entire number to double, idx points to first non-translated character
        temp = temp.substr(idx);
        vals.push_back(stod(temp, &idx));

        // Erase eaten characters
        temp.erase(0, idx);
    }
}
