/* DspPlugin_v71.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Miljko Bobrek
 */

#include "DspPlugin.h"

void DspPlugin::createParams_v71() {
//      BLXXX:Det:DspX:| sig nam|         '1' addr  size off default      | EPICS record description | (bi and mbbi description)
    createConfigParam("Meta0:Type",       '1', 0x0,   4,  0, 0); // Channel 0 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta0:Id",         '1', 0x0,   4,  4, 0); // Channel 0 id
    createConfigParam("Meta1:Type",       '1', 0x0,   4,  8, 0); // Channel 1 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta1:Id",         '1', 0x0,   4, 12, 1); // Channel 1 id
    createConfigParam("Meta2:Type",       '1', 0x0,   4, 16, 0); // Channel 2 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta2:Id",         '1', 0x0,   4, 20, 2); // Channel 2 id
    createConfigParam("Meta3:Type",       '1', 0x0,   4, 24, 0); // Channel 3 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta3:Id",         '1', 0x0,   4, 28, 3); // Channel 3 id
    createConfigParam("Meta4:Type",       '1', 0x1,   4,  0, 4); // Channel 4 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta4:Id",         '1', 0x1,   4,  4, 4); // Channel 4 id
    createConfigParam("Meta5:Type",       '1', 0x1,   4,  8, 4); // Channel 5 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta5:Id",         '1', 0x1,   4, 12, 5); // Channel 5 id
    createConfigParam("Meta6:Type",       '1', 0x1,   4, 16, 4); // Channel 6 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta6:Id",         '1', 0x1,   4, 20, 6); // Channel 6 id
    createConfigParam("Meta7:Type",       '1', 0x1,   4, 24, 4); // Channel 7 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta7:Id",         '1', 0x1,   4, 28, 7); // Channel 7 id
    createConfigParam("Meta8:Type",       '1', 0x2,   4,  0, 6); // Channel 8 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta8:Id",         '1', 0x2,   4,  4, 8); // Channel 8 id
    createConfigParam("Meta9:Type",       '1', 0x2,   4,  8, 6); // Channel 9 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta9:Id",         '1', 0x2,   4, 12, 9); // Channel 9 id
    createConfigParam("Meta10:Type",      '1', 0x2,   4, 16, 6); // Channel 10 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta10:Id",        '1', 0x2,   4, 20,10); // Channel 10 id
    createConfigParam("Meta11:Type",      '1', 0x2,   4, 24, 6); // Channel 11 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta11:Id",        '1', 0x2,   4, 28,11); // Channel 11 id
    createConfigParam("Meta12:Type",      '1', 0x3,   4,  0, 7); // Channel 12 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta12:Id",        '1', 0x3,   4,  4,12); // Channel 12 id
    createConfigParam("Meta13:Type",      '1', 0x3,   4,  8, 7); // Channel 13 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta13:Id",        '1', 0x3,   4, 12,13); // Channel 13 id
    createConfigParam("Meta14:Type",      '1', 0x3,   4, 16, 7); // Channel 14 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta14:Id",        '1', 0x3,   4, 20,14); // Channel 14 id
    createConfigParam("Meta15:Type",      '1', 0x3,   4, 24, 7); // Channel 15 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta15:Id",        '1', 0x3,   4, 28,15); // Channel 15 id
    createConfigParam("Meta10:Delay",     '1', 0x4,  32,  0, 0); // Channel  0 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta11:Delay",     '1', 0x5,  32,  0, 0); // Channel  1 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta12:Delay",     '1', 0x6,  32,  0, 0); // Channel  2 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta13:Delay",     '1', 0x7,  32,  0, 0); // Channel  3 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta14:Delay",     '1', 0x8,  32,  0, 0); // Channel  4 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta15:Delay",     '1', 0x9,  32,  0, 0); // Channel  5 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta16:Delay",     '1', 0xA,  32,  0, 0); // Channel  6 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta17:Delay",     '1', 0xB,  32,  0, 0); // Channel  7 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta18:Delay",     '1', 0xC,  32,  0, 0); // Channel  8 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta19:Delay",     '1', 0xD,  32,  0, 0); // Channel  9 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta110:Delay",    '1', 0xE,  32,  0, 0); // Channel 10 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta111:Delay",    '1', 0xF,  32,  0, 0); // Channel 11 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta112:Delay",    '1', 0x10, 32,  0, 0); // Channel 12 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta113:Delay",    '1', 0x11, 32,  0, 0); // Channel 13 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta114:Delay",    '1', 0x12, 32,  0, 0); // Channel 14 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta115:Delay",    '1', 0x13, 32,  0, 0); // Channel 15 delay              (scale:0.1,unit:us,prec:1)
    createConfigParam("Meta0:Mode",       '1', 0x14,  2,  0, 0); // Channel 0 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta1:Mode",       '1', 0x14,  2,  2, 0); // Channel 1 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta2:Mode",       '1', 0x14,  2,  4, 0); // Channel 2 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta3:Mode",       '1', 0x14,  2,  6, 0); // Channel 3 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta4:Mode",       '1', 0x14,  2,  8, 0); // Channel 4 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta5:Mode",       '1', 0x14,  2, 10, 0); // Channel 5 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta6:Mode",       '1', 0x14,  2, 12, 0); // Channel 6 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta7:Mode",       '1', 0x14,  2, 14, 0); // Channel 7 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta8:Mode",       '1', 0x14,  2, 16, 0); // Channel 8 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta9:Mode",       '1', 0x14,  2, 18, 0); // Channel 9 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta10:Mode",      '1', 0x14,  2, 20, 0); // Channel 10 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta11:Mode",      '1', 0x14,  2, 22, 0); // Channel 11 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta12:Mode",      '1', 0x14,  2, 24, 0); // Channel 12 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta13:Mode",      '1', 0x14,  2, 26, 0); // Channel 13 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta14:Mode",      '1', 0x14,  2, 28, 0); // Channel 14 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta15:Mode",      '1', 0x14,  2, 30, 0); // Channel 15 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("AcqRate",          '1', 0x15,  8,  0, 60); // Acqisition Frame Rate      (60=60Hz,30=30Hz,20=20Hz,15=15Hz,10=10Hz,5=5Hz,2=2Hz,1=1Hz)
    createConfigParam("Lvds1:Dis",        '1', 0x15,  1,  8, 0); // LVDS channel 1 disable      (0=enable,1=disable)
    createConfigParam("Lvds2:Dis",        '1', 0x15,  1,  9, 0); // LVDS channel 2 disable      (0=enable,1=disable)
    createConfigParam("Lvds3:Dis",        '1', 0x15,  1, 10, 0); // LVDS channel 3 disable      (0=enable,1=disable)
    createConfigParam("Lvds4:Dis",        '1', 0x15,  1, 11, 0); // LVDS channel 4 disable      (0=enable,1=disable)
    createConfigParam("Lvds5:Dis",        '1', 0x15,  1, 12, 0); // LVDS channel 5 disable      (0=enable,1=disable)
    createConfigParam("Lvds6:Dis",        '1', 0x15,  1, 13, 0); // LVDS channel 6 disable      (0=enable,1=disable)
    createConfigParam("Meta:Dis",         '1', 0x15,  1, 14, 0); // Meta channels disable       (0=enable,1=disable)
    createConfigParam("BucketTimeout",    '1', 0x15, 14, 15, 0x033E); // Bucket Timeout      (scale:0.0024094117,unit:ms)
    createConfigParam("TsyncTsDis",       '1', 0x15,  1, 29, 0); // Tsync Timestamp Disable (0=enable,1=disable)
    createConfigParam("TimeSource",       '1', 0x15,  1, 30, 0); // RTDL and EL source select (1=internal,0=acc timing)
    createConfigParam("Unused0",          '1', 0x16, 32,  0, 0); // Unused0
    createConfigParam("LegacyTsyncDly",   '1', 0x17, 32,  0, 0); // Legacy Tsync Delay             (scale:0.1,unit:us,prec:1)
    createConfigParam("LegacyEvLength",   '1', 0x18, 24,  0, 8); // Legacy Event Length            (unit:bytes)
    createConfigParam("LegacyDataFormat", '1', 0x18,  8, 24, 2); // Legacy Data Format
    createConfigParam("ClockCorrUpd",     '1', 0x19, 30,  0, 0); // DSP 40MHz Clock Corr Update
    createConfigParam("ClockCorrCtrl",    '1', 0x19,  2, 30, 0); // DSP 40MHz Clock Corr Ctrl (0=disable,1=disable,2=stall,3=advance)
    createConfigParam("TclkClkSource",    '1', 0x1A,  2,  0, 1); // Tclk Clock Source (0=none,1=external,2=internal,3=else)
    createConfigParam("DspClkSel",        '1', 0x1A,  1,  2, 0); // Dsp Clock Select (0=internal,1=external)
    createConfigParam("ChopperEquip",     '1', 0x1A,  1,  3, 0); // Chopper Equipment Select (0=AST,1=SKF)
    createConfigParam("ChopperRate",      '1', 0x1A,  8,  4, 60); // Chopper Frame Rate    (60=60Hz,30=30Hz,20=20Hz,15=15Hz,10=10Hz,5=5Hz,2=2Hz,1=1Hz)
    createConfigParam("Lvds1TsyncSel",    '1', 0x1A,  1, 12, 0); // LVDS1 Tsync Select     (0=new format,1=legacy)
    createConfigParam("Lvds2TsyncSel",    '1', 0x1A,  1, 13, 0); // LVDS2 Tsync Select     (0=new format,1=legacy)
    createConfigParam("Lvds3TsyncSel",    '1', 0x1A,  1, 14, 0); // LVDS3 Tsync Select     (0=new format,1=legacy)
    createConfigParam("Lvds4TsyncSel",    '1', 0x1A,  1, 15, 0); // LVDS4 Tsync Select     (0=new format,1=legacy)
    createConfigParam("Lvds5TsyncSel",    '1', 0x1A,  1, 16, 0); // LVDS5 Tsync Select     (0=new format,1=legacy)
    createConfigParam("Lvds6TsyncSel",    '1', 0x1A,  1, 17, 0); // LVDS6 Tsync Select     (0=new format,1=legacy)
    createConfigParam("MetaChan12Source", '1', 0x1A,  1, 18, 0); // Meta Channel12 Source  (0=ODB_In12,1=tsync_in)
    createConfigParam("MetaChan13Source", '1', 0x1A,  1, 19, 0); // Meta Channel13 Source  (0=ODB_In13,1=tclk_in)
    createConfigParam("MetaChan14Source", '1', 0x1A,  1, 20, 0); // Meta Channel14 Source  (0=ODB_In14,1=sysrstb_in)
    createConfigParam("MetaChan15Source", '1', 0x1A,  1, 21, 0); // Meta Channel15 Source  (0=ODB_In15,1=txenb_in)
    createConfigParam("Unused1",          '1', 0x1B, 32,  0, 0); // Unused1     
    createConfigParam("TrigDelay2",       '1', 0x1C, 32,  0, 0); // Trigger Delay2         (scale:0.0094117,unit:us,prec:1)
    createConfigParam("TrigDelay3",       '1', 0x1D, 32,  0, 0); // Trigger Delay3         (scale:0.0094117,unit:us,prec:1)
    createConfigParam("TrigFreqSel0",     '1', 0x1E,  4,  0, 0); // Trigger Frequency Select0 (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz) 
    createConfigParam("TrigFreqSel1",     '1', 0x1E,  4,  4, 0); // Trigger Frequency Select1 (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("TrigFreqSel2",     '1', 0x1E,  4,  8, 0); // Trigger Frequency Select2 (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("TrigFreqSel3",     '1', 0x1E,  4, 12, 0); // Trigger Frequency Select3 (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("TrigWidthSel2",    '1', 0x1F,  4,  0, 0); // Trigger Width Select2     (0=0.1us,1=1us,2=10us,3=100us,4=1ms,5=2ms,6=3ms,7=4ms)
    createConfigParam("TrigWidthSel3",    '1', 0x1F,  4,  4, 0); // Trigger Width Select3     (0=0.1us,1=1us,2=10us,3=100us,4=1ms,5=2ms,6=3ms,7=4ms)
    createConfigParam("TrigFrameDelay1",  '1', 0x1F,  8, 24, 0); // Trigger Frame Delay1 (unit:frames)
    

//      BLXXX:Det:DspX:| sig nam|      addr size off  | EPICS record description | (bi and mbbi description)
    createCounterParam("Lvds1:Good",      0x0, 32,  0); // LVDS 1 Number of good events
    createCounterParam("Lvds1:Lost",      0x1, 32,  0); // LVDS 1 Number of lost events
    createCounterParam("Lvds2:Good",      0x2, 32,  0); // LVDS 2 Number of good events
    createCounterParam("Lvds2:Lost",      0x3, 32,  0); // LVDS 2 Number of lost events
    createCounterParam("Lvds3:Good",      0x4, 32,  0); // LVDS 3 Number of good events
    createCounterParam("Lvds3:Lost",      0x5, 32,  0); // LVDS 3 Number of lost events
    createCounterParam("Lvds4:Good",      0x6, 32,  0); // LVDS 4 Number of good events
    createCounterParam("Lvds4:Lost",      0x7, 32,  0); // LVDS 4 Number of lost events
    createCounterParam("Lvds5:Good",      0x8, 32,  0); // LVDS 5 Number of good events
    createCounterParam("Lvds5:Lost",      0x9, 32,  0); // LVDS 5 Number of lost events
    createCounterParam("Lvds6:Good",      0xA, 32,  0); // LVDS 6 Number of good events
    createCounterParam("Lvds6:Lost",      0xB, 32,  0); // LVDS 6 Number of lost events
    createCounterParam("MetaEvents",      0xC, 32,  0); // Number of meta events
    createCounterParam("BucketA:GoodEvents", 0xD, 32,  0); // GoodABucketEvents (unit:max 15M)
    createCounterParam("BucketA:LostEvents", 0xE, 32,  0); // LostABucketEvents
    createCounterParam("BucketB:GoodEvents", 0xF, 32,  0); // GoodBBucketEvents (unit:max 15M)
    createCounterParam("BucketB:LostEvents", 0x10,32,  0); // LostBBucketEvents
    createCounterParam("TotalOpticalPackets", 0x11, 32,  0); // TotalOpticalPackets


//      BLXXX:Det:DspX:| sig nam|          addr size off  | EPICS record description | (bi and mbbi description)
    createStatusParam("Lvds1:Status",      0x0,  16,  0); // Lvds1 Status
    createStatusParam("Lvds2:Status",      0x0,  16, 16); // Lvds2 Status
    createStatusParam("Lvds3:Status",      0x1,  16,  0); // Lvds3 Status
    createStatusParam("Lvds4:Status",      0x1,  16, 16); // Lvds4 Status
    createStatusParam("Lvds5:Status",      0x2,  16,  0); // Lvds5 Status
    createStatusParam("Lvds6:Status",      0x2,  16, 16); // Lvds6 Status
    createStatusParam("TsyncCountsRF",     0x3,  32,  0); // TsyncCountRF
    createStatusParam("TsyncCountsInt",    0x4,  32,  0); // TsyncCountInt
    createStatusParam("TsyncCountsGPS",    0x5,  32,  0); // TsyncCountGPS
    createStatusParam("BucketA:Occupancy", 0x6,   4,  0); // NumberOfEmptyA-Buckets (unit:/12)
    createStatusParam("BucketB:Occupancy", 0x6,   4,  4); // NumberOfEmptyB-Buckets (unit:/12)
    createStatusParam("BucketsInActive1",  0x6,   1,  8); // BucketsInLvds1Active
    createStatusParam("BucketsInActive2",  0x6,   1,  9); // BucketsInLvds2Active
    createStatusParam("BucketsInActive3",  0x6,   1, 10); // BucketsInLvds3Active
    createStatusParam("BucketsInActive4",  0x6,   1, 11); // BucketsInLvds4Active
    createStatusParam("BucketsInActive5",  0x6,   1, 12); // BucketsInLvds5Active
    createStatusParam("BucketsInActive6",  0x6,   1, 13); // BucketsInLvds6Active
    createStatusParam("BucketsMetaActive", 0x6,   1, 14); // BucketsInMetaActive
    createStatusParam("RTDLReady",         0x6,   1, 15); // RTDL Ready
    createStatusParam("RTDLTXFault",       0x6,   1, 16); // RTDL SFP TX Fault
    createStatusParam("RTDLRXLoss",        0x6,   1, 17); // RTDL SFP RX Loss
    createStatusParam("RTDLUnplugged",     0x6,   1, 18); // RTDL SFP Unplugged
    createStatusParam("ELReady",           0x6,   1, 19); // EL Ready
    createStatusParam("ELTXFault",         0x6,   1, 20); // EL SFP TX Fault
    createStatusParam("ELRXLoss",          0x6,   1, 21); // EL SFP RX Loss
    createStatusParam("ELUnplugged",       0x6,   1, 22); // EL SFP Unplugged
    createStatusParam("ODBTXFault",        0x6,   1, 23); // ODB SFP TX Fault
    createStatusParam("ODBRXLoss",         0x6,   1, 24); // ODB SFP RX Loss
    createStatusParam("ODBUnplugged",      0x6,   1, 25); // ODB SFP Unplugged
    createStatusParam("Configured",        0x6,   1, 26); // DSP Configured
    createStatusParam("Acquiring",         0x6,   1, 27); // DSP Acquiring  (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("HWReady",           0x6,   1, 28); // DSP Hardware Ready (0=not ready,1=ready, archive:monitor)
    createStatusParam("HeartBit",          0x6,   1, 29); // Heart Bit (0=not present,1=present)
    createStatusParam("NoTsync",           0x7,   1,  0); // No TSYNC
    createStatusParam("NoRtdlXmit",        0x7,   1,  1); // No RTDL XMIT
    createStatusParam("NoRtdlValid",       0x7,   1,  2); // No RTDL VALID
    createStatusParam("RtdlCrcErrors",     0x7,   4,  4); // RTDL Time/Cycle CRC Errors
    createStatusParam("RtdlUpdated",       0x7,   4,  8); // RTDL Time/Cycle Updated
    createStatusParam("CommandSpacing",    0x8,  32,  0); // Command Spacing (scale:0.0094117,unit:us)
    createStatusParam("MetaChOwerflow0",   0x9,   1,  0); // Meta Channel Owerflow0
    createStatusParam("MetaChActivity0",   0x9,   1,  1); // Meta Channel Activity0
    createStatusParam("MetaChOwerflow1",   0x9,   1,  2); // Meta Channel Owerflow1
    createStatusParam("MetaChActivity1",   0x9,   1,  3); // Meta Channel Activity1
    createStatusParam("MetaChOwerflow2",   0x9,   1,  4); // Meta Channel Owerflow2
    createStatusParam("MetaChActivity2",   0x9,   1,  5); // Meta Channel Activity2
    createStatusParam("MetaChOwerflow3",   0x9,   1,  6); // Meta Channel Owerflow3
    createStatusParam("MetaChActivity3",   0x9,   1,  7); // Meta Channel Activity3
    createStatusParam("MetaChOwerflow4",   0x9,   1,  8); // Meta Channel Owerflow4
    createStatusParam("MetaChActivity4",   0x9,   1,  9); // Meta Channel Activity4
    createStatusParam("MetaChOwerflow5",   0x9,   1, 10); // Meta Channel Owerflow5
    createStatusParam("MetaChActivity5",   0x9,   1, 11); // Meta Channel Activity5
    createStatusParam("MetaChOwerflow6",   0x9,   1, 12); // Meta Channel Owerflow6
    createStatusParam("MetaChActivity6",   0x9,   1, 13); // Meta Channel Activity6
    createStatusParam("MetaChOwerflow7",   0x9,   1, 14); // Meta Channel Owerflow7
    createStatusParam("MetaChActivity7",   0x9,   1, 15); // Meta Channel Activity7
    createStatusParam("MetaChEvLost8",     0x9,   1, 16); // Meta Channel Events Lost8
    createStatusParam("MetaChActivity8",   0x9,   1, 17); // Meta Channel Activity8
    createStatusParam("MetaChEvLost9",     0x9,   1, 18); // Meta Channel Events Lost9
    createStatusParam("MetaChActivity9",   0x9,   1, 19); // Meta Channel Activity9
    createStatusParam("MetaChEvLost10",    0x9,   1, 20); // Meta Channel Events Lost10
    createStatusParam("MetaChActivity10",  0x9,   1, 21); // Meta Channel Activity10
    createStatusParam("MetaChEvLost11",    0x9,   1, 22); // Meta Channel Events Lost11
    createStatusParam("MetaChActivity11",  0x9,   1, 23); // Meta Channel Activity11
    createStatusParam("MetaChEvLost12",    0x9,   1, 24); // Meta Channel Events Lost12
    createStatusParam("MetaChActivity12",  0x9,   1, 25); // Meta Channel Activity12
    createStatusParam("MetaChEvLost13",    0x9,   1, 26); // Meta Channel Events Lost13
    createStatusParam("MetaChActivity13",  0x9,   1, 27); // Meta Channel Activity13
    createStatusParam("MetaChEvLost14",    0x9,   1, 28); // Meta Channel Events Lost14
    createStatusParam("MetaChActivity14",  0x9,   1, 29); // Meta Channel Activity14
    createStatusParam("MetaChEvLost15",    0x9,   1, 30); // Meta Channel Events Lost15
    createStatusParam("MetaChActivity15",  0x9,   1, 31); // Meta Channel Activity15
    createStatusParam("UnusedStatus",      0xB,  32,  0); // Unused15
}
