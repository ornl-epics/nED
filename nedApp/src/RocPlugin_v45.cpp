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
    createConfigParam("Ch1:A:Scale",      '1', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch2:A:Scale",      '1', 0x1,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch3:A:Scale",      '1', 0x2,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch4:A:Scale",      '1', 0x3,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch5:A:Scale",      '1', 0x4,  12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch6:A:Scale",      '1', 0x5,  12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch7:A:Scale",      '1', 0x6,  12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch8:A:Scale",      '1', 0x7,  12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch1:B:Scale",      '1', 0x8,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch2:B:Scale",      '1', 0x9,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch3:B:Scale",      '1', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch4:B:Scale",      '1', 0xB,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch5:B:Scale",      '1', 0xC,  12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch6:B:Scale",      '1', 0xD,  12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch7:B:Scale",      '1', 0xE,  12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch8:B:Scale",      '1', 0xF,  12, 0, 2048);  // Chan8 B scale

    createConfigParam("Ch1:A:Offset",     '1', 0x10, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch2:A:Offset",     '1', 0x11, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch3:A:Offset",     '1', 0x12, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch4:A:Offset",     '1', 0x13, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch5:A:Offset",     '1', 0x14, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch6:A:Offset",     '1', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch7:A:Offset",     '1', 0x16, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch8:A:Offset",     '1', 0x17, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch1:B:Offset",     '1', 0x18, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:B:Offset",     '1', 0x19, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:B:Offset",     '1', 0x1A, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:B:Offset",     '1', 0x1B, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5:B:Offset",     '1', 0x1C, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:B:Offset",     '1', 0x1D, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:B:Offset",     '1', 0x1E, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:B:Offset",     '1', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFact",    '1', 0x20, 12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFact",    '1', 0x21, 12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFact",    '1', 0x22, 12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFact",    '1', 0x23, 12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFact",    '1', 0x24, 12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFact",    '1', 0x25, 12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFact",    '1', 0x26, 12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFact",    '1', 0x27, 12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1:Enable",       'E', 0x0,  1,  0, 1);     // Chan1 enable                  (0=disable,1=enable)
    createConfigParam("Ch2:Enable",       'E', 0x0,  1,  1, 1);     // Chan2 enable                  (0=disable,1=enable)
    createConfigParam("Ch3:Enable",       'E', 0x0,  1,  2, 1);     // Chan3 enable                  (0=disable,1=enable)
    createConfigParam("Ch4:Enable",       'E', 0x0,  1,  3, 1);     // Chan4 enable                  (0=disable,1=enable)
    createConfigParam("Ch5:Enable",       'E', 0x0,  1,  4, 1);     // Chan5 enable                  (0=disable,1=enable)
    createConfigParam("Ch6:Enable",       'E', 0x0,  1,  5, 1);     // Chan6 enable                  (0=disable,1=enable)
    createConfigParam("Ch7:Enable",       'E', 0x0,  1,  6, 1);     // Chan7 enable                  (0=disable,1=enable)
    createConfigParam("Ch8:Enable",       'E', 0x0,  1,  7, 1);     // Chan8 enable                  (0=disable,1=enable)

    createConfigParam("Ch1:VerboseEn",    'E', 0x0,  1,  8, 0);     // Chan1 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch2:VerboseEn",    'E', 0x0,  1,  9, 0);     // Chan2 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch3:VerboseEn",    'E', 0x0,  1, 10, 0);     // Chan3 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch4:VerboseEn",    'E', 0x0,  1, 11, 0);     // Chan4 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch5:VerboseEn",    'E', 0x0,  1, 12, 0);     // Chan5 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch6:VerboseEn",    'E', 0x0,  1, 13, 0);     // Chan6 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch7:VerboseEn",    'E', 0x0,  1, 14, 0);     // Chan7 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch8:VerboseEn",    'E', 0x0,  1, 15, 0);     // Chan8 verbose enable          (0=disable,1=enable)

    createConfigParam("TimeVetoLow",      'E', 0x1,  32, 0, 0);     // Timestamp veto low
    createConfigParam("TimeVetoHigh",     'E', 0x3,  32, 0, 0);     // Timestamp veto high
    createConfigParam("FakeTrigDelay",    'E', 0x5,  16, 0, 20000); // Fake trigger delay

    createConfigParam("LvdsRate",         'F', 0x0,  1, 15, 0);     // LVDS output rate              (0=20Mhz,1=10Mhz)
    createConfigParam("HighResMode",      'F', 0x0,  1, 9,  1);     // High resolution mode          (0=low res 0-127,1=high res 0-255)
    createConfigParam("TxEnable",         'F', 0x0,  1, 8,  1);     // TX enable                     (0=external,1=always enabled)
    createConfigParam("OutputMode",       'F', 0x0,  2, 6,  0);     // Output mode                   (0=normal,1=raw,2=extended)
    createConfigParam("AcquireEn",        'F', 0x0,  1, 5,  1);     // ROC enable acquisition        (0=disable,1=enable)
    createConfigParam("TsyncSelect",      'F', 0x0,  1, 4,  0);     // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TclkSelect",       'F', 0x0,  1, 3,  0);     // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("Reset",            'F', 0x0,  1, 2,  0);     // Reset enable                  (0=disable,1=enable)
    createConfigParam("AcquireMode",      'F', 0x0,  2, 0,  0);     // Acquire mode                  (0=normal,1=verbose,2=fakedata,3=trigger)
}
