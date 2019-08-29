/* DspPlugin_v72.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Miljko Bobrek
 */

#include "DspPlugin.h"

void DspPlugin::createParams_v72() 
{
    createRegParam("VERSION", "HwRev",  true, 0,  8,  0);   // Hardware revision        (low:3,high:5)
    createRegParam("VERSION", "HwVer",  true, 0,  8,  8);   // Hardware revision        (low:1,high:3)
    createRegParam("VERSION", "FwRev",  true, 0,  8, 16);   // Firmware revision        (low:1,high:3)
    createRegParam("VERSION", "FwVer",  true, 0,  8, 24);   // Firmware revision        (low:6,high:8)
    createRegParam("VERSION", "FwYear", true, 1, 16,  0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwDay",  true, 1,  8, 16, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwMonth",true, 1,  8, 24, 0, CONV_HEX2DEC);

    //      BLXXX:Det:DspX:| sig nam|         '1' addr  size off default      | EPICS record description | (bi and mbbi description)
    createConfigParam("Meta0:Type",       '1', 0x0,   4,  0, 0); // Meta channel 0 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta0:PixId",      '1', 0x0,   4,  4, 0); // Meta channel 0 pixel id
    createConfigParam("Meta1:Type",       '1', 0x0,   4,  8, 0); // Meta channel 1 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta1:PixId",      '1', 0x0,   4, 12, 1); // Meta channel 1 pixel id
    createConfigParam("Meta2:Type",       '1', 0x0,   4, 16, 0); // Meta channel 2 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta2:PixId",      '1', 0x0,   4, 20, 2); // Meta channel 2 pixel id
    createConfigParam("Meta3:Type",       '1', 0x0,   4, 24, 0); // Meta channel 3 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta3:PixId",      '1', 0x0,   4, 28, 3); // Meta channel 3 pixel id
    createConfigParam("Meta4:Type",       '1', 0x1,   4,  0, 4); // Meta channel 4 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta4:PixId",      '1', 0x1,   4,  4, 4); // Meta channel 4 pixel id
    createConfigParam("Meta5:Type",       '1', 0x1,   4,  8, 4); // Meta channel 5 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta5:PixId",      '1', 0x1,   4, 12, 5); // Meta channel 5 pixel id
    createConfigParam("Meta6:Type",       '1', 0x1,   4, 16, 4); // Meta channel 6 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta6:PixId",      '1', 0x1,   4, 20, 6); // Meta channel 6 pixel id
    createConfigParam("Meta7:Type",       '1', 0x1,   4, 24, 4); // Meta channel 7 type - slow       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta7:PixId",      '1', 0x1,   4, 28, 7); // Meta channel 7 pixel id
    createConfigParam("Meta8:Type",       '1', 0x2,   4,  0, 6); // Meta channel 8 type - fast       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta8:PixId",      '1', 0x2,   4,  4, 8); // Meta channel 8 pixel id
    createConfigParam("Meta9:Type",       '1', 0x2,   4,  8, 6); // Meta channel 9 type - fast       (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta9:PixId",      '1', 0x2,   4, 12, 9); // Meta channel 9 pixel id
    createConfigParam("Meta10:Type",      '1', 0x2,   4, 16, 6); // Meta channel 10 type - fast      (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta10:PixId",     '1', 0x2,   4, 20,10); // Meta channel 10 pixel id
    createConfigParam("Meta11:Type",      '1', 0x2,   4, 24, 6); // Meta channel 11 type - fast      (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta11:PixId",     '1', 0x2,   4, 28,11); // Meta channel 11 pixel id
    createConfigParam("Meta12:Type",      '1', 0x3,   4,  0, 7); // Meta channel 12 type - fast      (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta12:PixId",     '1', 0x3,   4,  4,12); // Meta channel 12 pixel id
    createConfigParam("Meta13:Type",      '1', 0x3,   4,  8, 7); // Meta channel 13 type - fast      (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta13:PixId",     '1', 0x3,   4, 12,13); // Meta channel 13 pixel id
    createConfigParam("Meta14:Type",      '1', 0x3,   4, 16, 7); // Meta channel 14 type - fast      (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta14:PixId",     '1', 0x3,   4, 20,14); // Meta channel 14 pixel id
    createConfigParam("Meta15:Type",      '1', 0x3,   4, 24, 7); // Meta channel 15 type - fast      (0=detector,1=unused,2=unused,3=unused,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta15:Id",        '1', 0x3,   4, 28,15); // Meta channel 15 pixel id
    createConfigParam("Meta0:Delay",      '1', 0x4,  32,  0, 0); // Meta channel  0 delay            (scale:100,unit:ns)
    createConfigParam("Meta1:Delay",      '1', 0x5,  32,  0, 0); // Meta channel  1 delay            (scale:100,unit:ns)
    createConfigParam("Meta2:Delay",      '1', 0x6,  32,  0, 0); // Meta channel  2 delay            (scale:100,unit:ns)
    createConfigParam("Meta3:Delay",      '1', 0x7,  32,  0, 0); // Meta channel  3 delay            (scale:100,unit:ns)
    createConfigParam("Meta4:Delay",      '1', 0x8,  32,  0, 0); // Meta channel  4 delay            (scale:100,unit:ns)
    createConfigParam("Meta5:Delay",      '1', 0x9,  32,  0, 0); // Meta channel  5 delay            (scale:100,unit:ns)
    createConfigParam("Meta6:Delay",      '1', 0xA,  32,  0, 0); // Meta channel  6 delay            (scale:100,unit:ns)
    createConfigParam("Meta7:Delay",      '1', 0xB,  32,  0, 0); // Meta channel  7 delay            (scale:100,unit:ns)
    createConfigParam("Meta8:Delay",      '1', 0xC,  32,  0, 0); // Meta channel  8 delay            (scale:100,unit:ns)
    createConfigParam("Meta9:Delay",      '1', 0xD,  32,  0, 0); // Meta channel  9 delay            (scale:100,unit:ns)
    createConfigParam("Meta10:Delay",     '1', 0xE,  32,  0, 0); // Meta channel 10 delay            (scale:100,unit:ns)
    createConfigParam("Meta11:Delay",     '1', 0xF,  32,  0, 0); // Meta channel 11 delay            (scale:100,unit:ns)
    createConfigParam("Meta12:Delay",     '1', 0x10, 32,  0, 0); // Meta channel 12 delay            (scale:100,unit:ns)
    createConfigParam("Meta13:Delay",     '1', 0x11, 32,  0, 0); // Meta channel 13 delay            (scale:100,unit:ns)
    createConfigParam("Meta14:Delay",     '1', 0x12, 32,  0, 0); // Meta channel 14 delay            (scale:100,unit:ns)
    createConfigParam("Meta15:Delay",     '1', 0x13, 32,  0, 0); // Meta channel 15 delay            (scale:100,unit:ns)
    createConfigParam("Meta0:Mode",       '1', 0x14,  2,  0, 0); // Meta channel 0 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta1:Mode",       '1', 0x14,  2,  2, 0); // Meta channel 1 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta2:Mode",       '1', 0x14,  2,  4, 0); // Meta channel 2 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta3:Mode",       '1', 0x14,  2,  6, 0); // Meta channel 3 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta4:Mode",       '1', 0x14,  2,  8, 0); // Meta channel 4 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta5:Mode",       '1', 0x14,  2, 10, 0); // Meta channel 5 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta6:Mode",       '1', 0x14,  2, 12, 0); // Meta channel 6 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta7:Mode",       '1', 0x14,  2, 14, 0); // Meta channel 7 mode              (0=disabled,1=rising edge,2=falling edge,3=disabled)
    createConfigParam("Meta8:Mode",       '1', 0x14,  2, 16, 0); // Meta channel 8 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta9:Mode",       '1', 0x14,  2, 18, 0); // Meta channel 9 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta10:Mode",      '1', 0x14,  2, 20, 0); // Meta channel 10 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta11:Mode",      '1', 0x14,  2, 22, 0); // Meta channel 11 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta12:Mode",      '1', 0x14,  2, 24, 0); // Meta channel 12 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta13:Mode",      '1', 0x14,  2, 26, 0); // Meta channel 13 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta14:Mode",      '1', 0x14,  2, 28, 0); // Meta channel 14 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta15:Mode",      '1', 0x14,  2, 30, 0); // Meta channel 15 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("AcqFreq",          '1', 0x15,  8,  0, 60); // Acqisition Frame Frequency (60=60Hz,30=30Hz,20=20Hz,15=15Hz,10=10Hz,5=5Hz,2=2Hz,1=1Hz)
    createConfigParam("Lvds1:En",         '1', 0x15,  1,  8, 0); // LVDS channel 1 disable      (0=enable,1=disable)
    createConfigParam("Lvds2:En",         '1', 0x15,  1,  9, 0); // LVDS channel 2 disable      (0=enable,1=disable)
    createConfigParam("Lvds3:En",         '1', 0x15,  1, 10, 0); // LVDS channel 3 disable      (0=enable,1=disable)
    createConfigParam("Lvds4:En",         '1', 0x15,  1, 11, 0); // LVDS channel 4 disable      (0=enable,1=disable)
    createConfigParam("Lvds5:En",         '1', 0x15,  1, 12, 0); // LVDS channel 5 disable      (0=enable,1=disable)
    createConfigParam("Lvds6:En",         '1', 0x15,  1, 13, 0); // LVDS channel 6 disable      (0=enable,1=disable)
    createConfigParam("Meta:En",          '1', 0x15,  1, 14, 0); // Meta channels disable       (0=enable,1=disable)
    createConfigParam("BucketTimeout",    '1', 0x15, 14, 15, 0x033E); // Bucket Timeout         (scale:0.0024094117,unit:ms)
    createConfigParam("TsyncTsDis",       '1', 0x15,  1, 29, 0); // Tsync Timestamp Disable     (0=enable,1=disable)
    createConfigParam("TimeSource",       '1', 0x15,  1, 30, 0); // RTDL and EL source select   (1=internal,0=acc timing)
    createConfigParam("RTDLDisable",      '1', 0x15,  1, 31, 0); // RTDL Disable                (0=enable,1=disable)
    createConfigParam("TsyncDelay",       '1', 0x17, 32,  0, 0); // Legacy Tsync Delay          (scale:100,unit:ns,prec:1)
    createConfigParam("EventLength",      '1', 0x18, 24,  0, 8); // Legacy Event Length         (unit:bytes)
    createConfigParam("DataFormatId",     '1', 0x18,  8, 24, 2); // Legacy Data Format          (1=meta,2=pixel,4=LPSD verbose [alarm],5=XY PS,8=AROC raw [alarm],10=BNL raw,7=CROC raw)

    createConfigParam("ClockCorrUpd",     '1', 0x19, 30,  0, 0); // DSP 40MHz Clock Corr Update
    createConfigParam("ClockCorrCtrl",    '1', 0x19,  2, 30, 0); // DSP 40MHz Clock Corr Ctrl   (0=disable,1=disable,2=stall,3=advance)
    createConfigParam("TclkClkSource",    '1', 0x1A,  2,  0, 1); // Tclk Clock Source           (0=none,1=external,2=internal,3=else)
    createConfigParam("DspClkSel",        '1', 0x1A,  1,  2, 0); // Dsp Clock Select            (0=internal,1=external)
    createConfigParam("ChopperEquip",     '1', 0x1A,  1,  3, 0); // Chopper Equipment Select    (0=AST,1=SKF)
    createConfigParam("ChopperFreq",      '1', 0x1A,  8,  4, 60);// Chopper Frame Frequency     (60=60Hz,30=30Hz,20=20Hz,15=15Hz,10=10Hz,5=5Hz,2=2Hz,1=1Hz)
    createConfigParam("Lvds1:TsyncSel",   '1', 0x1A,  1, 12, 0); // LVDS1 Tsync Select          (0=new format,1=legacy)
    createConfigParam("Lvds2:TsyncSel",   '1', 0x1A,  1, 13, 0); // LVDS2 Tsync Select          (0=new format,1=legacy)
    createConfigParam("Lvds3:TsyncSel",   '1', 0x1A,  1, 14, 0); // LVDS3 Tsync Select          (0=new format,1=legacy)
    createConfigParam("Lvds4:TsyncSel",   '1', 0x1A,  1, 15, 0); // LVDS4 Tsync Select          (0=new format,1=legacy)
    createConfigParam("Lvds5:TsyncSel",   '1', 0x1A,  1, 16, 0); // LVDS5 Tsync Select          (0=new format,1=legacy)
    createConfigParam("Lvds6:TsyncSel",   '1', 0x1A,  1, 17, 0); // LVDS6 Tsync Select          (0=new format,1=legacy)
    createConfigParam("Meta8:Source",     '1', 0x1A,  1, 18, 0); // Meta channel 8 Source       (0=ODB_In8,1=ttl_in0)
    createConfigParam("Meta9:Source",     '1', 0x1A,  1, 19, 0); // Meta channel 9 Source       (0=ODB_In9,1=ttl_in1)
    createConfigParam("Meta10:Source",    '1', 0x1A,  1, 20, 0); // Meta channel 10 Source      (0=ODB_In10,1=ttl_in2)
    createConfigParam("Meta11:Source",    '1', 0x1A,  1, 21, 0); // Meta channel 11 Source      (0=ODB_In11,1=ttl_in3)
    createConfigParam("Meta12:Source",    '1', 0x1A,  1, 22, 0); // Meta channel 12 Source      (0=ODB_In12,1=tsync_in)
    createConfigParam("Meta13:Source",    '1', 0x1A,  1, 23, 0); // Meta channel 13 Source      (0=ODB_In13,1=tclk_in)
    createConfigParam("Meta14:Source",    '1', 0x1A,  1, 24, 0); // Meta channel 14 Source      (0=ODB_In14,1=sysrstb_in)
    createConfigParam("Meta15:Source",    '1', 0x1A,  1, 25, 0); // Meta channel 15 Source      (0=ODB_In15,1=txenb_in)
    createConfigParam("TimingPassthrough",'1', 0x1A,  1, 26, 0); // Timing passthrough          (0=disable,1=enable)
    createConfigParam("RTDLTXDis",        '1', 0x1A,  1, 27, 0); // RTDL TX disable             (0=enable,1=disable)
    createConfigParam("ELTXDis",          '1', 0x1A,  1, 28, 0); // EL TX disable               (0=enable,1=disable)
    createConfigParam("Trig0:Freq",       '1', 0x1E,  4,  0, 0); // Trigger 0 Frequency         (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("Trig1:Freq",       '1', 0x1E,  4,  4, 0); // Trigger 1 Frequency         (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("Trig1FrameDelay",  '1', 0x1F,  8, 24, 0); // Trigger 1 Frame Delay       (unit:frames)
    createConfigParam("Trig2:Freq",       '1', 0x1E,  4,  8, 0); // Trigger 2 Frequency         (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("Trig2:Width",      '1', 0x1F,  4,  0, 0); // Trigger 2 Width             (0=0.1us,1=1us,2=10us,3=100us,4=1ms,5=2ms,6=3ms,7=4ms)
    createConfigParam("Trig2:Delay",      '1', 0x1C, 32,  0, 0); // Trigger 2 Delay             (scale:9.4117,unit:ns)
    createConfigParam("Trig3:Freq",       '1', 0x1E,  4, 12, 0); // Trigger 3 Frequency         (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=10Hz,5=5Hz,6=2Hz,7=1Hz)
    createConfigParam("Trig3:Width",      '1', 0x1F,  4,  4, 0); // Trigger 3 Width             (0=0.1us,1=1us,2=10us,3=100us,4=1ms,5=2ms,6=3ms,7=4ms)
    createConfigParam("Trig3:Delay",      '1', 0x1D, 32,  0, 0); // Trigger 3 Delay             (scale:9.4117,unit:ns)
    createConfigParam("TestPatternDly",   '1', 0x1B, 16,  0, 0); // Test Pattern Delay          (scale:0.0024094117,unit:ms)
    createConfigParam("TestPattPlsPer",   '1', 0x1B, 16, 16, 10625); // Test Pattern Pulse Period   (scale:0.0094117,unit:us)


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
    createCounterParam("TotOptPackets",   0x11, 32,  0); // TotalOpticalPackets


//      BLXXX:Det:DspX:| sig nam|          addr size off  | EPICS record description | (bi and mbbi description)
    createStatusParam("Lvds1:ParityErr",   0x0,   1,  0); // LVDS1 had parity errors
    createStatusParam("Lvds1:EvBlocked",   0x0,   1,  1); // LVDS1 blocked events
    createStatusParam("Lvds1:EvShort",     0x0,   1,  2); // LVDS1 had short events
    createStatusParam("Lvds1:Ev2Starts",   0x0,   1,  3); // LVDS1 had double starts
    createStatusParam("Lvds1:FifoAlmostF", 0x0,   1,  4); // LVDS1 FIFO almost full
    createStatusParam("Lvds1:FifoFull",    0x0,   1,  5); // LVDS1 FIFO full
    createStatusParam("Lvds1:FifoHasData", 0x0,   1,  6); // LVDS1 FIFO has data
    createStatusParam("Lvds1:ChAlmostF",   0x0,   1,  7); // LVDS1 channel almost full
    createStatusParam("Lvds1:ChHasData",   0x0,   1,  8); // LVDS1 channel has data
    createStatusParam("Lvds1:ChHasCmd",    0x0,   1,  9); // LVDS1 channel has command
    createStatusParam("Lvds1:CmdAlmostF",  0x0,   1, 10); // LVDS1 command FIFO almost full
    createStatusParam("Lvds1:CmdWren",     0x0,   1, 11); // LVDS1 command FIFO WREN
    createStatusParam("Lvds1:DataAlmostF", 0x0,   1, 12); // LVDS1 data FIFO almost full
    createStatusParam("Lvds1:DataPresent", 0x0,   1, 13); // LVDS1 data FIFO has data
    createStatusParam("Lvds1:Overflow",    0x0,   1, 14); // LVDS1 overflow
    createStatusParam("Lvds1:BadPacket",   0x0,   1, 15); // LVDS1 had bad packet
    createStatusParam("Lvds2:ParityErr",   0x0,   1, 16); // LVDS2 had parity errors
    createStatusParam("Lvds2:EvBlocked",   0x0,   1, 17); // LVDS2 blocked events
    createStatusParam("Lvds2:EvShort",     0x0,   1, 18); // LVDS2 had short events
    createStatusParam("Lvds2:Ev2Starts",   0x0,   1, 19); // LVDS2 had double starts
    createStatusParam("Lvds2:FifoAlmostF", 0x0,   1, 20); // LVDS2 FIFO almost full
    createStatusParam("Lvds2:FifoFull",    0x0,   1, 21); // LVDS2 FIFO full
    createStatusParam("Lvds2:FifoHasData", 0x0,   1, 22); // LVDS2 FIFO has data
    createStatusParam("Lvds2:ChAlmostF",   0x0,   1, 23); // LVDS2 channel almost full
    createStatusParam("Lvds2:ChHasData",   0x0,   1, 24); // LVDS2 channel has data
    createStatusParam("Lvds2:ChHasCmd",    0x0,   1, 25); // LVDS2 channel has command
    createStatusParam("Lvds2:CmdAlmostF",  0x0,   1, 26); // LVDS2 command FIFO almost full
    createStatusParam("Lvds2:CmdWren",     0x0,   1, 27); // LVDS2 command FIFO WREN
    createStatusParam("Lvds2:DataAlmostF", 0x0,   1, 28); // LVDS2 data FIFO almost full
    createStatusParam("Lvds2:DataPresent", 0x0,   1, 29); // LVDS2 data FIFO has data
    createStatusParam("Lvds2:Overflow",    0x0,   1, 30); // LVDS2 overflow
    createStatusParam("Lvds2:BadPacket",   0x0,   1, 31); // LVDS2 had bad packet
    createStatusParam("Lvds3:ParityErr",   0x1,   1,  0); // LVDS3 had parity errors
    createStatusParam("Lvds3:EvBlocked",   0x1,   1,  1); // LVDS3 blocked events
    createStatusParam("Lvds3:EvShort",     0x1,   1,  2); // LVDS3 had short events
    createStatusParam("Lvds3:Ev2Starts",   0x1,   1,  3); // LVDS3 had double starts
    createStatusParam("Lvds3:FifoAlmostF", 0x1,   1,  4); // LVDS3 FIFO almost full
    createStatusParam("Lvds3:FifoFull",    0x1,   1,  5); // LVDS3 FIFO full
    createStatusParam("Lvds3:FifoHasData", 0x1,   1,  6); // LVDS3 FIFO has data
    createStatusParam("Lvds3:ChAlmostF",   0x1,   1,  7); // LVDS3 channel almost full
    createStatusParam("Lvds3:ChHasData",   0x1,   1,  8); // LVDS3 channel has data
    createStatusParam("Lvds3:ChHasCmd",    0x1,   1,  9); // LVDS3 channel has command
    createStatusParam("Lvds3:CmdAlmostF",  0x1,   1, 10); // LVDS3 command FIFO almost full
    createStatusParam("Lvds3:CmdWren",     0x1,   1, 11); // LVDS3 command FIFO WREN
    createStatusParam("Lvds3:DataAlmostF", 0x1,   1, 12); // LVDS3 data FIFO almost full
    createStatusParam("Lvds3:DataPresent", 0x1,   1, 13); // LVDS3 data FIFO has data
    createStatusParam("Lvds3:Overflow",    0x1,   1, 14); // LVDS3 overflow
    createStatusParam("Lvds3:BadPacket",   0x1,   1, 15); // LVDS3 had bad packet
    createStatusParam("Lvds4:ParityErr",   0x1,   1, 16); // LVDS4 had parity errors
    createStatusParam("Lvds4:EvBlocked",   0x1,   1, 17); // LVDS4 blocked events
    createStatusParam("Lvds4:EvShort",     0x1,   1, 18); // LVDS4 had short events
    createStatusParam("Lvds4:Ev2Starts",   0x1,   1, 19); // LVDS4 had double starts
    createStatusParam("Lvds4:FifoAlmostF", 0x1,   1, 20); // LVDS4 FIFO almost full
    createStatusParam("Lvds4:FifoFull",    0x1,   1, 21); // LVDS4 FIFO full
    createStatusParam("Lvds4:FifoHasData", 0x1,   1, 22); // LVDS4 FIFO has data
    createStatusParam("Lvds4:ChAlmostF",   0x1,   1, 23); // LVDS4 channel almost full
    createStatusParam("Lvds4:ChHasData",   0x1,   1, 24); // LVDS4 channel has data
    createStatusParam("Lvds4:ChHasCmd",    0x1,   1, 25); // LVDS4 channel has command
    createStatusParam("Lvds4:CmdAlmostF",  0x1,   1, 26); // LVDS4 command FIFO almost full
    createStatusParam("Lvds4:CmdWren",     0x1,   1, 27); // LVDS4 command FIFO WREN
    createStatusParam("Lvds4:DataAlmostF", 0x1,   1, 28); // LVDS4 data FIFO almost full
    createStatusParam("Lvds4:DataPresent", 0x1,   1, 29); // LVDS4 data FIFO has data
    createStatusParam("Lvds4:Overflow",    0x1,   1, 30); // LVDS4 overflow
    createStatusParam("Lvds4:BadPacket",   0x1,   1, 31); // LVDS4 had bad packet
    createStatusParam("Lvds5:ParityErr",   0x2,   1,  0); // LVDS5 had parity errors
    createStatusParam("Lvds5:EvBlocked",   0x2,   1,  1); // LVDS5 blocked events
    createStatusParam("Lvds5:EvShort",     0x2,   1,  2); // LVDS5 had short events
    createStatusParam("Lvds5:Ev2Starts",   0x2,   1,  3); // LVDS5 had double starts
    createStatusParam("Lvds5:FifoAlmostF", 0x2,   1,  4); // LVDS5 FIFO almost full
    createStatusParam("Lvds5:FifoFull",    0x2,   1,  5); // LVDS5 FIFO full
    createStatusParam("Lvds5:FifoHasData", 0x2,   1,  6); // LVDS5 FIFO has data
    createStatusParam("Lvds5:ChAlmostF",   0x2,   1,  7); // LVDS5 channel almost full
    createStatusParam("Lvds5:ChHasData",   0x2,   1,  8); // LVDS5 channel has data
    createStatusParam("Lvds5:ChHasCmd",    0x2,   1,  9); // LVDS5 channel has command
    createStatusParam("Lvds5:CmdAlmostF",  0x2,   1, 10); // LVDS5 command FIFO almost full
    createStatusParam("Lvds5:CmdWren",     0x2,   1, 11); // LVDS5 command FIFO WREN
    createStatusParam("Lvds5:DataAlmostF", 0x2,   1, 12); // LVDS5 data FIFO almost full
    createStatusParam("Lvds5:DataPresent", 0x2,   1, 13); // LVDS5 data FIFO has data
    createStatusParam("Lvds5:Overflow",    0x2,   1, 14); // LVDS5 overflow
    createStatusParam("Lvds5:BadPacket",   0x2,   1, 15); // LVDS5 had bad packet
    createStatusParam("Lvds6:ParityErr",   0x2,   1, 16); // LVDS6 had parity errors
    createStatusParam("Lvds6:EvBlocked",   0x2,   1, 17); // LVDS6 blocked events
    createStatusParam("Lvds6:EvShort",     0x2,   1, 18); // LVDS6 had short events
    createStatusParam("Lvds6:Ev2Starts",   0x2,   1, 19); // LVDS6 had double starts
    createStatusParam("Lvds6:FifoAlmostF", 0x2,   1, 20); // LVDS6 FIFO almost full
    createStatusParam("Lvds6:FifoFull",    0x2,   1, 21); // LVDS6 FIFO full
    createStatusParam("Lvds6:FifoHasData", 0x2,   1, 22); // LVDS6 FIFO has data
    createStatusParam("Lvds6:ChAlmostF",   0x2,   1, 23); // LVDS6 channel almost full
    createStatusParam("Lvds6:ChHasData",   0x2,   1, 24); // LVDS6 channel has data
    createStatusParam("Lvds6:ChHasCmd",    0x2,   1, 25); // LVDS6 channel has command
    createStatusParam("Lvds6:CmdAlmostF",  0x2,   1, 26); // LVDS6 command FIFO almost full
    createStatusParam("Lvds6:CmdWren",     0x2,   1, 27); // LVDS6 command FIFO WREN
    createStatusParam("Lvds6:DataAlmostF", 0x2,   1, 28); // LVDS6 data FIFO almost full
    createStatusParam("Lvds6:DataPresent", 0x2,   1, 29); // LVDS6 data FIFO has data
    createStatusParam("Lvds6:Overflow",    0x2,   1, 30); // LVDS6 overflow
    createStatusParam("Lvds6:BadPacket",   0x2,   1, 31); // LVDS6 had bad packet
    createStatusParam("TsyncCountsRF",     0x3,  32,  0); // TsyncCountRF
    createStatusParam("TsyncCountsInt",    0x4,  32,  0); // TsyncCountInt
    createStatusParam("TsyncCountsGPS",    0x5,  32,  0); // TsyncCountGPS
    createStatusParam("BucketA:Used",      0x6,   4,  0); // Used A buckets (calc:12-A,unit:of 12)
    createStatusParam("BucketB:Used",      0x6,   4,  4); // Used B buckets (calc:12-A,unit:of 12)
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
    createStatusParam("HeartBeat",         0x6,   1, 29); // Heart Beat (0=not present,1=present)
    createStatusParam("NoTsync",           0x7,   1,  0); // No TSYNC
    createStatusParam("NoRtdlXmit",        0x7,   1,  1); // No RTDL XMIT
    createStatusParam("NoRtdlValid",       0x7,   1,  2); // No RTDL VALID
    createStatusParam("RtdlCrcErrors",     0x7,   4,  4); // RTDL Time/Cycle CRC Errors
    createStatusParam("RtdlUpdated",       0x7,   4,  8); // RTDL Time/Cycle Updated
    createStatusParam("CommandSpacing",    0x8,  32,  0); // Command Spacing (scale:0.0094117,unit:us)
    createStatusParam("Meta0:Owerflow",    0x9,   1,  0); // Meta Channel Owerflow0
    createStatusParam("Meta0:Activity",    0x9,   1,  1); // Meta Channel Activity0
    createStatusParam("Meta1:Owerflow",    0x9,   1,  2); // Meta Channel Owerflow1
    createStatusParam("Meta1:Activity",    0x9,   1,  3); // Meta Channel Activity1
    createStatusParam("Meta2:Owerflow",    0x9,   1,  4); // Meta Channel Owerflow2
    createStatusParam("Meta2:Activity",    0x9,   1,  5); // Meta Channel Activity2
    createStatusParam("Meta3:Owerflow",    0x9,   1,  6); // Meta Channel Owerflow3
    createStatusParam("Meta3:Activity",    0x9,   1,  7); // Meta Channel Activity3
    createStatusParam("Meta4:Owerflow",    0x9,   1,  8); // Meta Channel Owerflow4
    createStatusParam("Meta4:Activity",    0x9,   1,  9); // Meta Channel Activity4
    createStatusParam("Meta5:Owerflow",    0x9,   1, 10); // Meta Channel Owerflow5
    createStatusParam("Meta5:Activity",    0x9,   1, 11); // Meta Channel Activity5
    createStatusParam("Meta6:Owerflow",    0x9,   1, 12); // Meta Channel Owerflow6
    createStatusParam("Meta6:Activity",    0x9,   1, 13); // Meta Channel Activity6
    createStatusParam("Meta7:Owerflow",    0x9,   1, 14); // Meta Channel Owerflow7
    createStatusParam("Meta7:Activity",    0x9,   1, 15); // Meta Channel Activity7
    createStatusParam("Meta8:EvLost",      0x9,   1, 16); // Meta Channel Events Lost8
    createStatusParam("Meta8:Activity",    0x9,   1, 17); // Meta Channel Activity8
    createStatusParam("Meta9:EvLost",      0x9,   1, 18); // Meta Channel Events Lost9
    createStatusParam("Meta9:Activity",    0x9,   1, 19); // Meta Channel Activity9
    createStatusParam("Meta10:EvLost",     0x9,   1, 20); // Meta Channel Events Lost10
    createStatusParam("Meta10:Activity",   0x9,   1, 21); // Meta Channel Activity10
    createStatusParam("Meta11:EvLost",     0x9,   1, 22); // Meta Channel Events Lost11
    createStatusParam("Meta11:Activity",   0x9,   1, 23); // Meta Channel Activity11
    createStatusParam("Meta12:EvLost",     0x9,   1, 24); // Meta Channel Events Lost12
    createStatusParam("Meta12:Activity",   0x9,   1, 25); // Meta Channel Activity12
    createStatusParam("Meta13:EvLost",     0x9,   1, 26); // Meta Channel Events Lost13
    createStatusParam("Meta13:Activity",   0x9,   1, 27); // Meta Channel Activity13
    createStatusParam("Meta14:EvLost",     0x9,   1, 28); // Meta Channel Events Lost14
    createStatusParam("Meta14:Activity",   0x9,   1, 29); // Meta Channel Activity14
    createStatusParam("Meta15:EvLost",     0x9,   1, 30); // Meta Channel Events Lost15
    createStatusParam("Meta15:Activity",   0x9,   1, 31); // Meta Channel Activity15
    createStatusParam("UnusedStatus",      0xB,  32,  0); // Unused15
}
