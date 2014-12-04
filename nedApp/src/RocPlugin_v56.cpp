/* RocPlugin_v56.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "RocPlugin.h"

/**
 * @file RocPlugin_v56.cpp
 *
 * ROC 5.6 parameters
 *
 * The ROC 5.6 firmware is identical to ROC 5.2 except that it adds support for
 * error counter and rate meter registers using READ_STATUS_COUNTERS (0x24)
 * command.
 */

void RocPlugin::createStatusParams_v56()
{
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
    createStatusParam("Acquiring",            0x1,  1,  3); // Acquiring data               (0=not acquiring,1=acquiring)
    createStatusParam("Discovered",           0x1,  1,  2); // Discovered                   (0=not discovered,1=discovered)
    createStatusParam("Configured",           0x1,  2,  0); // Configured                   (0=not configured,1=section 1 conf'd,2=section 2 conf'd,3=all conf'd)

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

    createStatusParam("Ch1A:AutoAdjEn",       0x4,  1, 15); // Chan1 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch1A:AutoAdjTrig",     0x4,  1, 14); // Chan1 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch1A:AutoAdjSample",   0x4,  1, 13); // Chan1 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch1A:AutoAdjSlope",    0x4,  1, 12); // Chan1 A Auto-Adjust Slope l  (0=not active,1=active)
    createStatusParam("Ch1A:AutoAdjOffset",   0x4,  1, 11); // Chan1 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch1A:AutoAdjSlope1",   0x4,  1, 10); // Chan1 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch1A:AutoAdjOverflow", 0x4,  1,  9); // Chan1 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch1A:InputOffset",     0x4,  9,  0); // Chan1 A Input Offset value

    createStatusParam("Ch1B:AdcOffset",       0x5,  8,  8); // Chan1 B ADC Offset value
    createStatusParam("Ch1A:AdcOffset",       0x5,  8,  0); // Chan1 A ADC Offset value

    createStatusParam("Ch1B:AutoAdjEn",       0x6,  1, 15); // Chan1 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch1B:AutoAdjTrig",     0x6,  1, 14); // Chan1 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch1B:AutoAdjSample",   0x6,  1, 13); // Chan1 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch1B:AutoAdjSlope",    0x6,  1, 12); // Chan1 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch1B:AutoAdjOffset",   0x6,  1, 11); // Chan1 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch1B:AutoAdjSlope1",   0x6,  1, 10); // Chan1 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch1B:AutoAdjOverflow", 0x6,  1,  9); // Chan1 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch1B:InputOffset",     0x6,  9,  0); // Chan1 B Input Offset value

    // NOTE: The next parameter spans over the LVDS words (16bits) *and* it also spans over the
    //       OCC dword (32 bits). BaseModulePlugin::createStatusParam() and BaseModulePlugin:rspReadStatus()
    //       functions are smart enough to accomodate the behaviour.
    createStatusParam("Ch2A:InputOffset",     0x7,  9,  8); // Chan2 A input offset value
    createStatusParam("Ch1:AdcMax",           0x7,  1,  7); // Chan1 got ADC max            (0=no,1=yes)
    createStatusParam("Ch1:AdcMin",           0x7,  1,  6); // Chan1 got ADC min            (0=no,1=yes)
    createStatusParam("Ch1:MultiDiscEvent",   0x7,  1,  5); // Chan1 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch1:Event",            0x7,  1,  4); // Chan1 got event              (0=no,1=yes)
    createStatusParam("Ch1:FifoFull",         0x7,  1,  3); // Chan1 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch1:FifoAlmostFullD",  0x7,  1,  2); // Chan1 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch1:FifoAlmostFull",   0x7,  1,  1); // Chan1 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch1:HasData",          0x7,  1,  0); // Chan1 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch2A:AdcOffset",       0x8,  8,  8); // Chan2 A ADC Offset value
    createStatusParam("Ch2A:AutoAdjEn",       0x8,  1,  7); // Chan2 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch2A:AutoAdjTrig",     0x8,  1,  6); // Chan2 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch2A:AutoAdjSample",   0x8,  1,  5); // Chan2 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch2A:AutoAdjSlope",    0x8,  1,  4); // Chan2 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch2A:AutoAdjOffset",   0x8,  1,  3); // Chan2 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch2A:AutoAdjSlope1",   0x8,  1,  2); // Chan2 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch2A:AutoAdjOverflow", 0x8,  1,  1); // Chan2 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch2B:InputOffset",     0x9,  9,  8); // Chan2 B input offset value
    createStatusParam("Ch2B:AdcOffset",       0x9,  8,  0); // Chan2 B ADC Offset value

    createStatusParam("Ch2:AdcMax",           0xA,  1, 15); // Chan2 got ADC max            (0=no,1=yes)
    createStatusParam("Ch2:AdcMin",           0xA,  1, 14); // Chan2 got ADC min            (0=no,1=yes)
    createStatusParam("Ch2:MultiDiscEvent",   0xA,  1, 13); // Chan2 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch2:Event",            0xA,  1, 12); // Chan2 got event              (0=no,1=yes)
    createStatusParam("Ch2:FifoFull",         0xA,  1, 11); // Chan2 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch2:FifoAlmostFullD",  0xA,  1, 10); // Chan2 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch2:FifoAlmostFull",   0xA,  1,  9); // Chan2 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch2:HasData",          0xA,  1,  8); // Chan2 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch2B:AutoAdjEn",       0xA,  1,  7); // Chan2 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch2B:AutoAdjTrig",     0xA,  1,  6); // Chan2 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch2B:AutoAdjSample",   0xA,  1,  5); // Chan2 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch2B:AutoAdjSlope",    0xA,  1,  4); // Chan2 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch2B:AutoAdjOffset",   0xA,  1,  3); // Chan2 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch2B:AutoAdjSlope1",   0xA,  1,  2); // Chan2 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch2B:AutoAdjOverflow", 0xA,  1,  1); // Chan2 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch3A:AutoAdjEn",       0xB,  1, 15); // Chan3 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch3A:AutoAdjTrig",     0xB,  1, 14); // Chan3 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch3A:AutoAdjSample",   0xB,  1, 13); // Chan3 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch3A:AutoAdjSlope",    0xB,  1, 12); // Chan3 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch3A:AutoAdjOffset",   0xB,  1, 11); // Chan3 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch3A:AutoAdjSlope1",   0xB,  1, 10); // Chan3 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch3A:AutoAdjOverflow", 0xB,  1,  9); // Chan3 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch3A:InputOffset",     0xB,  9,  0); // Chan3 A input offset value

    createStatusParam("Ch3A:AdcOffs",         0xC,  8,  0); // Chan3 A ADC Offset value
    createStatusParam("Ch3B:AdcOffs",         0xC,  8,  8); // Chan3 B ADC Offset value

    createStatusParam("Ch3B:AutoAdjEn",       0xD,  1, 15); // Chan3 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch3B:AutoAdjTrig",     0xD,  1, 14); // Chan3 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch3B:AutoAdjSample",   0xD,  1, 13); // Chan3 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch3B:AutoAdjSlope",    0xD,  1, 12); // Chan3 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch3B:AutoAdjOffset",   0xD,  1, 11); // Chan3 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch3B:AutoAdjSlope1",   0xD,  1, 10); // Chan3 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch3B:AutoAdjOverflow", 0xD,  1,  9); // Chan3 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch3B:InputOffset",     0xD,  9,  0); // Chan3 B input offset value

    createStatusParam("Ch4A:InputOffset",     0xE,  9,  8); // Chan4 A input offset value
    createStatusParam("Ch3:AdcMax",           0xE,  1,  7); // Chan3 got ADC max            (0=no,1=yes)
    createStatusParam("Ch3:AdcMin",           0xE,  1,  6); // Chan3 got ADC min            (0=no,1=yes)
    createStatusParam("Ch3:MultiDiscEvent",   0xE,  1,  5); // Chan3 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch3:Event",            0xE,  1,  4); // Chan3 got event              (0=no,1=yes)
    createStatusParam("Ch3:FifoFull",         0xE,  1,  3); // Chan3 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch3:FifoAlmostFullD",  0xE,  1,  2); // Chan3 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch3:FifoAlmostFull",   0xE,  1,  1); // Chan3 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch3:HasData",          0xE,  1,  0); // Chan3 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch4A:AdcOffs",         0xF,  8,  8); // Chan4 A ADC Offset value
    createStatusParam("Ch4A:AutoAdjEn",       0xF,  1,  7); // Chan4 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch4A:AutoAdjTrig",     0xF,  1,  6); // Chan4 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch4A:AutoAdjSample",   0xF,  1,  5); // Chan4 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch4A:AutoAdjSlope",    0xF,  1,  4); // Chan4 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch4A:AutoAdjOffset",   0xF,  1,  3); // Chan4 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch4A:AutoAdjSlope1",   0xF,  1,  2); // Chan4 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch4A:AutoAdjOverflow", 0xF,  1,  1); // Chan4 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch4B:InputOffset",     0x10, 9,  8); // Chan4 B input offset value
    createStatusParam("Ch4B:AdcOffs",         0x10, 8,  0); // Chan4 B ADC Offset value

    createStatusParam("Ch4:AdcMax",           0x11, 1, 15); // Chan4 got ADC max            (0=no,1=yes)
    createStatusParam("Ch4:AdcMin",           0x11, 1, 14); // Chan4 got ADC min            (0=no,1=yes)
    createStatusParam("Ch4:MultiDiscEvent",   0x11, 1, 13); // Chan4 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch4:Event",            0x11, 1, 12); // Chan4 got event              (0=no,1=yes)
    createStatusParam("Ch4:FifoFull",         0x11, 1, 11); // Chan4 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch4:FifoAlmostFullD",  0x11, 1, 10); // Chan4 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch4:FifoAlmostFull",   0x11, 1,  9); // Chan4 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch4:HasData",          0x11, 1,  8); // Chan4 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch4B:AutoAdjEn",       0x11, 1,  7); // Chan4 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch4B:AutoAdjTrig",     0x11, 1,  6); // Chan4 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch4B:AutoAdjSample",   0x11, 1,  5); // Chan4 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch4B:AutoAdjSlope",    0x11, 1,  4); // Chan4 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch4B:AutoAdjOffset",   0x11, 1,  3); // Chan4 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch4B:AutoAdjSlope1",   0x11, 1,  2); // Chan4 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch4B:AutoAdjOverflow", 0x11, 1,  1); // Chan4 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch5A:AutoAdjEn",       0x12, 1, 15); // Chan5 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch5A:AutoAdjTrig",     0x12, 1, 14); // Chan5 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch5A:AutoAdjSample",   0x12, 1, 13); // Chan5 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch5A:AutoAdjSlope",    0x12, 1, 12); // Chan5 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch5A:AutoAdjOffset",   0x12, 1, 11); // Chan5 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch5A:AutoAdjSlope1",   0x12, 1, 10); // Chan5 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch5A:AutoAdjOverflow", 0x12, 1,  9); // Chan5 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch5A:InputOffset",     0x12, 9,  0); // Chan5 A Input Offset value

    createStatusParam("Ch5B:AdcOffs",         0x13, 8,  8); // Chan5 B ADC Offset value
    createStatusParam("Ch5A:AdcOffs",         0x13, 8,  0); // Chan5 A ADC Offset value

    createStatusParam("Ch5B:AutoAdjEn",       0x14, 1, 15); // Chan5 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch5B:AutoAdjTrig",     0x14, 1, 14); // Chan5 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch5B:AutoAdjSample",   0x14, 1, 13); // Chan5 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch5B:AutoAdjSlope",    0x14, 1, 12); // Chan5 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch5B:AutoAdjOffset",   0x14, 1, 11); // Chan5 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch5B:AutoAdjSlope1",   0x14, 1, 10); // Chan5 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch5B:AutoAdjOverflow", 0x14, 1,  9); // Chan5 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch5B:InputOffset",     0x14, 9,  0); // Chan5 B Input Offset value

    createStatusParam("Ch6A:InputOffset",     0x15, 9,  8); // Chan6 A input offset value
    createStatusParam("Ch5:AdcMax",           0x15, 1,  7); // Chan5 got ADC max            (0=no,1=yes)
    createStatusParam("Ch5:AdcMin",           0x15, 1,  6); // Chan5 got ADC min            (0=no,1=yes)
    createStatusParam("Ch5:MultiDiscEvent",   0x15, 1,  5); // Chan5 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch5:Event",            0x15, 1,  4); // Chan5 got event              (0=no,1=yes)
    createStatusParam("Ch5:FifoFull",         0x15, 1,  3); // Chan5 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch5:FifoAlmostFullD",  0x15, 1,  2); // Chan5 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch5:FifoAlmostFull",   0x15, 1,  1); // Chan5 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch5:HasData",          0x15, 1,  0); // Chan5 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch6A:AdcOffs",         0x16, 8,  8); // Chan6 A ADC Offset value
    createStatusParam("Ch6A:AutoAdjEn",       0x16, 1,  7); // Chan6 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch6A:AutoAdjTrig",     0x16, 1,  6); // Chan6 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch6A:AutoAdjSample",   0x16, 1,  5); // Chan6 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch6A:AutoAdjSlope",    0x16, 1,  4); // Chan6 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch6A:AutoAdjOffset",   0x16, 1,  3); // Chan6 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch6A:AutoAdjSlope1",   0x16, 1,  2); // Chan6 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch6A:AutoAdjOverflow", 0x16, 1,  1); // Chan6 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch6B:InputOffset",     0x17, 9,  8); // Chan6 B input offset value
    createStatusParam("Ch6B:AdcOffs",         0x17, 8,  0); // Chan6 B ADC Offset value

    createStatusParam("Ch6:AdcMax",           0x18, 1, 15); // Chan6 got ADC max            (0=no,1=yes)
    createStatusParam("Ch6:AdcMin",           0x18, 1, 14); // Chan6 got ADC min            (0=no,1=yes)
    createStatusParam("Ch6:MultiDiscEvent",   0x18, 1, 13); // Chan6 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch6:Event",            0x18, 1, 12); // Chan6 got event              (0=no,1=yes)
    createStatusParam("Ch6:FifoFull",         0x18, 1, 11); // Chan6 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch6:FifoAlmostFullD",  0x18, 1, 10); // Chan6 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch6:FifoAlmostFull",   0x18, 1,  9); // Chan6 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch6:HasData",          0x18, 1,  8); // Chan6 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch6B:AutoAdjEn",       0x18, 1,  7); // Chan6 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch6B:AutoAdjTrig",     0x18, 1,  6); // Chan6 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch6B:AutoAdjSample",   0x18, 1,  5); // Chan6 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch6B:AutoAdjSlope",    0x18, 1,  4); // Chan6 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch6B:AutoAdjOffset",   0x18, 1,  3); // Chan6 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch6B:AutoAdjSlope1",   0x18, 1,  2); // Chan6 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch6B:AutoAdjOverflow", 0x18, 1,  1); // Chan6 B Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch7A:AutoAdjEn",       0x19, 1, 15); // Chan7 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch7A:AutoAdjTrig",     0x19, 1, 14); // Chan7 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch7A:AutoAdjSample",   0x19, 1, 13); // Chan7 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch7A:AutoAdjSlope",    0x19, 1, 12); // Chan7 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch7A:AutoAdjOffset",   0x19, 1, 11); // Chan7 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch7A:AutoAdjSlope1",   0x19, 1, 10); // Chan7 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch7A:AutoAdjOverflow", 0x19, 1,  9); // Chan7 A Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch7A:InputOffset",     0x19, 9,  0); // Chan7 A input offset value

    createStatusParam("Ch7A:AdcOffs",         0x1A, 8,  0); // Chan7 A ADC Offset value
    createStatusParam("Ch7B:AdcOffs",         0x1A, 8,  8); // Chan7 B ADC Offset value

    createStatusParam("Ch7B:AutoAdjEn",       0x1B, 1, 15); // Chan7 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch7B:AutoAdjTrig",     0x1B, 1, 14); // Chan7 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch7B:AutoAdjSample",   0x1B, 1, 13); // Chan7 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch7B:AutoAdjSlope",    0x1B, 1, 12); // Chan7 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch7B:AutoAdjOffset",   0x1B, 1, 11); // Chan7 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch7B:AutoAdjSlope1",   0x1B, 1, 10); // Chan7 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch7B:AutoAdjOverflow", 0x1B, 1,  9); // Chan7 B Auto-Adjust overflow (0=no,1=yes)
    createStatusParam("Ch7B:InputOffset",     0x1B, 9,  0); // Chan7 B input offset value

    createStatusParam("Ch8A:InputOffset",     0x1C, 9,  8); // Chan8 A input offset value
    createStatusParam("Ch7:AdcMax",           0x1C, 1,  7); // Chan7 got ADC max            (0=no,1=yes)
    createStatusParam("Ch7:AdcMin",           0x1C, 1,  6); // Chan7 got ADC min            (0=no,1=yes)
    createStatusParam("Ch7:MultiDiscEvent",   0x1C, 1,  5); // Chan7 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch7:Event",            0x1C, 1,  4); // Chan7 got event              (0=no,1=yes)
    createStatusParam("Ch7:FifoFull",         0x1C, 1,  3); // Chan7 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch7:FifoAlmostFullD",  0x1C, 1,  2); // Chan7 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch7:FifoAlmostFull",   0x1C, 1,  1); // Chan7 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch7:HasData",          0x1C, 1,  0); // Chan7 FIFO has data          (0=no,1=yes)

    createStatusParam("Ch8A:AdcOffs",         0x1D, 8,  8); // Chan8 A ADC Offset value
    createStatusParam("Ch8A:AutoAdjEn",       0x1D, 1,  7); // Chan8 A Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch8A:AutoAdjTrig",     0x1D, 1,  6); // Chan8 A Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch8A:AutoAdjSample",   0x1D, 1,  5); // Chan8 A Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch8A:AutoAdjSlope",    0x1D, 1,  4); // Chan8 A Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch8A:AutoAdjOffset",   0x1D, 1,  3); // Chan8 A Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch8A:AutoAdjSlope1",   0x1D, 1,  2); // Chan8 A Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch8A:AutoAdjOverflow", 0x1D, 1,  1); // Chan8 A Auto-Adjust overflow (0=no,1=yes)

    createStatusParam("Ch8B:InputOffset",     0x1E, 9,  8); // Chan8 B input offset value
    createStatusParam("Ch8B:AdcOffs",         0x1E, 8,  0); // Chan8 B ADC Offset value

    createStatusParam("Ch8:AdcMax",           0x1F, 1, 15); // Chan8 got ADC max            (0=no,1=yes)
    createStatusParam("Ch8:AdcMin",           0x1F, 1, 14); // Chan8 got ADC min            (0=no,1=yes)
    createStatusParam("Ch8:MultiDiscEvent",   0x1F, 1, 13); // Chan8 got multi-discp event  (0=no,1=yes)
    createStatusParam("Ch8:Event",            0x1F, 1, 12); // Chan8 got event              (0=no,1=yes)
    createStatusParam("Ch8:FifoFull",         0x1F, 1, 11); // Chan8 FIFO full detectec     (0=no,1=yes)
    createStatusParam("Ch8:FifoAlmostFullD",  0x1F, 1, 10); // Chan8 FIFO almost full detec (0=no,1=yes)
    createStatusParam("Ch8:FifoAlmostFull",   0x1F, 1,  9); // Chan8 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch8:HasData",          0x1F, 1,  8); // Chan8 FIFO has data          (0=no,1=yes)
    createStatusParam("Ch8B:AutoAdjEn",       0x1F, 1,  7); // Chan8 B Auto-Adjust Active   (0=not active,1=active)
    createStatusParam("Ch8B:AutoAdjTrig",     0x1F, 1,  6); // Chan8 B Auto-Adjust Got samp (0=no sample,1=got sample)
    createStatusParam("Ch8B:AutoAdjSample",   0x1F, 1,  5); // Chan8 B Auto-Adjust Sample l (0=not active,1=active)
    createStatusParam("Ch8B:AutoAdjSlope",    0x1F, 1,  4); // Chan8 B Auto-Adjust Slope li (0=not active,1=active)
    createStatusParam("Ch8B:AutoAdjOffset",   0x1F, 1,  3); // Chan8 B Auto-Adjust Offset l (0=not active,1=active)
    createStatusParam("Ch8B:AutoAdjSlope1",   0x1F, 1,  2); // Chan8 B Auto-Adjust Slope? l (0=not active,1=active)
    createStatusParam("Ch8B:AutoAdjOverflow", 0x1F, 1,  1); // Chan8 B Auto-Adjust overflow (0=no,1=yes)
}

