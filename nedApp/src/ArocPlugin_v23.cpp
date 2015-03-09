/* ArocPlugin_v23.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "ArocPlugin.h"

/**
 * @file ArocPlugin_v23.cpp
 *
 * AROC 2.3 parameters
 */
void ArocPlugin::createStatusParams_v23()
{
//    BLXXX:Det:RocXXX:| sig nam|                           | EPICS record description | (bi and mbbi description)
    createStatusParam("ErrParity",          0x0,  1,  0); // LVDS parity error            (0=no error,1=error)
    createStatusParam("ErrType",            0x0,  1,  1); // LVDS packet type error       (0=no error,1=error)
    createStatusParam("ErrLength",          0x0,  1,  2); // LVDS packet length error     (0=no error,1=error)
    createStatusParam("ErrTimeout",         0x0,  1,  3); // LVDS packet timeout          (0=no timeout,1=timeout)
    createStatusParam("ErrNoStart",         0x0,  1,  4); // LVDS data without start      (0=no error,1=error)
    createStatusParam("ErrPreStart",        0x0,  1,  5); // LVDS start before stop bit   (0=no error,1=error)
    createStatusParam("LvdsFifoFull",       0x0,  1,  6); // LVDS FIFO went full          (0=not full,1=full)
    createStatusParam("ErrNoTsync",         0x0,  1,  8); // Timestamp overflow error     (0=no error,1=error)
    createStatusParam("ErrCmdBad",          0x0,  1,  9); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrCmdLength",       0x0,  1, 10); // Command length error         (0=no error,1=error)
    createStatusParam("ErrProgramming",     0x0,  1, 11); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("TimeFifoAlmostFul",  0x0,  1, 12); // Timestamp FIFO almost full   (0=has space,1=almost full)
    createStatusParam("AdcFifoAlmostFul",   0x0,  1, 13); // ADC FIFO almost full         (0=has space,1=almost full)
    createStatusParam("EvFifoAlmostFul",    0x0,  1, 14); // Event FIFO almost full       (0=has space,1=almost full)

    createStatusParam("Ibc1",               0x1,  1,  0); // My IBC = 1                   (0=no,1=yes)
    createStatusParam("Ibc2",               0x1,  1,  1); // My IBC = 2                   (0=no,1=yes)
    createStatusParam("Ibc3",               0x1,  1,  2); // My IBC = 2                   (0=no,1=yes)
    createStatusParam("Ibc4",               0x1,  1,  3); // My IBC = 3                   (0=no,1=yes)
    createStatusParam("Ibc5",               0x1,  1,  4); // My IBC = 4                   (0=no,1=yes)
    createStatusParam("Ibc6",               0x1,  1,  5); // My IBC = 5                   (0=no,1=yes)
    createStatusParam("Ibc7",               0x1,  1,  6); // My IBC = 6                   (0=no,1=yes)
    createStatusParam("Ibc8",               0x1,  1,  7); // My IBC = 7                   (0=no,1=yes)
    createStatusParam("Ibc9",               0x1,  1,  8); // My IBC = 8                   (0=no,1=yes)
    createStatusParam("Configured",         0x1,  2,  9); // Configured                   (0=not configured [alarm],1=full config,2=part config,3=all config)
    createStatusParam("Discovered",         0x1,  1, 11); // Discovered                   (0=not discovered,1=discovered)

    createStatusParam("YDiscrimCnt",        0x2,  8,  0); // Y discriminator hit
    createStatusParam("XDiscrimCnt",        0x2,  8,  8); // X discriminator hit

    createStatusParam("Ibc1:Event",         0x3,  1,  0); // My IBC = 1                   (0=no,1=yes)
    createStatusParam("Ibc2:Event",         0x3,  1,  1); // My IBC = 2                   (0=no,1=yes)
    createStatusParam("Ibc3:Event",         0x3,  1,  2); // My IBC = 2                   (0=no,1=yes)
    createStatusParam("Ibc4:Event",         0x3,  1,  3); // My IBC = 3                   (0=no,1=yes)
    createStatusParam("Ibc5:Event",         0x3,  1,  4); // My IBC = 4                   (0=no,1=yes)
    createStatusParam("Ibc6:Event",         0x3,  1,  5); // My IBC = 5                   (0=no,1=yes)
    createStatusParam("Ibc7:Event",         0x3,  1,  6); // My IBC = 6                   (0=no,1=yes)
    createStatusParam("Ibc8:Event",         0x3,  1,  7); // My IBC = 7                   (0=no,1=yes)
    createStatusParam("Ibc9:Event",         0x3,  1,  8); // My IBC = 8                   (0=no,1=yes)
    createStatusParam("EvFifoHasData",      0x3,  1,  9); // Event FIFO has data          (0=no data,1=has data)
    createStatusParam("EvFifoAlmostFull",   0x3,  1, 10); // Event FIFO almost full       (0=has space,1=almost full)
    createStatusParam("RawFifoHasData",     0x3,  1, 11); // Raw FIFO has data            (0=no data,1=has data)
    createStatusParam("RawFifoAlmostFull",  0x3,  1, 12); // Raw FIFO almost full         (0=has space,1=almost full)
    createStatusParam("Acquiring",          0x3,  1, 13); // Acquiring data               (0=not acquiring [alarm],1=acquiring)
    createStatusParam("LvdsVerify",         0x3,  1, 14); // LVDS VERIFY detected         (0=not detected,1=detected)

    createStatusParam("YFifoHasData",       0x4,  8,  0); // Y FIFO has data
    createStatusParam("XFifoHasData",       0x4,  8,  8); // X FIFO has data

    createStatusParam("TclkLow",            0x5,  1,  0); // TCLK Got LOW                 (0=no,1=yes)
    createStatusParam("TclkHigh",           0x5,  1,  1); // TCLK Got HIGH                (0=no,1=yes)
    createStatusParam("TsyncLow",           0x5,  1,  2); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TsyncHigh",          0x5,  1,  3); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("TxenBLow",           0x5,  1,  4); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("TxenBHigh",          0x5,  1,  5); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("SysrstBLow",         0x5,  1,  6); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("SysrstBHigh",        0x5,  1,  7); // SYSRST_B Got HIGH            (0=no,1=yes)

    createStatusParam("X1:AdcOffsetVal",    0x6,  8,  0); // X1 ADC Offset value
    createStatusParam("X1:InputOffset",     0x6,  9,  8); // X1 Input Offset value
    createStatusParam("X1:AutoAdjOverflow", 0x7,  1,  1); // X1 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X1:AutoAdjSlope1",   0x7,  1,  2); // X1 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X1:AutoAdjOffset",   0x7,  1,  3); // X1 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X1:AutoAdjSlope",    0x7,  1,  4); // X1 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X1:AutoAdjSample",   0x7,  1,  5); // X1 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X1:AutoAdjTrig",     0x7,  1,  6); // X1 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X1:AutoAdjEn",       0x7,  1,  7); // X1 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X2:AdcOffsetVal",    0x7,  8,  8); // X2 ADC Offset value
    createStatusParam("X2:InputOffset",     0x8,  9,  0); // X2 ADC Offset value
    createStatusParam("X2:AutoAdjOverflow", 0x8,  1,  9); // X2 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X2:AutoAdjSlope1",   0x8,  1, 10); // X2 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X2:AutoAdjOffset",   0x8,  1, 11); // X2 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X2:AutoAdjSlope",    0x8,  1, 12); // X2 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X2:AutoAdjSample",   0x8,  1, 13); // X2 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X2:AutoAdjTrig",     0x8,  1, 14); // X2 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X2:AutoAdjEn",       0x8,  1, 15); // X2 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X3:AdcOffsetVal",    0x9,  8,  0); // X3 ADC Offset value
    createStatusParam("X3:InputOffset",     0x9,  9,  8); // X3 Input Offset value
    createStatusParam("X3:AutoAdjOverflow", 0xA,  1,  1); // X3 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X3:AutoAdjSlope1",   0xA,  1,  2); // X3 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X3:AutoAdjOffset",   0xA,  1,  3); // X3 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X3:AutoAdjSlope",    0xA,  1,  4); // X3 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X3:AutoAdjSample",   0xA,  1,  5); // X3 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X3:AutoAdjTrig",     0xA,  1,  6); // X3 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X3:AutoAdjEn",       0xA,  1,  7); // X3 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X4:AdcOffsetVal",    0xA,  8,  8); // X4 ADC Offset value
    createStatusParam("X4:InputOffset",     0xB,  9,  0); // X4 ADC Offset value
    createStatusParam("X4:AutoAdjOverflow", 0xB,  1,  9); // X4 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X4:AutoAdjSlope1",   0xB,  1, 10); // X4 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X4:AutoAdjOffset",   0xB,  1, 11); // X4 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X4:AutoAdjSlope",    0xB,  1, 12); // X4 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X4:AutoAdjSample",   0xB,  1, 13); // X4 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X4:AutoAdjTrig",     0xB,  1, 14); // X4 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X4:AutoAdjEn",       0xB,  1, 15); // X4 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X5:AdcOffsetVal",    0xC,  8,  0); // X5 ADC Offset value
    createStatusParam("X5:InputOffset",     0xC,  9,  8); // X5 Input Offset value
    createStatusParam("X5:AutoAdjOverflow", 0xD,  1,  1); // X5 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X5:AutoAdjSlope1",   0xD,  1,  2); // X5 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X5:AutoAdjOffset",   0xD,  1,  3); // X5 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X5:AutoAdjSlope",    0xD,  1,  4); // X5 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X5:AutoAdjSample",   0xD,  1,  5); // X5 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X5:AutoAdjTrig",     0xD,  1,  6); // X5 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X5:AutoAdjEn",       0xD,  1,  7); // X5 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X6:AdcOffsetVal",    0xD,  8,  8); // X6 ADC Offset value
    createStatusParam("X6:InputOffset",     0xE,  9,  0); // X6 ADC Offset value
    createStatusParam("X6:AutoAdjOverflow", 0xE,  1,  9); // X6 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X6:AutoAdjSlope1",   0xE,  1, 10); // X6 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X6:AutoAdjOffset",   0xE,  1, 11); // X6 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X6:AutoAdjSlope",    0xE,  1, 12); // X6 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X6:AutoAdjSample",   0xE,  1, 13); // X6 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X6:AutoAdjTrig",     0xE,  1, 14); // X6 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X6:AutoAdjEn",       0xE,  1, 15); // X6 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X7:AdcOffsetVal",    0xF,  8,  0); // X7 ADC Offset value
    createStatusParam("X7:InputOffset",     0xF,  9,  8); // X7 Input Offset value
    createStatusParam("X7:AutoAdjOverflow",0x10, 1,  1); // X7 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X7:AutoAdjSlope1",  0x10, 1,  2); // X7 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X7:AutoAdjOffset",  0x10, 1,  3); // X7 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X7:AutoAdjSlope",   0x10, 1,  4); // X7 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X7:AutoAdjSample",  0x10, 1,  5); // X7 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X7:AutoAdjTrig",    0x10, 1,  6); // X7 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X7:AutoAdjEn",      0x10, 1,  7); // X7 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("X8:AdcOffsetVal",   0x10, 8,  8); // X8 ADC Offset value
    createStatusParam("X8:InputOffset",    0x11, 9,  0); // X8 ADC Offset value
    createStatusParam("X8:AutoAdjOverflow",0x11, 1,  9); // X8 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("X8:AutoAdjSlope1",  0x11, 1, 10); // X8 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("X8:AutoAdjOffset",  0x11, 1, 11); // X8 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("X8:AutoAdjSlope",   0x11, 1, 12); // X8 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("X8:AutoAdjSample",  0x11, 1, 13); // X8 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("X8:AutoAdjTrig",    0x11, 1, 14); // X8 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("X8:AutoAdjEn",      0x11, 1, 15); // X8 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y1:AdcOffsetVal",   0x12,  8,  0); // Y1 ADC Offset value
    createStatusParam("Y1:InputOffset",    0x12,  9,  8); // Y1 Input Offset value
    createStatusParam("Y1:AutoAdjOverflow",0x13,  1,  1); // Y1 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y1:AutoAdjSlope1",  0x13,  1,  2); // Y1 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y1:AutoAdjOffset",  0x13,  1,  3); // Y1 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y1:AutoAdjSlope",   0x13,  1,  4); // Y1 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y1:AutoAdjSample",  0x13,  1,  5); // Y1 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y1:AutoAdjTrig",    0x13,  1,  6); // Y1 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y1:AutoAdjEn",      0x13,  1,  7); // Y1 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y2:AdcOffsetVal",   0x13,  8,  8); // Y2 ADC Offset value
    createStatusParam("Y2:InputOffset",    0x14,  9,  0); // Y2 ADC Offset value
    createStatusParam("Y2:AutoAdjOverflow",0x14,  1,  9); // Y2 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y2:AutoAdjSlope1",  0x14,  1, 10); // Y2 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y2:AutoAdjOffset",  0x14,  1, 11); // Y2 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y2:AutoAdjSlope",   0x14,  1, 12); // Y2 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y2:AutoAdjSample",  0x14,  1, 13); // Y2 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y2:AutoAdjTrig",    0x14,  1, 14); // Y2 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y2:AutoAdjEn",      0x14,  1, 15); // Y2 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y3:AdcOffsetVal",   0x15,  8,  0); // Y3 ADC Offset value
    createStatusParam("Y3:InputOffset",    0x15,  9,  8); // Y3 Input Offset value
    createStatusParam("Y3:AutoAdjOverflow",0x16,  1,  1); // Y3 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y3:AutoAdjSlope1",  0x16,  1,  2); // Y3 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y3:AutoAdjOffset",  0x16,  1,  3); // Y3 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y3:AutoAdjSlope",   0x16,  1,  4); // Y3 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y3:AutoAdjSample",  0x16,  1,  5); // Y3 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y3:AutoAdjTrig",    0x16,  1,  6); // Y3 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y3:AutoAdjEn",      0x16,  1,  7); // Y3 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y4:AdcOffsetVal",   0x16,  8,  8); // Y4 ADC Offset value
    createStatusParam("Y4:InputOffset",    0x17,  9,  0); // Y4 ADC Offset value
    createStatusParam("Y4:AutoAdjOverflow",0x17,  1,  9); // Y4 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y4:AutoAdjSlope1",  0x17,  1, 10); // Y4 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y4:AutoAdjOffset",  0x17,  1, 11); // Y4 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y4:AutoAdjSlope",   0x17,  1, 12); // Y4 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y4:AutoAdjSample",  0x17,  1, 13); // Y4 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y4:AutoAdjTrig",    0x17,  1, 14); // Y4 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y4:AutoAdjEn",      0x17,  1, 15); // Y4 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y5:AdcOffsetVal",   0x18,  8,  0); // Y5 ADC Offset value
    createStatusParam("Y5:InputOffset",    0x18,  9,  8); // Y5 Input Offset value
    createStatusParam("Y5:AutoAdjOverflow",0x19,  1,  1); // Y5 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y5:AutoAdjSlope1",  0x19,  1,  2); // Y5 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y5:AutoAdjOffset",  0x19,  1,  3); // Y5 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y5:AutoAdjSlope",   0x19,  1,  4); // Y5 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y5:AutoAdjSample",  0x19,  1,  5); // Y5 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y5:AutoAdjTrig",    0x19,  1,  6); // Y5 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y5:AutoAdjEn",      0x19,  1,  7); // Y5 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y6:AdcOffsetVal",   0x19,  8,  8); // Y6 ADC Offset value
    createStatusParam("Y6:InputOffset",    0x1A,  9,  0); // Y6 ADC Offset value
    createStatusParam("Y6:AutoAdjOverflow",0x1A,  1,  9); // Y6 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y6:AutoAdjSlope1",  0x1A,  1, 10); // Y6 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y6:AutoAdjOffset",  0x1A,  1, 11); // Y6 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y6:AutoAdjSlope",   0x1A,  1, 12); // Y6 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y6:AutoAdjSample",  0x1A,  1, 13); // Y6 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y6:AutoAdjTrig",    0x1A,  1, 14); // Y6 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y6:AutoAdjEn",      0x1A,  1, 15); // Y6 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y7:AdcOffsetVal",   0x1B,  8,  0); // Y7 ADC Offset value
    createStatusParam("Y7:InputOffset",    0x1B,  9,  8); // Y7 Input Offset value
    createStatusParam("Y7:AutoAdjOverflow",0x1C,  1,  1); // Y7 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y7:AutoAdjSlope1",  0x1C,  1,  2); // Y7 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y7:AutoAdjOffset",  0x1C,  1,  3); // Y7 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y7:AutoAdjSlope",   0x1C,  1,  4); // Y7 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y7:AutoAdjSample",  0x1C,  1,  5); // Y7 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y7:AutoAdjTrig",    0x1C,  1,  6); // Y7 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y7:AutoAdjEn",      0x1C,  1,  7); // Y7 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Y8:AdcOffsetVal",   0x1C,  8,  8); // Y8 ADC Offset value
    createStatusParam("Y8:InputOffset",    0x1D,  9,  0); // Y8 ADC Offset value
    createStatusParam("Y8:AutoAdjOverflow",0x1D,  1,  9); // Y8 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Y8:AutoAdjSlope1",  0x1D,  1, 10); // Y8 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Y8:AutoAdjOffset",  0x1D,  1, 11); // Y8 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Y8:AutoAdjSlope",   0x1D,  1, 12); // Y8 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Y8:AutoAdjSample",  0x1D,  1, 13); // Y8 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Y8:AutoAdjTrig",    0x1D,  1, 14); // Y8 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Y8:AutoAdjEn",      0x1D,  1, 15); // Y8 Auto-Adjust Active        (0=not active,1=active)
}

