/* RocPlugin_v45.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Bogdan Vacaliuc
 */

#include "RocPlugin.h"

void RocPlugin::createStatusParams_v45()
{
//    BLXXX:Det:RocXXX:| sig nam|                              | EPICS record description | (bi and mbbi description)
    createStatusParam("ErrProgramming",       0x0,  1, 15); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("ErrCmdLength",         0x0,  1, 14); // Command length error         (0=no error,1=error)
    createStatusParam("ErrCmdBad",            0x0,  1, 13); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("DataFifoAlmostFull",   0x0,  1, 12); // Data FIFO Almost full flag.  (0=not full,1=almost full)
    createStatusParam("DataFifoEmpty",        0x0,  1, 11); // Data FIFO Empty flag.        (1=not empty,0=empty)
    createStatusParam("CalcBadFinal",         0x0,  1, 10); // Calc: Bad Final Calculation  (0=no error,1=error)
    createStatusParam("CalcBadEffect",        0x0,  1,  9); // Calc: Bad Effective Calculat (0=no error,1=error)
    createStatusParam("CalcBadOverlow",       0x0,  1,  8); // Calc: Data overflow detected (0=no error,1=error)
    createStatusParam("CalcBadCount",         0x0,  1,  7); // Calc: Bad word count.        (0=no error,1=error)
    createStatusParam("ErrFifoFull",          0x0,  1,  6); // LVDS FIFO went full.         (0=not full,1=full)
    createStatusParam("ErrPreStart",          0x0,  1,  5); // LVDS start before stop bit   (0=no error,1=error)
    createStatusParam("ErrNoStart",           0x0,  1,  4); // LVDS data without start.     (0=no error,1=error)
    createStatusParam("ErrTimeout",           0x0,  1,  3); // LVDS packet timeout.         (0=no timeout,1=timeout)
    createStatusParam("ErrLength",            0x0,  1,  2); // LVDS packet length error.    (0=no error,1=error)
    createStatusParam("ErrType",              0x0,  1,  1); // LVDS packet type error.      (0=no error,1=error)
    createStatusParam("ErrParity",            0x0,  1,  0); // LVDS parity error.           (0=no error,1=error)

    createStatusParam("LvdsVerify",           0x1,  1, 10); // LVDS VERIFY detected         (0=not detected,1=detected)
    createStatusParam("LvdsPowerdown",        0x1,  1,  9); // LVDS POWERDOWN detected      (0=not detected,1=detected)
    createStatusParam("LvdsReset",            0x1,  1,  8); // LVDS RESET detected          (0=not detected,1=detected)
    createStatusParam("ErrNoTsync",           0x1,  1,  1); // Timestamp overflow error.    (0=no error,1=error)
    createStatusParam("ErrFatal",             0x1,  1,  0); // FATAL ERROR detected         (0=not detected,1=detected)

    createStatusParam("DataReady",            0x2,  8,  8); // Channel DATA Ready           (0=not ready,1=ready)
    createStatusParam("DataFpgaReady",        0x2,  8,  0); // Channel FPGA Ready           (1=not ready,0=ready)

    createStatusParam("HVStatus",             0x3,  1, 15); // High Voltage Status bit      (0=not present,1=present)
    createStatusParam("CalcFifoAlmostFull",   0x3,  1, 14); // Calc: FIFO Almost full       (0=not full,1=almost full)
    createStatusParam("CalcFifoEmpty",        0x3,  1, 13); // Calc: FIFO Empty flag        (1=not empty,0=empty)
    createStatusParam("CalcInputAlmostFull",  0x3,  1, 12); // Calc: INPUT Almost full      (0=not full,1=almost full)
    createStatusParam("CalcInputEmpty",       0x3,  1, 11); // Calc: Input Empty flag       (1=not empty,0=empty)
    createStatusParam("TxenBHigh",            0x3,  1, 10); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("TxenBLow",             0x3,  1,  9); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("SysrstBHigh",          0x3,  1,  8); // SYSRST_B Got HIGH            (0=no,1=yes)
    createStatusParam("SysrstBLow",           0x3,  1,  7); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("TsyncHigh",            0x3,  1,  6); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("TsyncLow",             0x3,  1,  5); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TclkHigh",             0x3,  1,  4); // TCLK Got HIGH                (0=no,1=yes)
    createStatusParam("TclkLow",              0x3,  1,  3); // TCLK Got LOW                 (0=no,1=yes)
    createStatusParam("CalcActive",           0x3,  1,  2); // Calculation: Active          (0=not active,1=active)
    createStatusParam("Acquiring",            0x3,  1,  1); // Acquiring data               (0=not acquiring [alarm],1=acquiring)
    createStatusParam("Discovered",           0x3,  1,  0); // Discovered                   (0=not discovered,1=discovered)
}

