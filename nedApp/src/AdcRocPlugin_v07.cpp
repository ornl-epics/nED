/* AdcRocPlugin_v07.cpp
 *
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Vladislav Sedov
 * @author Klemen Vodopivec
 */

#include "AdcRocPlugin.h"

void AdcRocPlugin::createParams_v07()
{
//    BLXXX:Det:RocXXX:| sig nam|      | EPICS record description | (bi and mbbi description)
    createStatusParam("Dig0:Trigger",       0x0,  1,  0);   // Detected DIG0 Tsync DAQ      (0=low,1=high)
    createStatusParam("Dig1:Trigger",       0x0,  1,  1);   // Detected DIG1                (0=low,1=high)
    createStatusParam("Dig2:Trigger",       0x0,  1,  2);   // Detected DIG2                (0=low,1=high)
    createStatusParam("Dig3:Trigger",       0x0,  1,  3);   // Detected DIG3                (0=low,1=high)
    createStatusParam("Dig4:Trigger",       0x0,  1,  4);   // Detected DIG4                (0=low,1=high)
    createStatusParam("Dig5:Trigger",       0x0,  1,  5);   // Detected DIG5                (0=low,1=high)
    createStatusParam("Dig6:Trigger",       0x0,  1,  6);   // Detected DIG6                (0=low,1=high)
    createStatusParam("Dig7:Trigger",       0x0,  1,  7);   // Detected DIG7                (0=low,1=high)
    createStatusParam("Adc0:Trigger",       0x0,  1,  8);   // Detected ADC0 triggered      (0=no,1=yes)
    createStatusParam("Adc1:Trigger",       0x0,  1,  9);   // Detected ADC1 triggered      (0=no,1=yes)
    createStatusParam("Adc2:Trigger",       0x0,  1, 10);   // Detected ADC2 triggered      (0=no,1=yes)
    createStatusParam("Adc3:Trigger",       0x0,  1, 11);   // Detected ADC3 triggered      (0=no,1=yes)
    createStatusParam("Adc4:Trigger",       0x0,  1, 12);   // Detected ADC4 triggered      (0=no,1=yes)
    createStatusParam("Adc5:Trigger",       0x0,  1, 13);   // Detected ADC5 triggered      (0=no,1=yes)
    createStatusParam("Adc6:Trigger",       0x0,  1, 14);   // Detected ADC6 triggered      (0=no,1=yes)
    createStatusParam("Adc7:Trigger",       0x0,  1, 15);   // Detected ADC7 triggered      (0=no,1=yes)

    createStatusParam("DIG",                0x2, 32,  0);   // DIG[32:0] scroc_8adc_acqa

    createStatusParam("TclkLow",            0x4,  1,  0);   // Detected TCLK                (0=no,1=yes)
    createStatusParam("TclkHigh",           0x4,  1,  1);   // Detected TCLK                (0=no,1=yes)
    createStatusParam("TsyncLow",           0x4,  1,  2);   // Detected TSYNC               (0=no,1=yes)
    createStatusParam("TsyncHigh",          0x4,  1,  3);   // Detected TSYNC               (0=no,1=yes)
    createStatusParam("TxenBLow",           0x4,  1,  4);   // Detected TXENB               (0=no,1=yes)
    createStatusParam("TxenBHigh",          0x4,  1,  5);   // Detected TXENB               (0=no,1=yes)
    createStatusParam("SysrstBLow",         0x4,  1,  6);   // Detected SYSRSTB             (0=no,1=yes)
    createStatusParam("SysrstBHigh",        0x4,  1,  7);   // Detected SYSRSTB             (0=no,1=yes)
    createStatusParam("LvdsVerify",         0x4,  1,  8);   // Cmd LVDS_VERIFY rcvd

    createStatusParam("Discovered",         0x5,  1,  0);   // Module discovered            (0=not discovered,1=discovered, archive:monitor)
    createStatusParam("Configured",         0x5,  1,  1);   // Module configured            (0=not configured [alarm],1=configured, archive:monitor)
    createStatusParam("Acquiring",          0x5,  1,  2);   // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("ErrParity",          0x5,  1,  3);   // LVDS parity error            (0=no,1=yes)
    createStatusParam("ErrType",            0x5,  1,  4);   // LVDS data type error         (0=no,1=yes)
    createStatusParam("ErrLength",          0x5,  1,  5);   // LVDS packet length error     (0=no,1=yes)
    createStatusParam("ErrTimeout",         0x5,  1,  6);   // LVDS packet timeout          (0=no,1=yes)
    createStatusParam("ErrNoStart",         0x5,  1,  7);   // LVDS data without start      (0=no,1=yes)
    createStatusParam("ErrPreStart",        0x5,  1,  8);   // LVDS start bit b/f stop bit  (0=no,1=yes)
    createStatusParam("LvdsAF",             0x5,  1,  9);   // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("ErrNoTsync",         0x5,  1, 11);   // On ACQ, Time Stamp at max    (0=no,1=yes)
    createStatusParam("ErrCmdBad",          0x5,  1, 12);   // Unrecognized command         (0=no,1=yes)
    createStatusParam("ErrCmdLength",       0x5,  1, 13);   // Command length error         (0=no,1=yes)
    createStatusParam("ErrProgramming",     0x5,  1, 14);   // WRITE_CNFG while ACQ         (0=no,1=yes)
    createStatusParam("CmdOutputAF",        0x5,  1, 15);   // CMD Output FIFO almost full  (0=no,1=yes)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createCounterParam("CntParity",         0x0,  16,  0);  // LVDS parity err counter
    createCounterParam("CntType",           0x1,  16,  0);  // LVDS data type err counter
    createCounterParam("CntLength",         0x2,  16,  0);  // LVDS length err counter
    createCounterParam("CntTimeout",        0x3,  16,  0);  // LVDS timeout err counter
    createCounterParam("CntNoStart",        0x4,  16,  0);  // LVDS no start err counter
    createCounterParam("CntPreStart",       0x5,  16,  0);  // LVDS start b/f stop count
    createCounterParam("CntFull",           0x6,  16,  0);  // LVDS fifo full err counter
    createCounterParam("CntTimerErr",       0x7,  16,  0);  // timer err counter
    createCounterParam("CntUnknownCmdErr",  0x8,  16,  0);  // unknown command err counter
    createCounterParam("CntPktLengthErr",   0x9,  16,  0);  // packet length err counter
    createCounterParam("CntProgramErr",     0xA,  16,  0);  // programming err counter
    createCounterParam("CntFifoFull",       0xB,  16,  0);  // board output FIFO full cnt
    createCounterParam("Dig0:CntFifoFull",  0xC,  16,  0);  // DIG0 output FIFO full cnt  Tsync DAQ
    createCounterParam("Dig1:CntFifoFull",  0xD,  16,  0);  // DIG1 output FIFO full cnt
    createCounterParam("Dig2:CntFifoFull",  0xE,  16,  0);  // DIG2 output FIFO full cnt
    createCounterParam("Dig3:CntFifoFull",  0xF,  16,  0);  // DIG3 output FIFO full cnt
    createCounterParam("Dig4:CntFifoFull",  0x10, 16,  0);  // DIG4 output FIFO full cnt
    createCounterParam("Dig5:CntFifoFull",  0x11, 16,  0);  // DIG5 output FIFO full cnt
    createCounterParam("Dig6:CntFifoFull",  0x12, 16,  0);  // DIG6 output FIFO full cnt
    createCounterParam("Dig7:CntFifoFull",  0x13, 16,  0);  // DIG7 output FIFO full cnt
    createCounterParam("Adc0:CntFifoFull",  0x14, 16,  0);  // ADC0 output FIFO full cnt
    createCounterParam("Adc1:CntFifoFull",  0x15, 16,  0);  // ADC1 output FIFO full cnt
    createCounterParam("Adc2:CntFifoFull",  0x16, 16,  0);  // ADC2 output FIFO full cnt
    createCounterParam("Adc3:CntFifoFull",  0x17, 16,  0);  // ADC3 output FIFO full cnt
    createCounterParam("Adc4:CntFifoFull",  0x18, 16,  0);  // ADC4 output FIFO full cnt
    createCounterParam("Adc5:CntFifoFull",  0x19, 16,  0);  // ADC5 output FIFO full cnt
    createCounterParam("Adc6:CntFifoFull",  0x1A, 16,  0);  // ADC6 output FIFO full cnt
    createCounterParam("Adc7:CntFifoFull",  0x1B, 16,  0);  // ADC7 output FIFO full cnt
    createCounterParam("CntLinkDisconnect", 0x1C, 16,  0);  // Ch Link disconnect counter
    createCounterParam("Dig0:RatePulses",   0x1D, 16,  0);  // DIG0 ratemeter Tsync DAQ        (scale:19.073,unit:per sec)
    createCounterParam("Dig1:RatePulses",   0x1E, 16,  0);  // DIG1 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig2:RatePulses",   0x1F, 16,  0);  // DIG2 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig3:RatePulses",   0x20, 16,  0);  // DIG3 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig4:RatePulses",   0x21, 16,  0);  // DIG4 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig5:RatePulses",   0x22, 16,  0);  // DIG5 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig6:RatePulses",   0x23, 16,  0);  // DIG6 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig7:RatePulses",   0x24, 16,  0);  // DIG7 ratemeter                  (scale:19.073,unit:per sec)
    createCounterParam("Dig0:RateEvents",   0x25, 16,  0);  // DIG0 events ratemeter Tsync DAQ (scale:19.073,unit:per sec)
    createCounterParam("Dig1:RateEvents",   0x26, 16,  0);  // DIG1 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Dig2:RateEvents",   0x27, 16,  0);  // DIG2 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Dig3:RateEvents",   0x28, 16,  0);  // DIG3 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Dig4:RateEvents",   0x29, 16,  0);  // DIG4 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Dig5:RateEvents",   0x2A, 16,  0);  // DIG5 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Dig6:RateEvents",   0x2B, 16,  0);  // DIG6 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Dig7:RateEvents",   0x2C, 16,  0);  // DIG7 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc0:RatePulses",   0x2D, 16,  0);  // ADC0 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc1:RatePulses",   0x2E, 16,  0);  // ADC1 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc2:RatePulses",   0x2F, 16,  0);  // ADC2 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc3:RatePulses",   0x30, 16,  0);  // ADC3 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc4:RatePulses",   0x31, 16,  0);  // ADC4 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc5:RatePulses",   0x32, 16,  0);  // ADC5 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc6:RatePulses",   0x33, 16,  0);  // ADC6 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc7:RatePulses",   0x34, 16,  0);  // ADC7 Ok pulses ratemeter        (scale:19.073,unit:per sec)
    createCounterParam("Adc0:RateEvents",   0x35, 16,  0);  // ADC0 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc1:RateEvents",   0x36, 16,  0);  // ADC1 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc2:RateEvents",   0x37, 16,  0);  // ADC2 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc3:RateEvents",   0x38, 16,  0);  // ADC3 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc4:RateEvents",   0x39, 16,  0);  // ADC4 events ratemeter<UCAMS ID> (scale:19.073,unit:per sec)
    createCounterParam("Adc5:RateEvents",   0x3A, 16,  0);  // ADC5 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc6:RateEvents",   0x3B, 16,  0);  // ADC6 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("Adc7:RateEvents",   0x3C, 16,  0);  // ADC7 events ratemeter           (scale:19.073,unit:per sec)
    createCounterParam("RateOut",           0x3D, 16,  0);  // Board total events ratemeter    (scale:19.073,unit:per sec)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("Dig0:PositionIdx",   '1', 0x0,  32, 0, 0x50000000); // Chan0 Digital position index Tsync DAQ
    createConfigParam("Dig1:PositionIdx",   '1', 0x2,  32, 0, 0x50010000); // Chan1 Digital position index
    createConfigParam("Dig2:PositionIdx",   '1', 0x4,  32, 0, 0x50020000); // Chan2 Digital position index
    createConfigParam("Dig3:PositionIdx",   '1', 0x6,  32, 0, 0x50030000); // Chan3 Digital position index
    createConfigParam("Dig4:PositionIdx",   '1', 0x8,  32, 0, 0x50040000); // Chan4 Digital position index
    createConfigParam("Dig5:PositionIdx",   '1', 0xA,  32, 0, 0x50050000); // Chan5 Digital position index
    createConfigParam("Dig6:PositionIdx",   '1', 0xC,  32, 0, 0x50060000); // Chan6 Digital position index
    createConfigParam("Dig7:PositionIdx",   '1', 0xE,  32, 0, 0x50070000); // Chan7 Digital position index
    createConfigParam("Adc0:PositionIdx",   '1', 0x10, 32, 0, 0x60000000); // Chan0 ADC position index
    createConfigParam("Adc1:PositionIdx",   '1', 0x12, 32, 0, 0x60010000); // Chan1 ADC position index
    createConfigParam("Adc2:PositionIdx",   '1', 0x14, 32, 0, 0x60020000); // Chan2 ADC position index
    createConfigParam("Adc3:PositionIdx",   '1', 0x16, 32, 0, 0x60030000); // Chan3 ADC position index
    createConfigParam("Adc4:PositionIdx",   '1', 0x18, 32, 0, 0x60040000); // Chan4 ADC position index
    createConfigParam("Adc5:PositionIdx",   '1', 0x1A, 32, 0, 0x60050000); // Chan5 ADC position index
    createConfigParam("Adc6:PositionIdx",   '1', 0x1C, 32, 0, 0x60060000); // Chan6 ADC position index
    createConfigParam("Adc7:PositionIdx",   '1', 0x1E, 32, 0, 0x60070000); // Chan7 ADC position index

    createConfigParam("Adc0:Threshold",     'C', 0x0, 16,  0, 10);   // Chan0 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc1:Threshold",     'C', 0x1, 16,  0, 10);   // Chan1 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc2:Threshold",     'C', 0x2, 16,  0, 10);   // Chan2 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc3:Threshold",     'C', 0x3, 16,  0, 10);   // Chan3 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc4:Threshold",     'C', 0x4, 16,  0, 10);   // Chan4 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc5:Threshold",     'C', 0x5, 16,  0, 10);   // Chan5 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc6:Threshold",     'C', 0x6, 16,  0, 10);   // Chan6 ADC Threshold             (unit:ADC LSB)
    createConfigParam("Adc7:Threshold",     'C', 0x7, 16,  0, 10);   // Chan7 ADC Threshold             (unit:ADC LSB)
    createConfigParam("PmTsyncDelay",       'C', 0x8, 25,  0, 0);    // Pulsed Magnet Tsync Delay       (scale:50,unit:ns)
    createConfigParam("PmRatePeriod",       'C', 0xA, 16,  0, 1000); // Pulsed Magnet Rate Period       (scale:16,unit:ms)
    createConfigParam("Dig0:Mode",          'C', 0xB,  2,  0, 0);    // Chan0 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig1:Mode",          'C', 0xB,  2,  2, 0);    // Chan1 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig2:Mode",          'C', 0xB,  2,  4, 0);    // Chan2 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig3:Mode",          'C', 0xB,  2,  6, 0);    // Chan3 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig4:Mode",          'C', 0xB,  2,  8, 0);    // Chan4 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig5:Mode",          'C', 0xB,  2, 10, 0);    // Chan5 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig6:Mode",          'C', 0xB,  2, 12, 0);    // Chan6 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)
    createConfigParam("Dig7:Mode",          'C', 0xB,  2, 14, 0);    // Chan7 Digital Edge mode         (0=None,1=Rising,2=Falling,3=Both)

    createConfigParam("AdcOversampling",    'D', 0x0,  3,  0, 0);   // ADC Oversampling
    createConfigParam("MPAlwaysReady",      'D', 0x0,  1,  3, 0);   // MP Always Ready                  (0=OFF,1=ON)
    createConfigParam("AdcRange",           'D', 0x0,  1,  4, 1);   // ADC Range                        (0=+/-5V,1=+/-10V)
    createConfigParam("PmEnable",           'D', 0x0,  1,  5, 0);   // Pulsed Magnet Enable             (0=disable,1=enable)
    createConfigParam("Adc0:Enable",        'D', 0x0,  1,  8, 1);   // Chan0 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc1:Enable",        'D', 0x0,  1,  9, 1);   // Chan1 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc2:Enable",        'D', 0x0,  1, 10, 1);   // Chan2 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc3:Enable",        'D', 0x0,  1, 11, 1);   // Chan3 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc4:Enable",        'D', 0x0,  1, 12, 1);   // Chan4 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc5:Enable",        'D', 0x0,  1, 13, 1);   // Chan5 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc6:Enable",        'D', 0x0,  1, 14, 1);   // Chan6 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc7:Enable",        'D', 0x0,  1, 15, 1);   // Chan7 ADC Enable                 (0=disable,1=enable)
    createConfigParam("Adc0:RateIdle",      'D', 0x1, 16,  0, 1000); // Chan0 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc1:RateIdle",      'D', 0x2, 16,  0, 1000); // Chan1 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc2:RateIdle",      'D', 0x3, 16,  0, 1000); // Chan2 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc3:RateIdle",      'D', 0x4, 16,  0, 1000); // Chan3 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc4:RateIdle",      'D', 0x5, 16,  0, 1000); // Chan4 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc5:RateIdle",      'D', 0x6, 16,  0, 1000); // Chan5 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc6:RateIdle",      'D', 0x7, 16,  0, 1000); // Chan6 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)
    createConfigParam("Adc7:RateIdle",      'D', 0x8, 16,  0, 1000); // Chan7 ADC Idle Rate             (calc:100000/(A+1),unit:Hz)

    createConfigParam("Adc0:TsyncSrc",      'D', 0x9,  2,  0,  0);  // TOF src for ADC ch 0             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc1:TsyncSrc",      'D', 0x9,  2,  4,  0);  // TOF src for ADC ch 1             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc2:TsyncSrc",      'D', 0x9,  2,  8,  0);  // TOF src for ADC ch 2             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc3:TsyncSrc",      'D', 0x9,  2, 12, 0);   // TOF src for ADC ch 3             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc4:TsyncSrc",      'D', 0xA,  2,  0,  0);  // TOF src for ADC ch 4             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc5:TsyncSrc",      'D', 0xA,  2,  4,  0);  // TOF src for ADC ch 5             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc6:TsyncSrc",      'D', 0xA,  2,  8,  0);  // TOF src for ADC ch 6             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Adc7:TsyncSrc",      'D', 0xA,  2, 12, 0);   // TOF src for ADC ch 7             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)

    createConfigParam("Dig0:TsyncSrc",      'D', 0xB,  2,  0,  0);  // TOF src for Dig ch 0 Tsync DAQ   (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig1:TsyncSrc",      'D', 0xB,  2,  4,  0);  // TOF src for Dig ch 1             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig2:TsyncSrc",      'D', 0xB,  2,  8,  0);  // TOF src for Dig ch 2             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig3:TsyncSrc",      'D', 0xB,  2, 12, 0);   // TOF src for Dig ch 3             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig4:TsyncSrc",      'D', 0xC,  2,  0,  0);  // TOF src for Dig ch 4             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig5:TsyncSrc",      'D', 0xC,  2,  4,  0);  // TOF src for Dig ch 5             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig6:TsyncSrc",      'D', 0xC,  2,  8,  0);  // TOF src for Dig ch 6             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)
    createConfigParam("Dig7:TsyncSrc",      'D', 0xC,  2, 12, 0);   // TOF src for Dig ch 7             (0=Tsync delay 0,1=Tsync delay 1,2=Tsync delay 2,3=Tsync delay 3)

    createConfigParam("Ctrl0:TsyncSrc",     'D', 0xD,  2,  0,  0);  // Tsync src for CTRL ch 0 Tsync    (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl1:TsyncSrc",     'D', 0xD,  2,  4,  0);  // Tsync src for CTRL ch 1 PM       (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl2:TsyncSrc",     'D', 0xD,  2,  8,  0);  // Tsync src for CTRL ch 2 HP       (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl3:TsyncSrc",     'D', 0xD,  2, 12,  0);  // Tsync src for CTRL ch 3          (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl4:TsyncSrc",     'D', 0xE,  2,  0,  0);  // Tsync src for CTRL ch 4          (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl5:TsyncSrc",     'D', 0xE,  2,  4,  0);  // Tsync src for CTRL ch 5          (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl6:TsyncSrc",     'D', 0xE,  2,  8,  0);  // Tsync src for CTRL ch 6          (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)
    createConfigParam("Ctrl7:TsyncSrc",     'D', 0xE,  2, 12,  0);  // Tsync src for CTRL ch 7          (0=TsyncDelay0,1=TsyncDelay1,2=TsyncDelay2,3=TsyncDelay3)

    createConfigParam("FakeTrigPeriod",     'E', 0x0, 16,  0, 0);   // Fake trigger period
    createConfigParam("HpTsyncDelay",       'E', 0x1, 20,  0, 0);   // Heat pulser: delay PMOS_ON from Tsync = (Code*4 + 1)*25ns, Code is 20-bit
    createConfigParam("NmosOnWidth",        'E', 0x3, 24,  0, 0);   // NMOS_ON pulse width              (calc:(A+1)*100,unit:ns)
    createConfigParam("PmosOnWidth",        'E', 0x5, 10,  0, 0);   // PMOS_ON pulse width              (calc:(A+1)*100,unit:ns)
    createConfigParam("NmosOnDelay",        'E', 0x5,  4, 12, 0);   // NMOS pulse delay after PMOS      (calc:(A+1)*100,unit:ns)
    createConfigParam("HpRate",             'E', 0x6, 10,  0, 0);   // Heat pulser rate: TsyncFrequency/(Code+1), Code is 10-bit
    createConfigParam("HpEnable",           'E', 0x6,  1, 12, 0);   // Heat pulser enable               (0=disable,1=enable)
    createConfigParam("TsyncDelay0",        'E', 0x7, 32,  0, 0);   // Tsync Delay 0                    (scale:100,unit:ns)
    createConfigParam("TsyncDelay1",        'E', 0x9, 32,  0, 0);   // Tsync Delay 1                    (scale:100,unit:ns)
    createConfigParam("TsyncDelay2",        'E', 0xB, 32,  0, 0);   // Tsync Delay 2                    (scale:100,unit:ns)
    createConfigParam("TsyncDelay3",        'E', 0xD, 32,  0, 0);   // Tsync Delay 3                    (scale:100,unit:ns)

    createConfigParam("LvdsReset",          'F', 0x0,  1,  0,  0);  // Reset source                     (0=disable,1=enable)
    createConfigParam("TclkSelect",         'F', 0x0,  1,  1,  0);  // Tclk source                      (0=external,1=internal)
    createConfigParam("TsyncSelect",        'F', 0x0,  1,  2,  0);  // Tsync source                     (0=external,1=internal)
    createConfigParam("TxEnable",           'F', 0x0,  1,  3,  1);  // Tx Enable force                  (0=do not force,1=force)
    createConfigParam("AcquireMode",        'F', 0x0,  2,  4,  2);  // Acquire Mode                     (0=idle,1=idle,2=normal,3=trigger)
    createConfigParam("GetDigChnls",        'F', 0x0,  1,  6,  2);  // Acquire Digital lines on Start   (0=disable,1=enable)
    createConfigParam("OutputSpeed",        'F', 0x0,  1, 10,  1);  // Output Mode                      (0=slow,1=40MHz)

    createConfigParam("TestPatternId",      'F', 0x1, 12,  0,  0);  // Test pattern id
    createConfigParam("TestPatternDebug",   'F', 0x1,  3, 12,  0);  // Engineering Use only
    createConfigParam("TestPatternEn",      'F', 0x1,  1, 15,  0);  // Test pattern enable              (0=disable,1=enable)
    createConfigParam("TestPatternRate",    'F', 0x2, 16,  0, 65535);// Test pattern rate                (65535=153 ev/s,9999=1 Kev/s,4999=2 Kev/s,1999=5 Kev/s,999=10 Kev/s,399=25 Kev/s,199=50 Kev/s,99=100 Kev/s,13=800 Kev/s,9=1 Mev/s,4=2 Mev/s,1=5 Mev/s,0=10 Mev/s)
    createConfigParam("AcqRate",            'F', 0x3, 10,  0,  1);  // Num cycles to advance @TSYNC     (1=60Hz,2=30Hz,3=20Hz,4=15Hz,6=10Hz,12=5Hz,60=1Hz)
    createConfigParam("DataFormatId",       'F', 0x4,  8,  0,  2);  // Data format identifier           (1=metadata,2=pixel)
    createConfigParam("Protocol",           'F', 0x4,  1, 15,  0);  // LVDS protocol select             (0=legacy,1=new)
}
