/* BnlRocPlugin_v03.cpp
 *
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Greg Guyotte
 */

#include "BnlRocPlugin.h"

void BnlRocPlugin::createStatusParams_v00()
{
//    BLXXX:Det:RocXXX:| sig nam|      | EPICS record description | (bi and mbbi description)
      createStatusParam("DateCode",             0x0, 16,  0);   // EEPROM Date Code
      createStatusParam("OptRxErr",             0x1,  1, 14);   // Optical RX Error
      createStatusParam("OptRxCrcErr",          0x1,  1, 13);   // Optical RX CRC Error
      createStatusParam("ProgErr",              0x1,  1, 12);   // Programming Error
      createStatusParam("ProgTimeoutErr",       0x1,  1, 11);   // Programming Timeout Error
      createStatusParam("PktLenErr",            0x1,  1, 10);   // Packet Length Error
      createStatusParam("UnkCmdErr",            0x1,  1,  9);   // Unknown command Error
      createStatusParam("LvdsCmdErr",           0x1,  1,  8);   // LVDS Command/Data Type Error
      createStatusParam("ErrParity",            0x1,  1,  7);   // LVDS Parity Error
      createStatusParam("TimerErr",             0x1,  1,  6);   // Timer Error
      createStatusParam("EventAcq",             0x1,  1,  5);   // Data Acquisition unit acquiring event
      createStatusParam("Acquiring",            0x1,  1,  4);   // Acquiring Data    (0=not acquiring, 1=acquiring)
      createStatusParam("EEPROMValid",          0x1,  1,  3);   // EEPROM Valid
      createStatusParam("Configured",           0x1,  1,  2);   // Configured
      createStatusParam("Discovered",           0x1,  1,  1);   // Discovered
      createStatusParam("HwIDValid",            0x1,  1,  0);   // Hardware ID valid
      createStatusParam("AOKN",                 0x2,  1, 15);   // Brookhaven AOKN Signal
      createStatusParam("YSAT",                 0x2,  1, 14);   // Got YSAT Signal
      createStatusParam("XSAT",                 0x2,  1, 13);   // Got XSAT Signal
      createStatusParam("Start",                0x2,  1, 12);   // Got a START
      createStatusParam("LVDS",                 0x2,  1, 11);   // Got LVDS Signal
      createStatusParam("OptBActive",           0x2,  1, 10);   // Optical B port is active
      createStatusParam("OptAActive",           0x2,  1,  9);   // Optical A port is active
      createStatusParam("ExtFifoFull",          0x2,  1,  8);   // Got external FIFO full
      createStatusParam("CntFifoFull",          0x2,  1,  7);   // Got Work count FIFO full
      createStatusParam("AcqFifoFull",          0x2,  1,  6);   // Got Acquistion FIFO full
      createStatusParam("CntOverflow",          0x2,  1,  5);   // Got data word count overflow
      createStatusParam("DataOverflow",         0x2,  1,  4);   // Got data overflow
      createStatusParam("ParserStatus",         0x2,  4,  3);   // Parser Status
}

