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

void AcpcPlugin::createStatusParams_v144()
{
//    BLXXX:Det:RocXXX:| sig nam|                            | EPICS record description | (bi and mbbi description)
    createStatusParam("Ctrl:RxFlg",         0x0,  8,  0); // CTRL error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ctrl:GotDataPkt",    0x0,  1,  8); // CTRL got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ctrl:GotCmdPkt",     0x0,  1,  9); // CTRL got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ctrl:FifoHasData",   0x0,  1, 10); // CTRL FIFO has data           (0=empty,1=has data)
    createStatusParam("Ctrl:FifoAlmostFull",0x0,  1, 11); // CTRL FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ctrl:FifoGotAlmFull",0x0,  1, 12); // CTRL FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ctrl:FifoFull",      0x0,  1, 13); // CTRL FIFO full               (0=not full,1=full)
    createStatusParam("Ctrl:CntBadPackets", 0x1, 16,  0); // CTRL num bad packets

    createStatusParam("Ch1:RxFlg",          0x2,  8,  0); // Chan1 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch1:GotDataPkt",     0x2,  1,  8); // Chan1 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch1:GotCmdPkt",      0x2,  1,  9); // Chan1 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch1:FifoHasData",    0x2,  1, 10); // Chan1 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch1:FifoAlmostFull", 0x2,  1, 11); // Chan1 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch1:FifoGotAlmFull", 0x2,  1, 12); // Chan1 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch1:FifoFull",       0x2,  1, 13); // Chan1 FIFO full               (0=not full,1=full)
    createStatusParam("Ch1:CntBadPackets",  0x3, 16,  0); // Chan1 num bad packets

    createStatusParam("Ch2:RxFlg",          0x4,  8,  0); // Chan2 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch2:GotDataPkt",     0x4,  1,  8); // Chan2 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch2:GotCmdPkt",      0x4,  1,  9); // Chan2 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch2:FifoHasData",    0x4,  1, 10); // Chan2 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch2:FifoAlmostFull", 0x4,  1, 11); // Chan2 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch2:FifoGotAlmFull", 0x4,  1, 12); // Chan2 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch2:FifoFull",       0x4,  1, 13); // Chan2 FIFO full               (0=not full,1=full)
    createStatusParam("Ch2:CntBadPackets",  0x5, 16,  0); // Chan2 num bad packets

    createStatusParam("Ch3:RxFlg",          0x6,  8,  0); // Chan3 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch3:GotDataPkt",     0x6,  1,  8); // Chan3 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch3:GotCmdPkt",      0x6,  1,  9); // Chan3 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch3:FifoHasData",    0x6,  1, 10); // Chan3 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch3:FifoAlmostFull", 0x6,  1, 11); // Chan3 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch3:FifoGotAlmFull", 0x6,  1, 12); // Chan3 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch3:FifoFull",       0x6,  1, 13); // Chan3 FIFO full               (0=not full,1=full)
    createStatusParam("Ch3:CntBadPackets",  0x7, 16,  0); // Chan3 num bad packets

    createStatusParam("Ch4:RxFlg",          0x8,  8,  0); // Chan4 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch4:GotDataPkt",     0x8,  1,  8); // Chan4 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch4:GotCmdPkt",      0x8,  1,  9); // Chan4 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch4:FifoHasData",    0x8,  1, 10); // Chan4 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch4:FifoAlmostFull", 0x8,  1, 11); // Chan4 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch4:FifoGotAlmFull", 0x8,  1, 12); // Chan4 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch4:FifoFull",       0x8,  1, 13); // Chan4 FIFO full               (0=not full,1=full)
    createStatusParam("Ch4:CntBadPackets",  0x9, 16,  0); // Chan4 num bad packets

    createStatusParam("Ch5:RxFlg",          0xA,  8,  0); // Chan5 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch5:GotDataPkt",     0xA,  1,  8); // Chan5 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch5:GotCmdPkt",      0xA,  1,  9); // Chan5 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch5:FifoHasData",    0xA,  1, 10); // Chan5 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch5:FifoAlmostFull", 0xA,  1, 11); // Chan5 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch5:FifoGotAlmFull", 0xA,  1, 12); // Chan5 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch5:FifoFull",       0xA,  1, 13); // Chan5 FIFO full               (0=not full,1=full)
    createStatusParam("Ch5:CntBadPackets",  0xB, 16,  0); // Chan5 num bad packets

    createStatusParam("Ch6:RxFlg",          0xC,  8,  0); // Chan6 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch6:GotDataPkt",     0xC,  1,  8); // Chan6 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch6:GotCmdPkt",      0xC,  1,  9); // Chan6 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch6:FifoHasData",    0xC,  1, 10); // Chan6 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch6:FifoAlmostFull", 0xC,  1, 11); // Chan6 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch6:FifoGotAlmFull", 0xC,  1, 12); // Chan6 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch6:FifoFull",       0xC,  1, 13); // Chan6 FIFO full               (0=not full,1=full)
    createStatusParam("Ch6:CntBadPackets",  0xD, 16,  0); // Chan6 num bad packets

    createStatusParam("Ch7:RxFlg",          0xE,  8,  0); // Chan7 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch7:GotDataPkt",     0xE,  1,  8); // Chan7 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch7:GotCmdPkt",      0xE,  1,  9); // Chan7 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch7:FifoHasData",    0xE,  1, 10); // Chan7 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch7:FifoAlmostFull", 0xE,  1, 11); // Chan7 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch7:FifoGotAlmFull", 0xE,  1, 12); // Chan7 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch7:FifoFull",       0xE,  1, 13); // Chan7 FIFO full               (0=not full,1=full)
    createStatusParam("Ch7:CntBadPackets",  0xF, 16,  0); // Chan7 num bad packets

    createStatusParam("Ch8:RxFlg",         0x10,  8,  0); // Chan8 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch8:GotDataPkt",    0x10,  1,  8); // Chan8 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch8:GotCmdPkt",     0x10,  1,  9); // Chan8 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch8:FifoHasData",   0x10,  1, 10); // Chan8 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch8:FifoAlmostFull",0x10,  1, 11); // Chan8 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch8:FifoGotAlmFull",0x10,  1, 12); // Chan8 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch8:FifoFull",      0x10,  1, 13); // Chan8 FIFO full               (0=not full,1=full)
    createStatusParam("Ch8:CntBadPackets", 0x11, 16,  0); // Chan8 num bad packets

    createStatusParam("Ch9:RxFlg",         0x12,  8,  0); // Chan9 error flags             (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch9:GotDataPkt",    0x12,  1,  8); // Chan9 got data packet         (0=no data pkt,1=got data pkt)
    createStatusParam("Ch9:GotCmdPkt",     0x12,  1,  9); // Chan9 got cmd packet          (0=no cmd pkt,1=got cmd pkt)
    createStatusParam("Ch9:FifoHasData",   0x12,  1, 10); // Chan9 FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch9:FifoAlmostFull",0x12,  1, 11); // Chan9 FIFO almost full        (0=not full,1=almost full)
    createStatusParam("Ch9:FifoGotAlmFull",0x12,  1, 12); // Chan9 FIFO got almost full    (0=not full,1=almost full)
    createStatusParam("Ch9:FifoFull",      0x12,  1, 13); // Chan9 FIFO full               (0=not full,1=full)
    createStatusParam("Ch9:CntBadPackets", 0x13, 16,  0); // Chan9 num bad packets

    createStatusParam("EepromCode",        0x14, 32,  0); // EEPROM code

    createStatusParam("MatrixState",       0x16,  5,  0); // MATRIX state
    createStatusParam("LvmrCalcState",     0x16,  5,  5); // LVMR calc state
    createStatusParam("LvmrCtrlState",     0x16,  5, 10); // LVMR control state
    createStatusParam("DataHandlState",    0x16,  5, 15); // Data handler state

    createStatusParam("Ibc1:Err",          0x17,  1,  5); // IBC1 problem                  (0=no error,1=error)
    createStatusParam("Ibc2:Err",          0x17,  1,  6); // IBC2 problem                  (0=no error,1=error)
    createStatusParam("Ibc3:Err",          0x17,  1,  7); // IBC3 problem                  (0=no error,1=error)
    createStatusParam("Ibc4:Err",          0x17,  1,  8); // IBC4 problem                  (0=no error,1=error)
    createStatusParam("Ibc5:Err",          0x17,  1,  9); // IBC5 problem                  (0=no error,1=error)
    createStatusParam("Ibc6:Err",          0x17,  1, 10); // IBC6 problem                  (0=no error,1=error)
    createStatusParam("Ibc7:Err",          0x17,  1, 11); // IBC7 problem                  (0=no error,1=error)
    createStatusParam("Ibc8:Err",          0x17,  1, 12); // IBC8 problem                  (0=no error,1=error)
    createStatusParam("Ibc9:Err",          0x17,  1, 13); // IBC9 problem                  (0=no error,1=error)

    createStatusParam("Ibc1:Missing",      0x17,  1, 14); // IBC1 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc2:Missing",      0x17,  1, 15); // IBC2 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc3:Missing",      0x18,  1,  0); // IBC3 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc4:Missing",      0x18,  1,  1); // IBC4 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc5:Missing",      0x18,  1,  2); // IBC5 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc6:Missing",      0x18,  1,  3); // IBC6 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc7:Missing",      0x18,  1,  4); // IBC7 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc8:Missing",      0x18,  1,  5); // IBC8 missing                  (0=not missing,1=missing)
    createStatusParam("Ibc9:Missing",      0x18,  1,  6); // IBC9 missing                  (0=not missing,1=missing)

    createStatusParam("Ibc1:TimeOverflow", 0x18,  1,  7); // IBC1 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc2:TimeOverflow", 0x18,  1,  8); // IBC2 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc3:TimeOverflow", 0x18,  1,  9); // IBC3 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc4:TimeOverflow", 0x18,  1, 10); // IBC4 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc5:TimeOverflow", 0x18,  1, 11); // IBC5 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc6:TimeOverflow", 0x18,  1, 12); // IBC6 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc7:TimeOverflow", 0x18,  1, 13); // IBC7 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc8:TimeOverflow", 0x18,  1, 14); // IBC8 outside time region      (0=not overflow,1=overflow)
    createStatusParam("Ibc9:TimeOverflow", 0x18,  1, 15); // IBC9 outside time region      (0=not overflow,1=overflow)

    createStatusParam("CntBadPacketErr",   0x19,  8,  0); // Bad Packet Errors
    createStatusParam("CntIbcErr",         0x19,  8,  8); // Bad IBC Errors

    createStatusParam("ErrIbcHwerr",       0x1A,  1,  0); // Bad IBC Cnt for HWERR         (0=no error,1=error)
    createStatusParam("DffAlmostFull",     0x1A,  1,  4); // DFF almost full               (0=has space,1=almost full)
    createStatusParam("DffHasData",        0x1A,  1,  5); // DFF has data                  (0=no data,1=has data)
    createStatusParam("Calc:HwErr",        0x1A,  1,  6); // CALC Hardware error           (0=no error,1=error)
    createStatusParam("Calc:BadDataPkt",   0x1A,  1,  7); // CALC Bad data packet          (0=no error,1=error)
    createStatusParam("Calc:Timeout",      0x1A,  1,  8); // CALC timeout                  (0=no timeout,1=timeout)
    createStatusParam("Calc:BadIbc",       0x1A,  1,  9); // CALC Bad IBC                  (0=no error,1=error)
    createStatusParam("Calc:BadEvent",     0x1A,  1, 10); // CALC Bad event                (0=no error,1=error)
    createStatusParam("Calc:MissIbcs",     0x1A,  1, 11); // CALC Missing IBCs             (0=no error,1=error)

    // TODO: missing registers 0x1B & 0x1C?

    // TODO: dcomserver starts at offset 30-1=0x1D and uses 4 16 bit registers for this section
    createStatusParam("ErrBadCmd",         0x1D,  1,  0); // Unknown command               (0=no error,1=error)
    createStatusParam("ErrPktLength",      0x1D,  1,  1); // Packet length error           (0=no error,1=error)
    createStatusParam("ErrProgramming",    0x1D,  1,  2); // Programming error             (0=no error,1=error)
    createStatusParam("Configured",        0x1D,  1,  4); // Configured                    (0=configured,1=not configured [alarm], archive:monitor)
    createStatusParam("ErrNoHwId",         0x1D,  1,  4); // Missing hardware id           (0=no error,1=error)
    createStatusParam("FilteredNack",      0x1D,  1,  5); // Filtered NACK                 (0=not action,1=filtered)
    createStatusParam("NewHwId",           0x1D,  1,  6); // Found new hardware id         (0=not found,1=found)
    createStatusParam("CntFilteredNacks",  0x1D,  8,  7); // Number of filtered NACKs
    createStatusParam("Acquiring",         0x1D,  1, 15); // Acquiring data                (0=not acquiring [alarm],1=acquiring, archive:monitor)

    createStatusParam("OutputModeRB",      0x1E,  2,  0); // Data mode                     (0=normal,1=raw,2=extended)
    createStatusParam("CntFilteredAcks",   0x1E,  8,  2); // Number of filtered ACKs
    createStatusParam("CmdIn:HasData",     0x1E,  1, 10); // CMD input FIFO has data       (0=no data,1=has data)
    createStatusParam("CmdIn:AlmostFull",  0x1E,  1, 11); // CMD input FIFO almost full    (0=has space,1=almost full)
    createStatusParam("Cmd:HasData",       0x1E,  1, 12); // CMD FIFO has data             (0=no data,1=has data)
    createStatusParam("Cmd:AlmostFull",    0x1E,  1, 13); // CMD FIFO almost full          (0=has space,1=almost full)
    createStatusParam("LvdsVerify",        0x1E,  1, 14); // LVDS VERIFY detected          (0=not detected,1=detected)

    createStatusParam("CntFilteredRsps",   0x1F, 16,  0); // Number of filtered responses

    createStatusParam("CntHwId",           0x20,  9,  0); // Number of hardware IDs

    // TODO: dcomserver starts at 33-1=0x20, but overlaps with previous section
    createStatusParam("TclkLow",           0x21,  1,  0); // TCLK Got LOW                  (0=no,1=yes)
    createStatusParam("TclkHigh",          0x21,  1,  1); // TCLK Got HIGH                 (0=no,1=yes)
    createStatusParam("TsyncLow",          0x21,  1,  2); // TSYNC Got LOW                 (0=no,1=yes)
    createStatusParam("TsyncHigh",         0x21,  1,  3); // TSYNC Got HIGH                (0=no,1=yes)
    createStatusParam("TxenBLow",          0x21,  1,  4); // TXEN_B Got LOW                (0=no,1=yes)
    createStatusParam("TxenBHigh",         0x21,  1,  5); // TXEN_B Got HIGH               (0=no,1=yes)
    createStatusParam("SysrstBLow",        0x21,  1,  6); // SYSRST_B Got LOW              (0=no,1=yes)
    createStatusParam("SysrstBHigh",       0x21,  1,  7); // SYSRST_B Got HIGH             (0=no,1=yes)
    createStatusParam("ErrNoStart",        0x21,  1,  8); // Sort: Data without START      (0=no error,1=error) - TODO: conflict, dcomserver maps two values to this bit (Sort: Almost Full)
    createStatusParam("ErrPreStart",       0x21,  1,  9); // LVDS start before stop bit    (0=no error,1=error)
    createStatusParam("ErrTimeout",        0x21,  1, 10); // LVDS packet timeout           (0=no timeout,1=timeout)
    createStatusParam("ErrLength",         0x21,  1, 11); // LVDS packet length error      (0=no error,1=error)
    createStatusParam("SortHasData",       0x21,  1, 12); // Sort data available           (0=no data,1=has data)
}

