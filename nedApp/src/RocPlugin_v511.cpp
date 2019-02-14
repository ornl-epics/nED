/* RocPlugin_v511.cpp
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "RocPlugin.h"

/**
 * @file RocPlugin_v511.cpp
 *
 * ROC 5.11 parameters
 *
 * The ROC 5.11 firmware is same as 5.10 except for CORELLI type of ROC
 */

void RocPlugin::createParams_v511()
{
    createRegParam("VERSION", "HwRev",  true, 0,  8, 0);   // Hardware revision
    createRegParam("VERSION", "HwVer",  true, 0,  8, 8);   // Hardware version
    createRegParam("VERSION", "FwRev",  true, 1,  8, 0);   // Firmware revision        (low:10,high:12)
    createRegParam("VERSION", "FwVer",  true, 1,  8, 8);   // Firmware version         (low:4,high:6)
    createRegParam("VERSION", "FwYear", true, 2, 16, 0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwDay",  true, 3,  8, 0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwMonth",true, 3,  8, 8, 0, CONV_HEX2DEC);

//    BLXXX:Det:RocXXX:| sig nam|                              | EPICS record description | (bi and mbbi description)
    createStatusParam("ErrUartByte",          0x0,  1, 13); // UART: Byte error             (0=no error,1=error)
    createStatusParam("ErrUartParity",        0x0,  1, 12); // UART: Parity error           (0=no error,1=error)
    createStatusParam("ErrProgramming",       0x0,  1, 11); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("ErrCmdLength",         0x0,  1, 10); // Command length error         (0=no error,1=error)
    createStatusParam("ErrCmdBad",            0x0,  1,  9); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrNoTsync",           0x0,  1,  8); // Timestamp overflow error.    (0=no error,1=error)
    createStatusParam("ErrFifoFull",          0x0,  1,  6); // LVDS FIFO went full.         (0=not full,1=full)
    createStatusParam("ErrPreStart",          0x0,  1,  5); // LVDS start before stop bit   (0=no error,1=error)
    createStatusParam("ErrNoStart",           0x0,  1,  4); // LVDS data without start.     (0=no error,1=error)
    createStatusParam("ErrTimeout",           0x0,  1,  3); // LVDS packet timeout.         (0=no timeout,1=timeout)
    createStatusParam("ErrLength",            0x0,  1,  2); // LVDS packet length error.    (0=no error,1=error)
    createStatusParam("ErrType",              0x0,  1,  1); // LVDS packet type error.      (0=no error,1=error)
    createStatusParam("ErrParity",            0x0,  1,  0); // LVDS parity error.           (0=no error,1=error)

    createStatusParam("LvdsVerify",           0x1,  1, 14); // LVDS VERIFY detected         (0=not detected,1=detected)
    createStatusParam("IntFifoFull",          0x1,  1, 13); // Internal Data FIFO Almost fu (0=not full,1=full)
    createStatusParam("IntFifoEmpty",         0x1,  1, 12); // Internal Data FIFO Empty flg (0=not empty,1=empty)
    createStatusParam("CalcBadFinal",         0x1,  1, 11); // Calc: Bad Final Calculation  (0=no error,1=error)
    createStatusParam("CalcBadEffect",        0x1,  1, 10); // Calc: Bad Effective Calculat (0=no error,1=error)
    createStatusParam("CalcBadOverlow",       0x1,  1,  9); // Calc: Data overflow detected (0=no error,1=error)
    createStatusParam("CalcBadCount",         0x1,  1,  8); // Calc: Bad word count.        (0=no error,1=error)
    createStatusParam("DataFifoAlmostFull",   0x1,  1,  7); // Data FIFO Almost full flag.  (0=not full,1=almost full)
    createStatusParam("DataFifoEmpty",        0x1,  1,  6); // Data FIFO Empty flag.        (0=not empty,1=empty)
    createStatusParam("HVStatus",             0x1,  1,  5); // High Voltage Status bit      (0=not present,1=present)
    createStatusParam("CalcActive",           0x1,  1,  4); // Calculation: Active          (0=not active,1=active)
    createStatusParam("Acquiring",            0x1,  1,  3); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("Discovered",           0x1,  1,  2); // Discovered                   (0=not discovered,1=discovered, archive:monitor)
    createStatusParam("Configured",           0x1,  2,  0); // Configured                   (0=not configured [alarm],1=section 1 conf'd,2=section 2 conf'd,3=all conf'd, archive:monitor)

    createStatusParam("RiseEdgeA",            0x2,  8,  8); // Discriminator A set
    createStatusParam("SysrstBHigh",          0x2,  1,  7); // SYSRST_B Got HIGH            (0=no,1=yes)
    createStatusParam("SysrstBLow",           0x2,  1,  6); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("TxenBHigh",            0x2,  1,  5); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("TxenBLow",             0x2,  1,  4); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("TsyncHigh",            0x2,  1,  3); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("TsyncLow",             0x2,  1,  2); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TclkHigh",             0x2,  1,  1); // TCLK Got HIGH                (0=no,1=yes)
    createStatusParam("TclkLow",              0x2,  1,  0); // TCLK Got LOW                 (0=no,1=yes)

    createStatusParam("RiseEdgeSum",          0x3,  8,  8); // Discriminator SUM set
    createStatusParam("RiseEdgeB",            0x3,  8,  0); // Discriminator B set

    createStatusParam("Ch1:A:AutoAdjEn",      0x4,  1, 15); // Chan1 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch1:A:AutoAdjTrig",    0x4,  1, 14); // Chan1 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch1:A:AutoAdjSample",  0x4,  1, 13); // Chan1 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch1:A:AutoAdjSlope",   0x4,  1, 12); // Chan1 A Auto-Adjust Slope l  (0=not active,1=active)
    createStatusParam("Ch1:A:AutoAdjOffset",  0x4,  1, 11); // Chan1 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch1:A:AutoAdjSlope1",  0x4,  1, 10); // Chan1 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch1:A:AutoAdjOverflow",0x4,  1,  9); // Chan1 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch1:A:InputOffset",    0x4,  9,  0); // Chan1 A Input Offset value

    createStatusParam("Ch1:B:AdcOffsetVal",   0x5,  8,  8); // Chan1 B ADC Offset value
    createStatusParam("Ch1:A:AdcOffsetVal",   0x5,  8,  0); // Chan1 A ADC Offset value

    createStatusParam("Ch1:B:AutoAdjEn",      0x6,  1, 15); // Chan1 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch1:B:AutoAdjTrig",    0x6,  1, 14); // Chan1 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch1:B:AutoAdjSample",  0x6,  1, 13); // Chan1 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch1:B:AutoAdjSlope",   0x6,  1, 12); // Chan1 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch1:B:AutoAdjOffset",  0x6,  1, 11); // Chan1 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch1:B:AutoAdjSlope1",  0x6,  1, 10); // Chan1 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch1:B:AutoAdjOverflow",0x6,  1,  9); // Chan1 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch1:B:InputOffset",    0x6,  9,  0); // Chan1 B Input Offset value

    // NOTE: The next parameter spans over the LVDS words (16bits) *and* it also spans over the
    //       OCC dword (32 bits). BaseModulePlugin::createStatusParam() and BaseModulePlugin:rspReadStatus()
    //       functions are smart enough to accomodate the behaviour.
    createStatusParam("Ch2:A:InputOffset",    0x7,  9,  8); // Chan2 A input offset value
    createStatusParam("Ch1:AdcMax",           0x7,  1,  7); // Chan1 got ADC max            (0=no,1=yes)
    createStatusParam("Ch1:AdcMin",           0x7,  1,  6); // Chan1 got ADC min            (0=no,1=yes)
    createStatusParam("Ch1:MultiDiscEvent",   0x7,  1,  5); // Chan1 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch1:Event",            0x7,  1,  4); // Chan1 got event              (0=no,1=yes)
    createStatusParam("Ch1:FifoFull",         0x7,  1,  3); // Chan1 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch1:FifoAlmostFullD",  0x7,  1,  2); // Chan1 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch1:FifoAlmostFull",   0x7,  1,  1); // Chan1 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch1:HasData",          0x7,  1,  0); // Chan1 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch2:A:AdcOffsetVal",   0x8,  8,  8); // Chan2 A ADC Offset value
    createStatusParam("Ch2:A:AutoAdjEn",      0x8,  1,  7); // Chan2 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch2:A:AutoAdjTrig",    0x8,  1,  6); // Chan2 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch2:A:AutoAdjSample",  0x8,  1,  5); // Chan2 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch2:A:AutoAdjSlope",   0x8,  1,  4); // Chan2 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch2:A:AutoAdjOffset",  0x8,  1,  3); // Chan2 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch2:A:AutoAdjSlope1",  0x8,  1,  2); // Chan2 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch2:A:AutoAdjOverflow",0x8,  1,  1); // Chan2 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch2:B:InputOffset",    0x9,  9,  8); // Chan2 B input offset value
    createStatusParam("Ch2:B:AdcOffsetVal",   0x9,  8,  0); // Chan2 B ADC Offset value

    createStatusParam("Ch2:AdcMax",           0xA,  1, 15); // Chan2 got ADC max            (0=no,1=yes)
    createStatusParam("Ch2:AdcMin",           0xA,  1, 14); // Chan2 got ADC min            (0=no,1=yes)
    createStatusParam("Ch2:MultiDiscEvent",   0xA,  1, 13); // Chan2 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch2:Event",            0xA,  1, 12); // Chan2 got event              (0=no,1=yes)
    createStatusParam("Ch2:FifoFull",         0xA,  1, 11); // Chan2 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch2:FifoAlmostFullD",  0xA,  1, 10); // Chan2 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch2:FifoAlmostFull",   0xA,  1,  9); // Chan2 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch2:HasData",          0xA,  1,  8); // Chan2 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch2:B:AutoAdjEn",      0xA,  1,  7); // Chan2 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch2:B:AutoAdjTrig",    0xA,  1,  6); // Chan2 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch2:B:AutoAdjSample",  0xA,  1,  5); // Chan2 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch2:B:AutoAdjSlope",   0xA,  1,  4); // Chan2 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch2:B:AutoAdjOffset",  0xA,  1,  3); // Chan2 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch2:B:AutoAdjSlope1",  0xA,  1,  2); // Chan2 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch2:B:AutoAdjOverflow",0xA,  1,  1); // Chan2 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch3:A:AutoAdjEn",      0xB,  1, 15); // Chan3 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch3:A:AutoAdjTrig",    0xB,  1, 14); // Chan3 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch3:A:AutoAdjSample",  0xB,  1, 13); // Chan3 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch3:A:AutoAdjSlope",   0xB,  1, 12); // Chan3 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch3:A:AutoAdjOffset",  0xB,  1, 11); // Chan3 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch3:A:AutoAdjSlope1",  0xB,  1, 10); // Chan3 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch3:A:AutoAdjOverflow",0xB,  1,  9); // Chan3 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch3:A:InputOffset",    0xB,  9,  0); // Chan3 A input offset value

    createStatusParam("Ch3:A:AdcOffsetVal",   0xC,  8,  0); // Chan3 A ADC Offset value
    createStatusParam("Ch3:B:AdcOffsetVal",   0xC,  8,  8); // Chan3 B ADC Offset value

    createStatusParam("Ch3:B:AutoAdjEn",      0xD,  1, 15); // Chan3 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch3:B:AutoAdjTrig",    0xD,  1, 14); // Chan3 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch3:B:AutoAdjSample",  0xD,  1, 13); // Chan3 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch3:B:AutoAdjSlope",   0xD,  1, 12); // Chan3 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch3:B:AutoAdjOffset",  0xD,  1, 11); // Chan3 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch3:B:AutoAdjSlope1",  0xD,  1, 10); // Chan3 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch3:B:AutoAdjOverflow",0xD,  1,  9); // Chan3 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch3:B:InputOffset",    0xD,  9,  0); // Chan3 B input offset value

    createStatusParam("Ch4:A:InputOffset",    0xE,  9,  8); // Chan4 A input offset value
    createStatusParam("Ch3:AdcMax",           0xE,  1,  7); // Chan3 got ADC max            (0=no,1=yes)
    createStatusParam("Ch3:AdcMin",           0xE,  1,  6); // Chan3 got ADC min            (0=no,1=yes)
    createStatusParam("Ch3:MultiDiscEvent",   0xE,  1,  5); // Chan3 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch3:Event",            0xE,  1,  4); // Chan3 got event              (0=no,1=yes)
    createStatusParam("Ch3:FifoFull",         0xE,  1,  3); // Chan3 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch3:FifoAlmostFullD",  0xE,  1,  2); // Chan3 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch3:FifoAlmostFull",   0xE,  1,  1); // Chan3 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch3:HasData",          0xE,  1,  0); // Chan3 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch4:A:AdcOffsetVal",   0xF,  8,  8); // Chan4 A ADC Offset value
    createStatusParam("Ch4:A:AutoAdjEn",      0xF,  1,  7); // Chan4 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch4:A:AutoAdjTrig",    0xF,  1,  6); // Chan4 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch4:A:AutoAdjSample",  0xF,  1,  5); // Chan4 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch4:A:AutoAdjSlope",   0xF,  1,  4); // Chan4 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch4:A:AutoAdjOffset",  0xF,  1,  3); // Chan4 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch4:A:AutoAdjSlope1",  0xF,  1,  2); // Chan4 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch4:A:AutoAdjOverflow",0xF,  1,  1); // Chan4 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch4:B:InputOffset",    0x10, 9,  8); // Chan4 B input offset value
    createStatusParam("Ch4:B:AdcOffsetVal",   0x10, 8,  0); // Chan4 B ADC Offset value

    createStatusParam("Ch4:AdcMax",           0x11, 1, 15); // Chan4 got ADC max            (0=no,1=yes)
    createStatusParam("Ch4:AdcMin",           0x11, 1, 14); // Chan4 got ADC min            (0=no,1=yes)
    createStatusParam("Ch4:MultiDiscEvent",   0x11, 1, 13); // Chan4 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch4:Event",            0x11, 1, 12); // Chan4 got event              (0=no,1=yes)
    createStatusParam("Ch4:FifoFull",         0x11, 1, 11); // Chan4 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch4:FifoAlmostFullD",  0x11, 1, 10); // Chan4 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch4:FifoAlmostFull",   0x11, 1,  9); // Chan4 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch4:HasData",          0x11, 1,  8); // Chan4 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch4:B:AutoAdjEn",      0x11, 1,  7); // Chan4 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch4:B:AutoAdjTrig",    0x11, 1,  6); // Chan4 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch4:B:AutoAdjSample",  0x11, 1,  5); // Chan4 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch4:B:AutoAdjSlope",   0x11, 1,  4); // Chan4 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch4:B:AutoAdjOffset",  0x11, 1,  3); // Chan4 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch4:B:AutoAdjSlope1",  0x11, 1,  2); // Chan4 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch4:B:AutoAdjOverflow",0x11, 1,  1); // Chan4 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch5:A:AutoAdjEn",      0x12, 1, 15); // Chan5 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch5:A:AutoAdjTrig",    0x12, 1, 14); // Chan5 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch5:A:AutoAdjSample",  0x12, 1, 13); // Chan5 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch5:A:AutoAdjSlope",   0x12, 1, 12); // Chan5 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch5:A:AutoAdjOffset",  0x12, 1, 11); // Chan5 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch5:A:AutoAdjSlope1",  0x12, 1, 10); // Chan5 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch5:A:AutoAdjOverflow",0x12, 1,  9); // Chan5 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch5:A:InputOffset",    0x12, 9,  0); // Chan5 A Input Offset value

    createStatusParam("Ch5:B:AdcOffsetVal",   0x13, 8,  8); // Chan5 B ADC Offset value
    createStatusParam("Ch5:A:AdcOffsetVal",   0x13, 8,  0); // Chan5 A ADC Offset value

    createStatusParam("Ch5:B:AutoAdjEn",      0x14, 1, 15); // Chan5 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch5:B:AutoAdjTrig",    0x14, 1, 14); // Chan5 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch5:B:AutoAdjSample",  0x14, 1, 13); // Chan5 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch5:B:AutoAdjSlope",   0x14, 1, 12); // Chan5 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch5:B:AutoAdjOffset",  0x14, 1, 11); // Chan5 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch5:B:AutoAdjSlope1",  0x14, 1, 10); // Chan5 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch5:B:AutoAdjOverflow",0x14, 1,  9); // Chan5 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch5:B:InputOffset",    0x14, 9,  0); // Chan5 B Input Offset value

    createStatusParam("Ch6:A:InputOffset",    0x15, 9,  8); // Chan6 A input offset value
    createStatusParam("Ch5:AdcMax",           0x15, 1,  7); // Chan5 got ADC max            (0=no,1=yes)
    createStatusParam("Ch5:AdcMin",           0x15, 1,  6); // Chan5 got ADC min            (0=no,1=yes)
    createStatusParam("Ch5:MultiDiscEvent",   0x15, 1,  5); // Chan5 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch5:Event",            0x15, 1,  4); // Chan5 got event              (0=no,1=yes)
    createStatusParam("Ch5:FifoFull",         0x15, 1,  3); // Chan5 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch5:FifoAlmostFullD",  0x15, 1,  2); // Chan5 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch5:FifoAlmostFull",   0x15, 1,  1); // Chan5 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch5:HasData",          0x15, 1,  0); // Chan5 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch6:A:AdcOffsetVal",   0x16, 8,  8); // Chan6 A ADC Offset value
    createStatusParam("Ch6:A:AutoAdjEn",      0x16, 1,  7); // Chan6 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch6:A:AutoAdjTrig",    0x16, 1,  6); // Chan6 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch6:A:AutoAdjSample",  0x16, 1,  5); // Chan6 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch6:A:AutoAdjSlope",   0x16, 1,  4); // Chan6 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch6:A:AutoAdjOffset",  0x16, 1,  3); // Chan6 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch6:A:AutoAdjSlope1",  0x16, 1,  2); // Chan6 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch6:A:AutoAdjOverflow",0x16, 1,  1); // Chan6 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch6:B:InputOffset",    0x17, 9,  8); // Chan6 B input offset value
    createStatusParam("Ch6:B:AdcOffsetVal",   0x17, 8,  0); // Chan6 B ADC Offset value

    createStatusParam("Ch6:AdcMax",           0x18, 1, 15); // Chan6 got ADC max            (0=no,1=yes)
    createStatusParam("Ch6:AdcMin",           0x18, 1, 14); // Chan6 got ADC min            (0=no,1=yes)
    createStatusParam("Ch6:MultiDiscEvent",   0x18, 1, 13); // Chan6 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch6:Event",            0x18, 1, 12); // Chan6 got event              (0=no,1=yes)
    createStatusParam("Ch6:FifoFull",         0x18, 1, 11); // Chan6 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch6:FifoAlmostFullD",  0x18, 1, 10); // Chan6 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch6:FifoAlmostFull",   0x18, 1,  9); // Chan6 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch6:HasData",          0x18, 1,  8); // Chan6 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch6:B:AutoAdjEn",      0x18, 1,  7); // Chan6 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch6:B:AutoAdjTrig",    0x18, 1,  6); // Chan6 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch6:B:AutoAdjSample",  0x18, 1,  5); // Chan6 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch6:B:AutoAdjSlope",   0x18, 1,  4); // Chan6 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch6:B:AutoAdjOffset",  0x18, 1,  3); // Chan6 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch6:B:AutoAdjSlope1",  0x18, 1,  2); // Chan6 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch6:B:AutoAdjOverflow",0x18, 1,  1); // Chan6 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch7:A:AutoAdjEn",      0x19, 1, 15); // Chan7 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch7:A:AutoAdjTrig",    0x19, 1, 14); // Chan7 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch7:A:AutoAdjSample",  0x19, 1, 13); // Chan7 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch7:A:AutoAdjSlope",   0x19, 1, 12); // Chan7 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch7:A:AutoAdjOffset",  0x19, 1, 11); // Chan7 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch7:A:AutoAdjSlope1",  0x19, 1, 10); // Chan7 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch7:A:AutoAdjOverflow",0x19, 1,  9); // Chan7 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch7:A:InputOffset",    0x19, 9,  0); // Chan7 A input offset value

    createStatusParam("Ch7:A:AdcOffsetVal",   0x1A, 8,  0); // Chan7 A ADC Offset value
    createStatusParam("Ch7:B:AdcOffsetVal",   0x1A, 8,  8); // Chan7 B ADC Offset value

    createStatusParam("Ch7:B:AutoAdjEn",      0x1B, 1, 15); // Chan7 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch7:B:AutoAdjTrig",    0x1B, 1, 14); // Chan7 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch7:B:AutoAdjSample",  0x1B, 1, 13); // Chan7 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch7:B:AutoAdjSlope",   0x1B, 1, 12); // Chan7 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch7:B:AutoAdjOffset",  0x1B, 1, 11); // Chan7 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch7:B:AutoAdjSlope1",  0x1B, 1, 10); // Chan7 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch7:B:AutoAdjOverflow",0x1B, 1,  9); // Chan7 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch7:B:InputOffset",    0x1B, 9,  0); // Chan7 B input offset value

    createStatusParam("Ch8:A:InputOffset",    0x1C, 9,  8); // Chan8 A input offset value
    createStatusParam("Ch7:AdcMax",           0x1C, 1,  7); // Chan7 got ADC max            (0=no,1=yes)
    createStatusParam("Ch7:AdcMin",           0x1C, 1,  6); // Chan7 got ADC min            (0=no,1=yes)
    createStatusParam("Ch7:MultiDiscEvent",   0x1C, 1,  5); // Chan7 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch7:Event",            0x1C, 1,  4); // Chan7 got event              (0=no,1=yes)
    createStatusParam("Ch7:FifoFull",         0x1C, 1,  3); // Chan7 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch7:FifoAlmostFullD",  0x1C, 1,  2); // Chan7 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch7:FifoAlmostFull",   0x1C, 1,  1); // Chan7 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch7:HasData",          0x1C, 1,  0); // Chan7 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch8:A:AdcOffsetVal",   0x1D, 8,  8); // Chan8 A ADC Offset value
    createStatusParam("Ch8:A:AutoAdjEn",      0x1D, 1,  7); // Chan8 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch8:A:AutoAdjTrig",    0x1D, 1,  6); // Chan8 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch8:A:AutoAdjSample",  0x1D, 1,  5); // Chan8 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch8:A:AutoAdjSlope",   0x1D, 1,  4); // Chan8 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch8:A:AutoAdjOffset",  0x1D, 1,  3); // Chan8 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch8:A:AutoAdjSlope1",  0x1D, 1,  2); // Chan8 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch8:A:AutoAdjOverflow",0x1D, 1,  1); // Chan8 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch8:B:InputOffset",    0x1E, 9,  8); // Chan8 B input offset value
    createStatusParam("Ch8:B:AdcOffsetVal",   0x1E, 8,  0); // Chan8 B ADC Offset value

    createStatusParam("Ch8:AdcMax",           0x1F, 1, 15); // Chan8 got ADC max            (0=no,1=yes)
    createStatusParam("Ch8:AdcMin",           0x1F, 1, 14); // Chan8 got ADC min            (0=no,1=yes)
    createStatusParam("Ch8:MultiDiscEvent",   0x1F, 1, 13); // Chan8 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch8:Event",            0x1F, 1, 12); // Chan8 got event              (0=no,1=yes)
    createStatusParam("Ch8:FifoFull",         0x1F, 1, 11); // Chan8 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch8:FifoAlmostFullD",  0x1F, 1, 10); // Chan8 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch8:FifoAlmostFull",   0x1F, 1,  9); // Chan8 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch8:HasData",          0x1F, 1,  8); // Chan8 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch8:B:AutoAdjEn",      0x1F, 1,  7); // Chan8 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch8:B:AutoAdjTrig",    0x1F, 1,  6); // Chan8 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch8:B:AutoAdjSample",  0x1F, 1,  5); // Chan8 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch8:B:AutoAdjSlope",   0x1F, 1,  4); // Chan8 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch8:B:AutoAdjOffset",  0x1F, 1,  3); // Chan8 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch8:B:AutoAdjSlope1",  0x1F, 1,  2); // Chan8 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch8:B:AutoAdjOverflow",0x1F, 1,  1); // Chan8 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("PreampA:Family",       0x20, 8,  0); // Pre-amp A family
    createStatusParam("PreampA:Mac1",         0x20, 8,  8); // Pre-amp A MAC byte 1
    createStatusParam("PreampA:Mac2",         0x21, 8,  0); // Pre-amp A MAC byte 2
    createStatusParam("PreampA:Mac3",         0x21, 8,  8); // Pre-amp A MAC byte 3
    createStatusParam("PreampA:Mac4",         0x22, 8,  0); // Pre-amp A MAC byte 4
    createStatusParam("PreampA:Mac5",         0x22, 8,  8); // Pre-amp A MAC byte 5
    createStatusParam("PreampA:Mac6",         0x23, 8,  0); // Pre-amp A MAC byte 6
    createStatusParam("PreampA:Crc",          0x23, 8,  8); // Pre-amp A CRC
    createStatusParam("PreampA:Config",       0x24, 8,  0); // Pre-amp A configuration
    createStatusParam("PreampB:Family",       0x24, 8,  8); // Pre-amp B family
    createStatusParam("PreampB:Mac1",         0x25, 8,  0); // Pre-amp B MAC byte 1
    createStatusParam("PreampB:Mac2",         0x25, 8,  8); // Pre-amp B MAC byte 2
    createStatusParam("PreampB:Mac3",         0x26, 8,  0); // Pre-amp B MAC byte 3
    createStatusParam("PreampB:Mac4",         0x26, 8,  8); // Pre-amp B MAC byte 4
    createStatusParam("PreampB:Mac5",         0x27, 8,  0); // Pre-amp B MAC byte 5
    createStatusParam("PreampB:Mac6",         0x27, 8,  8); // Pre-amp B MAC byte 6
    createStatusParam("PreampB:Crc",          0x28, 8,  0); // Pre-amp B CRC
    createStatusParam("PreampB:Config",       0x28, 8,  8); // Pre-amp B configuration

//     BLXXX:Det:RocXXX:| sig name       |                    | EPICS record description  | (bi and mbbi description)
    createCounterParam("CntParity",          0x0, 16,  0); // LVDS parity error counter
    createCounterParam("CntType",            0x1, 16,  0); // LVDS data type error counter
    createCounterParam("CntLength",          0x2, 16,  0); // LVDS length error counter
    createCounterParam("CntTimeout",         0x3, 16,  0); // LVDS timeout counter
    createCounterParam("CntNoStart",         0x4, 16,  0); // LVDS no start error counter
    createCounterParam("CntPreStart",        0x5, 16,  0); // LVDS start before stop cnt
    createCounterParam("CntFifoFull",        0x6, 16,  0); // LVDS FIFO full error counter
    createCounterParam("CntNoTsync",         0x7, 16,  0); // Timestamp overflow counter
    createCounterParam("CntCmdBad",          0x8, 16,  0); // Unknown command counter
    createCounterParam("CntCmdLength",       0x9, 16,  0); // Command length error counter
    createCounterParam("CntProgramming",     0xA, 16,  0); // Write cnfg disallowed cnt
    createCounterParam("CntUartParity",      0xB, 16,  0); // UART parity error counter
    createCounterParam("CntUartByte",        0xC, 16,  0); // UART byte error counter
    createCounterParam("CntDataAlmostFull" , 0xD, 16,  0); // Data almost full counter
    createCounterParam("Ch1:CntFifoFull",    0xE, 16,  0); // Ch1 ADC FIFO full counter
    createCounterParam("Ch2:CntFifoFull",    0xF, 16,  0); // Ch2 ADC FIFO full counter
    createCounterParam("Ch3:CntFifoFull",   0x10, 16,  0); // Ch3 ADC FIFO full counter
    createCounterParam("Ch4:CntFifoFull",   0x11, 16,  0); // Ch4 ADC FIFO full counter
    createCounterParam("Ch5:CntFifoFull",   0x12, 16,  0); // Ch5 ADC FIFO full counter
    createCounterParam("Ch6:CntFifoFull",   0x13, 16,  0); // Ch6 ADC FIFO full counter
    createCounterParam("Ch7:CntFifoFull",   0x14, 16,  0); // Ch7 ADC FIFO full counter
    createCounterParam("Ch8:CntFifoFull",   0x15, 16,  0); // Ch8 ADC FIFO full counter
    createCounterParam("CntMissClk",        0x16, 16,  0); // Link RX clock missing cnt
    createCounterParam("Ch1:RatePosEdge",   0x17, 16,  0); // Ch1 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch2:RatePosEdge",   0x18, 16,  0); // Ch2 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch3:RatePosEdge",   0x19, 16,  0); // Ch3 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch4:RatePosEdge",   0x1A, 16,  0); // Ch4 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch5:RatePosEdge",   0x1B, 16,  0); // Ch5 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch6:RatePosEdge",   0x1C, 16,  0); // Ch6 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch7:RatePosEdge",   0x1D, 16,  0); // Ch7 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch8:RatePosEdge",   0x1E, 16,  0); // Ch8 positive edge rate      (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch1:RateSumHigh",   0x1F, 16,  0); // Ch1 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch2:RateSumHigh",   0x20, 16,  0); // Ch2 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch3:RateSumHigh",   0x21, 16,  0); // Ch3 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch4:RateSumHigh",   0x22, 16,  0); // Ch4 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch5:RateSumHigh",   0x23, 16,  0); // Ch5 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch6:RateSumHigh",   0x24, 16,  0); // Ch6 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch7:RateSumHigh",   0x25, 16,  0); // Ch7 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch8:RateSumHigh",   0x26, 16,  0); // Ch8 SUM high rate           (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch1:RateSumLow",    0x27, 16,  0); // Ch1 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch2:RateSumLow",    0x28, 16,  0); // Ch2 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch3:RateSumLow",    0x29, 16,  0); // Ch3 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch4:RateSumLow",    0x2A, 16,  0); // Ch4 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch5:RateSumLow",    0x2B, 16,  0); // Ch5 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch6:RateSumLow",    0x2C, 16,  0); // Ch6 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch7:RateSumLow",    0x2D, 16,  0); // Ch7 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch8:RateSumLow",    0x2E, 16,  0); // Ch8 SUM low rate            (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch1:RateOut",       0x2F, 16,  0); // Ch1 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch2:RateOut",       0x30, 16,  0); // Ch2 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch3:RateOut",       0x31, 16,  0); // Ch3 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch4:RateOut",       0x32, 16,  0); // Ch4 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch5:RateOut",       0x33, 16,  0); // Ch5 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch6:RateOut",       0x34, 16,  0); // Ch6 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch7:RateOut",       0x35, 16,  0); // Ch7 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch8:RateOut",       0x36, 16,  0); // Ch8 outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("RateOut",           0x37, 16,  0); // Total outrate               (scale:19.0735,unit:cnts/s,prec:1)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:PositionIdx",  '1', 0x0,  32, 0, 0);     // Chan1 position index
    createConfigParam("Ch2:PositionIdx",  '1', 0x2,  32, 0, 256);   // Chan2 position index
    createConfigParam("Ch3:PositionIdx",  '1', 0x4,  32, 0, 512);   // Chan3 position index
    createConfigParam("Ch4:PositionIdx",  '1', 0x6,  32, 0, 768);   // Chan4 position index
    createConfigParam("Ch5:PositionIdx",  '1', 0x8,  32, 0, 1024);  // Chan5 position index
    createConfigParam("Ch6:PositionIdx",  '1', 0xA,  32, 0, 1280);  // Chan6 position index
    createConfigParam("Ch7:PositionIdx",  '1', 0xC,  32, 0, 1536);  // Chan7 position index
    createConfigParam("Ch8:PositionIdx",  '1', 0xE,  32, 0, 1792);  // Chan8 position index

    createConfigParam("Ch1:A:InOffset",   '2', 0x0,   9, 0, 100, CONV_SIGN_MAGN); // Chan1 A input offset
    createConfigParam("Ch2:A:InOffset",   '2', 0x1,   9, 0, 100, CONV_SIGN_MAGN); // Chan2 A input offset
    createConfigParam("Ch3:A:InOffset",   '2', 0x2,   9, 0, 100, CONV_SIGN_MAGN); // Chan3 A input offset
    createConfigParam("Ch4:A:InOffset",   '2', 0x3,   9, 0, 100, CONV_SIGN_MAGN); // Chan4 A input offset
    createConfigParam("Ch5:A:InOffset",   '2', 0x4,   9, 0, 100, CONV_SIGN_MAGN); // Chan5 A input offset
    createConfigParam("Ch6:A:InOffset",   '2', 0x5,   9, 0, 100, CONV_SIGN_MAGN); // Chan6 A input offset
    createConfigParam("Ch7:A:InOffset",   '2', 0x6,   9, 0, 100, CONV_SIGN_MAGN); // Chan7 A input offset
    createConfigParam("Ch8:A:InOffset",   '2', 0x7,   9, 0, 100, CONV_SIGN_MAGN); // Chan8 A input offset
    createConfigParam("Ch1:B:InOffset",   '2', 0x8,   9, 0, 100, CONV_SIGN_MAGN); // Chan1 B input offset
    createConfigParam("Ch2:B:InOffset",   '2', 0x9,   9, 0, 100, CONV_SIGN_MAGN); // Chan2 B input offset
    createConfigParam("Ch3:B:InOffset",   '2', 0xA,   9, 0, 100, CONV_SIGN_MAGN); // Chan3 B input offset
    createConfigParam("Ch4:B:InOffset",   '2', 0xB,   9, 0, 100, CONV_SIGN_MAGN); // Chan4 B input offset
    createConfigParam("Ch5:B:InOffset",   '2', 0xC,   9, 0, 100, CONV_SIGN_MAGN); // Chan5 B input offset
    createConfigParam("Ch6:B:InOffset",   '2', 0xD,   9, 0, 100, CONV_SIGN_MAGN); // Chan6 B input offset
    createConfigParam("Ch7:B:InOffset",   '2', 0xE,   9, 0, 100, CONV_SIGN_MAGN); // Chan7 B input offset
    createConfigParam("Ch8:B:InOffset",   '2', 0xF,   9, 0, 100, CONV_SIGN_MAGN); // Chan8 B input offset

    createConfigParam("Ch1:A:AdcOffset",  '2', 0x10, 12, 0, 100);   // Chan1 A ADC offset
    createConfigParam("Ch2:A:AdcOffset",  '2', 0x11, 12, 0, 100);   // Chan2 A ADC offset
    createConfigParam("Ch3:A:AdcOffset",  '2', 0x12, 12, 0, 100);   // Chan3 A ADC offset
    createConfigParam("Ch4:A:AdcOffset",  '2', 0x13, 12, 0, 100);   // Chan4 A ADC offset
    createConfigParam("Ch5:A:AdcOffset",  '2', 0x14, 12, 0, 100);   // Chan5 A ADC offset
    createConfigParam("Ch6:A:AdcOffset",  '2', 0x15, 12, 0, 100);   // Chan6 A ADC offset
    createConfigParam("Ch7:A:AdcOffset",  '2', 0x16, 12, 0, 100);   // Chan7 A ADC offset
    createConfigParam("Ch8:A:AdcOffset",  '2', 0x17, 12, 0, 100);   // Chan8 A ADC offset
    createConfigParam("Ch1:B:AdcOffset",  '2', 0x18, 12, 0, 100);   // Chan1 B ADC offset
    createConfigParam("Ch2:B:AdcOffset",  '2', 0x19, 12, 0, 100);   // Chan2 B ADC offset
    createConfigParam("Ch3:B:AdcOffset",  '2', 0x1A, 12, 0, 100);   // Chan3 B ADC offset
    createConfigParam("Ch4:B:AdcOffset",  '2', 0x1B, 12, 0, 100);   // Chan4 B ADC offset
    createConfigParam("Ch5:B:AdcOffset",  '2', 0x1C, 12, 0, 100);   // Chan5 B ADC offset
    createConfigParam("Ch6:B:AdcOffset",  '2', 0x1D, 12, 0, 100);   // Chan6 B ADC offset
    createConfigParam("Ch7:B:AdcOffset",  '2', 0x1E, 12, 0, 100);   // Chan7 B ADC offset
    createConfigParam("Ch8:B:AdcOffset",  '2', 0x1F, 12, 0, 100);   // Chan8 B ADC offset

    createConfigParam("Ch1:A:Threshold",  '2', 0x20, 12, 0, 400);   // Chan1 A threshold
    createConfigParam("Ch2:A:Threshold",  '2', 0x21, 12, 0, 400);   // Chan2 A threshold
    createConfigParam("Ch3:A:Threshold",  '2', 0x22, 12, 0, 400);   // Chan3 A threshold
    createConfigParam("Ch4:A:Threshold",  '2', 0x23, 12, 0, 400);   // Chan4 A threshold
    createConfigParam("Ch5:A:Threshold",  '2', 0x24, 12, 0, 400);   // Chan5 A threshold
    createConfigParam("Ch6:A:Threshold",  '2', 0x25, 12, 0, 400);   // Chan6 A threshold
    createConfigParam("Ch7:A:Threshold",  '2', 0x26, 12, 0, 400);   // Chan7 A threshold
    createConfigParam("Ch8:A:Threshold",  '2', 0x27, 12, 0, 400);   // Chan8 A threshold
    createConfigParam("Ch1:B:Threshold",  '2', 0x28, 12, 0, 400);   // Chan1 B threshold
    createConfigParam("Ch2:B:Threshold",  '2', 0x29, 12, 0, 400);   // Chan2 B threshold
    createConfigParam("Ch3:B:Threshold",  '2', 0x2A, 12, 0, 400);   // Chan3 B threshold
    createConfigParam("Ch4:B:Threshold",  '2', 0x2B, 12, 0, 400);   // Chan4 B threshold
    createConfigParam("Ch5:B:Threshold",  '2', 0x2C, 12, 0, 400);   // Chan5 B threshold
    createConfigParam("Ch6:B:Threshold",  '2', 0x2D, 12, 0, 400);   // Chan6 B threshold
    createConfigParam("Ch7:B:Threshold",  '2', 0x2E, 12, 0, 400);   // Chan7 B threshold
    createConfigParam("Ch8:B:Threshold",  '2', 0x2F, 12, 0, 400);   // Chan8 B threshold

    createConfigParam("Ch1:PosThreshold", '2', 0x30, 12, 0, 400);   // Chan1 positive threshold
    createConfigParam("Ch2:PosThreshold", '2', 0x31, 12, 0, 400);   // Chan2 positive threshold
    createConfigParam("Ch3:PosThreshold", '2', 0x32, 12, 0, 400);   // Chan3 positive threshold
    createConfigParam("Ch4:PosThreshold", '2', 0x33, 12, 0, 400);   // Chan4 positive threshold
    createConfigParam("Ch5:PosThreshold", '2', 0x34, 12, 0, 400);   // Chan5 positive threshold
    createConfigParam("Ch6:PosThreshold", '2', 0x35, 12, 0, 400);   // Chan6 positive threshold
    createConfigParam("Ch7:PosThreshold", '2', 0x36, 12, 0, 400);   // Chan7 positive threshold
    createConfigParam("Ch8:PosThreshold", '2', 0x37, 12, 0, 400);   // Chan8 positive threshold
    createConfigParam("Ch1:A:FullScale",  '2', 0x38, 12, 0, 10);    // Chan1 A full scale
    createConfigParam("Ch2:A:FullScale",  '2', 0x39, 12, 0, 10);    // Chan2 A full scale
    createConfigParam("Ch3:A:FullScale",  '2', 0x3A, 12, 0, 10);    // Chan3 A full scale
    createConfigParam("Ch4:A:FullScale",  '2', 0x3B, 12, 0, 10);    // Chan4 A full scale
    createConfigParam("Ch5:A:FullScale",  '2', 0x3C, 12, 0, 10);    // Chan5 A full scale
    createConfigParam("Ch6:A:FullScale",  '2', 0x3D, 12, 0, 10);    // Chan6 A full scale
    createConfigParam("Ch7:A:FullScale",  '2', 0x3E, 12, 0, 10);    // Chan7 A full scale
    createConfigParam("Ch8:A:FullScale",  '2', 0x3F, 12, 0, 10);    // Chan8 A full scale

    createConfigParam("Ch1:B:FullScale",  '2', 0x40, 12, 0, 10);    // Chan1 B full scale
    createConfigParam("Ch2:B:FullScale",  '2', 0x41, 12, 0, 10);    // Chan2 B full scale
    createConfigParam("Ch3:B:FullScale",  '2', 0x42, 12, 0, 10);    // Chan3 B full scale
    createConfigParam("Ch4:B:FullScale",  '2', 0x43, 12, 0, 10);    // Chan4 B full scale
    createConfigParam("Ch5:B:FullScale",  '2', 0x44, 12, 0, 10);    // Chan5 B full scale
    createConfigParam("Ch6:B:FullScale",  '2', 0x45, 12, 0, 10);    // Chan6 B full scale
    createConfigParam("Ch7:B:FullScale",  '2', 0x46, 12, 0, 10);    // Chan7 B full scale
    createConfigParam("Ch8:B:FullScale",  '2', 0x47, 12, 0, 10);    // Chan8 B full scale
    createConfigParam("Ch1:A:GainAdj",    '2', 0x48, 12, 0, 0);     // Chan1 A gain adjust
    createConfigParam("Ch2:A:GainAdj",    '2', 0x49, 12, 0, 0);     // Chan2 A gain adjust
    createConfigParam("Ch3:A:GainAdj",    '2', 0x4A, 12, 0, 0);     // Chan3 A gain adjust
    createConfigParam("Ch4:A:GainAdj",    '2', 0x4B, 12, 0, 0);     // Chan4 A gain adjust
    createConfigParam("Ch5:A:GainAdj",    '2', 0x4C, 12, 0, 0);     // Chan5 A gain adjust
    createConfigParam("Ch6:A:GainAdj",    '2', 0x4D, 12, 0, 0);     // Chan6 A gain adjust
    createConfigParam("Ch7:A:GainAdj",    '2', 0x4E, 12, 0, 0);     // Chan7 A gain adjust
    createConfigParam("Ch8:A:GainAdj",    '2', 0x4F, 12, 0, 0);     // Chan8 A gain adjust

    createConfigParam("Ch1:B:GainAdj",    '2', 0x50, 12, 0, 0);     // Chan1 B gain adjust
    createConfigParam("Ch2:B:GainAdj",    '2', 0x51, 12, 0, 0);     // Chan2 B gain adjust
    createConfigParam("Ch3:B:GainAdj",    '2', 0x52, 12, 0, 0);     // Chan3 B gain adjust
    createConfigParam("Ch4:B:GainAdj",    '2', 0x53, 12, 0, 0);     // Chan4 B gain adjust
    createConfigParam("Ch5:B:GainAdj",    '2', 0x54, 12, 0, 0);     // Chan5 B gain adjust
    createConfigParam("Ch6:B:GainAdj",    '2', 0x55, 12, 0, 0);     // Chan6 B gain adjust
    createConfigParam("Ch7:B:GainAdj",    '2', 0x56, 12, 0, 0);     // Chan7 B gain adjust
    createConfigParam("Ch8:B:GainAdj",    '2', 0x57, 12, 0, 0);     // Chan8 B gain adjust

    createConfigParam("Ch1:A:Scale",      '3', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch1:A:Offset",     '3', 0x1,  12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch1:B:Scale",      '3', 0x2,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch1:B:Offset",     '3', 0x3,  12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:A:Scale",      '3', 0x4,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch2:A:Offset",     '3', 0x5,  12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch2:B:Scale",      '3', 0x6,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch2:B:Offset",     '3', 0x7,  12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:A:Scale",      '3', 0x8,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch3:A:Offset",     '3', 0x9,  12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch3:B:Scale",      '3', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch3:B:Offset",     '3', 0xB,  12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:A:Scale",      '3', 0xC,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch4:A:Offset",     '3', 0xD,  12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch4:B:Scale",      '3', 0xE,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch4:B:Offset",     '3', 0xF,  12, 0, 1600);  // Chan4 B offset

    createConfigParam("Ch5:A:Scale",      '3', 0x10, 12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch5:A:Offset",     '3', 0x11, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch5:B:Scale",      '3', 0x12, 12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch5:B:Offset",     '3', 0x13, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:A:Scale",      '3', 0x14, 12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch6:A:Offset",     '3', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch6:B:Scale",      '3', 0x16, 12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch6:B:Offset",     '3', 0x17, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:A:Scale",      '3', 0x18, 12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch7:A:Offset",     '3', 0x19, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch7:B:Scale",      '3', 0x1A, 12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch7:B:Offset",     '3', 0x1B, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:A:Scale",      '3', 0x1C, 12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch8:A:Offset",     '3', 0x1D, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch8:B:Scale",      '3', 0x1E, 12, 0, 2048);  // Chan8 B scale
    createConfigParam("Ch8:B:Offset",     '3', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFact",    '4', 0x0,  12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFact",    '4', 0x1,  12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFact",    '4', 0x2,  12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFact",    '4', 0x3,  12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFact",    '4', 0x4,  12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFact",    '4', 0x5,  12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFact",    '4', 0x6,  12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFact",    '4', 0x7,  12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1:A:AvgMin",     'C', 0x0,  12, 0, 0);     // Chan1 A average minimum
    createConfigParam("Ch2:A:AvgMin",     'C', 0x1,  12, 0, 0);     // Chan2 A average minimum
    createConfigParam("Ch3:A:AvgMin",     'C', 0x2,  12, 0, 0);     // Chan3 A average minimum
    createConfigParam("Ch4:A:AvgMin",     'C', 0x3,  12, 0, 0);     // Chan4 A average minimum
    createConfigParam("Ch5:A:AvgMin",     'C', 0x4,  12, 0, 0);     // Chan5 A average minimum
    createConfigParam("Ch6:A:AvgMin",     'C', 0x5,  12, 0, 0);     // Chan6 A average minimum
    createConfigParam("Ch7:A:AvgMin",     'C', 0x6,  12, 0, 0);     // Chan7 A average minimum
    createConfigParam("Ch8:A:AvgMin",     'C', 0x7,  12, 0, 0);     // Chan8 A average minimum
    createConfigParam("Ch1:B:AvgMin",     'C', 0x8,  12, 0, 0);     // Chan1 B average minimum
    createConfigParam("Ch2:B:AvgMin",     'C', 0x9,  12, 0, 0);     // Chan2 B average minimum
    createConfigParam("Ch3:B:AvgMin",     'C', 0xA,  12, 0, 0);     // Chan3 B average minimum
    createConfigParam("Ch4:B:AvgMin",     'C', 0xB,  12, 0, 0);     // Chan4 B average minimum
    createConfigParam("Ch5:B:AvgMin",     'C', 0xC,  12, 0, 0);     // Chan5 B average minimum
    createConfigParam("Ch6:B:AvgMin",     'C', 0xD,  12, 0, 0);     // Chan6 B average minimum
    createConfigParam("Ch7:B:AvgMin",     'C', 0xE,  12, 0, 0);     // Chan7 B average minimum
    createConfigParam("Ch8:B:AvgMin",     'C', 0xF,  12, 0, 0);     // Chan8 B average minimum

    createConfigParam("Ch1:A:AvgMax",     'C', 0x10, 12, 0, 4095);  // Chan1 A average maximum
    createConfigParam("Ch2:A:AvgMax",     'C', 0x11, 12, 0, 4095);  // Chan2 A average maximum
    createConfigParam("Ch3:A:AvgMax",     'C', 0x12, 12, 0, 4095);  // Chan3 A average maximum
    createConfigParam("Ch4:A:AvgMax",     'C', 0x13, 12, 0, 4095);  // Chan4 A average maximum
    createConfigParam("Ch5:A:AvgMax",     'C', 0x14, 12, 0, 4095);  // Chan5 A average maximum
    createConfigParam("Ch6:A:AvgMax",     'C', 0x15, 12, 0, 4095);  // Chan6 A average maximum
    createConfigParam("Ch7:A:AvgMax",     'C', 0x16, 12, 0, 4095);  // Chan7 A average maximum
    createConfigParam("Ch8:A:AvgMax",     'C', 0x17, 12, 0, 4095);  // Chan8 A average maximum
    createConfigParam("Ch1:B:AvgMax",     'C', 0x18, 12, 0, 4095);  // Chan1 B average maximum
    createConfigParam("Ch2:B:AvgMax",     'C', 0x19, 12, 0, 4095);  // Chan2 B average maximum
    createConfigParam("Ch3:B:AvgMax",     'C', 0x1A, 12, 0, 4095);  // Chan3 B average maximum
    createConfigParam("Ch4:B:AvgMax",     'C', 0x1B, 12, 0, 4095);  // Chan4 B average maximum
    createConfigParam("Ch5:B:AvgMax",     'C', 0x1C, 12, 0, 4095);  // Chan5 B average maximum
    createConfigParam("Ch6:B:AvgMax",     'C', 0x1D, 12, 0, 4095);  // Chan6 B average maximum
    createConfigParam("Ch7:B:AvgMax",     'C', 0x1E, 12, 0, 4095);  // Chan7 B average maximum
    createConfigParam("Ch8:B:AvgMax",     'C', 0x1F, 12, 0, 4095);  // Chan8 B average maximum

    createConfigParam("Ch1:A:SampleMin",  'D', 0x0,  12, 0, 0);     // Chan1 A sample minimum
    createConfigParam("Ch2:A:SampleMin",  'D', 0x1,  12, 0, 0);     // Chan2 A sample minimum
    createConfigParam("Ch3:A:SampleMin",  'D', 0x2,  12, 0, 0);     // Chan3 A sample minimum
    createConfigParam("Ch4:A:SampleMin",  'D', 0x3,  12, 0, 0);     // Chan4 A sample minimum
    createConfigParam("Ch5:A:SampleMin",  'D', 0x4,  12, 0, 0);     // Chan5 A sample minimum
    createConfigParam("Ch6:A:SampleMin",  'D', 0x5,  12, 0, 0);     // Chan6 A sample minimum
    createConfigParam("Ch7:A:SampleMin",  'D', 0x6,  12, 0, 0);     // Chan7 A sample minimum
    createConfigParam("Ch8:A:SampleMin",  'D', 0x7,  12, 0, 0);     // Chan8 A sample minimum
    createConfigParam("Ch1:B:SampleMin",  'D', 0x8,  12, 0, 0);     // Chan1 B sample minimum
    createConfigParam("Ch2:B:SampleMin",  'D', 0x9,  12, 0, 0);     // Chan2 B sample minimum
    createConfigParam("Ch3:B:SampleMin",  'D', 0xA,  12, 0, 0);     // Chan3 B sample minimum
    createConfigParam("Ch4:B:SampleMin",  'D', 0xB,  12, 0, 0);     // Chan4 B sample minimum
    createConfigParam("Ch5:B:SampleMin",  'D', 0xC,  12, 0, 0);     // Chan5 B sample minimum
    createConfigParam("Ch6:B:SampleMin",  'D', 0xD,  12, 0, 0);     // Chan6 B sample minimum
    createConfigParam("Ch7:B:SampleMin",  'D', 0xE,  12, 0, 0);     // Chan7 B sample minimum
    createConfigParam("Ch8:B:SampleMin",  'D', 0xF,  12, 0, 0);     // Chan8 B sample minimum

    createConfigParam("Ch1:A:SampleMax",  'D', 0x10, 12, 0, 4095);  // Chan1 A sample maximum
    createConfigParam("Ch2:A:SampleMax",  'D', 0x11, 12, 0, 4095);  // Chan2 A sample maximum
    createConfigParam("Ch3:A:SampleMax",  'D', 0x12, 12, 0, 4095);  // Chan3 A sample maximum
    createConfigParam("Ch4:A:SampleMax",  'D', 0x13, 12, 0, 4095);  // Chan4 A sample maximum
    createConfigParam("Ch5:A:SampleMax",  'D', 0x14, 12, 0, 4095);  // Chan5 A sample maximum
    createConfigParam("Ch6:A:SampleMax",  'D', 0x15, 12, 0, 4095);  // Chan6 A sample maximum
    createConfigParam("Ch7:A:SampleMax",  'D', 0x16, 12, 0, 4095);  // Chan7 A sample maximum
    createConfigParam("Ch8:A:SampleMax",  'D', 0x17, 12, 0, 4095);  // Chan8 A sample maximum
    createConfigParam("Ch1:B:SampleMax",  'D', 0x18, 12, 0, 4095);  // Chan1 B sample maximum
    createConfigParam("Ch2:B:SampleMax",  'D', 0x19, 12, 0, 4095);  // Chan2 B sample maximum
    createConfigParam("Ch3:B:SampleMax",  'D', 0x1A, 12, 0, 4095);  // Chan3 B sample maximum
    createConfigParam("Ch4:B:SampleMax",  'D', 0x1B, 12, 0, 4095);  // Chan4 B sample maximum
    createConfigParam("Ch5:B:SampleMax",  'D', 0x1C, 12, 0, 4095);  // Chan5 B sample maximum
    createConfigParam("Ch6:B:SampleMax",  'D', 0x1D, 12, 0, 4095);  // Chan6 B sample maximum
    createConfigParam("Ch7:B:SampleMax",  'D', 0x1E, 12, 0, 4095);  // Chan7 B sample maximum
    createConfigParam("Ch8:B:SampleMax",  'D', 0x1F, 12, 0, 4095);  // Chan8 B sample maximum

    createConfigParam("MaximumSlope",     'D', 0x20, 12, 0, 0);     // Maximum slope

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

    createConfigParam("TimeVetoLow",      'E', 0x1,  32, 0, 0);     // Timestamp veto low            (scale:100,unit:ns,prec:1)
    createConfigParam("TimeVetoHigh",     'E', 0x3,  32, 0, 333);   // Timestamp veto high           (scale:100,unit:ns,prec:1)
    createConfigParam("FakeTrigDelay",    'E', 0x5,  16, 0, 200);   // Fake trigger delay            (scale:100,unit:ns,prec:1)
    createConfigParam("MinAdc",           'E', 0x6,  14, 0, 100);   // Minimum ADC
    createConfigParam("MaxAdc",           'E', 0x7,  14, 0, 1000);  // Maximum ADC
    createConfigParam("IntRelease",       'E', 0x8,   9, 0, -6, CONV_SIGN_2COMP); // Integrator release point
    createConfigParam("MinDiscrimWidth",  'E', 0x9,   9, 0, 1);     // Minimum discriminator width
    createConfigParam("Sample1",          'E', 0xA,   9, 0, 2);     // Sample 1 point [0:10]
    createConfigParam("Sample2",          'E', 0xB,   9, 0, 14);    // Sample 1 point [2:50]
    createConfigParam("TsyncDelay",       'E', 0xC,  32, 0, 0);     // TSYNC delay                   (scale:100,unit:ns,prec:1)

    createConfigParam("LvdsRate",         'F', 0x0,  1, 15, 1);     // LVDS output rate              (0=20Mhz,1=40Mhz)
    createConfigParam("AutoCorrectMode",  'F', 0x0,  1, 14, 0);     // Auto correction sample mode   (0=TSYNC,1=fake trigger)
    createConfigParam("EdgeDetectEn",     'F', 0x0,  1, 13, 1);     // Edge detect enable            (0=disable,1=enable)
    createConfigParam("MastDiscrimMode",  'F', 0x0,  2, 11, 0);     // Master discriminator select   (0=SUM discr,1=A discr,2=B discr,3=all)
    createConfigParam("AcquireEn",        'F', 0x0,  1, 10, 1);     // ROC enable acquisition        (0=disable,1=enable)
    createConfigParam("AutoCorrectionEn", 'F', 0x0,  1, 9,  1);     // Auto correction mode          (0=enabled,1=disabled)
    createConfigParam("Resolution",       'F', 0x0,  1, 8,  1);     // High resolution mode          (0=128,1=256)
    createConfigParam("OutputMode",       'F', 0x0,  2, 6,  0);     // Output mode                   (0=normal,1=raw [alarm],2=extended [alarm])
    createConfigParam("AcquireMode",      'F', 0x0,  2, 4,  0);     // Acquire mode                  (0=normal,1=verbose,2=fakedata,3=trigger)
    createConfigParam("TcTxEnMode",       'F', 0x0,  1, 3,  1);     // T&C TX enable mode            (0=external,1=internal)
    createConfigParam("TcTsyncMode",      'F', 0x0,  1, 2,  0);     // T&C TSYNC mode                (0=external,1=internal 60Hz)
    createConfigParam("TcTclkMode",       'F', 0x0,  1, 1,  0);     // T&C TCLK mode                 (0=external,1=internal 10MHz)
    createConfigParam("TcResetMode",      'F', 0x0,  1, 0,  0);     // T&C Reset mode                (0=internal,1=external)

    createConfigParam("TestPatternEn",    'F', 0x1,  1, 15, 0);     // Test pattern enable           (0=disable,1=enable)
    createConfigParam("TestPatternAltEn", 'F', 0x1,  1, 14, 0);     // Alternate test pattern enable (0=disable,1=enable)
    createConfigParam("TestPatternDebug", 'F', 0x1,  2, 12, 0);     // Engineering Use only
    createConfigParam("TestPatternId",    'F', 0x1, 12,  0, 0);     // Test pattern id
    createConfigParam("TestPatternRate",  'F', 0x2, 16,  0, 65535); // Test pattern rate             (65535=153 ev/s,9999=1 Kev/s,4999=2 Kev/s,1999=5 Kev/s,999=10 Kev/s,399=25 Kev/s,199=50 Kev/s,99=100 Kev/s,13=800 Kev/s,9=1 Mev/s,4=2 Mev/s,1=5 Mev/s,0=10 Mev/s)
    createConfigParam("AcqFreq",          'F', 0x3, 10,  0, 1);     // Num cycles to advance @TSYNC  (1=60Hz,2=30Hz,3=20Hz,4=15Hz,6=10Hz,12=5Hz,30=1Hz,60=1Hz)
    createConfigParam("DataFormat",       'F', 0x4,  8,  0, 2);     // Data format identifier        (2=pixel,3=raw,4=verbose)
    createConfigParam("Protocol",         'F', 0x4,  1, 15, 0);     // Extended event format         (0=legacy,1=new)
    createConfigParam("PreampIface",      'F', 0x5,  1,  0, 0);     // Preamplifier Interface        (0=SPI old preamps,1=I2C new preamps)
    createConfigParam("HVPSIface",        'F', 0x5,  1,  1, 0);     // HV PS Interface               (0=Serial old HVPS,1=I2C new HVPS)

//  BLXXX:Det:RocXXX:| parameter name |                 | EPICS record description  | (bi and mbbi description)
    createTempParam("TempBoard",        0x0, 16, 0, CONV_SIGN_2COMP); // ROC board temperature in degC   (calc:0.25*A,unit:C,prec:1,low:-50,high:50, archive:monitor)
    createTempParam("TempPreampA",      0x1, 16, 0, CONV_SIGN_2COMP); // Preamp A temperature in degC    (calc:0.25*A,unit:C,prec:1,low:-50,high:50, archive:monitor)
    createTempParam("TempPreampB",      0x2, 16, 0, CONV_SIGN_2COMP); // Preamp B temperature in degC    (calc:0.25*A,unit:C,prec:1,low:-50,high:50, archive:monitor)

    // Pre-amp registers
    createPreAmpCfgParam("PreampA:Dac",         0x0, 16,  0, 32768); // Pre-amp A DAC code
    createPreAmpCfgParam("PreampB:Dac",         0x1, 16,  0, 32768); // Pre-amp B DAC code

    createPreAmpTrigParam("PreampA:Tube1:En",   0x0,  1,  0, 0); // Pre-amp A tube 1 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube2:En",   0x0,  1,  1, 0); // Pre-amp A tube 2 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube3:En",   0x0,  1,  2, 0); // Pre-amp A tube 3 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube4:En",   0x0,  1,  3, 0); // Pre-amp A tube 4 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube5:En",   0x0,  1,  4, 0); // Pre-amp A tube 5 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube6:En",   0x0,  1,  5, 0); // Pre-amp A tube 6 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube7:En",   0x0,  1,  6, 0); // Pre-amp A tube 7 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampA:Tube8:En",   0x0,  1,  7, 0); // Pre-amp A tube 8 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube4:En",   0x0,  1,  8, 0); // Pre-amp B tube 1 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube3:En",   0x0,  1,  9, 0); // Pre-amp B tube 2 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube2:En",   0x0,  1, 10, 0); // Pre-amp B tube 3 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube1:En",   0x0,  1, 11, 0); // Pre-amp B tube 4 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube8:En",   0x0,  1, 12, 0); // Pre-amp B tube 5 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube7:En",   0x0,  1, 13, 0); // Pre-amp B tube 6 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube6:En",   0x0,  1, 14, 0); // Pre-amp B tube 7 enable (0=charge,1=discharge)
    createPreAmpTrigParam("PreampB:Tube5:En",   0x0,  1, 15, 0); // Pre-amp B tube 8 enable (0=charge,1=discharge)
}
