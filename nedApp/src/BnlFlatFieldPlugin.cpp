/* BnlFlatFieldPlugin.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BnlFlatFieldPlugin.h"
#include "Common.h"
#include "Log.h"

#include <cstring>
#include <likely.h>
#include <limits>
#include <math.h>

EPICS_REGISTER_PLUGIN(BnlFlatFieldPlugin, 5, "Port name", string, "Dispatcher port name", string, "Parameter file path", string, "Values file path", string, "Buffer size in bytes", int);

#define NUM_BNLFLATFIELDPLUGIN_PARAMS      ((int)(&LAST_BNLFLATFIELDPLUGIN_PARAM - &FIRST_BNLFLATFIELDPLUGIN_PARAM + 1)) + (20 + 17)*2

#define POS_SPECIAL     (1 << 30)

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

    createParam("ParamsPath",   asynParamOctet, &ParamsPath, paramFile); // Path to parameters file
    createParam("ValPath",      asynParamOctet, &ValPath, valFile); // Path to correction values file
    createParam("ErrImport",    asynParamInt32, &ErrImport, err); // Last mapping import error
    createParam("CntUnmap",     asynParamInt32, &CntUnmap,  0);   // Number of unmapped pixels
    createParam("CntError",     asynParamInt32, &CntError,  0);   // Number of unknown-error pixels
    createParam("CntSplit",     asynParamInt32, &CntSplit,  0);   // Number of packet train splits
    createParam("ResetCnt",     asynParamInt32, &ResetCnt);       // Reset counters
    createParam("CorrectMode",  asynParamInt32, &CorrectMode, MODE_PASSTHRU); // Event correction mode
    createParam("CentroidMin",  asynParamInt32, &CentroidMin, 0); // Centroid minimum parameter for X,Y calculation
    createParam("XCentroidScale", asynParamInt32, &XCentroidScale, 100); // X centroid scale factor
    createParam("YCentroidScale", asynParamInt32, &YCentroidScale, 70); // Y centroid scale factor
    createParam("HighResEn",    asynParamInt32, &HighResEn, 0);   // Switch between high and low resolution

    for (int i = 1; i <= 20; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "X%dScale", i);
        createParam(buf,        asynParamInt32, &XScales[i], 0);
        snprintf(buf, sizeof(buf), "X%dOffset", i);
        createParam(buf,        asynParamInt32, &XOffsets[i], 0);
    }
    for (int i = 1; i <= 17; i++) {
        char buf[20];
        snprintf(buf, sizeof(buf), "Y%dScale", i);
        createParam(buf,        asynParamInt32, &YScales[i], 0);
        snprintf(buf, sizeof(buf), "Y%dOffset", i);
        createParam(buf,        asynParamInt32, &YOffsets[i], 0);
    }
    callParamCallbacks();
}

void BnlFlatFieldPlugin::processDataUnlocked(const DasPacketList * const packetList)
{
    int nReceived = 0;
    int nProcessed = 0;
    int nSplits = 0;
    int nErrors = 0;
    int correctMode = MODE_PASSTHRU;
    int val;

    this->lock();
    getIntegerParam(RxCount,    &nReceived);
    getIntegerParam(ProcCount,  &nProcessed);
    getIntegerParam(CntError,   &nErrors);
    getIntegerParam(CntSplit,   &nSplits);
    getIntegerParam(CorrectMode,&correctMode);
    if (getDataMode() != BasePlugin::DATA_MODE_RAW || !m_tableX.initialized || !m_tableY.initialized)
        correctMode = MODE_PASSTHRU;
    // Although these are class variables, only set them here and not from writeInt32().
    // This prevents thread race conditions since the code below is not in thread safe section.
    getIntegerParam(CentroidMin,&m_centroidMin);
    getIntegerParam(XCentroidScale,&val);
    m_xCentroidScale = 1.0 + val/2048.0;
    getIntegerParam(YCentroidScale,&val);
    m_yCentroidScale = 1.0 + val/2048.0;
    getIntegerParam(HighResEn,  &val);
    m_highRes = (val > 0);

    if (getDataMode() == BasePlugin::DATA_MODE_RAW && correctMode != MODE_PASSTHRU) {
        for (int i = 0; i < 20; i++) {
            getIntegerParam(XScales[i+1],  &m_xScales[i]);
            getIntegerParam(XOffsets[i+1], &m_xOffsets[i]);
        }
        for (int i = 0; i < 17; i++) {
            getIntegerParam(YScales[i+1],  &m_yScales[i]);
            getIntegerParam(YOffsets[i+1], &m_yOffsets[i]);
        }
    }
    this->unlock();

    // Optimize pass thru mode
    if (correctMode == MODE_PASSTHRU) {
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
                nErrors = processPacketRaw(packet, newPacket);
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
    setIntegerParam(CntError,   nErrors);
    callParamCallbacks();
    this->unlock();
}

int BnlFlatFieldPlugin::processPacketRaw(const DasPacket *srcPacket, DasPacket *destPacket)
{
    typedef struct {
        uint32_t tof;
        uint32_t position;
        uint32_t samples[(20+17+1)/2];
    } RawEvent;

    // destPacket is guaranteed to be at least the size of srcPacket
    (void)srcPacket->copyHeader(destPacket, srcPacket->length());

    uint32_t nEvents, nDestEvents;
    const RawEvent *srcEvent= reinterpret_cast<const RawEvent *>(srcPacket->getData(&nEvents));
    DasPacket::Event *destEvent = reinterpret_cast<DasPacket::Event *>(destPacket->getData(&nDestEvents));
    nEvents /= (sizeof(RawEvent) / sizeof(uint32_t));

    while (nEvents-- > 0) {
        if (likely((srcEvent->position & POS_SPECIAL) == 0)) {
            double x,y;
            if (calculatePositionRaw(srcEvent->samples, &x, &y) == true &&
                correctPosition(&x, &y) == true) {

                uint16_t xd, yd;

                if (m_highRes) {
                    xd = ((uint16_t)(32 * x + 0.5) & 0x1FF) << 9;
                    yd = ((uint16_t)(32 * y + 0.5) & 0x1FF) << 9;
                } else {
                    xd = ((uint16_t)(16 * x + 0.5) & 0xFF)  << 8;
                    yd = ((uint16_t)(16 * y + 0.5) & 0xFF)  << 8;
                }

                destEvent->tof = srcEvent->tof;
                destEvent->pixelid = (srcEvent->position & 0xFFFC0000) | xd | yd;
                destEvent++;
                nDestEvents++;
            }
        }
        srcEvent++;
    }
    destPacket->payload_length += nDestEvents * sizeof(DasPacket::Event);
    return (nEvents - nDestEvents);
}

bool BnlFlatFieldPlugin::calculatePositionRaw(const uint32_t *samples, double *x, double *y)
{
    uint16_t *raw = (uint16_t *)samples;
    uint16_t xSamples[20];
    uint16_t ySamples[17];
    uint8_t xMaxIndex = 0; // [0..19]
    uint8_t yMaxIndex = 0; // [0..16]
    int denom;

    // The BNL raw format produces 20 X samples and 17 Y samples. In the raw
    // event data they alternate between X and Y up to position 17, then only
    // X follow.
    for (int i = 0; i < 17; i++) {
        xSamples[i] = m_xScales[i] * (16 * (raw[i]   & 0xFFF) - m_xOffsets[i]);
        ySamples[i] = m_yScales[i] * (16 * (raw[i+1] & 0xFFF) - m_yOffsets[i]);
    }
    for (int i = 17; i < 20; i++) {
        xSamples[i] = m_xScales[i] * (16 * (raw[i]   & 0xFFF) - m_xOffsets[i]);
    }

    // For code sanity, don't optimize finding max position into data parsing
    // above. Any good compiler will roll-out the for loop anyway.
    for (int i = 1; i < 20; i++) {
        if (xSamples[i] > xSamples[xMaxIndex])
            xMaxIndex = i;
    }
    for (int i = 1; i < 17; i++) {
        if (ySamples[i] > ySamples[yMaxIndex])
            yMaxIndex = i;
    }

    // Filter out common vetoes early
    if (xMaxIndex == 0 || xMaxIndex == 19 || yMaxIndex == 0 || yMaxIndex == 16)
        return false;

    // Interpolate X position
    denom = xSamples[xMaxIndex+1] + xSamples[xMaxIndex] + xSamples[xMaxIndex-1];
    if (denom < m_centroidMin)
        return false;
    *x = xMaxIndex + (1.0 * m_xCentroidScale * (xSamples[xMaxIndex+1] - xSamples[xMaxIndex-1]) / denom);

    // Interpolate Y position
    denom = ySamples[yMaxIndex+1] + ySamples[yMaxIndex] + ySamples[yMaxIndex-1];
    if (denom < m_centroidMin)
        return false;
    *y = xMaxIndex + (1.0 * m_yCentroidScale * (ySamples[yMaxIndex+1] - ySamples[yMaxIndex-1]) / denom);

    return true;
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
    double maxX, maxY;

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
