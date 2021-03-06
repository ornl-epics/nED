/* AcpcFemPlugin_v22.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcFemPlugin.h"

void AcpcFemPlugin::createParams_v14()
{
    createRegParam("VERSION", "HwDay",  true, 0,  8,  0, 0, CONV_HEX2DEC); // Hardware day
    createRegParam("VERSION", "HwMonth",true, 0,  8,  8, 0, CONV_HEX2DEC); // Hardware month
    createRegParam("VERSION", "HwYear", true, 1,  8,  0, 0, CONV_HEX2DEC2K); // Hardware year
    createRegParam("VERSION", "HwRev",  true, 2,  8,  0); // Hardware revision
    createRegParam("VERSION", "HwVer",  true, 2,  8,  8); // Hardware version
    createRegParam("VERSION", "FwDay",  true, 3,  8,  0, 0, CONV_HEX2DEC); // Firmware day
    createRegParam("VERSION", "FwMonth",true, 3,  8,  8, 0, CONV_HEX2DEC); // Firmware month
    createRegParam("VERSION", "FwYear", true, 4,  8,  0, 0, CONV_HEX2DEC2K); // Firmware year
    createRegParam("VERSION", "FwRev",  true, 5,  8,  0); // Firmware revision                  (low:3,high:5)
    createRegParam("VERSION", "FwVer",  true, 5,  8,  8); // Firmware version                   (low:0,high:2)

//     BLXXX:Det:FemXX:| sig name |                          | EPICS record description | (bi and mbbi description)
    createStatusParam("Lvds2:ErrFifoFull",    0x0,  1, 15); // Chan2 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds2:ErrPreStart",    0x0,  1, 14); // Chan2 got START during packt (0=no error,1=error)
    createStatusParam("Lvds2:ErrNoStart",     0x0,  1, 13); // Chan2 got data without START (0=no error,1=error)
    createStatusParam("Lvds2:ErrTimeout",     0x0,  1, 12); // Chan2 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds2:ErrLength",      0x0,  1, 11); // Chan2 long packet error      (0=no error,1=error)
    createStatusParam("Lvds2:ErrShort",       0x0,  1, 10); // Chan2 short packet error     (0=no error,1=error)
    createStatusParam("Lvds2:ErrType",        0x0,  1,  9); // Chan2 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds2:ErrParity",      0x0,  1,  8); // Chan2 parity error           (0=no error,1=error)
    createStatusParam("Lvds1:ErrFifoFull",    0x0,  1,  7); // Chan1 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds1:ErrPreStart",    0x0,  1,  6); // Chan1 got START during packt (0=no error,1=error)
    createStatusParam("Lvds1:ErrNoStart",     0x0,  1,  5); // Chan1 got data without START (0=no error,1=error)
    createStatusParam("Lvds1:ErrTimeout",     0x0,  1,  4); // Chan1 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds1:ErrLength",      0x0,  1,  3); // Chan1 long packet error      (0=no error,1=error)
    createStatusParam("Lvds1:ErrShort",       0x0,  1,  2); // Chan1 short packet error     (0=no error,1=error)
    createStatusParam("Lvds1:ErrType",        0x0,  1,  1); // Chan1 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds1:ErrParity",      0x0,  1,  0); // Chan1 parity error           (0=no error,1=error)

    createStatusParam("Lvds4:ErrFifoFull",    0x1,  1, 15); // Chan4 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds4:ErrPreStart",    0x1,  1, 14); // Chan4 got START during packt (0=no error,1=error)
    createStatusParam("Lvds4:ErrNoStart",     0x1,  1, 13); // Chan4 got data without START (0=no error,1=error)
    createStatusParam("Lvds4:ErrTimeout",     0x1,  1, 12); // Chan4 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds4:ErrLength",      0x1,  1, 11); // Chan4 long packet error      (0=no error,1=error)
    createStatusParam("Lvds4:ErrShort",       0x1,  1, 10); // Chan4 short packet error     (0=no error,1=error)
    createStatusParam("Lvds4:ErrType",        0x1,  1,  9); // Chan4 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds4:ErrParity",      0x1,  1,  8); // Chan4 parity error           (0=no error,1=error)
    createStatusParam("Lvds3:ErrFifoFull",    0x1,  1,  7); // Chan3 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds3:ErrPreStart",    0x1,  1,  6); // Chan3 got START during packt (0=no error,1=error)
    createStatusParam("Lvds3:ErrNoStart",     0x1,  1,  5); // Chan3 got data without START (0=no error,1=error)
    createStatusParam("Lvds3:ErrTimeout",     0x1,  1,  4); // Chan3 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds3:ErrLength",      0x1,  1,  3); // Chan3 long packet error      (0=no error,1=error)
    createStatusParam("Lvds3:ErrShort",       0x1,  1,  2); // Chan3 short packet error     (0=no error,1=error)
    createStatusParam("Lvds3:ErrType",        0x1,  1,  1); // Chan3 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds3:ErrParity",      0x1,  1,  0); // Chan3 parity error           (0=no error,1=error)

    createStatusParam("Lvds6:ErrFifoFull",    0x2,  1, 15); // Chan6 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds6:ErrPreStart",    0x2,  1, 14); // Chan6 got START during packt (0=no error,1=error)
    createStatusParam("Lvds6:ErrNoStart",     0x2,  1, 13); // Chan6 got data without START (0=no error,1=error)
    createStatusParam("Lvds6:ErrTimeout",     0x2,  1, 12); // Chan6 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds6:ErrLength",      0x2,  1, 11); // Chan6 long packet error      (0=no error,1=error)
    createStatusParam("Lvds6:ErrShort",       0x2,  1, 10); // Chan6 short packet error     (0=no error,1=error)
    createStatusParam("Lvds6:ErrType",        0x2,  1,  9); // Chan6 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds6:ErrParity",      0x2,  1,  8); // Chan6 parity error           (0=no error,1=error)
    createStatusParam("Lvds5:ErrFifoFull",    0x2,  1,  7); // Chan5 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds5:ErrPreStart",    0x2,  1,  6); // Chan5 got START during pack  (0=no error,1=error)
    createStatusParam("Lvds5:ErrNoStart",     0x2,  1,  5); // Chan5 got data without START (0=no error,1=error)
    createStatusParam("Lvds5:ErrTimeout",     0x2,  1,  4); // Chan5 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds5:ErrLength",      0x2,  1,  3); // Chan5 long packet error      (0=no error,1=error)
    createStatusParam("Lvds5:ErrShort",       0x2,  1,  2); // Chan5 short packet error     (0=no error,1=error)
    createStatusParam("Lvds5:ErrType",        0x2,  1,  1); // Chan5 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds5:ErrParity",      0x2,  1,  0); // Chan5 parity error           (0=no error,1=error)

    createStatusParam("Lvds8:ErrFifoFull",    0x3,  1, 15); // Chan8 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds8:ErrPreStart",    0x3,  1, 14); // Chan8 got START during packt (0=no error,1=error)
    createStatusParam("Lvds8:ErrNoStart",     0x3,  1, 13); // Chan8 got data without START (0=no error,1=error)
    createStatusParam("Lvds8:ErrTimeout",     0x3,  1, 12); // Chan8 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds8:ErrLength",      0x3,  1, 11); // Chan8 long packet error      (0=no error,1=error)
    createStatusParam("Lvds8:ErrShort",       0x3,  1, 10); // Chan8 short packet error     (0=no error,1=error)
    createStatusParam("Lvds8:ErrType",        0x3,  1,  9); // Chan8 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds8:ErrParity",      0x3,  1,  8); // Chan8 parity error           (0=no error,1=error)
    createStatusParam("Lvds7:ErrFifoFull",    0x3,  1,  7); // Chan7 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds7:ErrPreStart",    0x3,  1,  6); // Chan7 got START during pack  (0=no error,1=error)
    createStatusParam("Lvds7:ErrNoStart",     0x3,  1,  5); // Chan7 got data without START (0=no error,1=error)
    createStatusParam("Lvds7:ErrTimeout",     0x3,  1,  4); // Chan7 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds7:ErrLength",      0x3,  1,  3); // Chan7 long packet error      (0=no error,1=error)
    createStatusParam("Lvds7:ErrShort",       0x3,  1,  2); // Chan7 short packet error     (0=no error,1=error)
    createStatusParam("Lvds7:ErrType",        0x3,  1,  1); // Chan7 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds7:ErrParity",      0x3,  1,  0); // Chan7 parity error           (0=no error,1=error)

    createStatusParam("Ctr:ErrFifoFull",      0x4,  1, 15); // CTRL FIFO went full          (0=no,1=yes)
    createStatusParam("Ctr:ErrPreStart",      0x4,  1, 14); // CTRL got START during packe  (0=no error,1=error)
    createStatusParam("Ctr:ErrNoStart",       0x4,  1, 13); // CTRL got data without START  (0=no error,1=error)
    createStatusParam("Ctr:ErrTimeout",       0x4,  1, 12); // CTRL got TIMEOUT             (0=no timeout,1=timeout)
    createStatusParam("Ctr:ErrLength",        0x4,  1, 11); // CTRL long packet error       (0=no error,1=error)
    createStatusParam("Ctr:ErrShort",         0x4,  1, 10); // CTRL short packet error      (0=no error,1=error)
    createStatusParam("Ctr:ErrType",          0x4,  1,  9); // CTRL DATA/COMMAND type err   (0=no error,1=error)
    createStatusParam("Ctr:ErrParity",        0x4,  1,  8); // CTRL parity error            (0=no error,1=error)
    createStatusParam("Lvds9:ErrFifoFull",    0x4,  1,  7); // Chan9 FIFO went full         (0=no,1=yes)
    createStatusParam("Lvds9:ErrPreStart",    0x4,  1,  6); // Chan9 got START during pack  (0=no error,1=error)
    createStatusParam("Lvds9:ErrNoStart",     0x4,  1,  5); // Chan9 got data without START (0=no error,1=error)
    createStatusParam("Lvds9:ErrTimeout",     0x4,  1,  4); // Chan9 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Lvds9:ErrLength",      0x4,  1,  3); // Chan9 long packet error      (0=no error,1=error)
    createStatusParam("Lvds9:ErrShort",       0x4,  1,  2); // Chan9 short packet error     (0=no error,1=error)
    createStatusParam("Lvds9:ErrType",        0x4,  1,  1); // Chan9 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Lvds9:ErrParity",      0x4,  1,  0); // Chan9 parity error           (0=no error,1=error)

    createStatusParam("CntFilterNack",        0x5,  8,  7); // Filtered NACKs
    createStatusParam("FilterNack",           0x5,  1,  6); // Filtered NACK                (0=no,1=yes)
    createStatusParam("ErrEeprom",            0x5,  1,  5); // EEPROM error                 (0=no error,1=error)
    createStatusParam("ErrNoHw",              0x5,  1,  4); // Missing hardware             (0=not missing,1=missing)
    createStatusParam("Configured",           0x5,  1,  3); // Configured - doesn't work    (0=not configured,1=configured)
    createStatusParam("ErrProgram",           0x5,  1,  2); // Programming error            (0=no error,1=error)
    createStatusParam("ErrCmdLength",         0x5,  1,  1); // Command length error         (0=no error,1=error)
    createStatusParam("ErrCmdBad",            0x5,  1,  0); // Unknown command              (0=no error,1=error)

    createStatusParam("Lvds8:GotCmd",         0x6,  1, 15); // Chan8 got command packet     (0=no,1=yes)
    createStatusParam("Lvds8:GotData",        0x6,  1, 14); // Chan8 got data packet        (0=no,1=yes)
    createStatusParam("Lvds7:GotCmd",         0x6,  1, 13); // Chan7 got command packet     (0=no,1=yes)
    createStatusParam("Lvds7:GotData",        0x6,  1, 12); // Chan7 got data packet        (0=no,1=yes)
    createStatusParam("Lvds6:GotCmd",         0x6,  1, 11); // Chan6 got command packet     (0=no,1=yes)
    createStatusParam("Lvds6:GotData",        0x6,  1, 10); // Chan6 got data packet        (0=no,1=yes)
    createStatusParam("Lvds5:GotCmd",         0x6,  1,  9); // Chan5 got command packet     (0=no,1=yes)
    createStatusParam("Lvds5:GotData",        0x6,  1,  8); // Chan5 got data packet        (0=no,1=yes)
    createStatusParam("Lvds4:GotCmd",         0x6,  1,  7); // Chan4 got command packet     (0=no,1=yes)
    createStatusParam("Lvds4:GotData",        0x6,  1,  6); // Chan4 got data packet        (0=no,1=yes)
    createStatusParam("Lvds3:GotCmd",         0x6,  1,  5); // Chan3 got command packet     (0=no,1=yes)
    createStatusParam("Lvds3:GotData",        0x6,  1,  4); // Chan3 got data packet        (0=no,1=yes)
    createStatusParam("Lvds2:GotCmd",         0x6,  1,  3); // Chan2 got command packet     (0=no,1=yes)
    createStatusParam("Lvds2:GotData",        0x6,  1,  2); // Chan2 got data packet        (0=no,1=yes)
    createStatusParam("Lvds1:GotCmd",         0x6,  1,  1); // Chan1 got command packet     (0=no,1=yes)
    createStatusParam("Lvds1:GotData",        0x6,  1,  0); // Chan1 got data packet        (0=no,1=yes)

    createStatusParam("CntFilterAck",         0x7,  8,  8); // Filtered ACKS
    createStatusParam("OutputMode",           0x7,  2,  6); // Data mode TODO verify values (0=normal mode,1=off,2=raw mode,3=verbose mode)
    createStatusParam("Acquiring",            0x7,  1,  5); // Acquiring data               (0=not acquiring,1=acquiring)
    createStatusParam("FoundHw",              0x7,  1,  4); // Found new hardware           (0=no,1=yes)
    createStatusParam("Ctr:GotCmd",           0x7,  1,  3); // CTRL got command packet      (0=no,1=yes)
    createStatusParam("Ctr:FifoHasData",      0x7,  1,  2); // CTRL got data packet         (0=no,1=yes)
    createStatusParam("Lvds9:GotCmd",         0x7,  1,  1); // Chan9 got command packet     (0=no,1=yes)
    createStatusParam("Lvds9:GotData",        0x7,  1,  0); // Chan9 got data packet        (0=no,1=yes)

    createStatusParam("Lvds8:FifoAlmostFull", 0x8,  1, 15); // Chan8 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds8:FifoHasData",    0x8,  1, 14); // Chan8 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds7:FifoAlmostFull", 0x8,  1, 13); // Chan7 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds7:FifoHasData",    0x8,  1, 12); // Chan7 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds6:FifoAlmostFull", 0x8,  1, 11); // Chan6 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds6:FifoHasData",    0x8,  1, 10); // Chan6 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds5:FifoAlmostFull", 0x8,  1,  9); // Chan5 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds5:FifoHasData",    0x8,  1,  8); // Chan5 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds4:FifoAlmostFull", 0x8,  1,  7); // Chan4 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds4:FifoHasData",    0x8,  1,  6); // Chan4 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds3:FifoAlmostFull", 0x8,  1,  5); // Chan3 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds3:FifoHasData",    0x8,  1,  4); // Chan3 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds2:FifoAlmostFull", 0x8,  1,  3); // Chan2 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds2:FifoHasData",    0x8,  1,  2); // Chan2 FIFO has data          (0=empty,1=has data)
    createStatusParam("Lvds1:FifoAlmostFull", 0x8,  1,  1); // Chan1 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds1:FifoHasData",    0x8,  1,  0); // Chan1 FIFO not empty         (0=empty,1=has data)

    createStatusParam("CmdFifoAlmostFull",    0x9,  1, 15); // CMD FIFO almost full         (0=no,1=yes)
    createStatusParam("CmdFifoHasData",       0x9,  1, 14); // CMD FIFO has data            (0=empty,1=has data)
    createStatusParam("CmdInFifoAlmostFull",  0x9,  1, 13); // CMD input FIFO almost full   (0=no,1=yes)
    createStatusParam("CmdInFifoHasData",     0x9,  1, 12); // CMD input FIFO has data      (0=empty,1=has data)
    createStatusParam("Ibc2Missing",          0xD,  1,  9); // Good: IBC2 missing           (0=no,1=yes)
    createStatusParam("Ibc1Missing",          0xD,  1,  8); // Good: IBC1 missing           (0=no,1=yes)
    createStatusParam("DffAlmostFull",        0x9,  1,  7); // DFF almost full              (0=no,1=yes)
    createStatusParam("DffHasData",           0x9,  1,  6); // DFF has data                 (0=empty,1=has data)
    createStatusParam("CtrlFifoAlmostFull",   0x9,  1,  3); // CTRL FIFO almost full        (0=no,1=yes)
    createStatusParam("CtrlFifoHasData",      0x9,  1,  2); // CTRL FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds9:FifoAlmostFull", 0x9,  1,  1); // Chan9 FIFO almost full       (0=no,1=yes)
    createStatusParam("Lvds9:FifoHasData",    0x9,  1,  0); // Chan9 FIFO has data          (0=empty,1=has data)

    createStatusParam("CntFilterRsp",         0xA, 16,  8); // Filtered responses
    createStatusParam("SysrstB",              0xA,  2,  6); // Sysrst_B low/high detected   (0=empty,1=low,2=high,3=low and high)
    createStatusParam("TxEnable",             0xA,  2,  4); // TXEN low/high detected       (0=empty,1=low,2=high,3=low and high)
    createStatusParam("Tsync",                0xA,  2,  2); // TSYNC                        (0=empty,1=low - not ok,2=high - not ok,3=low and high)
    createStatusParam("Tclk",                 0xA,  2,  0); // TCLK                         (0=empty,1=low - not ok,2=high - not ok,3=low and high)

    createStatusParam("CntHwId",              0xC,  9, 10); // Hardware ID count

    createStatusParam("Mlos4",                0xD,  2, 14); // MLOS1                        (0=empty,1=always low,2=always high,3=low and high)
    createStatusParam("Mlos3",                0xD,  2, 12); // MLOS2                        (0=empty,1=always low,2=always high,3=low and high)
    createStatusParam("Mlos2",                0xD,  2, 10); // MLOS3                        (0=empty,1=always low,2=always high,3=low and high)
    createStatusParam("Mlos1",                0xD,  2,  8); // MLOS4                        (0=empty,1=always low,2=always high,3=low and high)

    createStatusParam("Ibc5:TimeOverflow",    0xD,  1, 15); // Bad: IBC5 time overflow      (0=no,1=yes)
    createStatusParam("Ibc4:TimeOverflow",    0xD,  1, 14); // Bad: IBC4 time overflow      (0=no,1=yes)
    createStatusParam("Ibc3:TimeOverflow",    0xD,  1, 13); // Bad: IBC3 time overflow      (0=no,1=yes)
    createStatusParam("Ibc2:TimeOverflow",    0xD,  1, 12); // Bad: IBC2 time overflow      (0=no,1=yes)
    createStatusParam("Ibc1:TimeOverflow",    0xD,  1, 11); // Bad: IBC1 time overflow      (0=no,1=yes)
    createStatusParam("Ibc9:Missing",         0xD,  1, 10); // Good: IBC9 missing           (0=no,1=yes)
    createStatusParam("Ibc8:Missing",         0xD,  1,  9); // Good: IBC8 missing           (0=no,1=yes)
    createStatusParam("Ibc7:Missing",         0xD,  1,  8); // Good: IBC7 missing           (0=no,1=yes)
    createStatusParam("Ibc6:Missing",         0xD,  1,  7); // Good: IBC6 missing           (0=no,1=yes)
    createStatusParam("Ibc5:Missing",         0xD,  1,  6); // Good: IBC5 missing           (0=no,1=yes)
    createStatusParam("Ibc4:Missing",         0xD,  1,  5); // Good: IBC4 missing           (0=no,1=yes)
    createStatusParam("Ibc3:Missing",         0xD,  1,  4); // Good: IBC3 missing           (0=no,1=yes)
    createStatusParam("LvdsVerify",           0xD,  1,  3); // Detected LVDS verify         (0=no,1=yes)

    createStatusParam("EepromCode",           0xE, 16,  0); // EEPROM code

    createStatusParam("ErrDataHw",            0xF,  1, 15); // Data packet hardware error   (0=no,1=yes)
    createStatusParam("ErrMaskCh",            0xF,  1, 12); // Data packet changing mask    (0=no,1=yes)
    createStatusParam("ErrMaskCh1",           0xF,  1, 11); // Data packet mask change      (0=no,1=yes)
    createStatusParam("ErrNoIbc",             0xF,  1, 10); // Data packet missing IBCs     (0=no,1=yes)
    createStatusParam("ErrBadEvent",          0xF,  1,  9); // Data packet bad event        (0=no,1=yes)
    createStatusParam("ErrBadIbc",            0xF,  1,  8); // Data packet bad IBC          (0=no,1=yes)
    createStatusParam("ErrBadTime",           0xF,  1,  7); // Data packet bad timestamp    (0=no,1=yes)
    createStatusParam("ErrTimeout",           0xF,  1,  6); // Data packet timeout          (0=no,1=yes)
    createStatusParam("ErrBadData",           0xF,  6,  0); // Bad data packet              (0=no,1=yes)
}
