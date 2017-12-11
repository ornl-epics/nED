/* DspPlugin_v70.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Miljko Bobrek
 */

#include "DspPlugin.h"

void DspPlugin::createParams_v70() {
//      BLXXX:Det:DspX:| sig nam|         '1' addr  size off default      | EPICS record description | (bi and mbbi description)
    createConfigParam("Meta0:Type",       '1', 0x0,   4,  0, 0); // Channel 0 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta0:Id",         '1', 0x0,   4,  4, 0); // Channel 0 id
    createConfigParam("Meta0:Delay",      '1', 0x0,  24,  8, 0); // Channel 0 delay               (scale=100,unit=ns)
    createConfigParam("Meta1:Type",       '1', 0x1,   4,  0, 0); // Channel 1 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta1:Id",         '1', 0x1,   4,  4, 0); // Channel 1 id
    createConfigParam("Meta1:Delay",      '1', 0x1,  24,  8, 0); // Channel 1 delay               (scale=100,unit=ns)
    createConfigParam("Meta2:Type",       '1', 0x2,   4,  0, 0); // Channel 2 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta2:Id",         '1', 0x2,   4,  4, 0); // Channel 2 id
    createConfigParam("Meta2:Delay",      '1', 0x2,  24,  8, 0); // Channel 2 delay               (scale=100,unit=ns)
    createConfigParam("Meta3:Type",       '1', 0x3,   4,  0, 0); // Channel 3 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta3:Id",         '1', 0x3,   4,  4, 0); // Channel 3 id
    createConfigParam("Meta3:Delay",      '1', 0x3,  24,  8, 0); // Channel 3 delay               (scale=100,unit=ns)
    createConfigParam("Meta4:Type",       '1', 0x4,   4,  0, 0); // Channel 4 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta4:Id",         '1', 0x4,   4,  4, 0); // Channel 4 id
    createConfigParam("Meta4:Delay",      '1', 0x4,  24,  8, 0); // Channel 4 delay               (scale=100,unit=ns)
    createConfigParam("Meta5:Type",       '1', 0x5,   4,  0, 0); // Channel 5 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta5:Id",         '1', 0x5,   4,  4, 0); // Channel 5 id
    createConfigParam("Meta5:Delay",      '1', 0x5,  24,  8, 0); // Channel 5 delay               (scale=100,unit=ns)
    createConfigParam("Meta6:Type",       '1', 0x6,   4,  0, 0); // Channel 6 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta6:Id",         '1', 0x6,   4,  4, 0); // Channel 6 id
    createConfigParam("Meta6:Delay",      '1', 0x6,  24,  8, 0); // Channel 6 delay               (scale=100,unit=ns)
    createConfigParam("Meta7:Type",       '1', 0x7,   4,  0, 0); // Channel 7 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta7:Id",         '1', 0x7,   4,  4, 0); // Channel 7 id
    createConfigParam("Meta7:Delay",      '1', 0x7,  24,  8, 0); // Channel 7 delay               (scale=100,unit=ns)
    createConfigParam("Meta8:Type",       '1', 0x8,   4,  0, 0); // Channel 8 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta8:Id",         '1', 0x8,   4,  4, 0); // Channel 8 id
    createConfigParam("Meta8:Delay",      '1', 0x8,  24,  8, 0); // Channel 8 delay               (scale=100,unit=ns)
    createConfigParam("Meta9:Type",       '1', 0x9,   4,  0, 0); // Channel 9 type                (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta9:Id",         '1', 0x9,   4,  4, 0); // Channel 9 id
    createConfigParam("Meta9:Delay",      '1', 0x9,  24,  8, 0); // Channel 9 delay               (scale=100,unit=ns)
    createConfigParam("Meta10:Type",      '1', 0xA,   4,  0, 0); // Channel 10 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta10:Id",        '1', 0xA,   4,  4, 0); // Channel 10 id
    createConfigParam("Meta10:Delay",     '1', 0xA,  24,  8, 0); // Channel 10 delay              (scale=100,unit=ns)
    createConfigParam("Meta11:Type",      '1', 0xB,   4,  0, 0); // Channel 11 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta11:Id",        '1', 0xB,   4,  4, 0); // Channel 11 id
    createConfigParam("Meta11:Delay",     '1', 0xB,  24,  8, 0); // Channel 11 delay              (scale=100,unit=ns)
    createConfigParam("Meta12:Type",      '1', 0xC,   4,  0, 0); // Channel 12 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta12:Id",        '1', 0xC,   4,  4, 0); // Channel 12 id
    createConfigParam("Meta12:Delay",     '1', 0xC,  24,  8, 0); // Channel 12 delay              (scale=100,unit=ns)
    createConfigParam("Meta13:Type",      '1', 0xD,   4,  0, 0); // Channel 13 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta13:Id",        '1', 0xD,   4,  4, 0); // Channel 13 id
    createConfigParam("Meta13:Delay",     '1', 0xD,  24,  8, 0); // Channel 13 delay              (scale=100,unit=ns)
    createConfigParam("Meta14:Type",      '1', 0xE,   4,  0, 0); // Channel 14 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta14:Id",        '1', 0xE,   4,  4, 0); // Channel 14 id
    createConfigParam("Meta14:Delay",     '1', 0xE,  24,  8, 0); // Channel 14 delay              (scale=100,unit=ns)
    createConfigParam("Meta15:Type",      '1', 0xF,   4,  0, 0); // Channel 15 type               (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Meta15:Id",        '1', 0xF,   4,  4, 0); // Channel 15 id
    createConfigParam("Meta15:Delay",     '1', 0xF,  24,  8, 0); // Channel 15 delay              (scale=100,unit=ns)
    createConfigParam("Meta0:Frame",      '1', 0x10,  8,  0, 0); // Channel 0 frame adjust
    createConfigParam("Meta1:Frame",      '1', 0x10,  8,  8, 0); // Channel 1 frame adjust
    createConfigParam("Meta2:Frame",      '1', 0x10,  8, 16, 0); // Channel 2 frame adjust
    createConfigParam("Meta3:Frame",      '1', 0x10,  8, 24, 0); // Channel 3 frame adjust
    createConfigParam("Meta4:Frame",      '1', 0x11,  8,  0, 0); // Channel 4 frame adjust
    createConfigParam("Meta5:Frame",      '1', 0x11,  8,  8, 0); // Channel 5 frame adjust
    createConfigParam("Meta6:Frame",      '1', 0x11,  8, 16, 0); // Channel 6 frame adjust
    createConfigParam("Meta7:Frame",      '1', 0x11,  8, 24, 0); // Channel 7 frame adjust
    createConfigParam("Meta8:Frame",      '1', 0x12,  8,  0, 0); // Channel 8 frame adjust
    createConfigParam("Meta9:Frame",      '1', 0x12,  8,  8, 0); // Channel 9 frame adjust
    createConfigParam("Meta10:Frame",     '1', 0x12,  8, 16, 0); // Channel 10 frame adjust
    createConfigParam("Meta11:Frame",     '1', 0x12,  8, 24, 0); // Channel 11 frame adjust
    createConfigParam("Meta12:Frame",     '1', 0x13,  8,  0, 0); // Channel 12 frame adjust
    createConfigParam("Meta13:Frame",     '1', 0x13,  8,  8, 0); // Channel 13 frame adjust
    createConfigParam("Meta14:Frame",     '1', 0x13,  8, 16, 0); // Channel 14 frame adjust
    createConfigParam("Meta15:Frame",     '1', 0x13,  8, 24, 0); // Channel 15 frame adjust
    createConfigParam("Meta0:Mode",       '1', 0x14,  2,  0, 0); // Channel 0 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta1:Mode",       '1', 0x14,  2,  2, 0); // Channel 1 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta2:Mode",       '1', 0x14,  2,  4, 0); // Channel 2 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta3:Mode",       '1', 0x14,  2,  6, 0); // Channel 3 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta4:Mode",       '1', 0x14,  2,  8, 0); // Channel 4 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta5:Mode",       '1', 0x14,  2, 10, 0); // Channel 5 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta6:Mode",       '1', 0x14,  2, 12, 0); // Channel 6 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta7:Mode",       '1', 0x14,  2, 14, 0); // Channel 7 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta8:Mode",       '1', 0x14,  2, 16, 0); // Channel 8 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta9:Mode",       '1', 0x14,  2, 18, 0); // Channel 9 mode              (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta10:Mode",      '1', 0x14,  2, 20, 0); // Channel 10 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta11:Mode",      '1', 0x14,  2, 22, 0); // Channel 11 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta12:Mode",      '1', 0x14,  2, 24, 0); // Channel 12 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta13:Mode",      '1', 0x14,  2, 26, 0); // Channel 13 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta14:Mode",      '1', 0x14,  2, 28, 0); // Channel 14 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("Meta15:Mode",      '1', 0x14,  2, 30, 0); // Channel 15 mode             (0=disabled,1=rising edge,2=falling edge,3=both)
    createConfigParam("MetaFormat",       '1', 0x15,  8,  0, 1); // Meta channels event format   (0=legacy,1=meta,2=pixel,3=XY,4=XY PS)
    createConfigParam("Lvds1:En",         '1', 0x15,  1,  8, 0); // LVDS channel 1 enable flag  (0=enable,1=disable)
    createConfigParam("Lvds2:En",         '1', 0x15,  1,  9, 0); // LVDS channel 2 enable flag  (0=enable,1=disable)
    createConfigParam("Lvds3:En",         '1', 0x15,  1, 10, 0); // LVDS channel 3 enable flag  (0=enable,1=disable)
    createConfigParam("Lvds4:En",         '1', 0x15,  1, 11, 0); // LVDS channel 4 enable flag  (0=enable,1=disable)
    createConfigParam("Lvds5:En",         '1', 0x15,  1, 12, 0); // LVDS channel 5 enable flag  (0=enable,1=disable)
    createConfigParam("Lvds6:En",         '1', 0x15,  1, 13, 0); // LVDS channel 6 enable flag  (0=enable,1=disable)
    createConfigParam("Meta:En",          '1', 0x15,  1, 14, 0); // Enable Meta channels         (0=enable,1=disable)
    createConfigParam("FrameRate",        '1', 0x15,  8, 16, 60); // AcqFrameRate               (unit:Hz)
    createConfigParam("MetaTestAsync",    '1', 0x15,  6, 24, 0); // ODBTestPulseAsync
    createConfigParam("TimeSource",       '1', 0x15,  1, 31, 0); // RTDL and Ev39 source select (1=internal,0=acc timing)
    createConfigParam("TofFixOffset",     '1', 0x16, 24,  0, 0); // TSYNC delay only
    createConfigParam("FrameOffset",      '1', 0x16,  8, 24, 0); // FrameOffset
    createConfigParam("BucketTimeout",    '1', 0x17, 32,  0, 0x00033E15); // BucketTimeout      (scale:0.0000094117,unit:ms)
    createConfigParam("OdbTestDelay0",    '1', 0x18, 16,  0, 0); // ODBTestPulseDelay0          (scale:2.409,unit:us)
    createConfigParam("OdbTestDelay1",    '1', 0x18, 16, 16, 0); // ODBTestPulseDelay1          (scale:2.409,unit:us)
    createConfigParam("OdbTestPeriod",    '1', 0x19, 16,  0, 0x0CF8); // ODBTestPulsePeriod     (scale:0.1506,unit:us)
    createConfigParam("UnusedConfig",     '1', 0x1F, 32,  0, 0); // Config31

//      BLXXX:Det:DspX:| sig nam|      addr size off  | EPICS record description | (bi and mbbi description)
    createCounterParam("Lvds1:Good",      0x0, 32,  0); // LVDS 1 Number of good packets
    createCounterParam("Lvds1:Lost",      0x1, 32,  0); // LVDS 1 Number of lost packets
    createCounterParam("Lvds2:Good",      0x2, 32,  0); // LVDS 2 Number of good packets
    createCounterParam("Lvds2:Lost",      0x3, 32,  0); // LVDS 2 Number of lost packets
    createCounterParam("Lvds3:Good",      0x4, 32,  0); // LVDS 3 Number of good packets
    createCounterParam("Lvds3:Lost",      0x5, 32,  0); // LVDS 3 Number of lost packets
    createCounterParam("Lvds4:Good",      0x6, 32,  0); // LVDS 4 Number of good packets
    createCounterParam("Lvds4:Lost",      0x7, 32,  0); // LVDS 4 Number of lost packets
    createCounterParam("Lvds5:Good",      0x8, 32,  0); // LVDS 5 Number of good packets
    createCounterParam("Lvds5:Lost",      0x9, 32,  0); // LVDS 5 Number of lost packets
    createCounterParam("Lvds6:Good",      0xA, 32,  0); // LVDS 6 Number of good packets
    createCounterParam("Lvds6:Lost",      0xB, 32,  0); // LVDS 6 Number of lost packets
    createCounterParam("MetaEvents",      0xC, 32,  0); // Number of meta events
    createCounterParam("BucketGood",      0xD, 32,  0); // Number of good bucket events
    createCounterParam("BucketLost",      0xE, 32,  0); // Number of lost bucket events
    createCounterParam("OptPackets",      0xF, 32,  0); // Number of output optical packets

//      BLXXX:Det:DspX:| sig nam|      addr size off  | EPICS record description | (bi and mbbi description)
    createStatusParam("Lvds1:Status",      0x0,  32,  0); // Lvds1Status
    createStatusParam("Lvds2:Status",      0x1,  32,  0); // Lvds2Status
    createStatusParam("Lvds3:Status",      0x2,  32,  0); // Lvds3Status
    createStatusParam("Lvds4:Status",      0x3,  32,  0); // Lvds4Status
    createStatusParam("Lvds5:Status",      0x4,  32,  0); // Lvds5Status
    createStatusParam("Lvds6:Status",      0x5,  32,  0); // Lvds6Status
    createStatusParam("EmptyBuckets",      0x6,   4,  0); // Number of empty buckets
    createStatusParam("Aux:Buckets",       0x6,   1,  4); // Number of buckets used for AUX
    createStatusParam("Lvds1:Buckets",     0x6,   1,  5); // Number of buckets used for LVDS 1
    createStatusParam("Lvds1:Buckets",     0x6,   1,  6); // Number of buckets used for LVDS 2
    createStatusParam("Lvds1:Buckets",     0x6,   1,  7); // Number of buckets used for LVDS 3
    createStatusParam("Lvds1:Buckets",     0x6,   1,  8); // Number of buckets used for LVDS 4
    createStatusParam("Lvds1:Buckets",     0x6,   1,  9); // Number of buckets used for LVDS 5
    createStatusParam("Lvds1:Buckets",     0x6,   1, 10); // Number of buckets used for LVDS 6
    createStatusParam("Meta:Buckets",      0x6,   1, 11); // Number of buckets used for meta
    createStatusParam("UnusedStatus",      0xF,  32,  0); // Unused15
}