void AcpcPlugin::createConfigParams_v144()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionIdx",      '1',  0x0, 32,  0,    0); // Chan1 position index

    createConfigParam("Unused_B_0",       'B',  0x0, 16,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_B_1",       'B',  0x1, 16,  0,    0); // UNUSED BY FIRMWARE

    createConfigParam("Unused_C_0",       'C',  0x0, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_2",       'C',  0x2, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_4",       'C',  0x4, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_6",       'C',  0x6, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_8",       'C',  0x8, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_A",       'C',  0xA, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_C",       'C',  0xC, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_C_E",       'C',  0xE, 32,  0,    0); // UNUSED BY FIRMWARE

    createConfigParam("Unused_D_0",       'D',  0x0, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_2",       'D',  0x2, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_4",       'D',  0x4, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_6",       'D',  0x6, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_8",       'D',  0x8, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_A",       'D',  0xA, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_C",       'D',  0xC, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_E",       'D',  0xE, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_10",      'D', 0x10, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_12",      'D', 0x12, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_14",      'D', 0x14, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_16",      'D', 0x16, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_18",      'D', 0x18, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_1A",      'D', 0x1A, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_1C",      'D', 0x1C, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_1E",      'D', 0x1E, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_20",      'D', 0x20, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_22",      'D', 0x22, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_24",      'D', 0x24, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_26",      'D', 0x26, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_28",      'D', 0x28, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_2A",      'D', 0x2A, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_2C",      'D', 0x2C, 32,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_D_2E",      'D', 0x2E, 32,  0,    0); // UNUSED BY FIRMWARE

    createConfigParam("Unused_E_0",       'E',  0x0, 32,  0,    0); // UNUSED BY FIRMWARE

    createConfigParam("TestModeRate",     'E',  0x2,  2, 11,    0); // Test packet rate             (0=2400 c/s, 1=9600 c/s, 2=13000 c/s, 3=59000 c/s)
    createConfigParam("TestModeCtrl",     'E',  0x2,  1, 14,    0); // Test mode                    (0=position calc, 1=test pattern)
    createConfigParam("TestModeEn",       'E',  0x2,  1, 15,    0); // Test mode enable             (0=disable,1=enable)

    createConfigParam("Unused_E_3",       'E',  0x3, 16,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_E_4",       'E',  0x4, 16,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_E_5",       'E',  0x5, 16,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_E_6",       'E',  0x6, 16,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_E_7",       'E',  0x7, 16,  0,    0); // UNUSED BY FIRMWARE
    createConfigParam("Unused_E_8",       'E',  0x8, 16,  0,    0); // UNUSED BY FIRMWARE

    createConfigParam("LowerPhotoSum",    'E',  0x9, 32,  0, 512000); // Lower PhotoSum
    createConfigParam("UpperPhotoSum",    'E',  0xB, 32,  0, 2867200); // Upper PhotoSum

    createConfigParam("TsyncDelay",       'E',  0xD, 32,  0,    0); // TSYNC delay

    createConfigParam("Ch1:Disable",      'E',  0xF,  1,  0,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch2:Disable",      'E',  0xF,  1,  1,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch3:Disable",      'E',  0xF,  1,  2,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch4:Disable",      'E',  0xF,  1,  3,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch5:Disable",      'E',  0xF,  1,  4,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch6:Disable",      'E',  0xF,  1,  5,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch7:Disable",      'E',  0xF,  1,  6,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch8:Disable",      'E',  0xF,  1,  7,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("Ch9:Disable",      'E',  0xF,  1,  8,    0); // Chan 1 disable               (0=enable, 1=disable)
    createConfigParam("TimeTolerance",    'E',  0xF,  7,  9,    0); // Time tolerance

    createConfigParam("SysrstBCtrl",      'E', 0x10,  2,  0,    0); // SYSRST_B Control             (0=control by T&C, 1=control init, 2=all zero, 3=all one)
    createConfigParam("TclkCtrl",         'E', 0x10,  2,  2,    0); // TCLK Control                 (0=control by T&C, 1=control init, 2=all zero, 3=all one)
    createConfigParam("TsyncCtrl",        'E', 0x10,  2,  4,    0); // TSYNC Control                (0=control by T&C, 1=control init, 2=all zero, 3=all one)
    createConfigParam("TxenCtrl",         'E', 0x10,  2,  6,    0); // TXEN Control                 (0=not used, 1=control init, 2=all zero, 3=all one)
    createConfigParam("LvdsTestEn",       'E', 0x10,  1,  8,    0); // LVDS test enable             (0=disable, 1=enable)

    createConfigParam("OutputMode",       'F',  0x0,  2,  4,    0); // Output mode                  (0=normal, 1=off, 2=raw, 3=verbose)

    createConfigParam("TestPatternEn",    'F',  0x1,  1, 15,    0); // Test pattern enable          (0=disable,1=enable)
    createConfigParam("TestPatternId",    'F',  0x1, 12,  0,    0); // Test pattern id
    createConfigParam("TestPatternRate",  'F',  0x2, 16,  0, 1999); // Test pattern rate            (65535=153 ev/s,9999=1 Kev/s,4999=2 Kev/s,1999=5 Kev/s,999=10 Kev/s,399=25 Kev/s,199=50 Kev/s,99=100 Kev/s,13=800 Kev/s,9=1 Mev/s,4=2 Mev/s,1=5 Mev/s,0=10 Mev/s)
}