void RocPlugin::createCounterParams_v56()
{
    createCounterParam("CntParity",         0x0, 16,  0); // LVDS parity error counter
    createCounterParam("CntType",           0x1, 16,  0); // LVDS data type error counter
    createCounterParam("CntLength",         0x2, 16,  0); // LVDS length error counter
    createCounterParam("CntTimeout",        0x3, 16,  0); // LVDS timeout counter
    createCounterParam("CntNoStart",        0x4, 16,  0); // LVDS no start error counter
    createCounterParam("CntPreStart",       0x5, 16,  0); // LVDS start before stop cnt
    createCounterParam("CntFifoFull",       0x6, 16,  0); // LVDS FIFO full error counter
    createCounterParam("CntNoTsync",        0x7, 16,  0); // Timestamp overflow counter
    createCounterParam("CntCmdBad",         0x8, 16,  0); // Unknown command counter
    createCounterParam("CntCmdLength",      0x9, 16,  0); // Command length error counter
    createCounterParam("CntProgramming",    0xA, 16,  0); // Write cnfg disallowed cnt
    createCounterParam("CntUartParity",     0xB, 16,  0); // UART parity error counter
    createCounterParam("CntUartByte",       0xC, 16,  0); // UART byte error counter
    createCounterParam("CntDataAlmostFull", 0xD, 16,  0); // Data almost full counter
    createCounterParam("Ch0:CntFifoFull",   0xE, 16,  0); // Ch0 ADC FIFO full counter
    createCounterParam("Ch1:CntFifoFull",   0xF, 16,  0); // Ch1 ADC FIFO full counter
    createCounterParam("Ch2:CntFifoFull",   0x10, 16,  0); // Ch2 ADC FIFO full counter
    createCounterParam("Ch3:CntFifoFull",   0x11, 16,  0); // Ch3 ADC FIFO full counter
    createCounterParam("Ch4:CntFifoFull",   0x12, 16,  0); // Ch4 ADC FIFO full counter
    createCounterParam("Ch5:CntFifoFull",   0x13, 16,  0); // Ch5 ADC FIFO full counter
    createCounterParam("Ch6:CntFifoFull",   0x14, 16,  0); // Ch6 ADC FIFO full counter
    createCounterParam("Ch7:CntFifoFull",   0x15, 16,  0); // Ch7 ADC FIFO full counter
    createCounterParam("CntMissClk",        0x16, 16,  0); // Link RX clock missing cnt
    createCounterParam("Ch0:CntPosEdge",    0x17, 16,  0); // Ch0 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch1:CntPosEdge",    0x18, 16,  0); // Ch1 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch2:CntPosEdge",    0x19, 16,  0); // Ch2 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch3:CntPosEdge",    0x1A, 16,  0); // Ch3 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch4:CntPosEdge",    0x1B, 16,  0); // Ch4 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch5:CntPosEdge",    0x1C, 16,  0); // Ch5 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch6:CntPosEdge",    0x1D, 16,  0); // Ch6 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch7:CntPosEdge",    0x1E, 16,  0); // Ch7 positive edge counter     (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch0:CntSumHigh",    0x1F, 16,  0); // Ch0 SUM high counter          (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch1:CntSumHigh",    0x20, 16,  0); // Ch1 SUM high counter          (calc:1000*A/52.4288,unit:counts/s,prec:0)
    createCounterParam("Ch2:CntSumHigh",    0x21, 16,  0); // Ch2 SUM high counter          (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch3:CntSumHigh",    0x22, 16,  0); // Ch3 SUM high counter          (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch4:CntSumHigh",    0x23, 16,  0); // Ch4 SUM high counter          (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch5:CntSumHigh",    0x24, 16,  0); // Ch5 SUM high counter          (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch6:CntSumHigh",    0x25, 16,  0); // Ch6 SUM high counter          (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch7:CntSumHigh",    0x26, 16,  0); // Ch7 SUM high counter          (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch0:CntSumLow",     0x27, 16,  0); // Ch0 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch1:CntSumLow",     0x28, 16,  0); // Ch1 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch2:CntSumLow",     0x29, 16,  0); // Ch2 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch3:CntSumLow",     0x2A, 16,  0); // Ch3 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch4:CntSumLow",     0x2B, 16,  0); // Ch4 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch5:CntSumLow",     0x2C, 16,  0); // Ch5 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch6:CntSumLow",     0x2D, 16,  0); // Ch6 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch7:CntSumLow",     0x2E, 16,  0); // Ch7 SUM low counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch0:CntRate",       0x2F, 16,  0); // Ch0 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch1:CntRate",       0x30, 16,  0); // Ch1 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch2:CntRate",       0x31, 16,  0); // Ch2 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch3:CntRate",       0x32, 16,  0); // Ch3 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch4:CntRate",       0x33, 16,  0); // Ch4 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch5:CntRate",       0x34, 16,  0); // Ch5 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch6:CntRate",       0x35, 16,  0); // Ch6 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch7:CntRate",       0x36, 16,  0); // Ch7 outrate counter           (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("CntOutRate",        0x37, 16,  0); // Total outrate counter         (calc:1000*A/52.4288,unit:counts/s,prec:0)
}

