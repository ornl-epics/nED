/* ArocPlugin_v25.cpp
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 * @modified by Vladislav Sedov - 08/06/2018
 *
 * This firmware supports new LVDS protocol with timing information from DSP.
 */

#include "ArocPlugin.h"

/**
 * @file ArocPlugin_v25.cpp
 *
 * AROC 2.4 parameters
 */
void ArocPlugin::createParams_v25()
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
    createStatusParam("Configured",         0x1,  2,  9); // Configured                   (0=not configured [alarm],1=full config,2=part config,3=all config, archive:monitor)
    createStatusParam("Discovered",         0x1,  1, 11); // Discovered                   (0=not discovered,1=discovered, archive:monitor)

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
    createStatusParam("Acquiring",          0x3,  1, 13); // Acquiring data               (0=not acquiring,1=acquiring)
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

    createStatusParam("Ch1:X:AdcOffsetVal",    0x6,  8,  0); // X1 ADC Offset value
    createStatusParam("Ch1:X:InputOffset",     0x6,  9,  8); // X1 Input Offset value
    createStatusParam("Ch1:X:AutoAdjOverflow", 0x7,  1,  1); // X1 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch1:X:AutoAdjSlope1",   0x7,  1,  2); // X1 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch1:X:AutoAdjOffset",   0x7,  1,  3); // X1 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch1:X:AutoAdjSlope",    0x7,  1,  4); // X1 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch1:X:AutoAdjSample",   0x7,  1,  5); // X1 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch1:X:AutoAdjTrig",     0x7,  1,  6); // X1 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch1:X:AutoAdjEn",       0x7,  1,  7); // X1 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch2:X:AdcOffsetVal",    0x7,  8,  8); // X2 ADC Offset value
    createStatusParam("Ch2:X:InputOffset",     0x8,  9,  0); // X2 ADC Offset value
    createStatusParam("Ch2:X:AutoAdjOverflow", 0x8,  1,  9); // X2 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch2:X:AutoAdjSlope1",   0x8,  1, 10); // X2 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch2:X:AutoAdjOffset",   0x8,  1, 11); // X2 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch2:X:AutoAdjSlope",    0x8,  1, 12); // X2 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch2:X:AutoAdjSample",   0x8,  1, 13); // X2 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch2:X:AutoAdjTrig",     0x8,  1, 14); // X2 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch2:X:AutoAdjEn",       0x8,  1, 15); // X2 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch3:X:AdcOffsetVal",    0x9,  8,  0); // X3 ADC Offset value
    createStatusParam("Ch3:X:InputOffset",     0x9,  9,  8); // X3 Input Offset value
    createStatusParam("Ch3:X:AutoAdjOverflow", 0xA,  1,  1); // X3 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch3:X:AutoAdjSlope1",   0xA,  1,  2); // X3 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch3:X:AutoAdjOffset",   0xA,  1,  3); // X3 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch3:X:AutoAdjSlope",    0xA,  1,  4); // X3 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch3:X:AutoAdjSample",   0xA,  1,  5); // X3 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch3:X:AutoAdjTrig",     0xA,  1,  6); // X3 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch3:X:AutoAdjEn",       0xA,  1,  7); // X3 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch4:X:AdcOffsetVal",    0xA,  8,  8); // X4 ADC Offset value
    createStatusParam("Ch4:X:InputOffset",     0xB,  9,  0); // X4 ADC Offset value
    createStatusParam("Ch4:X:AutoAdjOverflow", 0xB,  1,  9); // X4 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch4:X:AutoAdjSlope1",   0xB,  1, 10); // X4 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch4:X:AutoAdjOffset",   0xB,  1, 11); // X4 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch4:X:AutoAdjSlope",    0xB,  1, 12); // X4 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch4:X:AutoAdjSample",   0xB,  1, 13); // X4 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch4:X:AutoAdjTrig",     0xB,  1, 14); // X4 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch4:X:AutoAdjEn",       0xB,  1, 15); // X4 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch5:X:AdcOffsetVal",    0xC,  8,  0); // X5 ADC Offset value
    createStatusParam("Ch5:X:InputOffset",     0xC,  9,  8); // X5 Input Offset value
    createStatusParam("Ch5:X:AutoAdjOverflow", 0xD,  1,  1); // X5 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch5:X:AutoAdjSlope1",   0xD,  1,  2); // X5 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch5:X:AutoAdjOffset",   0xD,  1,  3); // X5 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch5:X:AutoAdjSlope",    0xD,  1,  4); // X5 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch5:X:AutoAdjSample",   0xD,  1,  5); // X5 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch5:X:AutoAdjTrig",     0xD,  1,  6); // X5 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch5:X:AutoAdjEn",       0xD,  1,  7); // X5 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch6:X:AdcOffsetVal",    0xD,  8,  8); // X6 ADC Offset value
    createStatusParam("Ch6:X:InputOffset",     0xE,  9,  0); // X6 ADC Offset value
    createStatusParam("Ch6:X:AutoAdjOverflow", 0xE,  1,  9); // X6 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch6:X:AutoAdjSlope1",   0xE,  1, 10); // X6 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch6:X:AutoAdjOffset",   0xE,  1, 11); // X6 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch6:X:AutoAdjSlope",    0xE,  1, 12); // X6 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch6:X:AutoAdjSample",   0xE,  1, 13); // X6 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch6:X:AutoAdjTrig",     0xE,  1, 14); // X6 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch6:X:AutoAdjEn",       0xE,  1, 15); // X6 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch7:X:AdcOffsetVal",    0xF,  8,  0); // X7 ADC Offset value
    createStatusParam("Ch7:X:InputOffset",     0xF,  9,  8); // X7 Input Offset value
    createStatusParam("Ch7:X:AutoAdjOverflow",0x10, 1,  1); // X7 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch7:X:AutoAdjSlope1",  0x10, 1,  2); // X7 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch7:X:AutoAdjOffset",  0x10, 1,  3); // X7 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch7:X:AutoAdjSlope",   0x10, 1,  4); // X7 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch7:X:AutoAdjSample",  0x10, 1,  5); // X7 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch7:X:AutoAdjTrig",    0x10, 1,  6); // X7 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch7:X:AutoAdjEn",      0x10, 1,  7); // X7 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch8:X:AdcOffsetVal",   0x10, 8,  8); // X8 ADC Offset value
    createStatusParam("Ch8:X:InputOffset",    0x11, 9,  0); // X8 ADC Offset value
    createStatusParam("Ch8:X:AutoAdjOverflow",0x11, 1,  9); // X8 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch8:X:AutoAdjSlope1",  0x11, 1, 10); // X8 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch8:X:AutoAdjOffset",  0x11, 1, 11); // X8 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch8:X:AutoAdjSlope",   0x11, 1, 12); // X8 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch8:X:AutoAdjSample",  0x11, 1, 13); // X8 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch8:X:AutoAdjTrig",    0x11, 1, 14); // X8 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch8:X:AutoAdjEn",      0x11, 1, 15); // X8 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch1:Y:AdcOffsetVal",   0x12,  8,  0); // Y1 ADC Offset value
    createStatusParam("Ch1:Y:InputOffset",    0x12,  9,  8); // Y1 Input Offset value
    createStatusParam("Ch1:Y:AutoAdjOverflow",0x13,  1,  1); // Y1 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch1:Y:AutoAdjSlope1",  0x13,  1,  2); // Y1 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch1:Y:AutoAdjOffset",  0x13,  1,  3); // Y1 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch1:Y:AutoAdjSlope",   0x13,  1,  4); // Y1 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch1:Y:AutoAdjSample",  0x13,  1,  5); // Y1 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch1:Y:AutoAdjTrig",    0x13,  1,  6); // Y1 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch1:Y:AutoAdjEn",      0x13,  1,  7); // Y1 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch2:Y:AdcOffsetVal",   0x13,  8,  8); // Y2 ADC Offset value
    createStatusParam("Ch2:Y:InputOffset",    0x14,  9,  0); // Y2 ADC Offset value
    createStatusParam("Ch2:Y:AutoAdjOverflow",0x14,  1,  9); // Y2 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch2:Y:AutoAdjSlope1",  0x14,  1, 10); // Y2 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch2:Y:AutoAdjOffset",  0x14,  1, 11); // Y2 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch2:Y:AutoAdjSlope",   0x14,  1, 12); // Y2 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch2:Y:AutoAdjSample",  0x14,  1, 13); // Y2 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch2:Y:AutoAdjTrig",    0x14,  1, 14); // Y2 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch2:Y:AutoAdjEn",      0x14,  1, 15); // Y2 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch3:Y:AdcOffsetVal",   0x15,  8,  0); // Y3 ADC Offset value
    createStatusParam("Ch3:Y:InputOffset",    0x15,  9,  8); // Y3 Input Offset value
    createStatusParam("Ch3:Y:AutoAdjOverflow",0x16,  1,  1); // Y3 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch3:Y:AutoAdjSlope1",  0x16,  1,  2); // Y3 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch3:Y:AutoAdjOffset",  0x16,  1,  3); // Y3 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch3:Y:AutoAdjSlope",   0x16,  1,  4); // Y3 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch3:Y:AutoAdjSample",  0x16,  1,  5); // Y3 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch3:Y:AutoAdjTrig",    0x16,  1,  6); // Y3 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch3:Y:AutoAdjEn",      0x16,  1,  7); // Y3 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch4:Y:AdcOffsetVal",   0x16,  8,  8); // Y4 ADC Offset value
    createStatusParam("Ch4:Y:InputOffset",    0x17,  9,  0); // Y4 ADC Offset value
    createStatusParam("Ch4:Y:AutoAdjOverflow",0x17,  1,  9); // Y4 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch4:Y:AutoAdjSlope1",  0x17,  1, 10); // Y4 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch4:Y:AutoAdjOffset",  0x17,  1, 11); // Y4 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch4:Y:AutoAdjSlope",   0x17,  1, 12); // Y4 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch4:Y:AutoAdjSample",  0x17,  1, 13); // Y4 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch4:Y:AutoAdjTrig",    0x17,  1, 14); // Y4 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch4:Y:AutoAdjEn",      0x17,  1, 15); // Y4 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch5:Y:AdcOffsetVal",   0x18,  8,  0); // Y5 ADC Offset value
    createStatusParam("Ch5:Y:InputOffset",    0x18,  9,  8); // Y5 Input Offset value
    createStatusParam("Ch5:Y:AutoAdjOverflow",0x19,  1,  1); // Y5 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch5:Y:AutoAdjSlope1",  0x19,  1,  2); // Y5 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch5:Y:AutoAdjOffset",  0x19,  1,  3); // Y5 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch5:Y:AutoAdjSlope",   0x19,  1,  4); // Y5 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch5:Y:AutoAdjSample",  0x19,  1,  5); // Y5 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch5:Y:AutoAdjTrig",    0x19,  1,  6); // Y5 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch5:Y:AutoAdjEn",      0x19,  1,  7); // Y5 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch6:Y:AdcOffsetVal",   0x19,  8,  8); // Y6 ADC Offset value
    createStatusParam("Ch6:Y:InputOffset",    0x1A,  9,  0); // Y6 ADC Offset value
    createStatusParam("Ch6:Y:AutoAdjOverflow",0x1A,  1,  9); // Y6 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch6:Y:AutoAdjSlope1",  0x1A,  1, 10); // Y6 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch6:Y:AutoAdjOffset",  0x1A,  1, 11); // Y6 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch6:Y:AutoAdjSlope",   0x1A,  1, 12); // Y6 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch6:Y:AutoAdjSample",  0x1A,  1, 13); // Y6 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch6:Y:AutoAdjTrig",    0x1A,  1, 14); // Y6 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch6:Y:AutoAdjEn",      0x1A,  1, 15); // Y6 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch7:Y:AdcOffsetVal",   0x1B,  8,  0); // Y7 ADC Offset value
    createStatusParam("Ch7:Y:InputOffset",    0x1B,  9,  8); // Y7 Input Offset value
    createStatusParam("Ch7:Y:AutoAdjOverflow",0x1C,  1,  1); // Y7 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch7:Y:AutoAdjSlope1",  0x1C,  1,  2); // Y7 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch7:Y:AutoAdjOffset",  0x1C,  1,  3); // Y7 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch7:Y:AutoAdjSlope",   0x1C,  1,  4); // Y7 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch7:Y:AutoAdjSample",  0x1C,  1,  5); // Y7 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch7:Y:AutoAdjTrig",    0x1C,  1,  6); // Y7 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch7:Y:AutoAdjEn",      0x1C,  1,  7); // Y7 Auto-Adjust Active        (0=not active,1=active)

    createStatusParam("Ch8:Y:AdcOffsetVal",   0x1C,  8,  8); // Y8 ADC Offset value
    createStatusParam("Ch8:Y:InputOffset",    0x1D,  9,  0); // Y8 ADC Offset value
    createStatusParam("Ch8:Y:AutoAdjOverflow",0x1D,  1,  9); // Y8 Auto-Adjust overflow      (0=no,1=yes)
    createStatusParam("Ch8:Y:AutoAdjSlope1",  0x1D,  1, 10); // Y8 Auto-Adjust Slope? l      (0=not active,1=active)
    createStatusParam("Ch8:Y:AutoAdjOffset",  0x1D,  1, 11); // Y8 Auto-Adjust Offset l      (0=not active,1=active)
    createStatusParam("Ch8:Y:AutoAdjSlope",   0x1D,  1, 12); // Y8 Auto-Adjust Slope l       (0=not active,1=active)
    createStatusParam("Ch8:Y:AutoAdjSample",  0x1D,  1, 13); // Y8 Auto-Adjust Sample l      (0=not active,1=active)
    createStatusParam("Ch8:Y:AutoAdjTrig",    0x1D,  1, 14); // Y8 Auto-Adjust Got samp      (0=no sample,1=got sample)
    createStatusParam("Ch8:Y:AutoAdjEn",      0x1D,  1, 15); // Y8 Auto-Adjust Active        (0=not active,1=active)

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
    createCounterParam("CntTimeFifoFull",    0xB, 16,  0); // Time FIFO full error counter
    createCounterParam("CntEventFifoFull",   0xC, 16,  0); // Event FIFO full error counter
    createCounterParam("CntDataAlmostFull" , 0xD, 16,  0); // Data almost full counter
    createCounterParam("CntMissClk",         0xE, 16,  0); // Link RX clock missing cnt
    createCounterParam("Ch1:X:RateInitDisc",  0xF, 16,  0); // Ch1 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch2:X:RateInitDisc", 0x10, 16,  0); // Ch2 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch3:X:RateInitDisc", 0x11, 16,  0); // Ch3 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch4:X:RateInitDisc", 0x12, 16,  0); // Ch4 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch5:X:RateInitDisc", 0x13, 16,  0); // Ch5 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch6:X:RateInitDisc", 0x14, 16,  0); // Ch6 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch7:X:RateInitDisc", 0x15, 16,  0); // Ch7 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch8:X:RateInitDisc", 0x16, 16,  0); // Ch8 X init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch1:Y:RateInitDisc", 0x17, 16,  0); // Ch1 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch2:Y:RateInitDisc", 0x18, 16,  0); // Ch2 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch3:Y:RateInitDisc", 0x19, 16,  0); // Ch3 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch4:Y:RateInitDisc", 0x1A, 16,  0); // Ch4 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch5:Y:RateInitDisc", 0x1B, 16,  0); // Ch5 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch6:Y:RateInitDisc", 0x1C, 16,  0); // Ch6 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch7:Y:RateInitDisc", 0x1D, 16,  0); // Ch7 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ch8:Y:RateInitDisc", 0x1E, 16,  0); // Ch8 Y init discriminator    (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("Ibc:RateOut",        0x1F, 16,  0); // IBC outrate                 (scale:19.0735,unit:cnts/s,prec:1)
    createCounterParam("RateOut",            0x20, 16,  0); // Total outrate               (scale:19.0735,unit:cnts/s,prec:1)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionIdx",      '1',  0x0, 32,  0,    0); // Chan1 position index

    createConfigParam("Ch1:X:InOffset",   '2',  0x0,  9,  0,  100, CONV_SIGN_MAGN); // Chan1 X input offset
    createConfigParam("Ch2:X:InOffset",   '2',  0x1,  9,  0,  100, CONV_SIGN_MAGN); // Chan2 X input offset
    createConfigParam("Ch3:X:InOffset",   '2',  0x2,  9,  0,  100, CONV_SIGN_MAGN); // Chan3 X input offset
    createConfigParam("Ch4:X:InOffset",   '2',  0x3,  9,  0,  100, CONV_SIGN_MAGN); // Chan4 X input offset
    createConfigParam("Ch5:X:InOffset",   '2',  0x4,  9,  0,  100, CONV_SIGN_MAGN); // Chan5 X input offset
    createConfigParam("Ch6:X:InOffset",   '2',  0x5,  9,  0,  100, CONV_SIGN_MAGN); // Chan6 X input offset
    createConfigParam("Ch7:X:InOffset",   '2',  0x6,  9,  0,  100, CONV_SIGN_MAGN); // Chan7 X input offset
    createConfigParam("Ch8:X:InOffset",   '2',  0x7,  9,  0,  100, CONV_SIGN_MAGN); // Chan8 X input offset
    createConfigParam("Ch1:Y:InOffset",   '2',  0x8,  9,  0,  100, CONV_SIGN_MAGN); // Chan1 Y input offset
    createConfigParam("Ch2:Y:InOffset",   '2',  0x9,  9,  0,  100, CONV_SIGN_MAGN); // Chan2 Y input offset
    createConfigParam("Ch3:Y:InOffset",   '2',  0xA,  9,  0,  100, CONV_SIGN_MAGN); // Chan3 Y input offset
    createConfigParam("Ch4:Y:InOffset",   '2',  0xB,  9,  0,  100, CONV_SIGN_MAGN); // Chan4 Y input offset
    createConfigParam("Ch5:Y:InOffset",   '2',  0xC,  9,  0,  100, CONV_SIGN_MAGN); // Chan5 Y input offset
    createConfigParam("Ch6:Y:InOffset",   '2',  0xD,  9,  0,  100, CONV_SIGN_MAGN); // Chan6 Y input offset
    createConfigParam("Ch7:Y:InOffset",   '2',  0xE,  9,  0,  100, CONV_SIGN_MAGN); // Chan7 Y input offset
    createConfigParam("Ch8:Y:InOffset",   '2',  0xF,  9,  0,  100, CONV_SIGN_MAGN); // Chan8 Y input offset

    createConfigParam("Ch1:X:AdcOffset",  '2', 0x10,  8,  0,  100); // Chan1 X ADC offset
    createConfigParam("Ch2:X:AdcOffset",  '2', 0x11,  8,  0,  100); // Chan2 X ADC offset
    createConfigParam("Ch3:X:AdcOffset",  '2', 0x12,  8,  0,  100); // Chan3 X ADC offset
    createConfigParam("Ch4:X:AdcOffset",  '2', 0x13,  8,  0,  100); // Chan4 X ADC offset
    createConfigParam("Ch5:X:AdcOffset",  '2', 0x14,  8,  0,  100); // Chan5 X ADC offset
    createConfigParam("Ch6:X:AdcOffset",  '2', 0x15,  8,  0,  100); // Chan6 X ADC offset
    createConfigParam("Ch7:X:AdcOffset",  '2', 0x16,  8,  0,  100); // Chan7 X ADC offset
    createConfigParam("Ch8:X:AdcOffset",  '2', 0x17,  8,  0,  100); // Chan8 X ADC offset
    createConfigParam("Ch1:Y:AdcOffset",  '2', 0x18,  8,  0,  100); // Chan1 Y ADC offset
    createConfigParam("Ch2:Y:AdcOffset",  '2', 0x19,  8,  0,  100); // Chan2 Y ADC offset
    createConfigParam("Ch3:Y:AdcOffset",  '2', 0x1A,  8,  0,  100); // Chan3 Y ADC offset
    createConfigParam("Ch4:Y:AdcOffset",  '2', 0x1B,  8,  0,  100); // Chan4 Y ADC offset
    createConfigParam("Ch5:Y:AdcOffset",  '2', 0x1C,  8,  0,  100); // Chan5 Y ADC offset
    createConfigParam("Ch6:Y:AdcOffset",  '2', 0x1D,  8,  0,  100); // Chan6 Y ADC offset
    createConfigParam("Ch7:Y:AdcOffset",  '2', 0x1E,  8,  0,  100); // Chan7 Y ADC offset
    createConfigParam("Ch8:Y:AdcOffset",  '2', 0x1F,  8,  0,  100); // Chan8 Y ADC offset

    createConfigParam("Ch1:X:Threshold",  '2', 0x20, 12,  0, 1725); // Chan1 X threshold
    createConfigParam("Ch2:X:Threshold",  '2', 0x21, 12,  0, 1725); // Chan2 X threshold
    createConfigParam("Ch3:X:Threshold",  '2', 0x22, 12,  0, 1725); // Chan3 X threshold
    createConfigParam("Ch4:X:Threshold",  '2', 0x23, 12,  0, 1725); // Chan4 X threshold
    createConfigParam("Ch5:X:Threshold",  '2', 0x24, 12,  0, 1725); // Chan5 X threshold
    createConfigParam("Ch6:X:Threshold",  '2', 0x25, 12,  0, 1725); // Chan6 X threshold
    createConfigParam("Ch7:X:Threshold",  '2', 0x26, 12,  0, 1725); // Chan7 X threshold
    createConfigParam("Ch8:X:Threshold",  '2', 0x27, 12,  0, 1725); // Chan8 X threshold
    createConfigParam("Ch1:Y:Threshold",  '2', 0x28, 12,  0, 1725); // Chan1 Y threshold
    createConfigParam("Ch2:Y:Threshold",  '2', 0x29, 12,  0, 1725); // Chan2 Y threshold
    createConfigParam("Ch3:Y:Threshold",  '2', 0x2A, 12,  0, 1725); // Chan3 Y threshold
    createConfigParam("Ch4:Y:Threshold",  '2', 0x2B, 12,  0, 1725); // Chan4 Y threshold
    createConfigParam("Ch5:Y:Threshold",  '2', 0x2C, 12,  0, 1725); // Chan5 Y threshold
    createConfigParam("Ch6:Y:Threshold",  '2', 0x2D, 12,  0, 1725); // Chan6 Y threshold
    createConfigParam("Ch7:Y:Threshold",  '2', 0x2E, 12,  0, 1725); // Chan7 Y threshold
    createConfigParam("Ch8:Y:Threshold",  '2', 0x2F, 12,  0, 1725); // Chan8 Y threshold

    createConfigParam("Ch1:X:FullScale",  '2', 0x30,  8,  0,    0); // Chan1 X full scale
    createConfigParam("Ch2:X:FullScale",  '2', 0x31,  8,  0,    0); // Chan2 X full scale
    createConfigParam("Ch3:X:FullScale",  '2', 0x32,  8,  0,    0); // Chan3 X full scale
    createConfigParam("Ch4:X:FullScale",  '2', 0x33,  8,  0,    0); // Chan4 X full scale
    createConfigParam("Ch5:X:FullScale",  '2', 0x34,  8,  0,    0); // Chan5 X full scale
    createConfigParam("Ch6:X:FullScale",  '2', 0x35,  8,  0,    0); // Chan6 X full scale
    createConfigParam("Ch7:X:FullScale",  '2', 0x36,  8,  0,    0); // Chan7 X full scale
    createConfigParam("Ch8:X:FullScale",  '2', 0x37,  8,  0,    0); // Chan8 X full scale
    createConfigParam("Ch1:Y:FullScale",  '2', 0x38,  8,  0,    0); // Chan1 Y full scale
    createConfigParam("Ch2:Y:FullScale",  '2', 0x39,  8,  0,    0); // Chan2 Y full scale
    createConfigParam("Ch3:Y:FullScale",  '2', 0x3A,  8,  0,    0); // Chan3 Y full scale
    createConfigParam("Ch4:Y:FullScale",  '2', 0x3B,  8,  0,    0); // Chan4 Y full scale
    createConfigParam("Ch5:Y:FullScale",  '2', 0x3C,  8,  0,    0); // Chan5 Y full scale
    createConfigParam("Ch6:Y:FullScale",  '2', 0x3D,  8,  0,    0); // Chan6 Y full scale
    createConfigParam("Ch7:Y:FullScale",  '2', 0x3E,  8,  0,    0); // Chan7 Y full scale
    createConfigParam("Ch8:Y:FullScale",  '2', 0x3F,  8,  0,    0); // Chan8 Y full scale

    createConfigParam("Ch1:X:GainAdj",    '2', 0x40,  8,  0,    0); // Chan1 X gain adjust
    createConfigParam("Ch2:X:GainAdj",    '2', 0x41,  8,  0,    0); // Chan2 X gain adjust
    createConfigParam("Ch3:X:GainAdj",    '2', 0x42,  8,  0,    0); // Chan3 X gain adjust
    createConfigParam("Ch4:X:GainAdj",    '2', 0x43,  8,  0,    0); // Chan4 X gain adjust
    createConfigParam("Ch5:X:GainAdj",    '2', 0x44,  8,  0,    0); // Chan5 X gain adjust
    createConfigParam("Ch6:X:GainAdj",    '2', 0x45,  8,  0,    0); // Chan6 X gain adjust
    createConfigParam("Ch7:X:GainAdj",    '2', 0x46,  8,  0,    0); // Chan7 X gain adjust
    createConfigParam("Ch8:X:GainAdj",    '2', 0x47,  8,  0,    0); // Chan8 X gain adjust
    createConfigParam("Ch1:Y:GainAdj",    '2', 0x48,  8,  0,    0); // Chan1 Y gain adjust
    createConfigParam("Ch2:Y:GainAdj",    '2', 0x49,  8,  0,    0); // Chan2 Y gain adjust
    createConfigParam("Ch3:Y:GainAdj",    '2', 0x4A,  8,  0,    0); // Chan3 Y gain adjust
    createConfigParam("Ch4:Y:GainAdj",    '2', 0x4B,  8,  0,    0); // Chan4 Y gain adjust
    createConfigParam("Ch5:Y:GainAdj",    '2', 0x4C,  8,  0,    0); // Chan5 Y gain adjust
    createConfigParam("Ch6:Y:GainAdj",    '2', 0x4D,  8,  0,    0); // Chan6 Y gain adjust
    createConfigParam("Ch7:Y:GainAdj",    '2', 0x4E,  8,  0,    0); // Chan7 Y gain adjust
    createConfigParam("Ch8:Y:GainAdj",    '2', 0x4F,  8,  0,    0); // Chan8 Y gain adjust

    createConfigParam("Ch1:X:Scale",      'C',  0x0, 16,  0,    0); // Chan1 X scale
    createConfigParam("Ch2:X:Scale",      'C',  0x1, 16,  0,    0); // Chan2 X scale
    createConfigParam("Ch3:X:Scale",      'C',  0x2, 16,  0,    0); // Chan3 X scale
    createConfigParam("Ch4:X:Scale",      'C',  0x3, 16,  0,    0); // Chan4 X scale
    createConfigParam("Ch5:X:Scale",      'C',  0x4, 16,  0,    0); // Chan5 X scale
    createConfigParam("Ch6:X:Scale",      'C',  0x5, 16,  0,    0); // Chan6 X scale
    createConfigParam("Ch7:X:Scale",      'C',  0x6, 16,  0,    0); // Chan7 X scale
    createConfigParam("Ch8:X:Scale",      'C',  0x7, 16,  0,    0); // Chan8 X scale
    createConfigParam("Ch1:Y:Scale",      'C',  0x8, 16,  0,    0); // Chan1 Y scale
    createConfigParam("Ch2:Y:Scale",      'C',  0x9, 16,  0,    0); // Chan2 Y scale
    createConfigParam("Ch3:Y:Scale",      'C',  0xA, 16,  0,    0); // Chan3 Y scale
    createConfigParam("Ch4:Y:Scale",      'C',  0xB, 16,  0,    0); // Chan4 Y scale
    createConfigParam("Ch5:Y:Scale",      'C',  0xC, 16,  0,    0); // Chan5 Y scale
    createConfigParam("Ch6:Y:Scale",      'C',  0xD, 16,  0,    0); // Chan6 Y scale
    createConfigParam("Ch7:Y:Scale",      'C',  0xE, 16,  0,    0); // Chan7 Y scale
    createConfigParam("Ch8:Y:Scale",      'C',  0xF, 16,  0,    0); // Chan8 Y scale

    createConfigParam("Ch1:X:Offset",     'C', 0x10, 16,  0,    0); // Chan1 X Offset
    createConfigParam("Ch2:X:Offset",     'C', 0x11, 16,  0,    0); // Chan2 X Offset
    createConfigParam("Ch3:X:Offset",     'C', 0x12, 16,  0,    0); // Chan3 X Offset
    createConfigParam("Ch4:X:Offset",     'C', 0x13, 16,  0,    0); // Chan4 X Offset
    createConfigParam("Ch5:X:Offset",     'C', 0x14, 16,  0,    0); // Chan5 X Offset
    createConfigParam("Ch6:X:Offset",     'C', 0x15, 16,  0,    0); // Chan6 X Offset
    createConfigParam("Ch7:X:Offset",     'C', 0x16, 16,  0,    0); // Chan7 X Offset
    createConfigParam("Ch8:X:Offset",     'C', 0x17, 16,  0,    0); // Chan8 X Offset
    createConfigParam("Ch1:Y:Offset",     'C', 0x18, 16,  0,    0); // Chan1 Y Offset
    createConfigParam("Ch2:Y:Offset",     'C', 0x19, 16,  0,    0); // Chan2 Y Offset
    createConfigParam("Ch3:Y:Offset",     'C', 0x1A, 16,  0,    0); // Chan3 Y Offset
    createConfigParam("Ch4:Y:Offset",     'C', 0x1B, 16,  0,    0); // Chan4 Y Offset
    createConfigParam("Ch5:Y:Offset",     'C', 0x1C, 16,  0,    0); // Chan5 Y Offset
    createConfigParam("Ch6:Y:Offset",     'C', 0x1D, 16,  0,    0); // Chan6 Y Offset
    createConfigParam("Ch7:Y:Offset",     'C', 0x1E, 16,  0,    0); // Chan7 Y Offset
    createConfigParam("Ch8:Y:Offset",     'C', 0x1F, 16,  0,    0); // Chan8 Y Offset

    createConfigParam("Ch1:X:AvgMin",     'D',  0x0, 16,  0,  100); // Chan1 X average minimum
    createConfigParam("Ch2:X:AvgMin",     'D',  0x1, 16,  0,  100); // Chan2 X average minimum
    createConfigParam("Ch3:X:AvgMin",     'D',  0x2, 16,  0,  100); // Chan3 X average minimum
    createConfigParam("Ch4:X:AvgMin",     'D',  0x3, 16,  0,  100); // Chan4 X average minimum
    createConfigParam("Ch5:X:AvgMin",     'D',  0x4, 16,  0,  100); // Chan5 X average minimum
    createConfigParam("Ch6:X:AvgMin",     'D',  0x5, 16,  0,  100); // Chan6 X average minimum
    createConfigParam("Ch7:X:AvgMin",     'D',  0x6, 16,  0,  100); // Chan7 X average minimum
    createConfigParam("Ch8:X:AvgMin",     'D',  0x7, 16,  0,  100); // Chan8 X average minimum
    createConfigParam("Ch1:Y:AvgMin",     'D',  0x8, 16,  0,  100); // Chan1 Y average minimum
    createConfigParam("Ch2:Y:AvgMin",     'D',  0x9, 16,  0,  100); // Chan2 Y average minimum
    createConfigParam("Ch3:Y:AvgMin",     'D',  0xA, 16,  0,  100); // Chan3 Y average minimum
    createConfigParam("Ch4:Y:AvgMin",     'D',  0xB, 16,  0,  100); // Chan4 Y average minimum
    createConfigParam("Ch5:Y:AvgMin",     'D',  0xC, 16,  0,  100); // Chan5 Y average minimum
    createConfigParam("Ch6:Y:AvgMin",     'D',  0xD, 16,  0,  100); // Chan6 Y average minimum
    createConfigParam("Ch7:Y:AvgMin",     'D',  0xE, 16,  0,  100); // Chan7 Y average minimum
    createConfigParam("Ch8:Y:AvgMin",     'D',  0xF, 16,  0,  100); // Chan8 Y average minimum

    createConfigParam("Ch1:X:AvgMax",     'D', 0x10, 16,  0, 4095); // Chan1 X average maximum
    createConfigParam("Ch2:X:AvgMax",     'D', 0x11, 16,  0, 4095); // Chan2 X average maximum
    createConfigParam("Ch3:X:AvgMax",     'D', 0x12, 16,  0, 4095); // Chan3 X average maximum
    createConfigParam("Ch4:X:AvgMax",     'D', 0x13, 16,  0, 4095); // Chan4 X average maximum
    createConfigParam("Ch5:X:AvgMax",     'D', 0x14, 16,  0, 4095); // Chan5 X average maximum
    createConfigParam("Ch6:X:AvgMax",     'D', 0x15, 16,  0, 4095); // Chan6 X average maximum
    createConfigParam("Ch7:X:AvgMax",     'D', 0x16, 16,  0, 4095); // Chan7 X average maximum
    createConfigParam("Ch8:X:AvgMax",     'D', 0x17, 16,  0, 4095); // Chan8 X average maximum
    createConfigParam("Ch1:Y:AvgMax",     'D', 0x18, 16,  0, 4095); // Chan1 Y average maximum
    createConfigParam("Ch2:Y:AvgMax",     'D', 0x19, 16,  0, 4095); // Chan2 Y average maximum
    createConfigParam("Ch3:Y:AvgMax",     'D', 0x1A, 16,  0, 4095); // Chan3 Y average maximum
    createConfigParam("Ch4:Y:AvgMax",     'D', 0x1B, 16,  0, 4095); // Chan4 Y average maximum
    createConfigParam("Ch5:Y:AvgMax",     'D', 0x1C, 16,  0, 4095); // Chan5 Y average maximum
    createConfigParam("Ch6:Y:AvgMax",     'D', 0x1D, 16,  0, 4095); // Chan6 Y average maximum
    createConfigParam("Ch7:Y:AvgMax",     'D', 0x1E, 16,  0, 4095); // Chan7 Y average maximum
    createConfigParam("Ch8:Y:AvgMax",     'D', 0x1F, 16,  0, 4095); // Chan8 Y average maximum

    createConfigParam("Ch1:X:SampleMin",  'D', 0x20, 10,  0,  500); // Chan1 X sample minimum
    createConfigParam("Ch2:X:SampleMin",  'D', 0x21, 10,  0,  500); // Chan2 X sample minimum
    createConfigParam("Ch3:X:SampleMin",  'D', 0x22, 10,  0,  500); // Chan3 X sample minimum
    createConfigParam("Ch4:X:SampleMin",  'D', 0x23, 10,  0,  500); // Chan4 X sample minimum
    createConfigParam("Ch5:X:SampleMin",  'D', 0x24, 10,  0,  500); // Chan5 X sample minimum
    createConfigParam("Ch6:X:SampleMin",  'D', 0x25, 10,  0,  500); // Chan6 X sample minimum
    createConfigParam("Ch7:X:SampleMin",  'D', 0x26, 10,  0,  500); // Chan7 X sample minimum
    createConfigParam("Ch8:X:SampleMin",  'D', 0x27, 10,  0,  500); // Chan8 X sample minimum
    createConfigParam("Ch1:Y:SampleMin",  'D', 0x28, 10,  0,  500); // Chan1 Y sample minimum
    createConfigParam("Ch2:Y:SampleMin",  'D', 0x29, 10,  0,  500); // Chan2 Y sample minimum
    createConfigParam("Ch3:Y:SampleMin",  'D', 0x2A, 10,  0,  500); // Chan3 Y sample minimum
    createConfigParam("Ch4:Y:SampleMin",  'D', 0x2B, 10,  0,  500); // Chan4 Y sample minimum
    createConfigParam("Ch5:Y:SampleMin",  'D', 0x2C, 10,  0,  500); // Chan5 Y sample minimum
    createConfigParam("Ch6:Y:SampleMin",  'D', 0x2D, 10,  0,  500); // Chan6 Y sample minimum
    createConfigParam("Ch7:Y:SampleMin",  'D', 0x2E, 10,  0,  500); // Chan7 Y sample minimum
    createConfigParam("Ch8:Y:SampleMin",  'D', 0x2F, 10,  0,  500); // Chan8 Y sample minimum

    createConfigParam("Ch1:X:SampleMax",  'D', 0x30, 10,  0, 4095); // Chan1 X sample maximum
    createConfigParam("Ch2:X:SampleMax",  'D', 0x31, 10,  0, 4095); // Chan2 X sample maximum
    createConfigParam("Ch3:X:SampleMax",  'D', 0x32, 10,  0, 4095); // Chan3 X sample maximum
    createConfigParam("Ch4:X:SampleMax",  'D', 0x33, 10,  0, 4095); // Chan4 X sample maximum
    createConfigParam("Ch5:X:SampleMax",  'D', 0x34, 10,  0, 4095); // Chan5 X sample maximum
    createConfigParam("Ch6:X:SampleMax",  'D', 0x35, 10,  0, 4095); // Chan6 X sample maximum
    createConfigParam("Ch7:X:SampleMax",  'D', 0x36, 10,  0, 4095); // Chan7 X sample maximum
    createConfigParam("Ch8:X:SampleMax",  'D', 0x37, 10,  0, 4095); // Chan8 X sample maximum
    createConfigParam("Ch1:Y:SampleMax",  'D', 0x38, 10,  0, 4095); // Chan1 Y sample maximum
    createConfigParam("Ch2:Y:SampleMax",  'D', 0x39, 10,  0, 4095); // Chan2 Y sample maximum
    createConfigParam("Ch3:Y:SampleMax",  'D', 0x3A, 10,  0, 4095); // Chan3 Y sample maximum
    createConfigParam("Ch4:Y:SampleMax",  'D', 0x3B, 10,  0, 4095); // Chan4 Y sample maximum
    createConfigParam("Ch5:Y:SampleMax",  'D', 0x3C, 10,  0, 4095); // Chan5 Y sample maximum
    createConfigParam("Ch6:Y:SampleMax",  'D', 0x3D, 10,  0, 4095); // Chan6 Y sample maximum
    createConfigParam("Ch7:Y:SampleMax",  'D', 0x3E, 10,  0, 4095); // Chan7 Y sample maximum
    createConfigParam("Ch8:Y:SampleMax",  'D', 0x3F, 10,  0, 4095); // Chan8 Y sample maximum

    createConfigParam("MaximumSlope",     'D', 0x40, 16,  0,   20); // Maximum slope

    createConfigParam("TimeVetoLow",      'E',  0x0, 32,  0,    0); // Timestamp veto low
    createConfigParam("TimeVetoHigh",     'E',  0x2, 32,  0, 2147483647); // Timestamp veto high
    createConfigParam("FakeTrigDelay",    'E',  0x4, 16,  0, 5000); // Type1 calibration trigger delay
    createConfigParam("Sample1",          'E',  0x5, 10,  0,    2); // Type1 calibration sample1
    createConfigParam("Sample2",          'E',  0x6, 10,  0,   12); // Type1 calibration sample2
    createConfigParam("IntRelease",       'E',  0x7, 10,  0,  -6, CONV_SIGN_2COMP); // Integrator release point
    createConfigParam("IbcMask",          'E',  0x8,  9,  0,  511); // IBC mask
    createConfigParam("TsyncDelay",       'E',  0x9, 32,  0,    0); // TSYNC delay

    createConfigParam("TcResetMode",      'F',  0x0,  1,  0,    0); // Reset enable                  (0=disable,1=enable)
    createConfigParam("TcTclkMode",       'F',  0x0,  1,  1,    0); // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("TcTsyncMode",      'F',  0x0,  1,  2,    0); // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TcTxEnMode",       'F',  0x0,  1,  3,    1); // TX enable                     (0=external,1=always enabled)
    createConfigParam("AcquireMode",      'F',  0x0,  2,  4,    0); // Acquire mode                  (0=idle,1=fakedata,2=normal,3=trigger)
    createConfigParam("AutoCorrectionEn", 'F',  0x0,  1,  6,    0); // Auto correction enable        (0=enable,1=disable)
    createConfigParam("IbcSelect",        'F',  0x0,  1,  7,    0); // IBC mode                      (0=external,1=internal))
    createConfigParam("XorderEn",         'F',  0x0,  1,  8,    1); // X order                       (0=disabled,1=enabled)
    createConfigParam("RawDiscOutput",    'F',  0x0,  1,  9,    1); // Raw disc output               (0=disabled,1=enabled)
    createConfigParam("YorderEn",         'F',  0x0,  1, 10,    1); // Y order                       (0=disabled,1=enabled)
    createConfigParam("VariableMode",     'F',  0x0,  1, 11,    0); // Variable sample mode          (0=disabled,1=enabled)
    createConfigParam("OutputMode",       'F',  0x0,  2, 12,    1); // Output mode                   (0=raw [alarm],1=normal,2=extended [alarm])
    createConfigParam("TpSelect",         'F',  0x0,  2, 14,    2); // Test point select

    createConfigParam("TestPatternId",    'F',  0x1, 12,  0,    0); // Test pattern id
    createConfigParam("TestPatternDebug", 'F',  0x1,  2, 12,    0); // Engineering Use only
    createConfigParam("TestPatternAltEn", 'F',  0x1,  1, 14,    0); // Alternate test pattern enable (0=disable,1=enable)
    createConfigParam("TestPatternEn",    'F',  0x1,  1, 15,    0); // Test pattern enable           (0=disable,1=enable)
    createConfigParam("TestPatternRate",  'F',  0x2, 16,  0,    0); // Test pattern rate
    createConfigParam("CycleAdvance",     'F',  0x3, 10,  0,    1); // Num cycles to advance @TSYNC  (1=60Hz,2=30Hz,3=20Hz,4=15Hz,6=10Hz,12=5Hz,60=1Hz)
    createConfigParam("DataFormatId",     'F',  0x4,  8,  0,    8); // Data format identifier        (5=normal,8=AROC raw,0=unsupported)
    createConfigParam("Protocol",         'F',  0x4,  1, 15,    0); // LVDS protocol select          (0=legacy,1=new)

//  BLXXX:Det:RocXXX:| parameter name |                 | EPICS record description  | (bi and mbbi description)
    createTempParam("TempBoard",        0x0, 16, 0, CONV_SIGN_2COMP); // ROC board temperature in degC   (calc:0.25*A,unit:C,prec:1,low:-50,high:38,hihi:45,archive:monitor)
}
