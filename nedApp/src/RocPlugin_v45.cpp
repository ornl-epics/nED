/* RocPlugin_v45.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Bogdan Vacaliuc
 */

#include "RocPlugin.h"

void RocPlugin::createParams_v45()
{
    createRegParam("VERSION", "HwRev",  true, 0,  8, 0);   // Hardware revision
    createRegParam("VERSION", "HwVer",  true, 0,  8, 8);   // Hardware version
    createRegParam("VERSION", "FwRev",  true, 1,  8, 0);   // Firmware revision        (low:4,high:6)
    createRegParam("VERSION", "FwVer",  true, 1,  8, 8);   // Firmware version         (low:3,high:5)
    createRegParam("VERSION", "FwYear", true, 2, 16, 0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwDay",  true, 3,  8, 0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwMonth",true, 3,  8, 8, 0, CONV_HEX2DEC);

//    BLXXX:Det:RocXXX:| sig nam|                              | EPICS record description | (bi and mbbi description)
    createStatusParam("ErrParity",            0x0,  1,  0); // LVDS parity error.           (0=no error,1=error)
    createStatusParam("ErrType",              0x0,  1,  1); // LVDS packet type error.      (0=no error,1=error)
    createStatusParam("ErrLength",            0x0,  1,  2); // LVDS packet length error.    (0=no error,1=error)
    createStatusParam("ErrTimeout",           0x0,  1,  3); // LVDS packet timeout.         (0=no timeout,1=timeout)
    createStatusParam("ErrNoStart",           0x0,  1,  4); // LVDS data without start.     (0=no error,1=error)
    createStatusParam("ErrPreStart",          0x0,  1,  5); // LVDS start before stop bit   (0=no error,1=error)
    createStatusParam("ErrFifoFull",          0x0,  1,  6); // LVDS FIFO went full.         (0=not full,1=full)
    createStatusParam("CalcBadCount",         0x0,  1,  7); // Calc: Bad word count.        (0=no error,1=error)
    createStatusParam("CalcBadOverlow",       0x0,  1,  8); // Calc: Data overflow detected (0=no error,1=error)
    createStatusParam("CalcBadEffect",        0x0,  1,  9); // Calc: Bad Effective Calculat (0=no error,1=error)
    createStatusParam("CalcBadFinal",         0x0,  1, 10); // Calc: Bad Final Calculation  (0=no error,1=error)
    createStatusParam("DataFifoEmpty",        0x0,  1, 11); // Data FIFO Empty flag.        (1=not empty,0=empty)
    createStatusParam("DataFifoAlmostFull",   0x0,  1, 12); // Data FIFO Almost full flag.  (0=not full,1=almost full)
    createStatusParam("ErrCmdBad",            0x0,  1, 13); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrCmdLength",         0x0,  1, 14); // Command length error         (0=no error,1=error)
    createStatusParam("ErrProgramming",       0x0,  1, 15); // WRITE_CNFG during ACQ        (0=no error,1=error)

    createStatusParam("ErrFatal",             0x1,  1,  0); // FATAL ERROR detected         (0=not detected,1=detected)
    createStatusParam("ErrNoTsync",           0x1,  1,  1); // Timestamp overflow error.    (0=no error,1=error)
    createStatusParam("LvdsReset",            0x1,  1,  8); // LVDS RESET detected          (0=not detected,1=detected)
    createStatusParam("LvdsPowerdown",        0x1,  1,  9); // LVDS POWERDOWN detected      (0=not detected,1=detected)
    createStatusParam("LvdsVerify",           0x1,  1, 10); // LVDS VERIFY detected         (0=not detected,1=detected)

    createStatusParam("DataFpgaReady",        0x2,  8,  0); // Channel FPGA Ready           (1=not ready,0=ready)
    createStatusParam("DataReady",            0x2,  8,  8); // Channel DATA Ready           (0=not ready,1=ready)

    createStatusParam("Discovered",           0x3,  1,  0); // Discovered                   (0=not discovered,1=discovered, archive:monitor)
    createStatusParam("Acquiring",            0x3,  1,  1); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("CalcActive",           0x3,  1,  2); // Calculation: Active          (0=not active,1=active)
    createStatusParam("TclkLow",              0x3,  1,  3); // TCLK Got LOW                 (0=no,1=yes)
    createStatusParam("TclkHigh",             0x3,  1,  4); // TCLK Got HIGH                (0=no,1=yes)
    createStatusParam("TsyncLow",             0x3,  1,  5); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TsyncHigh",            0x3,  1,  6); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("SysrstBLow",           0x3,  1,  7); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("SysrstBHigh",          0x3,  1,  8); // SYSRST_B Got HIGH            (0=no,1=yes)
    createStatusParam("TxenBLow",             0x3,  1,  9); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("TxenBHigh",            0x3,  1, 10); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("CalcInputEmpty",       0x3,  1, 11); // Calc: Input Empty flag       (1=not empty,0=empty)
    createStatusParam("CalcInputAlmostFull",  0x3,  1, 12); // Calc: INPUT Almost full      (0=not full,1=almost full)
    createStatusParam("CalcFifoEmpty",        0x3,  1, 13); // Calc: FIFO Empty flag        (1=not empty,0=empty)
    createStatusParam("CalcFifoAlmostFull",   0x3,  1, 14); // Calc: FIFO Almost full       (0=not full,1=almost full)
    createStatusParam("HVStatus",             0x3,  1, 15); // High Voltage Status bit      (0=not present,1=present)

    createChanStatusParam("Ch1:A:InputOffset",    1, 0x0,  9,  0); // Chan1 A Input Offset value
    createChanStatusParam("Ch1:A:AutoAdjOverflow",1, 0x0,  1,  9); // Chan1 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch1:A:AutoAdjSlope1",  1, 0x0,  1, 10); // Chan1 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch1:A:AutoAdjOffset",  1, 0x0,  1, 11); // Chan1 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch1:A:AutoAdjSlope",   1, 0x0,  1, 12); // Chan1 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch1:A:AutoAdjSample",  1, 0x0,  1, 13); // Chan1 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch1:A:AutoAdjTrig",    1, 0x0,  1, 14); // Chan1 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch1:A:AutoAdjEn",      1, 0x0,  1, 15); // Chan1 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch1:A:AdcOffsetVal",   1, 0x1,  8,  0); // Chan1 A ADC Offset value
    createChanStatusParam("Ch1:B:AdcOffsetVal",   1, 0x1,  8,  8); // Chan1 B ADC Offset value
    createChanStatusParam("Ch1:B:InputOffset",    1, 0x2,  9,  0); // Chan1 B Input Offset value
    createChanStatusParam("Ch1:B:AutoAdjOverflow",1, 0x2,  1,  9); // Chan1 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch1:B:AutoAdjSlope1",  1, 0x2,  1, 10); // Chan1 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch1:B:AutoAdjOffset",  1, 0x2,  1, 11); // Chan1 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch1:B:AutoAdjSlope",   1, 0x2,  1, 12); // Chan1 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch1:B:AutoAdjSample",  1, 0x2,  1, 13); // Chan1 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch1:B:AutoAdjTrig",    1, 0x2,  1, 14); // Chan1 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch1:B:AutoAdjEn",      1, 0x2,  1, 15); // Chan1 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch1:Configured",       1, 0x3,  1,  0); // Chan1 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch1:Enabled",          1, 0x3,  1,  1); // Chan1 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch1:Acquiring",        1, 0x3,  1,  2); // Chan1 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch1:FpgaHasData",      1, 0x3,  1,  3); // Chan1 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch1:AdcFifoHasData",   1, 0x3,  1,  4); // Chan1 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch1:AdcFifoAlmostFull",1, 0x3,  1,  5); // Chan1 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch1:PosDiscEvent",     1, 0x3,  1,  6); // Chan1 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch1:AdcFifoFull",      1, 0x3,  1,  7); // Chan1 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch1:EventFifoFull",    1, 0x3,  1,  8); // Chan1 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch1:AdcMin",           1, 0x3,  1,  9); // Chan1 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch1:AdcMax",           1, 0x3,  1, 10); // Chan1 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch1:ErrProgramming",   1, 0x3,  1, 11); // Chan1 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch1:ErrCmdLength",     1, 0x3,  1, 12); // Chan1 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch1:ErrCmdBad",        1, 0x3,  1, 13); // Chan1 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch1:Event",            1, 0x3,  1, 14); // Chan1 got event              (0=no,1=yes)
    createChanStatusParam("Ch1:MultiDiscEvent",   1, 0x3,  1, 15); // Chan1 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch2:A:InputOffset",    2, 0x0,  9,  0); // Chan2 A Input Offset value
    createChanStatusParam("Ch2:A:AutoAdjOverflow",2, 0x0,  1,  9); // Chan2 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch2:A:AutoAdjSlope1",  2, 0x0,  1, 10); // Chan2 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch2:A:AutoAdjOffset",  2, 0x0,  1, 11); // Chan2 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch2:A:AutoAdjSlope",   2, 0x0,  1, 12); // Chan2 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch2:A:AutoAdjSample",  2, 0x0,  1, 13); // Chan2 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch2:A:AutoAdjTrig",    2, 0x0,  1, 14); // Chan2 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch2:A:AutoAdjEn",      2, 0x0,  1, 15); // Chan2 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch2:A:AdcOffsetVal",   2, 0x1,  8,  0); // Chan2 A ADC Offset value
    createChanStatusParam("Ch2:B:AdcOffsetVal",   2, 0x1,  8,  8); // Chan2 B ADC Offset value
    createChanStatusParam("Ch2:B:InputOffset",    2, 0x2,  9,  0); // Chan2 B Input Offset value
    createChanStatusParam("Ch2:B:AutoAdjOverflow",2, 0x2,  1,  9); // Chan2 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch2:B:AutoAdjSlope1",  2, 0x2,  1, 10); // Chan2 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch2:B:AutoAdjOffset",  2, 0x2,  1, 11); // Chan2 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch2:B:AutoAdjSlope",   2, 0x2,  1, 12); // Chan2 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch2:B:AutoAdjSample",  2, 0x2,  1, 13); // Chan2 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch2:B:AutoAdjTrig",    2, 0x2,  1, 14); // Chan2 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch2:B:AutoAdjEn",      2, 0x2,  1, 15); // Chan2 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch2:Configured",       2, 0x3,  1,  0); // Chan2 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch2:Enabled",          2, 0x3,  1,  1); // Chan2 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch2:Acquiring",        2, 0x3,  1,  2); // Chan2 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch2:FpgaHasData",      2, 0x3,  1,  3); // Chan2 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch2:AdcFifoHasData",   2, 0x3,  1,  4); // Chan2 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch2:AdcFifoAlmostFull",2, 0x3,  1,  5); // Chan2 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch2:PosDiscEvent",     2, 0x3,  1,  6); // Chan2 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch2:AdcFifoFull",      2, 0x3,  1,  7); // Chan2 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch2:EventFifoFull",    2, 0x3,  1,  8); // Chan2 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch2:AdcMin",           2, 0x3,  1,  9); // Chan2 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch2:AdcMax",           2, 0x3,  1, 10); // Chan2 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch2:ErrProgramming",   2, 0x3,  1, 11); // Chan2 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch2:ErrCmdLength",     2, 0x3,  1, 12); // Chan2 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch2:ErrCmdBad",        2, 0x3,  1, 13); // Chan2 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch2:Event",            2, 0x3,  1, 14); // Chan2 got event              (0=no,1=yes)
    createChanStatusParam("Ch2:MultiDiscEvent",   2, 0x3,  1, 15); // Chan2 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch3:A:InputOffset",    3, 0x0,  9,  0); // Chan3 A Input Offset value
    createChanStatusParam("Ch3:A:AutoAdjOverflow",3, 0x0,  1,  9); // Chan3 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch3:A:AutoAdjSlope1",  3, 0x0,  1, 10); // Chan3 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch3:A:AutoAdjOffset",  3, 0x0,  1, 11); // Chan3 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch3:A:AutoAdjSlope",   3, 0x0,  1, 12); // Chan3 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch3:A:AutoAdjSample",  3, 0x0,  1, 13); // Chan3 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch3:A:AutoAdjTrig",    3, 0x0,  1, 14); // Chan3 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch3:A:AutoAdjEn",      3, 0x0,  1, 15); // Chan3 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch3:A:AdcOffsetVal",   3, 0x1,  8,  0); // Chan3 A ADC Offset value
    createChanStatusParam("Ch3:B:AdcOffsetVal",   3, 0x1,  8,  8); // Chan3 B ADC Offset value
    createChanStatusParam("Ch3:B:InputOffset",    3, 0x2,  9,  0); // Chan3 B Input Offset value
    createChanStatusParam("Ch3:B:AutoAdjOverflow",3, 0x2,  1,  9); // Chan3 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch3:B:AutoAdjSlope1",  3, 0x2,  1, 10); // Chan3 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch3:B:AutoAdjOffset",  3, 0x2,  1, 11); // Chan3 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch3:B:AutoAdjSlope",   3, 0x2,  1, 12); // Chan3 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch3:B:AutoAdjSample",  3, 0x2,  1, 13); // Chan3 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch3:B:AutoAdjTrig",    3, 0x2,  1, 14); // Chan3 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch3:B:AutoAdjEn",      3, 0x2,  1, 15); // Chan3 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch3:Configured",       3, 0x3,  1,  0); // Chan3 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch3:Enabled",          3, 0x3,  1,  1); // Chan3 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch3:Acquiring",        3, 0x3,  1,  2); // Chan3 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch3:FpgaHasData",      3, 0x3,  1,  3); // Chan3 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch3:AdcFifoHasData",   3, 0x3,  1,  4); // Chan3 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch3:AdcFifoAlmostFull",3, 0x3,  1,  5); // Chan3 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch3:PosDiscEvent",     3, 0x3,  1,  6); // Chan3 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch3:AdcFifoFull",      3, 0x3,  1,  7); // Chan3 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch3:EventFifoFull",    3, 0x3,  1,  8); // Chan3 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch3:AdcMin",           3, 0x3,  1,  9); // Chan3 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch3:AdcMax",           3, 0x3,  1, 10); // Chan3 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch3:ErrProgramming",   3, 0x3,  1, 11); // Chan3 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch3:ErrCmdLength",     3, 0x3,  1, 12); // Chan3 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch3:ErrCmdBad",        3, 0x3,  1, 13); // Chan3 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch3:Event",            3, 0x3,  1, 14); // Chan3 got event              (0=no,1=yes)
    createChanStatusParam("Ch3:MultiDiscEvent",   3, 0x3,  1, 15); // Chan3 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch4:A:InputOffset",    4, 0x0,  9,  0); // Chan4 A Input Offset value
    createChanStatusParam("Ch4:A:AutoAdjOverflow",4, 0x0,  1,  9); // Chan4 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch4:A:AutoAdjSlope1",  4, 0x0,  1, 10); // Chan4 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch4:A:AutoAdjOffset",  4, 0x0,  1, 11); // Chan4 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch4:A:AutoAdjSlope",   4, 0x0,  1, 12); // Chan4 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch4:A:AutoAdjSample",  4, 0x0,  1, 13); // Chan4 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch4:A:AutoAdjTrig",    4, 0x0,  1, 14); // Chan4 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch4:A:AutoAdjEn",      4, 0x0,  1, 15); // Chan4 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch4:A:AdcOffsetVal",   4, 0x1,  8,  0); // Chan4 A ADC Offset value
    createChanStatusParam("Ch4:B:AdcOffsetVal",   4, 0x1,  8,  8); // Chan4 B ADC Offset value
    createChanStatusParam("Ch4:B:InputOffset",    4, 0x2,  9,  0); // Chan4 B Input Offset value
    createChanStatusParam("Ch4:B:AutoAdjOverflow",4, 0x2,  1,  9); // Chan4 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch4:B:AutoAdjSlope1",  4, 0x2,  1, 10); // Chan4 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch4:B:AutoAdjOffset",  4, 0x2,  1, 11); // Chan4 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch4:B:AutoAdjSlope",   4, 0x2,  1, 12); // Chan4 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch4:B:AutoAdjSample",  4, 0x2,  1, 13); // Chan4 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch4:B:AutoAdjTrig",    4, 0x2,  1, 14); // Chan4 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch4:B:AutoAdjEn",      4, 0x2,  1, 15); // Chan4 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch4:Configured",       4, 0x3,  1,  0); // Chan4 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch4:Enabled",          4, 0x3,  1,  1); // Chan4 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch4:Acquiring",        4, 0x3,  1,  2); // Chan4 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch4:FpgaHasData",      4, 0x3,  1,  3); // Chan4 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch4:AdcFifoHasData",   4, 0x3,  1,  4); // Chan4 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch4:AdcFifoAlmostFull",4, 0x3,  1,  5); // Chan4 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch4:PosDiscEvent",     4, 0x3,  1,  6); // Chan4 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch4:AdcFifoFull",      4, 0x3,  1,  7); // Chan4 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch4:EventFifoFull",    4, 0x3,  1,  8); // Chan4 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch4:AdcMin",           4, 0x3,  1,  9); // Chan4 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch4:AdcMax",           4, 0x3,  1, 10); // Chan4 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch4:ErrProgramming",   4, 0x3,  1, 11); // Chan4 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch4:ErrCmdLength",     4, 0x3,  1, 12); // Chan4 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch4:ErrCmdBad",        4, 0x3,  1, 13); // Chan4 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch4:Event",            4, 0x3,  1, 14); // Chan4 got event              (0=no,1=yes)
    createChanStatusParam("Ch4:MultiDiscEvent",   4, 0x3,  1, 15); // Chan4 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch5:A:InputOffset",    5, 0x0,  9,  0); // Chan5 A Input Offset value
    createChanStatusParam("Ch5:A:AutoAdjOverflow",5, 0x0,  1,  9); // Chan5 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch5:A:AutoAdjSlope1",  5, 0x0,  1, 10); // Chan5 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch5:A:AutoAdjOffset",  5, 0x0,  1, 11); // Chan5 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch5:A:AutoAdjSlope",   5, 0x0,  1, 12); // Chan5 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch5:A:AutoAdjSample",  5, 0x0,  1, 13); // Chan5 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch5:A:AutoAdjTrig",    5, 0x0,  1, 14); // Chan5 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch5:A:AutoAdjEn",      5, 0x0,  1, 15); // Chan5 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch5:A:AdcOffsetVal",   5, 0x1,  8,  0); // Chan5 A ADC Offset value
    createChanStatusParam("Ch5:B:AdcOffsetVal",   5, 0x1,  8,  8); // Chan5 B ADC Offset value
    createChanStatusParam("Ch5:B:InputOffset",    5, 0x2,  9,  0); // Chan5 B Input Offset value
    createChanStatusParam("Ch5:B:AutoAdjOverflow",5, 0x2,  1,  9); // Chan5 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch5:B:AutoAdjSlope1",  5, 0x2,  1, 10); // Chan5 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch5:B:AutoAdjOffset",  5, 0x2,  1, 11); // Chan5 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch5:B:AutoAdjSlope",   5, 0x2,  1, 12); // Chan5 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch5:B:AutoAdjSample",  5, 0x2,  1, 13); // Chan5 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch5:B:AutoAdjTrig",    5, 0x2,  1, 14); // Chan5 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch5:B:AutoAdjEn",      5, 0x2,  1, 15); // Chan5 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch5:Configured",       5, 0x3,  1,  0); // Chan5 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch5:Enabled",          5, 0x3,  1,  1); // Chan5 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch5:Acquiring",        5, 0x3,  1,  2); // Chan5 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch5:FpgaHasData",      5, 0x3,  1,  3); // Chan5 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch5:AdcFifoHasData",   5, 0x3,  1,  4); // Chan5 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch5:AdcFifoAlmostFull",5, 0x3,  1,  5); // Chan5 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch5:PosDiscEvent",     5, 0x3,  1,  6); // Chan5 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch5:AdcFifoFull",      5, 0x3,  1,  7); // Chan5 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch5:EventFifoFull",    5, 0x3,  1,  8); // Chan5 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch5:AdcMin",           5, 0x3,  1,  9); // Chan5 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch5:AdcMax",           5, 0x3,  1, 10); // Chan5 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch5:ErrProgramming",   5, 0x3,  1, 11); // Chan5 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch5:ErrCmdLength",     5, 0x3,  1, 12); // Chan5 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch5:ErrCmdBad",        5, 0x3,  1, 13); // Chan5 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch5:Event",            5, 0x3,  1, 14); // Chan5 got event              (0=no,1=yes)
    createChanStatusParam("Ch5:MultiDiscEvent",   5, 0x3,  1, 15); // Chan5 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch6:A:InputOffset",    6, 0x0,  9,  0); // Chan6 A Input Offset value
    createChanStatusParam("Ch6:A:AutoAdjOverflow",6, 0x0,  1,  9); // Chan6 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch6:A:AutoAdjSlope1",  6, 0x0,  1, 10); // Chan6 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch6:A:AutoAdjOffset",  6, 0x0,  1, 11); // Chan6 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch6:A:AutoAdjSlope",   6, 0x0,  1, 12); // Chan6 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch6:A:AutoAdjSample",  6, 0x0,  1, 13); // Chan6 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch6:A:AutoAdjTrig",    6, 0x0,  1, 14); // Chan6 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch6:A:AutoAdjEn",      6, 0x0,  1, 15); // Chan6 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch6:A:AdcOffsetVal",   6, 0x1,  8,  0); // Chan6 A ADC Offset value
    createChanStatusParam("Ch6:B:AdcOffsetVal",   6, 0x1,  8,  8); // Chan6 B ADC Offset value
    createChanStatusParam("Ch6:B:InputOffset",    6, 0x2,  9,  0); // Chan6 B Input Offset value
    createChanStatusParam("Ch6:B:AutoAdjOverflow",6, 0x2,  1,  9); // Chan6 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch6:B:AutoAdjSlope1",  6, 0x2,  1, 10); // Chan6 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch6:B:AutoAdjOffset",  6, 0x2,  1, 11); // Chan6 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch6:B:AutoAdjSlope",   6, 0x2,  1, 12); // Chan6 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch6:B:AutoAdjSample",  6, 0x2,  1, 13); // Chan6 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch6:B:AutoAdjTrig",    6, 0x2,  1, 14); // Chan6 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch6:B:AutoAdjEn",      6, 0x2,  1, 15); // Chan6 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch6:Configured",       6, 0x3,  1,  0); // Chan6 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch6:Enabled",          6, 0x3,  1,  1); // Chan6 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch6:Acquiring",        6, 0x3,  1,  2); // Chan6 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch6:FpgaHasData",      6, 0x3,  1,  3); // Chan6 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch6:AdcFifoHasData",   6, 0x3,  1,  4); // Chan6 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch6:AdcFifoAlmostFull",6, 0x3,  1,  5); // Chan6 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch6:PosDiscEvent",     6, 0x3,  1,  6); // Chan6 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch6:AdcFifoFull",      6, 0x3,  1,  7); // Chan6 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch6:EventFifoFull",    6, 0x3,  1,  8); // Chan6 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch6:AdcMin",           6, 0x3,  1,  9); // Chan6 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch6:AdcMax",           6, 0x3,  1, 10); // Chan6 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch6:ErrProgramming",   6, 0x3,  1, 11); // Chan6 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch6:ErrCmdLength",     6, 0x3,  1, 12); // Chan6 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch6:ErrCmdBad",        6, 0x3,  1, 13); // Chan6 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch6:Event",            6, 0x3,  1, 14); // Chan6 got event              (0=no,1=yes)
    createChanStatusParam("Ch6:MultiDiscEvent",   6, 0x3,  1, 15); // Chan6 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch7:A:InputOffset",    7, 0x0,  9,  0); // Chan7 A Input Offset value
    createChanStatusParam("Ch7:A:AutoAdjOverflow",7, 0x0,  1,  9); // Chan7 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch7:A:AutoAdjSlope1",  7, 0x0,  1, 10); // Chan7 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch7:A:AutoAdjOffset",  7, 0x0,  1, 11); // Chan7 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch7:A:AutoAdjSlope",   7, 0x0,  1, 12); // Chan7 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch7:A:AutoAdjSample",  7, 0x0,  1, 13); // Chan7 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch7:A:AutoAdjTrig",    7, 0x0,  1, 14); // Chan7 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch7:A:AutoAdjEn",      7, 0x0,  1, 15); // Chan7 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch7:A:AdcOffsetVal",   7, 0x1,  8,  0); // Chan7 A ADC Offset value
    createChanStatusParam("Ch7:B:AdcOffsetVal",   7, 0x1,  8,  8); // Chan7 B ADC Offset value
    createChanStatusParam("Ch7:B:InputOffset",    7, 0x2,  9,  0); // Chan7 B Input Offset value
    createChanStatusParam("Ch7:B:AutoAdjOverflow",7, 0x2,  1,  9); // Chan7 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch7:B:AutoAdjSlope1",  7, 0x2,  1, 10); // Chan7 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch7:B:AutoAdjOffset",  7, 0x2,  1, 11); // Chan7 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch7:B:AutoAdjSlope",   7, 0x2,  1, 12); // Chan7 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch7:B:AutoAdjSample",  7, 0x2,  1, 13); // Chan7 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch7:B:AutoAdjTrig",    7, 0x2,  1, 14); // Chan7 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch7:B:AutoAdjEn",      7, 0x2,  1, 15); // Chan7 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch7:Configured",       7, 0x3,  1,  0); // Chan7 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch7:Enabled",          7, 0x3,  1,  1); // Chan7 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch7:Acquiring",        7, 0x3,  1,  2); // Chan7 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch7:FpgaHasData",      7, 0x3,  1,  3); // Chan7 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch7:AdcFifoHasData",   7, 0x3,  1,  4); // Chan7 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch7:AdcFifoAlmostFull",7, 0x3,  1,  5); // Chan7 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch7:PosDiscEvent",     7, 0x3,  1,  6); // Chan7 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch7:AdcFifoFull",      7, 0x3,  1,  7); // Chan7 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch7:EventFifoFull",    7, 0x3,  1,  8); // Chan7 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch7:AdcMin",           7, 0x3,  1,  9); // Chan7 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch7:AdcMax",           7, 0x3,  1, 10); // Chan7 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch7:ErrProgramming",   7, 0x3,  1, 11); // Chan7 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch7:ErrCmdLength",     7, 0x3,  1, 12); // Chan7 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch7:ErrCmdBad",        7, 0x3,  1, 13); // Chan7 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch7:Event",            7, 0x3,  1, 14); // Chan7 got event              (0=no,1=yes)
    createChanStatusParam("Ch7:MultiDiscEvent",   7, 0x3,  1, 15); // Chan7 got multi-discrim ev   (0=no,1=yes)

    createChanStatusParam("Ch8:A:InputOffset",    8, 0x0,  9,  0); // Chan8 A Input Offset value
    createChanStatusParam("Ch8:A:AutoAdjOverflow",8, 0x0,  1,  9); // Chan8 A Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch8:A:AutoAdjSlope1",  8, 0x0,  1, 10); // Chan8 A Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch8:A:AutoAdjOffset",  8, 0x0,  1, 11); // Chan8 A Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch8:A:AutoAdjSlope",   8, 0x0,  1, 12); // Chan8 A Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch8:A:AutoAdjSample",  8, 0x0,  1, 13); // Chan8 A Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch8:A:AutoAdjTrig",    8, 0x0,  1, 14); // Chan8 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch8:A:AutoAdjEn",      8, 0x0,  1, 15); // Chan8 A Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch8:A:AdcOffsetVal",   8, 0x1,  8,  0); // Chan8 A ADC Offset value
    createChanStatusParam("Ch8:B:AdcOffsetVal",   8, 0x1,  8,  8); // Chan8 B ADC Offset value
    createChanStatusParam("Ch8:B:InputOffset",    8, 0x2,  9,  0); // Chan8 B Input Offset value
    createChanStatusParam("Ch8:B:AutoAdjOverflow",8, 0x2,  1,  9); // Chan8 B Auto-Adjust overflow (0=no,1=yes)
    createChanStatusParam("Ch8:B:AutoAdjSlope1",  8, 0x2,  1, 10); // Chan8 B Auto-Adjust Slope? l (0=not active,1=active)
    createChanStatusParam("Ch8:B:AutoAdjOffset",  8, 0x2,  1, 11); // Chan8 B Auto-Adjust Offset l (0=not active,1=active)
    createChanStatusParam("Ch8:B:AutoAdjSlope",   8, 0x2,  1, 12); // Chan8 B Auto-Adjust Slope l  (0=not active,1=active)
    createChanStatusParam("Ch8:B:AutoAdjSample",  8, 0x2,  1, 13); // Chan8 B Auto-Adjust Sample l (0=not active,1=active)
    createChanStatusParam("Ch8:B:AutoAdjTrig",    8, 0x2,  1, 14); // Chan8 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createChanStatusParam("Ch8:B:AutoAdjEn",      8, 0x2,  1, 15); // Chan8 B Auto-Adjust Active   (0=not active,1=active)
    createChanStatusParam("Ch8:Configured",       8, 0x3,  1,  0); // Chan8 configured             (0=not configured [alarm],1=configured, archive:monitor)
    createChanStatusParam("Ch8:Enabled",          8, 0x3,  1,  1); // Chan8 enabled                (0=disabled [alarm],1=enabled, archive:monitor)
    createChanStatusParam("Ch8:Acquiring",        8, 0x3,  1,  2); // Chan8 Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createChanStatusParam("Ch8:FpgaHasData",      8, 0x3,  1,  3); // Chan8 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch8:AdcFifoHasData",   8, 0x3,  1,  4); // Chan8 FIFO has data          (0=no,1=yes)
    createChanStatusParam("Ch8:AdcFifoAlmostFull",8, 0x3,  1,  5); // Chan8 FIFO almost full       (0=no,1=yes)
    createChanStatusParam("Ch8:PosDiscEvent",     8, 0x3,  1,  6); // Chan8 positive discrim ev    (0=no,1=yes)
    createChanStatusParam("Ch8:AdcFifoFull",      8, 0x3,  1,  7); // Chan8 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch8:EventFifoFull",    8, 0x3,  1,  8); // Chan8 FIFO full detectec     (0=no,1=yes)
    createChanStatusParam("Ch8:AdcMin",           8, 0x3,  1,  9); // Chan8 got ADC min            (0=no,1=yes)
    createChanStatusParam("Ch8:AdcMax",           8, 0x3,  1, 10); // Chan8 got ADC max            (0=no,1=yes)
    createChanStatusParam("Ch8:ErrProgramming",   8, 0x3,  1, 11); // Chan8 WRITE_CNFG during ACQ  (0=no error,1=error)
    createChanStatusParam("Ch8:ErrCmdLength",     8, 0x3,  1, 12); // Chan8 Command length error   (0=no error,1=error)
    createChanStatusParam("Ch8:ErrCmdBad",        8, 0x3,  1, 13); // Chan8 bad command error      (0=no error,1=error)
    createChanStatusParam("Ch8:Event",            8, 0x3,  1, 14); // Chan8 got event              (0=no,1=yes)
    createChanStatusParam("Ch8:MultiDiscEvent",   8, 0x3,  1, 15); // Chan8 got multi-discrim ev   (0=no,1=yes)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:A:Scale",      '8',  0x0, 12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch1:A:Offset",     '8',  0x1, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch1:B:Scale",      '8',  0x2, 12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch1:B:Offset",     '8',  0x3, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:A:Scale",      '8',  0x4, 12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch2:A:Offset",     '8',  0x5, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch2:B:Scale",      '8',  0x6, 12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch2:B:Offset",     '8',  0x7, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:A:Scale",      '8',  0x8, 12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch3:A:Offset",     '8',  0x9, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch3:B:Scale",      '8',  0xA, 12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch3:B:Offset",     '8',  0xB, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:A:Scale",      '8',  0xC, 12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch4:A:Offset",     '8',  0xD, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch4:B:Scale",      '8',  0xE, 12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch4:B:Offset",     '8',  0xF, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5:A:Scale",      '8', 0x10, 12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch5:A:Offset",     '8', 0x11, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch5:B:Scale",      '8', 0x12, 12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch5:B:Offset",     '8', 0x13, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:A:Scale",      '8', 0x14, 12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch6:A:Offset",     '8', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch6:B:Scale",      '8', 0x16, 12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch6:B:Offset",     '8', 0x17, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:A:Scale",      '8', 0x18, 12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch7:A:Offset",     '8', 0x19, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch7:B:Scale",      '8', 0x1A, 12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch7:B:Offset",     '8', 0x1B, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:A:Scale",      '8', 0x1C, 12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch8:A:Offset",     '8', 0x1D, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch8:B:Scale",      '8', 0x1E, 12, 0, 2048);  // Chan8 B scale
    createConfigParam("Ch8:B:Offset",     '8', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFact",    '9',  0x0, 12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFact",    '9',  0x1, 12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFact",    '9',  0x2, 12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFact",    '9',  0x3, 12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFact",    '9',  0x4, 12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFact",    '9',  0x5, 12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFact",    '9',  0x6, 12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFact",    '9',  0x7, 12, 0, 2048);  // Chan8 scale factor

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

    createConfigParam("TimeVetoLow",      'E',  0x1,  32, 0, 0);    // Timestamp veto low            (scale:100,unit:ns,prec:1)
    createConfigParam("TimeVetoHigh",     'E',  0x3,  32, 0, 333);  // Timestamp veto high           (scale:100,unit:ns,prec:1)
    createConfigParam("FakeTrigDelay",    'E',  0x5,  16, 0, 200);  // Fake trigger delay            (scale:100,unit:ns,prec:1)

    createConfigParam("LvdsRate",         'F',  0x0,  1, 15, 0);    // LVDS output rate              (0=20Mhz,1=10Mhz)
    createConfigParam("Resolution",       'F',  0x0,  1, 9,  0);    // High resolution mode          (0=128,1=256)
    createConfigParam("TcTxEnMode",       'F',  0x0,  1, 8,  1);    // T&C TX enable mode            (0=external,1=internal)
    createConfigParam("OutputMode",       'F',  0x0,  2, 6,  0);    // Output mode                   (0=normal,1=raw [alarm],2=extended [alarm])
    createConfigParam("AcquireEn",        'F',  0x0,  1, 5,  1);    // ROC enable acquisition        (0=disable,1=enable)
    createConfigParam("TcTsyncMode",      'F',  0x0,  1, 4,  0);    // T&C TSYNC mode                (0=external,1=internal 60Hz)
    createConfigParam("TcTclkMode",       'F',  0x0,  1, 3,  0);    // T&C TCLK mode                 (0=external,1=internal 10MHz)
    createConfigParam("TcResetMode",      'F',  0x0,  1, 2,  0);    // T&C Reset mode                (0=internal,1=external)
    createConfigParam("AcquireMode",      'F',  0x0,  2, 0,  0);    // Acquire mode                  (0=normal,1=verbose,2=fakedata,3=trigger)

    // Now channel parameters - note the extra channel parameter
    createChanConfigParam("Ch1:PositionIdx",  1, '1',  0x0, 32, 0, 6144);   // Chan1 position index
    createChanConfigParam("Ch1:A:InOffset",   1, '1',  0x2,  9, 0, 6, CONV_SIGN_MAGN); // Chan1 A input offset
    createChanConfigParam("Ch1:B:InOffset",   1, '1',  0x3,  9, 0, 5, CONV_SIGN_MAGN); // Chan1 B input offset
    createChanConfigParam("Ch1:A:FullScale",  1, '1',  0x4,  8, 0, 0);      // Chan1 A full scale
    createChanConfigParam("Ch1:Sum:FullScale",1, '1',  0x5,  8, 0, 0);      // Chan1 SUM full scale
    createChanConfigParam("Ch1:B:FullScale",  1, '1',  0x6,  8, 0, 0);      // Chan1 B full scale
    createChanConfigParam("Ch1:PosThreshold", 1, '1',  0x7,  8, 0, 122);    // Chan1 positive threshold
    createChanConfigParam("Ch1:NegThreshold", 1, '1',  0x8,  8, 0, 110);    // Chan1 negative threshold
    createChanConfigParam("Ch1:A:AdcOffset",  1, '1',  0x9,  8, 0, 48);     // Chan1 A ADC offset
    createChanConfigParam("Ch1:Sum:AdcOffset",1, '1',  0xA,  8, 0, 0);      // Chan1 sum zero
    createChanConfigParam("Ch1:B:AdcOffset",  1, '1',  0xB,  8, 0, 79);     // Chan1 B ADC offset
    createChanConfigParam("Ch1:A:GainAdj",    1, '1',  0xC,  8, 0, 0);      // Chan1 A gain adjust
    createChanConfigParam("Ch1:B:GainAdj",    1, '1',  0xD,  8, 0, 0);      // Chan1 B gain adjust
    createChanConfigParam("Ch1:A:AvgMin",     1, '1',  0xE, 16, 0, 1466);   // Chan1 A average minimum
    createChanConfigParam("Ch1:B:AvgMin",     1, '1',  0xF, 16, 0, 1466);   // Chan1 B average minimum
    createChanConfigParam("Ch1:A:AvgMax",     1, '1', 0x10, 16, 0, 1581);   // Chan1 A average maximum
    createChanConfigParam("Ch1:B:AvgMax",     1, '1', 0x11, 16, 0, 1581);   // Chan1 B average maximum
    createChanConfigParam("Ch1:MaximumSlope", 1, '1', 0x12, 16, 0, 0);      // Chan1 Maximum slope
    createChanConfigParam("Ch1:MinAdc",       1, '8',  0x0, 14, 0, 100);    // Chan1 Minimum ADC
    createChanConfigParam("Ch1:MaxAdc",       1, '8',  0x1, 14, 0, 1000);   // Chan1 Maximum ADC
    createChanConfigParam("Ch1:IntRelease",   1, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP); // Chan1 Integrator release p
    createChanConfigParam("Ch1:MinDiscrimWidth",1,'8', 0x3,  9, 0, 1);      // Chan1 min pulse width
    createChanConfigParam("Ch1:VetoInhibit",  1, '8',  0x4, 12, 0, 10);     // Chan1 veto inhibit point
    createChanConfigParam("Ch1:NegDiscrTime", 1, '8',  0x5, 12, 0, 0);      // Chan1 negative discr timeout
    createChanConfigParam("Ch1:Sample1",      1, '8',  0x6,  9, 0, 0);      // Chan1 Sample 1 point [0:10]
    createChanConfigParam("Ch1:Sample2",      1, '8',  0x7,  9, 0, 15);     // Chan1 Sample 2 point [0:10]

    createChanConfigParam("Ch2:PositionIdx",  2, '1',  0x0, 32, 0, 6272);   // Chan2 position index
    createChanConfigParam("Ch2:A:InOffset",   2, '1',  0x2,  9, 0, 261, CONV_SIGN_MAGN); // Chan2 A input offset
    createChanConfigParam("Ch2:B:InOffset",   2, '1',  0x3,  9, 0, 271, CONV_SIGN_MAGN); // Chan2 B input offset
    createChanConfigParam("Ch2:A:FullScale",  2, '1',  0x4,  8, 0, 0);      // Chan2 A full scale
    createChanConfigParam("Ch2:Sum:FullScale",2, '1',  0x5,  8, 0, 0);      // Chan2 SUM full scale
    createChanConfigParam("Ch2:B:FullScale",  2, '1',  0x6,  8, 0, 0);      // Chan2 B full scale
    createChanConfigParam("Ch2:PosThreshold", 2, '1',  0x7,  8, 0, 122);    // Chan2 positive threshold
    createChanConfigParam("Ch2:NegThreshold", 2, '1',  0x8,  8, 0, 110);    // Chan2 negative threshold
    createChanConfigParam("Ch2:A:AdcOffset",  2, '1',  0x9,  8, 0, 84);     // Chan2 A ADC offset
    createChanConfigParam("Ch2:Sum:AdcOffset",2, '1',  0xA,  8, 0, 0);      // Chan2 sum zero
    createChanConfigParam("Ch2:B:AdcOffset",  2, '1',  0xB,  8, 0, 94);     // Chan2 B ADC offset
    createChanConfigParam("Ch2:A:GainAdj",    2, '1',  0xC,  8, 0, 0);      // Chan2 A gain adjust
    createChanConfigParam("Ch2:B:GainAdj",    2, '1',  0xD,  8, 0, 0);      // Chan2 B gain adjust
    createChanConfigParam("Ch2:A:AvgMin",     2, '1',  0xE, 16, 0, 1691);   // Chan2 A average minimum
    createChanConfigParam("Ch2:B:AvgMin",     2, '1',  0xF, 16, 0, 1691);   // Chan2 B average minimum
    createChanConfigParam("Ch2:A:AvgMax",     2, '1', 0x10, 16, 0, 1690);   // Chan2 A average maximum
    createChanConfigParam("Ch2:B:AvgMax",     2, '1', 0x11, 16, 0, 1690);   // Chan2 B average maximum
    createChanConfigParam("Ch2:MaximumSlope", 2, '1', 0x12, 16, 0, 0);      // Chan2 Maximum slope
    createChanConfigParam("Ch2:MinAdc",       2, '8',  0x0, 14, 0, 100);    // Chan2 Minimum ADC
    createChanConfigParam("Ch2:MaxAdc",       2, '8',  0x1, 14, 0, 1000);   // Chan2 Maximum ADC
    createChanConfigParam("Ch2:IntRelease",   2, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan2 Integrator release p
    createChanConfigParam("Ch2:MinDiscrimWidth",2,'8', 0x3,  9, 0, 0);      // Chan2 min pulse width
    createChanConfigParam("Ch2:VetoInhibit",  2, '8',  0x4, 12, 0, 10);     // Chan2 veto inhibit point
    createChanConfigParam("Ch2:NegDiscrTime", 2, '8',  0x5, 12, 0, 0);      // Chan2 negative discr timeout
    createChanConfigParam("Ch2:Sample1",      2, '8',  0x6,  9, 0, 0);      // Chan2 Sample 1 point [0:10]
    createChanConfigParam("Ch2:Sample2",      2, '8',  0x7,  9, 0, 15);     // Chan2 Sample 2 point [0:10]

    createChanConfigParam("Ch3:PositionIdx",  3, '1',  0x0, 32, 0, 6400);   // Chan3 position index
    createChanConfigParam("Ch3:A:InOffset",   3, '1',  0x2,  9, 0, 262, CONV_SIGN_MAGN); // Chan3 A input offset
    createChanConfigParam("Ch3:B:InOffset",   3, '1',  0x3,  9, 0, 4, CONV_SIGN_MAGN); // Chan3 B input offset
    createChanConfigParam("Ch3:A:FullScale",  3, '1',  0x4,  8, 0, 0);      // Chan3 A full scale
    createChanConfigParam("Ch3:Sum:FullScale",3, '1',  0x5,  8, 0, 0);      // Chan3 SUM full scale
    createChanConfigParam("Ch3:B:FullScale",  3, '1',  0x6,  8, 0, 0);      // Chan3 B full scale
    createChanConfigParam("Ch3:PosThreshold", 3, '1',  0x7,  8, 0, 122);    // Chan3 positive threshold
    createChanConfigParam("Ch3:NegThreshold", 3, '1',  0x8,  8, 0, 110);    // Chan3 negative threshold
    createChanConfigParam("Ch3:A:AdcOffset",  3, '1',  0x9,  8, 0, 60);     // Chan3 A ADC offset
    createChanConfigParam("Ch3:Sum:AdcOffset",3, '1',  0xA,  8, 0, 0);      // Chan3 sum zero
    createChanConfigParam("Ch3:B:AdcOffset",  3, '1',  0xB,  8, 0, 90);     // Chan3 B ADC offset
    createChanConfigParam("Ch3:A:GainAdj",    3, '1',  0xC,  8, 0, 0);      // Chan3 A gain adjust
    createChanConfigParam("Ch3:B:GainAdj",    3, '1',  0xD,  8, 0, 0);      // Chan3 B gain adjust
    createChanConfigParam("Ch3:A:AvgMin",     3, '1',  0xE, 16, 0, 1694);   // Chan3 A average minimum
    createChanConfigParam("Ch3:B:AvgMin",     3, '1',  0xF, 16, 0, 1694);   // Chan3 B average minimum
    createChanConfigParam("Ch3:A:AvgMax",     3, '1', 0x10, 16, 0, 1593);   // Chan3 A average maximum
    createChanConfigParam("Ch3:B:AvgMax",     3, '1', 0x11, 16, 0, 1593);   // Chan3 B average maximum
    createChanConfigParam("Ch3:MaximumSlope", 3, '1', 0x12, 16, 0, 0);      // Chan3 Maximum slope
    createChanConfigParam("Ch3:MinAdc",       3, '8',  0x0, 14, 0, 100);    // Chan3 Minimum ADC
    createChanConfigParam("Ch3:MaxAdc",       3, '8',  0x1, 14, 0, 1000);   // Chan3 Maximum ADC
    createChanConfigParam("Ch3:IntRelease",   3, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan3 Integrator release p
    createChanConfigParam("Ch3:MinDiscrimWidth",3,'8', 0x3,  9, 0, 0);      // Chan3 min pulse width
    createChanConfigParam("Ch3:VetoInhibit",  3, '8',  0x4, 12, 0, 10);     // Chan3 veto inhibit point
    createChanConfigParam("Ch3:NegDiscrTime", 3, '8',  0x5, 12, 0, 0);      // Chan3 negative discr timeout
    createChanConfigParam("Ch3:Sample1",      3, '8',  0x6,  9, 0, 0);      // Chan3 Sample 1 point [0:10]
    createChanConfigParam("Ch3:Sample2",      3, '8',  0x7,  9, 0, 15);     // Chan3 Sample 2 point [0:10]

    createChanConfigParam("Ch4:PositionIdx",  4, '1',  0x0, 32, 0, 6528);   // Chan4 position index
    createChanConfigParam("Ch4:A:InOffset",   4, '1',  0x2,  9, 0, 297, CONV_SIGN_MAGN); // Chan4 A input offset
    createChanConfigParam("Ch4:B:InOffset",   4, '1',  0x3,  9, 0, 12, CONV_SIGN_MAGN); // Chan4 B input offset
    createChanConfigParam("Ch4:A:FullScale",  4, '1',  0x4,  8, 0, 0);      // Chan4 A full scale
    createChanConfigParam("Ch4:Sum:FullScale",4, '1',  0x5,  8, 0, 0);      // Chan4 SUM full scale
    createChanConfigParam("Ch4:B:FullScale",  4, '1',  0x6,  8, 0, 0);      // Chan4 B full scale
    createChanConfigParam("Ch4:PosThreshold", 4, '1',  0x7,  8, 0, 122);    // Chan4 positive threshold
    createChanConfigParam("Ch4:NegThreshold", 4, '1',  0x8,  8, 0, 110);    // Chan4 negative threshold
    createChanConfigParam("Ch4:A:AdcOffset",  4, '1',  0x9,  8, 0, 63);     // Chan4 A ADC offset
    createChanConfigParam("Ch4:Sum:AdcOffset",4, '1',  0xA,  8, 0, 0);      // Chan4 sum zero
    createChanConfigParam("Ch4:B:AdcOffset",  4, '1',  0xB,  8, 0, 77);     // Chan4 B ADC offset
    createChanConfigParam("Ch4:A:GainAdj",    4, '1',  0xC,  8, 0, 0);      // Chan4 A gain adjust
    createChanConfigParam("Ch4:B:GainAdj",    4, '1',  0xD,  8, 0, 0);      // Chan4 B gain adjust
    createChanConfigParam("Ch4:A:AvgMin",     4, '1',  0xE, 16, 0, 1650);   // Chan4 A average minimum
    createChanConfigParam("Ch4:B:AvgMin",     4, '1',  0xF, 16, 0, 1650);   // Chan4 B average minimum
    createChanConfigParam("Ch4:A:AvgMax",     4, '1', 0x10, 16, 0, 1396);   // Chan4 A average maximum
    createChanConfigParam("Ch4:B:AvgMax",     4, '1', 0x11, 16, 0, 1396);   // Chan4 B average maximum
    createChanConfigParam("Ch4:MaximumSlope", 4, '1', 0x12, 16, 0, 0);      // Chan4 Maximum slope
    createChanConfigParam("Ch4:MinAdc",       4, '8',  0x0, 14, 0, 100);    // Chan4 Minimum ADC
    createChanConfigParam("Ch4:MaxAdc",       4, '8',  0x1, 14, 0, 1000);   // Chan4 Maximum ADC
    createChanConfigParam("Ch4:IntRelease",   4, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan4 Integrator release p
    createChanConfigParam("Ch4:MinDiscrimWidth",4,'8', 0x3,  9, 0, 0);      // Chan4 min pulse width
    createChanConfigParam("Ch4:VetoInhibit",  4, '8',  0x4, 12, 0, 10);     // Chan4 veto inhibit point
    createChanConfigParam("Ch4:NegDiscrTime", 4, '8',  0x5, 12, 0, 0);      // Chan4 negative discr timeout
    createChanConfigParam("Ch4:Sample1",      4, '8',  0x6,  9, 0, 0);      // Chan4 Sample 1 point [0:10]
    createChanConfigParam("Ch4:Sample2",      4, '8',  0x7,  9, 0, 15);     // Chan4 Sample 2 point [0:10]

    createChanConfigParam("Ch5:PositionIdx",  5, '1',  0x0, 32, 0, 6656);   // Chan5 position index
    createChanConfigParam("Ch5:A:InOffset",   5, '1',  0x2,  9, 0, 263, CONV_SIGN_MAGN); // Chan5 A input offset
    createChanConfigParam("Ch5:B:InOffset",   5, '1',  0x3,  9, 0, 268, CONV_SIGN_MAGN); // Chan5 B input offset
    createChanConfigParam("Ch5:A:FullScale",  5, '1',  0x4,  8, 0, 0);      // Chan5 A full scale
    createChanConfigParam("Ch5:Sum:FullScale",5, '1',  0x5,  8, 0, 0);      // Chan5 SUM full scale
    createChanConfigParam("Ch5:B:FullScale",  5, '1',  0x6,  8, 0, 0);      // Chan5 B full scale
    createChanConfigParam("Ch5:PosThreshold", 5, '1',  0x7,  8, 0, 122);    // Chan5 positive threshold
    createChanConfigParam("Ch5:NegThreshold", 5, '1',  0x8,  8, 0, 110);    // Chan5 negative threshold
    createChanConfigParam("Ch5:A:AdcOffset",  5, '1',  0x9,  8, 0, 66);     // Chan5 A ADC offset
    createChanConfigParam("Ch5:Sum:AdcOffset",5, '1',  0xA,  8, 0, 0);      // Chan5 sum zero
    createChanConfigParam("Ch5:B:AdcOffset",  5, '1',  0xB,  8, 0, 97);     // Chan5 B ADC offset
    createChanConfigParam("Ch5:A:GainAdj",    5, '1',  0xC,  8, 0, 0);      // Chan5 A gain adjust
    createChanConfigParam("Ch5:B:GainAdj",    5, '1',  0xD,  8, 0, 0);      // Chan5 B gain adjust
    createChanConfigParam("Ch5:A:AvgMin",     5, '1',  0xE, 16, 0, 1488);   // Chan5 A average minimum
    createChanConfigParam("Ch5:B:AvgMin",     5, '1',  0xF, 16, 0, 1488);   // Chan5 B average minimum
    createChanConfigParam("Ch5:A:AvgMax",     5, '1', 0x10, 16, 0, 1567);   // Chan5 A average maximum
    createChanConfigParam("Ch5:B:AvgMax",     5, '1', 0x11, 16, 0, 1567);   // Chan5 B average maximum
    createChanConfigParam("Ch5:MaximumSlope", 5, '1', 0x12, 16, 0, 0);      // Chan5 Maximum slope
    createChanConfigParam("Ch5:MinAdc",       5, '8',  0x0, 14, 0, 100);    // Chan5 Minimum ADC
    createChanConfigParam("Ch5:MaxAdc",       5, '8',  0x1, 14, 0, 1000);   // Chan5 Maximum ADC
    createChanConfigParam("Ch5:IntRelease",   5, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan5 Integrator release p
    createChanConfigParam("Ch5:MinDiscrimWidth",5,'8', 0x3,  9, 0, 0);      // Chan5 min pulse width
    createChanConfigParam("Ch5:VetoInhibit",  5, '8',  0x4, 12, 0, 10);     // Chan5 veto inhibit point
    createChanConfigParam("Ch5:NegDiscrTime", 5, '8',  0x5, 12, 0, 0);      // Chan5 negative discr timeout
    createChanConfigParam("Ch5:Sample1",      5, '8',  0x6,  9, 0, 0);      // Chan5 Sample 1 point [0:10]
    createChanConfigParam("Ch5:Sample2",      5, '8',  0x7,  9, 0, 15);     // Chan5 Sample 2 point [0:10]

    createChanConfigParam("Ch6:PositionIdx",  6, '1',  0x0, 32, 0, 6784);   // Chan6 position index
    createChanConfigParam("Ch6:A:InOffset",   6, '1',  0x2,  9, 0, 3, CONV_SIGN_MAGN); // Chan6 A input offset
    createChanConfigParam("Ch6:B:InOffset",   6, '1',  0x3,  9, 0, 2, CONV_SIGN_MAGN); // Chan6 B input offset
    createChanConfigParam("Ch6:A:FullScale",  6, '1',  0x4,  8, 0, 0);      // Chan6 A full scale
    createChanConfigParam("Ch6:Sum:FullScale",6, '1',  0x5,  8, 0, 0);      // Chan6 SUM full scale
    createChanConfigParam("Ch6:B:FullScale",  6, '1',  0x6,  8, 0, 0);      // Chan6 B full scale
    createChanConfigParam("Ch6:PosThreshold", 6, '1',  0x7,  8, 0, 122);    // Chan6 positive threshold
    createChanConfigParam("Ch6:NegThreshold", 6, '1',  0x8,  8, 0, 110);    // Chan6 negative threshold
    createChanConfigParam("Ch6:A:AdcOffset",  6, '1',  0x9,  8, 0, 90);     // Chan6 A ADC offset
    createChanConfigParam("Ch6:Sum:AdcOffset",6, '1',  0xA,  8, 0, 0);      // Chan6 sum zero
    createChanConfigParam("Ch6:B:AdcOffset",  6, '1',  0xB,  8, 0, 77);     // Chan6 B ADC offset
    createChanConfigParam("Ch6:A:GainAdj",    6, '1',  0xC,  8, 0, 0);      // Chan6 A gain adjust
    createChanConfigParam("Ch6:B:GainAdj",    6, '1',  0xD,  8, 0, 0);      // Chan6 B gain adjust
    createChanConfigParam("Ch6:A:AvgMin",     6, '1',  0xE, 16, 0, 1667);   // Chan6 A average minimum
    createChanConfigParam("Ch6:B:AvgMin",     6, '1',  0xF, 16, 0, 1667);   // Chan6 B average minimum
    createChanConfigParam("Ch6:A:AvgMax",     6, '1', 0x10, 16, 0, 1561);   // Chan6 A average maximum
    createChanConfigParam("Ch6:B:AvgMax",     6, '1', 0x11, 16, 0, 1561);   // Chan6 B average maximum
    createChanConfigParam("Ch6:MaximumSlope", 6, '1', 0x12, 16, 0, 0);      // Chan6 Maximum slope
    createChanConfigParam("Ch6:MinAdc",       6, '8',  0x0, 14, 0, 100);    // Chan6 Minimum ADC
    createChanConfigParam("Ch6:MaxAdc",       6, '8',  0x1, 14, 0, 1000);   // Chan6 Maximum ADC
    createChanConfigParam("Ch6:IntRelease",   6, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan6 Integrator release p
    createChanConfigParam("Ch6:MinDiscrimWidth",6,'8', 0x3,  9, 0, 0);      // Chan6 min pulse width
    createChanConfigParam("Ch6:VetoInhibit",  6, '8',  0x4, 12, 0, 10);     // Chan6 veto inhibit point
    createChanConfigParam("Ch6:NegDiscrTime", 6, '8',  0x5, 12, 0, 0);      // Chan6 negative discr timeout
    createChanConfigParam("Ch6:Sample1",      6, '8',  0x6,  9, 0, 0);      // Chan6 Sample 1 point [0:10]
    createChanConfigParam("Ch6:Sample2",      6, '8',  0x7,  9, 0, 15);     // Chan6 Sample 2 point [0:10]

    createChanConfigParam("Ch7:PositionIdx",  7, '1',  0x0, 32, 0, 6912);   // Chan7 position index
    createChanConfigParam("Ch7:A:InOffset",   7, '1',  0x2,  9, 0, 258, CONV_SIGN_MAGN); // Chan7 A input offset
    createChanConfigParam("Ch7:B:InOffset",   7, '1',  0x3,  9, 0, 9, CONV_SIGN_MAGN); // Chan7 B input offset
    createChanConfigParam("Ch7:A:FullScale",  7, '1',  0x4,  8, 0, 0);      // Chan7 A full scale
    createChanConfigParam("Ch7:Sum:FullScale",7, '1',  0x5,  8, 0, 0);      // Chan7 SUM full scale
    createChanConfigParam("Ch7:B:FullScale",  7, '1',  0x6,  8, 0, 0);      // Chan7 B full scale
    createChanConfigParam("Ch7:PosThreshold", 7, '1',  0x7,  8, 0, 122);    // Chan7 positive threshold
    createChanConfigParam("Ch7:NegThreshold", 7, '1',  0x8,  8, 0, 110);    // Chan7 negative threshold
    createChanConfigParam("Ch7:A:AdcOffset",  7, '1',  0x9,  8, 0, 63);     // Chan7 A ADC offset
    createChanConfigParam("Ch7:Sum:AdcOffset",7, '1',  0xA,  8, 0, 0);      // Chan7 sum zero
    createChanConfigParam("Ch7:B:AdcOffset",  7, '1',  0xB,  8, 0, 98);     // Chan7 B ADC offset
    createChanConfigParam("Ch7:A:GainAdj",    7, '1',  0xC,  8, 0, 0);      // Chan7 A gain adjust
    createChanConfigParam("Ch7:B:GainAdj",    7, '1',  0xD,  8, 0, 0);      // Chan7 B gain adjust
    createChanConfigParam("Ch7:A:AvgMin",     7, '1',  0xE, 16, 0, 1671);   // Chan7 A average minimum
    createChanConfigParam("Ch7:B:AvgMin",     7, '1',  0xF, 16, 0, 1671);   // Chan7 B average minimum
    createChanConfigParam("Ch7:A:AvgMax",     7, '1', 0x10, 16, 0, 1483);   // Chan7 A average maximum
    createChanConfigParam("Ch7:B:AvgMax",     7, '1', 0x11, 16, 0, 1483);   // Chan7 B average maximum
    createChanConfigParam("Ch7:MaximumSlope", 7, '1', 0x12, 16, 0, 0);      // Chan7 Maximum slope
    createChanConfigParam("Ch7:MinAdc",       7, '8',  0x0, 14, 0, 100);    // Chan7 Minimum ADC
    createChanConfigParam("Ch7:MaxAdc",       7, '8',  0x1, 14, 0, 1000);   // Chan7 Maximum ADC
    createChanConfigParam("Ch7:IntRelease",   7, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan7 Integrator release p
    createChanConfigParam("Ch7:MinDiscrimWidth",7,'8', 0x3,  9, 0, 0);      // Chan7 min pulse width
    createChanConfigParam("Ch7:VetoInhibit",  7, '8',  0x4, 12, 0, 10);     // Chan7 veto inhibit point
    createChanConfigParam("Ch7:NegDiscrTime", 7, '8',  0x5, 12, 0, 0);      // Chan7 negative discr timeout
    createChanConfigParam("Ch7:Sample1",      7, '8',  0x6,  9, 0, 0);      // Chan7 Sample 1 point [0:10]
    createChanConfigParam("Ch7:Sample2",      7, '8',  0x7,  9, 0, 15);     // Chan7 Sample 2 point [0:10]

    createChanConfigParam("Ch8:PositionIdx",  8, '1',  0x0, 32, 0, 7040);   // Chan8 position index
    createChanConfigParam("Ch8:A:InOffset",   8, '1',  0x2,  9, 0, 3, CONV_SIGN_MAGN); // Chan8 A input offset
    createChanConfigParam("Ch8:B:InOffset",   8, '1',  0x3,  9, 0, 5, CONV_SIGN_MAGN); // Chan8 B input offset
    createChanConfigParam("Ch8:A:FullScale",  8, '1',  0x4,  8, 0, 0);      // Chan8 A full scale
    createChanConfigParam("Ch8:Sum:FullScale",8, '1',  0x5,  8, 0, 0);      // Chan8 SUM full scale
    createChanConfigParam("Ch8:B:FullScale",  8, '1',  0x6,  8, 0, 0);      // Chan8 B full scale
    createChanConfigParam("Ch8:PosThreshold", 8, '1',  0x7,  8, 0, 122);    // Chan8 positive threshold
    createChanConfigParam("Ch8:NegThreshold", 8, '1',  0x8,  8, 0, 110);    // Chan8 negative threshold
    createChanConfigParam("Ch8:A:AdcOffset",  8, '1',  0x9,  8, 0, 61);     // Chan8 A ADC offset
    createChanConfigParam("Ch8:Sum:AdcOffset",8, '1',  0xA,  8, 0, 0);      // Chan8 sum zero
    createChanConfigParam("Ch8:B:AdcOffset",  8, '1',  0xB,  8, 0, 113);    // Chan8 B ADC offset
    createChanConfigParam("Ch8:A:GainAdj",    8, '1',  0xC,  8, 0, 0);      // Chan8 A gain adjust
    createChanConfigParam("Ch8:B:GainAdj",    8, '1',  0xD,  8, 0, 0);      // Chan8 B gain adjust
    createChanConfigParam("Ch8:A:AvgMin",     8, '1',  0xE, 16, 0, 1541);   // Chan8 A average minimum
    createChanConfigParam("Ch8:B:AvgMin",     8, '1',  0xF, 16, 0, 1541);   // Chan8 B average minimum
    createChanConfigParam("Ch8:A:AvgMax",     8, '1', 0x10, 16, 0, 1373);   // Chan8 A average maximum
    createChanConfigParam("Ch8:B:AvgMax",     8, '1', 0x11, 16, 0, 1373);   // Chan8 B average maximum
    createChanConfigParam("Ch8:MaximumSlope", 8, '1', 0x12, 16, 0, 0);      // Chan8 Maximum slope
    createChanConfigParam("Ch8:MinAdc",       8, '8',  0x0, 14, 0, 100);    // Chan8 Minimum ADC
    createChanConfigParam("Ch8:MaxAdc",       8, '8',  0x1, 14, 0, 1000);   // Chan8 Maximum ADC
    createChanConfigParam("Ch8:IntRelease",   8, '8',  0x2,  9, 0, -6, CONV_SIGN_2COMP);    // Chan8 Integrator release p
    createChanConfigParam("Ch8:MinDiscrimWidth",8,'8', 0x3,  9, 0, 0);      // Chan8 min pulse width
    createChanConfigParam("Ch8:VetoInhibit",  8, '8',  0x4, 12, 0, 10);     // Chan8 veto inhibit point
    createChanConfigParam("Ch8:NegDiscrTime", 8, '8',  0x5, 12, 0, 0);      // Chan8 negative discr timeout
    createChanConfigParam("Ch8:Sample1",      8, '8',  0x6,  9, 0, 0);      // Chan8 Sample 1 point [0:10]
    createChanConfigParam("Ch8:Sample2",      8, '8',  0x7,  9, 0, 15);     // Chan8 Sample 2 point [0:10]

//  BLXXX:Det:RocXXX:| parameter name |                 | EPICS record description  | (bi and mbbi description)
    createTempParam("TempBoard",        0x0, 16, 0, CONV_SIGN_2COMP); // ROC board temperature in degC   (calc:0.25*A,unit:C,prec:1,low:-50,high:50, archive:monitor)
    createTempParam("TempPreampA",      0x1, 16, 0, CONV_SIGN_2COMP); // Preamp A temperature in degC    (calc:0.25*A,unit:C,prec:1,low:-50,high:50, archive:monitor)
    createTempParam("TempPreampB",      0x2, 16, 0, CONV_SIGN_2COMP); // Preamp B temperature in degC    (calc:0.25*A,unit:C,prec:1,low:-50,high:50, archive:monitor)
}
