/* AcpcPlugin_v41.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"

/**
 * @file AcpcPlugin_v41.cpp
 *
 * ACPC 4.01 parameters
 *
 * ACPC 4.01 is Miljkos firmware that produces good X,Y position in normal mode.
 */

void AcpcPlugin::createParams_v144()
{
//    BLXXX:Det:RocXXX:| sig nam|                            | EPICS record description | (bi and mbbi description)
    createStatusParam("Ctrl:RxFlg",           0x0,  8,  0); // CTRL error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ctrl:GotDataPkt",      0x0,  1,  8); // CTRL got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ctrl:GotCmdPkt",       0x0,  1,  9); // CTRL got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ctrl:FifoHasData",     0x0,  1, 10); // CTRL FIFO has data           (0=empty,1=has data)
    createStatusParam("Ctrl:FifoAlmostFull",  0x0,  1, 11); // CTRL FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ctrl:FifoGotAlmFull",  0x0,  1, 12); // CTRL FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ctrl:FifoFull",        0x0,  1, 13); // CTRL FIFO full               (0=not full,1=full)
    createStatusParam("Ctrl:CntBadPackets",   0x1, 16,  0); // CTRL num bad packets

    createStatusParam("Lvds1:RxFlg",          0x2,  8,  0); // Chan1 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds1:GotDataPkt",     0x2,  1,  8); // Chan1 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds1:GotCmdPkt",      0x2,  1,  9); // Chan1 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds1:FifoHasData",    0x2,  1, 10); // Chan1 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds1:FifoAlmostFull", 0x2,  1, 11); // Chan1 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds1:FifoGotAlmFull", 0x2,  1, 12); // Chan1 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds1:FifoFull",       0x2,  1, 13); // Chan1 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds1:CntBadPackets",  0x3, 16,  0); // Chan1 num bad packets

    createStatusParam("Lvds2:RxFlg",          0x4,  8,  0); // Chan2 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds2:GotDataPkt",     0x4,  1,  8); // Chan2 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds2:GotCmdPkt",      0x4,  1,  9); // Chan2 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds2:FifoHasData",    0x4,  1, 10); // Chan2 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds2:FifoAlmostFull", 0x4,  1, 11); // Chan2 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds2:FifoGotAlmFull", 0x4,  1, 12); // Chan2 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds2:FifoFull",       0x4,  1, 13); // Chan2 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds2:CntBadPackets",  0x5, 16,  0); // Chan2 num bad packets

    createStatusParam("Lvds3:RxFlg",          0x6,  8,  0); // Chan3 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds3:GotDataPkt",     0x6,  1,  8); // Chan3 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds3:GotCmdPkt",      0x6,  1,  9); // Chan3 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds3:FifoHasData",    0x6,  1, 10); // Chan3 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds3:FifoAlmostFull", 0x6,  1, 11); // Chan3 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds3:FifoGotAlmFull", 0x6,  1, 12); // Chan3 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds3:FifoFull",       0x6,  1, 13); // Chan3 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds3:CntBadPackets",  0x7, 16,  0); // Chan3 num bad packets

    createStatusParam("Lvds4:RxFlg",          0x8,  8,  0); // Chan4 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds4:GotDataPkt",     0x8,  1,  8); // Chan4 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds4:GotCmdPkt",      0x8,  1,  9); // Chan4 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds4:FifoHasData",    0x8,  1, 10); // Chan4 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds4:FifoAlmostFull", 0x8,  1, 11); // Chan4 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds4:FifoGotAlmFull", 0x8,  1, 12); // Chan4 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds4:FifoFull",       0x8,  1, 13); // Chan4 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds4:CntBadPackets",  0x9, 16,  0); // Chan4 num bad packets

    createStatusParam("Lvds5:RxFlg",          0xA,  8,  0); // Chan5 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds5:GotDataPkt",     0xA,  1,  8); // Chan5 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds5:GotCmdPkt",      0xA,  1,  9); // Chan5 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds5:FifoHasData",    0xA,  1, 10); // Chan5 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds5:FifoAlmostFull", 0xA,  1, 11); // Chan5 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds5:FifoGotAlmFull", 0xA,  1, 12); // Chan5 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds5:FifoFull",       0xA,  1, 13); // Chan5 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds5:CntBadPackets",  0xB, 16,  0); // Chan5 num bad packets

    createStatusParam("Lvds6:RxFlg",          0xC,  8,  0); // Chan6 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds6:GotDataPkt",     0xC,  1,  8); // Chan6 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds6:GotCmdPkt",      0xC,  1,  9); // Chan6 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds6:FifoHasData",    0xC,  1, 10); // Chan6 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds6:FifoAlmostFull", 0xC,  1, 11); // Chan6 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds6:FifoGotAlmFull", 0xC,  1, 12); // Chan6 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds6:FifoFull",       0xC,  1, 13); // Chan6 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds6:CntBadPackets",  0xD, 16,  0); // Chan6 num bad packets

    createStatusParam("Lvds7:RxFlg",          0xE,  8,  0); // Chan7 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds7:GotDataPkt",     0xE,  1,  8); // Chan7 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds7:GotCmdPkt",      0xE,  1,  9); // Chan7 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds7:FifoHasData",    0xE,  1, 10); // Chan7 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds7:FifoAlmostFull", 0xE,  1, 11); // Chan7 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds7:FifoGotAlmFull", 0xE,  1, 12); // Chan7 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds7:FifoFull",       0xE,  1, 13); // Chan7 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds7:CntBadPackets",  0xF, 16,  0); // Chan7 num bad packets

    createStatusParam("Lvds8:RxFlg",         0x10,  8,  0); // Chan8 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds8:GotDataPkt",    0x10,  1,  8); // Chan8 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds8:GotCmdPkt",     0x10,  1,  9); // Chan8 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds8:FifoHasData",   0x10,  1, 10); // Chan8 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds8:FifoAlmostFull",0x10,  1, 11); // Chan8 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds8:FifoGotAlmFull",0x10,  1, 12); // Chan8 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds8:FifoFull",      0x10,  1, 13); // Chan8 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds8:CntBadPackets", 0x11, 16,  0); // Chan8 num bad packets

    createStatusParam("Lvds9:RxFlg",         0x12,  8,  0); // Chan9 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds9:GotDataPkt",    0x12,  1,  8); // Chan9 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Lvds9:GotCmdPkt",     0x12,  1,  9); // Chan9 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Lvds9:FifoHasData",   0x12,  1, 10); // Chan9 FIFO has data           (0=empty,1=has data)
    createStatusParam("Lvds9:FifoAlmostFull",0x12,  1, 11); // Chan9 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Lvds9:FifoGotAlmFull",0x12,  1, 12); // Chan9 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Lvds9:FifoFull",      0x12,  1, 13); // Chan9 FIFO full               (0=not full,1=full)
    createStatusParam("Lvds9:CntBadPackets", 0x13, 16,  0); // Chan9 num bad packets

    createStatusParam("EepromCode",          0x14, 32,  0); // EEPROM code

    createStatusParam("MatrixState",         0x16,  5,  0); // MATRIX state
    createStatusParam("LvmrCalcState",       0x16,  5,  5); // LVMR calc state
    createStatusParam("LvmrCtrlState",       0x16,  5, 10); // LVMR control state
    createStatusParam("DataHandlState",      0x16,  5, 15); // Data handler state

    createStatusParam("Ibc1:Err",            0x17,  1,  5); // IBC1 problem                  (0=no error,1=error)
    createStatusParam("Ibc2:Err",            0x17,  1,  6); // IBC2 problem                  (0=no error,1=error)
    createStatusParam("Ibc3:Err",            0x17,  1,  7); // IBC3 problem                  (0=no error,1=error)
    createStatusParam("Ibc4:Err",            0x17,  1,  8); // IBC4 problem                  (0=no error,1=error)
    createStatusParam("Ibc5:Err",            0x17,  1,  9); // IBC5 problem                  (0=no error,1=error)
    createStatusParam("Ibc6:Err",            0x17,  1, 10); // IBC6 problem                  (0=no error,1=error)
    createStatusParam("Ibc7:Err",            0x17,  1, 11); // IBC7 problem                  (0=no error,1=error)
    createStatusParam("Ibc8:Err",            0x17,  1, 12); // IBC8 problem                  (0=no error,1=error)
    createStatusParam("Ibc9:Err",            0x17,  1, 13); // IBC9 problem                  (0=no error,1=error)

    createStatusParam("Ibc1:Missing",        0x17,  1, 14); // IBC1 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc2:Missing",        0x17,  1, 15); // IBC2 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc3:Missing",        0x18,  1,  0); // IBC3 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc4:Missing",        0x18,  1,  1); // IBC4 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc5:Missing",        0x18,  1,  2); // IBC5 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc6:Missing",        0x18,  1,  3); // IBC6 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc7:Missing",        0x18,  1,  4); // IBC7 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc8:Missing",        0x18,  1,  5); // IBC8 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc9:Missing",        0x18,  1,  6); // IBC9 missing                  (0=not missing,1=missing)

    createStatusParam("Ibc1:TimeOverflow",   0x18,  1,  7); // IBC1 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc2:TimeOverflow",   0x18,  1,  8); // IBC2 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc3:TimeOverflow",   0x18,  1,  9); // IBC3 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc4:TimeOverflow",   0x18,  1, 10); // IBC4 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc5:TimeOverflow",   0x18,  1, 11); // IBC5 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc6:TimeOverflow",   0x18,  1, 12); // IBC6 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc7:TimeOverflow",   0x18,  1, 13); // IBC7 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc8:TimeOverflow",   0x18,  1, 14); // IBC8 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc9:TimeOverflow",   0x18,  1, 15); // IBC9 outside time region      (0=not overflow,1=overflow)

    createStatusParam("CntBadPacketErr",     0x19,  8,  0); // Bad Packet Errors
    createStatusParam("CntIbcErr",           0x19,  8,  8); // Bad IBC Errors

    createStatusParam("ErrIbcHwerr",         0x1A,  1,  0); // Bad IBC Cnt for HWERR         (0=no error,1=error)
    createStatusParam("DffAlmostFull",       0x1A,  1,  4); // DFF almost full               (0=has space,1=almost full)
    createStatusParam("DffHasData",          0x1A,  1,  5); // DFF has data                  (0=no data,1=has data)
    createStatusParam("Calc:HwErr",          0x1A,  1,  6); // CALC Hardware error           (0=no error,1=error)
    createStatusParam("Calc:BadDataPkt",     0x1A,  1,  7); // CALC Bad data packet          (0=no error,1=error)
    createStatusParam("Calc:Timeout",        0x1A,  1,  8); // CALC timeout                  (0=no timeout,1=timeout)
    createStatusParam("Calc:BadIbc",         0x1A,  1,  9); // CALC Bad IBC                  (0=no error,1=error)
    createStatusParam("Calc:BadEvent",       0x1A,  1, 10); // CALC Bad event                (0=no error,1=error)
    createStatusParam("Calc:MissIbcs",       0x1A,  1, 11); // CALC Missing IBCs             (0=no error,1=error)

    // TODO: missing registers 0x1B & 0x1C?

    // TODO: dcomserver starts at offset 30-1=0x1D and uses 4 16 bit registers for this section
    createStatusParam("ErrBadCmd",           0x1D,  1,  0); // Unknown command               (0=no error,1=error)
    createStatusParam("ErrPktLength",        0x1D,  1,  1); // Packet length error           (0=no error,1=error)
    createStatusParam("ErrProgramming",      0x1D,  1,  2); // Programming error             (0=no error,1=error)
    createStatusParam("Configured",          0x1D,  1,  4); // Configured                    (0=configured,1=not configured [alarm], archive:monitor)
    createStatusParam("ErrNoHwId",           0x1D,  1,  4); // Missing hardware id           (0=no error,1=error)
    createStatusParam("FilteredNack",        0x1D,  1,  5); // Filtered NACK                 (0=not action,1=filtered)
    createStatusParam("NewHwId",             0x1D,  1,  6); // Found new hardware id         (0=not found,1=found)
    createStatusParam("CntFilteredNacks",    0x1D,  8,  7); // Number of filtered NACKs
    createStatusParam("Acquiring",           0x1D,  1, 15); // Acquiring data                (0=not acquiring,1=acquiring, archive:monitor)

    createStatusParam("OutputModeRB",        0x1E,  2,  0); // Data mode                     (0=normal,1=raw,2=extended)
    createStatusParam("CntFilteredAcks",     0x1E,  8,  2); // Number of filtered ACKs
    createStatusParam("CmdIn:HasData",       0x1E,  1, 10); // CMD input FIFO has data       (0=no data,1=has data)
    createStatusParam("CmdIn:AlmostFull",    0x1E,  1, 11); // CMD input FIFO almost full    (0=has space,1=almost full)
    createStatusParam("Cmd:HasData",         0x1E,  1, 12); // CMD FIFO has data             (0=no data,1=has data)
    createStatusParam("Cmd:AlmostFull",      0x1E,  1, 13); // CMD FIFO almost full          (0=has space,1=almost full)
    createStatusParam("LvdsVerify",          0x1E,  1, 14); // LVDS VERIFY detected          (0=not detected,1=detected)

    createStatusParam("CntFilteredRsps",     0x1F, 16,  0); // Number of filtered responses

    createStatusParam("CntHwId",             0x20,  9,  0); // Number of hardware IDs

    // TODO: dcomserver starts at 33-1=0x20, but overlaps with previous section
    createStatusParam("TclkLow",             0x21,  1,  0); // TCLK Got LOW                  (0=no,1=yes)
    createStatusParam("TclkHigh",            0x21,  1,  1); // TCLK Got HIGH                 (0=no,1=yes)
    createStatusParam("TsyncLow",            0x21,  1,  2); // TSYNC Got LOW                 (0=no,1=yes)
    createStatusParam("TsyncHigh",           0x21,  1,  3); // TSYNC Got HIGH                (0=no,1=yes)
    createStatusParam("TxenBLow",            0x21,  1,  4); // TXEN_B Got LOW                (0=no,1=yes)
    createStatusParam("TxenBHigh",           0x21,  1,  5); // TXEN_B Got HIGH               (0=no,1=yes)
    createStatusParam("SysrstBLow",          0x21,  1,  6); // SYSRST_B Got LOW              (0=no,1=yes)
    createStatusParam("SysrstBHigh",         0x21,  1,  7); // SYSRST_B Got HIGH             (0=no,1=yes)
    createStatusParam("ErrNoStart",          0x21,  1,  8); // Sort: Data without START      (0=no error,1=error) - TODO: conflict, dcomserver maps two values to this bit (Sort: Almost Full)
    createStatusParam("ErrPreStart",         0x21,  1,  9); // LVDS start before stop bit    (0=no error,1=error)
    createStatusParam("ErrTimeout",          0x21,  1, 10); // LVDS packet timeout           (0=no timeout,1=timeout)
    createStatusParam("ErrLength",           0x21,  1, 11); // LVDS packet length error      (0=no error,1=error)
    createStatusParam("SortHasData",         0x21,  1, 12); // Sort data available           (0=no data,1=has data)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionIdx",        '1',  0x0, 32,  0,    0); // Chan1 position index

    createConfigParam("Lvds1:Mode",         'B',  0x0,  2,  0,    2); // Chan1 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds2:Mode",         'B',  0x0,  2,  2,    2); // Chan2 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds3:Mode",         'B',  0x0,  2,  4,    2); // Chan3 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds4:Mode",         'B',  0x0,  2,  6,    2); // Chan4 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds5:Mode",         'B',  0x0,  2,  8,    2); // Chan5 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds6:Mode",         'B',  0x0,  2, 10,    2); // Chan6 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds7:Mode",         'B',  0x0,  2, 12,    2); // Chan7 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds8:Mode",         'B',  0x0,  2, 14,    2); // Chan8 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)
    createConfigParam("Lvds9:Mode",         'B',  0x1,  2,  0,    2); // Chan9 mode                    (0=fall edge late,1=fall edge early,2=rise edge,3=rise edge late)

    createConfigParam("PromptPlsRej1",      'C',  0x0, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 1 - TODO: verify register width
    createConfigParam("PromptPlsRej2",      'C',  0x2, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 2 - TODO: verify register width
    createConfigParam("PromptPlsRej3",      'C',  0x4, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 3 - TODO: verify register width
    createConfigParam("PromptPlsRej4",      'C',  0x6, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 4 - TODO: verify register width
    createConfigParam("PromptPlsRej5",      'C',  0x8, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 5 - TODO: verify register width
    createConfigParam("PromptPlsRej6",      'C',  0xA, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 6 - TODO: verify register width
    createConfigParam("PromptPlsRej7",      'C',  0xC, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 7 - TODO: verify register width
    createConfigParam("PromptPlsRej8",      'C',  0xE, 32,  0, 0xFFFFFFFF); // Prompt pulse reject range 8 - TODO: verify register width

    createConfigParam("RelPosition1",       'D',  0x0, 32,  0,    0); // Relative position 1         - TODO: verify register width
    createConfigParam("RelPosition2",       'D',  0x2, 32,  0,    0); // Relative position 2         - TODO: verify register width
    createConfigParam("RelPosition3",       'D',  0x4, 32,  0,    0); // Relative position 3         - TODO: verify register width
    createConfigParam("RelPosition4",       'D',  0x6, 32,  0,    0); // Relative position 4         - TODO: verify register width
    createConfigParam("RelPosition5",       'D',  0x8, 32,  0,    0); // Relative position 5         - TODO: verify register width
    createConfigParam("RelPosition6",       'D',  0xA, 32,  0,    0); // Relative position 6         - TODO: verify register width
    createConfigParam("RelPosition7",       'D',  0xC, 32,  0,    0); // Relative position 7         - TODO: verify register width
    createConfigParam("RelPosition8",       'D',  0xE, 32,  0,    0); // Relative position 8         - TODO: verify register width
    createConfigParam("RelPosition9",       'D', 0x10, 32,  0,    0); // Relative position 9         - TODO: verify register width
    createConfigParam("RelPosition10",      'D', 0x12, 32,  0,    0); // Relative position 10        - TODO: verify register width
    createConfigParam("RelPosition11",      'D', 0x14, 32,  0,    0); // Relative position 11        - TODO: verify register width
    createConfigParam("RelPosition12",      'D', 0x16, 32,  0,    0); // Relative position 12        - TODO: verify register width
    createConfigParam("RelPosition13",      'D', 0x18, 32,  0,    0); // Relative position 13        - TODO: verify register width
    createConfigParam("RelPosition14",      'D', 0x1A, 32,  0,    0); // Relative position 14        - TODO: verify register width
    createConfigParam("RelPosition15",      'D', 0x1C, 32,  0,    0); // Relative position 15        - TODO: verify register width
    createConfigParam("RelPosition16",      'D', 0x1E, 32,  0,    0); // Relative position 32        - TODO: verify register width
    createConfigParam("RelPosition17",      'D', 0x20, 32,  0,    0); // Relative position 17        - TODO: verify register width
    createConfigParam("RelPosition18",      'D', 0x22, 32,  0,    0); // Relative position 18        - TODO: verify register width
    createConfigParam("RelPosition19",      'D', 0x24, 32,  0,    0); // Relative position 19        - TODO: verify register width
    createConfigParam("RelPosition20",      'D', 0x26, 32,  0,    0); // Relative position 20        - TODO: verify register width
    createConfigParam("RelPosition21",      'D', 0x28, 32,  0,    0); // Relative position 21        - TODO: verify register width
    createConfigParam("RelPosition22",      'D', 0x2A, 32,  0,    0); // Relative position 22        - TODO: verify register width
    createConfigParam("RelPosition23",      'D', 0x2C, 32,  0,    0); // Relative position 23        - TODO: verify register width
    createConfigParam("RelPosition24",      'D', 0x2E, 32,  0,    0); // Relative position 24        - TODO: verify register width

    createConfigParam("ScaleInit",          'E',  0x0, 32,  0,    0); // Scale init                  - TODO: verify register width
    createConfigParam("NumIter",            'E',  0x2,  5,  0,    7); // Max number of iters         - TODO: verify name & desc
    createConfigParam("IterInit",           'E',  0x2,  6,  5,    0); // Iter init                   - TODO: verify name & desc
    createConfigParam("IterUp",             'E',  0x2,  4, 11,    3); // Iter up                     - TODO: verify name & desc
    createConfigParam("IterDown",           'E',  0x2,  4, 15,    1); // Iter down                   - TODO: verify name & desc
    createConfigParam("InitScaleFactor",    'E',  0x3,  2,  3,    0); // Init scale factor           - TODO: verify name & desc
    createConfigParam("InitCalcPoints",     'E',  0x3,  2,  5,    3); // Init calc points            - TODO: verify name & desc
    createConfigParam("EdgeCalcPoints",     'E',  0x3,  2,  7,    2); // Edge calc points            - TODO: verify name & desc
    createConfigParam("BesEdgeCalcPoints",  'E',  0x3,  2,  9,    2); // Beside edge calc points     - TODO: verify name & desc
    createConfigParam("SkipCalcs",          'E',  0x3,  1, 11,    0); // Skip calculations           - TODO: verify name & desc
    createConfigParam("DeltaInvert",        'E',  0x3,  2, 12,    0); // Delta invert                - TODO: verify name & desc
    createConfigParam("GdsInvert",          'E',  0x3,  1, 14,    0); // GDS invert                  - TODO: verify name & desc
    createConfigParam("DebugAddr1",         'E',  0x4,  5,  0,    7); // Debug address 1
    createConfigParam("DebugAddr2",         'E',  0x4,  5,  5,    6); // Debug address 2             - TODO: verify offset
    createConfigParam("DebugAddr3",         'E',  0x4,  5, 10,   15); // Debug address 3             - TODO: verify offset
    createConfigParam("DebugAddr4",         'E',  0x4,  5, 15,   14); // Debug address 4             - TODO: verify offset
    createConfigParam("DebugAddr5",         'E',  0x5,  5,  4,    8); // Debug address 5             - TODO: verify offset
    createConfigParam("DebugAddr6",         'E',  0x5,  5,  9,    5); // Debug address 6             - TODO: verify offset
    createConfigParam("DebugMtxDis",        'E',  0x5,  1, 30,    0); // Disable MTX debug           (0=enabled,1=disabled)
    createConfigParam("DebugLvmrDis",       'E',  0x5,  1, 31,    0); // Disable LVMR debug          (0=enabled,1=disabled)
    createConfigParam("DebugMtrOutput",     'E',  0x6,  3,  0,    0); // MTR debug output
    createConfigParam("DebugLvmrOutput",    'E',  0x6,  2,  3,    0); // MTR debug output
    createConfigParam("DebugStep",          'E',  0x6,  2,  5,    0); // MTR debug output
    createConfigParam("DebugWrenDis",       'E',  0x6,  1,  7,    0); // MTR debug output            (0=enabled,1=disabled)
    createConfigParam("SkipCalcMain",       'E',  0x6,  1,  8,    0); // Skip main calc              (0=enabled,1=disabled)
    createConfigParam("InitPosShift",       'E',  0x7,  6,  0,   15); // Init position shift
    createConfigParam("DivideShiftCnt",     'E',  0x7,  6,  6,   24); // Divide shift count
    createConfigParam("OffsetCorrEn",       'E',  0x8,  1,  0,    1); // Offset correction enable    (0=disabled,1=enabled)
    createConfigParam("SendPhotosum",       'E',  0x8,  1,  1,    1); // Send photosum               (0=disabled,1=enabled)
    createConfigParam("Tbd",                'E',  0x8,  6,  2,   20); // TODO: One or many registers
    createConfigParam("LowerPhotoSum",      'E',  0x9, 28,  0, 512000); // Lower PhotoSum
    createConfigParam("UpperPhotoSum",      'E',  0xB, 28,  0, 2867200); // Upper PhotoSum
    createConfigParam("TsyncDelay",         'E',  0xD, 32,  0,    0); // TSYNC delay
    createConfigParam("TimeRange",          'E',  0xF, 32,  0, 3584); // Time range - TODO: not sure about name
    createConfigParam("LvdsSysrstCtrl",     'E', 0x10,  2,  0,    0); // LVDS T&C SYSRST# buffer ctrl (0=sysrst,1=sysrst,2=always 0,3=always 1)
    createConfigParam("LvdsTclkCtrl",       'E', 0x10,  2,  2,    0); // LVDS TX control T&C TCLK mod (0=TCLK,1=TCLK,2=always 0,3=always 1)
    createConfigParam("LvdsTsyncCtrl",      'E', 0x10,  2,  4,    0); // LVDS TSYNC_NORMAL control    (0=polarity,1=TSYNC WIDTH,2=always 0,3=always 1)
    createConfigParam("LvdsTxenCtrl",       'E', 0x10,  2,  6,    0); // LVDS T&C TXEN# control       (0=ChLnk parser,1=ChLnk parser,2=ChLnk RX,3=ChLnk inv RX)
    createConfigParam("LvdsTestEn",         'E', 0x10,  1, 10,    0); // LVDS test enable             (0=disable,1=enable)
    createConfigParam("PeakMinThreshold",   'E', 0x11, 16,  0,    1); // ACPC peak min threshold
    createConfigParam("PeakMaxThreshold",   'E', 0x12, 16,  0, 0xFFFF); // ACPC peak max threshold

    createConfigParam("LvdsResetMode",      'F',  0x0,  1,  0,    0); // Reset mode                   (0=soft,1=hard)
    createConfigParam("LvdsTclkMode",       'F',  0x0,  1,  1,    0); // Reset mode                   (0=external,1=internal)
    createConfigParam("LvdsTsyncMode",      'F',  0x0,  1,  2,    0); // Reset mode                   (0=external,1=internal)
    createConfigParam("LvdsTxenMode",       'F',  0x0,  1,  3,    1); // Reset mode                   (0=external,1=internal)
    createConfigParam("OutputMode",         'F',  0x0,  2,  4,    0); // Output mode                  (0=normal,1=off,2=raw,2=verbose)
    createConfigParam("XorderEn",           'F',  0x0,  1,  6,    1); // X order                      (0=disabled,1=enabled)
    createConfigParam("YorderEn",           'F',  0x0,  1,  7,    1); // Y order                      (0=disabled,1=enabled)
    createConfigParam("SortEn",             'F',  0x0,  1,  8,    0); // Enable sorting               (0=disabled,1=enabled)
    createConfigParam("AlignEn",            'F',  0x0,  1,  9,    0); // Enable aligning              (0=disabled,1=enabled)
    createConfigParam("RisingEdgeEn",       'F',  0x0,  1, 11,    0); // Rising edge reading          (0=disabled,1=enabled)
    createConfigParam("VetoDis",            'F',  0x0,  1, 12,    1); // Disable NORMAL error vetoing (0=enabled,1=disabled)
    createConfigParam("ConfirmStartStop",   'F',  0x0,  1, 13,    0); // Confirm START and STOP       (0=disabled,1=enabled)
    createConfigParam("VerboseRspEn",       'F',  0x0,  1, 15,    0); // Verbose CMD response         (0=disabled,1=enabled)
}
