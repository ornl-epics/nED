/* DspPlugin_v52.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspPlugin.h"

void DspPlugin::createParams_v52() {
    createRegParam("VERSION", "HwDay",  true, 0,  8,  0, 0, CONV_HEX2DEC); // Hardware day
    createRegParam("VERSION", "HwMonth",true, 0,  8,  8, 0, CONV_HEX2DEC); // Hardware month
    createRegParam("VERSION", "HwYear", true, 0,  8, 16, 0, CONV_HEX2DEC2K); // Hardware year
    createRegParam("VERSION", "HwRev",  true, 0,  4, 24); // Hardware revision
    createRegParam("VERSION", "HwVer",  true, 0,  4, 28); // Hardware version
    createRegParam("VERSION", "FwDay",  true, 1,  8,  0, 0, CONV_HEX2DEC); // Firmware day
    createRegParam("VERSION", "FwMonth",true, 1,  8,  8, 0, CONV_HEX2DEC); // Firmware month
    createRegParam("VERSION", "FwYear", true, 1,  8, 16, 0, CONV_HEX2DEC2K); // Firmware year
    createRegParam("VERSION", "FwRev",  true, 1,  4, 24); // Firmware revision                  (low:1,high:3)
    createRegParam("VERSION", "FwVer",  true, 1,  4, 28); // Firmware version                   (low:4,high:6)

//      BLXXX:Det:DspX:| sig nam|                                 | EPICS record description | (bi and mbbi description)
    createConfigParam("PixId0",         'B', 0x0,  32,  0, 0); // State 0 Position Index Off
    createConfigParam("PixId1",         'B', 0x1,  32,  0, 0); // State 1 Position Index Off
    createConfigParam("PixId2",         'B', 0x2,  32,  0, 0); // State 2 Position Index Off
    createConfigParam("PixId3",         'B', 0x3,  32,  0, 0); // State 3 Position Index Off
    createConfigParam("PixId4",         'B', 0x4,  32,  0, 0); // State 4 Position Index Off

    createConfigParam("TsyncDelay",     'C', 0x0,  32,  0, 0); // LVDS TSYNC delay             (scale:9.4,unit:ns)
    createConfigParam("Meta0:Mode",     'C', 0x1,   2,  0, 0); // ODB0 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta1:Mode",     'C', 0x1,   2,  2, 0); // ODB1 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta2:Mode",     'C', 0x1,   2,  4, 0); // ODB2 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta3:Mode",     'C', 0x1,   2,  6, 0); // ODB3 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta4:Mode",     'C', 0x1,   2,  8, 0); // ODB4 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta5:Mode",     'C', 0x1,   2, 10, 0); // ODB5 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta6:Mode",     'C', 0x1,   2, 12, 0); // ODB6 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta7:Mode",     'C', 0x1,   2, 14, 0); // ODB7 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta8:Mode",     'C', 0x1,   2, 16, 0); // ODB8 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta9:Mode",     'C', 0x1,   2, 18, 0); // ODB9 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta10:Mode",    'C', 0x1,   2, 20, 0); // ODB10 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta11:Mode",    'C', 0x1,   2, 22, 0); // ODB11 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta12:Mode",    'C', 0x1,   2, 24, 0); // ODB12 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta13:Mode",    'C', 0x1,   2, 26, 0); // ODB13 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta14:Mode",    'C', 0x1,   2, 28, 0); // ODB14 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta15:Mode",    'C', 0x1,   2, 30, 0); // ODB15 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta0:PixId",    'C', 0x2,  32,  0, 0x50000000); // ODB0 edge pixel id
    createConfigParam("Meta1:PixId",    'C', 0x3,  32,  0, 0x50000002); // ODB1 edge pixel id
    createConfigParam("Meta2:PixId",    'C', 0x4,  32,  0, 0x50000004); // ODB2 edge pixel id
    createConfigParam("Meta3:PixId",    'C', 0x5,  32,  0, 0x50000006); // ODB3 edge pixel id
    createConfigParam("Meta4:PixId",    'C', 0x6,  32,  0, 0x50000008); // ODB4 edge pixel id
    createConfigParam("Meta5:PixId",    'C', 0x7,  32,  0, 0x5000000a); // ODB5 edge pixel id
    createConfigParam("Meta6:PixId",    'C', 0x8,  32,  0, 0x5000000c); // ODB6 edge pixel id
    createConfigParam("Meta7:PixId",    'C', 0x9,  32,  0, 0x5000000e); // ODB7 edge pixel id
    createConfigParam("Meta8:PixId",    'C', 0xA,  32,  0, 0x50000010); // ODB8 edge pixel id
    createConfigParam("Meta9:PixId",    'C', 0xB,  32,  0, 0x50000012); // ODB9 edge pixel id
    createConfigParam("Meta10:PixId",   'C', 0xC,  32,  0, 0x50000014); // ODB10 edge pixel id
    createConfigParam("Meta11:PixId",   'C', 0xD,  32,  0, 0x50000016); // ODB11 edge pixel id
    createConfigParam("Meta12:PixId",   'C', 0xE,  32,  0, 0x50000018); // ODB12 edge pixel id
    createConfigParam("Meta13:PixId",   'C', 0xF,  32,  0, 0x5000001a); // ODB13 edge pixel id
    createConfigParam("Meta14:PixId",   'C', 0x10, 32,  0, 0x5000001c); // ODB14 edge pixel id
    createConfigParam("Meta15:PixId",   'C', 0x11, 32,  0, 0x5000001e); // ODB15 edge pixel id

    createConfigParam("LvdsWatchdog",   'D', 0x0,   1, 12, 0); // Alternate LVDS out when idle  (0=disable, 1=enable)
    createConfigParam("LvdsCmdMode",    'D', 0x0,   1, 13, 0); // LVDS command parser mode      (0=as command,1=as data)
    createConfigParam("LvdsDataMode",   'D', 0x0,   1, 14, 0); // LVDS data parser mode         (0=as data,1=as command)
    createConfigParam("LvdsDataSize",   'D', 0x0,   9, 15, 0); // LVDS data pkt num words
    createConfigParam("OpticDelay",     'D', 0x0,   7, 24, 0); // Optical packet delay
    createConfigParam("OpticDelayEn",   'D', 0x0,   1, 30, 0); // Optical packet delay switch   (0=disable, 1=enable)

    createConfigParam("LvdsFilterCmd",  'D', 0x1, 16,  0, 0); // LVDS command to filter
    createConfigParam("LvdsFilterMask", 'D', 0x1, 16, 16, 0xFFFF); // LVDS command filter mask

//  D:2 = fake_info - not supported even in DSP-T 6.3+
    createConfigParam("OptMaxSize",     'D', 0x3, 14,  0, 16111); // Optical packet max dwords
    createConfigParam("OptMasterTc",    'D', 0x3,  1, 16, 1); // Optical master T&C           (0=disable, 1=enable)
    createConfigParam("OptMasterFrame", 'D', 0x3,  1, 18, 0); // Optical master frame         (0=disable, 1=enable)
    createConfigParam("OptMaster",      'D', 0x3,  1, 19, 1); // Optical master               (0=disable, 1=enable)

    createConfigParam("LvdsTsyncGen",   'E', 0x0, 32,  0, 0); // LVDS TSYNC period
    createConfigParam("TsyncWaitDelay", 'E', 0x1, 19,  0, 0); // TSYNC stretch veto delay
    createConfigParam("LvdsSysrstCtrl", 'E', 0x1,  2, 24, 0); // LVDS T&C SYSRST# buffer ctrl (0=sysrst,1=sysrst,2=always 0,3=always 1)
    createConfigParam("LvdsTsyncCtrl",  'E', 0x1,  2, 26, 0); // LVDS TSYNC_NORMAL control    (0=polarity,1=TSYNC WIDTH,2=always 0,3=always 1)
    createConfigParam("LvdsTclkCtrl",   'E', 0x1,  2, 28, 0); // LVDS TX control T&C TCLK mod (0=TCLK,1=TCLK,2=always 0,3=always 1)
    createConfigParam("DipSwEn",        'E', 0x1,  1, 30, 0); // Enable on-board DIP switches (0=disable, 1=enable)
    createConfigParam("EndOfChainEn",   'E', 0x1,  1, 31, 0); // Enable end of chain vetos    (0=disable, 1=enable)

    createConfigParam("HystMinLow",     'F', 0x0,  4,  0, 4); // Chop HYST minimum low
    createConfigParam("HystMinHi",      'F', 0x0,  4,  4, 4); // Chop HYST minimum high
    createConfigParam("OperMode",       'F', 0x0,  3,  8, 0); // Operation mode               (0=normal,1=fake data,2=fake trigger,3=idle)
    createConfigParam("BadTlkEn",       'F', 0x0,  1, 11, 0); // Send bad TLK packet switch   (0=no,1=yes)

    createConfigParam("SysRstMode",     'F', 0x1,  2,  0, 0); // Reset mode => SYSRST_O#      (0=not used,1=not used,2=from LVDS T&C,3=from optical T&C)
    createConfigParam("LvdsTclkMode",   'E', 0x1,  2,  2, 0); // LVDS TX control TCLK mode    (0=TCLK from int,1=TCLK from int,2=TCLK from LVDS,3=TCLK from optical)
    createConfigParam("LvdsTsyncMode",  'E', 0x1,  2,  4, 1); // LVDS TSYNC_O mode            (0=local TSYNC,1=TSYNC from TREF,2=TSYNC from LVDS,3=TSYNC from opt)
    createConfigParam("BlockVetoData",  'F', 0x1,  1,  6, 0); // Block data from vetoed pkts  (0=disable,1=enable)
    createConfigParam("DataEn",         'F', 0x1,  1,  7, 0); // Send each data pkt to optic  (0=disable,1=enable)
    createConfigParam("OpticTcHyst",    'F', 0x1,  1,  8, 0); // Optical T&C hysteresis       (0=disable,1=enable)
    createConfigParam("LvdsPowerRst",   'F', 0x1,  1,  9, 1); // Execute power down sequence  (0=execute,1=bypass)
    createConfigParam("SerialAParity",  'F', 0x1,  2, 12, 0); // Serial A parity mode         (0=no parity, 1=no parity, 1=even parity, 2=odd parity)
    createConfigParam("SerialBParity",  'F', 0x1,  2, 14, 0); // Serial B parity mode         (0=no parity, 1=no parity, 1=even parity, 2=odd parity)
    createConfigParam("LvdsOutClck",    'F', 0x1,  2, 16, 0); // LVDS output clock mode       (0=35.3 Mhz, 1=30.2 Mhz, 2=26.5 Mhz, 3=23.6 Mhz)
    createConfigParam("SysPassEn",      'F', 0x1,  1, 18, 0); // Response for passthru cmd    (0=don't send,1=send)
    createConfigParam("SysStartEn",     'F', 0x1,  1, 19, 1); // Wait for Start/Stop response (0=don't wait,1=wait)
    createConfigParam("Lvds1:En",       'F', 0x1,  1, 20, 0); // LVDS chan1 disable           (0=enable,1=disable)
    createConfigParam("Lvds2:En",       'F', 0x1,  1, 21, 0); // LVDS chan2 disable           (0=enable,1=disable)
    createConfigParam("Lvds3:En",       'F', 0x1,  1, 22, 0); // LVDS chan3 disable           (0=enable,1=disable)
    createConfigParam("Lvds4:En",       'F', 0x1,  1, 23, 0); // LVDS chan4 disable           (0=enable,1=disable)
    createConfigParam("Lvds5:En",       'F', 0x1,  1, 24, 0); // LVDS chan5 disable           (0=enable,1=disable)
    createConfigParam("Lvds6:En",       'F', 0x1,  1, 25, 0); // LVDS chan6 disable           (0=enable,1=disable)
    createConfigParam("PositionAdj",    'F', 0x1,  1, 26, 0); // Position index adjust enable (0=enable,1=disable)
    createConfigParam("DualTlk",        'F', 0x1,  1, 27, 0); // Dual TLK mode                (0=enable,1=disable)
    createConfigParam("LvdsPowerCtrl",  'F', 0x1,  1, 28, 1); // Power Down during reset      (0=power down,1=power up)
    createConfigParam("LvdsFilterAll",  'F', 0x1,  2, 30, 0); // Filter all commands          (0=disable, 1=enable)


    // STATUS params
    createStatusParam("Lvds1:Parity",   0x0,  1,  0); // Parity error                 (0=no,1=yes)
    createStatusParam("Lvds1:Type",     0x0,  1,  1); // Data type error              (0=no,1=yes)
    createStatusParam("Lvds1:Short",    0x0,  1,  2); // Short length error           (0=no,1=yes)
    createStatusParam("Lvds1:Long",     0x0,  1,  3); // Long length error            (0=no,1=yes)
    createStatusParam("Lvds1:Timeout",  0x0,  1,  4); // Timeout error                (0=no,1=yes)
    createStatusParam("Lvds1:NoStart",  0x0,  1,  5); // Data without start           (0=no,1=yes)
    createStatusParam("Lvds1:PreStart", 0x0,  1,  6); // Start during data            (0=no,1=yes)
    createStatusParam("Lvds1:FifoFull", 0x0,  1,  7); // FIFO went full               (0=no,1=yes)
    createStatusParam("Lvds2:Parity",   0x0,  1,  8); // Parity error                 (0=no,1=yes)
    createStatusParam("Lvds2:Type",     0x0,  1,  9); // Data type error              (0=no,1=yes)
    createStatusParam("Lvds2:Short",    0x0,  1, 10); // Short length error           (0=no,1=yes)
    createStatusParam("Lvds2:Long",     0x0,  1, 11); // Long length error            (0=no,1=yes)
    createStatusParam("Lvds2:Timeout",  0x0,  1, 12); // Timeout error                (0=no,1=yes)
    createStatusParam("Lvds2:NoStart",  0x0,  1, 13); // Data without start           (0=no,1=yes)
    createStatusParam("Lvds2:PreStart", 0x0,  1, 14); // Start during data            (0=no,1=yes)
    createStatusParam("Lvds2:FifoFull", 0x0,  1, 15); // FIFO went full               (0=no,1=yes)
    createStatusParam("Lvds3:Parity",   0x0,  1, 16); // Parity error                 (0=no,1=yes)
    createStatusParam("Lvds3:Type",     0x0,  1, 17); // Data type error              (0=no,1=yes)
    createStatusParam("Lvds3:Short",    0x0,  1, 18); // Short length error           (0=no,1=yes)
    createStatusParam("Lvds3:Long",     0x0,  1, 19); // Long length error            (0=no,1=yes)
    createStatusParam("Lvds3:Timeout",  0x0,  1, 20); // Timeout error                (0=no,1=yes)
    createStatusParam("Lvds3:NoStart",  0x0,  1, 21); // Data without start           (0=no,1=yes)
    createStatusParam("Lvds3:PreStart", 0x0,  1, 22); // Start during data            (0=no,1=yes)
    createStatusParam("Lvds3:FifoFull", 0x0,  1, 23); // FIFO went full               (0=no,1=yes)
    createStatusParam("Lvds4:Parity",   0x0,  1, 24); // Parity error                 (0=no,1=yes)
    createStatusParam("Lvds4:Type",     0x0,  1, 25); // Data type error              (0=no,1=yes)
    createStatusParam("Lvds4:Short",    0x0,  1, 26); // Short length error           (0=no,1=yes)
    createStatusParam("Lvds4:Long",     0x0,  1, 27); // Long length error            (0=no,1=yes)
    createStatusParam("Lvds4:Timeout",  0x0,  1, 28); // Timeout error                (0=no,1=yes)
    createStatusParam("Lvds4:NoStart",  0x0,  1, 29); // Data without start           (0=no,1=yes)
    createStatusParam("Lvds4:PreStart", 0x0,  1, 30); // Start during data            (0=no,1=yes)
    createStatusParam("Lvds4:FifoFull", 0x0,  1, 31); // FIFO went full               (0=no,1=yes)

    createStatusParam("Lvds5:Parity",   0x1,  1,  0); // Parity error                 (0=no,1=yes)
    createStatusParam("Lvds5:Type",     0x1,  1,  1); // Data type error              (0=no,1=yes)
    createStatusParam("Lvds5:Short",    0x1,  1,  2); // Short length error           (0=no,1=yes)
    createStatusParam("Lvds5:Long",     0x1,  1,  3); // Long length error            (0=no,1=yes)
    createStatusParam("Lvds5:Timeout",  0x1,  1,  4); // Timeout error                (0=no,1=yes)
    createStatusParam("Lvds5:NoStart",  0x1,  1,  5); // Data without start           (0=no,1=yes)
    createStatusParam("Lvds5:PreStart", 0x1,  1,  6); // Start during data            (0=no,1=yes)
    createStatusParam("Lvds5:FifoFull", 0x1,  1,  7); // FIFO went full               (0=no,1=yes)
    createStatusParam("Lvds6:Parity",   0x1,  1,  8); // Parity error                 (0=no,1=yes)
    createStatusParam("Lvds6:Type",     0x1,  1,  9); // Data type error              (0=no,1=yes)
    createStatusParam("Lvds6:Short",    0x1,  1, 10); // Short length error           (0=no,1=yes)
    createStatusParam("Lvds6:Long",     0x1,  1, 11); // Long length error            (0=no,1=yes)
    createStatusParam("Lvds6:Timeout",  0x1,  1, 12); // Timeout error                (0=no,1=yes)
    createStatusParam("Lvds6:NoStart",  0x1,  1, 13); // Data without start           (0=no,1=yes)
    createStatusParam("Lvds6:PreStart", 0x1,  1, 14); // Start during data            (0=no,1=yes)
    createStatusParam("Lvds6:FifoFull", 0x1,  1, 15); // FIFO went full               (0=no,1=yes)
    createStatusParam("OptDC:Los",      0x1,  1, 16); // Optical data/command LOS     (0=no,1=yes)
    createStatusParam("OptDC:TxFault",  0x1,  1, 17); // Optical data/command TXFAULT (0=no,1=yes)
    createStatusParam("OptTC:Los",      0x1,  1, 18); // Optical T&C detected LOS     (0=no,1=yes)
    createStatusParam("OptTC:TxFault",  0x1,  1, 19); // Optical T&C detected TXFAULT (0=no,1=yes)
    createStatusParam("OptTC:Failure",  0x1,  1, 20); // Optical T&C detected FAILURE (0=no,1=yes)
    createStatusParam("OptTC:PktErrCnt",0x1,  8, 21); // Optical T&C detected TXFAULT (0=no,1=yes)
    createStatusParam("OptDC:Timeout",  0x1,  1, 29); // Optical data/command timeout (0=no,1=yes)
    createStatusParam("OptDC:Sec1:SOF", 0x1,  1, 30); // Optical data/command SOF     (0=no,1=yes)
    createStatusParam("OptDC:Sec1:EOF", 0x1,  1, 31); // Optical data/command EOF     (0=no,1=yes)

    createStatusParam("OptDC:Sec2:SOF", 0x2,  1,  0); // Optical data/command SOF     (0=no,1=yes)
    createStatusParam("OptDC:Sec2:EOF", 0x2,  1,  1); // Optical data/command EOF     (0=no,1=yes)
    createStatusParam("OptDC:Sec3:SOF", 0x2,  1,  2); // Optical data/command SOF     (0=no,1=yes)
    createStatusParam("OptDC:Sec3:EOF", 0x2,  1,  3); // Optical data/command EOF     (0=no,1=yes)
    createStatusParam("OptDC:Sec4:SOF", 0x2,  1,  4); // Optical data/command SOF     (0=no,1=yes)
    createStatusParam("OptDC:Sec4:EOF", 0x2,  1,  5); // Optical data/command EOF     (0=no,1=yes)
    createStatusParam("OptDC:LowCRC",   0x2,  1,  6); // Optical data/command highCRC (0=no,1=yes)
    createStatusParam("OptDC:HighCRC",  0x2,  1,  7); // Optical data/command low CRC (0=no,1=yes)
    createStatusParam("UnknownCmd",     0x2,  1,  8); // Unknown command              (0=no,1=yes)
    createStatusParam("PktLengthErr",   0x2,  1,  9); // Packet length error          (0=no,1=yes)
    createStatusParam("ConfigErr",      0x2,  1, 10); // Programming error            (0=no,1=yes)
    createStatusParam("MissingHwid",    0x2,  1, 12); // Missing HWID in init         (0=no,1=yes)
    createStatusParam("EepromErr",      0x2,  1, 13); // Detected EEPROM error        (0=no,1=yes)
    createStatusParam("SerialA:Parity", 0x2,  1, 14); // Serial A parity error        (0=no,1=yes)
    createStatusParam("SerialA:Byte",   0x2,  1, 15); // Serial A byte error          (0=no,1=yes)
    createStatusParam("SerialB:Parity", 0x2,  1, 16); // Serial B parity error        (0=no,1=yes)
    createStatusParam("SerialB:Byte",   0x2,  1, 17); // Serial B byte error          (0=no,1=yes)
    createStatusParam("FilterNackCmd",  0x2,  1, 18); // Filtered NACK command        (0=no,1=yes)
    createStatusParam("DataNoStart",    0x2,  1, 19); // Data without start           (0=no,1=yes)
    createStatusParam("DataTimeout",    0x2,  1, 20); // Data timeout error           (0=no,1=yes)
    createStatusParam("DataPreStart",   0x2,  1, 21); // Start during data            (0=no,1=yes)
    createStatusParam("OptDC:FifoAF",   0x2,  1, 22); // Optical D/C FIFO almost full (0=no,1=yes)
    createStatusParam("OptDC:PTFifoAF", 0x2,  1, 23); // Optical D/C PT FIFO almost f (0=no,1=yes)
    createStatusParam("FilterNackCnt",  0x2,  8, 24); // Filtered NACKs count

    createStatusParam("Lvds1:GotData",  0x3,  1,  0); // Got data packet              (0=no,1=yes)
    createStatusParam("Lvds1:GotCmd",   0x3,  1,  1); // Got cmd packet               (0=no,1=yes)
    createStatusParam("Lvds2:GotData",  0x3,  1,  2); // Got data packet              (0=no,1=yes)
    createStatusParam("Lvds2:GotCmd",   0x3,  1,  3); // Got cmd packet               (0=no,1=yes)
    createStatusParam("Lvds3:GotData",  0x3,  1,  4); // Got data packet              (0=no,1=yes)
    createStatusParam("Lvds3:GotCmd",   0x3,  1,  5); // Got cmd packet               (0=no,1=yes)
    createStatusParam("Lvds4:GotData",  0x3,  1,  6); // Got data packet              (0=no,1=yes)
    createStatusParam("Lvds4:GotCmd",   0x3,  1,  7); // Got cmd packet               (0=no,1=yes)
    createStatusParam("Lvds5:GotData",  0x3,  1,  8); // Got data packet              (0=no,1=yes)
    createStatusParam("Lvds5:GotCmd",   0x3,  1,  9); // Got cmd packet               (0=no,1=yes)
    createStatusParam("Lvds6:GotData",  0x3,  1, 10); // Got data packet              (0=no,1=yes)
    createStatusParam("Lvds6:GotCmd",   0x3,  1, 11); // Got cmd packet               (0=no,1=yes)
    createStatusParam("OptDC:Good",     0x3,  1, 12); // Optical D/C link good        (0=no,1=yes)
    createStatusParam("OptTC:Good",     0x3,  1, 13); // Optical T&C link good        (0=no,1=yes)
    createStatusParam("OptDC:GoodPkt",  0x3,  1, 14); // Optical D/C link good pkt    (0=no,1=yes)
    createStatusParam("DataVeto",       0x3,  1, 15); // Got data veto                (0=no,1=yes)
    createStatusParam("DataPkt",        0x3,  1, 16); // Got data packet              (0=no,1=yes)
    createStatusParam("PolarityVeto",   0x3,  1, 17); // Got polarity veto            (0=no,1=yes)
    createStatusParam("NewHwid",        0x3,  1, 19); // Got new HWID                 (0=no,1=yes)
    createStatusParam("Acquiring",      0x3,  1, 20); // Acquiring data               (0=no,1=yes)
    createStatusParam("Configured",     0x3,  1, 21); // Configured                   (0=no [alarm],1=yes)
    createStatusParam("FilterAckCnt",   0x3,  8, 24); // Filtered ACKs count

    createStatusParam("Lvds1:LFReady",  0x4,  1,  0); // LVDS FIFO data ready         (0=no,1=yes)
    createStatusParam("Lvds1:LFAFull",  0x4,  1,  1); // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("Lvds1:FReady",   0x4,  1,  2); // FIFO data ready              (0=no,1=yes)
    createStatusParam("Lvds1:FAFull",   0x4,  1,  3); // FIFO almost full             (0=no,1=yes)
    createStatusParam("Lvds2:LFReady",  0x4,  1,  4); // LVDS FIFO data ready         (0=no,1=yes)
    createStatusParam("Lvds2:LFAFull",  0x4,  1,  5); // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("Lvds2:FReady",   0x4,  1,  6); // FIFO data ready              (0=no,1=yes)
    createStatusParam("Lvds2:FAFull",   0x4,  1,  7); // FIFO almost full             (0=no,1=yes)
    createStatusParam("Lvds3:LFReady",  0x4,  1,  8); // LVDS FIFO data ready         (0=no,1=yes)
    createStatusParam("Lvds3:LFAFull",  0x4,  1,  9); // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("Lvds3:FReady",   0x4,  1, 10); // FIFO data ready              (0=no,1=yes)
    createStatusParam("Lvds3:FAFull",   0x4,  1, 11); // FIFO almost full             (0=no,1=yes)
    createStatusParam("Lvds4:LFReady",  0x4,  1, 12); // LVDS FIFO data ready         (0=no,1=yes)
    createStatusParam("Lvds4:LFAFull",  0x4,  1, 13); // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("Lvds4:FReady",   0x4,  1, 14); // FIFO data ready              (0=no,1=yes)
    createStatusParam("Lvds4:FAFull",   0x4,  1, 15); // FIFO almost full             (0=no,1=yes)
    createStatusParam("Lvds5:LFReady",  0x4,  1, 16); // LVDS FIFO data ready         (0=no,1=yes)
    createStatusParam("Lvds5:LFAFull",  0x4,  1, 17); // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("Lvds5:FReady",   0x4,  1, 18); // FIFO data ready              (0=no,1=yes)
    createStatusParam("Lvds5:FAFull",   0x4,  1, 19); // FIFO almost full             (0=no,1=yes)
    createStatusParam("Lvds6:LFReady",  0x4,  1, 20); // LVDS FIFO data ready         (0=no,1=yes)
    createStatusParam("Lvds6:LFAFull",  0x4,  1, 21); // LVDS FIFO almost full        (0=no,1=yes)
    createStatusParam("Lvds6:FReady",   0x4,  1, 22); // FIFO data ready              (0=no,1=yes)
    createStatusParam("Lvds6:FAFull",   0x4,  1, 23); // FIFO almost full             (0=no,1=yes)
    createStatusParam("ReaderFReady",   0x4,  1, 24); // Data reader FIFO ready       (0=no,1=yes)
    createStatusParam("ReaderFAFull",   0x4,  1, 25); // Data reader FIFO almost full (0=no,1=yes)
    createStatusParam("DataFReady",     0x4,  1, 26); // Data FIFO data ready         (0=no,1=yes)
    createStatusParam("DataFAFull",     0x4,  1, 27); // Data FIFO almost full        (0=no,1=yes)
    createStatusParam("DataWCFReady",   0x4,  1, 28); // Data word cnt FIFO data rdy  (0=no,1=yes)
    createStatusParam("DataWCFAFull",   0x4,  1, 29); // Data word cnt FIFO almost f  (0=no,1=yes)
    createStatusParam("OutLFAFull",     0x4,  1, 30); // Out LVDS FIFO almost full    (0=no,1=yes)

// TODO

    createStatusParam("TsyncMinCnt",    0xA, 32,  0); // TSYNC minimum count
    createStatusParam("TsyncMaxCnt",    0xB, 32,  0); // TSYNC maximum count

// TODO

    createStatusParam("EdgeStart",      0xD,  1,  0); // EDGE: Made start             (0=no,1=yes)
    createStatusParam("EdgeStop",       0xD,  1,  1); // EDGE: Made stop              (0=no,1=yes)
    createStatusParam("EdgeGotEF",      0xD,  1,  2); // EDGE: Got EF=0               (0=no,1=yes)
    createStatusParam("EdgeGotAF",      0xD,  1,  3); // EDGE: Got AF=1               (0=no,1=yes)
}
