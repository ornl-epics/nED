/* AcpcPlugin_v174.cpp
 *
 * Copyright (c) 2016 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcPlugin.h"

/**
 * @file AcpcPlugin_v174.cpp
 *
 * ACPC 17.04 parameters
 *
 * ACPC 17.04 is compatible with DSP7.
 */

void AcpcPlugin::createParams_v174()
{
    createRegParam("VERSION", "HwRev",  true, 0,  8,  0); // Hardware revision
    createRegParam("VERSION", "HwVer",  true, 0,  8,  8); // Hardware version
    createRegParam("VERSION", "HwYear", true, 1,  8,  0, 0, CONV_HEX2DEC); // Hardware year     (offset:2000)
    createRegParam("VERSION", "HwDay",  true, 2,  8,  0, 0, CONV_HEX2DEC); // Hardware day
    createRegParam("VERSION", "HwMonth",true, 2,  8,  8, 0, CONV_HEX2DEC); // Hardware month
    createRegParam("VERSION", "FwRev",  true, 3,  8,  0); // Firmware revision                  (low:3,high:5)
    createRegParam("VERSION", "FwVer",  true, 3,  8,  8); // Firmware version                   (low:16,high:18)
    createRegParam("VERSION", "FwYear", true, 4,  8,  0, 0, CONV_HEX2DEC); // Firmware year     (offset:2000)
    createRegParam("VERSION", "FwDay",  true, 5,  8,  0, 0, CONV_HEX2DEC); // Firmware day
    createRegParam("VERSION", "FwMonth",true, 5,  8,  8, 0, CONV_HEX2DEC); // Firmware month
    createRegParam("VERSION", "Eeprom1",true, 6, 32,  0);
    createRegParam("VERSION", "Eeprom2",true, 8, 32,  8);

    createStatusParam("ErrCmdBad",            0x0,  1,  0); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrPktLengthError",    0x0,  1,  1); // Packet length error          (0=no error,1=error)
    createStatusParam("ErrCfgError",          0x0,  1,  2); // Configuration error          (0=no error,1=error)
    createStatusParam("Configured",           0x0,  1,  3); // Configured                   (0=not configured [alarm],1=configured)
    createStatusParam("OutputModeRB",         0x0,  2,  6); // Output mode                  (0=normal,1=off,2=raw,3=verbose)
    createStatusParam("TcResetModeRB",        0x0,  1,  8); // Reset mode                   (0=soft, 1=hard)
    createStatusParam("TcTclkModeRB",         0x0,  1,  9); // Tclk mode                    (0=external, 1=internal)
    createStatusParam("TcTsynModeRB",         0x0,  1, 10); // TSYNC mode                   (0=external, 1=internal)
    createStatusParam("TcTxenModeRB",         0x0,  1, 11); // TXen  mode                   (0=external, 1=internal)
    createStatusParam("DataFormatRB",         0x0,  1, 12); // New format                   (0=legacy, 1=new)
    createStatusParam("Acquiring",            0x0,  1, 13); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("Discovered",           0x0,  1, 14); // Discovered                   (0=not discovered,1=discovered, archive:monitor)

    createStatusParam("TclkLow",              0x1,  1,  0); // TCLK Got LOW                  (0=no,1=yes)
    createStatusParam("TclkHigh",             0x1,  1,  1); // TCLK Got HIGH                 (0=no,1=yes)
    createStatusParam("TsyncLow",             0x1,  1,  2); // TSYNC Got LOW                 (0=no,1=yes)
    createStatusParam("TsyncHigh",            0x1,  1,  3); // TSYNC Got HIGH                (0=no,1=yes)
    createStatusParam("TxenBLow",             0x1,  1,  4); // TXEN_B Got LOW                (0=no,1=yes)
    createStatusParam("TxenBHigh",            0x1,  1,  5); // TXEN_B Got HIGH               (0=no,1=yes)
    createStatusParam("AROCsMissing",         0x1,  9,  6); // AROCs missing

    createStatusParam("Lvds1:DataParseError", 0x2,  8,  0); // Data parser error status
    createStatusParam("Lvds1:DataParseGood",  0x2,  2,  8); // Data parser good status
    createStatusParam("Lvds1:FifoEmpty",      0x2,  1, 10); // LVDS1 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds1:FifoAlmostFull", 0x2,  1, 11); // LVDS1 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds1:FifoGotAf",      0x2,  1, 12); // LVDS1 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds1:FifoGotEf",      0x2,  1, 13); // LVDS1 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds1:CntDataError",   0x3, 16,  0); // LVDS1 data errors counter

    createStatusParam("Lvds2:DataParseError", 0x4,  8,  0); // Data parser error status
    createStatusParam("Lvds2:DataParseGood",  0x4,  2,  8); // Data parser good status
    createStatusParam("Lvds2:FifoEmpty",      0x4,  1, 10); // LVDS2 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds2:FifoAlmostFull", 0x4,  1, 11); // LVDS2 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds2:FifoGotAf",      0x4,  1, 12); // LVDS2 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds2:FifoGotEf",      0x4,  1, 13); // LVDS2 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds2:CntDataError",   0x5,  16,  0); // LVDS2 data errors counter

    createStatusParam("Lvds3:DataParseError", 0x6,  8,  0); // Data parser error status
    createStatusParam("Lvds3:DataParseGood",  0x6,  2,  8); // Data parser good status
    createStatusParam("Lvds3:FifoEmpty",      0x6,  1, 10); // LVDS3 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds3:FifoAlmostFull", 0x6,  1, 11); // LVDS3 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds3:FifoGotAf",      0x6,  1, 12); // LVDS3 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds3:FifoGotEf",      0x6,  1, 13); // LVDS3 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds3:CntDataError",   0x7, 16,  0); // LVDS3 data errors counter

    createStatusParam("Lvds4:DataParseError", 0x8,  8,  0); // Data parser error status
    createStatusParam("Lvds4:DataParseGood",  0x8,  2,  8); // Data parser good status
    createStatusParam("Lvds4:FifoEmpty",      0x8,  1, 10); // LVDS4 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds4:FifoAlmostFull", 0x8,  1, 11); // LVDS4 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds4:FifoGotAf",      0x8,  1, 12); // LVDS4 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds4:FifoGotEf",      0x8,  1, 13); // LVDS4 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds4:CntDataError",   0x9, 16,  0); // LVDS4 data errors counter

    createStatusParam("Lvds5:DataParseError", 0xA,  8,  0); // Data parser error status
    createStatusParam("Lvds5:DataParseGood",  0xA,  2,  8); // Data parser good status
    createStatusParam("Lvds5:FifoEmpty",      0xA,  1, 10); // LVDS5 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds5:FifoAlmostFull", 0xA,  1, 11); // LVDS5 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds5:FifoGotAf",      0xA,  1, 12); // LVDS5 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds5:FifoGotEf",      0xA,  1, 13); // LVDS5 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds5:CntDataError",   0xB, 16,  0); // LVDS5 data errors counter

    createStatusParam("Lvds6:DataParseError", 0xC,  8,  0); // Data parser error status
    createStatusParam("Lvds6:DataParseGood",  0xC,  2,  8); // Data parser good status
    createStatusParam("Lvds6:FifoEmpty",      0xC,  1, 10); // LVDS6 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds6:FifoAlmostFull", 0xC,  1, 11); // LVDS6 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds6:FifoGotAf",      0xC,  1, 12); // LVDS6 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds6:FifoGotEf",      0xC,  1, 13); // LVDS6 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds6:CntDataError",   0xD, 16,  0); // LVDS6 data errors counter

    createStatusParam("Lvds7:DataParseError", 0xE,  8,  0); // Data parser error status
    createStatusParam("Lvds7:DataParseGood",  0xE,  2,  8); // Data parser good status
    createStatusParam("Lvds7:FifoEmpty",      0xE,  1, 10); // LVDS7 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds7:FifoAlmostFull", 0xE,  1, 11); // LVDS7 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds7:FifoGotAf",      0xE,  1, 12); // LVDS7 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds7:FifoGotEf",      0xE,  1, 13); // LVDS7 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds7:CntDataError",   0xF, 16,  0); // LVDS7 data errors counter

    createStatusParam("Lvds8:DataParseError",0x10,  8,  0); // Data parser error status
    createStatusParam("Lvds8:DataParseGood", 0x10,  2,  8); // Data parser good status
    createStatusParam("Lvds8:FifoEmpty",     0x10,  1, 10); // LVDS8 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds8:FifoAlmostFull",0x10,  1, 11); // LVDS8 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds8:FifoGotAf",     0x10,  1, 12); // LVDS8 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds8:FifoGotEf",     0x10,  1, 13); // LVDS8 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds8:CntDataError",  0x11, 16,  0); // LVDS8 data errors counter

    createStatusParam("Lvds9:DataParseError",0x12,  8,  0); // Data parser error status
    createStatusParam("Lvds9:DataParseGood", 0x12,  2,  8); // Data parser good status
    createStatusParam("Lvds9:FifoEmpty",     0x12,  1, 10); // LVDS9 FIFO empty             (0=empty, 1=not empty)
    createStatusParam("Lvds9:FifoAlmostFull",0x12,  1, 11); // LVDS9 FIFO almost full       (0=not full, 1=almost full)
    createStatusParam("Lvds9:FifoGotAf",     0x12,  1, 12); // LVDS9 FIFO got almost full   (0=not full, 1=almost full)
    createStatusParam("Lvds9:FifoGotEf",     0x12,  1, 13); // LVDS9 FIFO got empty         (0=not full, 1=almost full)
    createStatusParam("Lvds9:CntDataError",  0x13, 16,  0); // LVDS9 data errors counter

    createCounterParam("RateTimestampError",  0x0, 16,  0); // Timestamp errors             (unit:cnts/s)
    createCounterParam("RateFlushing",        0x1,  8,  0); // Flushing counter             (unit:cnts/s)
    createCounterParam("RatePhotosumOver",    0x2, 16,  0); // PhotoSum over high threshold (unit:cnts/s)
    createCounterParam("RatePhotosumUnder",   0x3, 16,  0); // PhotoSum under low threshold (unit:cnts/s)
    createCounterParam("RateBusy",            0x4, 16,  0); // ACPC busy rejects            (unit:cnts/s)
    createCounterParam("RateEventsOut",       0x5, 16,  0); // Total events out             (unit:cnts/s)
    createCounterParam("RateEventsIn",        0x6, 32,  0); // Total events in              (unit:cnts/s)

//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionIdx",        '1',  0x0, 32,  0,    0); // Chan1 position index
    createConfigParam("PhotosumMin",        '1',  0x2, 32,  0,  600); // Low PhotoSum threshold        (scale:0.0009765625)
    createConfigParam("PhotosumMax",        '1',  0x4, 32,  0, 2700); // High PhotoSum threshold       (scale:0.0009765625)
    createConfigParam("TsyncDelay",         '1',  0x6, 32,  0,    0); // TSYNC delay
    createConfigParam("Lvds1:En",           '1',  0x8,  1,  0,    0); // Channel 1 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds2:En",           '1',  0x8,  1,  1,    0); // Channel 2 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds3:En",           '1',  0x8,  1,  2,    0); // Channel 3 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds4:En",           '1',  0x8,  1,  3,    0); // Channel 4 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds5:En",           '1',  0x8,  1,  4,    0); // Channel 5 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds6:En",           '1',  0x8,  1,  5,    0); // Channel 6 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds7:En",           '1',  0x8,  1,  6,    0); // Channel 7 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds8:En",           '1',  0x8,  1,  7,    0); // Channel 8 Enable              (0=enable, 1=disable)
    createConfigParam("Lvds9:En",           '1',  0x8,  1,  8,    0); // Channel 9 Enable              (0=enable, 1=disable)
    createConfigParam("TimestampTolerance", '1',  0x8,  7,  9,   31); // Timestamp tolerance
    createConfigParam("TcResetMode",        '1',  0x9,  1,  0,    0); // Reset mode                    (0=soft, 1=hard)
    createConfigParam("TcTclkMode",         '1',  0x9,  1,  1,    0); // Tclk mode                     (0=external, 1=internal)
    createConfigParam("TcTsyncMode",        '1',  0x9,  1,  2,    0); // TSYNC mode                    (0=external, 1=internal)
    createConfigParam("TcTxenMode",         '1',  0x9,  1,  3,    0); // TXen  mode                    (0=external, 1=internal)
    createConfigParam("OutputMode",         '1',  0x9,  2,  4,    0); // Output mode                   (0=normal,1=passthrough [alarm],2=unused,3=verbose [alarm])
    createConfigParam("DataFormat",         '1',  0x9,  1,  6,    0); // Data format select            (0=legacy, 1=new)
    createConfigParam("LvdsTestEn",         '1',  0x9,  1,  7,    0); // LVDS test pattern enable      (0=disable, 1=enable)
    createConfigParam("DetectorSelect",     '1',  0x9,  1, 10,    0); // Detector select               (0=MANDI/TOPAZ, 1=SNAP)
    createConfigParam("TestPatternEn",      '1',  0x9,  1, 15,    0); // Test pattern enable           (0=disable,1=enable)
    createConfigParam("TestPatternMode",    '1',  0x9,  1, 14,    0); // Test pattern mode             (0=pos calc, 1=packet)
    createConfigParam("TestPatternRate",    '1',  0x9,  2, 11,    0); // Test pattern rate             (0=2700 ev/s, 1=9600 ev/s, 2=13000 ev/s, 3=59000 ev/s)
    createConfigParam("DataFormatId",       '1',  0xA,  8,  0,    5); // Data format identifier        (5=normal,6=verbose)
}
