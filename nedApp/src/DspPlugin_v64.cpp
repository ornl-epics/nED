/* DspPlugin_v64.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspPlugin.h"

void DspPlugin::createConfigParams_v64() {
//      BLXXX:Det:DspX:| sig nam|                                 | EPICS record description | (bi and mbbi description)
    createConfigParam("PixIdOff",       'B', 0x0,  32,  0, 0); // Pixel id offset

    // Chopper parameters
    createConfigParam("Chop0:Delay",    'C', 0x0,  32,  0, 0); // Chop0 delay for N*9.4ns cyc
    createConfigParam("Chop1:Delay",    'C', 0x1,  32,  0, 0); // Chop1 delay for N*9.4ns cyc
    createConfigParam("Chop2:Delay",    'C', 0x2,  32,  0, 0); // Chop2 delay for N*9.4ns cyc
    createConfigParam("Chop3:Delay",    'C', 0x3,  32,  0, 0); // Chop3 delay for N*9.4ns cyc
    createConfigParam("Chop4:Delay",    'C', 0x4,  32,  0, 0); // Chop4 delay for N*9.4ns cyc
    createConfigParam("Chop5:Delay",    'C', 0x5,  32,  0, 0); // Chop5 delay for N*9.4ns cyc
    createConfigParam("Chop6:Delay",    'C', 0x6,  32,  0, 0); // Chop6 delay for N*9.4ns cyc
    createConfigParam("Chop7:Delay",    'C', 0x7,  32,  0, 0); // Chop7 delay for N*9.4ns cyc

    createConfigParam("Chop0:Freq",     'C', 0x8,   4,  0, 0); // Chop0 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop1:Freq",     'C', 0x8,   4,  4, 1); // Chop1 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop2:Freq",     'C', 0x8,   4,  8, 2); // Chop2 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop3:Freq",     'C', 0x8,   4, 12, 3); // Chop3 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop4:Freq",     'C', 0x8,   4, 16, 4); // Chop4 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop5:Freq",     'C', 0x8,   4, 20, 5); // Chop5 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop6:Freq",     'C', 0x8,   4, 24, 6); // Chop6 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("Chop7:Freq",     'C', 0x8,   4, 28, 7); // Chop7 frequency selector     (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)

    createConfigParam("ChopDutyCycle",  'C', 0x9,  32,  0, 83400); // N*100ns ref pulse high
    createConfigParam("ChopMaxPeriod",  'C', 0xA,  32,  0, 166800); // N*100ns master/ref delay
    createConfigParam("ChopFixOffset",  'C', 0xB,  32,  0, 0); // Chopper TOF fixed offset       - todo: make proper record links to chopper ioc

    createConfigParam("ChopFr6",        'C', 0xC,   8,  0, 4); // RTDL Frame 6
    createConfigParam("ChopFr7",        'C', 0xC,   8,  8, 5); // RTDL Frame 7
    createConfigParam("ChopFr8",        'C', 0xC,   8, 16, 6); // RTDL Frame 8
    createConfigParam("ChopFr9",        'C', 0xC,   8, 24, 7); // RTDL Frame 9
    createConfigParam("ChopFr10",       'C', 0xD,   8,  0, 8); // RTDL Frame 10
    createConfigParam("ChopFr11",       'C', 0xD,   8,  8, 15); // RTDL Frame 11
    createConfigParam("ChopFr12",       'C', 0xD,   8, 16, 17); // RTDL Frame 12
    createConfigParam("ChopFr13",       'C', 0xD,   8, 24, 24); // RTDL Frame 13
    createConfigParam("ChopFr14",       'C', 0xE,   8,  0, 25); // RTDL Frame 14
    createConfigParam("ChopFr15",       'C', 0xE,   8,  8, 26); // RTDL Frame 15
    createConfigParam("ChopFr16",       'C', 0xE,   8, 16, 28); // RTDL Frame 16
    createConfigParam("ChopFr17",       'C', 0xE,   8, 24, 29); // RTDL Frame 17
    createConfigParam("ChopFr18",       'C', 0xF,   8,  0, 30); // RTDL Frame 18
    createConfigParam("ChopFr19",       'C', 0xF,   8,  8, 31); // RTDL Frame 19
    createConfigParam("ChopFr20",       'C', 0xF,   8, 16, 32); // RTDL Frame 20
    createConfigParam("ChopFr21",       'C', 0xF,   8, 24, 33); // RTDL Frame 21
    createConfigParam("ChopFr22",       'C', 0x10,  8,  0, 34); // RTDL Frame 22
    createConfigParam("ChopFr23",       'C', 0x10,  8,  8, 35); // RTDL Frame 23
    createConfigParam("ChopFr24",       'C', 0x10,  8, 16, 36); // RTDL Frame 24
    createConfigParam("ChopFr25",       'C', 0x10,  8, 24, 37); // RTDL Frame 25
    createConfigParam("ChopFr26",       'C', 0x11,  8,  0, 38); // RTDL Frame 26
    createConfigParam("ChopFr27",       'C', 0x11,  8,  8, 39); // RTDL Frame 27
    createConfigParam("ChopFr28",       'C', 0x11,  8, 16, 40); // RTDL Frame 28
    createConfigParam("ChopFr29",       'C', 0x11,  8, 24, 41); // RTDL Frame 29
    createConfigParam("ChopFr30",       'C', 0x12,  8,  0, 1); // RTDL Frame 30
    createConfigParam("ChopFr31",       'C', 0x12,  8,  8, 2); // RTDL Frame 31
// dcomserver thinks this one is valid
//    createConfigParam1("ChopRtdlFr32", 'C', 0x12,  8, 16, 3); // RTDL Frame 32

    createConfigParam("ChopTrefTrig",   'C', 0x13,  2,  0, 3); // Chopper TREF trigger select  (0=Extract,1=Cycle Start,2=Beam On,3=TREF event)
    createConfigParam("ChopTrefFreq",   'C', 0x13,  4,  2, 1); // TREF frequency select        (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("ChopRtdlOffset", 'C', 0x13,  4,  8, 0); // Chopper RTDL frame offset
    createConfigParam("ChopTrefEvent",  'C', 0x13,  8, 12, 39); // Chop TREF event trig [0:255]
    createConfigParam("ChopHystMinLow", 'C', 0x13,  4, 20, 4); // Chop HYST minimum low [0:7]
    createConfigParam("ChopHystMinHi",  'C', 0x13,  4, 24, 4); // Chop HYST minimum high [0:7]
    createConfigParam("ChopFreqCntCtrl", 'C', 0x13,  2, 28, 1); // Chop frequency count control (0=strobe at X, 1=strobe at X-1, 2=strobe at X-2)
    createConfigParam("ChopFreqCycle",  'C', 0x13,  1, 30, 1); // Chop frequency cycle select  (0=Present cycle number, 1=Next cycle number)
    createConfigParam("ChopSweepEn",    'C', 0x13,  1, 31, 0); // Chop sweep enable            (0=TOF fixed off,1=TOF fract off)

    createConfigParam("STsyncDelMax",   'C', 0x14, 32,  0, 0); // Synth mast strobe max delay
    createConfigParam("STsyncDelAdj",   'C', 0x15, 32,  0, 0); // Synth mast strobe delay adj
    createConfigParam("STsyncFractAdj", 'C', 0x16, 32,  0, 0); // Synth mast strobe fract adj
    createConfigParam("FakeTimeHigh",   'C', 0x17, 32,  0, 0); // Fake mode time high DWord

    // Meta parameters
    createConfigParam("Ch0:Mode",       'D', 0x0,   2,  0, 0); // Chan0 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch1:Mode",       'D', 0x0,   2,  2, 0); // Chan1 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch2:Mode",       'D', 0x0,   2,  4, 0); // Chan2 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch3:Mode",       'D', 0x0,   2,  6, 0); // Chan3 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch4:Mode",       'D', 0x0,   2,  8, 0); // Chan4 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch5:Mode",       'D', 0x0,   2, 10, 0); // Chan5 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch6:Mode",       'D', 0x0,   2, 12, 0); // Chan6 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch7:Mode",       'D', 0x0,   2, 14, 0); // Chan7 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch8:Mode",       'D', 0x0,   2, 16, 0); // Chan8 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch9:Mode",       'D', 0x0,   2, 18, 0); // Chan9 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch10:Mode",      'D', 0x0,   2, 20, 0); // Chan10 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch11:Mode",      'D', 0x0,   2, 22, 0); // Chan11 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch12:Mode",      'D', 0x0,   2, 24, 0); // Chan12 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch13:Mode",      'D', 0x0,   2, 26, 0); // Chan13 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch14:Mode",      'D', 0x0,   2, 28, 0); // Chan14 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch15:Mode",      'D', 0x0,   2, 30, 0); // Chan15 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch16:Mode",      'D', 0x1,   2,  0, 0); // Chan16 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch17:Mode",      'D', 0x1,   2,  2, 0); // Chan17 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch18:Mode",      'D', 0x1,   2,  4, 0); // Chan18 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch19:Mode",      'D', 0x1,   2,  6, 0); // Chan19 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch20:Mode",      'D', 0x1,   2,  8, 0); // Chan20 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch21:Mode",      'D', 0x1,   2, 10, 0); // Chan21 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch22:Mode",      'D', 0x1,   2, 12, 0); // Chan22 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch23:Mode",      'D', 0x1,   2, 14, 0); // Chan23 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch24:Mode",      'D', 0x1,   2, 16, 0); // Chan24 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch25:Mode",      'D', 0x1,   2, 18, 0); // Chan25 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch26:Mode",      'D', 0x1,   2, 20, 0); // Chan26 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch27:Mode",      'D', 0x1,   2, 22, 0); // Chan27 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch28:Mode",      'D', 0x1,   2, 24, 0); // Chan28 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch29:Mode",      'D', 0x1,   2, 26, 0); // Chan29 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch30:Mode",      'D', 0x1,   2, 28, 0); // Chan30 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Ch31:Mode",      'D', 0x1,   2, 30, 0); // Chan31 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)

    createConfigParam("Ch0:PixId",      'D', 0x2,  32,  0, 0x50000000); // Chan0 edge pixel id
    createConfigParam("Ch1:PixId",      'D', 0x3,  32,  0, 0x50000002); // Chan1 edge pixel id
    createConfigParam("Ch2:PixId",      'D', 0x4,  32,  0, 0x50000004); // Chan2 edge pixel id
    createConfigParam("Ch3:PixId",      'D', 0x5,  32,  0, 0x50000006); // Chan3 edge pixel id
    createConfigParam("Ch4:PixId",      'D', 0x6,  32,  0, 0x50000008); // Chan4 edge pixel id
    createConfigParam("Ch5:PixId",      'D', 0x7,  32,  0, 0x5000000a); // Chan5 edge pixel id
    createConfigParam("Ch6:PixId",      'D', 0x8,  32,  0, 0x5000000c); // Chan6 edge pixel id
    createConfigParam("Ch7:PixId",      'D', 0x9,  32,  0, 0x5000000e); // Chan7 edge pixel id
    createConfigParam("Ch8:PixId",      'D', 0xA,  32,  0, 0x50000010); // Chan8 edge pixel id
    createConfigParam("Ch9:PixId",      'D', 0xB,  32,  0, 0x50000012); // Chan9 edge pixel id
    createConfigParam("Ch10:PixId",     'D', 0xC,  32,  0, 0x50000014); // Chan10 edge pixel id
    createConfigParam("Ch11:PixId",     'D', 0xD,  32,  0, 0x50000016); // Chan11 edge pixel id
    createConfigParam("Ch12:PixId",     'D', 0xE,  32,  0, 0x50000018); // Chan12 edge pixel id
    createConfigParam("Ch13:PixId",     'D', 0xF,  32,  0, 0x5000001a); // Chan13 edge pixel id
    createConfigParam("Ch14:PixId",     'D', 0x10, 32,  0, 0x5000001c); // Chan14 edge pixel id
    createConfigParam("Ch15:PixId",     'D', 0x11, 32,  0, 0x5000001e); // Chan15 edge pixel id
    createConfigParam("Ch16:PixId",     'D', 0x12, 32,  0, 0x50000020); // Chan16 edge pixel id
    createConfigParam("Ch17:PixId",     'D', 0x13, 32,  0, 0x50000022); // Chan17 edge pixel id
    createConfigParam("Ch18:PixId",     'D', 0x14, 32,  0, 0x50000024); // Chan18 edge pixel id
    createConfigParam("Ch19:PixId",     'D', 0x15, 32,  0, 0x500000026); // Chan19 edge pixel id
    createConfigParam("Ch20:PixId",     'D', 0x16, 32,  0, 0x500000028); // Chan20 edge pixel id
    createConfigParam("Ch21:PixId",     'D', 0x17, 32,  0, 0x50000002a); // Chan21 edge pixel id
    createConfigParam("Ch22:PixId",     'D', 0x18, 32,  0, 0x50000002c); // Chan22 edge pixel id
    createConfigParam("Ch23:PixId",     'D', 0x19, 32,  0, 0x50000002e); // Chan23 edge pixel id
    createConfigParam("Ch24:PixId",     'D', 0x1A, 32,  0, 0x500000030); // Chan24 edge pixel id
    createConfigParam("Ch25:PixId",     'D', 0x1B, 32,  0, 0x500000032); // Chan25 edge pixel id
    createConfigParam("Ch26:PixId",     'D', 0x1C, 32,  0, 0x500000034); // Chan26 edge pixel id
    createConfigParam("Ch27:PixId",     'D', 0x1D, 32,  0, 0x500000036); // Chan27 edge pixel id
    createConfigParam("Ch28:PixId",     'D', 0x1E, 32,  0, 0x500000038); // Chan28 edge pixel id
    createConfigParam("Ch29:PixId",     'D', 0x1F, 32,  0, 0x50000003a); // Chan29 edge pixel id
    createConfigParam("Ch30:PixId",     'D', 0x20, 32,  0, 0x50000003c); // Chan30 edge pixel id
    createConfigParam("Ch31:PixId",     'D', 0x21, 32,  0, 0x50000003e); // Chan31 edge pixel id

    createConfigParam("Ch0:Cycle",      'D', 0x22,  5,  0, 0); // Chan0 edge cycle number adj
    createConfigParam("Ch1:Cycle",      'D', 0x22,  5,  5, 0); // Chan1 edge cycle number adj
    createConfigParam("Ch2:Cycle",      'D', 0x22,  5, 10, 0); // Chan2 edge cycle number adj
    createConfigParam("Ch3:Cycle",      'D', 0x22,  5, 15, 0); // Chan3 edge cycle number adj
    createConfigParam("Ch4:Cycle",      'D', 0x22,  5, 20, 0); // Chan4 edge cycle number adj
    createConfigParam("Ch5:Cycle",      'D', 0x22,  5, 25, 0); // Chan5 edge cycle number adj
    createConfigParam("Ch6:Cycle",      'D', 0x23,  5,  0, 0); // Chan6 edge cycle number adj
    createConfigParam("Ch7:Cycle",      'D', 0x23,  5,  5, 0); // Chan7 edge cycle number adj
    createConfigParam("Ch8:Cycle",      'D', 0x23,  5, 10, 0); // Chan8 edge cycle number adj
    createConfigParam("Ch9:Cycle",      'D', 0x23,  5, 15, 0); // Chan9 edge cycle number adj
    createConfigParam("Ch10:Cycle",     'D', 0x23,  5, 20, 0); // Chan10 edge cycle number adj
    createConfigParam("Ch11:Cycle",     'D', 0x23,  5, 25, 0); // Chan11 edge cycle number adj
    createConfigParam("Ch12:Cycle",     'D', 0x24,  5,  0, 0); // Chan12 edge cycle number adj
    createConfigParam("Ch13:Cycle",     'D', 0x24,  5,  5, 0); // Chan13 edge cycle number adj
    createConfigParam("Ch14:Cycle",     'D', 0x24,  5, 10, 0); // Chan14 edge cycle number adj
    createConfigParam("Ch15:Cycle",     'D', 0x24,  5, 15, 0); // Chan15 edge cycle number adj
    createConfigParam("Ch16:Cycle",     'D', 0x24,  5, 20, 0); // Chan16 edge cycle number adj
    createConfigParam("Ch17:Cycle",     'D', 0x24,  5, 25, 0); // Chan17 edge cycle number adj
    createConfigParam("Ch18:Cycle",     'D', 0x25,  5,  0, 0); // Chan18 edge cycle number adj
    createConfigParam("Ch19:Cycle",     'D', 0x25,  5,  5, 0); // Chan19 edge cycle number adj
    createConfigParam("Ch20:Cycle",     'D', 0x25,  5, 10, 0); // Chan20 edge cycle number adj
    createConfigParam("Ch21:Cycle",     'D', 0x25,  5, 15, 0); // Chan21 edge cycle number adj
    createConfigParam("Ch22:Cycle",     'D', 0x25,  5, 20, 0); // Chan22 edge cycle number adj
    createConfigParam("Ch23:Cycle",     'D', 0x25,  5, 25, 0); // Chan23 edge cycle number adj
    createConfigParam("Ch24:Cycle",     'D', 0x26,  5,  0, 0); // Chan24 edge cycle number adj
    createConfigParam("Ch25:Cycle",     'D', 0x26,  5,  5, 0); // Chan25 edge cycle number adj
    createConfigParam("Ch26:Cycle",     'D', 0x26,  5, 10, 0); // Chan26 edge cycle number adj
    createConfigParam("Ch27:Cycle",     'D', 0x26,  5, 15, 0); // Chan27 edge cycle number adj
    createConfigParam("Ch28:Cycle",     'D', 0x26,  5, 20, 0); // Chan28 edge cycle number adj
    createConfigParam("Ch29:Cycle",     'D', 0x26,  5, 25, 0); // Chan29 edge cycle number adj
    createConfigParam("Ch30:Cycle",     'D', 0x27,  5,  0, 0); // Chan30 edge cycle number adj
    createConfigParam("Ch31:Cycle",     'D', 0x27,  5,  5, 0); // Chan31 edge cycle number adj

    createConfigParam("Ch0:Delay",      'D', 0x28, 32,  0, 0); // Chan0 edge delay
    createConfigParam("Ch1:Delay",      'D', 0x29, 32,  0, 0); // Chan1 edge delay
    createConfigParam("Ch2:Delay",      'D', 0x2A, 32,  0, 0); // Chan2 edge delay
    createConfigParam("Ch3:Delay",      'D', 0x2B, 32,  0, 0); // Chan3 edge delay
    createConfigParam("Ch4:Delay",      'D', 0x2C, 32,  0, 0); // Chan4 edge delay
    createConfigParam("Ch5:Delay",      'D', 0x2D, 32,  0, 0); // Chan5 edge delay
    createConfigParam("Ch6:Delay",      'D', 0x2E, 32,  0, 0); // Chan6 edge delay
    createConfigParam("Ch7:Delay",      'D', 0x2F, 32,  0, 0); // Chan7 edge delay
    createConfigParam("Ch8:Delay",      'D', 0x30, 32,  0, 0); // Chan8 edge delay
    createConfigParam("Ch9:Delay",      'D', 0x31, 32,  0, 0); // Chan9 edge delay
    createConfigParam("Ch10:Delay",     'D', 0x32, 32,  0, 0); // Chan10 edge delay
    createConfigParam("Ch11:Delay",     'D', 0x33, 32,  0, 0); // Chan11 edge delay
    createConfigParam("Ch12:Delay",     'D', 0x34, 32,  0, 0); // Chan12 edge delay
    createConfigParam("Ch13:Delay",     'D', 0x35, 32,  0, 0); // Chan13 edge delay
    createConfigParam("Ch14:Delay",     'D', 0x36, 32,  0, 0); // Chan14 edge delay
    createConfigParam("Ch15:Delay",     'D', 0x37, 32,  0, 0); // Chan15 edge delay
    createConfigParam("Ch16:Delay",     'D', 0x38, 32,  0, 0); // Chan16 edge delay
    createConfigParam("Ch17:Delay",     'D', 0x39, 32,  0, 0); // Chan17 edge delay
    createConfigParam("Ch18:Delay",     'D', 0x3A, 32,  0, 0); // Chan18 edge delay
    createConfigParam("Ch19:Delay",     'D', 0x3B, 32,  0, 0); // Chan19 edge delay
    createConfigParam("Ch20:Delay",     'D', 0x3C, 32,  0, 0); // Chan20 edge delay
    createConfigParam("Ch21:Delay",     'D', 0x3D, 32,  0, 0); // Chan21 edge delay
    createConfigParam("Ch22:Delay",     'D', 0x3E, 32,  0, 0); // Chan22 edge delay
    createConfigParam("Ch23:Delay",     'D', 0x3F, 32,  0, 0); // Chan23 edge delay
    createConfigParam("Ch24:Delay",     'D', 0x40, 32,  0, 0); // Chan24 edge delay
    createConfigParam("Ch25:Delay",     'D', 0x41, 32,  0, 0); // Chan25 edge delay
    createConfigParam("Ch26:Delay",     'D', 0x42, 32,  0, 0); // Chan26 edge delay
    createConfigParam("Ch27:Delay",     'D', 0x43, 32,  0, 0); // Chan27 edge delay
    createConfigParam("Ch28:Delay",     'D', 0x44, 32,  0, 0); // Chan28 edge delay
    createConfigParam("Ch29:Delay",     'D', 0x45, 32,  0, 0); // Chan29 edge delay
    createConfigParam("Ch30:Delay",     'D', 0x46, 32,  0, 0); // Chan30 edge delay
    createConfigParam("Ch31:Delay",     'D', 0x47, 32,  0, 0); // Chan31 edge delay

    // LVDS & optical parameters
    createConfigParam("Ch1:Flow",       'E', 0x0,  1,  0, 0); // LVDS chan0 TXEN control       (0=flow control,1=no flow control)
    createConfigParam("Ch2:Flow",       'E', 0x0,  1,  3, 0); // LVDS chan1 TXEN control       (0=flow control,1=no flow control)
    createConfigParam("Ch3:Flow",       'E', 0x0,  1,  6, 0); // LVDS chan2 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Ch4:Flow",       'E', 0x0,  1,  9, 0); // LVDS chan3 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Ch5:Flow",       'E', 0x0,  1, 12, 0); // LVDS chan4 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Ch6:Flow",       'E', 0x0,  1, 15, 0); // LVDS chan5 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Ch1:IgnE",       'E', 0x0,  1,  1, 0); // LVDS chan0 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Ch2:IgnE",       'E', 0x0,  1,  4, 0); // LVDS chan1 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Ch3:IgnE",       'E', 0x0,  1,  7, 0); // LVDS chan2 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Ch4:IgnE",       'E', 0x0,  1, 10, 0); // LVDS chan3 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Ch5:IgnE",       'E', 0x0,  1, 13, 0); // LVDS chan4 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Ch6:IgnE",       'E', 0x0,  1, 16, 0); // LVDS chan5 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Ch1:Dis",        'E', 0x0,  1,  2, 0); // LVDS chan0 disable            (0=enable,1=disable)
    createConfigParam("Ch2:Dis",        'E', 0x0,  1,  5, 0); // LVDS chan1 disable            (0=enable,1=disable)
    createConfigParam("Ch3:Dis",        'E', 0x0,  1,  8, 0); // LVDS chan2 disable            (0=enable,1=disable)
    createConfigParam("Ch4:Dis",        'E', 0x0,  1, 11, 0); // LVDS chan3 disable            (0=enable,1=disable)
    createConfigParam("Ch5:Dis",        'E', 0x0,  1, 14, 0); // LVDS chan4 disable            (0=enable,1=disable)
    createConfigParam("Ch6:Dis",        'E', 0x0,  1, 17, 0); // LVDS chan5 disable            (0=enable,1=disable)
    createConfigParam("LvdsCmdMode",    'E', 0x0,  1, 18, 0); // LVDS command parser mode      (0=as command,1=as data)
    createConfigParam("LvdsDataMode",   'E', 0x0,  1, 19, 0); // LVDS data parser mode         (0=as data,1=as command)
    createConfigParam("LvdsDataSize",   'E', 0x0,  8, 20, 4); // LVDS data pkt num words
    createConfigParam("LvdsPowerCtrl",  'E', 0x0,  1, 28, 1); // Power Down during reset       (0=power down,1=power up)
    createConfigParam("LvdsPowerRst",   'E', 0x0,  1, 29, 1); // Execite power down sequence   (0=execute,1=bypass)
    createConfigParam("LvdsFilterAll",  'E', 0x0,  2, 30, 0); // Filter all commands

    createConfigParam("LvdsFilterCmd",  'E', 0x1, 16,  0, 0); // LVDS command to filter
    createConfigParam("LvdsFilterMask", 'E', 0x1, 16, 16, 65535); // LVDS command filter mask

//      BLXXX:Det:DspX:| sig nam|                            | EPICS record description | (bi and mbbi description)
    createConfigParam("LvdsTxCtrl",     'E', 0x2,  1,  0, 0); // LVDS TX control TCLK mode    (0=TCLK from int,1=TCLK from LVDS)
    createConfigParam("LvdsTxTclk",     'E', 0x2,  2,  2, 0); // LVDS TX control T&C TCLK mod (0=TCLK,1=TCLK,2=always 0,3=always 1)
    createConfigParam("LvdsTsyncMode",  'E', 0x2,  2,  4, 1); // LVDS TSYNC_O mode            (0=local TSYNC,1=TSYNC from TREF,2=TSYNC from LVDS,3=TSYNC from opt)
    createConfigParam("LvdsTsCtrl",     'E', 0x2,  2,  6, 0); // LVDS TSYNC_NORMAL control    (0=polarity,1=TSYNC WIDTH,2=always 0,3=always 1)
    createConfigParam("LvdsSysrstCtrl", 'E', 0x2,  2,  8, 0); // LVDS T&C SYSRST# buffer ctrl (0=sysrst,1=sysrst,2=always 0,3=always 1)
    createConfigParam("LvdsTxenCttrl",  'E', 0x2,  2, 10, 0); // LVDS T&C TXEN# control       (0=ChLnk parser,1=ChLnk parser,2=ChLnk RX,3=ChLnk inv RX)
    createConfigParam("LvdsOutClck",    'E', 0x2,  2, 16, 0); // LVDS output clock mode
    createConfigParam("LvdsNRetry",     'E', 0x2,  2, 18, 3); // LVDS downstream retrys
    createConfigParam("Ch1:WordLen",    'E', 0x2,  1, 20, 0); // LVDS chan1 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Ch2:WordLen",    'E', 0x2,  1, 21, 0); // LVDS chan2 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Ch3:WordLen",    'E', 0x2,  1, 22, 0); // LVDS chan3 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Ch4:WordLen",    'E', 0x2,  1, 23, 0); // LVDS chan4 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Ch5:WordLen",    'E', 0x2,  1, 24, 0); // LVDS chan5 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Ch6:WordLen",    'E', 0x2,  1, 25, 0); // LVDS chan6 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("LvdsClkMargin",  'E', 0x2,  2, 26, 0); // LVDS clock margin
    createConfigParam("LvdsTstPattern", 'E', 0x2,  1, 30, 0); // LVDS T&C test pattern        (0=disable,1=enable)
    createConfigParam("LvdsTestEn",     'E', 0x2,  1, 31, 0); // LVDS test enable             (0=disable,1=enable)

    createConfigParam("Ch1:SrcCtrl",    'E', 0x3,  2,  0, 0); // LVDS ch1 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Ch2:SrcCtrl",    'E', 0x3,  2,  2, 0); // LVDS ch2 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Ch3:SrcCtrl",    'E', 0x3,  2,  4, 0); // LVDS ch3 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Ch4:SrcCtrl",    'E', 0x3,  2,  6, 0); // LVDS ch4 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Ch5:SrcCtrl",    'E', 0x3,  2,  8, 0); // LVDS ch5 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Ch6:SrcCtrl",    'E', 0x3,  2, 10, 0); // LVDS ch6 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("LvdsTsMeta",     'E', 0x3,  2, 14, 2); // LVDS TSYNC metadata src ctrl (0=RTDL,1=LVDS,2=detector TSYNC,3=OFB[0])

    createConfigParam("LvdsTsyncGen",   'E', 0x4, 32,  0, 166660); // LVDS TSYNC generation divisor   - 40MHz/this value to obtain TSYNC period
    createConfigParam("LvdsTsyncDelay", 'E', 0x5, 32,  0, 0); // LVDS TSYNC delay divisor            - 106.25MHz/this value
    createConfigParam("LvdsTsyncWidth", 'E', 0x6, 32,  0, 83330); // LVDS TSYNC width divisor            - 10MHz/this value

    createConfigParam("OptA:CrossA",    'E', 0x8,  2,  2, 1); // Crossbar Switch Pass ctrl A  (1=Send to trans A,2=send to trans B)
    createConfigParam("OptB:CrossB",    'E', 0x8,  2, 10, 0); // Crossbar Switch Pass ctrl B  (1=Send to trans A,2=send to trans B)
    createConfigParam("OptA:TxMode",    'E', 0x8,  2,  0, 0); // Optical TX A output mode     (0=Normal,1=Timing,2=Chopper,3=Timing master)
    createConfigParam("OptA:EOC",       'E', 0x8,  1,  4, 1); // Optical TX A End of Chain
    createConfigParam("OptA:FilterCmd", 'E', 0x8,  2,  5, 0); // Optical TX A Command Filter
    createConfigParam("OptB:TxMode",    'E', 0x8,  2,  8, 3); // Optical TX B output mode     (0=Normal,1=Timing,2=Chopper,3=Timing master)
    createConfigParam("OptB:EOC",       'E', 0x8,  1, 12, 1); // Optical TX B End of Chain
    createConfigParam("OptB:FilterCmd", 'E', 0x8,  2, 13, 0); // Optical TX B Command Filter
    createConfigParam("OptHystEn",      'E', 0x8,  1, 16, 0); // Optical hysteresis enable    (0=from TLK data,1=match optical)
    createConfigParam("OptBlankEn",     'E', 0x8,  1, 17, 0); // Optical empty data frame CRC (0=no blank frame, 1=add blank frame)
    createConfigParam("OptTxDelay",     'E', 0x8,  7, 24, 3); // Optical packet send delay           - Number of 313ns cycles to wait between DSP packet transmissions
    createConfigParam("OptTxDelayC",    'E', 0x8,  1, 31, 1); // Optical packet send delay ct (0=use OPT_TX_DELAY,1=prev word count)

    createConfigParam("OptMaxSize",     'E', 0x9, 16,  0, 16111); // Optical packet max dwords
    createConfigParam("OptNeutronEop",  'E', 0x9,  1, 16, 1); // Optical Neutron send EOP     (0=disabled,1=enabled)
    createConfigParam("OptMetaEop",     'E', 0x9,  1, 17, 0); // Optical Metadata send EOP    (0=disabled,1=enabled)
    createConfigParam("OptTofCtrl",     'E', 0x9,  1, 18, 1); // TOF control                  (0=fixed TOF,1=full time offset)

    createConfigParam("FakeTrig",       'E', 0xA, 32,  0, 2621480); // Fake Trigger Information

    createConfigParam("SysRstMode",     'F', 0x0,  2,  0, 0); // Reset mode => SYSRST_O#      (0=not used,1=not used,2=from LVDS T&C,3=from optical T&C)
    createConfigParam("SysStartMode",   'F', 0x0,  3,  4, 0); // Start/Stop mode              (0=normal, 1=fake data mode,2=not defined,3=not defined)
    createConfigParam("SysFakeEn",      'F', 0x0,  1,  7, 0); // Fake metadata trigger enable (0=disabled,1=enabled)
    createConfigParam("SysFastEn",      'F', 0x0,  1,  8, 0); // Send data immediately switch (0=big packets,1=send immediately)
    createConfigParam("SysPassEn",      'F', 0x0,  1,  9, 0); // Response for passthru cmd    (0=don't send,1=send)
    createConfigParam("SysStartEn",     'F', 0x0,  1, 10, 1); // Wait for Start/Stop response (0=don't wait,1=wait)
    createConfigParam("SysRtdlMode",    'F', 0x0,  2, 12, 1); // RTDL mode                    (0=no RTDL,1=master,2=slave,3=fake mode)
    createConfigParam("SysRtdlAEn",     'F', 0x0,  1, 14, 1); // RTDL port A output enable    (0=disable,1=enable)
    createConfigParam("SysRtdlBEn",     'F', 0x0,  1, 15, 1); // RTDL port B output enable    (0=disable,1=enable)
    createConfigParam("SysTofOffsetEn", 'F', 0x0,  1, 16, 0); // Enable TOF full offset       (0=disable,1=enable)
    createConfigParam("SysFifoSync",    'F', 0x0,  1, 17, 0); // FIFO sync switch             (0=disable,1=enable)
    createConfigParam("SysRtdlData",    'F', 0x0,  1, 18, 1); // Send RTDL command as data    (0=disable,1=enable)
    createConfigParam("SysFixRtdlEn",   'F', 0x0,  1, 19, 1); // Correct RTDL information     (0=disable,1=enable)
    createConfigParam("SysBadPktEn",    'F', 0x0,  1, 30, 0); // Send bad packets             (0=disable,1=enable)
    createConfigParam("SysReset",       'F', 0x0,  1, 31, 0); // Force system reset           (0=disable,1=enable)

    createConfigParam("TestPatternId",  'F', 0x1, 12, 0,  0); // Test pattern id
    createConfigParam("TestPatternEn",  'F', 0x1,  1, 15, 0); // pattern enable               (0=disable,1=enable)
    createConfigParam("TestPatternRate",'F', 0x1, 16, 16, 0); // Test pattern rate

}

void DspPlugin::createStatusParams_v64()
{
//      BLXXX:Det:DspX:| sig nam|                     | EPICS record description | (bi and mbbi description)
    createStatusParam("Configured",    0x0,  1,  0); // Configured                   (0=not configured,1=configured)
    createStatusParam("Acquiring",     0x0,  1,  1); // Acquiring data               (0=not acquiring,1=acquiring)
    createStatusParam("ErrProgram",    0x0,  1,  2); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("ErrLength",     0x0,  1,  3); // Packet length error          (0=no error,1=error)
    createStatusParam("ErrCmdBad",     0x0,  1,  4); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("TxFifoFull",    0x0,  1,  5); // LVDS TxFIFO went full        (0=not full,1=full)
    createStatusParam("CmdErr",        0x0,  1,  6); // LVDS command error           (0=no error,1=error)
    createStatusParam("EepromInit",    0x0,  1,  7); // EEPROM initialization status (0=not ok,1=ok)
    createStatusParam("OptA:TxStat",   0x0,  5, 16); // Optical port A TX status
    createStatusParam("OptA:TxOut",    0x0,  2, 22); // Optical port A TX output
    createStatusParam("OptB:TxStat",   0x0,  5, 24); // Optical port B TX status
    createStatusParam("OptB:TxOut",    0x0,  2, 30); // Optical port B TX output

    createStatusParam("OptA:ErrRxCnt", 0x1,  8,  0); // RX A errors count
    createStatusParam("OptA:ErrFlags", 0x1, 13,  8); // Error flags                  (8=packet timeout,9=SOF/address sw,10=EOF/address sw,11=SOF/hdr sw,12=EOF/hdr sw,13=SOF/payload sw,14=EOF/payload sw,15=SOF/CRC switch,16=EOF/CRC switch,17=CRC low word,18=CRC high word,19=pri FIFO al full,20=sec FIFO al full)
    createStatusParam("OptA:LastGood", 0x1,  1, 21); // Last packet was good         (0=no,1=yes)
    createStatusParam("OptA:PriS",     0x1,  1, 23); // Stack FIFO Not Empty         (0=empty,1=not empty)
    createStatusParam("OptA:PriF",     0x1,  1, 24); // Stack FIFO Almost Full       (0=not full,1=almost full)
    createStatusParam("OptA:SecS",     0x1,  1, 25); // Secondary FIFO Not Empty     (0=empty,1=not empty)
    createStatusParam("OptA:SecF",     0x1,  1, 26); // Secondary FIFO Almost Full   (0=not full,1=almost full)
    createStatusParam("OptA:PassS",    0x1,  1, 27); // PassThrough FIFO Not Empty   (0=empty,1=not empty)
    createStatusParam("OptA:PassF",    0x1,  1, 28); // PassThrough FIFO Almost Full (0=not full,1=almost full)
    createStatusParam("OptA:Time",     0x1,  1, 29); // Timeout Pri/Sec FIFO transfr (0=no timeout,1=timeout)
    createStatusParam("OptA:Stack",    0x1,  1, 30); // RX pkt while stack almost fu (0=no,1=yes)
    createStatusParam("OptA:Fifo",     0x1,  1, 31); // RX while the FIFO almost ful

    createStatusParam("OptB:ErrRxCnt", 0x2,  8,  0); // RX B errors count
    createStatusParam("OptB:ErrFlags", 0x2, 13,  8); // Error flags                  (8=packet timeout,9=SOF/address sw,10=EOF/address sw,11=SOF/hdr sw,12=EOF/hdr sw,13=SOF/payload sw,14=EOF/payload sw,15=SOF/CRC switch,16=EOF/CRC switch,17=CRC low word,18=CRC high word,19=pri FIFO al full,20=sec FIFO al full)
    createStatusParam("OptB:LastGood", 0x2,  1, 21); // Last packet was good         (0=no,1=yes)
    createStatusParam("OptB:PriS",     0x2,  1, 22); // Stack FIFO Not Empty         (0=empty,1=not empty)
    createStatusParam("OptB:PriF",     0x2,  1, 24); // Stack FIFO Almost Full       (0=not full,1=almost full)
    createStatusParam("OptB:SecS",     0x2,  1, 25); // Secondary FIFO Not Empty     (0=empty,1=not empty)
    createStatusParam("OptB:SecF",     0x2,  1, 26); // Secondary FIFO Almost Full   (0=not full,1=almost full)
    createStatusParam("OptB:PassS",    0x2,  1, 27); // PassThrough FIFO Not Empty   (0=empty,1=not empty)
    createStatusParam("OptB:PassF",    0x2,  1, 28); // PassThrough FIFO Almost Full (0=not full,1=almost full)
    createStatusParam("OptB:Time",     0x2,  1, 29); // Timeout Pri/Sec FIFO transfr (0=no timeout,1=timeout)
    createStatusParam("OptB:Stack",    0x2,  1, 30); // RX pkt while stack almost fu (0=no,1=yes)
    createStatusParam("OptB:Fifo",     0x2,  1, 31); // RX while the FIFO almost ful

    createStatusParam("Ch1:RxFlg",     0x3,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch1:RxStat",    0x3,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Ch1:RxData",    0x3,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Ch1:RxAF",      0x3,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Ch1:RxParD",    0x3,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Ch1:RxParF",    0x3,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Ch1:ExtEn",     0x3,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Ch1:ParsEn",    0x3,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Ch1:ErrCnt",    0x3, 16, 16); // Data packet errors count

    createStatusParam("Ch2:RxFlags",   0x4,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch2:RxStat",    0x4,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Ch2:RxData",    0x4,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Ch2:RxAF",      0x4,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Ch2:RxParD",    0x4,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Ch2:RxParF",    0x4,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Ch2:ExtEn",     0x4,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Ch2:ParsEn",    0x4,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Ch2:ErrCnt",    0x4, 16, 16); // Data packet errors count

    createStatusParam("Ch3:RxFlags",   0x5,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch3:RxStat",    0x5,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Ch3:RxData",    0x5,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Ch3:RxAF",      0x5,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Ch3:RxParD",    0x5,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Ch3:RxParF",    0x5,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Ch3:ExtEn",     0x5,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Ch3:ParsEn",    0x5,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Ch3:ErrCnt",    0x5, 16, 16); // Data packet errors count

    createStatusParam("Ch4:RxFlags",   0x6,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch4:RxStat",    0x6,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Ch4:RxData",    0x6,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Ch4:RxAF",      0x6,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Ch4:RxParD",    0x6,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Ch4:RxParF",    0x6,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Ch4:ExtEn",     0x6,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Ch4:ParsEn",    0x6,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Ch4:ErrCnt",    0x6, 16, 16); // Data packet errors count

    createStatusParam("Ch5:RxFlags",   0x7,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch5:RxStat",    0x7,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Ch5:RxData",    0x7,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Ch5:RxAF",      0x7,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Ch5:RxParD",    0x7,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Ch5:RxParF",    0x7,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Ch5:ExtEn",     0x7,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Ch5:ParsEn",    0x7,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Ch5:ErrCnt",    0x7, 16, 16); // Data packet errors count

    createStatusParam("Ch6:RxFlags",   0x8,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Ch6:RxStat",    0x8,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Ch6:RxData",    0x8,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Ch6:RxAF",      0x8,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Ch6:RxParD",    0x8,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Ch6:RxParF",    0x8,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Ch6:ExtEn",     0x8,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Ch6:ParsEn",    0x8,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Ch6:ErrCnt",    0x8, 16, 16); // Data packet errors count

    createStatusParam("CntGood",       0x9, 32,  0); // Good data packet count

    createStatusParam("CntCmds",       0xA, 16,  0); // Filtered command count
    createStatusParam("CntAcks",       0xA,  8, 16); // Filtered ACKSs count
    createStatusParam("CntNacks",      0xA,  8, 24); // Filtered NACKs count

    createStatusParam("NumHwIds",      0xB,  8,  0); // Detected Hardware IDs count
    createStatusParam("NewHwId",       0xB,  1,  9); // New Hardware ID detected
    createStatusParam("MissHwId",      0xB,  1, 10); // Missing Hardware ID
    createStatusParam("CmdData",       0xB,  1, 12); // Sorter cmd FIFO has data     (0=empty,1=has data)
    createStatusParam("CmdAF",         0xB,  1, 13); // Sorter cmd FIFO almost full  (0=not full,1=almost full)
    createStatusParam("RxData",        0xB,  1, 14); // Channel Link cmd FIFO has da (0=empty,1=has data)
    createStatusParam("RxAF",          0xB,  1, 15); // Channel Link cmd FIFO al. fu (0=not full,1=almost full)
    createStatusParam("OptA:CrSw",     0xB,  2, 16); // Crossbar Switch A pass ctrl
    createStatusParam("OptB:CrSw",     0xB,  2, 18); // Crossbar Switch B pass ctrl

    createStatusParam("ClkMinDelay",   0xC, 32,  0); // Min N*40MHz TSYNC/ref delay
    createStatusParam("ClkMaxDelay",   0xD, 32,  0); // Max N*40MHz TSYNC/ref delay
    createStatusParam("EepromStat",    0xE, 32,  0); // EEPROM Status
    createStatusParam("LvdsStatus",    0xF, 32,  0); // LVDS status
    createStatusParam("MetaInfo0",    0x10, 32,  0); // Metadata channel info
    createStatusParam("MetaInfo1",    0x11, 32,  0); // Metadata channel info
    createStatusParam("MetaInfo2",    0x12, 32,  0); // Metadata channel info
    createStatusParam("DetailInfo",   0x13, 32,  0); // Detailed info
    createStatusParam("TofOffset",    0x14, 32,  0); // TOF offset
    createStatusParam("RtdlInfo",     0x15, 32,  0); // RTDL info

    createStatusParam("CntBadRtdl",   0x16, 16,  0); // RTDL frame CRC errors count
    createStatusParam("CntBadData",   0x16, 16, 16); // Ev Link frame CRC errors cnt
}
