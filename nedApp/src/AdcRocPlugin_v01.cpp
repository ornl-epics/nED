/* AdcRocPlugin_v01.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "AdcRocPlugin.h"

void AdcRocPlugin::createStatusParams_v01()
{
//    BLXXX:Det:RocXXX:| sig nam|                              | EPICS record description | (bi and mbbi description)
//    createStatusParam("ErrProgramming",       0x0,  1, 15); // WRITE_CNFG during ACQ        (0=no error,1=error)
}

void AdcRocPlugin::createCounterParams_v01()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
//    createConfigParam("Ch1:A:Scale",      '1', 0x0,  12, 0, 2048);  // Chan1 A scale
}

void AdcRocPlugin::createConfigParams_v01()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
      createConfigParam("Ch1:DIG:PositionIdx",  '1', 0x0, 32, 0, 0);     // Chan1 Digital position index
      createConfigParam("Ch2:DIG:PositionIdx",  '1', 0x2, 32, 0, 256);   // Chan2 Digital position index
      createConfigParam("Ch3:DIG:PositionIdx",  '1', 0x4, 32, 0, 512);   // Chan3 Digital position index
      createConfigParam("Ch4:DIG:PositionIdx",  '1', 0x6, 32, 0, 768);   // Chan4 Digital position index
      createConfigParam("Ch5:DIG:PositionIdx",  '1', 0x8, 32, 0, 1024);  // Chan5 Digital position index
      createConfigParam("Ch6:DIG:PositionIdx",  '1', 0xA, 32, 0, 1280);  // Chan6 Digital position index
      createConfigParam("Ch7:DIG:PositionIdx",  '1', 0xC, 32, 0, 1536);  // Chan7 Digital position index
      createConfigParam("Ch8:DIG:PositionIdx",  '1', 0xE, 32, 0, 1792);  // Chan8 Digital position index
      createConfigParam("Ch1:ADC:PositionIdx",  '1', 0x10, 32, 0, 0);    // Chan1 ADC position index
      createConfigParam("Ch2:ADC:PositionIdx",  '1', 0x12, 32, 0, 256);  // Chan2 ADC position index
      createConfigParam("Ch3:ADC:PositionIdx",  '1', 0x14, 32, 0, 512);  // Chan3 ADC position index
      createConfigParam("Ch4:ADC:PositionIdx",  '1', 0x16, 32, 0, 768);  // Chan4 ADC position index
      createConfigParam("Ch5:ADC:PositionIdx",  '1', 0x18, 32, 0, 1024); // Chan5 ADC position index
      createConfigParam("Ch6:ADC:PositionIdx",  '1', 0x1A, 32, 0, 1280); // Chan6 ADC position index
      createConfigParam("Ch7:ADC:PositionIdx",  '1', 0x1C, 32, 0, 1536); // Chan7 ADC position index
      createConfigParam("Ch8:ADC:PositionIdx",  '1', 0x1E, 32, 0, 1792); // Chan8 ADC position index
      createConfigParam("PositionIdxBus",       '1', 0x20, 32, 0, 0);    // Position Index bus *** UNUSED BY FIRMWARE ***

      createConfigParam("Ch1:ADC:Threshold",    'C', 0x0, 16, 0, 0);     // Chan1 ADC Threshold 
      createConfigParam("Ch2:ADC:Threshold",    'C', 0x1, 16, 0, 0);     // Chan2 ADC Threshold
      createConfigParam("Ch3:ADC:Threshold",    'C', 0x2, 16, 0, 0);     // Chan3 ADC Threshold
      createConfigParam("Ch4:ADC:Threshold",    'C', 0x3, 16, 0, 0);     // Chan4 ADC Threshold
      createConfigParam("Ch5:ADC:Threshold",    'C', 0x4, 16, 0, 0);     // Chan5 ADC Threshold
      createConfigParam("Ch6:ADC:Threshold",    'C', 0x5, 16, 0, 0);     // Chan6 ADC Threshold
      createConfigParam("Ch7:ADC:Threshold",    'C', 0x6, 16, 0, 0);     // Chan7 ADC Threshold
      createConfigParam("Ch8:ADC:Threshold",    'C', 0x7, 16, 0, 0);     // Chan8 ADC Threshold
      createConfigParam("PM_TsyncDelay",        'C', 0x8, 24, 0, 0);     // Pulsed Magnet Tsync Delay
      createConfigParam("PM_RatePeriod",        'C', 0xA, 16, 0, 0);     // Pulsed Magnet Rate Period
      createConfigParam("Ch1:DIG:Mode",         'C', 0xB, 2, 0, 0);     // Chan1 Digital position index
      createConfigParam("Ch2:DIG:Mode",         'C', 0xB, 2, 0, 256);   // Chan2 Digital position index
      createConfigParam("Ch3:DIG:Mode",         'C', 0xB, 2, 0, 512);   // Chan3 Digital position index
      createConfigParam("Ch4:DIG:Mode",         'C', 0xB, 2, 0, 768);   // Chan4 Digital position index
      createConfigParam("Ch5:DIG:Mode",         'C', 0xB, 2, 0, 1024);  // Chan5 Digital position index
      createConfigParam("Ch6:DIG:Mode",         'C', 0xB, 2, 0, 1280);  // Chan6 Digital position index
      createConfigParam("Ch7:DIG:Mode",         'C', 0xB, 2, 0, 1536);  // Chan7 Digital position index
      createConfigParam("Ch8:DIG:Mode",         'C', 0xB, 2, 0, 1792);  // Chan8 Digital position index

      createConfigParam("ADC_OS",               'D', 0x0, 3, 0, 0);     // ADC Oversampling
      createConfigParam("ADC_Range",            'D', 0x0, 1, 4, 0);     // ADC Range (0=5V,1=10V)
      createConfigParam("PM_Enable",            'D', 0x0, 1, 5, 0);     // Pulsed Magnet Enable
      createConfigParam("Ch1:ADC:Enable",       'D', 0x0, 1, 8, 0);     // Chan1 ADC Enable
      createConfigParam("Ch2:ADC:Enable",       'D', 0x0, 1, 9, 0);     // Chan2 ADC Enable
      createConfigParam("Ch3:ADC:Enable",       'D', 0x0, 1, 10, 0);    // Chan3 ADC Enable
      createConfigParam("Ch4:ADC:Enable",       'D', 0x0, 1, 11, 0);    // Chan4 ADC Enable
      createConfigParam("Ch5:ADC:Enable",       'D', 0x0, 1, 12, 0);    // Chan5 ADC Enable
      createConfigParam("Ch6:ADC:Enable",       'D', 0x0, 1, 13, 0);    // Chan6 ADC Enable
      createConfigParam("Ch7:ADC:Enable",       'D', 0x0, 1, 14, 0);    // Chan7 ADC Enable
      createConfigParam("Ch8:ADC:Enable",       'D', 0x0, 1, 15, 0);    // Chan8 ADC Enable
      createConfigParam("Ch1:ADC:RateCoef",     'D', 0x1, 16, 0, 0);    // Chan1 ADC Rate Coefficient
      createConfigParam("Ch2:ADC:RateCoef",     'D', 0x2, 16, 0, 0);    // Chan2 ADC Rate Coefficient
      createConfigParam("Ch3:ADC:RateCoef",     'D', 0x3, 16, 0, 0);    // Chan3 ADC Rate Coefficient
      createConfigParam("Ch4:ADC:RateCoef",     'D', 0x4, 16, 0, 0);    // Chan4 ADC Rate Coefficient
      createConfigParam("Ch5:ADC:RateCoef",     'D', 0x5, 16, 0, 0);    // Chan5 ADC Rate Coefficient
      createConfigParam("Ch6:ADC:RateCoef",     'D', 0x6, 16, 0, 0);    // Chan6 ADC Rate Coefficient
      createConfigParam("Ch7:ADC:RateCoef",     'D', 0x7, 16, 0, 0);    // Chan7 ADC Rate Coefficient
      createConfigParam("Ch8:ADC:RateCoef",     'D', 0x8, 16, 0, 0);    // Chan8 ADC Rate Coefficient
      createConfigParam("IdleTime",             'D', 0xD, 16, 0, 0);    // Idle time *** UNUSED BY FIRMWARE ***

      createConfigParam("FakeTrigPeriod",       'E', 0x0, 16, 0, 0);    // Fake trigger period
      createConfigParam("TsyncDelay",           'E', 0x1, 32, 0, 0);    // Tsync delay
      createConfigParam("EnableDigChs",         'E', 0x3, 16, 0, 0);    // Enables digital channels

      createConfigParam("ResetSource",          'F', 0x0,  1, 0, 0);    // Reset source
      createConfigParam("TclkSource",           'F', 0x0,  1, 1, 0);    // Tclk source
      createConfigParam("TsyncSource",          'F', 0x0,  1, 2, 0);    // Tsync source
      createConfigParam("TxEnForce",            'F', 0x0,  1, 3, 0);    // Tx Enable force
      createConfigParam("AcquireMode",          'F', 0x0,  2, 4, 0);    // Acquire Mode (0=normal,1=normal,2=normal,3=trigger)
      createConfigParam("OutputMode",           'F', 0x0,  1,10, 0);    // Output Mode
      createConfigParam("TpCtrl",               'F', 0x0,  5,11, 0);    // TP Control *** UNUSED BY FIRMWARE ***
}
