/* FlatFieldPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FlatFieldPlugin.h"

#include <vector>

#define NUM_FLATFIELDPLUGIN_PARAMS ((int)(&LAST_FLATFIELDPLUGIN_PARAM - &FIRST_FLATFIELDPLUGIN_PARAM + 1))

EPICS_REGISTER_PLUGIN(FlatFieldPlugin, 3, "Port name", string, "Dispatcher port name", string, "Blocking", int);

FlatFieldPlugin::FlatFieldPlugin(const char *portName, const char *dispatcherPortName, int blocking)
    : BasePlugin(portName, dispatcherPortName, REASON_OCCDATA, blocking, NUM_FLATFIELDPLUGIN_PARAMS, 1,
                 defaultInterfaceMask, defaultInterruptMask)
{
    createParam("RefTable", asynParamInt32Array, &RefTable);
}

FlatFieldPlugin::~FlatFieldPlugin()
{}

void FlatFieldPlugin::initTable()
{
}

asynStatus FlatFieldPlugin::writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements)
{
    if (pasynUser->reason == RefTable) {
        if (nElements != (TABLE_X_SIZE*TABLE_Y_SIZE))
            return asynError;
        std::vector<uint32_t> refTable;
        for (size_t i = 0; i < nElements; i++)
            refTable.push_back(*(value+i));
//        initTable(refTable);
    }
    return writeInt32Array(pasynUser, value, nElements);
}

/*
bool FlatFieldPlugin::initTable()
{
	CFile mydatafile;
	CString cstemp;
	double x_a,x_b,x_c;
	double diff;
	int i,j;
	int start[5],end[5];
	double *pd;
	double section[512];  //used to pass to Quadfit
	double *xsectiond=new double[512*512];
	double *ysectiond=new double[512*512];
	double *xsmooth=new double[512*512];
	double *ysmooth=new double[512*512];
	unsigned int *data=new unsigned int[512*512];

	cstemp.Format("%s%s.v3ff",dir,filename);
	i=mydatafile.Open(cstemp,CFile::modeRead,0);

//okay get the array
	if (!i)
	{
		delete data;
		delete xsectiond;
		delete ysectiond;
		delete xsmooth;
		delete ysmooth;
		return -1;
	}


	if (m_ff_x[index]==NULL)
	{
		m_ff_x[index] = new double[512*512];
	}

	if (m_ff_y[index]==NULL)
	{
		m_ff_y[index] = new double[512*512];
	}


	mydatafile.Read(data,512*512*sizeof(unsigned int));
	mydatafile.Close();
//////////////////////////////////////////
///////////new code //////////////////////
//////////////////////////////////////////
//first, flatten the thing globally....
	double coef[6];
	QuadFit2D(data,60,450,coef);
	for (i=0;i<512;i++)
	{
		for (j=0;j<512;j++)
		{
			xsectiond[512*j+i]=0;
			ysectiond[512*i+j]=0;
			xsmooth[i*512+j]=1.0*data[i*512+j]/(coef[0]+i*coef[1]+j*coef[2]+i*j*coef[3]+i*i*coef[4]+j*j*coef[5]);
		}
	}

	for (i=0;i<512;i++)
	{
		for (j=0;j<512;j++)
		{
			xsectiond[i*512+j]=xsmooth[i*512+j];
			ysectiond[i*512+j]=xsmooth[i*512+j];
		}
	}
//now, we must remove the variations in each alternate dimension..
//I.e. if we want x we use average out the y variations.
//lets don't smooth at all and see what happens

	start[0]=85-40;
	end[0]=85+40;
	start[1]=256-40;
	end[1]=256+40;
	start[2]=512-85-40;
	end[2]=512-85+40;

	for (i=0;i<512;i++)  //i is x pixel.....
	{
		for (j=0;j<512;j++)
		{
			section[j]=xsmooth[512*i+j];
		}
		QuadFitMulti(section,3,start,end,&x_a,&x_b,&x_c);
		for (j=0;j<512;j++)
		{
			xsectiond[i*512+j]=(x_a+x_b*j+x_c*j*j);
		}
	}
	for (i=0;i<512;i++)  //i is y pixel.....
	{
		for (j=0;j<512;j++)
		{
			section[j]=xsmooth[512*j+i];
		}
		QuadFitMulti(section,3,start,end,&x_a,&x_b,&x_c);
		for (j=0;j<512;j++)
		{
			ysectiond[j*512+i]=(x_a+x_b*j+x_c*j*j);
		}
	}
//now find the modulation along each axis
	for (i=0;i<512;i++)
	{
		for (j=0;j<512;j++)
		{
			section[j]=xsectiond[j*512+i];
		}
//do sections of about 80 or four pixels....fixed widto start......
		QuadFitMulti(section,3,start,end,&x_a,&x_b,&x_c);
		for (j=0;j<512;j++)
		{
			xsectiond[j*512+i]=xsectiond[j*512+i]/(x_a+x_b*j+x_c*j*j);
		}
	}

//////////////////////////////////
	for (i=0;i<512;i++)
	{
		for (j=0;j<512;j++)
		{
			section[j]=ysectiond[i*512+j];
		}
		QuadFitMulti(section,3,start,end,&x_a,&x_b,&x_c);
		for (j=0;j<512;j++)
		{
			ysectiond[i*512+j]=ysectiond[i*512+j]/(x_a+x_b*j+x_c*j*j);
		}
	}



	for (j=0;j<512;j++)
	{
		for (i=0;i<512;i++)
		{
			xsmooth[i*512+j]=xsectiond[512*i+j];
			ysmooth[j*512+i]=ysectiond[512*j+i];
		}
	}


	for (i=0;i<512;i++)
	{
		for (j=0;j<512;j++)
		{
			xsmooth[512*i+j]=xsmooth[512*i+j]-1.0;
			ysmooth[512*j+i]=ysmooth[512*j+i]-1.0;
		}
	}


	pd=m_ff_x[index];   //we start at zero.or is zero at pixel 256
	for (i=0;i<512;i++)
	{
		pd[512*85+i]=0.0;
		for (j=84;j>=0;j--)
		{
			pd[i+512*j]=pd[i+512*(j+1)]+.5*(xsmooth[i+512*(j+1)]+xsmooth[i+512*j]);
			if (pd[i+512*j] > 10.0)
				pd[i+512*j]=10.0;
			if (pd[i+512*j] < -10.0)
				pd[i+512*j]=-10.0;
		}
		for (j=86;j<=170;j++)
		{
			pd[i+512*j]=pd[i+512*(j-1)]-.5*(xsmooth[i+512*(j-1)]+xsmooth[i+512*j]);
			if (pd[i+512*j] > 10.0)
				pd[i+512*j]=10.0;
			if (pd[i+512*j] < -10.0)
				pd[i+512*j]=-10.0;
		}
		pd[512*256+i]=0.0;
		for (j=255;j>=171;j--)
		{
			pd[i+512*j]=pd[i+512*(j+1)]+.5*(xsmooth[i+512*(j+1)]+xsmooth[i+512*j]);
			if (pd[i+512*j] > 10.0)
				pd[i+512*j]=10.0;
			if (pd[i+512*j] < -10.0)
				pd[i+512*j]=-10.0;
			xsectiond[i+512*j]=pd[i+512*j];
		}
		for (j=257;j<=340;j++)
		{
			pd[i+512*j]=pd[i+512*(j-1)]-.5*(xsmooth[i+512*(j-1)]+xsmooth[i+512*j]);
			if (pd[i+512*j] > 10.0)
				pd[i+512*j]=10.0;
			if (pd[i+512*j] < -10.0)
				pd[i+512*j]=-10.0;
		}
		pd[512*429+i]=0.0;
		for (j=428;j>=341;j--)
		{
			pd[i+512*j]=pd[i+512*(j+1)]+.5*(xsmooth[i+512*(j+1)]+xsmooth[i+512*j]);
			if (pd[i+512*j] > 10.0)
				pd[i+512*j]=10.0;
			if (pd[i+512*j] < -10.0)
				pd[i+512*j]=-10.0;
			xsectiond[i+512*j]=pd[i+512*j];
		}
		for (j=430;j<=511;j++)
		{
			pd[i+512*j]=pd[i+512*(j-1)]-.5*(xsmooth[i+512*(j-1)]+xsmooth[i+512*j]);
			if (pd[i+512*j] > 10.0)
				pd[i+512*j]=10.0;
			if (pd[i+512*j] < -10.0)
				pd[i+512*j]=-10.0;
		}
	}


//patch the boundaries at 170,171 and 340,341
//estimate value at 170.5 and 340.5 from both sides...
	for (i=0;i<512;i++)
	{
		coef[0]=.5*((pd[i+512*170]-pd[i+512*169])+(pd[i+512*172]-pd[i+512*171]));  //ave slope between sections.
		coef[1]=coef[0]+pd[i+512*170]-pd[i+512*171];  //difference between lower edge....
		coef[2]=.5*((pd[i+512*340]-pd[i+512*339])+(pd[i+512*342]-pd[i+512*341]));
		coef[3]=coef[2]+pd[i+512*340]-pd[i+512*341];
//okay how much do I rotate the center section?
		if (coef[3] < 0.0)   //means I want to rotate center counter clockwise
		{
			if (coef[1] < 0.0)  //in this case I can rotate center then match the edges...
			{
				//rotate half the smaller distance.....
				if (fabs(coef[3]) < fabs(coef[1]))
				{
					diff=-.5*coef[3];
				}
				else
				{
					diff=-.5*coef[1];
				}
				//256 is zero....
				for (j=171;j<=340;j++)
				{
					pd[i+512*j]=pd[i+512*j]+diff*(j-256.0)/(340.5-256.0);
				}
				coef[1]=coef[1]+diff;
				coef[3]=coef[3]+diff;
			}
//now rotate outer segments....
			for (j=0;j<=170;j++)  //point 85 is zero change
			{
				pd[i+512*j]=pd[i+512*j]-coef[1]*(j-85.0)/(170.5-85.0);
			}
			for (j=341;j<=511;j++)  //point 429 is zero change
			{
				pd[i+512*j]=pd[i+512*j]-coef[3]*(j-429.0)/(429.0-340.5);
			}
		}
		else   //rotate clockwise to match edge....
		{
			if (coef[3] > 0.0)  //in this case can I rotate center then match the edge...
			{
				//rotate half the smaller distance.....
				if (coef[1] < coef[3])
				{
					diff=.5*coef[1];   //this causes the sign of change to work out okay...
				}
				else
				{
					diff=.5*coef[3];
				}
				//256 is zero....
				for (j=171;j<=340;j++)
				{
					pd[i+512*j]=pd[i+512*j]-diff*(j-256.0)/(340.5-256.0);
				}
				coef[1]=coef[1]-diff;
				coef[3]=coef[3]-diff;
			}
//now rotate outer segments....
			for (j=0;j<=170;j++)  //point 85 is zero change
			{
				pd[i+512*j]=pd[i+512*j]-coef[1]*(j-85.0)/(170.5-85.0);
			}
			for (j=341;j<=511;j++)  //point 429 is zero change
			{
				pd[i+512*j]=pd[i+512*j]-coef[3]*(j-429.0)/(429.0-340.5);
			}
		}
	}



	pd=m_ff_y[index];   //we start at zero....the gaps
	for (i=0;i<512;i++)
	{
		pd[512*i+85]=0.0;
		for (j=84;j>=0;j--)
		{
			pd[i*512+j]=pd[i*512+(j+1)]+.5*(ysmooth[512*i+(j+1)]+ysmooth[512*i+j]);
			if (pd[i*512+j] > 10.0)
				pd[i*512+j]=10.0;
			if (pd[i*512+j] < -10.0)
				pd[i*512+j]=-10.0;
		}
		for (j=86;j<=170;j++)
		{
			pd[i*512+j]=pd[i*512+(j-1)]-.5*(ysmooth[i*512+(j-1)]+ysmooth[i*512+j]);
			if (pd[i*512+j] > 10.0)
				pd[i*512+j]=10.0;
			if (pd[i*512+j] < -10.0)
				pd[i*512+j]=-10.0;
		}
		pd[512*i+256]=0.0;
		for (j=255;j>=171;j--)
		{
			pd[i*512+j]=pd[i*512+(j+1)]+.5*(ysmooth[512*i+(j+1)]+ysmooth[512*i+j]);
			if (pd[i*512+j] > 10.0)
				pd[i*512+j]=10.0;
			if (pd[i*512+j] < -10.0)
				pd[i*512+j]=-10.0;
			ysectiond[i*512+j]=pd[i*512+j];
		}
		for (j=257;j<=340;j++)
		{
			pd[i*512+j]=pd[i*512+(j-1)]-.5*(ysmooth[i*512+(j-1)]+ysmooth[i*512+j]);
			if (pd[i*512+j] > 10.0)
				pd[i*512+j]=10.0;
			if (pd[i*512+j] < -10.0)
				pd[i*512+j]=-10.0;
		}
		pd[512*i+429]=0.0;
		for (j=428;j>=341;j--)
		{
			pd[i*512+j]=pd[i*512+(j+1)]+.5*(ysmooth[512*i+(j+1)]+ysmooth[512*i+j]);
			if (pd[i*512+j] > 10.0)
				pd[i*512+j]=10.0;
			if (pd[i*512+j] < -10.0)
				pd[i*512+j]=-10.0;
			ysectiond[i*512+j]=pd[i*512+j];
		}
		for (j=430;j<=511;j++)
		{
			pd[i*512+j]=pd[i*512+(j-1)]-.5*(ysmooth[i*512+(j-1)]+ysmooth[i*512+j]);
			if (pd[i*512+j] > 10.0)
				pd[i*512+j]=10.0;
			if (pd[i*512+j] < -10.0)
				pd[i*512+j]=-10.0;
		}
	}

//patch the boundaries at 170,171 and 340,341
//estimate value at 170.5 and 340.5 from both sides...

	for (i=0;i<512;i++)
	{
		coef[0]=.5*((pd[i*512+170]-pd[i*512+169])+(pd[i*512+172]-pd[i*512+171]));  //ave slope between sections.
		coef[1]=coef[0]+pd[i*512+170]-pd[i*512+171];  //difference between lower edge....
		coef[2]=.5*((pd[i*512+340]-pd[i*512+339])+(pd[i*512+342]-pd[i*512+341]));
		coef[3]=coef[2]+pd[i*512+340]-pd[i*512+341];
//okay how much do I rotate the center section?
		if (coef[3] < 0.0)   //means I want to rotate center counter clockwise
		{
			if (coef[1] < 0.0)  //in this case I can rotate center then match the edges...
			{
				//rotate half the smaller distance.....
				if (fabs(coef[3]) < fabs(coef[1]))
				{
					diff=-.5*coef[3];
				}
				else
				{
					diff=-.5*coef[1];
				}
				//256 is zero....
				for (j=171;j<=340;j++)
				{
					pd[i*512+j]=pd[i*512+j]+diff*(j-256.0)/(340.5-256.0);
				}
				coef[1]=coef[1]+diff;
				coef[3]=coef[3]+diff;
			}
//now rotate outer segments....
			for (j=0;j<=170;j++)  //point 85 is zero change
			{
				pd[i*512+j]=pd[i*512+j]-coef[1]*(j-85.0)/(170.5-85.0);
			}
			for (j=341;j<=511;j++)  //point 429 is zero change
			{
				pd[i*512+j]=pd[i*512+j]-coef[3]*(j-429.0)/(429.0-340.5);
			}
		}
		else   //rotate clockwise to match edge....
		{
			if (coef[3] > 0.0)  //in this case can I rotate center then match the edge...
			{
				//rotate half the smaller distance.....
				if (coef[1] < coef[3])
				{
					diff=.5*coef[1];   //this causes the sign of change to work out okay...
				}
				else
				{
					diff=.5*coef[3];
				}
				//256 is zero....
				for (j=171;j<=340;j++)
				{
					pd[i*512+j]=pd[i*512+j]-diff*(j-256.0)/(340.5-256.0);
				}
				coef[1]=coef[1]-diff;
				coef[3]=coef[3]-diff;
			}
//now rotate outer segments....
			for (j=0;j<=170;j++)  //point 85 is zero change
			{
				pd[i*512+j]=pd[i*512+j]-coef[1]*(j-85.0)/(170.5-85.0);
			}
			for (j=341;j<=511;j++)  //point 429 is zero change
			{
				pd[i*512+j]=pd[i*512+j]-coef[3]*(j-429.0)/(429.0-340.5);
			}
		}
	}
///////////////////////////////////////////
///////////////////////////////////////////


//	m_bcorrect[index]=true;
	delete data;
	delete xsectiond;
	delete ysectiond;
	delete xsmooth;
	delete ysmooth;
	return 0;
}
*/