void RocPlugin::createConfigParams_v56()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:PositionIdx",  '1', 0x0,  32, 0, 0);     // Chan1 position index
    createConfigParam("Ch2:PositionIdx",  '1', 0x2,  32, 0, 256);   // Chan2 position index
    createConfigParam("Ch3:PositionIdx",  '1', 0x4,  32, 0, 512);   // Chan3 position index
    createConfigParam("Ch4:PositionIdx",  '1', 0x6,  32, 0, 768);   // Chan4 position index
    createConfigParam("Ch5:PositionIdx",  '1', 0x8,  32, 0, 1024);  // Chan5 position index
    createConfigParam("Ch6:PositionIdx",  '1', 0xA,  32, 0, 1280);  // Chan6 position index
    createConfigParam("Ch7:PositionIdx",  '1', 0xC,  32, 0, 1536);  // Chan7 position index
    createConfigParam("Ch8:PositionIdx",  '1', 0xE,  32, 0, 1792);  // Chan8 position index

    createConfigParam("Ch1A:InOffset",    '2', 0x0,  12, 0, 100);   // Chan1 A input offset
    createConfigParam("Ch2A:InOffset",    '2', 0x1,  12, 0, 100);   // Chan2 A input offset
    createConfigParam("Ch3A:InOffset",    '2', 0x2,  12, 0, 100);   // Chan3 A input offset
    createConfigParam("Ch4A:InOffset",    '2', 0x3,  12, 0, 100);   // Chan4 A input offset
    createConfigParam("Ch5A:InOffset",    '2', 0x4,  12, 0, 100);   // Chan5 A input offset
    createConfigParam("Ch6A:InOffset",    '2', 0x5,  12, 0, 100);   // Chan6 A input offset
    createConfigParam("Ch7A:InOffset",    '2', 0x6,  12, 0, 100);   // Chan7 A input offset
    createConfigParam("Ch8A:InOffset",    '2', 0x7,  12, 0, 100);   // Chan8 A input offset
    createConfigParam("Ch1B:InOffset",    '2', 0x8,  12, 0, 100);   // Chan1 B input offset
    createConfigParam("Ch2B:InOffset",    '2', 0x9,  12, 0, 100);   // Chan2 B input offset
    createConfigParam("Ch3B:InOffset",    '2', 0xA,  12, 0, 100);   // Chan3 B input offset
    createConfigParam("Ch4B:InOffset",    '2', 0xB,  12, 0, 100);   // Chan4 B input offset
    createConfigParam("Ch5B:InOffset",    '2', 0xC,  12, 0, 100);   // Chan5 B input offset
    createConfigParam("Ch6B:InOffset",    '2', 0xD,  12, 0, 100);   // Chan6 B input offset
    createConfigParam("Ch7B:InOffset",    '2', 0xE,  12, 0, 100);   // Chan7 B input offset
    createConfigParam("Ch8B:InOffset",    '2', 0xF,  12, 0, 100);   // Chan8 B input offset

    createConfigParam("Ch1A:AdcOffset",   '2', 0x10, 12, 0, 100);   // Chan1 A ADC offset
    createConfigParam("Ch2A:AdcOffset",   '2', 0x11, 12, 0, 100);   // Chan2 A ADC offset
    createConfigParam("Ch3A:AdcOffset",   '2', 0x12, 12, 0, 100);   // Chan3 A ADC offset
    createConfigParam("Ch4A:AdcOffset",   '2', 0x13, 12, 0, 100);   // Chan4 A ADC offset
    createConfigParam("Ch5A:AdcOffset",   '2', 0x14, 12, 0, 100);   // Chan5 A ADC offset
    createConfigParam("Ch6A:AdcOffset",   '2', 0x15, 12, 0, 100);   // Chan6 A ADC offset
    createConfigParam("Ch7A:AdcOffset",   '2', 0x16, 12, 0, 100);   // Chan7 A ADC offset
    createConfigParam("Ch8A:AdcOffset",   '2', 0x17, 12, 0, 100);   // Chan8 A ADC offset
    createConfigParam("Ch1B:AdcOffset",   '2', 0x18, 12, 0, 100);   // Chan1 B ADC offset
    createConfigParam("Ch2B:AdcOffset",   '2', 0x19, 12, 0, 100);   // Chan2 B ADC offset
    createConfigParam("Ch3B:AdcOffset",   '2', 0x1A, 12, 0, 100);   // Chan3 B ADC offset
    createConfigParam("Ch4B:AdcOffset",   '2', 0x1B, 12, 0, 100);   // Chan4 B ADC offset
    createConfigParam("Ch5B:AdcOffset",   '2', 0x1C, 12, 0, 100);   // Chan5 B ADC offset
    createConfigParam("Ch6B:AdcOffset",   '2', 0x1D, 12, 0, 100);   // Chan6 B ADC offset
    createConfigParam("Ch7B:AdcOffset",   '2', 0x1E, 12, 0, 100);   // Chan7 B ADC offset
    createConfigParam("Ch8B:AdcOffset",   '2', 0x1F, 12, 0, 100);   // Chan8 B ADC offset

    createConfigParam("Ch1A:Threshold",   '2', 0x20, 12, 0, 400);   // Chan1 A threshold
    createConfigParam("Ch2A:Threshold",   '2', 0x21, 12, 0, 400);   // Chan2 A threshold
    createConfigParam("Ch3A:Threshold",   '2', 0x22, 12, 0, 400);   // Chan3 A threshold
    createConfigParam("Ch4A:Threshold",   '2', 0x23, 12, 0, 400);   // Chan4 A threshold
    createConfigParam("Ch5A:Threshold",   '2', 0x24, 12, 0, 400);   // Chan5 A threshold
    createConfigParam("Ch6A:Threshold",   '2', 0x25, 12, 0, 400);   // Chan6 A threshold
    createConfigParam("Ch7A:Threshold",   '2', 0x26, 12, 0, 400);   // Chan7 A threshold
    createConfigParam("Ch8A:Threshold",   '2', 0x27, 12, 0, 400);   // Chan8 A threshold
    createConfigParam("Ch1B:Threshold",   '2', 0x28, 12, 0, 400);   // Chan1 B threshold
    createConfigParam("Ch2B:Threshold",   '2', 0x29, 12, 0, 400);   // Chan2 B threshold
    createConfigParam("Ch3B:Threshold",   '2', 0x2A, 12, 0, 400);   // Chan3 B threshold
    createConfigParam("Ch4B:Threshold",   '2', 0x2B, 12, 0, 400);   // Chan4 B threshold
    createConfigParam("Ch5B:Threshold",   '2', 0x2C, 12, 0, 400);   // Chan5 B threshold
    createConfigParam("Ch6B:Threshold",   '2', 0x2D, 12, 0, 400);   // Chan6 B threshold
    createConfigParam("Ch7B:Threshold",   '2', 0x2E, 12, 0, 400);   // Chan7 B threshold
    createConfigParam("Ch8B:Threshold",   '2', 0x2F, 12, 0, 400);   // Chan8 B threshold

    createConfigParam("Ch1:PosThreshold", '2', 0x30, 12, 0, 400);   // Chan1 positive threshold
    createConfigParam("Ch2:PosThreshold", '2', 0x31, 12, 0, 400);   // Chan2 positive threshold
    createConfigParam("Ch3:PosThreshold", '2', 0x32, 12, 0, 400);   // Chan3 positive threshold
    createConfigParam("Ch4:PosThreshold", '2', 0x33, 12, 0, 400);   // Chan4 positive threshold
    createConfigParam("Ch5:PosThreshold", '2', 0x34, 12, 0, 400);   // Chan5 positive threshold
    createConfigParam("Ch6:PosThreshold", '2', 0x35, 12, 0, 400);   // Chan6 positive threshold
    createConfigParam("Ch7:PosThreshold", '2', 0x36, 12, 0, 400);   // Chan7 positive threshold
    createConfigParam("Ch8:PosThreshold", '2', 0x37, 12, 0, 400);   // Chan8 positive threshold
    createConfigParam("Ch1A:FullScale",   '2', 0x38, 12, 0, 10);    // Chan1 A full scale
    createConfigParam("Ch2A:FullScale",   '2', 0x39, 12, 0, 10);    // Chan2 A full scale
    createConfigParam("Ch3A:FullScale",   '2', 0x3A, 12, 0, 10);    // Chan3 A full scale
    createConfigParam("Ch4A:FullScale",   '2', 0x3B, 12, 0, 10);    // Chan4 A full scale
    createConfigParam("Ch5A:FullScale",   '2', 0x3C, 12, 0, 10);    // Chan5 A full scale
    createConfigParam("Ch6A:FullScale",   '2', 0x3D, 12, 0, 10);    // Chan6 A full scale
    createConfigParam("Ch7A:FullScale",   '2', 0x3E, 12, 0, 10);    // Chan7 A full scale
    createConfigParam("Ch8A:FullScale",   '2', 0x3F, 12, 0, 10);    // Chan8 A full scale

    createConfigParam("Ch1B:FullScale",   '2', 0x40, 12, 0, 10);    // Chan1 B full scale
    createConfigParam("Ch2B:FullScale",   '2', 0x41, 12, 0, 10);    // Chan2 B full scale
    createConfigParam("Ch3B:FullScale",   '2', 0x42, 12, 0, 10);    // Chan3 B full scale
    createConfigParam("Ch4B:FullScale",   '2', 0x43, 12, 0, 10);    // Chan4 B full scale
    createConfigParam("Ch5B:FullScale",   '2', 0x44, 12, 0, 10);    // Chan5 B full scale
    createConfigParam("Ch6B:FullScale",   '2', 0x45, 12, 0, 10);    // Chan6 B full scale
    createConfigParam("Ch7B:FullScale",   '2', 0x46, 12, 0, 10);    // Chan7 B full scale
    createConfigParam("Ch8B:FullScale",   '2', 0x47, 12, 0, 10);    // Chan8 B full scale
    createConfigParam("Ch1A:GainAdj",     '2', 0x48, 12, 0, 0);     // Chan1 A gain adjust
    createConfigParam("Ch2A:GainAdj",     '2', 0x49, 12, 0, 0);     // Chan2 A gain adjust
    createConfigParam("Ch3A:GainAdj",     '2', 0x4A, 12, 0, 0);     // Chan3 A gain adjust
    createConfigParam("Ch4A:GainAdj",     '2', 0x4B, 12, 0, 0);     // Chan4 A gain adjust
    createConfigParam("Ch5A:GainAdj",     '2', 0x4C, 12, 0, 0);     // Chan5 A gain adjust
    createConfigParam("Ch6A:GainAdj",     '2', 0x4D, 12, 0, 0);     // Chan6 A gain adjust
    createConfigParam("Ch7A:GainAdj",     '2', 0x4E, 12, 0, 0);     // Chan7 A gain adjust
    createConfigParam("Ch8A:GainAdj",     '2', 0x4F, 12, 0, 0);     // Chan8 A gain adjust

    createConfigParam("Ch1B:GainAdj",     '2', 0x50, 12, 0, 0);     // Chan1 B gain adjust
    createConfigParam("Ch2B:GainAdj",     '2', 0x51, 12, 0, 0);     // Chan2 B gain adjust
    createConfigParam("Ch3B:GainAdj",     '2', 0x52, 12, 0, 0);     // Chan3 B gain adjust
    createConfigParam("Ch4B:GainAdj",     '2', 0x53, 12, 0, 0);     // Chan4 B gain adjust
    createConfigParam("Ch5B:GainAdj",     '2', 0x54, 12, 0, 0);     // Chan5 B gain adjust
    createConfigParam("Ch6B:GainAdj",     '2', 0x55, 12, 0, 0);     // Chan6 B gain adjust
    createConfigParam("Ch7B:GainAdj",     '2', 0x56, 12, 0, 0);     // Chan7 B gain adjust
    createConfigParam("Ch8B:GainAdj",     '2', 0x57, 12, 0, 0);     // Chan8 B gain adjust

    createConfigParam("Ch1A:Scale",       '3', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch2A:Scale",       '3', 0x1,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch3A:Scale",       '3', 0x2,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch4A:Scale",       '3', 0x3,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch5A:Scale",       '3', 0x4,  12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch6A:Scale",       '3', 0x5,  12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch7A:Scale",       '3', 0x6,  12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch8A:Scale",       '3', 0x7,  12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch1B:Scale",       '3', 0x8,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch2B:Scale",       '3', 0x9,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch3B:Scale",       '3', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch4B:Scale",       '3', 0xB,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch5B:Scale",       '3', 0xC,  12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch6B:Scale",       '3', 0xD,  12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch7B:Scale",       '3', 0xE,  12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch8B:Scale",       '3', 0xF,  12, 0, 2048);  // Chan8 B scale

    createConfigParam("Ch1A:Offset",      '3', 0x10, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch2A:Offset",      '3', 0x11, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch3A:Offset",      '3', 0x12, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch4A:Offset",      '3', 0x13, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch5A:Offset",      '3', 0x14, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch6A:Offset",      '3', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch7A:Offset",      '3', 0x16, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch8A:Offset",      '3', 0x17, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch1B:Offset",      '3', 0x18, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2B:Offset",      '3', 0x19, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3B:Offset",      '3', 0x1A, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4B:Offset",      '3', 0x1B, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5B:Offset",      '3', 0x1C, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6B:Offset",      '3', 0x1D, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7B:Offset",      '3', 0x1E, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8B:Offset",      '3', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFact",    '4', 0x0,  12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFact",    '4', 0x1,  12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFact",    '4', 0x2,  12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFact",    '4', 0x3,  12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFact",    '4', 0x4,  12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFact",    '4', 0x5,  12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFact",    '4', 0x6,  12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFact",    '4', 0x7,  12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1A:AvgMin",      'C', 0x0,  12, 0, 100);   // Chan1 A average minimum
    createConfigParam("Ch2A:AvgMin",      'C', 0x1,  12, 0, 100);   // Chan2 A average minimum
    createConfigParam("Ch3A:AvgMin",      'C', 0x2,  12, 0, 100);   // Chan3 A average minimum
    createConfigParam("Ch4A:AvgMin",      'C', 0x3,  12, 0, 100);   // Chan4 A average minimum
    createConfigParam("Ch5A:AvgMin",      'C', 0x4,  12, 0, 100);   // Chan5 A average minimum
    createConfigParam("Ch6A:AvgMin",      'C', 0x5,  12, 0, 100);   // Chan6 A average minimum
    createConfigParam("Ch7A:AvgMin",      'C', 0x6,  12, 0, 100);   // Chan7 A average minimum
    createConfigParam("Ch8A:AvgMin",      'C', 0x7,  12, 0, 100);   // Chan8 A average minimum
    createConfigParam("Ch1B:AvgMin",      'C', 0x8,  12, 0, 100);   // Chan1 B average minimum
    createConfigParam("Ch2B:AvgMin",      'C', 0x9,  12, 0, 100);   // Chan2 B average minimum
    createConfigParam("Ch3B:AvgMin",      'C', 0xA,  12, 0, 100);   // Chan3 B average minimum
    createConfigParam("Ch4B:AvgMin",      'C', 0xB,  12, 0, 100);   // Chan4 B average minimum
    createConfigParam("Ch5B:AvgMin",      'C', 0xC,  12, 0, 100);   // Chan5 B average minimum
    createConfigParam("Ch6B:AvgMin",      'C', 0xD,  12, 0, 100);   // Chan6 B average minimum
    createConfigParam("Ch7B:AvgMin",      'C', 0xE,  12, 0, 100);   // Chan7 B average minimum
    createConfigParam("Ch8B:AvgMin",      'C', 0xF,  12, 0, 100);   // Chan8 B average minimum

    createConfigParam("Ch1A:AvgMax",      'C', 0x10, 12, 0, 1000);  // Chan1 A average maximum
    createConfigParam("Ch2A:AvgMax",      'C', 0x11, 12, 0, 1000);  // Chan2 A average maximum
    createConfigParam("Ch3A:AvgMax",      'C', 0x12, 12, 0, 1000);  // Chan3 A average maximum
    createConfigParam("Ch4A:AvgMax",      'C', 0x13, 12, 0, 1000);  // Chan4 A average maximum
    createConfigParam("Ch5A:AvgMax",      'C', 0x14, 12, 0, 1000);  // Chan5 A average maximum
    createConfigParam("Ch6A:AvgMax",      'C', 0x15, 12, 0, 1000);  // Chan6 A average maximum
    createConfigParam("Ch7A:AvgMax",      'C', 0x16, 12, 0, 1000);  // Chan7 A average maximum
    createConfigParam("Ch8A:AvgMax",      'C', 0x17, 12, 0, 1000);  // Chan8 A average maximum
    createConfigParam("Ch1B:AvgMax",      'C', 0x18, 12, 0, 1000);  // Chan1 B average maximum
    createConfigParam("Ch2B:AvgMax",      'C', 0x19, 12, 0, 1000);  // Chan2 B average maximum
    createConfigParam("Ch3B:AvgMax",      'C', 0x1A, 12, 0, 1000);  // Chan3 B average maximum
    createConfigParam("Ch4B:AvgMax",      'C', 0x1B, 12, 0, 1000);  // Chan4 B average maximum
    createConfigParam("Ch5B:AvgMax",      'C', 0x1C, 12, 0, 1000);  // Chan5 B average maximum
    createConfigParam("Ch6B:AvgMax",      'C', 0x1D, 12, 0, 1000);  // Chan6 B average maximum
    createConfigParam("Ch7B:AvgMax",      'C', 0x1E, 12, 0, 1000);  // Chan7 B average maximum
    createConfigParam("Ch8B:AvgMax",      'C', 0x1F, 12, 0, 1000);  // Chan8 B average maximum

    createConfigParam("Ch1A:SampleMin",   'D', 0x0,  12, 0, 100);   // Chan1 A sample minimum
    createConfigParam("Ch2A:SampleMin",   'D', 0x1,  12, 0, 100);   // Chan2 A sample minimum
    createConfigParam("Ch3A:SampleMin",   'D', 0x2,  12, 0, 100);   // Chan3 A sample minimum
    createConfigParam("Ch4A:SampleMin",   'D', 0x3,  12, 0, 100);   // Chan4 A sample minimum
    createConfigParam("Ch5A:SampleMin",   'D', 0x4,  12, 0, 100);   // Chan5 A sample minimum
    createConfigParam("Ch6A:SampleMin",   'D', 0x5,  12, 0, 100);   // Chan6 A sample minimum
    createConfigParam("Ch7A:SampleMin",   'D', 0x6,  12, 0, 100);   // Chan7 A sample minimum
    createConfigParam("Ch8A:SampleMin",   'D', 0x7,  12, 0, 100);   // Chan8 A sample minimum
    createConfigParam("Ch1B:SampleMin",   'D', 0x8,  12, 0, 100);   // Chan1 B sample minimum
    createConfigParam("Ch2B:SampleMin",   'D', 0x9,  12, 0, 100);   // Chan2 B sample minimum
    createConfigParam("Ch3B:SampleMin",   'D', 0xA,  12, 0, 100);   // Chan3 B sample minimum
    createConfigParam("Ch4B:SampleMin",   'D', 0xB,  12, 0, 100);   // Chan4 B sample minimum
    createConfigParam("Ch5B:SampleMin",   'D', 0xC,  12, 0, 100);   // Chan5 B sample minimum
    createConfigParam("Ch6B:SampleMin",   'D', 0xD,  12, 0, 100);   // Chan6 B sample minimum
    createConfigParam("Ch7B:SampleMin",   'D', 0xE,  12, 0, 100);   // Chan7 B sample minimum
    createConfigParam("Ch8B:SampleMin",   'D', 0xF,  12, 0, 100);   // Chan8 B sample minimum

    createConfigParam("Ch1A:SampleMax",   'D', 0x10, 12, 0, 1000);  // Chan1 A sample maximum
    createConfigParam("Ch2A:SampleMax",   'D', 0x11, 12, 0, 1000);  // Chan2 A sample maximum
    createConfigParam("Ch3A:SampleMax",   'D', 0x12, 12, 0, 1000);  // Chan3 A sample maximum
    createConfigParam("Ch4A:SampleMax",   'D', 0x13, 12, 0, 1000);  // Chan4 A sample maximum
    createConfigParam("Ch5A:SampleMax",   'D', 0x14, 12, 0, 1000);  // Chan5 A sample maximum
    createConfigParam("Ch6A:SampleMax",   'D', 0x15, 12, 0, 1000);  // Chan6 A sample maximum
    createConfigParam("Ch7A:SampleMax",   'D', 0x16, 12, 0, 1000);  // Chan7 A sample maximum
    createConfigParam("Ch8A:SampleMax",   'D', 0x17, 12, 0, 1000);  // Chan8 A sample maximum
    createConfigParam("Ch1B:SampleMax",   'D', 0x18, 12, 0, 1000);  // Chan1 B sample maximum
    createConfigParam("Ch2B:SampleMax",   'D', 0x19, 12, 0, 1000);  // Chan2 B sample maximum
    createConfigParam("Ch3B:SampleMax",   'D', 0x1A, 12, 0, 1000);  // Chan3 B sample maximum
    createConfigParam("Ch4B:SampleMax",   'D', 0x1B, 12, 0, 1000);  // Chan4 B sample maximum
    createConfigParam("Ch5B:SampleMax",   'D', 0x1C, 12, 0, 1000);  // Chan5 B sample maximum
    createConfigParam("Ch6B:SampleMax",   'D', 0x1D, 12, 0, 1000);  // Chan6 B sample maximum
    createConfigParam("Ch7B:SampleMax",   'D', 0x1E, 12, 0, 1000);  // Chan7 B sample maximum
    createConfigParam("Ch8B:SampleMax",   'D', 0x1F, 12, 0, 1000);  // Chan8 B sample maximum

    createConfigParam("Ch8B:SlopeMax",    'D', 0x20, 12, 0, 0);     // Maximum slope

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
    createConfigParam("MinAdc",           'E', 0x6,  16, 0, 100);   // Minimum ADC
    createConfigParam("MaxAdc",           'E', 0x7,  16, 0, 1000);  // Maximum ADC
    createConfigParam("IntRelease",       'E', 0x8,  16, 0, -6);    // Integrator release point
    createConfigParam("MinDiscrimWidth",  'E', 0x9,  12, 0, 1);     // Minimum discriminator width
    createConfigParam("Sample1",          'E', 0xA,  4,  0, 2);     // Sample 1 point [0:10]
    createConfigParam("Sample2",          'E', 0xB,  6,  0, 14);    // Sample 1 point [2:50]
    createConfigParam("TsyncDelay",  'E', 0xC,  32, 0, 0);     // TSYNC delay

    createConfigParam("LvdsRate",         'F', 0x0,  1, 15, 1);     // LVDS output rate              (0=20Mhz,1=40Mhz)
    createConfigParam("AutoCorrectMode",  'F', 0x0,  1, 14, 0);     // Auto correction sample mode   (0=TSYNC,1=fake trigger)
    createConfigParam("EdgeDetectEn",     'F', 0x0,  1, 13, 1);     // Edge detect enable            (0=disable,1=enable)
    createConfigParam("MastDiscrimMode",  'F', 0x0,  2, 11, 0);     // Master discriminator select   (0=SUM discr,1=A discr,2=B discr,3=all)
    createConfigParam("AcquireEn",        'F', 0x0,  1, 10, 1);     // ROC enable acquisition        (0=disable,1=enable)
    createConfigParam("AutoCorrection",   'F', 0x0,  1, 9,  0);     // Auto correction mode          (0=enabled,1=disabled)
    createConfigParam("HighResMode",      'F', 0x0,  1, 8,  1);     // High resolution mode          (0=low res 0-127,1=high res 0-255)
    createConfigParam("OutputMode",       'F', 0x0,  2, 6,  0);     // Output mode                   (0=normal,1=raw,2=extended)
    createConfigParam("AcquireMode",      'F', 0x0,  2, 4,  0);     // Output mode                   (0=normal,1=verbose,2=fake trigger)
    createConfigParam("TxEnable",         'F', 0x0,  1, 3,  1);     // TX enable                     (0=external,1=always enabled)
    createConfigParam("TsyncSelect",      'F', 0x0,  1, 2,  0);     // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TclkSelect",       'F', 0x0,  1, 1,  0);     // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("Reset",            'F', 0x0,  1, 0,  0);     // Reset enable                  (0=disable,1=enable)

    createConfigParam("TestPatternEn",    'F', 0x1,  1, 15, 0);     // Test pattern enable          (0=disable,1=enable)
    createConfigParam("TestPatternRate",  'F', 0x1,  3, 12, 0);     // Test pattern rate
    createConfigParam("TestPatternId",    'F', 0x1, 12, 0,  0);     // Test pattern id
}
