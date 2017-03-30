/* AdcRocPlugin_v02.cpp
 *
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "AdcRocPlugin.h"

void AdcRocPlugin::createParams_v02()
{
//    BLXXX:Det:RocXXX:| sig nam|      | EPICS record description | (bi and mbbi description)
    createStatusParam("Ch8:ADCTrig",          0x0,  1, 15);   // Detected ADC7 triggered
    createStatusParam("Ch7:ADCTrig",          0x0,  1, 14);   // Detected ADC6 triggered
    createStatusParam("Ch6:ADCTrig",          0x0,  1, 13);   // Detected ADC5 triggered
    createStatusParam("Ch5:ADCTrig",          0x0,  1, 12);   // Detected ADC4 triggered
    createStatusParam("Ch4:ADCTrig",          0x0,  1, 11);   // Detected ADC3 triggered
    createStatusParam("Ch3:ADCTrig",          0x0,  1, 10);   // Detected ADC2 triggered
    createStatusParam("Ch2:ADCTrig",          0x0,  1,  9);   // Detected ADC1 triggered
    createStatusParam("Ch1:ADCTrig",          0x0,  1,  8);   // Detected ADC0 triggered
    createStatusParam("Ch8:DIGTrig",          0x0,  1,  7);   // Detected DIG7 @ HIGH or LOW
    createStatusParam("Ch7:DIGTrig",          0x0,  1,  6);   // Detected DIG6 @ HIGH or LOW
    createStatusParam("Ch6:DIGTrig",          0x0,  1,  5);   // Detected DIG5 @ HIGH or LOW
    createStatusParam("Ch5:DIGTrig",          0x0,  1,  4);   // Detected DIG4 @ HIGH or LOW
    createStatusParam("Ch4:DIGTrig",          0x0,  1,  3);   // Detected DIG3 @ HIGH or LOW
    createStatusParam("Ch3:DIGTrig",          0x0,  1,  2);   // Detected DIG2 @ HIGH or LOW
    createStatusParam("Ch2:DIGTrig",          0x0,  1,  1);   // Detected DIG1 @ HIGH or LOW
    createStatusParam("Ch1:DIGTrig",          0x0,  1,  0);   // Detected DIG0 @ HIGH or LOW

    createStatusParam("Rsvd1",                0x1, 16,  0);   // Reserved status

    createStatusParam("DIG",                  0x2, 32,  0);   // DIG[32:0] scroc_8adc_acqa

    createStatusParam("LvdsVerify",           0x4,  1,  8);   // Cmd LVDS_VERIFY rcvd
    createStatusParam("SysrstBHigh",          0x4,  1,  7);   // Detected SYSRSTB @ HIGH
    createStatusParam("SysrstBLow",           0x4,  1,  6);   // Detected SYSRSTB @ LOW
    createStatusParam("TxenBHigh",            0x4,  1,  5);   // Detected TXENB @ HIGH
    createStatusParam("TxenBLow",             0x4,  1,  4);   // Detected TXENB @ LOW
    createStatusParam("TsyncHigh",            0x4,  1,  3);   // Detected TSYNC @ HIGH
    createStatusParam("TsyncLow",             0x4,  1,  2);   // Detected TSYNC @ LOW
    createStatusParam("TclkHigh",             0x4,  1,  1);   // Detected TCLK @ HIGH
    createStatusParam("TclkLow",              0x4,  1,  0);   // Detected TCLK @ LOWa

    createStatusParam("CmdOutputAF",          0x5,  1, 15);   // CMD Output FIFO almost full
    createStatusParam("ProgramErr",           0x5,  1, 14);   // WRITE_CNFG while ACQ
    createStatusParam("PktLengthErr",         0x5,  1, 13);   // Command length error
    createStatusParam("UnknownCmdErr",        0x5,  1, 12);   // Unrecognized command
    createStatusParam("TimerErr",             0x5,  1, 11);   // On ACQ, Time Stamp at max
    createStatusParam("Rsvd2",                0x5,  1, 10);   // Reserved
    createStatusParam("LvdsAF",               0x5,  1,  9);   // LVDS FIFO almost full
    createStatusParam("ErrPreStart",          0x5,  1,  8);   // LVDS start bit b/f stop bit
    createStatusParam("ErrNoStart",           0x5,  1,  7);   // LVDS data without start
    createStatusParam("ErrTimeout",           0x5,  1,  6);   // LVDS packet timeout
    createStatusParam("ErrLength",            0x5,  1,  5);   // LVDS packet length error
    createStatusParam("ErrType",              0x5,  1,  4);   // LVDS data type error
    createStatusParam("ErrParity",            0x5,  1,  3);   // LVDS parity error
    createStatusParam("Acquiring",            0x5,  1,  2);   // Acquiring data                    (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("Configured",           0x5,  1,  1);   // Module configured                 (0=not configured [alarm],1=configured, archive:monitor)
    createStatusParam("Discovered",           0x5,  1,  0);   // Module discovered                 (0=not discovered,1=discovered, archive:monitor)
    
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:DIG:PositionIdx",  '1', 0x0, 32, 0, 0x50000000);  // Chan1 Digital position index
    createConfigParam("Ch2:DIG:PositionIdx",  '1', 0x2, 32, 0, 0x50010000);  // Chan2 Digital position index
    createConfigParam("Ch3:DIG:PositionIdx",  '1', 0x4, 32, 0, 0x50020000);  // Chan3 Digital position index
    createConfigParam("Ch4:DIG:PositionIdx",  '1', 0x6, 32, 0, 0x50030000);  // Chan4 Digital position index
    createConfigParam("Ch5:DIG:PositionIdx",  '1', 0x8, 32, 0, 0x50040000);  // Chan5 Digital position index
    createConfigParam("Ch6:DIG:PositionIdx",  '1', 0xA, 32, 0, 0x50050000);  // Chan6 Digital position index
    createConfigParam("Ch7:DIG:PositionIdx",  '1', 0xC, 32, 0, 0x50060000);  // Chan7 Digital position index
    createConfigParam("Ch8:DIG:PositionIdx",  '1', 0xE, 32, 0, 0x50070000);  // Chan8 Digital position index
    createConfigParam("Ch1:ADC:PositionIdx",  '1', 0x10, 32, 0, 0x60000000); // Chan1 ADC position index
    createConfigParam("Ch2:ADC:PositionIdx",  '1', 0x12, 32, 0, 0x60010000); // Chan2 ADC position index
    createConfigParam("Ch3:ADC:PositionIdx",  '1', 0x14, 32, 0, 0x60020000); // Chan3 ADC position index
    createConfigParam("Ch4:ADC:PositionIdx",  '1', 0x16, 32, 0, 0x60030000); // Chan4 ADC position index
    createConfigParam("Ch5:ADC:PositionIdx",  '1', 0x18, 32, 0, 0x60040000); // Chan5 ADC position index
    createConfigParam("Ch6:ADC:PositionIdx",  '1', 0x1A, 32, 0, 0x60050000); // Chan6 ADC position index
    createConfigParam("Ch7:ADC:PositionIdx",  '1', 0x1C, 32, 0, 0x60060000); // Chan7 ADC position index
    createConfigParam("Ch8:ADC:PositionIdx",  '1', 0x1E, 32, 0, 0x60070000); // Chan8 ADC position index
    createConfigParam("PositionIdxBus",       '1', 0x20, 32, 0, 0);          // Pos Index bus *** UNUSED ***

    createConfigParam("Ch1:ADC:Threshold",    'C', 0x0, 16, 0, 10);  // Chan1 ADC Threshold
    createConfigParam("Ch2:ADC:Threshold",    'C', 0x1, 16, 0, 10);  // Chan2 ADC Threshold
    createConfigParam("Ch3:ADC:Threshold",    'C', 0x2, 16, 0, 10);  // Chan3 ADC Threshold
    createConfigParam("Ch4:ADC:Threshold",    'C', 0x3, 16, 0, 10);  // Chan4 ADC Threshold
    createConfigParam("Ch5:ADC:Threshold",    'C', 0x4, 16, 0, 10);  // Chan5 ADC Threshold
    createConfigParam("Ch6:ADC:Threshold",    'C', 0x5, 16, 0, 10);  // Chan6 ADC Threshold
    createConfigParam("Ch7:ADC:Threshold",    'C', 0x6, 16, 0, 10);  // Chan7 ADC Threshold
    createConfigParam("Ch8:ADC:Threshold",    'C', 0x7, 16, 0, 10);  // Chan8 ADC Threshold
    createConfigParam("PmTsyncDelay",         'C', 0x8, 24, 0, 0);   // Pulsed Magnet Tsync Delay
    createConfigParam("PmRatePeriod",         'C', 0xA, 16, 0, 1000);  // Pulsed Magnet Rate Period
    createConfigParam("Ch1:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan1 Digital mode
    createConfigParam("Ch2:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan2 Digital mode
    createConfigParam("Ch3:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan3 Digital mode
    createConfigParam("Ch4:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan4 Digital mode
    createConfigParam("Ch5:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan5 Digital mode
    createConfigParam("Ch6:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan6 Digital mode
    createConfigParam("Ch7:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan7 Digital mode
    createConfigParam("Ch8:DIG:Mode",         'C', 0xB,  2, 0, 0);   // Chan8 Digital mode

    createConfigParam("AdcOversampling",      'D', 0x0, 3, 0, 0);     // ADC Oversampling
    createConfigParam("AdcRange",             'D', 0x0, 1, 4, 1);     // ADC Range (0=5V,1=10V)
    createConfigParam("PmEnable",             'D', 0x0, 1, 5, 1);     // Pulsed Magnet Enable
    createConfigParam("Ch1:ADC:Enable",       'D', 0x0, 1, 8, 1);     // Chan1 ADC Enable
    createConfigParam("Ch2:ADC:Enable",       'D', 0x0, 1, 9, 1);     // Chan2 ADC Enable
    createConfigParam("Ch3:ADC:Enable",       'D', 0x0, 1, 10, 1);    // Chan3 ADC Enable
    createConfigParam("Ch4:ADC:Enable",       'D', 0x0, 1, 11, 1);    // Chan4 ADC Enable
    createConfigParam("Ch5:ADC:Enable",       'D', 0x0, 1, 12, 1);    // Chan5 ADC Enable
    createConfigParam("Ch6:ADC:Enable",       'D', 0x0, 1, 13, 1);    // Chan6 ADC Enable
    createConfigParam("Ch7:ADC:Enable",       'D', 0x0, 1, 14, 1);    // Chan7 ADC Enable
    createConfigParam("Ch8:ADC:Enable",       'D', 0x0, 1, 15, 1);    // Chan8 ADC Enable
    createConfigParam("Ch1:ADC:RateCoef",     'D', 0x1, 16, 0, 1000); // Chan1 ADC Rate Coefficient
    createConfigParam("Ch2:ADC:RateCoef",     'D', 0x2, 16, 0, 1000); // Chan2 ADC Rate Coefficient
    createConfigParam("Ch3:ADC:RateCoef",     'D', 0x3, 16, 0, 1000); // Chan3 ADC Rate Coefficient
    createConfigParam("Ch4:ADC:RateCoef",     'D', 0x4, 16, 0, 1000); // Chan4 ADC Rate Coefficient
    createConfigParam("Ch5:ADC:RateCoef",     'D', 0x5, 16, 0, 1000); // Chan5 ADC Rate Coefficient
    createConfigParam("Ch6:ADC:RateCoef",     'D', 0x6, 16, 0, 1000); // Chan6 ADC Rate Coefficient
    createConfigParam("Ch7:ADC:RateCoef",     'D', 0x7, 16, 0, 1000); // Chan7 ADC Rate Coefficient
    createConfigParam("Ch8:ADC:RateCoef",     'D', 0x8, 16, 0, 1000); // Chan8 ADC Rate Coefficient
    createConfigParam("IdleTime",             'D', 0xD, 16, 0, 0);    // Idle time *** UNUSED ***

    createConfigParam("FakeTrigPeriod",       'E', 0x0, 16, 0, 0);    // Fake trigger period
    createConfigParam("TsyncDelay",           'E', 0x1, 32, 0, 0);    // Tsync delay
    createConfigParam("EnableDigChs",         'E', 0x3, 16, 0, 31);   // Enables digital channels

    createConfigParam("Reset",                'F', 0x0,  1, 0, 0);    // Reset source (0=disable,1=enable)
    createConfigParam("TclkSelect",           'F', 0x0,  1, 1, 0);    // Tclk source (0=external,1=internal)
    createConfigParam("TsyncSelect",          'F', 0x0,  1, 2, 0);    // Tsync source (0=external,1=internal)
    createConfigParam("TxEnable",             'F', 0x0,  1, 3, 1);    // Tx Enable force (0=do not force,1=force)
    createConfigParam("AcquireMode",          'F', 0x0,  2, 4, 2);    // Acquire Mode (0=idle,1=idle,2=normal,3=trigger)
    createConfigParam("OutputMode",           'F', 0x0,  1,10, 1);    // Output Mode (0=slow,1=40MHz)
    createConfigParam("TpCtrl",               'F', 0x0,  5,11, 0);    // TP Control *** UNUSED ***
}
