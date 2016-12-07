/* AcpcPlugin_v41.cpp
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"

/**
 * @file AcpcPlugin_v171.cpp
 *
 * ACPC 17.01 parameters
 *
 * ACPC 17.01 is next version on Miljko's ACPC, which cleans up register interface.
 */

void AcpcPlugin::createStatusParams_v171()
{
    createStatusParam("Lvds1:DataParseError", 0x0,  8,  0); // Data parser error status
    createStatusParam("Lvds1:DataParseGood",  0x0,  2,  8); // Data parser good status
    createStatusParam("Lvds1:FifoEmpty",      0x0,  1, 10); // LVDS1 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds1:FifoAlmostFull", 0x0,  1, 11); // LVDS1 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds1:FifoGotAf",      0x0,  1, 12); // LVDS1 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds1:FifoGotEf",      0x0,  1, 13); // LVDS1 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds1:CntDataError",   0x1,  0, 16); // LVDS1 data errors counter

    createStatusParam("Lvds2:DataParseError", 0x2,  8,  0); // Data parser error status
    createStatusParam("Lvds2:DataParseGood",  0x2,  2,  8); // Data parser good status
    createStatusParam("Lvds2:FifoEmpty",      0x2,  1, 10); // LVDS2 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds2:FifoAlmostFull", 0x2,  1, 11); // LVDS2 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds2:FifoGotAf",      0x2,  1, 12); // LVDS2 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds2:FifoGotEf",      0x2,  1, 13); // LVDS2 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds2:CntDataError",   0x3,  0, 16); // LVDS2 data errors counter

    createStatusParam("Lvds3:DataParseError", 0x4,  8,  0); // Data parser error status
    createStatusParam("Lvds3:DataParseGood",  0x4,  2,  8); // Data parser good status
    createStatusParam("Lvds3:FifoEmpty",      0x4,  1, 10); // LVDS3 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds3:FifoAlmostFull", 0x4,  1, 11); // LVDS3 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds3:FifoGotAf",      0x4,  1, 12); // LVDS3 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds3:FifoGotEf",      0x4,  1, 13); // LVDS3 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds3:CntDataError",   0x5,  0, 16); // LVDS3 data errors counter

    createStatusParam("Lvds4:DataParseError", 0x6,  8,  0); // Data parser error status
    createStatusParam("Lvds4:DataParseGood",  0x6,  2,  8); // Data parser good status
    createStatusParam("Lvds4:FifoEmpty",      0x6,  1, 10); // LVDS4 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds4:FifoAlmostFull", 0x6,  1, 11); // LVDS4 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds4:FifoGotAf",      0x6,  1, 12); // LVDS4 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds4:FifoGotEf",      0x6,  1, 13); // LVDS4 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds4:CntDataError",   0x7,  0, 16); // LVDS4 data errors counter

    createStatusParam("Lvds5:DataParseError", 0x8,  8,  0); // Data parser error status
    createStatusParam("Lvds5:DataParseGood",  0x8,  2,  8); // Data parser good status
    createStatusParam("Lvds5:FifoEmpty",      0x8,  1, 10); // LVDS5 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds5:FifoAlmostFull", 0x8,  1, 11); // LVDS5 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds5:FifoGotAf",      0x8,  1, 12); // LVDS5 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds5:FifoGotEf",      0x8,  1, 13); // LVDS5 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds5:CntDataError",   0x9,  0, 16); // LVDS5 data errors counter

    createStatusParam("Lvds6:DataParseError", 0xA,  8,  0); // Data parser error status
    createStatusParam("Lvds6:DataParseGood",  0xA,  2,  8); // Data parser good status
    createStatusParam("Lvds6:FifoEmpty",      0xA,  1, 10); // LVDS6 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds6:FifoAlmostFull", 0xA,  1, 11); // LVDS6 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds6:FifoGotAf",      0xA,  1, 12); // LVDS6 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds6:FifoGotEf",      0xA,  1, 13); // LVDS6 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds6:CntDataError",   0xB,  0, 16); // LVDS6 data errors counter

    createStatusParam("Lvds7:DataParseError", 0xC,  8,  0); // Data parser error status
    createStatusParam("Lvds7:DataParseGood",  0xC,  2,  8); // Data parser good status
    createStatusParam("Lvds7:FifoEmpty",      0xC,  1, 10); // LVDS7 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds7:FifoAlmostFull", 0xC,  1, 11); // LVDS7 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds7:FifoGotAf",      0xC,  1, 12); // LVDS7 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds7:FifoGotEf",      0xC,  1, 13); // LVDS7 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds7:CntDataError",   0xD,  0, 16); // LVDS7 data errors counter

    createStatusParam("Lvds8:DataParseError", 0xE,  8,  0); // Data parser error status
    createStatusParam("Lvds8:DataParseGood",  0xE,  2,  8); // Data parser good status
    createStatusParam("Lvds8:FifoEmpty",      0xE,  1, 10); // LVDS8 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds8:FifoAlmostFull", 0xE,  1, 11); // LVDS8 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds8:FifoGotAf",      0xE,  1, 12); // LVDS8 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds8:FifoGotEf",      0xE,  1, 13); // LVDS8 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds8:CntDataError",   0xF,  0, 16); // LVDS8 data errors counter

    createStatusParam("Lvds9:DataParseError",0x10,  8,  0); // Data parser error status
    createStatusParam("Lvds9:DataParseGood", 0x10,  2,  8); // Data parser good status
    createStatusParam("Lvds9:FifoEmpty",     0x10,  1, 10); // LVDS9 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds9:FifoAlmostFull",0x10,  1, 11); // LVDS9 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds9:FifoGotAf",     0x10,  1, 12); // LVDS9 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds9:FifoGotEf",     0x10,  1, 13); // LVDS9 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds9:CntDataError",  0x11,  0, 16); // LVDS9 data errors counter

    createStatusParam("CntTimestampError",   0x12, 16,  0); // Timestamp errors
    createStatusParam("CntFlushing",         0x13,  8,  0); // Flushing counter
    createStatusParam("CntBusy",             0x14, 32,  0); // ACPC busy rejects
    createStatusParam("CntEventsOut",        0x16, 32,  0); // ACPC busy rejects
    createStatusParam("CntEventsIn",         0x18, 32,  0); // ACPC busy rejects

    createStatusParam("ErrCmdBad",           0x1A,  1,  0); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrPktLengthError",   0x1A,  1,  1); // Packet length error          (0=no error,1=error)
    createStatusParam("ErrCfgError",         0x1A,  1,  2); // Configuration error          (0=no error,1=error)
    createStatusParam("Configured",          0x1A,  1,  3); // Configured                   (0=not configured [alarm],1=configured)
    createConfigParam("OutputModeRB",        0x1A,  2,  6); // Output mode                  (0=normal,1=off,2=raw,3=verbose)
    createConfigParam("TcResetModeRB",       0x1A,  1,  8); // Reset mode                   (0=soft, 1=hard)
    createConfigParam("TcTclkModeRB",        0x1A,  1,  9); // Tclk mode                    (0=external, 1=internal)
    createConfigParam("TcTsynModeRB",        0x1A,  1, 10); // TSYNC mode                   (0=external, 1=internal)
    createConfigParam("TcTxenModeRB",        0x1A,  1, 11); // TXen  mode                   (0=external, 1=internal)
    createConfigParam("VerboseStatusRB",     0x1A,  1, 12); // Verbose status               (0=short, 1=long)
    createStatusParam("Acquiring",           0x1A,  1, 13); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("Discovered",          0x1A,  1, 14); // Discovered                   (0=not discovered,1=discovered, archive:monitor)

    createStatusParam("SysrstBLow",          0x1B,  1,  0); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("SysrstBHigh",         0x1B,  1,  1); // SYSRST_B Got HIGH            (0=no,1=yes)
    createStatusParam("TxenBLow",            0x1B,  1,  2); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("TxenBHigh",           0x1B,  1,  3); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("TsyncLow",            0x1B,  1,  4); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TsyncHigh",           0x1B,  1,  5); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("TclkLow",             0x1B,  1,  6); // TCLK Got LOW                 (0=no,1=yes)
    createStatusParam("TclkHigh",            0x1B,  1,  7); // TCLK Got HIGH                (0=no,1=yes)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionIdx",        '1',  0x0, 32,  0,    0); // Chan1 position index

    createConfigParam("PhotosumMin",        'E',  0x0, 32,  0,    0);
    createConfigParam("PhotosumMax",        'E',  0x2, 32,  0,    0);
    createConfigParam("TsyncDelay",         'E',  0x4, 32,  0,    0); // TSYNC delay
    createConfigParam("Ch1:Enable",         'E',  0x6,  1,  0,    0); // Channel 1 Enable              (0=enable, 1=disable)
    createConfigParam("Ch2:Enable",         'E',  0x6,  1,  1,    0); // Channel 2 Enable              (0=enable, 1=disable)
    createConfigParam("Ch3:Enable",         'E',  0x6,  1,  2,    0); // Channel 3 Enable              (0=enable, 1=disable)
    createConfigParam("Ch4:Enable",         'E',  0x6,  1,  3,    0); // Channel 4 Enable              (0=enable, 1=disable)
    createConfigParam("Ch5:Enable",         'E',  0x6,  1,  4,    0); // Channel 5 Enable              (0=enable, 1=disable)
    createConfigParam("Ch6:Enable",         'E',  0x6,  1,  5,    0); // Channel 6 Enable              (0=enable, 1=disable)
    createConfigParam("Ch7:Enable",         'E',  0x6,  1,  6,    0); // Channel 7 Enable              (0=enable, 1=disable)
    createConfigParam("Ch8:Enable",         'E',  0x6,  1,  7,    0); // Channel 8 Enable              (0=enable, 1=disable)
    createConfigParam("Ch9:Enable",         'E',  0x6,  1,  8,    0); // Channel 9 Enable              (0=enable, 1=disable)
    createConfigParam("TimeTolerance",      'E',  0x6,  6,  9,    0);

    createConfigParam("TcResetMode",        'F',  0x0,  1,  0,    0); // Reset mode                    (0=soft, 1=hard)
    createConfigParam("TcTclkMode",         'F',  0x0,  1,  1,    0); // Tclk mode                     (0=external, 1=internal)
    createConfigParam("TcTsynMode",         'F',  0x0,  1,  2,    0); // TSYNC mode                    (0=external, 1=internal)
    createConfigParam("TcTxenMode",         'F',  0x0,  1,  3,    0); // TXen  mode                    (0=external, 1=internal)
    createConfigParam("OutputMode",         'F',  0x0,  2,  4,    0); // Output mode                   (0=normal,1=off,2=raw,2=verbose)
    createConfigParam("VerboseStatus",      'F',  0x0,  1,  6,    1); // Verbose status                (0=short, 1=long)
    createConfigParam("DetectorSelect",     'F',  0x0,  1, 10,    0); // Detector select               (0=SNAP, 1=MANDI,TOPAZ)
    createConfigParam("MSEsamples",         'F',  0x0,  1, 11,    0); // MSE samples                   (0=five, 1=three)
    createConfigParam("TestPatternRate",    'F',  0x0,  2, 12,    0); // Test pattern rate             (0=2700 ev/s, 1=9600 ev/s, 2=13000 ev/s, 3=59000 ev/s)
    createConfigParam("TestPatternAltEn",   'F',  0x0,  1, 14,    0); // Test pattern mode             (0=pos calc, 1=packet)
    createConfigParam("TestPatternEn",      'F',  0x0,  1, 15,    0); // Test pattern enable           (0=disable,1=enable)
}
