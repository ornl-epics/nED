/* DspPlugin_v64.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspPlugin.h"

void DspPlugin::createParams_v66() {
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

    createConfigParam("ChopDutyCycle",  'C', 0x9,  32,  0, 83400); // Ref pulse hold time      (scale:100,unit:ns)
    createConfigParam("ChopMaxPeriod",  'C', 0xA,  32,  0, 166800); // Ref pulse delay         (scale:100,unit:ns)
    createConfigParam("TofFixOffset",   'C', 0xB,  32,  0, 0); // TOF fixed offset             (scale:100,unit:ns)

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

    createConfigParam("TrefTrigger",    'C', 0x13,  2,  0, 3); // TREF RTDL Strobe trigger     (0=Extract,1=Cycle Start,2=Beam On,3=TREF event)
    createConfigParam("TsyncFreq",      'C', 0x13,  4,  2, 1); // Out TSYNC frequency          (0=60Hz,1=30Hz,2=20Hz,3=15Hz,4=12.5Hz,5=10Hz,6=7.5Hz,7=6Hz,8=5Hz,9=4Hz,10=3Hz,11=2.4Hz,12=2Hz,13=1.5Hz,14=1.25Hz,15=1Hz)
    createConfigParam("TsyncFrame",     'C', 0x13,  4,  8, 0); // TSYNC frame offset
    createConfigParam("TrefEvent",      'C', 0x13,  8, 12, 39); // TREF event number
    createConfigParam("HystMinLow",     'C', 0x13,  4, 20, 4); // Chop HYST minimum low
    createConfigParam("HystMinHi",      'C', 0x13,  4, 24, 4); // Chop HYST minimum high
    createConfigParam("ChopFreqCtrl",   'C', 0x13,  2, 28, 1); // Chop frequency count mode    (0=strobe at X, 1=strobe at X-1, 2=strobe at X-2)
    createConfigParam("ChopFreqCycle",  'C', 0x13,  1, 30, 1); // Chop frequency cycle select  (0=Present cycle number, 1=Next cycle number)
    createConfigParam("ChopSweepEn",    'C', 0x13,  1, 31, 0); // Chop sweep enable            (0=TOF fixed off,1=TOF fract off)

    createConfigParam("STsyncDelMax",   'C', 0x14, 32,  0, 0); // Synth mast strobe max delay
    createConfigParam("STsyncDelAdj",   'C', 0x15, 32,  0, 0); // Synth mast strobe delay adj
    createConfigParam("STsyncFractAdj", 'C', 0x16, 32,  0, 0); // Synth mast strobe fract adj
    createConfigParam("FakeTimeHigh",   'C', 0x17, 32,  0, 0); // Fake mode time high DWord

    // Meta parameters
    createConfigParam("Meta0:Mode",     'D', 0x0,   2,  0, 0); // ODB0 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta1:Mode",     'D', 0x0,   2,  2, 0); // ODB1 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta2:Mode",     'D', 0x0,   2,  4, 0); // ODB2 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta3:Mode",     'D', 0x0,   2,  6, 0); // ODB3 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta4:Mode",     'D', 0x0,   2,  8, 0); // ODB4 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta5:Mode",     'D', 0x0,   2, 10, 0); // ODB5 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta6:Mode",     'D', 0x0,   2, 12, 0); // ODB6 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta7:Mode",     'D', 0x0,   2, 14, 0); // ODB7 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta8:Mode",     'D', 0x0,   2, 16, 0); // ODB8 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta9:Mode",     'D', 0x0,   2, 18, 0); // ODB9 edge detection mode      (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta10:Mode",    'D', 0x0,   2, 20, 0); // ODB10 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta11:Mode",    'D', 0x0,   2, 22, 0); // ODB11 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta12:Mode",    'D', 0x0,   2, 24, 0); // ODB12 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta13:Mode",    'D', 0x0,   2, 26, 0); // ODB13 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta14:Mode",    'D', 0x0,   2, 28, 0); // ODB14 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta15:Mode",    'D', 0x0,   2, 30, 0); // ODB15 edge detection mode     (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta16:Mode",    'D', 0x1,   2,  0, 0); // Meta16 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta17:Mode",    'D', 0x1,   2,  2, 0); // Meta17 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta18:Mode",    'D', 0x1,   2,  4, 0); // Meta18 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta19:Mode",    'D', 0x1,   2,  6, 0); // Meta19 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta20:Mode",    'D', 0x1,   2,  8, 0); // Meta20 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta21:Mode",    'D', 0x1,   2, 10, 0); // Meta21 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta22:Mode",    'D', 0x1,   2, 12, 0); // Meta22 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta23:Mode",    'D', 0x1,   2, 14, 0); // Meta23 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta24:Mode",    'D', 0x1,   2, 16, 0); // Meta24 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta25:Mode",    'D', 0x1,   2, 18, 0); // Meta25 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta26:Mode",    'D', 0x1,   2, 20, 0); // Meta26 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta27:Mode",    'D', 0x1,   2, 22, 0); // Meta27 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta28:Mode",    'D', 0x1,   2, 24, 0); // Meta28 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta29:Mode",    'D', 0x1,   2, 26, 0); // Meta29 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta30:Mode",    'D', 0x1,   2, 28, 0); // Meta30 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)
    createConfigParam("Meta31:Mode",    'D', 0x1,   2, 30, 0); // Meta31 edge detection mode    (0=disable channel,1=detect ris edge,2=detect fall edge,3=detect any edge)

    createConfigParam("Meta0:PixId",    'D', 0x2,  32,  0, 0x50000000); // ODB0 edge pixel id
    createConfigParam("Meta1:PixId",    'D', 0x3,  32,  0, 0x50000002); // ODB1 edge pixel id
    createConfigParam("Meta2:PixId",    'D', 0x4,  32,  0, 0x50000004); // ODB2 edge pixel id
    createConfigParam("Meta3:PixId",    'D', 0x5,  32,  0, 0x50000006); // ODB3 edge pixel id
    createConfigParam("Meta4:PixId",    'D', 0x6,  32,  0, 0x50000008); // ODB4 edge pixel id
    createConfigParam("Meta5:PixId",    'D', 0x7,  32,  0, 0x5000000a); // ODB5 edge pixel id
    createConfigParam("Meta6:PixId",    'D', 0x8,  32,  0, 0x5000000c); // ODB6 edge pixel id
    createConfigParam("Meta7:PixId",    'D', 0x9,  32,  0, 0x5000000e); // ODB7 edge pixel id
    createConfigParam("Meta8:PixId",    'D', 0xA,  32,  0, 0x50000010); // ODB8 edge pixel id
    createConfigParam("Meta9:PixId",    'D', 0xB,  32,  0, 0x50000012); // ODB9 edge pixel id
    createConfigParam("Meta10:PixId",   'D', 0xC,  32,  0, 0x50000014); // ODB10 edge pixel id
    createConfigParam("Meta11:PixId",   'D', 0xD,  32,  0, 0x50000016); // ODB11 edge pixel id
    createConfigParam("Meta12:PixId",   'D', 0xE,  32,  0, 0x50000018); // ODB12 edge pixel id
    createConfigParam("Meta13:PixId",   'D', 0xF,  32,  0, 0x5000001a); // ODB13 edge pixel id
    createConfigParam("Meta14:PixId",   'D', 0x10, 32,  0, 0x5000001c); // ODB14 edge pixel id
    createConfigParam("Meta15:PixId",   'D', 0x11, 32,  0, 0x5000001e); // ODB15 edge pixel id
    createConfigParam("Meta16:PixId",   'D', 0x12, 32,  0, 0x50000020); // Meta16 edge pixel id
    createConfigParam("Meta17:PixId",   'D', 0x13, 32,  0, 0x50000022); // Meta17 edge pixel id
    createConfigParam("Meta18:PixId",   'D', 0x14, 32,  0, 0x50000024); // Meta18 edge pixel id
    createConfigParam("Meta19:PixId",   'D', 0x15, 32,  0, 0x50000026); // Meta19 edge pixel id
    createConfigParam("Meta20:PixId",   'D', 0x16, 32,  0, 0x50000028); // Meta20 edge pixel id
    createConfigParam("Meta21:PixId",   'D', 0x17, 32,  0, 0x5000002a); // Meta21 edge pixel id
    createConfigParam("Meta22:PixId",   'D', 0x18, 32,  0, 0x5000002c); // Meta22 edge pixel id
    createConfigParam("Meta23:PixId",   'D', 0x19, 32,  0, 0x5000002e); // Meta23 edge pixel id
    createConfigParam("Meta24:PixId",   'D', 0x1A, 32,  0, 0x50000030); // Meta24 edge pixel id
    createConfigParam("Meta25:PixId",   'D', 0x1B, 32,  0, 0x50000032); // Meta25 edge pixel id
    createConfigParam("Meta26:PixId",   'D', 0x1C, 32,  0, 0x50000034); // Meta26 edge pixel id
    createConfigParam("Meta27:PixId",   'D', 0x1D, 32,  0, 0x50000036); // Meta27 edge pixel id
    createConfigParam("Meta28:PixId",   'D', 0x1E, 32,  0, 0x50000038); // Meta28 edge pixel id
    createConfigParam("Meta29:PixId",   'D', 0x1F, 32,  0, 0x5000003a); // Meta29 edge pixel id
    createConfigParam("Meta30:PixId",   'D', 0x20, 32,  0, 0x5000003c); // Meta30 edge pixel id
    createConfigParam("Meta31:PixId",   'D', 0x21, 32,  0, 0x5000003e); // Meta31 edge pixel id

    createConfigParam("Meta0:Cycle",    'D', 0x22,  5,  0, 0); // ODB0 edge cycle number adj
    createConfigParam("Meta1:Cycle",    'D', 0x22,  5,  5, 0); // ODB1 edge cycle number adj
    createConfigParam("Meta2:Cycle",    'D', 0x22,  5, 10, 0); // ODB2 edge cycle number adj
    createConfigParam("Meta3:Cycle",    'D', 0x22,  5, 15, 0); // ODB3 edge cycle number adj
    createConfigParam("Meta4:Cycle",    'D', 0x22,  5, 20, 0); // ODB4 edge cycle number adj
    createConfigParam("Meta5:Cycle",    'D', 0x22,  5, 25, 0); // ODB5 edge cycle number adj
    createConfigParam("Meta6:Cycle",    'D', 0x23,  5,  0, 0); // ODB6 edge cycle number adj
    createConfigParam("Meta7:Cycle",    'D', 0x23,  5,  5, 0); // ODB7 edge cycle number adj
    createConfigParam("Meta8:Cycle",    'D', 0x23,  5, 10, 0); // ODB8 edge cycle number adj
    createConfigParam("Meta9:Cycle",    'D', 0x23,  5, 15, 0); // ODB9 edge cycle number adj
    createConfigParam("Meta10:Cycle",   'D', 0x23,  5, 20, 0); // ODB10 edge cycle number adj
    createConfigParam("Meta11:Cycle",   'D', 0x23,  5, 25, 0); // ODB11 edge cycle number adj
    createConfigParam("Meta12:Cycle",   'D', 0x24,  5,  0, 0); // ODB12 edge cycle number adj
    createConfigParam("Meta13:Cycle",   'D', 0x24,  5,  5, 0); // ODB13 edge cycle number adj
    createConfigParam("Meta14:Cycle",   'D', 0x24,  5, 10, 0); // ODB14 edge cycle number adj
    createConfigParam("Meta15:Cycle",   'D', 0x24,  5, 15, 0); // ODB15 edge cycle number adj
    createConfigParam("Meta16:Cycle",   'D', 0x24,  5, 20, 0); // Meta16 edge cycle number adj
    createConfigParam("Meta17:Cycle",   'D', 0x24,  5, 25, 0); // Meta17 edge cycle number adj
    createConfigParam("Meta18:Cycle",   'D', 0x25,  5,  0, 0); // Meta18 edge cycle number adj
    createConfigParam("Meta19:Cycle",   'D', 0x25,  5,  5, 0); // Meta19 edge cycle number adj
    createConfigParam("Meta20:Cycle",   'D', 0x25,  5, 10, 0); // Meta20 edge cycle number adj
    createConfigParam("Meta21:Cycle",   'D', 0x25,  5, 15, 0); // Meta21 edge cycle number adj
    createConfigParam("Meta22:Cycle",   'D', 0x25,  5, 20, 0); // Meta22 edge cycle number adj
    createConfigParam("Meta23:Cycle",   'D', 0x25,  5, 25, 0); // Meta23 edge cycle number adj
    createConfigParam("Meta24:Cycle",   'D', 0x26,  5,  0, 0); // Meta24 edge cycle number adj
    createConfigParam("Meta25:Cycle",   'D', 0x26,  5,  5, 0); // Meta25 edge cycle number adj
    createConfigParam("Meta26:Cycle",   'D', 0x26,  5, 10, 0); // Meta26 edge cycle number adj
    createConfigParam("Meta27:Cycle",   'D', 0x26,  5, 15, 0); // Meta27 edge cycle number adj
    createConfigParam("Meta28:Cycle",   'D', 0x26,  5, 20, 0); // Meta28 edge cycle number adj
    createConfigParam("Meta29:Cycle",   'D', 0x26,  5, 25, 0); // Meta29 edge cycle number adj
    createConfigParam("Meta30:Cycle",   'D', 0x27,  5,  0, 0); // Meta30 edge cycle number adj
    createConfigParam("Meta31:Cycle",   'D', 0x27,  5,  5, 0); // Meta31 edge cycle number adj

    createConfigParam("Meta0:Delay",    'D', 0x28, 32,  0, 0); // ODB0 edge delay
    createConfigParam("Meta1:Delay",    'D', 0x29, 32,  0, 0); // ODB1 edge delay
    createConfigParam("Meta2:Delay",    'D', 0x2A, 32,  0, 0); // ODB2 edge delay
    createConfigParam("Meta3:Delay",    'D', 0x2B, 32,  0, 0); // ODB3 edge delay
    createConfigParam("Meta4:Delay",    'D', 0x2C, 32,  0, 0); // ODB4 edge delay
    createConfigParam("Meta5:Delay",    'D', 0x2D, 32,  0, 0); // ODB5 edge delay
    createConfigParam("Meta6:Delay",    'D', 0x2E, 32,  0, 0); // ODB6 edge delay
    createConfigParam("Meta7:Delay",    'D', 0x2F, 32,  0, 0); // ODB7 edge delay
    createConfigParam("Meta8:Delay",    'D', 0x30, 32,  0, 0); // ODB8 edge delay
    createConfigParam("Meta9:Delay",    'D', 0x31, 32,  0, 0); // ODB9 edge delay
    createConfigParam("Meta10:Delay",   'D', 0x32, 32,  0, 0); // ODB10 edge delay
    createConfigParam("Meta11:Delay",   'D', 0x33, 32,  0, 0); // ODB11 edge delay
    createConfigParam("Meta12:Delay",   'D', 0x34, 32,  0, 0); // ODB12 edge delay
    createConfigParam("Meta13:Delay",   'D', 0x35, 32,  0, 0); // ODB13 edge delay
    createConfigParam("Meta14:Delay",   'D', 0x36, 32,  0, 0); // ODB14 edge delay
    createConfigParam("Meta15:Delay",   'D', 0x37, 32,  0, 0); // ODB15 edge delay
    createConfigParam("Meta16:Delay",   'D', 0x38, 32,  0, 0); // Meta16 edge delay
    createConfigParam("Meta17:Delay",   'D', 0x39, 32,  0, 0); // Meta17 edge delay
    createConfigParam("Meta18:Delay",   'D', 0x3A, 32,  0, 0); // Meta18 edge delay
    createConfigParam("Meta19:Delay",   'D', 0x3B, 32,  0, 0); // Meta19 edge delay
    createConfigParam("Meta20:Delay",   'D', 0x3C, 32,  0, 0); // Meta20 edge delay
    createConfigParam("Meta21:Delay",   'D', 0x3D, 32,  0, 0); // Meta21 edge delay
    createConfigParam("Meta22:Delay",   'D', 0x3E, 32,  0, 0); // Meta22 edge delay
    createConfigParam("Meta23:Delay",   'D', 0x3F, 32,  0, 0); // Meta23 edge delay
    createConfigParam("Meta24:Delay",   'D', 0x40, 32,  0, 0); // Meta24 edge delay
    createConfigParam("Meta25:Delay",   'D', 0x41, 32,  0, 0); // Meta25 edge delay
    createConfigParam("Meta26:Delay",   'D', 0x42, 32,  0, 0); // Meta26 edge delay
    createConfigParam("Meta27:Delay",   'D', 0x43, 32,  0, 0); // Meta27 edge delay
    createConfigParam("Meta28:Delay",   'D', 0x44, 32,  0, 0); // Meta28 edge delay
    createConfigParam("Meta29:Delay",   'D', 0x45, 32,  0, 0); // Meta29 edge delay
    createConfigParam("Meta30:Delay",   'D', 0x46, 32,  0, 0); // Meta30 edge delay
    createConfigParam("Meta31:Delay",   'D', 0x47, 32,  0, 0); // Meta31 edge delay

    // LVDS & optical parameters
    createConfigParam("Lvds1:FlowCtrl", 'E', 0x0,  1,  0, 0); // LVDS chan1 TXEN control       (0=flow control,1=no flow control)
    createConfigParam("Lvds2:FlowCtrl", 'E', 0x0,  1,  3, 0); // LVDS chan2 TXEN control       (0=flow control,1=no flow control)
    createConfigParam("Lvds3:FlowCtrl", 'E', 0x0,  1,  6, 0); // LVDS chan3 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Lvds4:FlowCtrl", 'E', 0x0,  1,  9, 0); // LVDS chan4 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Lvds5:FlowCtrl", 'E', 0x0,  1, 12, 0); // LVDS chan5 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Lvds6:FlowCtrl", 'E', 0x0,  1, 15, 0); // LVDS chan6 TXEX control       (0=flow control,1=no flow control)
    createConfigParam("Lvds1:ErrPkts",  'E', 0x0,  1,  1, 0); // LVDS chan1 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Lvds2:ErrPkts",  'E', 0x0,  1,  4, 0); // LVDS chan2 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Lvds3:ErrPkts",  'E', 0x0,  1,  7, 0); // LVDS chan3 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Lvds4:ErrPkts",  'E', 0x0,  1, 10, 0); // LVDS chan4 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Lvds5:ErrPkts",  'E', 0x0,  1, 13, 0); // LVDS chan5 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Lvds6:ErrPkts",  'E', 0x0,  1, 16, 0); // LVDS chan6 ignore error pkts  (0=ignore,1=keep)
    createConfigParam("Lvds1:En",       'E', 0x0,  1,  2, 0); // LVDS chan1 disable            (0=enable,1=disable)
    createConfigParam("Lvds2:En",       'E', 0x0,  1,  5, 0); // LVDS chan2 disable            (0=enable,1=disable)
    createConfigParam("Lvds3:En",       'E', 0x0,  1,  8, 0); // LVDS chan3 disable            (0=enable,1=disable)
    createConfigParam("Lvds4:En",       'E', 0x0,  1, 11, 0); // LVDS chan4 disable            (0=enable,1=disable)
    createConfigParam("Lvds5:En",       'E', 0x0,  1, 14, 0); // LVDS chan5 disable            (0=enable,1=disable)
    createConfigParam("Lvds6:En",       'E', 0x0,  1, 17, 0); // LVDS chan6 disable            (0=enable,1=disable)
    createConfigParam("LvdsCmdMode",    'E', 0x0,  1, 18, 0); // LVDS command parser mode      (0=as command,1=as data)
    createConfigParam("LvdsDataMode",   'E', 0x0,  1, 19, 0); // LVDS data parser mode         (0=as data,1=as command)
    createConfigParam("LvdsDataSize",   'E', 0x0,  8, 20, 4); // LVDS data pkt num words
    createConfigParam("LvdsPowerCtrl",  'E', 0x0,  1, 28, 1); // Power Down during reset       (0=power down,1=power up)
    createConfigParam("LvdsPowerRst",   'E', 0x0,  1, 29, 1); // Execute power down sequence   (0=execute,1=bypass)
    createConfigParam("LvdsFilterAll",  'E', 0x0,  2, 30, 0); // Filter all commands

    createConfigParam("LvdsFilterCmd",  'E', 0x1, 16,  0, 0); // LVDS command to filter
    createConfigParam("LvdsFilterMask", 'E', 0x1, 16, 16, 65535); // LVDS command filter mask

//      BLXXX:Det:DspX:| sig nam|                            | EPICS record description | (bi and mbbi description)
    createConfigParam("LvdsTclkMode",   'E', 0x2,  1,  0, 0); // LVDS TX control TCLK mode    (0=TCLK from int,1=TCLK from LVDS)
    createConfigParam("LvdsTclkCtrl",   'E', 0x2,  2,  2, 0); // LVDS TX control T&C TCLK mod (0=TCLK,1=TCLK,2=always 0,3=always 1)
    createConfigParam("LvdsTsyncMode",  'E', 0x2,  2,  4, 1); // LVDS TSYNC_O mode            (0=local TSYNC,1=TSYNC from TREF,2=TSYNC from LVDS,3=TSYNC from opt)
    createConfigParam("LvdsTsyncCtrl",  'E', 0x2,  2,  6, 0); // LVDS TSYNC_NORMAL control    (0=polarity,1=TSYNC WIDTH,2=always 0,3=always 1)
    createConfigParam("LvdsSysrstCtrl", 'E', 0x2,  2,  8, 0); // LVDS T&C SYSRST# buffer ctrl (0=sysrst,1=sysrst,2=always 0,3=always 1)
    createConfigParam("LvdsTxenCtrl",   'E', 0x2,  2, 10, 0); // LVDS T&C TXEN# control       (0=ChLnk parser,1=ChLnk parser,2=ChLnk RX,3=ChLnk inv RX)
    createConfigParam("LvdsOutClck",    'E', 0x2,  2, 16, 0); // LVDS output clock mode       (0=35.3 Mhz, 1=30.2 Mhz, 2=26.5 Mhz, 3=23.6 Mhz)
    createConfigParam("LvdsNRetry",     'E', 0x2,  2, 18, 3); // LVDS downstream retrys
    createConfigParam("Lvds1:WordLen",  'E', 0x2,  1, 20, 0); // LVDS chan1 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Lvds2:WordLen",  'E', 0x2,  1, 21, 0); // LVDS chan2 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Lvds3:WordLen",  'E', 0x2,  1, 22, 0); // LVDS chan3 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Lvds4:WordLen",  'E', 0x2,  1, 23, 0); // LVDS chan4 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Lvds5:WordLen",  'E', 0x2,  1, 24, 0); // LVDS chan5 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("Lvds6:WordLen",  'E', 0x2,  1, 25, 0); // LVDS chan6 data word length  (0=RX FIFO data,1=set to 4)
    createConfigParam("LvdsClkMargin",  'E', 0x2,  2, 26, 0); // LVDS clock margin
    createConfigParam("LvdsTstPattern", 'E', 0x2,  1, 30, 0); // LVDS T&C test pattern        (0=disable,1=enable)
    createConfigParam("LvdsTestEn",     'E', 0x2,  1, 31, 0); // LVDS test enable             (0=disable,1=enable)

    createConfigParam("Lvds1:SrcCtrl",  'E', 0x3,  2,  0, 0); // LVDS ch1 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Lvds2:SrcCtrl",  'E', 0x3,  2,  2, 0); // LVDS ch2 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Lvds3:SrcCtrl",  'E', 0x3,  2,  4, 0); // LVDS ch3 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Lvds4:SrcCtrl",  'E', 0x3,  2,  6, 0); // LVDS ch4 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Lvds5:SrcCtrl",  'E', 0x3,  2,  8, 0); // LVDS ch5 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("Lvds6:SrcCtrl",  'E', 0x3,  2, 10, 0); // LVDS ch6 TSYNC T&C src ctrl  (0=TSYNC_NORMAL,1=TSYNC_LOCAL str,2=TSYNC_LOCA no s,3=TRefStrbFixed)
    createConfigParam("LvdsTsMeta",     'E', 0x3,  2, 14, 2); // LVDS TSYNC metadata src ctrl (0=RTDL,1=LVDS,2=detector TSYNC,3=OFB[0])

    createConfigParam("LvdsTsyncGen",   'E', 0x4, 32,  0, 0); // LVDS TSYNC period
    createConfigParam("TsyncDelay",     'E', 0x5, 32,  0, 0); // LVDS TSYNC delay             (scale:9.4,unit:ns)
    createConfigParam("TsyncWidth",     'E', 0x6, 32,  0, 83330); // LVDS TSYNC width divisor (scale:100,unit:ns)

    createConfigParam("OptA:TxMode",    'E', 0x8,  2,  0, 0); // Optical TX A output mode     (0=Normal,1=Timing,2=Chopper,3=Timing master)
    createConfigParam("OptA:CrossA",    'E', 0x8,  2,  2, 1); // Crossbar Switch Pass ctrl A  (1=Send to trans A,2=send to trans B)
    createConfigParam("OptA:EOC",       'E', 0x8,  1,  4, 1); // Optical TX A End of Chain
    createConfigParam("OptA:FilterCmd", 'E', 0x8,  2,  5, 0); // Optical TX A Command Filter
    createConfigParam("OptB:TxMode",    'E', 0x8,  2,  8, 3); // Optical TX B output mode     (0=Normal,1=Timing,2=Chopper,3=Timing master)
    createConfigParam("OptB:CrossB",    'E', 0x8,  2, 10, 2); // Crossbar Switch Pass ctrl B  (1=Send to trans A,2=send to trans B)
    createConfigParam("OptB:EOC",       'E', 0x8,  1, 12, 1); // Optical TX B End of Chain
    createConfigParam("OptB:FilterCmd", 'E', 0x8,  2, 13, 0); // Optical TX B Command Filter
    createConfigParam("OptHystEn",      'E', 0x8,  1, 16, 0); // Optical hysteresis enable    (0=from TLK data,1=match optical)
    createConfigParam("OptBlankEn",     'E', 0x8,  1, 17, 0); // Optical empty data frame CRC (0=no blank frame, 1=add blank frame)
    createConfigParam("OptRspLimit",    'E', 0x8,  1, 18, 0); // 64 responses limit           (0=no limit, 1=limit to 64)
    createConfigParam("OptDataFormat",  'E', 0x8,  4, 20, 0); // Data format
    createConfigParam("OptTxDelay",     'E', 0x8,  7, 24, 3); // Optical packet send delay    (0=0ns, 1=313ns, 3=1us, 100=31.3us)
    createConfigParam("OptTxDelayC",    'E', 0x8,  1, 31, 1); // Optical packet send delay ct (0=use OPT_TX_DELAY,1=prev word count)

    createConfigParam("OptMaxSize",     'E', 0x9, 16,  0, 3600); // Optical packet max dwords
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

    createConfigParam("TestPatternId",  'F', 0x1, 12,  0, 0); // Test pattern id
    createConfigParam("TestPatternSel", 'F', 0x1,  2, 12, 0); // Test pattern select          (0=optic,1=alternate 1,2=alternate 2,3=alternate 3)
    createConfigParam("TestPatternRtdlEn",'F',0x1, 1, 14, 1); // Test pattern RTDL enable     (0=disable,1=enable)
    createConfigParam("TestPatternEn",  'F', 0x1,  1, 15, 0); // Test pattern enable          (0=disable,1=enable)
    createConfigParam("TestPatternRate",'F', 0x1, 16, 16, 0); // Test pattern rate            (65535=1.6 Kev/s (lowest), 53124=2 Kev/s, 13280=8 Kev/s, 5311=20 Kev/s, 1061=100 Kev/s, 264=400 Kev/s, 105=1 Mev/s, 52=2 Mev/s, 34=3 Mev/s, 25=4 Mev/s, 20=5M ev/s, 16=6 Mev/s, 12=8 Mev/s, 9=10 Mev/s, 6=15 Mev/s)

    createConfigParam("TestPatPktSize", 'F', 0x2,  4,  0, 0); // Test pattern sub-packet size
    createConfigParam("TestPatAlt1Rate",'F', 0x2, 12,  4, 0); // Test pattern sub-packet size
    createConfigParam("RateMeterInt",   'F', 0x2,  2, 16, 0); // Rate meter counting interval (0=0.001 sec,1=0.01 sec,2=0.1 sec,3=1.0 sec)
    createConfigParam("TestPatAlt2ChEn",'F', 0x2,  2, 18, 0); // Needs desc and values
    createConfigParam("TestPatAlt2Rate",'F', 0x2, 12, 20, 0); // Alt2 test pattern rate

    createConfigParam("ChopSubFreqSel", 'F', 0x3,  2,  0, 0); // Map upper 8 bits on ODB
    createConfigParam("GCtrlReg3Dbg2",  'F', 0x3,  1,  2, 0); // TBD
    createConfigParam("GCtrlReg3Dbg3",  'F', 0x3,  1,  3, 0); // TBD
    createConfigParam("GCtrlReg3Dbg4",  'F', 0x3,  4,  4, 0); // TBD
    createConfigParam("GCtrlReg3Dbg5",  'F', 0x3,  8,  8, 0); // TBD
    createConfigParam("GCtrlReg3Dbg6",  'F', 0x3, 16, 16, 0); // TBD

    createConfigParam("GCtrlReg4Dbg1",  'F', 0x4,  4,  0, 0); // TBD
    createConfigParam("GCtrlReg4Dbg2",  'F', 0x4,  4,  4, 0); // TBD
    createConfigParam("GCtrlReg4Dbg3",  'F', 0x4,  8,  8, 0); // TBD
    createConfigParam("GCtrlReg4Dbg4",  'F', 0x4, 16, 16, 0); // TBD

//      BLXXX:Det:DspX:| sig nam|                     | EPICS record description | (bi and mbbi description)
    createCounterParam("PktLenErrCnt",     0x0, 16,  0); // TBD
    createCounterParam("SCntrs:1",         0x0, 16, 16); // x0111
    createCounterParam("BadCmdCnt",        0x1, 16,  0); // Unrecognized commands
    createCounterParam("BadCmdLvdsCnt",    0x1, 16, 16); // LVDS CMD parsing errors
    createCounterParam("LvdsFifoAFCnt",    0x2, 16,  0); // LVDS Tx FIFO Almost Full
    createCounterParam("SCntrs:5",         0x2, 16, 16); // x0555
    createCounterParam("OptA:LosCnt",      0x3, 16,  0); // TBD
    createCounterParam("OptA:TxFltCnt",    0x3, 16, 16); // TBD
    createCounterParam("OptA:ComFailCnt",  0x4, 16,  0); // TBD
    createCounterParam("OptB:ComFailCnt",  0x4, 16, 16); // TBD
    createCounterParam("OptB:LosCnt",      0x5, 16,  0); // TBD
    createCounterParam("OptB:TxFaultCnt",  0x5, 16, 16); // TBD
    createCounterParam("OptA:FramErrCnt",  0x6, 16,  0); // TBD
    createCounterParam("OptA:CRCErrCnt",   0x6, 16, 16); // CRC low word
    createCounterParam("OptA:FfAfCnt",     0x7, 16,  0); // TBD
    createCounterParam("OptA:StkAfCnt",    0x7, 16, 16); // TBD
    createCounterParam("OptA:SecFAfCnt",   0x8, 16,  0); // TBD
    createCounterParam("OptA:PktFlagCnt",  0x8, 16, 16); // TBD
    createCounterParam("OptB:FramErrCnt",  0x9, 16,  0); // TBD
    createCounterParam("OptB:CRCErrCnt",   0x9, 16, 16); // CRC low word
    createCounterParam("OptB:FfAfCnt",     0xA, 16,  0); // TBD
    createCounterParam("OptB:StkAfCnt",    0xA, 16, 16); // TBD
    createCounterParam("OptB:SecFAfCnt",   0xB, 16,  0); // TBD
    createCounterParam("OptB:PktFlagCnt",  0xB, 16, 16); // TBD
    createCounterParam("ChL1ParityErCnt",  0xC, 16,  0); // Even parity error
    createCounterParam("ChL1FrameErCnt",   0xC, 16, 16); // Composite framing errors
    createCounterParam("ChL1IDTFfCnt",     0xD, 16,  0); // External IDT FIFO is full
    createCounterParam("ChL1ExtAfCnt",     0xD, 16, 16); // External FIFO AF
    createCounterParam("ChL1PpAfCnt",      0xE, 16,  0); // Channel-link packet FIFO AF
    createCounterParam("ChL1SCntrs:29",    0xE, 16, 16); // ZERO
    createCounterParam("ChL2ParityErCnt",  0xF, 16,  0); // Even parity error
    createCounterParam("ChL2FrameErCnt",   0xF, 16, 16); // Composite framing errors
    createCounterParam("ChL2IDTFfCnt",    0x10, 16,  0); // External IDT FIFO is full
    createCounterParam("ChL2ExtAfCnt",    0x10, 16, 16); // External FIFO AF
    createCounterParam("ChL2PpAfCnt",     0x11, 16,  0); // Channel-link packet FIFO AF
    createCounterParam("ChL2SCntrs:35",   0x11, 16, 16); // ZERO
    createCounterParam("ChL3ParityErCnt", 0x12, 16,  0); // Even parity error
    createCounterParam("ChL3FrameErCnt",  0x12, 16, 16); // Composite framing errors
    createCounterParam("ChL3IDTFfCnt",    0x13, 16,  0); // External IDT FIFO is full
    createCounterParam("ChL3ExtAfCnt",    0x13, 16, 16); // External FIFO AF
    createCounterParam("ChL3PpAfCnt",     0x14, 16,  0); // Channel-link packet FIFO AF
    createCounterParam("ChL3SCntrs:41",   0x14, 16, 16); // ZERO
    createCounterParam("ChL4ParityErCnt", 0x15, 16,  0); // Even parity error
    createCounterParam("ChL4FrameErCnt",  0x15, 16, 16); // Composite framing errors
    createCounterParam("ChL4IDTFfCnt",    0x16, 16,  0); // External IDT FIFO is full
    createCounterParam("ChL4ExtAfCnt",    0x16, 16, 16); // External FIFO AF
    createCounterParam("ChL4PpAfCnt",     0x17, 16,  0); // Channel-link packet FIFO AF
    createCounterParam("ChL4SCntrs:47",   0x17, 16, 16); // ZERO
    createCounterParam("ChL5ParityErCnt", 0x18, 16,  0); // Even parity error
    createCounterParam("ChL5FrameErCnt",  0x18, 16, 16); // Composite framing errors
    createCounterParam("ChL5IDTFfCnt",    0x19, 16,  0); // External IDT FIFO is full
    createCounterParam("ChL5ExtAfCnt",    0x19, 16, 16); // External FIFO AF
    createCounterParam("ChL5PpAfCnt",     0x1A, 16,  0); // Channel-link packet FIFO AF
    createCounterParam("ChL5SCntrs:53",   0x1A, 16, 16); // ZERO
    createCounterParam("ChL6ParityErCnt", 0x1B, 16,  0); // Even parity error
    createCounterParam("ChL6FrameErCnt",  0x1B, 16, 16); // Composite framing errors
    createCounterParam("ChL6IDTFfCnt",    0x1C, 16,  0); // External IDT FIFO is full
    createCounterParam("ChL6ExtAfCnt",    0x1C, 16, 16); // External FIFO AF
    createCounterParam("ChL6PpAfCnt",     0x1D, 16,  0); // Channel-link packet FIFO AF
    createCounterParam("ChL6SCntrs:59",   0x1D, 16, 16); // ZERO
    createCounterParam("SorterAfCnt",     0x1E, 16,  0); // Sorter Command FIFO AF
    createCounterParam("ChLCmdRdAfCnt",   0x1E, 16, 16); // Channel link command FIFO AF
    createCounterParam("MD1:EdgeAfCnt",   0x1F, 16,  0); // Metadata FIFO Subgroup 0
    createCounterParam("MD2:EdgeAfCnt",   0x1F, 16, 16); // Metadata FIFO Subgroup 1
    createCounterParam("MD3:EdgeAfCnt",   0x20, 16,  0); // Metadata FIFO Subgroup 2
    createCounterParam("MD4:EdgeAfCnt",   0x20, 16, 16); // Metadata FIFO Subgroup 3
    createCounterParam("MDPckAfCnt",      0x21, 16,  0); // Metadata Packet FIFO AF
    createCounterParam("SCntrs:67",       0x21, 16, 16); // x0167
    createCounterParam("NEvPSAfCnt",      0x22, 16,  0); // Neutron event sorter AF
    createCounterParam("NEvPL1AfCnt",     0x22, 16, 16); // Level 1 dff_af
    createCounterParam("NEvPL1WcAFCnt",   0x23, 16,  0); // Level 1 dff_wcnt_af
    createCounterParam("MEvPSAfCnt",      0x23, 16, 16); // Sorter fifo_af
    createCounterParam("MEvPL1AfCnt",     0x24, 16,  0); // Level 1 dff_af
    createCounterParam("MEvPL1WcAFCnt",   0x24, 16, 16); // Level 1 dff_wcnt_af
    createCounterParam("EvPL2AfCnt",      0x25, 16,  0); // Level 2 dff_af
    createCounterParam("EvPL2WcAfCnt",    0x25, 16, 16); // Level 2 dff_wcnt_af
    createCounterParam("SCntrs:76",       0x26, 16,  0); // x0176
    createCounterParam("SCntrs:77",       0x26, 16, 16); // x0177

    createCounterParam("Lvds1:Rate",      0x27, 24,  0); // LVDS Ch1 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("Lvds2:Rate",      0x28, 24,  0); // LVDS Ch2 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("Lvds3:Rate",      0x29, 24,  0); // LVDS Ch3 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("Lvds4:Rate",      0x2A, 24,  0); // LVDS Ch4 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("Lvds5:Rate",      0x2B, 24,  0); // LVDS Ch5 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("Lvds6:Rate",      0x2C, 24,  0); // LVDS Ch6 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("Lvds7:Rate",      0x2D, 24,  0); // LVDS Ch7 rate            (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("OptA:Rate",       0x2E, 24,  0); // Optic A rate             (calc:2*A*10^(3-B), calclink:RateMeterInt, unit:byte/s, prec:2)
    createCounterParam("OptB:Rate",       0x2F, 24,  0); // Optic B rate             (calc:2*A*10^(3-B), calclink:RateMeterInt, unit:byte/s, prec:2)
    createCounterParam("OptA:Payload",    0x30, 24,  0); // Optic A payload rate     (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("RateDoutPls",     0x31, 24,  0); // TBD                      (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("SCounters100",    0x32, 24,  0); // TBD                      (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("SCounters102",    0x33, 24,  0); // TBD                      (calc:A*10^(3-B), calclink:RateMeterInt, unit:event/s, prec:2)
    createCounterParam("SCounters104",    0x34, 24,  0); // TBD
    createCounterParam("SCounters106",    0x35, 16,  0); // TBD
    createCounterParam("SCounters107",    0x35, 16, 16); // TBD
    createCounterParam("SCounters108",    0x36, 16,  0); // TBD
    createCounterParam("SCounters109",    0x36, 16, 16); // TBD

//      BLXXX:Det:DspX:| sig nam|                     | EPICS record description | (bi and mbbi description)
    createStatusParam("Configured",    0x0,  1,  0); // Configured                   (0=not configured [alarm],1=configured, archive:monitor)
    createStatusParam("Acquiring",     0x0,  1,  1); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("ErrProgram",    0x0,  1,  2); // WRITE_CNFG during ACQ        (0=no error,1=error)
    createStatusParam("ErrLength",     0x0,  1,  3); // Packet length error          (0=no error,1=error)
    createStatusParam("ErrCmdBad",     0x0,  1,  4); // Unrecognized command error   (0=no error,1=error)
    createStatusParam("TxFifoFull",    0x0,  1,  5); // LVDS TxFIFO went full        (0=not full,1=full)
    createStatusParam("CmdErr",        0x0,  1,  6); // LVDS command error           (0=no error,1=error)
    createStatusParam("EepromInit",    0x0,  1,  7); // EEPROM initialization status (0=not ok,1=ok)
    createStatusParam("OptRspLimitDef",0x0,  1,  8); // Responses limit default      (0=limit to 64,1=no limit)
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

    createStatusParam("Lvds1:RxFlg",   0x3,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds1:RxStat",  0x3,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Lvds1:RxData",  0x3,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Lvds1:RxAF",    0x3,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Lvds1:RxParD",  0x3,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Lvds1:RxParF",  0x3,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Lvds1:ExtEn",   0x3,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Lvds1:ParsEn",  0x3,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Lvds1:ErrCnt",  0x3, 16, 16); // Data packet errors count

    createStatusParam("Lvds2:RxFlags", 0x4,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds2:RxStat",  0x4,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Lvds2:RxData",  0x4,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Lvds2:RxAF",    0x4,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Lvds2:RxParD",  0x4,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Lvds2:RxParF",  0x4,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Lvds2:ExtEn",   0x4,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Lvds2:ParsEn",  0x4,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Lvds2:ErrCnt",  0x4, 16, 16); // Data packet errors count
    createStatusParam("Lvds3:RxFlags", 0x5,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds3:RxStat",  0x5,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Lvds3:RxData",  0x5,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Lvds3:RxAF",    0x5,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Lvds3:RxParD",  0x5,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Lvds3:RxParF",  0x5,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Lvds3:ExtEn",   0x5,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Lvds3:ParsEn",  0x5,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Lvds3:ErrCnt",  0x5, 16, 16); // Data packet errors count

    createStatusParam("Lvds4:RxFlags", 0x6,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds4:RxStat",  0x6,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Lvds4:RxData",  0x6,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Lvds4:RxAF",    0x6,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Lvds4:RxParD",  0x6,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Lvds4:RxParF",  0x6,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Lvds4:ExtEn",   0x6,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Lvds4:ParsEn",  0x6,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Lvds4:ErrCnt",  0x6, 16, 16); // Data packet errors count

    createStatusParam("Lvds5:RxFlags", 0x7,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds5:RxStat",  0x7,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Lvds5:RxData",  0x7,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Lvds5:RxAF",    0x7,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Lvds5:RxParD",  0x7,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Lvds5:RxParF",  0x7,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Lvds5:ExtEn",   0x7,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Lvds5:ParsEn",  0x7,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Lvds5:ErrCnt",  0x7, 16, 16); // Data packet errors count

    createStatusParam("Lvds6:RxFlags", 0x8,  8,  0); // Error flags                  (0=parity error,1=packet type err,2=start&last set,3=len >300 words,4=FIFO timeout,5=no first word,6=last befor first,7=out FIFO full)
    createStatusParam("Lvds6:RxStat",  0x8,  2,  8); // Status OK                    (0=good cmd packet,1=good data packet)
    createStatusParam("Lvds6:RxData",  0x8,  1, 10); // External FIFO has data       (0=empty,1=has data)
    createStatusParam("Lvds6:RxAF",    0x8,  1, 11); // External FIFO almost full    (0=not full,1=almost full)
    createStatusParam("Lvds6:RxParD",  0x8,  1, 12); // ChLnk pkt pars FIFO has data (0=empty,1=has data)
    createStatusParam("Lvds6:RxParF",  0x8,  1, 13); // ChLnk pkt pars FIFO almost f (0=not full,1=almost full)
    createStatusParam("Lvds6:ExtEn",   0x8,  1, 14); // External FIFO Read enabled   (0=disabled,1=enabled)
    createStatusParam("Lvds6:ParsEn",  0x8,  1, 15); // ChLnk pkt pars FIFO Write en (0=disabled,1=enabled)
    createStatusParam("Lvds6:ErrCnt",  0x8, 16, 16); // Data packet errors count

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

    // TODO: define the names and descriptions when ready
    createStatusParam("Unused23LB",   0x17, 16,  0); // TODO
    createStatusParam("Unused23HB",   0x17, 16, 16); // TODO
    createStatusParam("Unused24LB",   0x18, 16,  0); // TODO
    createStatusParam("Unused24HB",   0x18, 16, 16); // TODO
}
