/* FemPlugin_v32.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FemPlugin.h"

void FemPlugin::createStatusParams_v35()
{
//     BLXXX:Det:FemXX:| sig name        |                 | EPICS record description | (bi and mbbi description)
    createStatusParam("Ch1:ErrParity",    0x0,  1,  0); // Chan1 parity error           (0=no,1=yes)
    createStatusParam("Ch1:ErrType",      0x0,  1,  1); // Chan1 packet type error      (0=no,1=yes)
    createStatusParam("Ch1:ErrShortPkt",  0x0,  1,  2); // Chan1 short packet error     (0=no,1=yes)
    createStatusParam("Ch1:ErrLength",    0x0,  1,  3); // Chan1 long packet error      (0=no,1=yes)
    createStatusParam("Ch1:ErrTimeout",   0x0,  1,  4); // Chan1 timeout                (0=no,1=yes)
    createStatusParam("Ch1:ErrNoStart",   0x0,  1,  5); // Chan1 no START               (0=no,1=yes)
    createStatusParam("Ch1:ErrPreStart",  0x0,  1,  6); // Chan1 START before STOP      (0=no,1=yes)
    createStatusParam("Ch1:ErrFifoFull",  0x0,  1,  7); // Chan1 FIFO full              (0=no,1=yes)
    createStatusParam("Ch2:ErrParity",    0x0,  1,  8); // Chan2 parity error           (0=no,1=yes)
    createStatusParam("Ch2:ErrType",      0x0,  1,  9); // Chan2 packet type error      (0=no,1=yes)
    createStatusParam("Ch2:ErrShortPkt",  0x0,  1, 10); // Chan2 short packet error     (0=no,1=yes)
    createStatusParam("Ch2:ErrLength",    0x0,  1, 11); // Chan2 long packet error      (0=no,1=yes)
    createStatusParam("Ch2:ErrTimeout",   0x0,  1, 12); // Chan2 timeout                (0=no,1=yes)
    createStatusParam("Ch2:ErrNoStart",   0x0,  1, 13); // Chan2 no START               (0=no,1=yes)
    createStatusParam("Ch2:ErrPreStart",  0x0,  1, 14); // Chan2 START before STOP      (0=no,1=yes)
    createStatusParam("Ch2:ErrFifoFull",  0x0,  1, 15); // Chan2 FIFO full              (0=no,1=yes)

    createStatusParam("Ch3:ErrParity",    0x1,  1,  0); // Chan3 parity error           (0=no,1=yes)
    createStatusParam("Ch3:ErrType",      0x1,  1,  1); // Chan3 packet type error      (0=no,1=yes)
    createStatusParam("Ch3:ErrShortPkt",  0x1,  1,  2); // Chan3 short packet error     (0=no,1=yes)
    createStatusParam("Ch3:ErrLength",    0x1,  1,  3); // Chan3 long packet error      (0=no,1=yes)
    createStatusParam("Ch3:ErrTimeout",   0x1,  1,  4); // Chan3 timeout                (0=no,1=yes)
    createStatusParam("Ch3:ErrNoStart",   0x1,  1,  5); // Chan3 no START               (0=no,1=yes)
    createStatusParam("Ch3:ErrPreStart",  0x1,  1,  6); // Chan3 START before STOP      (0=no,1=yes)
    createStatusParam("Ch3:ErrFifoFull",  0x1,  1,  7); // Chan3 FIFO full              (0=no,1=yes)
    createStatusParam("Ch4:ErrParity",    0x1,  1,  8); // Chan4 parity error           (0=no,1=yes)
    createStatusParam("Ch4:ErrType",      0x1,  1,  9); // Chan4 packet type error      (0=no,1=yes)
    createStatusParam("Ch4:ErrShortPkt",  0x1,  1, 10); // Chan4 short packet error     (0=no,1=yes)
    createStatusParam("Ch4:ErrLength",    0x1,  1, 11); // Chan4 long packet error      (0=no,1=yes)
    createStatusParam("Ch4:ErrTimeout",   0x1,  1, 12); // Chan4 timeout                (0=no,1=yes)
    createStatusParam("Ch4:ErrNoStart",   0x1,  1, 13); // Chan4 no START               (0=no,1=yes)
    createStatusParam("Ch4:ErrPreStart",  0x1,  1, 14); // Chan4 START before STOP      (0=no,1=yes)
    createStatusParam("Ch4:ErrFifoFull",  0x1,  1, 15); // Chan4 FIFO full              (0=no,1=yes)

    createStatusParam("Ch5:ErrParity",    0x2,  1,  0); // Chan5 parity error           (0=no,1=yes)
    createStatusParam("Ch5:ErrType",      0x2,  1,  1); // Chan5 packet type error      (0=no,1=yes)
    createStatusParam("Ch5:ErrShortPkt",  0x2,  1,  2); // Chan5 short packet error     (0=no,1=yes)
    createStatusParam("Ch5:ErrLength",    0x2,  1,  3); // Chan5 long packet error      (0=no,1=yes)
    createStatusParam("Ch5:ErrTimeout",   0x2,  1,  4); // Chan5 timeout                (0=no,1=yes)
    createStatusParam("Ch5:ErrNoStart",   0x2,  1,  5); // Chan5 no START               (0=no,1=yes)
    createStatusParam("Ch5:ErrPreStart",  0x2,  1,  6); // Chan5 START before STOP      (0=no,1=yes)
    createStatusParam("Ch5:ErrFifoFull",  0x2,  1,  7); // Chan5 FIFO full              (0=no,1=yes)
    createStatusParam("Ch6:ErrParity",    0x2,  1,  8); // Chan6 parity error           (0=no,1=yes)
    createStatusParam("Ch6:ErrType",      0x2,  1,  9); // Chan6 packet type error      (0=no,1=yes)
    createStatusParam("Ch6:ErrShortPkt",  0x2,  1, 10); // Chan6 short packet error     (0=no,1=yes)
    createStatusParam("Ch6:ErrLength",    0x2,  1, 11); // Chan6 long packet error      (0=no,1=yes)
    createStatusParam("Ch6:ErrTimeout",   0x2,  1, 12); // Chan6 timeout                (0=no,1=yes)
    createStatusParam("Ch6:ErrNoStart",   0x2,  1, 13); // Chan6 no START               (0=no,1=yes)
    createStatusParam("Ch6:ErrPreStart",  0x2,  1, 14); // Chan6 START before STOP      (0=no,1=yes)
    createStatusParam("Ch6:ErrFifoFull",  0x2,  1, 15); // Chan6 FIFO full              (0=no,1=yes)

    createStatusParam("Ch7:ErrParity",    0x3,  1,  0); // Chan7 parity error           (0=no,1=yes)
    createStatusParam("Ch7:ErrType",      0x3,  1,  1); // Chan7 packet type error      (0=no,1=yes)
    createStatusParam("Ch7:ErrShortPkt",  0x3,  1,  2); // Chan7 short packet error     (0=no,1=yes)
    createStatusParam("Ch7:ErrLength",    0x3,  1,  3); // Chan7 long packet error      (0=no,1=yes)
    createStatusParam("Ch7:ErrTimeout",   0x3,  1,  4); // Chan7 timeout                (0=no,1=yes)
    createStatusParam("Ch7:ErrNoStart",   0x3,  1,  5); // Chan7 no START               (0=no,1=yes)
    createStatusParam("Ch7:ErrPreStart",  0x3,  1,  6); // Chan7 START before STOP      (0=no,1=yes)
    createStatusParam("Ch7:ErrFifoFull",  0x3,  1,  7); // Chan7 FIFO full              (0=no,1=yes)
    createStatusParam("Ch8:ErrParity",    0x3,  1,  8); // Chan8 parity error           (0=no,1=yes)
    createStatusParam("Ch8:ErrType",      0x3,  1,  9); // Chan8 packet type error      (0=no,1=yes)
    createStatusParam("Ch8:ErrShortPkt",  0x3,  1, 10); // Chan8 short packet error     (0=no,1=yes)
    createStatusParam("Ch8:ErrLength",    0x3,  1, 11); // Chan8 long packet error      (0=no,1=yes)
    createStatusParam("Ch8:ErrTimeout",   0x3,  1, 12); // Chan8 timeout                (0=no,1=yes)
    createStatusParam("Ch8:ErrNoStart",   0x3,  1, 13); // Chan8 no START               (0=no,1=yes)
    createStatusParam("Ch8:ErrPreStart",  0x3,  1, 14); // Chan8 START before STOP      (0=no,1=yes)
    createStatusParam("Ch8:ErrFifoFull",  0x3,  1, 15); // Chan8 FIFO full              (0=no,1=yes)

    createStatusParam("Ch9:ErrParity",    0x4,  1,  0); // Chan9 parity error           (0=no,1=yes)
    createStatusParam("Ch9:ErrType",      0x4,  1,  1); // Chan9 packet type error      (0=no,1=yes)
    createStatusParam("Ch9:ErrShortPkt",  0x4,  1,  2); // Chan9 short packet error     (0=no,1=yes)
    createStatusParam("Ch9:ErrLength",    0x4,  1,  3); // Chan9 long packet error      (0=no,1=yes)
    createStatusParam("Ch9:ErrTimeout",   0x4,  1,  4); // Chan9 timeout                (0=no,1=yes)
    createStatusParam("Ch9:ErrNoStart",   0x4,  1,  5); // Chan9 no START               (0=no,1=yes)
    createStatusParam("Ch9:ErrPreStart",  0x4,  1,  6); // Chan9 START before STOP      (0=no,1=yes)
    createStatusParam("Ch9:ErrFifoFull",  0x4,  1,  7); // Chan9 FIFO full              (0=no,1=yes)

    createStatusParam("Ctrl:ErrParity",   0x5,  1,  0); // CTRL parity error            (0=no,1=yes)
    createStatusParam("Ctrl:ErrType",     0x5,  1,  1); // CTRL packet type error       (0=no,1=yes)
    createStatusParam("Ctrl:ErrShortPkt", 0x5,  1,  2); // CTRL short packet error      (0=no,1=yes)
    createStatusParam("Ctrl:ErrLength",   0x5,  1,  3); // CTRL long packet error       (0=no,1=yes)
    createStatusParam("Ctrl:ErrTimeout",  0x5,  1,  4); // CTRL timeout                 (0=no,1=yes)
    createStatusParam("Ctrl:ErrNoStart",  0x5,  1,  5); // CTRL no START                (0=no,1=yes)
    createStatusParam("Ctrl:ErrPreStart", 0x5,  1,  6); // CTRL START before STOP       (0=no,1=yes)
    createStatusParam("Ctrl:ErrFifoFull", 0x5,  1,  7); // CTRL FIFO full               (0=no,1=yes)
    createStatusParam("ErrBadCmd",        0x5,  1,  8); // Unknown command              (0=no error,1=error)
    createStatusParam("ErrPacketLength",  0x5,  1,  9); // Packet length error          (0=no error,1=error)
    createStatusParam("ErrProgramming",   0x5,  1, 10); // Programming error            (0=no error,1=error)
    createStatusParam("ErrConfigured",    0x5,  1, 11); // Unconfigured                 (0=configured,1=not configured)
    createStatusParam("ErrNoHwId",        0x5,  1, 12); // Missing hardware             (0=not missing,1=missing)
    createStatusParam("ErrEeprom",        0x5,  1, 13); // EEPROM error                 (0=no error,1=error)
    createStatusParam("ErrStatusFilter",  0x5,  1, 14); // Status Filter Error          (0=no,1=yes)

    createStatusParam("CntFilteredNacks", 0x6,  8,  0); // Number of filtered NACKS

    createStatusParam("Ch1:GotData",      0x7,  1,  0); // Chan1 got data packet        (0=no,1=yes)
    createStatusParam("Ch1:GotCmd",       0x7,  1,  1); // Chan1 got command packet     (0=no,1=yes)
    createStatusParam("Ch2:GotData",      0x7,  1,  2); // Chan2 got data packet        (0=no,1=yes)
    createStatusParam("Ch2:GotCmd",       0x7,  1,  3); // Chan2 got command packet     (0=no,1=yes)
    createStatusParam("Ch3:GotData",      0x7,  1,  4); // Chan3 got data packet        (0=no,1=yes)
    createStatusParam("Ch3:GotCmd",       0x7,  1,  5); // Chan3 got command packet     (0=no,1=yes)
    createStatusParam("Ch4:GotData",      0x7,  1,  6); // Chan4 got data packet        (0=no,1=yes)
    createStatusParam("Ch4:GotCmd",       0x7,  1,  7); // Chan4 got command packet     (0=no,1=yes)
    createStatusParam("Ch5:GotData",      0x7,  1,  8); // Chan5 got data packet        (0=no,1=yes)
    createStatusParam("Ch5:GotCmd",       0x7,  1,  9); // Chan5 got command packet     (0=no,1=yes)
    createStatusParam("Ch6:GotData",      0x7,  1, 10); // Chan6 got data packet        (0=no,1=yes)
    createStatusParam("Ch6:GotCmd",       0x7,  1, 11); // Chan6 got command packet     (0=no,1=yes)
    createStatusParam("Ch7:GotData",      0x7,  1, 12); // Chan7 got data packet        (0=no,1=yes)
    createStatusParam("Ch7:GotCmd",       0x7,  1, 13); // Chan7 got command packet     (0=no,1=yes)
    createStatusParam("Ch8:GotData",      0x7,  1, 14); // Chan8 got data packet        (0=no,1=yes)
    createStatusParam("Ch8:GotCmd",       0x7,  1, 15); // Chan8 got command packet     (0=no,1=yes)

    createStatusParam("Ch9:GotData",      0x8,  1,  0); // Chan9 got data packet        (0=no,1=yes)
    createStatusParam("Ch9:GotCmd",       0x8,  1,  1); // Chan9 got command packet     (0=no,1=yes)
    createStatusParam("Ctrl:GotData",     0x8,  1,  4); // CTRL got data packet         (0=no,1=yes)
    createStatusParam("Ctrl:GotCmd",      0x8,  1,  5); // CTRL got command packet      (0=no,1=yes)
    createStatusParam("NewHwId",          0x8,  1,  6); // Found new hardware ID        (0=no,1=yes)
    createStatusParam("CntFilteredAcks",  0x8,  8,  8); // Number Filtered ACKS

    createStatusParam("Ch1:FifoHasData",  0x9,  1,  0); // Chan1 FIFO not empty         (0=empty,1=has data)
    createStatusParam("Ch1:FifoAlmostFul",0x9,  1,  1); // Chan1 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch2:FifoHasData",  0x9,  1,  2); // Chan2 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch2:FifoAlmostFul",0x9,  1,  3); // Chan2 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch3:FifoHasData",  0x9,  1,  4); // Chan3 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch3:FifoAlmostFul",0x9,  1,  5); // Chan3 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch4:FifoHasData",  0x9,  1,  6); // Chan4 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch4:FifoAlmostFul",0x9,  1,  7); // Chan4 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch5:FifoHasData",  0x9,  1,  8); // Chan5 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch5:FifoAlmostFul",0x9,  1,  9); // Chan5 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch6:FifoHasData",  0x9,  1, 10); // Chan6 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch6:FifoAlmostFul",0x9,  1, 11); // Chan6 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch7:FifoHasData",  0x9,  1, 12); // Chan7 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch7:FifoAlmostFul",0x9,  1, 13); // Chan7 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ch8:FifoHasData",  0x9,  1, 14); // Chan8 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch8:FifCmd",       0x9,  1, 15); // Chan8 FIFO almost full       (0=not full,1=almost full)

    createStatusParam("Ch9:FifoHasData",  0xA,  1,  0); // Chan9 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch9:FifoAlmostFul",0xA,  1,  1); // Chan9 FIFO almost full       (0=not full,1=almost full)
    createStatusParam("Ctrl:FifoHasData", 0xA,  1,  4); // CTRL FIFO has data           (0=empty,1=has data)
    createStatusParam("Ctrl:FifoAlmostFul",0xA, 1,  5); // CTRL FIFO almost full        (0=not full,1=almost full)
    createStatusParam("CmdOut:HasData",   0xA,  1, 12); // CMD output FIFO has data     (0=empty,1=has data)
    createStatusParam("CmdOut:AlmostFull",0xA,  1, 13); // CMD output FIFO almost full  (0=not full,1=almost full)
    createStatusParam("CmdInt:HasData",   0xA,  1, 14); // CMD out iface FIFO has data  (0=empty,1=has data)
    createStatusParam("CmdInt:AlmostFull",0xA,  1, 15); // CMD out iface FIFO almost f  (0=not full,1=almost full)

    createStatusParam("Tclk=0",           0xB,  1,  0); // LVDS input TCLK = 0          (0=no,1=yes)
    createStatusParam("Tclk=1",           0xB,  1,  1); // LVDS input TCLK = 1          (0=no,1=yes)
    createStatusParam("Tsync=0",          0xB,  1,  2); // LVDS input TSYNC = 0         (0=no,1=yes)
    createStatusParam("Tsync=1",          0xB,  1,  3); // LVDS input TSYNC = 1         (0=no,1=yes)
    createStatusParam("TxEn=0",           0xB,  1,  4); // LVDS input TXEN# = 0         (0=no,1=yes)
    createStatusParam("TxEn=1",           0xB,  2,  5); // LVDS input TXEN# = 1         (0=no,1=yes)
    createStatusParam("Sysrst=0",         0xB,  1,  6); // LVDS input Sysrst# = 0       (0=no,1=yes)
    createStatusParam("Sysrs1",           0xB,  1,  7); // LVDS input Sysrst# = 0       (0=no,1=yes)
    createStatusParam("CntFilteredCmds",  0xB, 16,  8); // N filtered command packets

    createStatusParam("EepromStatus",     0xC, 17,  8); // EEPROM status - not used

    createStatusParam("CntHwIds",         0xD,  9,  9); // Num of Hardware IDs detected

    createStatusParam("LvdsVerifyReady",  0xE,  1,  2); // LVDS verify ready            (0=not ready,1=ready)
    createStatusParam("TclkLos=0",        0xE,  1,  8); // TCLK LOS = 0                 (0=no,1=yes)
    createStatusParam("TclkLos=1",        0xE,  1,  9); // TCLK LOS = 1                 (0=no,1=yes)
    createStatusParam("TsyncLos=0",       0xE,  1, 10); // TSYNC LOS = 0                (0=no,1=yes)
    createStatusParam("TsyncLos=1",       0xE,  1, 11); // TSYNC LOS = 1                (0=no,1=yes)
    createStatusParam("SysrstBLos=0",     0xE,  1, 12); // SYSRST_B LOS = 0             (0=no,1=yes)
    createStatusParam("SysrstBLos=1",     0xE,  1, 13); // SYSRST_B LOS = 1             (0=no,1=yes)
    createStatusParam("TxenBLos=0",       0xE,  1, 14); // TXEN_B LOS = 0               (0=no,1=yes)
    createStatusParam("TxenBLos=1",       0xE,  1, 15); // TXEN_B LOS = 1               (0=no,1=yes)

    createStatusParam("EepromCode",       0xF, 16,  0); // EEPROM code

    createStatusParam("LvdsDebug0",      0x10, 16,  0); // LVDS Debug [15:0]

    createStatusParam("LvdsDebug1",      0x11, 16,  0); // LVDS Debug [31:16]

    createStatusParam("LvdsDebug2",      0x12, 12,  0); // LVDS Debug [43:32]
    createStatusParam("LvdsDebugCntrl",  0x12,  4, 12); // LVDS Debug Control Index
}

void FemPlugin::createConfigParams_v35()
{
//     BLXXX:Det:FemXX:| sig name |                                | EPICS record description | (bi and mbbi description)
    createConfigParam("ResetCtrl",    'E', 0x0,  2,  0, 0);     // Reset control                (0=not used,1=not used,2=from LVDS,3=from optic)
    createConfigParam("TclkCtrl",     'E', 0x0,  2,  2, 0);     // TCLK control                 (0=TCLK,1=TCLK,2=always 0,3=always 1)
    createConfigParam("TsyncCtrl",    'E', 0x0,  2,  4, 0);     // TSYNC control                (0=from polariry,1=from TSYNC width,2=always 0,3=always 1)
    createConfigParam("TxenCtrl",     'E', 0x0,  2,  6, 0);     // TXEN control                 (0=TXEN,1=TXEN,2=always 0,3=always 1)
    createConfigParam("LvdsMonitorEn",'E', 0x0,  1,  8, 0);     // LVDS debug monitor           (0=disable,1=enable)

    createConfigParam("ResetMode",    'F', 0x0,  1,  0, 1);     // Reset mode                   (0=internal,1=external)
    createConfigParam("TclkMode",     'F', 0x0,  1,  1, 0);     // TCLK mode                    (0=external,1=internal)
    createConfigParam("TsyncMode",    'F', 0x0,  1,  2, 0);     // TSYNC mode                   (0=external,1=internal)
    createConfigParam("TxenMode",     'F', 0x0,  1,  3, 0);     // TXEN mode                    (0=external,1=internal)
    createConfigParam("Ch1:Disable",  'F', 0x0,  1,  4, 0);     // Chan1 disable                (0=enable,1=disable)
    createConfigParam("Ch2:Disable",  'F', 0x0,  1,  5, 0);     // Chan2 disable                (0=enable,1=disable)
    createConfigParam("Ch3:Disable",  'F', 0x0,  1,  6, 0);     // Chan3 disable                (0=enable,1=disable)
    createConfigParam("Ch4:Disable",  'F', 0x0,  1,  7, 0);     // Chan4 disable                (0=enable,1=disable)
    createConfigParam("Ch5:Disable",  'F', 0x0,  1,  8, 0);     // Chan5 disable                (0=enable,1=disable)
    createConfigParam("Ch6:Disable",  'F', 0x0,  1,  9, 0);     // Chan6 disable                (0=enable,1=disable)
    createConfigParam("Ch7:Disable",  'F', 0x0,  1, 10, 0);     // Chan7 disable                (0=enable,1=disable)
    createConfigParam("Ch8:Disable",  'F', 0x0,  1, 11, 0);     // Chan8 disable                (0=enable,1=disable)
    createConfigParam("Ch9:Disable",  'F', 0x0,  1, 12, 0);     // Chan9 disable                (0=enable,1=disable)
    createConfigParam("Ch10:Disable", 'F', 0x0,  1, 13, 0);     // Chan10 disable               (0=enable,1=disable)
    createConfigParam("VerbResponse", 'F', 0x0,  1, 14, 0);     // Verbose command response     (0=disable,1=enable)
    createConfigParam("VerbDisc",     'F', 0x0,  1, 15, 0);     // Verbose discover             (0=disable,1=enable)
}