bool FlatFieldPlugin::quadFit2D(unsigned int *data, int startindex, int endindex, double *coef)
{
	//do a sum to find my matrix..
	//the viviT matrix is easy.
	int i,j;
	int ix,iy;
	int currentc;
	double rows[6][6];  //using row column has index....
	double rows_sum[6][6];
	double vector_sum[6];
	double d;
	for (i=0;i<6;i++)
	{
		vector_sum[i]=0.0;
		for (j=0;j<6;j++)
		{
			rows_sum[i][j]=0.0;
		}
	}

	for (ix=startindex;ix<=endindex;ix++)
	{
		for (iy=startindex;iy<=endindex;iy++)
		{
			vector_sum[0]+=data[512*ix+iy];
			vector_sum[1]+=(ix*data[512*ix+iy]);
			vector_sum[2]+=(iy*data[512*ix+iy]);
			vector_sum[3]+=(ix*iy*data[512*ix+iy]);
			vector_sum[4]+=(ix*ix*data[512*ix+iy]);
			vector_sum[5]+=(iy*iy*data[512*ix+iy]);
			for (i=0;i<6;i++)  //i is row index....
			{
				if (i==0)
				{
					rows[i][0]=1;
					rows[i][1]=ix;
					rows[i][2]=iy;
					rows[i][3]=iy*ix;
					rows[i][4]=ix*ix;
					rows[i][5]=iy*iy;
				}
				else if (i==1)
				{
					for (j=0;j<6;j++)
					{
						rows[i][j]=rows[0][j]*ix;
					}
				}
				else if (i==2)
				{
					for (j=0;j<6;j++)
					{
						rows[i][j]=rows[0][j]*iy;
					}
				}
				else if (i==3)
				{
					for (j=0;j<6;j++)
					{
						rows[i][j]=rows[0][j]*ix*iy;
					}
				}
				else if (i==4)
				{
					for (j=0;j<6;j++)
					{
						rows[i][j]=rows[0][j]*ix*ix;
					}
				}
				else if (i==5)
				{
					for (j=0;j<6;j++)
					{
						rows[i][j]=rows[0][j]*iy*iy;
					}
				}
			}
//now add to rowsummatrix
			for (i=0;i<6;i++)
			{
				for (j=0;j<6;j++)
				{
					rows_sum[i][j]+=rows[i][j];
				}
			}
		}
	}

	//okay now solve by quassian elimination.
	//first column
	for (currentc=0;currentc<5;currentc++)
	{
		for (i=currentc;i<6;i++)
		{
			if (rows_sum[i][currentc] != 0)
			{
				d=rows_sum[i][currentc];
				for (j=currentc;j<6;j++)
				{
					rows_sum[i][j]/=d;
				}
				vector_sum[i]/=d;
			}
		}
	//okay we now subtract since the first row has a one in it.
		for (i=currentc+1;i<6;i++)
		{
			if (rows_sum[i][currentc] != 0)
			{
				for (j=currentc;j<6;j++)
				{
					rows_sum[i][j]=rows_sum[i][j]-rows_sum[currentc][j];
				}
				vector_sum[i]=vector_sum[i]-vector_sum[currentc];
			}
		}
	}
//okay, rows 2 through 6 now have zeros in the first column, now we do the next row.....
//okay now return the values.
	coef[5]=vector_sum[5]/rows_sum[5][5];
	coef[4]=(vector_sum[4]-rows_sum[4][5]*coef[5])/rows_sum[4][4];
	coef[3]=(vector_sum[3]-rows_sum[3][5]*coef[5]-rows_sum[3][4]*coef[4])/rows_sum[3][3];
	coef[2]=(vector_sum[2]-rows_sum[2][5]*coef[5]-rows_sum[2][4]*coef[4]-rows_sum[2][3]*coef[3])/rows_sum[2][2];
	coef[1]=(vector_sum[1]-rows_sum[1][5]*coef[5]-rows_sum[1][4]*coef[4]-rows_sum[1][3]*coef[3]-rows_sum[1][2]*coef[2])/rows_sum[1][1];
	coef[0]=(vector_sum[0]-rows_sum[0][5]*coef[5]-rows_sum[0][4]*coef[4]-rows_sum[0][3]*coef[3]-rows_sum[0][2]*coef[2]-rows_sum[0][1]*coef[1])/rows_sum[0][0];

	return 0;
}
