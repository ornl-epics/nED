/* RocPlugin_v52.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "RocPlugin.h"

/**
 * @file RocPlugin_v52.cpp
 *
 * ROC 5.2 parameters
 *
 * ROC 5.2 is golden firmware (September 2014). It's being well tested, firmware
 * sources are available and it's installed to most beam-lines.
 */

void RocPlugin::createStatusParams_v52()
{
//    BLXXX:Det:RocXXX:| sig nam|                     | EPICS record description | (bi and mbbi description)
    createStatusParam("ErrUartByt",  0x0,  1, 13); // UART: Byte error             (0=no error,1=error)
    createStatusParam("ErrUartPar",  0x0,  1, 12); // UART: Parity error           (0=no error,1=error)
    createStatusParam("ErrProg",     0x0,  1, 11); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("ErrCmdLen",   0x0,  1, 10); // Command length error         (0=no error,1=error)
    createStatusParam("ErrBadCmd",   0x0,  1,  9); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrNoTsync",  0x0,  1,  8); // Timestamp overflow error.    (0=no error,1=error)
    createStatusParam("ErrFifFul",   0x0,  1,  6); // LVDS FIFO went full.         (0=not full,1=full)
    createStatusParam("ErrStart",    0x0,  1,  5); // LVDS start before stop bit   (0=no error,1=error)
    createStatusParam("ErrNoStart",  0x0,  1,  4); // LVDS data without start.     (0=no error,1=error)
    createStatusParam("ErrTimeout",  0x0,  1,  3); // LVDS packet timeout.         (0=no timeout,1=timeout)
    createStatusParam("ErrLength",   0x0,  1,  2); // LVDS packet length error.    (0=no error,1=error)
    createStatusParam("ErrDataTyp",  0x0,  1,  1); // LVDS data type error.        (0=no error,1=error)
    createStatusParam("ErrParity",   0x0,  1,  0); // LVDS parity error.           (0=no error,1=error)

    createStatusParam("LvdsVerDet",  0x1,  1, 14); // LVDS VERIFY detected         (0=not detected,1=detected)
    createStatusParam("IntFifoFul",  0x1,  1, 13); // Int Data FIFO Almost full    (0=not full,1=full)
    createStatusParam("IntFifoEmp",  0x1,  1, 12); // Int Data FIFO Empty flag     (0=not empty,1=empty)
    createStatusParam("CalcBadFin",  0x1,  1, 11); // Calc: Bad Final Calculation  (0=no error,1=error)
    createStatusParam("CalcBadEff",  0x1,  1, 10); // Calc: Bad Effective Calc     (0=no error,1=error)
    createStatusParam("CalcBadOvr",  0x1,  1,  9); // Calc: Data overflow detected (0=no error,1=error)
    createStatusParam("CalcBadCnt",  0x1,  1,  8); // Calc: Bad word count.        (0=no error,1=error)
    createStatusParam("DataFifoFu",  0x1,  1,  7); // Data FIFO Almost full flag.  (0=not full,1=almost full)
    createStatusParam("DataFifoEm",  0x1,  1,  6); // Data FIFO Empty flag.        (0=not empty,1=empty)
    createStatusParam("HVStatus",    0x1,  1,  5); // High Voltage Status bit      (0=not present,1=present)
    createStatusParam("CalcActive",  0x1,  1,  4); // Calculation: Active          (0=not active,1=active)
    createStatusParam("Acquiring",   0x1,  1,  3); // Acquiring data               (0=not acquiring,1=acquiring)
    createStatusParam("Discovered",  0x1,  1,  2); // Discovered                   (0=not discovered,1=discovered)
    createStatusParam("Configured",  0x1,  2,  0); // Configured                   (0=not configured,1=section 1 conf'd,2=section 2 conf'd,3=all conf'd)

    createStatusParam("RisEdgeA",    0x2,  8,  8); // Discriminator A set
    createStatusParam("SysrstBHi",   0x2,  1,  7); // SYSRST_B Got HIGH            (0=no,1=yes)
    createStatusParam("SysrstBLo",   0x2,  1,  6); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("TxenBHigh",   0x2,  1,  5); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("TxenBLow",    0x2,  1,  4); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("TsyncHigh",   0x2,  1,  3); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("TsyncLow",    0x2,  1,  2); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TclkHigh",    0x2,  1,  1); // TCLK Got HIGH                (0=no,1=yes)
    createStatusParam("TclkLow",     0x2,  1,  0); // TCLK Got LOW                 (0=no,1=yes)

    createStatusParam("RisEdgeSum",  0x3,  8,  8); // Discriminator SUM set
    createStatusParam("RisEdgeB",    0x3,  8,  0); // Discriminator B set

    createStatusParam("Ch1:A:En",    0x4,  1, 15); // Ch1 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch1:A:Samp",  0x4,  1, 14); // Ch1 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch1:A:SL",    0x4,  1, 13); // Ch1 A Auto-Adjust Sample li  (0=not active,1=active)
    createStatusParam("Ch1:A:SlL",   0x4,  1, 12); // Ch1 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch1:A:OffL",  0x4,  1, 11); // Ch1 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch1:A:SlLi",  0x4,  1, 10); // Ch1 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch1:A:Ovr",   0x4,  1,  9); // Ch1 A Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch1:A:Slop",  0x4,  9,  0); // Ch1 A Input Offset value

    createStatusParam("Ch1:B:Off",   0x5,  8,  8); // Ch1 B ADC Offset value
    createStatusParam("Ch1:A:Off",   0x5,  8,  0); // Ch1 A ADC Offset value

    createStatusParam("Ch1:B:En",    0x6,  1, 15); // Ch1 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch1:B:Samp",  0x6,  1, 14); // Ch1 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch1:B:SL",    0x6,  1, 13); // Ch1 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch1:B:SlL",   0x6,  1, 12); // Ch1 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch1:B:OffL",  0x6,  1, 11); // Ch1 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch1:B:SlLi",  0x6,  1, 10); // Ch1 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch1:B:Ovr",   0x6,  1,  9); // Ch1 B Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch1:B:Slop",  0x6,  9,  0); // Ch1 B Input Offset value

    // NOTE: The next parameter spans over the LVDS words (16bits) *and* it also spans over the
    //       OCC dword (32 bits). BaseModulePlugin::createStatusParam() and BaseModulePlugin:rspReadStatus()
    //       functions are smart enough to accomodate the behaviour.
    createStatusParam("Ch2:A:Slop",  0x7,  9,  8); // Ch2 A input offset value
    createStatusParam("Ch1:AdcMax",  0x7,  1,  7); // Ch1 got ADC max              (0=no,1=yes)
    createStatusParam("Ch1:AdcMin",  0x7,  1,  6); // Ch1 got ADC min              (0=no,1=yes)
    createStatusParam("Ch1:MDEv",    0x7,  1,  5); // Ch1 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch1:Ev",      0x7,  1,  4); // Ch1 got event                (0=no,1=yes)
    createStatusParam("Ch1:FifFul",  0x7,  1,  3); // Ch1 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch1:FifAFF",  0x7,  1,  2); // Ch1 FIFO almost full deteced (0=no,1=yes)
    createStatusParam("Ch1:FifAF",   0x7,  1,  1); // Ch1 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch1:Data",    0x7,  1,  0); // Ch1 FIFO has data            (0=no,1=yes)

    createStatusParam("Ch2:A:Off",   0x8,  8,  8); // Ch2 A ADC Offset value
    createStatusParam("Ch2:A:En",    0x8,  1,  7); // Ch2 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch2:A:Samp",  0x8,  1,  6); // Ch2 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch2:A:SL",    0x8,  1,  5); // Ch2 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch2:A:SlL",   0x8,  1,  4); // Ch2 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch2:A:OffL",  0x8,  1,  3); // Ch2 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch2:A:SlLi",  0x8,  1,  2); // Ch2 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch2:A:Ovr",   0x8,  1,  1); // Ch2 A Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch2:B:Slop",  0x9,  9,  8); // Ch2 B input offset value
    createStatusParam("Ch2:B:Off",   0x9,  8,  0); // Ch2 B ADC Offset value

    createStatusParam("Ch2:AdcMax",  0xA,  1, 15); // Ch2 got ADC max              (0=no,1=yes)
    createStatusParam("Ch2:AdcMin",  0xA,  1, 14); // Ch2 got ADC min              (0=no,1=yes)
    createStatusParam("Ch2:MDEv",    0xA,  1, 13); // Ch2 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch2:Ev",      0xA,  1, 12); // Ch2 got event                (0=no,1=yes)
    createStatusParam("Ch2:FifFul",  0xA,  1, 11); // Ch2 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch2:FifAFF",  0xA,  1, 10); // Ch2 FIFO almost full deteced (0=no,1=yes)
    createStatusParam("Ch2:FifAF",   0xA,  1,  9); // Ch2 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch2:Data",    0xA,  1,  8); // Ch2 FIFO has data            (0=no,1=yes)
    createStatusParam("Ch2:B:En",    0xA,  1,  7); // Ch2 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch2:B:Samp",  0xA,  1,  6); // Ch2 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch2:B:SL",    0xA,  1,  5); // Ch2 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch2:B:SlL",   0xA,  1,  4); // Ch2 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch2:B:OffL",  0xA,  1,  3); // Ch2 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch2:B:SlLi",  0xA,  1,  2); // Ch2 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch2:B:Ovr",   0xA,  1,  1); // Ch2 B Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch3:A:En",    0xB,  1, 15); // Ch3 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch3:A:Samp",  0xB,  1, 14); // Ch3 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch3:A:SL",    0xB,  1, 13); // Ch3 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch3:A:SlL",   0xB,  1, 12); // Ch3 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch3:A:OffL",  0xB,  1, 11); // Ch3 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch3:A:SlLi",  0xB,  1, 10); // Ch3 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch3:A:Ovr",   0xB,  1,  9); // Ch3 A Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch3:A:Slop",  0xB,  9,  0); // Ch3 A input offset value

    createStatusParam("Ch3:A:Off",   0xC,  8,  0); // Ch3 A ADC Offset value
    createStatusParam("Ch3:B:Off",   0xC,  8,  8); // Ch3 B ADC Offset value

    createStatusParam("Ch3:B:En",    0xD,  1, 15); // Ch3 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch3:B:Samp",  0xD,  1, 14); // Ch3 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch3:B:SL",    0xD,  1, 13); // Ch3 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch3:B:SlL",   0xD,  1, 12); // Ch3 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch3:B:OffL",  0xD,  1, 11); // Ch3 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch3:B:SlLi",  0xD,  1, 10); // Ch3 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch3:B:Ovr",   0xD,  1,  9); // Ch3 B Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch3:B:Slop",  0xD,  9,  0); // Ch3 B input offset value

    createStatusParam("Ch4:A:Slop",  0xE,  9,  8); // Ch4 A input offset value
    createStatusParam("Ch3:AdcMax",  0xE,  1,  7); // Ch3 got ADC max              (0=no,1=yes)
    createStatusParam("Ch3:AdcMin",  0xE,  1,  6); // Ch3 got ADC min              (0=no,1=yes)
    createStatusParam("Ch3:MDEv",    0xE,  1,  5); // Ch3 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch3:Ev",      0xE,  1,  4); // Ch3 got event                (0=no,1=yes)
    createStatusParam("Ch3:FifFul",  0xE,  1,  3); // Ch3 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch3:FifAFF",  0xE,  1,  2); // Ch3 FIFO almost full detec   (0=no,1=yes)
    createStatusParam("Ch3:FifAmF",  0xE,  1,  1); // Ch3 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch3:Data",    0xE,  1,  0); // Ch3 FIFO has data            (0=no,1=yes)

    createStatusParam("Ch4:A:Off",   0xF,  8,  8); // Ch4 A ADC Offset value
    createStatusParam("Ch4:A:En",    0xF,  1,  7); // Ch4 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch4:A:Samp",  0xF,  1,  6); // Ch4 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch4:A:SL",    0xF,  1,  5); // Ch4 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch4:A:SlL",   0xF,  1,  4); // Ch4 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch4:A:OffL",  0xF,  1,  3); // Ch4 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch4:A:SlLi",  0xF,  1,  2); // Ch4 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch4:A:Ovr",   0xF,  1,  1); // Ch4 A Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch4:B:Slop", 0x10,  9,  8); // Ch4 B input offset value
    createStatusParam("Ch4:B:Off",  0x10,  8,  0); // Ch4 B ADC Offset value

    createStatusParam("Ch4:AdcMax", 0x11,  1, 15); // Ch4 got ADC max              (0=no,1=yes)
    createStatusParam("Ch4:AdcMin", 0x11,  1, 14); // Ch4 got ADC min              (0=no,1=yes)
    createStatusParam("Ch4:MDEv",   0x11,  1, 13); // Ch4 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch4:Ev",     0x11,  1, 12); // Ch4 got event                (0=no,1=yes)
    createStatusParam("Ch4:FifFul", 0x11,  1, 11); // Ch4 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch4:FifAFF", 0x11,  1, 10); // Ch4 FIFO almost full detec   (0=no,1=yes)
    createStatusParam("Ch4:FifAmF", 0x11,  1,  9); // Ch4 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch4:Data",   0x11,  1,  8); // Ch4 FIFO has data            (0=no,1=yes)
    createStatusParam("Ch4:B:En",   0x11,  1,  7); // Ch4 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch4:B:Samp", 0x11,  1,  6); // Ch4 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch4:B:SL",   0x11,  1,  5); // Ch4 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch4:B:SlL",  0x11,  1,  4); // Ch4 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch4:B:OffL", 0x11,  1,  3); // Ch4 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch4:B:SlLi", 0x11,  1,  2); // Ch4 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch4:B:Ovr",  0x11,  1,  1); // Ch4 B Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch5:A:En",   0x12,  1, 15); // Ch5 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch5:A:Samp", 0x12,  1, 14); // Ch5 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch5:A:SL",   0x12,  1, 13); // Ch5 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch5:A:SlL",  0x12,  1, 12); // Ch5 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch5:A:OffL", 0x12,  1, 11); // Ch5 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch5:A:SlLi", 0x12,  1, 10); // Ch5 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch5:A:Ovr",  0x12,  1,  9); // Ch5 A Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch5:A:Slop", 0x12,  9,  0); // Ch5 A Input Offset value

    createStatusParam("Ch5:B:Off",  0x13,  8,  8); // Ch5 B ADC Offset value
    createStatusParam("Ch5:A:Off",  0x13,  8,  0); // Ch5 A ADC Offset value

    createStatusParam("Ch5:B:En",   0x14,  1, 15); // Ch5 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch5:B:Samp", 0x14,  1, 14); // Ch5 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch5:B:SL",   0x14,  1, 13); // Ch5 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch5:B:SlL",  0x14,  1, 12); // Ch5 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch5:B:OffL", 0x14,  1, 11); // Ch5 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch5:B:SlLi", 0x14,  1, 10); // Ch5 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch5:B:Ovr",  0x14,  1,  9); // Ch5 B Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch5:B:Slop", 0x14,  9,  0); // Ch5 B Input Offset value

    createStatusParam("Ch6:A:Slop", 0x15,  9,  8); // Ch6 A input offset value
    createStatusParam("Ch5:AdcMax", 0x15,  1,  7); // Ch5 got ADC max              (0=no,1=yes)
    createStatusParam("Ch5:AdcMin", 0x15,  1,  6); // Ch5 got ADC min              (0=no,1=yes)
    createStatusParam("Ch5:MDEv",   0x15,  1,  5); // Ch5 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch5:Ev",     0x15,  1,  4); // Ch5 got event                (0=no,1=yes)
    createStatusParam("Ch5:FifFul", 0x15,  1,  3); // Ch5 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch5:FifAFF", 0x15,  1,  2); // Ch5 FIFO almost full detec   (0=no,1=yes)
    createStatusParam("Ch5:FifAmF", 0x15,  1,  1); // Ch5 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch5:Data",   0x15,  1,  0); // Ch5 FIFO has data            (0=no,1=yes)

    createStatusParam("Ch6:A:Off",  0x16,  8,  8); // Ch6 A ADC Offset value
    createStatusParam("Ch6:A:En",   0x16,  1,  7); // Ch6 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch6:A:Samp", 0x16,  1,  6); // Ch6 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch6:A:SL",   0x16,  1,  5); // Ch6 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch6:A:SlL",  0x16,  1,  4); // Ch6 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch6:A:OffL", 0x16,  1,  3); // Ch6 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch6:A:SlLi", 0x16,  1,  2); // Ch6 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch6:A:Ovr",  0x16,  1,  1); // Ch6 A Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch6:B:Slop", 0x17,  9,  8); // Ch6 B input offset value
    createStatusParam("Ch6:B:Off",  0x17,  8,  0); // Ch6 B ADC Offset value

    createStatusParam("Ch6:AdcMax", 0x18,  1, 15); // Ch6 got ADC max              (0=no,1=yes)
    createStatusParam("Ch6:AdcMin", 0x18,  1, 14); // Ch6 got ADC min              (0=no,1=yes)
    createStatusParam("Ch6:MDEv",   0x18,  1, 13); // Ch6 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch6:Ev",     0x18,  1, 12); // Ch6 got event                (0=no,1=yes)
    createStatusParam("Ch6:FifFul", 0x18,  1, 11); // Ch6 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch6:FifAFF", 0x18,  1, 10); // Ch6 FIFO almost full detec   (0=no,1=yes)
    createStatusParam("Ch6:FifAmF", 0x18,  1,  9); // Ch6 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch6:Data",   0x18,  1,  8); // Ch6 FIFO has data            (0=no,1=yes)
    createStatusParam("Ch6:B:En",   0x18,  1,  7); // Ch6 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch6:B:Samp", 0x18,  1,  6); // Ch6 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch6:B:SL",   0x18,  1,  5); // Ch6 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch6:B:SlL",  0x18,  1,  4); // Ch6 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch6:B:OffL", 0x18,  1,  3); // Ch6 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch6:B:SlLi", 0x18,  1,  2); // Ch6 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch6:B:Ovr",  0x18,  1,  1); // Ch6 B Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch7:A:En",   0x19,  1, 15); // Ch7 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch7:A:Samp", 0x19,  1, 14); // Ch7 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch7:A:SL",   0x19,  1, 13); // Ch7 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch7:A:SlL",  0x19,  1, 12); // Ch7 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch7:A:OffL", 0x19,  1, 11); // Ch7 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch7:A:SlLi", 0x19,  1, 10); // Ch7 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch7:A:Ovr",  0x19,  1,  9); // Ch7 A Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch7:A:Slop", 0x19,  9,  0); // Ch7 A input offset value

    createStatusParam("Ch7:A:Off",  0x1A,  8,  0); // Ch7 A ADC Offset value
    createStatusParam("Ch7:B:Off",  0x1A,  8,  8); // Ch7 B ADC Offset value

    createStatusParam("Ch7:B:En",   0x1B,  1, 15); // Ch7 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch7:B:Samp", 0x1B,  1, 14); // Ch7 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch7:B:SL",   0x1B,  1, 13); // Ch7 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch7:B:SlL",  0x1B,  1, 12); // Ch7 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch7:B:OffL", 0x1B,  1, 11); // Ch7 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch7:B:SlLi", 0x1B,  1, 10); // Ch7 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch7:B:Ovr",  0x1B,  1,  9); // Ch7 B Auto-Adjust overflow   (0=no,1=yes)
    createStatusParam("Ch7:B:Slop", 0x1B,  9,  0); // Ch7 B input offset value

    createStatusParam("Ch8:A:Slop", 0x1C,  9,  8); // Ch8 A input offset value
    createStatusParam("Ch7:AdcMax", 0x1C,  1,  7); // Ch7 got ADC max              (0=no,1=yes)
    createStatusParam("Ch7:AdcMin", 0x1C,  1,  6); // Ch7 got ADC min              (0=no,1=yes)
    createStatusParam("Ch7:MDEv",   0x1C,  1,  5); // Ch7 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch7:Ev",     0x1C,  1,  4); // Ch7 got event                (0=no,1=yes)
    createStatusParam("Ch7:FifFul", 0x1C,  1,  3); // Ch7 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch7:FifAFF", 0x1C,  1,  2); // Ch7 FIFO almost full detec   (0=no,1=yes)
    createStatusParam("Ch7:FifAmF", 0x1C,  1,  1); // Ch7 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch7:Data",   0x1C,  1,  0); // Ch7 FIFO has data            (0=no,1=yes)

    createStatusParam("Ch8:A:Off",  0x1D,  8,  8); // Ch8 A ADC Offset value
    createStatusParam("Ch8:A:En",   0x1D,  1,  7); // Ch8 A Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch8:A:Samp", 0x1D,  1,  6); // Ch8 A Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch8:A:SL",   0x1D,  1,  5); // Ch8 A Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch8:A:SlL",  0x1D,  1,  4); // Ch8 A Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch8:A:OffL", 0x1D,  1,  3); // Ch8 A Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch8:A:SlLi", 0x1D,  1,  2); // Ch8 A Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch8:A:Ovr",  0x1D,  1,  1); // Ch8 A Auto-Adjust overflow   (0=no,1=yes)

    createStatusParam("Ch8:B:Slop", 0x1E,  9,  8); // Ch8 B input offset value
    createStatusParam("Ch8:B:Off",  0x1E,  8,  0); // Ch8 B ADC Offset value

    createStatusParam("Ch8:AdcMax", 0x1F,  1, 15); // Ch8 got ADC max              (0=no,1=yes)
    createStatusParam("Ch8:AdcMin", 0x1F,  1, 14); // Ch8 got ADC min              (0=no,1=yes)
    createStatusParam("Ch8:MDEv",   0x1F,  1, 13); // Ch8 got multi-discp event    (0=no,1=yes)
    createStatusParam("Ch8:Ev",     0x1F,  1, 12); // Ch8 got event                (0=no,1=yes)
    createStatusParam("Ch8:FifFul", 0x1F,  1, 11); // Ch8 FIFO full detectec       (0=no,1=yes)
    createStatusParam("Ch8:FifAFF", 0x1F,  1, 10); // Ch8 FIFO almost full detec   (0=no,1=yes)
    createStatusParam("Ch8:FifAmF", 0x1F,  1,  9); // Ch8 FIFO almost full         (0=no,1=yes)
    createStatusParam("Ch8:Data",   0x1F,  1,  8); // Ch8 FIFO has data            (0=no,1=yes)
    createStatusParam("Ch8:B:En",   0x1F,  1,  7); // Ch8 B Auto-Adjust Active     (0=not active,1=active)
    createStatusParam("Ch8:B:Samp", 0x1F,  1,  6); // Ch8 B Auto-Adjust Got sample (0=no sample,1=got sample)
    createStatusParam("Ch8:B:SL",   0x1F,  1,  5); // Ch8 B Auto-Adjust Sample lim (0=not active,1=active)
    createStatusParam("Ch8:B:SlL",  0x1F,  1,  4); // Ch8 B Auto-Adjust Slope lim  (0=not active,1=active)
    createStatusParam("Ch8:B:OffL", 0x1F,  1,  3); // Ch8 B Auto-Adjust Offset lim (0=not active,1=active)
    createStatusParam("Ch8:B:SlLi", 0x1F,  1,  2); // Ch8 B Auto-Adjust Slope? lim (0=not active,1=active)
    createStatusParam("Ch8:B:Ovr",  0x1F,  1,  1); // Ch8 B Auto-Adjust overflow   (0=no,1=yes)
}