void BnlRocPlugin::createConfigParams_v00()
{
//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
      createConfigParam("PositionIdx",         '1',  0x0,  32,  0,   0);  // Position Index
      createConfigParam("MainThreshold",       '1',  0x2,  16,  0,   0);  // DAC Value for Main Threshold
      createConfigParam("MainPeakThreshold",   '1',  0x3,  16,  0,   0);  // DAC value for Main Peak Threshold
      createConfigParam("X1Scale",             '2',  0x0,  16,  0,   0);  // X1 Scale
      createConfigParam("X1Offset",            '2',  0x1,  16,  0,   0);  // X1 Offset
      createConfigParam("X2Scale",             '2',  0x2,  16,  0,   0);  // X2 Scale    
      createConfigParam("X2Offset",            '2',  0x3,  16,  0,   0);  // X2 Offset
      createConfigParam("X3Scale",             '2',  0x4,  16,  0,   0);  // X3 Scale 
      createConfigParam("X3Offset",            '2',  0x5,  16,  0,   0);  // X3 Offset
      createConfigParam("X4Scale",             '2',  0x6,  16,  0,   0);  // X4 Scale 
      createConfigParam("X4Offset",            '2',  0x7,  16,  0,   0);  // X4 Offset
      createConfigParam("X5Scale",             '2',  0x8,  16,  0,   0);  // X5 Scale 
      createConfigParam("X5Offset",            '2',  0x9,  16,  0,   0);  // X5 Offset
      createConfigParam("X6Scale",             '2',  0xA,  16,  0,   0);  // X6 Scale 
      createConfigParam("X6Offset",            '2',  0xB,  16,  0,   0);  // X6 Offset
      createConfigParam("X7Scale",             '2',  0xC,  16,  0,   0);  // X7 Scale 
      createConfigParam("X7Offset",            '2',  0xD,  16,  0,   0);  // X7 Offset
      createConfigParam("X8Scale",             '2',  0xE,  16,  0,   0);  // X8 Scale 
      createConfigParam("X8Offset",            '2',  0xF,  16,  0,   0);  // X8 Offset
      createConfigParam("X9Scale",             '2', 0x10,  16,  0,   0);  // X9 Scale 
      createConfigParam("X9Offset",            '2', 0x11,  16,  0,   0);  // X9 Offset
      createConfigParam("X10Scale",            '2', 0x12,  16,  0,   0);  // X10 Scale 
      createConfigParam("X10Offset",           '2', 0x13,  16,  0,   0);  // X10 Offset 
      createConfigParam("X11Scale",            '2', 0x14,  16,  0,   0);  // X11 Scale
      createConfigParam("X11Offset",           '2', 0x15,  16,  0,   0);  // X11 Offset
      createConfigParam("X12Scale",            '2', 0x16,  16,  0,   0);  // X12 Scale 
      createConfigParam("X12Offset",           '2', 0x17,  16,  0,   0);  // X12 Offset
      createConfigParam("X13Scale",            '2', 0x18,  16,  0,   0);  // X13 Scale 
      createConfigParam("X13Offset",           '2', 0x19,  16,  0,   0);  // X13 Offset
      createConfigParam("X14Scale",            '2', 0x1A,  16,  0,   0);  // X14 Scale 
      createConfigParam("X14Offset",           '2', 0x1B,  16,  0,   0);  // X14 Offset
      createConfigParam("X15Scale",            '2', 0x1C,  16,  0,   0);  // X15 Scale 
      createConfigParam("X15Offset",           '2', 0x1D,  16,  0,   0);  // X15 Offset
      createConfigParam("X16Scale",            '2', 0x1E,  16,  0,   0);  // X16 Scale 
      createConfigParam("X16Offset",           '2', 0x1F,  16,  0,   0);  // X16 Offset
      createConfigParam("X17Scale",            '2', 0x20,  16,  0,   0);  // X17 Scale 
      createConfigParam("X17Offset",           '2', 0x21,  16,  0,   0);  // X17 Offset
      createConfigParam("X18Scale",            '2', 0x22,  16,  0,   0);  // X18 Scale 
      createConfigParam("X18Offset",           '2', 0x23,  16,  0,   0);  // X18 Offset
      createConfigParam("X19Scale",            '2', 0x24,  16,  0,   0);  // X19 Scale 
      createConfigParam("X19Offset",           '2', 0x25,  16,  0,   0);  // X19 Offset
      createConfigParam("X20Scale",            '2', 0x26,  16,  0,   0);  // X20 Scale 
      createConfigParam("X20Offset",           '2', 0x27,  16,  0,   0);  // X20 Offset
      createConfigParam("Y1Scale",             '3',  0x0,  16,  0,   0);  // Y1 Scale 
      createConfigParam("Y1Offset",            '3',  0x1,  16,  0,   0);  // Y1 Offset
      createConfigParam("Y2Scale",             '3',  0x2,  16,  0,   0);  // Y2 Scale 
      createConfigParam("Y2Offset",            '3',  0x3,  16,  0,   0);  // Y2 Offset
      createConfigParam("Y3Scale",             '3',  0x4,  16,  0,   0);  // Y3 Scale 
      createConfigParam("Y3Offset",            '3',  0x5,  16,  0,   0);  // Y3 Offset
      createConfigParam("Y4Scale",             '3',  0x6,  16,  0,   0);  // Y4 Scale 
      createConfigParam("Y4Offset",            '3',  0x7,  16,  0,   0);  // Y4 Offset
      createConfigParam("Y5Scale",             '3',  0x8,  16,  0,   0);  // Y5 Scale 
      createConfigParam("Y5Offset",            '3',  0x9,  16,  0,   0);  // Y5 Offset
      createConfigParam("Y6Scale",             '3',  0xA,  16,  0,   0);  // Y6 Scale 
      createConfigParam("Y6Offset",            '3',  0xB,  16,  0,   0);  // Y6 Offset
      createConfigParam("Y7Scale",             '3',  0xC,  16,  0,   0);  // Y7 Scale 
      createConfigParam("Y7Offset",            '3',  0xD,  16,  0,   0);  // Y7 Offset
      createConfigParam("Y8Scale",             '3',  0xE,  16,  0,   0);  // Y8 Scale 
      createConfigParam("Y8Offset",            '3',  0xF,  16,  0,   0);  // Y8 Offset
      createConfigParam("Y9Scale",             '3', 0x10,  16,  0,   0);  // Y9 Scale 
      createConfigParam("Y9Offset",            '3', 0x11,  16,  0,   0);  // Y9 Offset
      createConfigParam("Y10Scale",            '3', 0x12,  16,  0,   0);  // Y10 Scale 
      createConfigParam("Y10Offset",           '3', 0x13,  16,  0,   0);  // Y10 Offset
      createConfigParam("Y11Scale",            '3', 0x14,  16,  0,   0);  // Y11 Scale 
      createConfigParam("Y11Offset",           '3', 0x15,  16,  0,   0);  // Y11 Offset
      createConfigParam("Y12Scale",            '3', 0x16,  16,  0,   0);  // Y12 Scale 
      createConfigParam("Y12Offset",           '3', 0x17,  16,  0,   0);  // Y12 Offset
      createConfigParam("Y13Scale",            '3', 0x18,  16,  0,   0);  // Y13 Scale 
      createConfigParam("Y13Offset",           '3', 0x19,  16,  0,   0);  // Y13 Offset
      createConfigParam("Y14Scale",            '3', 0x1A,  16,  0,   0);  // Y14 Scale 
      createConfigParam("Y14Offset",           '3', 0x1B,  16,  0,   0);  // Y14 Offset
      createConfigParam("Y15Scale",            '3', 0x1C,  16,  0,   0);  // Y15 Scale 
      createConfigParam("Y15Offset",           '3', 0x1D,  16,  0,   0);  // Y15 Offset
      createConfigParam("Y16Scale",            '3', 0x1E,  16,  0,   0);  // Y16 Scale 
      createConfigParam("Y16Offset",           '3', 0x1F,  16,  0,   0);  // Y16 Offset
      createConfigParam("Y17Scale",            '3', 0x20,  16,  0,   0);  // Y17 Scale 
      createConfigParam("Y17Offset",           '3', 0x21,  16,  0,   0);  // Y17 Offset
      createConfigParam("LowerTimeVeto",       'E',  0x0,  32,  0,   0);  // Lower Time Veto
      createConfigParam("UpperTimeVeto",       'E',  0x2,  32,  0,   0);  // Upper Time Veto
      createConfigParam("PosLatchValue",       'E',  0x4,  16,  0,   0);  // Position Latch Value
      createConfigParam("CentroidMin",         'E',  0x5,  16,  0,   0);  // Centroid minimum
      createConfigParam("XCentroidScale",      'E',  0x6,  16,  0,   0);  // X Centroid Scale
      createConfigParam("YCentroidScale",      'E',  0x7,  16,  0,   0);  // Y Centroid Scale
      createConfigParam("HighResMode",         'F',  0x0,   1, 14,   0);  // High Resolution Mode (0=normal,1=highres)
      createConfigParam("CathodePulseEn",      'F',  0x0,   1, 13,   0);  // Cathode Test pulse Enable (0=off,1=on)
      createConfigParam("AnodePulseEn",        'F',  0x0,   1, 12,   0);  // Anode Test pulse Enable (0=off,1=on)
      createConfigParam("Gain",                'F',  0x0,   2, 10,   0);  // Gain Setting (0=4X,1=8X,2=12X,3=16X)
      createConfigParam("IgnoreVeto",          'F',  0x0,   1,  9,   0);  // Ignore Veto (0=False,1=True) 
      createConfigParam("IgnoreMultStarts",    'F',  0x0,   1,  8,   0);  // Ignore Multiple Starts (0=False,1=True)
      createConfigParam("TxEnable",            'F',  0x0,   1,  7,   0);  // TXEN internal
      createConfigParam("OutputMode",          'F',  0x0,   2,  5,   0);  // Output Format (0=Normal,1=Raw,2=Ext)
      createConfigParam("TsyncSelect",         'F',  0x0,   1,  4,   0);  // TSYNC interal
      createConfigParam("TclkSelect",          'F',  0x0,   1,  3,   0);  // TCLK internal
      createConfigParam("Reset",               'F',  0x0,   1,  2,   0);  // RESET external
      createConfigParam("AcquireMode",         'F',  0x0,   2,  0,   2);  // Acquisition Mode (0=idle,1=fakedata,2=normal,3=trigger)
}