void RocPlugin::createConfigParams_v45()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:A:Scale",      '1',  0x0, 12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch1:A:Offset",     '1',  0x1, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch1:B:Scale",      '1',  0x2, 12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch1:B:Offset",     '1',  0x3, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:A:Scale",      '1',  0x4, 12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch2:A:Offset",     '1',  0x5, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch2:B:Scale",      '1',  0x6, 12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch2:B:Offset",     '1',  0x7, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:A:Scale",      '1',  0x8, 12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch3:A:Offset",     '1',  0x9, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch3:B:Scale",      '1',  0xA, 12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch3:B:Offset",     '1',  0xB, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:A:Scale",      '1',  0xC, 12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch4:A:Offset",     '1',  0xD, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch4:B:Scale",      '1',  0xE, 12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch4:B:Offset",     '1',  0xF, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5:A:Scale",      '1', 0x10, 12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch5:A:Offset",     '1', 0x11, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch5:B:Scale",      '1', 0x12, 12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch5:B:Offset",     '1', 0x13, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:A:Scale",      '1', 0x14, 12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch6:A:Offset",     '1', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch6:B:Scale",      '1', 0x16, 12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch6:B:Offset",     '1', 0x17, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:A:Scale",      '1', 0x18, 12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch7:A:Offset",     '1', 0x19, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch7:B:Scale",      '1', 0x1A, 12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch7:B:Offset",     '1', 0x1B, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:A:Scale",      '1', 0x1C, 12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch8:A:Offset",     '1', 0x1D, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch8:B:Scale",      '1', 0x1E, 12, 0, 2048);  // Chan8 B scale
    createConfigParam("Ch8:B:Offset",     '1', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFact",    '1', 0x20, 12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFact",    '1', 0x21, 12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFact",    '1', 0x22, 12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFact",    '1', 0x23, 12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFact",    '1', 0x24, 12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFact",    '1', 0x25, 12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFact",    '1', 0x26, 12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFact",    '1', 0x27, 12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1:Enable",       'E',  0x0,  1,  0, 1);    // Chan1 enable                  (0=disable,1=enable)
    createConfigParam("Ch2:Enable",       'E',  0x0,  1,  1, 1);    // Chan2 enable                  (0=disable,1=enable)
    createConfigParam("Ch3:Enable",       'E',  0x0,  1,  2, 1);    // Chan3 enable                  (0=disable,1=enable)
    createConfigParam("Ch4:Enable",       'E',  0x0,  1,  3, 1);    // Chan4 enable                  (0=disable,1=enable)
    createConfigParam("Ch5:Enable",       'E',  0x0,  1,  4, 1);    // Chan5 enable                  (0=disable,1=enable)
    createConfigParam("Ch6:Enable",       'E',  0x0,  1,  5, 1);    // Chan6 enable                  (0=disable,1=enable)
    createConfigParam("Ch7:Enable",       'E',  0x0,  1,  6, 1);    // Chan7 enable                  (0=disable,1=enable)
    createConfigParam("Ch8:Enable",       'E',  0x0,  1,  7, 1);    // Chan8 enable                  (0=disable,1=enable)

    createConfigParam("Ch1:VerboseEn",    'E',  0x0,  1,  8, 0);    // Chan1 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch2:VerboseEn",    'E',  0x0,  1,  9, 0);    // Chan2 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch3:VerboseEn",    'E',  0x0,  1, 10, 0);    // Chan3 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch4:VerboseEn",    'E',  0x0,  1, 11, 0);    // Chan4 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch5:VerboseEn",    'E',  0x0,  1, 12, 0);    // Chan5 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch6:VerboseEn",    'E',  0x0,  1, 13, 0);    // Chan6 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch7:VerboseEn",    'E',  0x0,  1, 14, 0);    // Chan7 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch8:VerboseEn",    'E',  0x0,  1, 15, 0);    // Chan8 verbose enable          (0=disable,1=enable)

    createConfigParam("TimeVetoLow",      'E',  0x1,  32, 0, 0);    // Timestamp veto low
    createConfigParam("TimeVetoHigh",     'E',  0x3,  32, 0, 0);    // Timestamp veto high
    createConfigParam("FakeTrigDelay",    'E',  0x5,  16, 0, 20000);// Fake trigger delay

    createConfigParam("LvdsRate",         'F',  0x0,  1, 15, 0);    // LVDS output rate              (0=20Mhz,1=10Mhz)
    createConfigParam("HighResMode",      'F',  0x0,  1, 9,  1);    // High resolution mode          (0=low res 0-127,1=high res 0-255)
    createConfigParam("TxEnable",         'F',  0x0,  1, 8,  1);    // TX enable                     (0=external,1=always enabled)
    createConfigParam("OutputMode",       'F',  0x0,  2, 6,  0);    // Output mode                   (0=normal,1=raw,2=extended)
    createConfigParam("AcquireEn",        'F',  0x0,  1, 5,  1);    // ROC enable acquisition        (0=disable,1=enable)
    createConfigParam("TsyncSelect",      'F',  0x0,  1, 4,  0);    // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TclkSelect",       'F',  0x0,  1, 3,  0);    // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("Reset",            'F',  0x0,  1, 2,  0);    // Reset enable                  (0=disable,1=enable)
    createConfigParam("AcquireMode",      'F',  0x0,  2, 0,  0);    // Acquire mode                  (0=normal,1=verbose,2=fakedata,3=trigger)

    // Now channel parameters - note the extra channel parameter
    createConfigParam("Ch1:PositionIdx",  1, '1',  0x0, 32, 0, 0);  // Chan1 position index
    createConfigParam("Ch1:A:InOffset",   1, '2',  0x0, 12, 0, 100);// Chan1 A input offset
    createConfigParam("Ch1:B:InOffset",   1, '2',  0x1, 12, 0, 100);// Chan1 B input offset
    createConfigParam("Ch1:A:FullScale",  1, '2',  0x2, 12, 0, 10); // Chan1 A full scale
    createConfigParam("Ch1:Unknown",      1, '2',  0x3, 12, 0, 0);  // Chan1 unknown register
    createConfigParam("Ch1:B:FullScale",  1, '2',  0x4, 12, 0, 10); // Chan1 B full scale
    createConfigParam("Ch1:PosThreshold", 1, '2',  0x5, 12, 0, 400);// Chan1 positive threshold
    createConfigParam("Ch1:NegThreshold", 1, '2',  0x6, 12, 0, 0);  // Chan1 negative threshold
    createConfigParam("Ch1:A:AdcOffset",  1, '2',  0x7, 12, 0, 100);// Chan1 A ADC offset
    createConfigParam("Ch1:SumZero",      1, '2',  0x8, 12, 0, 0);  // Chan1 sum zero
    createConfigParam("Ch1:B:AdcOffset",  1, '2',  0x9, 12, 0, 100);// Chan1 B ADC offset
    createConfigParam("Ch1:A:GainAdj",    1, '2',  0xA, 12, 0, 0);  // Chan1 A gain adjust
    createConfigParam("Ch1:B:GainAdj",    1, '2',  0xB, 12, 0, 0);  // Chan1 B gain adjust
    createConfigParam("Ch1:A:AvgMin",     1, 'D',  0x0, 12, 0, 100);// Chan1 A average minimum
    createConfigParam("Ch1:A:AvgMax",     1, 'D',  0x1, 12, 0, 1000); // Chan1 A average maximum
    createConfigParam("Ch1:B:AvgMin",     1, 'D',  0x2, 12, 0, 100);// Chan1 B average minimum
    createConfigParam("Ch1:B:AvgMax",     1, 'D',  0x3, 12, 0, 1000); // Chan1 B average maximum
    createConfigParam("Ch1:MaximumSlope", 1, 'D',  0x4, 12, 0, 0);  // Chan1 Maximum slope
    createConfigParam("Ch1:MinAdc",       1, 'E',  0x0, 14, 0, 100);// Chan1 Minimum ADC
    createConfigParam("Ch1:MaxAdc",       1, 'E',  0x1, 14, 0, 1000); // Chan1 Maximum ADC
    createConfigParam("Ch1:IntRelease",   1, 'E',  0x2,  9, 0, 506); // Chan1 Integrator release p
    createConfigParam("Ch1:MinPulseWidth",1, 'E',  0x3,  9, 0, 1);  // Chan1 min pulse width
    createConfigParam("Ch1:VetoInhibit",  1, 'E',  0x4, 12, 0, 0);  // Chan1 veto inhibit point
    createConfigParam("Ch1:NegDiscrTime", 1, 'E',  0x5, 12, 0, 0);  // Chan1 negative discr timeout
    createConfigParam("Ch1:Sample1",      1, 'E',  0x6,  9, 0, 2);  // Chan1 Sample 1 point [0:10]
    createConfigParam("Ch1:Sample2",      1, 'E',  0x7,  9, 0, 2);  // Chan1 Sample 2 point [0:10]

    createConfigParam("Ch2:PositionIdx",  2, '1',  0x0, 32, 0, 0);  // Chan2 position index
    createConfigParam("Ch2:A:InOffset",   2, '2',  0x0, 12, 0, 100);// Chan2 A input offset
    createConfigParam("Ch2:B:InOffset",   2, '2',  0x1, 12, 0, 100);// Chan2 B input offset
    createConfigParam("Ch2:A:FullScale",  2, '2',  0x2, 12, 0, 10); // Chan2 A full scale
    createConfigParam("Ch2:Unknown",      2, '2',  0x3, 12, 0, 0);  // Chan2 unknown register
    createConfigParam("Ch2:B:FullScale",  2, '2',  0x4, 12, 0, 10); // Chan2 B full scale
    createConfigParam("Ch2:PosThreshold", 2, '2',  0x5, 12, 0, 400);// Chan2 positive threshold
    createConfigParam("Ch2:NegThreshold", 2, '2',  0x6, 12, 0, 0);  // Chan2 negative threshold
    createConfigParam("Ch2:A:AdcOffset",  2, '2',  0x7, 12, 0, 100);// Chan2 A ADC offset
    createConfigParam("Ch2:SumZero",      2, '2',  0x8, 12, 0, 0);  // Chan2 sum zero
    createConfigParam("Ch2:B:AdcOffset",  2, '2',  0x9, 12, 0, 100);// Chan2 B ADC offset
    createConfigParam("Ch2:A:GainAdj",    2, '2',  0xA, 12, 0, 0);  // Chan2 A gain adjust
    createConfigParam("Ch2:B:GainAdj",    2, '2',  0xB, 12, 0, 0);  // Chan2 B gain adjust
    createConfigParam("Ch2:A:AvgMin",     2, 'D',  0x0, 12, 0, 100);// Chan2 A average minimum
    createConfigParam("Ch2:A:AvgMax",     2, 'D',  0x1, 12, 0, 1000); // Chan2 A average maximum
    createConfigParam("Ch2:B:AvgMin",     2, 'D',  0x2, 12, 0, 100);// Chan2 B average minimum
    createConfigParam("Ch2:B:AvgMax",     2, 'D',  0x3, 12, 0, 1000); // Chan2 B average maximum
    createConfigParam("Ch2:MaximumSlope", 2, 'D',  0x4, 12, 0, 0);  // Chan2 Maximum slope
    createConfigParam("Ch2:MinAdc",       2, 'E',  0x0, 14, 0, 100);// Chan2 Minimum ADC
    createConfigParam("Ch2:MaxAdc",       2, 'E',  0x1, 14, 0, 1000); // Chan2 Maximum ADC
    createConfigParam("Ch2:IntRelease",   2, 'E',  0x2,  9, 0, 506); // Chan2 Integrator release p
    createConfigParam("Ch2:MinPulseWidth",2, 'E',  0x3,  9, 0, 1);  // Chan2 min pulse width
    createConfigParam("Ch2:VetoInhibit",  2, 'E',  0x4, 12, 0, 0);  // Chan2 veto inhibit point
    createConfigParam("Ch2:NegDiscrTime", 2, 'E',  0x5, 12, 0, 0);  // Chan2 negative discr timeout
    createConfigParam("Ch2:Sample1",      2, 'E',  0x6,  9, 0, 2);  // Chan2 Sample 1 point [0:10]
    createConfigParam("Ch2:Sample2",      2, 'E',  0x7,  9, 0, 2);  // Chan2 Sample 2 point [0:10]

    createConfigParam("Ch3:PositionIdx",  3, '1',  0x0, 32, 0, 0);  // Chan3 position index
    createConfigParam("Ch3:A:InOffset",   3, '2',  0x0, 12, 0, 100);// Chan3 A input offset
    createConfigParam("Ch3:B:InOffset",   3, '2',  0x1, 12, 0, 100);// Chan3 B input offset
    createConfigParam("Ch3:A:FullScale",  3, '2',  0x2, 12, 0, 10); // Chan3 A full scale
    createConfigParam("Ch3:Unknown",      3, '2',  0x3, 12, 0, 0);  // Chan3 unknown register
    createConfigParam("Ch3:B:FullScale",  3, '2',  0x4, 12, 0, 10); // Chan3 B full scale
    createConfigParam("Ch3:PosThreshold", 3, '2',  0x5, 12, 0, 400);// Chan3 positive threshold
    createConfigParam("Ch3:NegThreshold", 3, '2',  0x6, 12, 0, 0);  // Chan3 negative threshold
    createConfigParam("Ch3:A:AdcOffset",  3, '2',  0x7, 12, 0, 100);// Chan3 A ADC offset
    createConfigParam("Ch3:SumZero",      3, '2',  0x8, 12, 0, 0);  // Chan3 sum zero
    createConfigParam("Ch3:B:AdcOffset",  3, '2',  0x9, 12, 0, 100);// Chan3 B ADC offset
    createConfigParam("Ch3:A:GainAdj",    3, '2',  0xA, 12, 0, 0);  // Chan3 A gain adjust
    createConfigParam("Ch3:B:GainAdj",    3, '2',  0xB, 12, 0, 0);  // Chan3 B gain adjust
    createConfigParam("Ch3:A:AvgMin",     3, 'D',  0x0, 12, 0, 100);// Chan3 A average minimum
    createConfigParam("Ch3:A:AvgMax",     3, 'D',  0x1, 12, 0, 1000); // Chan3 A average maximum
    createConfigParam("Ch3:B:AvgMin",     3, 'D',  0x2, 12, 0, 100);// Chan3 B average minimum
    createConfigParam("Ch3:B:AvgMax",     3, 'D',  0x3, 12, 0, 1000); // Chan3 B average maximum
    createConfigParam("Ch3:MaximumSlope", 3, 'D',  0x4, 12, 0, 0);  // Chan3 Maximum slope
    createConfigParam("Ch3:MinAdc",       3, 'E',  0x0, 14, 0, 100);// Chan3 Minimum ADC
    createConfigParam("Ch3:MaxAdc",       3, 'E',  0x1, 14, 0, 1000); // Chan3 Maximum ADC
    createConfigParam("Ch3:IntRelease",   3, 'E',  0x2,  9, 0, 506); // Chan3 Integrator release p
    createConfigParam("Ch3:MinPulseWidth",3, 'E',  0x3,  9, 0, 1);  // Chan3 min pulse width
    createConfigParam("Ch3:VetoInhibit",  3, 'E',  0x4, 12, 0, 0);  // Chan3 veto inhibit point
    createConfigParam("Ch3:NegDiscrTime", 3, 'E',  0x5, 12, 0, 0);  // Chan3 negative discr timeout
    createConfigParam("Ch3:Sample1",      3, 'E',  0x6,  9, 0, 2);  // Chan3 Sample 1 point [0:10]
    createConfigParam("Ch3:Sample2",      3, 'E',  0x7,  9, 0, 2);  // Chan3 Sample 2 point [0:10]

    createConfigParam("Ch4:PositionIdx",  4, '1',  0x0, 32, 0, 0);  // Chan4 position index
    createConfigParam("Ch4:A:InOffset",   4, '2',  0x0, 12, 0, 100);// Chan4 A input offset
    createConfigParam("Ch4:B:InOffset",   4, '2',  0x1, 12, 0, 100);// Chan4 B input offset
    createConfigParam("Ch4:A:FullScale",  4, '2',  0x2, 12, 0, 10); // Chan4 A full scale
    createConfigParam("Ch4:Unknown",      4, '2',  0x3, 12, 0, 0);  // Chan4 unknown register
    createConfigParam("Ch4:B:FullScale",  4, '2',  0x4, 12, 0, 10); // Chan4 B full scale
    createConfigParam("Ch4:PosThreshold", 4, '2',  0x5, 12, 0, 400);// Chan4 positive threshold
    createConfigParam("Ch4:NegThreshold", 4, '2',  0x6, 12, 0, 0);  // Chan4 negative threshold
    createConfigParam("Ch4:A:AdcOffset",  4, '2',  0x7, 12, 0, 100);// Chan4 A ADC offset
    createConfigParam("Ch4:SumZero",      4, '2',  0x8, 12, 0, 0);  // Chan4 sum zero
    createConfigParam("Ch4:B:AdcOffset",  4, '2',  0x9, 12, 0, 100);// Chan4 B ADC offset
    createConfigParam("Ch4:A:GainAdj",    4, '2',  0xA, 12, 0, 0);  // Chan4 A gain adjust
    createConfigParam("Ch4:B:GainAdj",    4, '2',  0xB, 12, 0, 0);  // Chan4 B gain adjust
    createConfigParam("Ch4:A:AvgMin",     4, 'D',  0x0, 12, 0, 100);// Chan4 A average minimum
    createConfigParam("Ch4:A:AvgMax",     4, 'D',  0x1, 12, 0, 1000); // Chan4 A average maximum
    createConfigParam("Ch4:B:AvgMin",     4, 'D',  0x2, 12, 0, 100);// Chan4 B average minimum
    createConfigParam("Ch4:B:AvgMax",     4, 'D',  0x3, 12, 0, 1000); // Chan4 B average maximum
    createConfigParam("Ch4:MaximumSlope", 4, 'D',  0x4, 12, 0, 0);  // Chan4 Maximum slope
    createConfigParam("Ch4:MinAdc",       4, 'E',  0x0, 14, 0, 100);// Chan4 Minimum ADC
    createConfigParam("Ch4:MaxAdc",       4, 'E',  0x1, 14, 0, 1000); // Chan4 Maximum ADC
    createConfigParam("Ch4:IntRelease",   4, 'E',  0x2,  9, 0, 506); // Chan4 Integrator release p
    createConfigParam("Ch4:MinPulseWidth",4, 'E',  0x3,  9, 0, 1);  // Chan4 min pulse width
    createConfigParam("Ch4:VetoInhibit",  4, 'E',  0x4, 12, 0, 0);  // Chan4 veto inhibit point
    createConfigParam("Ch4:NegDiscrTime", 4, 'E',  0x5, 12, 0, 0);  // Chan4 negative discr timeout
    createConfigParam("Ch4:Sample1",      4, 'E',  0x6,  9, 0, 2);  // Chan4 Sample 1 point [0:10]
    createConfigParam("Ch4:Sample2",      4, 'E',  0x7,  9, 0, 2);  // Chan4 Sample 2 point [0:10]

    createConfigParam("Ch5:PositionIdx",  5, '1',  0x0, 32, 0, 0);  // Chan5 position index
    createConfigParam("Ch5:A:InOffset",   5, '2',  0x0, 12, 0, 100);// Chan5 A input offset
    createConfigParam("Ch5:B:InOffset",   5, '2',  0x1, 12, 0, 100);// Chan5 B input offset
    createConfigParam("Ch5:A:FullScale",  5, '2',  0x2, 12, 0, 10); // Chan5 A full scale
    createConfigParam("Ch5:Unknown",      5, '2',  0x3, 12, 0, 0);  // Chan5 unknown register
    createConfigParam("Ch5:B:FullScale",  5, '2',  0x4, 12, 0, 10); // Chan5 B full scale
    createConfigParam("Ch5:PosThreshold", 5, '2',  0x5, 12, 0, 400);// Chan5 positive threshold
    createConfigParam("Ch5:NegThreshold", 5, '2',  0x6, 12, 0, 0);  // Chan5 negative threshold
    createConfigParam("Ch5:A:AdcOffset",  5, '2',  0x7, 12, 0, 100);// Chan5 A ADC offset
    createConfigParam("Ch5:SumZero",      5, '2',  0x8, 12, 0, 0);  // Chan5 sum zero
    createConfigParam("Ch5:B:AdcOffset",  5, '2',  0x9, 12, 0, 100);// Chan5 B ADC offset
    createConfigParam("Ch5:A:GainAdj",    5, '2',  0xA, 12, 0, 0);  // Chan5 A gain adjust
    createConfigParam("Ch5:B:GainAdj",    5, '2',  0xB, 12, 0, 0);  // Chan5 B gain adjust
    createConfigParam("Ch5:A:AvgMin",     5, 'D',  0x0, 12, 0, 100);// Chan5 A average minimum
    createConfigParam("Ch5:A:AvgMax",     5, 'D',  0x1, 12, 0, 1000); // Chan5 A average maximum
    createConfigParam("Ch5:B:AvgMin",     5, 'D',  0x2, 12, 0, 100);// Chan5 B average minimum
    createConfigParam("Ch5:B:AvgMax",     5, 'D',  0x3, 12, 0, 1000); // Chan5 B average maximum
    createConfigParam("Ch5:MaximumSlope", 5, 'D',  0x4, 12, 0, 0);  // Chan5 Maximum slope
    createConfigParam("Ch5:MinAdc",       5, 'E',  0x0, 14, 0, 100);// Chan5 Minimum ADC
    createConfigParam("Ch5:MaxAdc",       5, 'E',  0x1, 14, 0, 1000); // Chan5 Maximum ADC
    createConfigParam("Ch5:IntRelease",   5, 'E',  0x2,  9, 0, 506); // Chan5 Integrator release p
    createConfigParam("Ch5:MinPulseWidth",5, 'E',  0x3,  9, 0, 1);  // Chan5 min pulse width
    createConfigParam("Ch5:VetoInhibit",  5, 'E',  0x4, 12, 0, 0);  // Chan5 veto inhibit point
    createConfigParam("Ch5:NegDiscrTime", 5, 'E',  0x5, 12, 0, 0);  // Chan5 negative discr timeout
    createConfigParam("Ch5:Sample1",      5, 'E',  0x6,  9, 0, 2);  // Chan5 Sample 1 point [0:10]
    createConfigParam("Ch5:Sample2",      5, 'E',  0x7,  9, 0, 2);  // Chan5 Sample 2 point [0:10]

    createConfigParam("Ch6:PositionIdx",  6, '1',  0x0, 32, 0, 0);  // Chan6 position index
    createConfigParam("Ch6:A:InOffset",   6, '2',  0x0, 12, 0, 100);// Chan6 A input offset
    createConfigParam("Ch6:B:InOffset",   6, '2',  0x1, 12, 0, 100);// Chan6 B input offset
    createConfigParam("Ch6:A:FullScale",  6, '2',  0x2, 12, 0, 10); // Chan6 A full scale
    createConfigParam("Ch6:Unknown",      6, '2',  0x3, 12, 0, 0);  // Chan6 unknown register
    createConfigParam("Ch6:B:FullScale",  6, '2',  0x4, 12, 0, 10); // Chan6 B full scale
    createConfigParam("Ch6:PosThreshold", 6, '2',  0x5, 12, 0, 400);// Chan6 positive threshold
    createConfigParam("Ch6:NegThreshold", 6, '2',  0x6, 12, 0, 0);  // Chan6 negative threshold
    createConfigParam("Ch6:A:AdcOffset",  6, '2',  0x7, 12, 0, 100);// Chan6 A ADC offset
    createConfigParam("Ch6:SumZero",      6, '2',  0x8, 12, 0, 0);  // Chan6 sum zero
    createConfigParam("Ch6:B:AdcOffset",  6, '2',  0x9, 12, 0, 100);// Chan6 B ADC offset
    createConfigParam("Ch6:A:GainAdj",    6, '2',  0xA, 12, 0, 0);  // Chan6 A gain adjust
    createConfigParam("Ch6:B:GainAdj",    6, '2',  0xB, 12, 0, 0);  // Chan6 B gain adjust
    createConfigParam("Ch6:A:AvgMin",     6, 'D',  0x0, 12, 0, 100);// Chan6 A average minimum
    createConfigParam("Ch6:A:AvgMax",     6, 'D',  0x1, 12, 0, 1000); // Chan6 A average maximum
    createConfigParam("Ch6:B:AvgMin",     6, 'D',  0x2, 12, 0, 100);// Chan6 B average minimum
    createConfigParam("Ch6:B:AvgMax",     6, 'D',  0x3, 12, 0, 1000); // Chan6 B average maximum
    createConfigParam("Ch6:MaximumSlope", 6, 'D',  0x4, 12, 0, 0);  // Chan6 Maximum slope
    createConfigParam("Ch6:MinAdc",       6, 'E',  0x0, 14, 0, 100);// Chan6 Minimum ADC
    createConfigParam("Ch6:MaxAdc",       6, 'E',  0x1, 14, 0, 1000); // Chan6 Maximum ADC
    createConfigParam("Ch6:IntRelease",   6, 'E',  0x2,  9, 0, 506); // Chan6 Integrator release p
    createConfigParam("Ch6:MinPulseWidth",6, 'E',  0x3,  9, 0, 1);  // Chan6 min pulse width
    createConfigParam("Ch6:VetoInhibit",  6, 'E',  0x4, 12, 0, 0);  // Chan6 veto inhibit point
    createConfigParam("Ch6:NegDiscrTime", 6, 'E',  0x5, 12, 0, 0);  // Chan6 negative discr timeout
    createConfigParam("Ch6:Sample1",      6, 'E',  0x6,  9, 0, 2);  // Chan6 Sample 1 point [0:10]
    createConfigParam("Ch6:Sample2",      6, 'E',  0x7,  9, 0, 2);  // Chan6 Sample 2 point [0:10]

    createConfigParam("Ch7:PositionIdx",  7, '1',  0x0, 32, 0, 0);  // Chan7 position index
    createConfigParam("Ch7:A:InOffset",   7, '2',  0x0, 12, 0, 100);// Chan7 A input offset
    createConfigParam("Ch7:B:InOffset",   7, '2',  0x1, 12, 0, 100);// Chan7 B input offset
    createConfigParam("Ch7:A:FullScale",  7, '2',  0x2, 12, 0, 10); // Chan7 A full scale
    createConfigParam("Ch7:Unknown",      7, '2',  0x3, 12, 0, 0);  // Chan7 unknown register
    createConfigParam("Ch7:B:FullScale",  7, '2',  0x4, 12, 0, 10); // Chan7 B full scale
    createConfigParam("Ch7:PosThreshold", 7, '2',  0x5, 12, 0, 400);// Chan7 positive threshold
    createConfigParam("Ch7:NegThreshold", 7, '2',  0x6, 12, 0, 0);  // Chan7 negative threshold
    createConfigParam("Ch7:A:AdcOffset",  7, '2',  0x7, 12, 0, 100);// Chan7 A ADC offset
    createConfigParam("Ch7:SumZero",      7, '2',  0x8, 12, 0, 0);  // Chan7 sum zero
    createConfigParam("Ch7:B:AdcOffset",  7, '2',  0x9, 12, 0, 100);// Chan7 B ADC offset
    createConfigParam("Ch7:A:GainAdj",    7, '2',  0xA, 12, 0, 0);  // Chan7 A gain adjust
    createConfigParam("Ch7:B:GainAdj",    7, '2',  0xB, 12, 0, 0);  // Chan7 B gain adjust
    createConfigParam("Ch7:A:AvgMin",     7, 'D',  0x0, 12, 0, 100);// Chan7 A average minimum
    createConfigParam("Ch7:A:AvgMax",     7, 'D',  0x1, 12, 0, 1000); // Chan7 A average maximum
    createConfigParam("Ch7:B:AvgMin",     7, 'D',  0x2, 12, 0, 100);// Chan7 B average minimum
    createConfigParam("Ch7:B:AvgMax",     7, 'D',  0x3, 12, 0, 1000); // Chan7 B average maximum
    createConfigParam("Ch7:MaximumSlope", 7, 'D',  0x4, 12, 0, 0);  // Chan7 Maximum slope
    createConfigParam("Ch7:MinAdc",       7, 'E',  0x0, 14, 0, 100);// Chan7 Minimum ADC
    createConfigParam("Ch7:MaxAdc",       7, 'E',  0x1, 14, 0, 1000); // Chan7 Maximum ADC
    createConfigParam("Ch7:IntRelease",   7, 'E',  0x2,  9, 0, 506); // Chan7 Integrator release p
    createConfigParam("Ch7:MinPulseWidth",7, 'E',  0x3,  9, 0, 1);  // Chan7 min pulse width
    createConfigParam("Ch7:VetoInhibit",  7, 'E',  0x4, 12, 0, 0);  // Chan7 veto inhibit point
    createConfigParam("Ch7:NegDiscrTime", 7, 'E',  0x5, 12, 0, 0);  // Chan7 negative discr timeout
    createConfigParam("Ch7:Sample1",      7, 'E',  0x6,  9, 0, 2);  // Chan7 Sample 1 point [0:10]
    createConfigParam("Ch7:Sample2",      7, 'E',  0x7,  9, 0, 2);  // Chan7 Sample 2 point [0:10]

    createConfigParam("Ch8:PositionIdx",  8, '1',  0x0, 32, 0, 0);  // Chan8 position index
    createConfigParam("Ch8:A:InOffset",   8, '2',  0x0, 12, 0, 100);// Chan8 A input offset
    createConfigParam("Ch8:B:InOffset",   8, '2',  0x1, 12, 0, 100);// Chan8 B input offset
    createConfigParam("Ch8:A:FullScale",  8, '2',  0x2, 12, 0, 10); // Chan8 A full scale
    createConfigParam("Ch8:Unknown",      8, '2',  0x3, 12, 0, 0);  // Chan8 unknown register
    createConfigParam("Ch8:B:FullScale",  8, '2',  0x4, 12, 0, 10); // Chan8 B full scale
    createConfigParam("Ch8:PosThreshold", 8, '2',  0x5, 12, 0, 400);// Chan8 positive threshold
    createConfigParam("Ch8:NegThreshold", 8, '2',  0x6, 12, 0, 0);  // Chan8 negative threshold
    createConfigParam("Ch8:A:AdcOffset",  8, '2',  0x7, 12, 0, 100);// Chan8 A ADC offset
    createConfigParam("Ch8:SumZero",      8, '2',  0x8, 12, 0, 0);  // Chan8 sum zero
    createConfigParam("Ch8:B:AdcOffset",  8, '2',  0x9, 12, 0, 100);// Chan8 B ADC offset
    createConfigParam("Ch8:A:GainAdj",    8, '2',  0xA, 12, 0, 0);  // Chan8 A gain adjust
    createConfigParam("Ch8:B:GainAdj",    8, '2',  0xB, 12, 0, 0);  // Chan8 B gain adjust
    createConfigParam("Ch8:A:AvgMin",     8, 'D',  0x0, 12, 0, 100);// Chan8 A average minimum
    createConfigParam("Ch8:A:AvgMax",     8, 'D',  0x1, 12, 0, 1000); // Chan8 A average maximum
    createConfigParam("Ch8:B:AvgMin",     8, 'D',  0x2, 12, 0, 100);// Chan8 B average minimum
    createConfigParam("Ch8:B:AvgMax",     8, 'D',  0x3, 12, 0, 1000); // Chan8 B average maximum
    createConfigParam("Ch8:MaximumSlope", 8, 'D',  0x4, 12, 0, 0);  // Chan8 Maximum slope
    createConfigParam("Ch8:MinAdc",       8, 'E',  0x0, 14, 0, 100);// Chan8 Minimum ADC
    createConfigParam("Ch8:MaxAdc",       8, 'E',  0x1, 14, 0, 1000); // Chan8 Maximum ADC
    createConfigParam("Ch8:IntRelease",   8, 'E',  0x2,  9, 0, 506); // Chan8 Integrator release p
    createConfigParam("Ch8:MinPulseWidth",8, 'E',  0x3,  9, 0, 1);  // Chan8 min pulse width
    createConfigParam("Ch8:VetoInhibit",  8, 'E',  0x4, 12, 0, 0);  // Chan8 veto inhibit point
    createConfigParam("Ch8:NegDiscrTime", 8, 'E',  0x5, 12, 0, 0);  // Chan8 negative discr timeout
    createConfigParam("Ch8:Sample1",      8, 'E',  0x6,  9, 0, 2);  // Chan8 Sample 1 point [0:10]
    createConfigParam("Ch8:Sample2",      8, 'E',  0x7,  9, 0, 2);  // Chan8 Sample 2 point [0:10]
}