void RocPlugin::createConfigParams_v52()
{
//    BLXXX:Det:RocXXX:| sig nam|                                | EPICS record description | (bi and mbbi description)
    createConfigParam("Ch1:PosIdx", '1', 0x0,  32, 0, 0);     // Chan1 position index
    createConfigParam("Ch2:PosIdx", '1', 0x2,  32, 0, 256);   // Chan2 position index
    createConfigParam("Ch3:PosIdx", '1', 0x4,  32, 0, 512);   // Chan3 position index
    createConfigParam("Ch4:PosIdx", '1', 0x6,  32, 0, 768);   // Chan4 position index
    createConfigParam("Ch5:PosIdx", '1', 0x8,  32, 0, 1024);  // Chan5 position index
    createConfigParam("Ch6:PosIdx", '1', 0xA,  32, 0, 1280);  // Chan6 position index
    createConfigParam("Ch7:PosIdx", '1', 0xC,  32, 0, 1536);  // Chan7 position index
    createConfigParam("Ch8:PosIdx", '1', 0xE,  32, 0, 1792);  // Chan8 position index

    createConfigParam("Ch1:A:InOf", '2', 0x0,  12, 0, 100);   // Chan1 A input offset
    createConfigParam("Ch2:A:InOf", '2', 0x1,  12, 0, 100);   // Chan2 A input offset
    createConfigParam("Ch3:A:InOf", '2', 0x2,  12, 0, 100);   // Chan3 A input offset
    createConfigParam("Ch4:A:InOf", '2', 0x3,  12, 0, 100);   // Chan4 A input offset
    createConfigParam("Ch5:A:InOf", '2', 0x4,  12, 0, 100);   // Chan5 A input offset
    createConfigParam("Ch6:A:InOf", '2', 0x5,  12, 0, 100);   // Chan6 A input offset
    createConfigParam("Ch7:A:InOf", '2', 0x6,  12, 0, 100);   // Chan7 A input offset
    createConfigParam("Ch8:A:InOf", '2', 0x7,  12, 0, 100);   // Chan8 A input offset
    createConfigParam("Ch1:B:InOf", '2', 0x8,  12, 0, 100);   // Chan1 B input offset
    createConfigParam("Ch2:B:InOf", '2', 0x9,  12, 0, 100);   // Chan2 B input offset
    createConfigParam("Ch3:B:InOf", '2', 0xA,  12, 0, 100);   // Chan3 B input offset
    createConfigParam("Ch4:B:InOf", '2', 0xB,  12, 0, 100);   // Chan4 B input offset
    createConfigParam("Ch5:B:InOf", '2', 0xC,  12, 0, 100);   // Chan5 B input offset
    createConfigParam("Ch6:B:InOf", '2', 0xD,  12, 0, 100);   // Chan6 B input offset
    createConfigParam("Ch7:B:InOf", '2', 0xE,  12, 0, 100);   // Chan7 B input offset
    createConfigParam("Ch8:B:InOf", '2', 0xF,  12, 0, 100);   // Chan8 B input offset

    createConfigParam("Ch1:A:AdcO", '2', 0x10, 12, 0, 100);   // Chan1 A ADC offset
    createConfigParam("Ch2:A:AdcO", '2', 0x11, 12, 0, 100);   // Chan2 A ADC offset
    createConfigParam("Ch3:A:AdcO", '2', 0x12, 12, 0, 100);   // Chan3 A ADC offset
    createConfigParam("Ch4:A:AdcO", '2', 0x13, 12, 0, 100);   // Chan4 A ADC offset
    createConfigParam("Ch5:A:AdcO", '2', 0x14, 12, 0, 100);   // Chan5 A ADC offset
    createConfigParam("Ch6:A:AdcO", '2', 0x15, 12, 0, 100);   // Chan6 A ADC offset
    createConfigParam("Ch7:A:AdcO", '2', 0x16, 12, 0, 100);   // Chan7 A ADC offset
    createConfigParam("Ch8:A:AdcO", '2', 0x17, 12, 0, 100);   // Chan8 A ADC offset
    createConfigParam("Ch1:B:AdcO", '2', 0x18, 12, 0, 100);   // Chan1 B ADC offset
    createConfigParam("Ch2:B:AdcO", '2', 0x19, 12, 0, 100);   // Chan2 B ADC offset
    createConfigParam("Ch3:B:AdcO", '2', 0x1A, 12, 0, 100);   // Chan3 B ADC offset
    createConfigParam("Ch4:B:AdcO", '2', 0x1B, 12, 0, 100);   // Chan4 B ADC offset
    createConfigParam("Ch5:B:AdcO", '2', 0x1C, 12, 0, 100);   // Chan5 B ADC offset
    createConfigParam("Ch6:B:AdcO", '2', 0x1D, 12, 0, 100);   // Chan6 B ADC offset
    createConfigParam("Ch7:B:AdcO", '2', 0x1E, 12, 0, 100);   // Chan7 B ADC offset
    createConfigParam("Ch8:B:AdcO", '2', 0x1F, 12, 0, 100);   // Chan8 B ADC offset

    createConfigParam("Ch1:A:Thrs", '2', 0x20, 12, 0, 400);   // Chan1 A threshold
    createConfigParam("Ch2:A:Thrs", '2', 0x21, 12, 0, 400);   // Chan2 A threshold
    createConfigParam("Ch3:A:Thrs", '2', 0x22, 12, 0, 400);   // Chan3 A threshold
    createConfigParam("Ch4:A:Thrs", '2', 0x23, 12, 0, 400);   // Chan4 A threshold
    createConfigParam("Ch5:A:Thrs", '2', 0x24, 12, 0, 400);   // Chan5 A threshold
    createConfigParam("Ch6:A:Thrs", '2', 0x25, 12, 0, 400);   // Chan6 A threshold
    createConfigParam("Ch7:A:Thrs", '2', 0x26, 12, 0, 400);   // Chan7 A threshold
    createConfigParam("Ch8:A:Thrs", '2', 0x27, 12, 0, 400);   // Chan8 A threshold
    createConfigParam("Ch1:B:Thrs", '2', 0x28, 12, 0, 400);   // Chan1 B threshold
    createConfigParam("Ch2:B:Thrs", '2', 0x29, 12, 0, 400);   // Chan2 B threshold
    createConfigParam("Ch3:B:Thrs", '2', 0x2A, 12, 0, 400);   // Chan3 B threshold
    createConfigParam("Ch4:B:Thrs", '2', 0x2B, 12, 0, 400);   // Chan4 B threshold
    createConfigParam("Ch5:B:Thrs", '2', 0x2C, 12, 0, 400);   // Chan5 B threshold
    createConfigParam("Ch6:B:Thrs", '2', 0x2D, 12, 0, 400);   // Chan6 B threshold
    createConfigParam("Ch7:B:Thrs", '2', 0x2E, 12, 0, 400);   // Chan7 B threshold
    createConfigParam("Ch8:B:Thrs", '2', 0x2F, 12, 0, 400);   // Chan8 B threshold

    createConfigParam("Ch1:PosThr", '2', 0x30, 12, 0, 400);   // Chan1 positive threshold
    createConfigParam("Ch2:PosThr", '2', 0x31, 12, 0, 400);   // Chan2 positive threshold
    createConfigParam("Ch3:PosThr", '2', 0x32, 12, 0, 400);   // Chan3 positive threshold
    createConfigParam("Ch4:PosThr", '2', 0x33, 12, 0, 400);   // Chan4 positive threshold
    createConfigParam("Ch5:PosThr", '2', 0x34, 12, 0, 400);   // Chan5 positive threshold
    createConfigParam("Ch6:PosThr", '2', 0x35, 12, 0, 400);   // Chan6 positive threshold
    createConfigParam("Ch7:PosThr", '2', 0x36, 12, 0, 400);   // Chan7 positive threshold
    createConfigParam("Ch8:PosThr", '2', 0x37, 12, 0, 400);   // Chan8 positive threshold
    createConfigParam("Ch1:A:FSca", '2', 0x38, 12, 0, 10);    // Chan1 A full scale
    createConfigParam("Ch2:A:FSca", '2', 0x39, 12, 0, 10);    // Chan2 A full scale
    createConfigParam("Ch3:A:FSca", '2', 0x3A, 12, 0, 10);    // Chan3 A full scale
    createConfigParam("Ch4:A:FSca", '2', 0x3B, 12, 0, 10);    // Chan4 A full scale
    createConfigParam("Ch5:A:FSca", '2', 0x3C, 12, 0, 10);    // Chan5 A full scale
    createConfigParam("Ch6:A:FSca", '2', 0x3D, 12, 0, 10);    // Chan6 A full scale
    createConfigParam("Ch7:A:FSca", '2', 0x3E, 12, 0, 10);    // Chan7 A full scale
    createConfigParam("Ch8:A:FSca", '2', 0x3F, 12, 0, 10);    // Chan8 A full scale

    createConfigParam("Ch1:B:FSca", '2', 0x40, 12, 0, 10);    // Chan1 B full scale
    createConfigParam("Ch2:B:FSca", '2', 0x41, 12, 0, 10);    // Chan2 B full scale
    createConfigParam("Ch3:B:FSca", '2', 0x42, 12, 0, 10);    // Chan3 B full scale
    createConfigParam("Ch4:B:FSca", '2', 0x43, 12, 0, 10);    // Chan4 B full scale
    createConfigParam("Ch5:B:FSca", '2', 0x44, 12, 0, 10);    // Chan5 B full scale
    createConfigParam("Ch6:B:FSca", '2', 0x45, 12, 0, 10);    // Chan6 B full scale
    createConfigParam("Ch7:B:FSca", '2', 0x46, 12, 0, 10);    // Chan7 B full scale
    createConfigParam("Ch8:B:FSca", '2', 0x47, 12, 0, 10);    // Chan8 B full scale
    createConfigParam("Ch1:A:GAdj", '2', 0x48, 12, 0, 0);     // Chan1 A gain adjust
    createConfigParam("Ch2:A:GAdj", '2', 0x49, 12, 0, 0);     // Chan2 A gain adjust
    createConfigParam("Ch3:A:GAdj", '2', 0x4A, 12, 0, 0);     // Chan3 A gain adjust
    createConfigParam("Ch4:A:GAdj", '2', 0x4B, 12, 0, 0);     // Chan4 A gain adjust
    createConfigParam("Ch5:A:GAdj", '2', 0x4C, 12, 0, 0);     // Chan5 A gain adjust
    createConfigParam("Ch6:A:GAdj", '2', 0x4D, 12, 0, 0);     // Chan6 A gain adjust
    createConfigParam("Ch7:A:GAdj", '2', 0x4E, 12, 0, 0);     // Chan7 A gain adjust
    createConfigParam("Ch8:A:GAdj", '2', 0x4F, 12, 0, 0);     // Chan8 A gain adjust

    createConfigParam("Ch1:B:GAdj", '2', 0x50, 12, 0, 0);     // Chan1 B gain adjust
    createConfigParam("Ch2:B:GAdj", '2', 0x51, 12, 0, 0);     // Chan2 B gain adjust
    createConfigParam("Ch3:B:GAdj", '2', 0x52, 12, 0, 0);     // Chan3 B gain adjust
    createConfigParam("Ch4:B:GAdj", '2', 0x53, 12, 0, 0);     // Chan4 B gain adjust
    createConfigParam("Ch5:B:GAdj", '2', 0x54, 12, 0, 0);     // Chan5 B gain adjust
    createConfigParam("Ch6:B:GAdj", '2', 0x55, 12, 0, 0);     // Chan6 B gain adjust
    createConfigParam("Ch7:B:GAdj", '2', 0x56, 12, 0, 0);     // Chan7 B gain adjust
    createConfigParam("Ch8:B:GAdj", '2', 0x57, 12, 0, 0);     // Chan8 B gain adjust

    createConfigParam("Ch1:A:Scal", '3', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch2:A:Scal", '3', 0x1,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch3:A:Scal", '3', 0x2,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch4:A:Scal", '3', 0x3,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch5:A:Scal", '3', 0x4,  12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch6:A:Scal", '3', 0x5,  12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch7:A:Scal", '3', 0x6,  12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch8:A:Scal", '3', 0x7,  12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch1:B:Scal", '3', 0x8,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch2:B:Scal", '3', 0x9,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch3:B:Scal", '3', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch4:B:Scal", '3', 0xB,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch5:B:Scal", '3', 0xC,  12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch6:B:Scal", '3', 0xD,  12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch7:B:Scal", '3', 0xE,  12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch8:B:Scal", '3', 0xF,  12, 0, 2048);  // Chan8 B scale

    createConfigParam("Ch1:A:Offs", '3', 0x10, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch2:A:Offs", '3', 0x11, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch3:A:Offs", '3', 0x12, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch4:A:Offs", '3', 0x13, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch5:A:Offs", '3', 0x14, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch6:A:Offs", '3', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch7:A:Offs", '3', 0x16, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch8:A:Offs", '3', 0x17, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch1:B:Offs", '3', 0x18, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:B:Offs", '3', 0x19, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:B:Offs", '3', 0x1A, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:B:Offs", '3', 0x1B, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5:B:Offs", '3', 0x1C, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:B:Offs", '3', 0x1D, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:B:Offs", '3', 0x1E, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:B:Offs", '3', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleF", '4', 0x0,  12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleF", '4', 0x1,  12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleF", '4', 0x2,  12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleF", '4', 0x3,  12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleF", '4', 0x4,  12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleF", '4', 0x5,  12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleF", '4', 0x6,  12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleF", '4', 0x7,  12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1:A:AvMi", 'C', 0x0,  12, 0, 100);   // Chan1 A average minimum
    createConfigParam("Ch2:A:AvMi", 'C', 0x1,  12, 0, 100);   // Chan2 A average minimum
    createConfigParam("Ch3:A:AvMi", 'C', 0x2,  12, 0, 100);   // Chan3 A average minimum
    createConfigParam("Ch4:A:AvMi", 'C', 0x3,  12, 0, 100);   // Chan4 A average minimum
    createConfigParam("Ch5:A:AvMi", 'C', 0x4,  12, 0, 100);   // Chan5 A average minimum
    createConfigParam("Ch6:A:AvMi", 'C', 0x5,  12, 0, 100);   // Chan6 A average minimum
    createConfigParam("Ch7:A:AvMi", 'C', 0x6,  12, 0, 100);   // Chan7 A average minimum
    createConfigParam("Ch8:A:AvMi", 'C', 0x7,  12, 0, 100);   // Chan8 A average minimum
    createConfigParam("Ch1:B:AvMi", 'C', 0x8,  12, 0, 100);   // Chan1 B average minimum
    createConfigParam("Ch2:B:AvMi", 'C', 0x9,  12, 0, 100);   // Chan2 B average minimum
    createConfigParam("Ch3:B:AvMi", 'C', 0xA,  12, 0, 100);   // Chan3 B average minimum
    createConfigParam("Ch4:B:AvMi", 'C', 0xB,  12, 0, 100);   // Chan4 B average minimum
    createConfigParam("Ch5:B:AvMi", 'C', 0xC,  12, 0, 100);   // Chan5 B average minimum
    createConfigParam("Ch6:B:AvMi", 'C', 0xD,  12, 0, 100);   // Chan6 B average minimum
    createConfigParam("Ch7:B:AvMi", 'C', 0xE,  12, 0, 100);   // Chan7 B average minimum
    createConfigParam("Ch8:B:AvMi", 'C', 0xF,  12, 0, 100);   // Chan8 B average minimum

    createConfigParam("Ch1:A:AvMa", 'C', 0x10, 12, 0, 1000);  // Chan1 A average maximum
    createConfigParam("Ch2:A:AvMa", 'C', 0x11, 12, 0, 1000);  // Chan2 A average maximum
    createConfigParam("Ch3:A:AvMa", 'C', 0x12, 12, 0, 1000);  // Chan3 A average maximum
    createConfigParam("Ch4:A:AvMa", 'C', 0x13, 12, 0, 1000);  // Chan4 A average maximum
    createConfigParam("Ch5:A:AvMa", 'C', 0x14, 12, 0, 1000);  // Chan5 A average maximum
    createConfigParam("Ch6:A:AvMa", 'C', 0x15, 12, 0, 1000);  // Chan6 A average maximum
    createConfigParam("Ch7:A:AvMa", 'C', 0x16, 12, 0, 1000);  // Chan7 A average maximum
    createConfigParam("Ch8:A:AvMa", 'C', 0x17, 12, 0, 1000);  // Chan8 A average maximum
    createConfigParam("Ch1:B:AvMa", 'C', 0x18, 12, 0, 1000);  // Chan1 B average maximum
    createConfigParam("Ch2:B:AvMa", 'C', 0x19, 12, 0, 1000);  // Chan2 B average maximum
    createConfigParam("Ch3:B:AvMa", 'C', 0x1A, 12, 0, 1000);  // Chan3 B average maximum
    createConfigParam("Ch4:B:AvMa", 'C', 0x1B, 12, 0, 1000);  // Chan4 B average maximum
    createConfigParam("Ch5:B:AvMa", 'C', 0x1C, 12, 0, 1000);  // Chan5 B average maximum
    createConfigParam("Ch6:B:AvMa", 'C', 0x1D, 12, 0, 1000);  // Chan6 B average maximum
    createConfigParam("Ch7:B:AvMa", 'C', 0x1E, 12, 0, 1000);  // Chan7 B average maximum
    createConfigParam("Ch8:B:AvMa", 'C', 0x1F, 12, 0, 1000);  // Chan8 B average maximum

    createConfigParam("Ch1:A:SaMi", 'D', 0x0,  12, 0, 100);   // Chan1 A sample minimum
    createConfigParam("Ch2:A:SaMi", 'D', 0x1,  12, 0, 100);   // Chan2 A sample minimum
    createConfigParam("Ch3:A:SaMi", 'D', 0x2,  12, 0, 100);   // Chan3 A sample minimum
    createConfigParam("Ch4:A:SaMi", 'D', 0x3,  12, 0, 100);   // Chan4 A sample minimum
    createConfigParam("Ch5:A:SaMi", 'D', 0x4,  12, 0, 100);   // Chan5 A sample minimum
    createConfigParam("Ch6:A:SaMi", 'D', 0x5,  12, 0, 100);   // Chan6 A sample minimum
    createConfigParam("Ch7:A:SaMi", 'D', 0x6,  12, 0, 100);   // Chan7 A sample minimum
    createConfigParam("Ch8:A:SaMi", 'D', 0x7,  12, 0, 100);   // Chan8 A sample minimum
    createConfigParam("Ch1:B:SaMi", 'D', 0x8,  12, 0, 100);   // Chan1 B sample minimum
    createConfigParam("Ch2:B:SaMi", 'D', 0x9,  12, 0, 100);   // Chan2 B sample minimum
    createConfigParam("Ch3:B:SaMi", 'D', 0xA,  12, 0, 100);   // Chan3 B sample minimum
    createConfigParam("Ch4:B:SaMi", 'D', 0xB,  12, 0, 100);   // Chan4 B sample minimum
    createConfigParam("Ch5:B:SaMi", 'D', 0xC,  12, 0, 100);   // Chan5 B sample minimum
    createConfigParam("Ch6:B:SaMi", 'D', 0xD,  12, 0, 100);   // Chan6 B sample minimum
    createConfigParam("Ch7:B:SaMi", 'D', 0xE,  12, 0, 100);   // Chan7 B sample minimum
    createConfigParam("Ch8:B:SaMi", 'D', 0xF,  12, 0, 100);   // Chan8 B sample minimum

    createConfigParam("Ch1:A:SaMa", 'D', 0x10, 12, 0, 1000);  // Chan1 A sample maximum
    createConfigParam("Ch2:A:SaMa", 'D', 0x11, 12, 0, 1000);  // Chan2 A sample maximum
    createConfigParam("Ch3:A:SaMa", 'D', 0x12, 12, 0, 1000);  // Chan3 A sample maximum
    createConfigParam("Ch4:A:SaMa", 'D', 0x13, 12, 0, 1000);  // Chan4 A sample maximum
    createConfigParam("Ch5:A:SaMa", 'D', 0x14, 12, 0, 1000);  // Chan5 A sample maximum
    createConfigParam("Ch6:A:SaMa", 'D', 0x15, 12, 0, 1000);  // Chan6 A sample maximum
    createConfigParam("Ch7:A:SaMa", 'D', 0x16, 12, 0, 1000);  // Chan7 A sample maximum
    createConfigParam("Ch8:A:SaMa", 'D', 0x17, 12, 0, 1000);  // Chan8 A sample maximum
    createConfigParam("Ch1:B:SaMa", 'D', 0x18, 12, 0, 1000);  // Chan1 B sample maximum
    createConfigParam("Ch2:B:SaMa", 'D', 0x19, 12, 0, 1000);  // Chan2 B sample maximum
    createConfigParam("Ch3:B:SaMa", 'D', 0x1A, 12, 0, 1000);  // Chan3 B sample maximum
    createConfigParam("Ch4:B:SaMa", 'D', 0x1B, 12, 0, 1000);  // Chan4 B sample maximum
    createConfigParam("Ch5:B:SaMa", 'D', 0x1C, 12, 0, 1000);  // Chan5 B sample maximum
    createConfigParam("Ch6:B:SaMa", 'D', 0x1D, 12, 0, 1000);  // Chan6 B sample maximum
    createConfigParam("Ch7:B:SaMa", 'D', 0x1E, 12, 0, 1000);  // Chan7 B sample maximum
    createConfigParam("Ch8:B:SaMa", 'D', 0x1F, 12, 0, 1000);  // Chan8 B sample maximum

    createConfigParam("Ch8:B:SlMa", 'D', 0x20, 12, 0, 0);     // Maximum slope

    createConfigParam("Ch1:En",     'E', 0x0,  1,  0, 1);     // Chan1 enable                 (0=disable,1=enable)
    createConfigParam("Ch2:En",     'E', 0x0,  1,  1, 1);     // Chan2 enable                 (0=disable,1=enable)
    createConfigParam("Ch3:En",     'E', 0x0,  1,  2, 1);     // Chan3 enable                 (0=disable,1=enable)
    createConfigParam("Ch4:En",     'E', 0x0,  1,  3, 1);     // Chan4 enable                 (0=disable,1=enable)
    createConfigParam("Ch5:En",     'E', 0x0,  1,  4, 1);     // Chan5 enable                 (0=disable,1=enable)
    createConfigParam("Ch6:En",     'E', 0x0,  1,  5, 1);     // Chan6 enable                 (0=disable,1=enable)
    createConfigParam("Ch7:En",     'E', 0x0,  1,  6, 1);     // Chan7 enable                 (0=disable,1=enable)
    createConfigParam("Ch8:En",     'E', 0x0,  1,  7, 1);     // Chan8 enable                 (0=disable,1=enable)

    createConfigParam("Ch1:VerbEn", 'E', 0x0,  1,  8, 0);     // Chan1 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch2:VerbEn", 'E', 0x0,  1,  9, 0);     // Chan2 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch3:VerbEn", 'E', 0x0,  1, 10, 0);     // Chan3 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch4:VerbEn", 'E', 0x0,  1, 11, 0);     // Chan4 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch5:VerbEn", 'E', 0x0,  1, 12, 0);     // Chan5 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch6:VerbEn", 'E', 0x0,  1, 13, 0);     // Chan6 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch7:VerbEn", 'E', 0x0,  1, 14, 0);     // Chan7 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch8:VerbEn", 'E', 0x0,  1, 15, 0);     // Chan8 verbose enable         (0=disable,1=enable)

    createConfigParam("TimeVetoLo", 'E', 0x1,  32, 0, 0);     // Timestamp veto low
    createConfigParam("TimeVetoHi", 'E', 0x3,  32, 0, 0);     // Timestamp veto high
    createConfigParam("FakTrigDly", 'E', 0x5,  16, 0, 20000); // Fake trigger delay
    createConfigParam("MinAdcVal",  'E', 0x6,  16, 0, 100);   // Minimum ADC
    createConfigParam("MaxAdcVal",  'E', 0x7,  16, 0, 1000);  // Maximum ADC
    createConfigParam("IntRelease", 'E', 0x8,  16, 0, -6);    // Integrator releas pnt [-6:-1]
    createConfigParam("MinDisWidt", 'E', 0x9,  12, 0, 1);     // Minimum discriminator width
    createConfigParam("Sample1",    'E', 0xA,  4,  0, 2);     // Sample 1 point [0:10]
    createConfigParam("Sample2",    'E', 0xB,  6,  0, 14);    // Sample 1 point [2:50]
    createConfigParam("TsyncDelay", 'E', 0xC,  32, 0, 0);     // TSYNC delay

    createConfigParam("LvdsRate",   'F', 0x0,  1, 15, 1);     // LVDS output rate             (0=20Mhz,1=40Mhz)
    createConfigParam("AutoCorSam", 'F', 0x0,  1, 14, 0);     // Auto correction sample mode  (0=TSYNC,1=fake trigger)
    createConfigParam("EdgeDetEn",  'F', 0x0,  1, 13, 1);     // Edge detect enable           (0=disable,1=enable)
    createConfigParam("MstDiscSel", 'F', 0x0,  2, 11, 0);     // Master discriminator select  (0=SUM discr,1=A discr,2=B discr,3=all)
    createConfigParam("AcqEn",      'F', 0x0,  1, 10, 1);     // ROC enable acquisition       (0=disable,1=enable)
    createConfigParam("AutoCorrEn", 'F', 0x0,  1, 9,  0);     // Auto correction mode         (0=enabled,1=disabled)
    createConfigParam("HighResMod", 'F', 0x0,  1, 8,  1);     // High resolution mode         (0=low res 0-127,1=high res 0-255)
    createConfigParam("OutputMode", 'F', 0x0,  2, 6,  0);     // Output mode                  (0=normal,1=raw,2=extended)
    createConfigParam("AcqMode",    'F', 0x0,  2, 4,  0);     // Output mode                  (0=normal,1=verbose,2=fake trigger)
    createConfigParam("TxEnable",   'F', 0x0,  1, 3,  1);     // TX enable                    (0=external,1=always enabled)
    createConfigParam("TsyncSel",   'F', 0x0,  1, 2,  0);     // TSYNC select                 (0=external,1=internal 60Hz)
    createConfigParam("TclkSel",    'F', 0x0,  1, 1,  0);     // TCLK select                  (0=external,1=internal 10MHz)
    createConfigParam("Reset",      'F', 0x0,  1, 0,  0);     // Reset enable                 (0=disable,1=enable)
}