void ArocPlugin::createConfigParams_v23()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionIdx",      '1',  0x0, 32,  0,    0); // Chan1 position index

    createConfigParam("Ch1:A:InOffset",   '2',  0x0, 12,  0,  100); // Chan1 A input offset
    createConfigParam("Ch2:A:InOffset",   '2',  0x1, 12,  0,  100); // Chan2 A input offset
    createConfigParam("Ch3:A:InOffset",   '2',  0x2, 12,  0,  100); // Chan3 A input offset
    createConfigParam("Ch4:A:InOffset",   '2',  0x3, 12,  0,  100); // Chan4 A input offset
    createConfigParam("Ch5:A:InOffset",   '2',  0x4, 12,  0,  100); // Chan5 A input offset
    createConfigParam("Ch6:A:InOffset",   '2',  0x5, 12,  0,  100); // Chan6 A input offset
    createConfigParam("Ch7:A:InOffset",   '2',  0x6, 12,  0,  100); // Chan7 A input offset
    createConfigParam("Ch8:A:InOffset",   '2',  0x7, 12,  0,  100); // Chan8 A input offset
    createConfigParam("Ch1:B:InOffset",   '2',  0x8, 12,  0,  100); // Chan1 B input offset
    createConfigParam("Ch2:B:InOffset",   '2',  0x9, 12,  0,  100); // Chan2 B input offset
    createConfigParam("Ch3:B:InOffset",   '2',  0xA, 12,  0,  100); // Chan3 B input offset
    createConfigParam("Ch4:B:InOffset",   '2',  0xB, 12,  0,  100); // Chan4 B input offset
    createConfigParam("Ch5:B:InOffset",   '2',  0xC, 12,  0,  100); // Chan5 B input offset
    createConfigParam("Ch6:B:InOffset",   '2',  0xD, 12,  0,  100); // Chan6 B input offset
    createConfigParam("Ch7:B:InOffset",   '2',  0xE, 12,  0,  100); // Chan7 B input offset
    createConfigParam("Ch8:B:InOffset",   '2',  0xF, 12,  0,  100); // Chan8 B input offset

    createConfigParam("Ch1:A:AdcOffset",  '2', 0x10, 12,  0,  100); // Chan1 A ADC offset
    createConfigParam("Ch2:A:AdcOffset",  '2', 0x11, 12,  0,  100); // Chan2 A ADC offset
    createConfigParam("Ch3:A:AdcOffset",  '2', 0x12, 12,  0,  100); // Chan3 A ADC offset
    createConfigParam("Ch4:A:AdcOffset",  '2', 0x13, 12,  0,  100); // Chan4 A ADC offset
    createConfigParam("Ch5:A:AdcOffset",  '2', 0x14, 12,  0,  100); // Chan5 A ADC offset
    createConfigParam("Ch6:A:AdcOffset",  '2', 0x15, 12,  0,  100); // Chan6 A ADC offset
    createConfigParam("Ch7:A:AdcOffset",  '2', 0x16, 12,  0,  100); // Chan7 A ADC offset
    createConfigParam("Ch8:A:AdcOffset",  '2', 0x17, 12,  0,  100); // Chan8 A ADC offset
    createConfigParam("Ch1:B:AdcOffset",  '2', 0x18, 12,  0,  100); // Chan1 B ADC offset
    createConfigParam("Ch2:B:AdcOffset",  '2', 0x19, 12,  0,  100); // Chan2 B ADC offset
    createConfigParam("Ch3:B:AdcOffset",  '2', 0x1A, 12,  0,  100); // Chan3 B ADC offset
    createConfigParam("Ch4:B:AdcOffset",  '2', 0x1B, 12,  0,  100); // Chan4 B ADC offset
    createConfigParam("Ch5:B:AdcOffset",  '2', 0x1C, 12,  0,  100); // Chan5 B ADC offset
    createConfigParam("Ch6:B:AdcOffset",  '2', 0x1D, 12,  0,  100); // Chan6 B ADC offset
    createConfigParam("Ch7:B:AdcOffset",  '2', 0x1E, 12,  0,  100); // Chan7 B ADC offset
    createConfigParam("Ch8:B:AdcOffset",  '2', 0x1F, 12,  0,  100); // Chan8 B ADC offset

    createConfigParam("Ch1:A:Threshold",  '2', 0x20, 12,  0, 1725); // Chan1 A threshold
    createConfigParam("Ch2:A:Threshold",  '2', 0x21, 12,  0, 1725); // Chan2 A threshold
    createConfigParam("Ch3:A:Threshold",  '2', 0x22, 12,  0, 1725); // Chan3 A threshold
    createConfigParam("Ch4:A:Threshold",  '2', 0x23, 12,  0, 1725); // Chan4 A threshold
    createConfigParam("Ch5:A:Threshold",  '2', 0x24, 12,  0, 1725); // Chan5 A threshold
    createConfigParam("Ch6:A:Threshold",  '2', 0x25, 12,  0, 1725); // Chan6 A threshold
    createConfigParam("Ch7:A:Threshold",  '2', 0x26, 12,  0, 1725); // Chan7 A threshold
    createConfigParam("Ch8:A:Threshold",  '2', 0x27, 12,  0, 1725); // Chan8 A threshold
    createConfigParam("Ch1:B:Threshold",  '2', 0x28, 12,  0, 1725); // Chan1 B threshold
    createConfigParam("Ch2:B:Threshold",  '2', 0x29, 12,  0, 1725); // Chan2 B threshold
    createConfigParam("Ch3:B:Threshold",  '2', 0x2A, 12,  0, 1725); // Chan3 B threshold
    createConfigParam("Ch4:B:Threshold",  '2', 0x2B, 12,  0, 1725); // Chan4 B threshold
    createConfigParam("Ch5:B:Threshold",  '2', 0x2C, 12,  0, 1725); // Chan5 B threshold
    createConfigParam("Ch6:B:Threshold",  '2', 0x2D, 12,  0, 1725); // Chan6 B threshold
    createConfigParam("Ch7:B:Threshold",  '2', 0x2E, 12,  0, 1725); // Chan7 B threshold
    createConfigParam("Ch8:B:Threshold",  '2', 0x2F, 12,  0, 1725); // Chan8 B threshold

    createConfigParam("Ch1:A:FullScale",  '2', 0x30, 12,  0,    0); // Chan1 A full scale
    createConfigParam("Ch2:A:FullScale",  '2', 0x31, 12,  0,    0); // Chan2 A full scale
    createConfigParam("Ch3:A:FullScale",  '2', 0x32, 12,  0,    0); // Chan3 A full scale
    createConfigParam("Ch4:A:FullScale",  '2', 0x33, 12,  0,    0); // Chan4 A full scale
    createConfigParam("Ch5:A:FullScale",  '2', 0x34, 12,  0,    0); // Chan5 A full scale
    createConfigParam("Ch6:A:FullScale",  '2', 0x35, 12,  0,    0); // Chan6 A full scale
    createConfigParam("Ch7:A:FullScale",  '2', 0x36, 12,  0,    0); // Chan7 A full scale
    createConfigParam("Ch8:A:FullScale",  '2', 0x37, 12,  0,    0); // Chan8 A full scale
    createConfigParam("Ch1:B:FullScale",  '2', 0x38, 12,  0,    0); // Chan1 B full scale
    createConfigParam("Ch2:B:FullScale",  '2', 0x39, 12,  0,    0); // Chan2 B full scale
    createConfigParam("Ch3:B:FullScale",  '2', 0x3A, 12,  0,    0); // Chan3 B full scale
    createConfigParam("Ch4:B:FullScale",  '2', 0x3B, 12,  0,    0); // Chan4 B full scale
    createConfigParam("Ch5:B:FullScale",  '2', 0x3C, 12,  0,    0); // Chan5 B full scale
    createConfigParam("Ch6:B:FullScale",  '2', 0x3D, 12,  0,    0); // Chan6 B full scale
    createConfigParam("Ch7:B:FullScale",  '2', 0x3E, 12,  0,    0); // Chan7 B full scale
    createConfigParam("Ch8:B:FullScale",  '2', 0x3F, 12,  0,    0); // Chan8 B full scale

    createConfigParam("Ch1:A:GainAdj",    '2', 0x40, 12,  0,    0); // Chan1 A gain adjust
    createConfigParam("Ch2:A:GainAdj",    '2', 0x41, 12,  0,    0); // Chan2 A gain adjust
    createConfigParam("Ch3:A:GainAdj",    '2', 0x42, 12,  0,    0); // Chan3 A gain adjust
    createConfigParam("Ch4:A:GainAdj",    '2', 0x43, 12,  0,    0); // Chan4 A gain adjust
    createConfigParam("Ch5:A:GainAdj",    '2', 0x44, 12,  0,    0); // Chan5 A gain adjust
    createConfigParam("Ch6:A:GainAdj",    '2', 0x45, 12,  0,    0); // Chan6 A gain adjust
    createConfigParam("Ch7:A:GainAdj",    '2', 0x46, 12,  0,    0); // Chan7 A gain adjust
    createConfigParam("Ch8:A:GainAdj",    '2', 0x47, 12,  0,    0); // Chan8 A gain adjust
    createConfigParam("Ch1:B:GainAdj",    '2', 0x48, 12,  0,    0); // Chan1 B gain adjust
    createConfigParam("Ch2:B:GainAdj",    '2', 0x49, 12,  0,    0); // Chan2 B gain adjust
    createConfigParam("Ch3:B:GainAdj",    '2', 0x4A, 12,  0,    0); // Chan3 B gain adjust
    createConfigParam("Ch4:B:GainAdj",    '2', 0x4B, 12,  0,    0); // Chan4 B gain adjust
    createConfigParam("Ch5:B:GainAdj",    '2', 0x4C, 12,  0,    0); // Chan5 B gain adjust
    createConfigParam("Ch6:B:GainAdj",    '2', 0x4D, 12,  0,    0); // Chan6 B gain adjust
    createConfigParam("Ch7:B:GainAdj",    '2', 0x4E, 12,  0,    0); // Chan7 B gain adjust
    createConfigParam("Ch8:B:GainAdj",    '2', 0x4F, 12,  0,    0); // Chan8 B gain adjust

    createConfigParam("Ch1:A:Scale",      'C',  0x0, 12,  0,    0); // Chan1 A scale
    createConfigParam("Ch2:A:Scale",      'C',  0x1, 12,  0,    0); // Chan2 A scale
    createConfigParam("Ch3:A:Scale",      'C',  0x2, 12,  0,    0); // Chan3 A scale
    createConfigParam("Ch4:A:Scale",      'C',  0x3, 12,  0,    0); // Chan4 A scale
    createConfigParam("Ch5:A:Scale",      'C',  0x4, 12,  0,    0); // Chan5 A scale
    createConfigParam("Ch6:A:Scale",      'C',  0x5, 12,  0,    0); // Chan6 A scale
    createConfigParam("Ch7:A:Scale",      'C',  0x6, 12,  0,    0); // Chan7 A scale
    createConfigParam("Ch8:A:Scale",      'C',  0x7, 12,  0,    0); // Chan8 A scale
    createConfigParam("Ch1:B:Scale",      'C',  0x8, 12,  0,    0); // Chan1 B scale
    createConfigParam("Ch2:B:Scale",      'C',  0x9, 12,  0,    0); // Chan2 B scale
    createConfigParam("Ch3:B:Scale",      'C',  0xA, 12,  0,    0); // Chan3 B scale
    createConfigParam("Ch4:B:Scale",      'C',  0xB, 12,  0,    0); // Chan4 B scale
    createConfigParam("Ch5:B:Scale",      'C',  0xC, 12,  0,    0); // Chan5 B scale
    createConfigParam("Ch6:B:Scale",      'C',  0xD, 12,  0,    0); // Chan6 B scale
    createConfigParam("Ch7:B:Scale",      'C',  0xE, 12,  0,    0); // Chan7 B scale
    createConfigParam("Ch8:B:Scale",      'C',  0xF, 12,  0,    0); // Chan8 B scale

    createConfigParam("Ch1:A:Offset",     'C', 0x10, 12,  0,    0); // Chan1 A Offset
    createConfigParam("Ch2:A:Offset",     'C', 0x11, 12,  0,    0); // Chan2 A Offset
    createConfigParam("Ch3:A:Offset",     'C', 0x12, 12,  0,    0); // Chan3 A Offset
    createConfigParam("Ch4:A:Offset",     'C', 0x13, 12,  0,    0); // Chan4 A Offset
    createConfigParam("Ch5:A:Offset",     'C', 0x14, 12,  0,    0); // Chan5 A Offset
    createConfigParam("Ch6:A:Offset",     'C', 0x15, 12,  0,    0); // Chan6 A Offset
    createConfigParam("Ch7:A:Offset",     'C', 0x16, 12,  0,    0); // Chan7 A Offset
    createConfigParam("Ch8:A:Offset",     'C', 0x17, 12,  0,    0); // Chan8 A Offset
    createConfigParam("Ch1:B:Offset",     'C', 0x18, 12,  0,    0); // Chan1 B Offset
    createConfigParam("Ch2:B:Offset",     'C', 0x19, 12,  0,    0); // Chan2 B Offset
    createConfigParam("Ch3:B:Offset",     'C', 0x1A, 12,  0,    0); // Chan3 B Offset
    createConfigParam("Ch4:B:Offset",     'C', 0x1B, 12,  0,    0); // Chan4 B Offset
    createConfigParam("Ch5:B:Offset",     'C', 0x1C, 12,  0,    0); // Chan5 B Offset
    createConfigParam("Ch6:B:Offset",     'C', 0x1D, 12,  0,    0); // Chan6 B Offset
    createConfigParam("Ch7:B:Offset",     'C', 0x1E, 12,  0,    0); // Chan7 B Offset
    createConfigParam("Ch8:B:Offset",     'C', 0x1F, 12,  0,    0); // Chan8 B Offset

    createConfigParam("Ch1:A:AvgMin",     'D',  0x0, 12,  0,  100); // Chan1 A average minimum
    createConfigParam("Ch2:A:AvgMin",     'D',  0x1, 12,  0,  100); // Chan2 A average minimum
    createConfigParam("Ch3:A:AvgMin",     'D',  0x2, 12,  0,  100); // Chan3 A average minimum
    createConfigParam("Ch4:A:AvgMin",     'D',  0x3, 12,  0,  100); // Chan4 A average minimum
    createConfigParam("Ch5:A:AvgMin",     'D',  0x4, 12,  0,  100); // Chan5 A average minimum
    createConfigParam("Ch6:A:AvgMin",     'D',  0x5, 12,  0,  100); // Chan6 A average minimum
    createConfigParam("Ch7:A:AvgMin",     'D',  0x6, 12,  0,  100); // Chan7 A average minimum
    createConfigParam("Ch8:A:AvgMin",     'D',  0x7, 12,  0,  100); // Chan8 A average minimum
    createConfigParam("Ch1:B:AvgMin",     'D',  0x8, 12,  0,  100); // Chan1 B average minimum
    createConfigParam("Ch2:B:AvgMin",     'D',  0x9, 12,  0,  100); // Chan2 B average minimum
    createConfigParam("Ch3:B:AvgMin",     'D',  0xA, 12,  0,  100); // Chan3 B average minimum
    createConfigParam("Ch4:B:AvgMin",     'D',  0xB, 12,  0,  100); // Chan4 B average minimum
    createConfigParam("Ch5:B:AvgMin",     'D',  0xC, 12,  0,  100); // Chan5 B average minimum
    createConfigParam("Ch6:B:AvgMin",     'D',  0xD, 12,  0,  100); // Chan6 B average minimum
    createConfigParam("Ch7:B:AvgMin",     'D',  0xE, 12,  0,  100); // Chan7 B average minimum
    createConfigParam("Ch8:B:AvgMin",     'D',  0xF, 12,  0,  100); // Chan8 B average minimum

    createConfigParam("Ch1:A:AvgMax",     'D', 0x10, 12,  0, 4095); // Chan1 A average maximum
    createConfigParam("Ch2:A:AvgMax",     'D', 0x11, 12,  0, 4095); // Chan2 A average maximum
    createConfigParam("Ch3:A:AvgMax",     'D', 0x12, 12,  0, 4095); // Chan3 A average maximum
    createConfigParam("Ch4:A:AvgMax",     'D', 0x13, 12,  0, 4095); // Chan4 A average maximum
    createConfigParam("Ch5:A:AvgMax",     'D', 0x14, 12,  0, 4095); // Chan5 A average maximum
    createConfigParam("Ch6:A:AvgMax",     'D', 0x15, 12,  0, 4095); // Chan6 A average maximum
    createConfigParam("Ch7:A:AvgMax",     'D', 0x16, 12,  0, 4095); // Chan7 A average maximum
    createConfigParam("Ch8:A:AvgMax",     'D', 0x17, 12,  0, 4095); // Chan8 A average maximum
    createConfigParam("Ch1:B:AvgMax",     'D', 0x18, 12,  0, 4095); // Chan1 B average maximum
    createConfigParam("Ch2:B:AvgMax",     'D', 0x19, 12,  0, 4095); // Chan2 B average maximum
    createConfigParam("Ch3:B:AvgMax",     'D', 0x1A, 12,  0, 4095); // Chan3 B average maximum
    createConfigParam("Ch4:B:AvgMax",     'D', 0x1B, 12,  0, 4095); // Chan4 B average maximum
    createConfigParam("Ch5:B:AvgMax",     'D', 0x1C, 12,  0, 4095); // Chan5 B average maximum
    createConfigParam("Ch6:B:AvgMax",     'D', 0x1D, 12,  0, 4095); // Chan6 B average maximum
    createConfigParam("Ch7:B:AvgMax",     'D', 0x1E, 12,  0, 4095); // Chan7 B average maximum
    createConfigParam("Ch8:B:AvgMax",     'D', 0x1F, 12,  0, 4095); // Chan8 B average maximum

    createConfigParam("Ch1:A:SampleMin",  'D', 0x20, 12,  0,  500); // Chan1 A sample minimum
    createConfigParam("Ch2:A:SampleMin",  'D', 0x21, 12,  0,  500); // Chan2 A sample minimum
    createConfigParam("Ch3:A:SampleMin",  'D', 0x22, 12,  0,  500); // Chan3 A sample minimum
    createConfigParam("Ch4:A:SampleMin",  'D', 0x23, 12,  0,  500); // Chan4 A sample minimum
    createConfigParam("Ch5:A:SampleMin",  'D', 0x24, 12,  0,  500); // Chan5 A sample minimum
    createConfigParam("Ch6:A:SampleMin",  'D', 0x25, 12,  0,  500); // Chan6 A sample minimum
    createConfigParam("Ch7:A:SampleMin",  'D', 0x26, 12,  0,  500); // Chan7 A sample minimum
    createConfigParam("Ch8:A:SampleMin",  'D', 0x27, 12,  0,  500); // Chan8 A sample minimum
    createConfigParam("Ch1:B:SampleMin",  'D', 0x28, 12,  0,  500); // Chan1 B sample minimum
    createConfigParam("Ch2:B:SampleMin",  'D', 0x29, 12,  0,  500); // Chan2 B sample minimum
    createConfigParam("Ch3:B:SampleMin",  'D', 0x2A, 12,  0,  500); // Chan3 B sample minimum
    createConfigParam("Ch4:B:SampleMin",  'D', 0x2B, 12,  0,  500); // Chan4 B sample minimum
    createConfigParam("Ch5:B:SampleMin",  'D', 0x2C, 12,  0,  500); // Chan5 B sample minimum
    createConfigParam("Ch6:B:SampleMin",  'D', 0x2D, 12,  0,  500); // Chan6 B sample minimum
    createConfigParam("Ch7:B:SampleMin",  'D', 0x2E, 12,  0,  500); // Chan7 B sample minimum
    createConfigParam("Ch8:B:SampleMin",  'D', 0x2F, 12,  0,  500); // Chan8 B sample minimum

    createConfigParam("Ch1:A:SampleMax",  'D', 0x30, 12,  0, 4095); // Chan1 A sample maximum
    createConfigParam("Ch2:A:SampleMax",  'D', 0x31, 12,  0, 4095); // Chan2 A sample maximum
    createConfigParam("Ch3:A:SampleMax",  'D', 0x32, 12,  0, 4095); // Chan3 A sample maximum
    createConfigParam("Ch4:A:SampleMax",  'D', 0x33, 12,  0, 4095); // Chan4 A sample maximum
    createConfigParam("Ch5:A:SampleMax",  'D', 0x34, 12,  0, 4095); // Chan5 A sample maximum
    createConfigParam("Ch6:A:SampleMax",  'D', 0x35, 12,  0, 4095); // Chan6 A sample maximum
    createConfigParam("Ch7:A:SampleMax",  'D', 0x36, 12,  0, 4095); // Chan7 A sample maximum
    createConfigParam("Ch8:A:SampleMax",  'D', 0x37, 12,  0, 4095); // Chan8 A sample maximum
    createConfigParam("Ch1:B:SampleMax",  'D', 0x38, 12,  0, 4095); // Chan1 B sample maximum
    createConfigParam("Ch2:B:SampleMax",  'D', 0x39, 12,  0, 4095); // Chan2 B sample maximum
    createConfigParam("Ch3:B:SampleMax",  'D', 0x3A, 12,  0, 4095); // Chan3 B sample maximum
    createConfigParam("Ch4:B:SampleMax",  'D', 0x3B, 12,  0, 4095); // Chan4 B sample maximum
    createConfigParam("Ch5:B:SampleMax",  'D', 0x3C, 12,  0, 4095); // Chan5 B sample maximum
    createConfigParam("Ch6:B:SampleMax",  'D', 0x3D, 12,  0, 4095); // Chan6 B sample maximum
    createConfigParam("Ch7:B:SampleMax",  'D', 0x3E, 12,  0, 4095); // Chan7 B sample maximum
    createConfigParam("Ch8:B:SampleMax",  'D', 0x3F, 12,  0, 4095); // Chan8 B sample maximum

    createConfigParam("MaximumSlope",     'D', 0x40, 12,  0,   20); // Maximum slope

    createConfigParam("TimeVetoLow",      'E',  0x0, 32,  0,    0); // Timestamp veto low
    createConfigParam("TimeVetoHigh",     'E',  0x2, 32,  0, 2147483647); // Timestamp veto high
    createConfigParam("TriggerDelay",     'E',  0x4, 16,  0, 5000); // Type1 calibration trigger delay
    createConfigParam("Sample1",          'E',  0x5, 16,  0,    2); // Type1 calibration sample1
    createConfigParam("Sample2",          'E',  0x6, 16,  0,   12); // Type1 calibration sample2
    createConfigParam("IntRelease",       'E',  0x7,  9,  0,  506); // Integrator release point
    createConfigParam("IbcMask",          'E',  0x8, 16,  0,  511); // IBC mask
    createConfigParam("TsyncDelay",       'E',  0x9, 32,  0,    0); // TSYNC delay

    createConfigParam("Reset",            'F',  0x0,  1,  0,    0); // Reset enable                  (0=disable,1=enable)
    createConfigParam("TclkSelect",       'F',  0x0,  1,  1,    0); // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("TsyncSelect",      'F',  0x0,  1,  2,    0); // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TxEnable",         'F',  0x0,  1,  3,    1); // TX enable                     (0=external,1=always enabled)
    createConfigParam("AcquireMode",      'F',  0x0,  2,  4,    0); // Acquire mode                  (0=normal,1=verbose,2=fakedata,3=trigger)
    createConfigParam("AutoCorrEn",       'F',  0x0,  1,  6,    0); // Auto correction enable        (0=enable,1=disable)
    createConfigParam("IbcSelect",        'F',  0x0,  1,  7,    0); // IBC mode                      (0=external,1=internal))
    createConfigParam("XorderEn",         'F',  0x0,  1,  8,    1); // X order                       (0=disabled,1=enabled)
    createConfigParam("RawDiscOutput",    'F',  0x0,  1,  9,    1); // Raw disc output               (0=disabled,1=enabled)
    createConfigParam("YorderEn",         'F',  0x0,  1, 10,    1); // Y order                       (0=disabled,1=enabled)
    createConfigParam("VariableMode",     'F',  0x0,  1, 11,    0); // Variable sample mode          (0=disabled,1=enabled)
    createConfigParam("OutputMode",       'F',  0x0,  2, 12,    1); // Output mode - TODO values     (0=raw,1=calculated)
    createConfigParam("TpSelect",         'F',  0x0,  2, 14,    2); // TP select - TODO values
}
