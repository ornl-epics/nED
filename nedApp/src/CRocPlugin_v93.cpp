/* CRocPlugin_v93.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "CRocPlugin.h"

/**
 * @file CRocPlugin_v93.cpp
 *
 * CROC 9.3 parameters
 */

void CRocPlugin::createParams_v93()
{
//    BLXXX:Det:RocXXX:| sig nam|                              | EPICS record description | (bi and mbbi description)
    createStatusParam("Y1:Discrim",           0x0,  1,  0); // Discriminator hit on Y1      (0=no hit,1=hit)
    createStatusParam("Y2:Discrim",           0x0,  1,  1); // Discriminator hit on Y2      (0=no hit,1=hit)
    createStatusParam("Y3:Discrim",           0x0,  1,  2); // Discriminator hit on Y3      (0=no hit,1=hit)
    createStatusParam("Y4:Discrim",           0x0,  1,  3); // Discriminator hit on Y4      (0=no hit,1=hit)
    createStatusParam("Y5:Discrim",           0x0,  1,  4); // Discriminator hit on Y5      (0=no hit,1=hit)
    createStatusParam("Y6:Discrim",           0x0,  1,  5); // Discriminator hit on Y6      (0=no hit,1=hit)
    createStatusParam("Y7:Discrim",           0x0,  1,  6); // Discriminator hit on Y7      (0=no hit,1=hit)
    createStatusParam("X1:Discrim",           0x0,  1,  7); // Discriminator hit on X1      (0=no hit,1=hit)
    createStatusParam("X2:Discrim",           0x0,  1,  8); // Discriminator hit on X2      (0=no hit,1=hit)
    createStatusParam("X3:Discrim",           0x0,  1,  9); // Discriminator hit on X3      (0=no hit,1=hit)
    createStatusParam("X4:Discrim",           0x0,  1, 10); // Discriminator hit on X4      (0=no hit,1=hit)
    createStatusParam("X5:Discrim",           0x0,  1, 11); // Discriminator hit on X5      (0=no hit,1=hit)
    createStatusParam("X6:Discrim",           0x0,  1, 12); // Discriminator hit on X6      (0=no hit,1=hit)
    createStatusParam("X7:Discrim",           0x0,  1, 13); // Discriminator hit on X7      (0=no hit,1=hit)
    createStatusParam("X8:Discrim",           0x0,  1, 14); // Discriminator hit on X8      (0=no hit,1=hit)
    createStatusParam("X9:Discrim",           0x0,  1, 15); // Discriminator hit on X9      (0=no hit,1=hit)
    createStatusParam("X10:Discrim",          0x1,  1,  0); // Discriminator hit on X10     (0=no hit,1=hit)
    createStatusParam("X11:Discrim",          0x1,  1,  1); // Discriminator hit on X11     (0=no hit,1=hit)
    createStatusParam("G1:Discrim",           0x1,  1,  2); // Discriminator hit on G1      (0=no hit,1=hit)
    createStatusParam("G2:Discrim",           0x1,  1,  3); // Discriminator hit on G2      (0=no hit,1=hit)
    createStatusParam("G3:Discrim",           0x1,  1,  4); // Discriminator hit on G3      (0=no hit,1=hit)
    createStatusParam("G4:Discrim",           0x1,  1,  5); // Discriminator hit on G4      (0=no hit,1=hit)
    createStatusParam("G5:Discrim",           0x1,  1,  6); // Discriminator hit on G5      (0=no hit,1=hit)
    createStatusParam("G6:Discrim",           0x1,  1,  7); // Discriminator hit on G6      (0=no hit,1=hit)
    createStatusParam("G7:Discrim",           0x1,  1,  8); // Discriminator hit on G7      (0=no hit,1=hit)
    createStatusParam("G8:Discrim",           0x1,  1,  9); // Discriminator hit on G8      (0=no hit,1=hit)
    createStatusParam("G9:Discrim",           0x1,  1, 10); // Discriminator hit on G9      (0=no hit,1=hit)
    createStatusParam("G10:Discrim",          0x1,  1, 11); // Discriminator hit on G10     (0=no hit,1=hit)
    createStatusParam("G11:Discrim",          0x1,  1, 12); // Discriminator hit on G11     (0=no hit,1=hit)
    createStatusParam("G12:Discrim",          0x1,  1, 13); // Discriminator hit on G12     (0=no hit,1=hit)
    createStatusParam("G13:Discrim",          0x1,  1, 14); // Discriminator hit on G13     (0=no hit,1=hit)
    createStatusParam("G14:Discrim",          0x1,  1, 15); // Discriminator hit on G14     (0=no hit,1=hit)
    createStatusParam("TimeRange1DeadTime",   0x2,  1,  0); // TimeRange1 dead time cond    (0=no,1=yes)
    createStatusParam("TimeRange2DeadTime",   0x2,  1,  1); // TimeRange2 dead time cond    (0=no,1=yes)
    createStatusParam("TimeRange3DeadTime",   0x2,  1,  2); // TimeRange3 dead time cond    (0=no,1=yes)
    createStatusParam("TimeRange4DeadTime",   0x2,  1,  3); // TimeRange4 dead time cond    (0=no,1=yes)
    createStatusParam("SavedEvent",           0x2,  1,  4); // Saved event                  (0=no,1=yes)
    createStatusParam("EvFifoAlmFull",        0x2,  1,  5); // Event FIFO almost full       (0=not full,1=full)
    createStatusParam("EvFifoHasData",        0x2,  1,  6); // Event FIFO has data          (0=no data,1=has data)
    createStatusParam("InactiveCond",         0x2,  1,  7); // Got inactive condition       (0=no,1=yes)
    createStatusParam("Idle",                 0x2,  1,  8); // Reached idle state           (0=no,1=yes)
    createStatusParam("Done",                 0x2,  1,  9); // Reached done state           (0=no,1=yes)
    createStatusParam("VetoWhileTimecnt",     0x2,  1, 10); // Got veto while timecnt check (0=no,1=yes)
    createStatusParam("VetoWhileYcalc",       0x2,  1, 11); // Got veto while Y calculation (0=no,1=yes)
    createStatusParam("VetoWhileXcalc",       0x2,  1, 12); // Got veto while X calculation (0=no,1=yes)
    createStatusParam("VetoWhileGcalc",       0x2,  1, 13); // Got veto while G calculation (0=no,1=yes)
    createStatusParam("ChFifoEmpty",          0x2,  1, 14); // Channel FIFO empty           (0=not empty,1=empty)
    createStatusParam("ChFifoAlmFull",        0x2,  1, 15); // Channel FIFO Almost full     (0=not full,1=full)
    createStatusParam("Discovered",           0x3,  1,  0); // Discovered                   (0=not discovered,1=discovered, archive:monitor)
    createStatusParam("Configured",           0x3,  1,  1); // Configured                   (0=not configured [alarm],1=configured, archive:monitor)
    createStatusParam("Acquiring",            0x3,  1,  2); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("ErrParity",            0x3,  1,  3); // LVDS parity error.           (0=no error,1=error)
    createStatusParam("ErrType",              0x3,  1,  4); // LVDS packet type error.      (0=no error,1=error)
    createStatusParam("ErrLength",            0x3,  1,  5); // LVDS packet length error.    (0=no error,1=error)
    createStatusParam("ErrTimeout",           0x3,  1,  6); // LVDS packet timeout.         (0=no timeout,1=timeout)
    createStatusParam("ErrNoStart",           0x3,  1,  7); // LVDS data without start.     (0=no error,1=error)
    createStatusParam("ErrPreStart",          0x3,  1,  8); // LVDS start before stop bit   (0=no error,1=error)
    createStatusParam("ErrFifoFull",          0x3,  1,  9); // LVDS FIFO went full.         (0=not full,1=full)
    createStatusParam("ErrNoTsync",           0x3,  1, 11); // Timestamp overflow error.    (0=no error,1=error)
    createStatusParam("ErrCmdBad",            0x3,  1, 12); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("ErrCmdLength",         0x3,  1, 13); // Command length error         (0=no error,1=error)
    createStatusParam("ErrProgramming",       0x3,  1, 14); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("OutFifoAlmostFull",    0x3,  1, 15); // Output FIFO almost full      (0=not full,1=full)
    createStatusParam("TclkLow",              0x4,  1,  0); // TCLK Got LOW                 (0=no,1=yes)
    createStatusParam("TclkHigh",             0x4,  1,  1); // TCLK Got HIGH                (0=no,1=yes)
    createStatusParam("TsyncLow",             0x4,  1,  2); // TSYNC Got LOW                (0=no,1=yes)
    createStatusParam("TsyncHigh",            0x4,  1,  3); // TSYNC Got HIGH               (0=no,1=yes)
    createStatusParam("TxenBLow",             0x4,  1,  4); // TXEN_B Got LOW               (0=no,1=yes)
    createStatusParam("TxenBHigh",            0x4,  1,  5); // TXEN_B Got HIGH              (0=no,1=yes)
    createStatusParam("SysrstBLow",           0x4,  1,  6); // SYSRST_B Got LOW             (0=no,1=yes)
    createStatusParam("SysrstBHigh",          0x4,  1,  7); // SYSRST_B Got HIGH            (0=no,1=yes)
    createStatusParam("LvdsVerify",           0x4,  1,  8); // Detected LVDS verify         (0=no,1=yes)


//    BLXXX:Det:RocXXX:| sig nam |                                     | EPICS record description  | (bi and mbbi description)
    createConfigParam("PositionId",       '1',  0x0, 32, 0, 0);     // Position index
    createConfigParam("TsyncDelay",       'D',  0x0, 32, 0, 0);     // TSYNC delay                   (scale:100,unit:ns)
    createConfigParam("TimeVetoLow",      'E',  0x0, 32, 0, 0);     // Timestamp veto low            (scale:100,unit:ns)
    createConfigParam("TimeVetoHigh",     'E',  0x2, 32, 0, 333333);// Timestamp veto high           (scale:100,unit:ns)
    createConfigParam("FakeTrigDelay",    'E',  0x4, 16, 0, 20000); // Fake trigger delay
    createConfigParam("PulseToPulseTime", 'E',  0x5, 16, 0, 0);     // Pulse-to-pulse time
    createConfigParam("TimeRange1",       'E',  0x6, 16, 0, 0);     // Time range width #1
    createConfigParam("TimeRange2",       'E',  0x7, 16, 0, 0);     // Time range width #2
    createConfigParam("TimeRange3",       'E',  0x8, 16, 0, 0);     // Time range width #3
    createConfigParam("TimeRange4",       'E',  0x9, 16, 0, 0);     // Time range width #4
    createConfigParam("Tp17Select",       'E',  0xA,  8, 0, 0);     // TP17 select
    createConfigParam("Tp18Select",       'E',  0xA,  8, 8, 0);     // TP18 select
    createConfigParam("DeadTime",         'E',  0xB, 16, 0, 0);     // Dead time
    createConfigParam("TimeRangeMin",     'E',  0xC, 13, 0, 0);     // Time range count min nmin[0]
    createConfigParam("TimeRangeMinCnt",  'E',  0xC,  2,13, 0);     // Minimum number of valid time
    createConfigParam("YMin",             'E',  0xD,  8, 0, 0);     // Y minimum nmin[1]
    createConfigParam("XMin",             'E',  0xD,  8, 8, 0);     // X minimum nmin[2]
    createConfigParam("Gcnt",             'E',  0xE,  8, 0, 0);     // G minimum nmin[3]
    createConfigParam("XCntMin",          'E',  0xE,  4, 8, 0);     // X above nmin[2] - nmin[4]
    createConfigParam("GMin",             'E',  0xE,  4,12, 0);     // nongap_maxcnt_dratio
    createConfigParam("PrsInfo0",         'E',  0xF, 16, 0, 0);     // prs_info[47:32]
    createConfigParam("PrsInfo1",         'E', 0x10, 16, 0, 0);     // prs_info[63:48]
    createConfigParam("PrsInfo2",         'E', 0x11, 16, 0, 0);     // prs_info[79:64]
    createConfigParam("TcResetMode",      'F',  0x0,  1, 0, 0);     // T&C Reset mode                (0=internal,1=external)
    createConfigParam("TcTclkMode",       'F',  0x0,  1, 1, 0);     // T&C TCLK mode                 (0=external,1=internal 10MHz)
    createConfigParam("TcTsyncMode",      'F',  0x0,  1, 2, 0);     // T&C TSYNC mode                (0=external,1=internal 60Hz)
    createConfigParam("TcTxEnMode",       'F',  0x0,  1, 3, 1);     // T&C TX enable mode            (0=external,1=internal)
    createConfigParam("AcquireMode",      'F',  0x0,  2, 4, 2);     // Acquire mode                  (0=idle,1=idle,2=normal,3=trigger)
    createConfigParam("OutputMode",       'F',  0x0,  2, 6, 0);     // Output mode                   (0=normal,1=raw [alarm],2=extended [alarm])
    createConfigParam("EdgeMode",         'F',  0x0,  1, 8, 0);     // Edge mode                     (0=low,1=falling edge)
    createConfigParam("CoincidenceMode",  'F',  0x0,  2, 9, 0);     // Coincidence mode              (0=multiple hit,1=sections,2=both)
    createConfigParam("LvdsRate",         'F',  0x0,  1,15, 0);     // LVDS output rate              (0=40Mhz,1=20Mhz)

    // Meta-parameters needed for calculation but not available in firmware
    createMetaConfigParam("XCntMax",                  8,    0);     // Num of X above threshold
    createMetaConfigParam("YCntMax",                  8,    0);     // Num of Y above threshold
    createMetaConfigParam("FiberCoding",              8,    0);     // Type of mapping algorithm     (0=v2,1=v3)
    createMetaConfigParam("GGapMin1",                 8,    0);     // Num of X above threshold
    createMetaConfigParam("GGapMin2",                 8,    0);     // Num of X above threshold
    createMetaConfigParam("G1GapMin",                 8,    0);     // Gap+1 G threshold
    createMetaConfigParam("X1GapMin",                 8,    0);     // Gap+1 X threshold
    createMetaConfigParam("GMin2",                    8,    0);     // Num of X above threshold

    createMetaConfigParam("GNoiseThreshold",         32,    8);     // G noise threshold
    createMetaConfigParam("XNoiseThreshold",         32,    8);     // X noise threshold
    createMetaConfigParam("YNoiseThreshold",         32,    8);     // Y noise threshold

    createMetaConfigParam("GWeight0",                 8,    1);     // Weight for max G found
    createMetaConfigParam("GWeight1",                 8,    3);     // Weight for G +-1 away
    createMetaConfigParam("GWeight2",                 8,    5);     // Weight for G +-2 away
    createMetaConfigParam("GWeight3",                 8,   10);     // Weight for G +-3 away
    createMetaConfigParam("GWeight4",                 8,   10);     // Weight for G +-4 away
    createMetaConfigParam("GWeight5",                 8,   10);     // Weight for G +-5 away
    createMetaConfigParam("GWeight6",                 8,   10);     // Weight for G +-6 away
    createMetaConfigParam("GWeight7",                 8,   10);     // Weight for G +-7 away
    createMetaConfigParam("GWeight8",                 8,   10);     // Weight for G +-8 away
    createMetaConfigParam("GWeight9",                 8,   10);     // Weight for G +-9 away
    createMetaConfigParam("GWeight10",                8,   10);     // Weight for G +-10 away
    createMetaConfigParam("GWeight11",                8,   10);     // Weight for G +-11 away
    createMetaConfigParam("GWeight12",                8,   10);     // Weight for G +-12 away
    createMetaConfigParam("GWeight13",                8,   10);     // Weight for G +-13 away
    createMetaConfigParam("XWeight0",                 8,    2);     // Weight for max X found
    createMetaConfigParam("XWeight1",                 8,    3);     // Weight for X +-1 away
    createMetaConfigParam("XWeight2",                 8,    5);     // Weight for X +-2 away
    createMetaConfigParam("XWeight3",                 8,   10);     // Weight for X +-3 away
    createMetaConfigParam("XWeight4",                 8,   10);     // Weight for X +-4 away
    createMetaConfigParam("XWeight5",                 8,   10);     // Weight for X +-5 away
    createMetaConfigParam("XWeight6",                 8,   10);     // Weight for X +-6 away
    createMetaConfigParam("XWeight7",                 8,   10);     // Weight for X +-7 away
    createMetaConfigParam("XWeight8",                 8,    5);     // Weight for X +-8 away
    createMetaConfigParam("XWeight9",                 8,    3);     // Weight for X +-9 away
    createMetaConfigParam("XWeight10",                8,    2);     // Weight for X +-10 away
    createMetaConfigParam("YWeight0",                 8,    1);     // Weight for max Y found
    createMetaConfigParam("YWeight1",                 8,    5);     // Weight for Y +-1 away
    createMetaConfigParam("YWeight2",                 8,   10);     // Weight for Y +-2 away
    createMetaConfigParam("YWeight3",                 8,   10);     // Weight for Y +-3 away
    createMetaConfigParam("YWeight4",                 8,   10);     // Weight for Y +-4 away
    createMetaConfigParam("YWeight5",                 8,   10);     // Weight for Y +-5 away
    createMetaConfigParam("YWeight6",                 8,   10);     // Weight for Y +-6 away
}
