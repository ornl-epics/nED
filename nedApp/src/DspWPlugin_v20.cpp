/* DspWPlugin_v20.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspWPlugin.h"

void DspWPlugin::createParams_v20() {
//      BLXXX:Det:DspX:| sig nam|                                 | EPICS record description | (bi and mbbi description)
    createConfigParam("TestEventPeriod",  '1', 0x0,  8,  0, 15); // Test event period
    createConfigParam("TsyncFreq",        '1', 0x0,  8,  8, 60); // TSYNC frequency            (unit:Hz)
    createConfigParam("Ch0:Enable",       '1', 0x1,  1,  0, 1);  // Channel 0 enable           (0=disable,1=enable)
    createConfigParam("Ch1:Enable",       '1', 0x1,  1,  1, 1);  // Channel 1 enable           (0=disable,1=enable)
    createConfigParam("Ch2:Enable",       '1', 0x1,  1,  2, 1);  // Channel 2 enable           (0=disable,1=enable)
    createConfigParam("Ch3:Enable",       '1', 0x1,  1,  3, 1);  // Channel 3 enable           (0=disable,1=enable)
    createConfigParam("Ch4:Enable",       '1', 0x1,  1,  4, 1);  // Channel 4 enable           (0=disable,1=enable)
    createConfigParam("Ch5:Enable",       '1', 0x1,  1,  5, 1);  // Channel 5 enable           (0=disable,1=enable)
    createConfigParam("Ch6:Enable",       '1', 0x1,  1,  6, 1);  // Channel 6 enable           (0=disable,1=enable)
    createConfigParam("Ch7:Enable",       '1', 0x1,  1,  7, 1);  // Channel 7 enable           (0=disable,1=enable)
    createConfigParam("Ch0:TestEn",       '1', 0x1,  1,  8, 0);  // Channel 0 test enable      (0=disable,1=enable)
    createConfigParam("Ch1:TestEn",       '1', 0x1,  1,  9, 0);  // Channel 1 test enable      (0=disable,1=enable)
    createConfigParam("Ch2:TestEn",       '1', 0x1,  1, 10, 0);  // Channel 2 test enable      (0=disable,1=enable)
    createConfigParam("Ch3:TestEn",       '1', 0x1,  1, 11, 0);  // Channel 3 test enable      (0=disable,1=enable)
    createConfigParam("Ch4:TestEn",       '1', 0x1,  1, 12, 0);  // Channel 4 test enable      (0=disable,1=enable)
    createConfigParam("Ch5:TestEn",       '1', 0x1,  1, 13, 0);  // Channel 5 test enable      (0=disable,1=enable)
    createConfigParam("Ch6:TestEn",       '1', 0x1,  1, 14, 0);  // Channel 6 test enable      (0=disable,1=enable)
    createConfigParam("Ch7:TestEn",       '1', 0x1,  1, 15, 0);  // Channel 7 test enable      (0=disable,1=enable)
    createConfigParam("Ch0:LoopEn",       '1', 0x1,  1, 16, 0);  // Channel 0 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch1:LoopEn",       '1', 0x1,  1, 17, 0);  // Channel 1 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch2:LoopEn",       '1', 0x1,  1, 18, 0);  // Channel 2 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch3:LoopEn",       '1', 0x1,  1, 19, 0);  // Channel 3 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch4:LoopEn",       '1', 0x1,  1, 20, 0);  // Channel 4 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch5:LoopEn",       '1', 0x1,  1, 21, 0);  // Channel 5 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch6:LoopEn",       '1', 0x1,  1, 22, 0);  // Channel 6 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch7:LoopEn",       '1', 0x1,  1, 23, 0);  // Channel 7 loopback enable  (0=disable,1=enable)
    createConfigParam("RunMode",          '1', 0x1,  3, 24, 0);  // Run Mode                   (0=histograming,1=stroboscopic,2=test,3=loopback)
    createConfigParam("PllErrorGain",     '1', 0x1,  1, 27, 0);  // PLL Error Gain             (0=1,1=1/2)
    createConfigParam("PllErrorRange",    '1', 0x1,  3, 28, 2);  // PLL Error Range            (0=1us,1=2us,2=3us,3=4us,4=5us,5=6us,6=7us,7=8us)
    createConfigParam("ExtTimeDisable",   '1', 0x1,  1, 31, 1);  // External Timing Disable    (1=disable,0=enable)
    createConfigParam("Opt0:Enable",      '1', 0x2,  1,  0, 1);  // Optical 0 TX enable        (0=disable,1=enable)
    createConfigParam("Opt1:Enable",      '1', 0x2,  1,  1, 1);  // Optical 1 TX enable        (0=disable,1=enable)
    createConfigParam("Opt2:Enable",      '1', 0x2,  1,  2, 0);  // Optical 2 TX enable        (0=disable,1=enable)
    createConfigParam("Opt3:Enable",      '1', 0x2,  1,  3, 0);  // Optical 3 TX enable        (0=disable,1=enable)
    createConfigParam("Ch0:OptEn",        '1', 0x2,  1,  4, 0);  // Channel 0 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch1:OptEn",        '1', 0x2,  1,  5, 0);  // Channel 1 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch2:OptEn",        '1', 0x2,  1,  6, 0);  // Channel 2 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch3:OptEn",        '1', 0x2,  1,  7, 0);  // Channel 3 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch4:OptEn",        '1', 0x2,  1,  8, 0);  // Channel 4 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch5:OptEn",        '1', 0x2,  1,  9, 0);  // Channel 5 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch6:OptEn",        '1', 0x2,  1, 10, 0);  // Channel 6 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch7:OptEn",        '1', 0x2,  1, 11, 0);  // Channel 7 optical TX en    (0=enable,1=disable)
    createConfigParam("OptFMC1En",        '1', 0x2,  8, 12, 0);  // OptFMC1En
    createConfigParam("MaxDataPktSize",   '1', 0x2,  3, 20, 0);  // Max data packet size       (0=2000 bytes, 1=4000 bytes, 2=6000 bytes, 3=8000 bytes, 4=10000 bytes, 5=6000 bytes, 6=7000 bytes, 7=8000 bytes)
    createConfigParam("MetaDisable",      '1', 0x2,  1, 23, 0);  // Meta Disable               (1=disable,0=enable)
    createConfigParam("Odb0:Mode",        '1', 0x3,  2,  0, 0);  // Detect Mode Channel0       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb1:Mode",        '1', 0x3,  2,  2, 0);  // Detect Mode Channel1       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb2:Mode",        '1', 0x3,  2,  4, 0);  // Detect Mode Channel2       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb3:Mode",        '1', 0x3,  2,  6, 1);  // Detect Mode Channel3       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb4:Mode",        '1', 0x3,  2,  8, 1);  // Detect Mode Channel4       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb5:Mode",        '1', 0x3,  2, 10, 1);  // Detect Mode Channel5       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb6:Mode",        '1', 0x3,  2, 12, 1);  // Detect Mode Channel6       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb7:Mode",        '1', 0x3,  2, 14, 1);  // Detect Mode Channel7       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb8:Mode",        '1', 0x3,  2, 16, 1);  // Detect Mode Channel8       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb9:Mode",        '1', 0x3,  2, 18, 1);  // Detect Mode Channel9       (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb10:Mode",       '1', 0x3,  2, 20, 1);  // Detect Mode Channel10      (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb11:Mode",       '1', 0x3,  2, 22, 1);  // Detect Mode Channel11      (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb12:Mode",       '1', 0x3,  2, 24, 1);  // Detect Mode Channel12      (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb13:Mode",       '1', 0x3,  2, 26, 1);  // Detect Mode Channel13      (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb14:Mode",       '1', 0x3,  2, 28, 1);  // Detect Mode Channel14      (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb15:Mode",       '1', 0x3,  2, 30, 1);  // Detect Mode Channel15      (0=disable,1=rising edge,2=falling edge,3=both edges)
    createConfigParam("Odb0:Type",        '1', 0x4,  4,  0, 0);  // Meta event type 0          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb1:Type",        '1', 0x4,  4,  4, 0);  // Meta event type 1          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb2:Type",        '1', 0x4,  4,  8, 0);  // Meta event type 2          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb3:Type",        '1', 0x4,  4, 12, 0);  // Meta event type 3          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb4:Type",        '1', 0x4,  4, 16, 0);  // Meta event type 4          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb5:Type",        '1', 0x4,  4, 20, 0);  // Meta event type 5          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb6:Type",        '1', 0x4,  4, 24, 0);  // Meta event type 6          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb7:Type",        '1', 0x4,  4, 28, 0);  // Meta event type 7          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb8:Type",        '1', 0x5,  4,  0, 0);  // Meta event type 8          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb9:Type",        '1', 0x5,  4,  4, 0);  // Meta event type 9          (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb10:Type",       '1', 0x5,  4,  8, 0);  // Meta event type 10         (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb11:Type",       '1', 0x5,  4, 12, 0);  // Meta event type 11         (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb12:Type",       '1', 0x5,  4, 16, 0);  // Meta event type 12         (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb13:Type",       '1', 0x5,  4, 20, 0);  // Meta event type 13         (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb14:Type",       '1', 0x5,  4, 24, 0);  // Meta event type 14         (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb15:Type",       '1', 0x5,  4, 28, 0);  // Meta event type 15         (0=detector,4=beam monitor,5=signal,6=ADC,7=chopper)
    createConfigParam("Odb0:Id",          '1', 0x6,  4,  0, 0);  // Detect Channel0
    createConfigParam("Odb1:Id",          '1', 0x6,  4,  4, 1);  // Detect Channel1
    createConfigParam("Odb2:Id",          '1', 0x6,  4,  8, 2);  // Detect Channel2
    createConfigParam("Odb3:Id",          '1', 0x6,  4, 12, 3);  // Detect Channel3
    createConfigParam("Odb4:Id",          '1', 0x6,  4, 16, 4);  // Detect Channel4
    createConfigParam("Odb5:Id",          '1', 0x6,  4, 20, 5);  // Detect Channel5
    createConfigParam("Odb6:Id",          '1', 0x6,  4, 24, 6);  // Detect Channel6
    createConfigParam("Odb7:Id",          '1', 0x6,  4, 28, 7);  // Detect Channel7
    createConfigParam("Odb8:Id",          '1', 0x7,  4,  0, 8);  // Detect Channel8
    createConfigParam("Odb9:Id",          '1', 0x7,  4,  4, 9);  // Detect Channel9
    createConfigParam("Odb10:Id",         '1', 0x7,  4,  8,10);  // Detect Channel10
    createConfigParam("Odb11:Id",         '1', 0x7,  4, 12,11);  // Detect Channel11
    createConfigParam("Odb12:Id",         '1', 0x7,  4, 16,12);  // Detect Channel12
    createConfigParam("Odb13:Id",         '1', 0x7,  4, 20,13);  // Detect Channel13
    createConfigParam("Odb14:Id",         '1', 0x7,  4, 24,14);  // Detect Channel14
    createConfigParam("Odb15:Id",         '1', 0x7,  4, 28,15);  // Detect Channel15

//      BLXXX:Det:DspX:| sig nam|                          | EPICS record description | (bi and mbbi description)
    createStatusParam("OccLinkStatus",    0x0,  1,  0); // OCC link status              (0=connected,1=disconnected)
    createStatusParam("OdbLinkStatus",    0x0,  1,  1); // ODB link status              (0=connected,1=disconnected)
    createStatusParam("Aux1LinkStatus",   0x0,  1,  2); // AUX1 link status             (0=connected,1=disconnected)
    createStatusParam("Aux2LinkStatus",   0x0,  1,  3); // AUX2 link status             (0=connected,1=disconnected)
    createStatusParam("Ch0:LinkStatus",   0x0,  1,  4); // Channel 0 link status        (0=connected,1=disconnected)
    createStatusParam("Ch1:LinkStatus",   0x0,  1,  5); // Channel 1 link status        (0=connected,1=disconnected)
    createStatusParam("Ch2:LinkStatus",   0x0,  1,  6); // Channel 2 link status        (0=connected,1=disconnected)
    createStatusParam("Ch3:LinkStatus",   0x0,  1,  7); // Channel 3 link status        (0=connected,1=disconnected)
    createStatusParam("Ch4:LinkStatus",   0x0,  1,  8); // Channel 4 link status        (0=connected,1=disconnected)
    createStatusParam("Ch5:LinkStatus",   0x0,  1,  9); // Channel 5 link status        (0=connected,1=disconnected)
    createStatusParam("Ch6:LinkStatus",   0x0,  1, 10); // Channel 6 link status        (0=connected,1=disconnected)
    createStatusParam("Ch7:LinkStatus",   0x0,  1, 11); // Channel 7 link status        (0=connected,1=disconnected)
    createStatusParam("Configured",       0x0,  1, 12); // Configured                   (0=not configured [alarm],1=configured, archive:monitor)
    createStatusParam("Ch0:MetaData",     0x0,  1, 16); // Channel  0 seen meta data    (0=no,1=yes)
    createStatusParam("Ch1:MetaData",     0x0,  1, 17); // Channel  1 seen meta data    (0=no,1=yes)
    createStatusParam("Ch2:MetaData",     0x0,  1, 18); // Channel  2 seen meta data    (0=no,1=yes)
    createStatusParam("Ch3:MetaData",     0x0,  1, 19); // Channel  3 seen meta data    (0=no,1=yes)
    createStatusParam("Ch4:MetaData",     0x0,  1, 20); // Channel  4 seen meta data    (0=no,1=yes)
    createStatusParam("Ch5:MetaData",     0x0,  1, 21); // Channel  5 seen meta data    (0=no,1=yes)
    createStatusParam("Ch6:MetaData",     0x0,  1, 22); // Channel  6 seen meta data    (0=no,1=yes)
    createStatusParam("Ch7:MetaData",     0x0,  1, 23); // Channel  7 seen meta data    (0=no,1=yes)
    createStatusParam("Ch8:MetaData",     0x0,  1, 24); // Channel  8 seen meta data    (0=no,1=yes)
    createStatusParam("Ch9:MetaData",     0x0,  1, 25); // Channel  9 seen meta data    (0=no,1=yes)
    createStatusParam("Ch10:MetaData",    0x0,  1, 26); // Channel 10 seen meta data    (0=no,1=yes)
    createStatusParam("Ch11:MetaData",    0x0,  1, 27); // Channel 11 seen meta data    (0=no,1=yes)
    createStatusParam("Ch12:MetaData",    0x0,  1, 28); // Channel 12 seen meta data    (0=no,1=yes)
    createStatusParam("Ch13:MetaData",    0x0,  1, 29); // Channel 13 seen meta data    (0=no,1=yes)
    createStatusParam("Ch14:MetaData",    0x0,  1, 30); // Channel 14 seen meta data    (0=no,1=yes)
    createStatusParam("Ch15:MetaData",    0x0,  1, 31); // Channel 15 seen meta data    (0=no,1=yes)
    createStatusParam("Acquiring",        0x1,  1,  0); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("RunModeStatus",    0x1,  3,  1); // Run mode status              (0=histogram,1=stroboscopic,2=test,3=loopback)
    createStatusParam("InternalTiming",   0x1,  1,  4); // Internal Timing              (0=off,1=on)
    createStatusParam("TimeLocked",       0x1,  1,  5); // Locked to NTP                (0=not locked,1=locked)
    createStatusParam("NoPps",            0x1,  1,  6); // No PPS                       (0=PPS present,1=no PPS)
    createStatusParam("TimePpsSlow",      0x1,  1,  7); // PPS signal is slow           (0=normal,1=slow)
    createStatusParam("TimePpsFast",      0x1,  1,  8); // PPS signal is fast           (0=normal,1=fast)
    createStatusParam("Ch0:NeutronData",  0x1,  1, 24); // Channel 0 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch1:NeutronData",  0x1,  1, 25); // Channel 1 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch2:NeutronData",  0x1,  1, 26); // Channel 2 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch3:NeutronData",  0x1,  1, 27); // Channel 3 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch4:NeutronData",  0x1,  1, 28); // Channel 4 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch5:NeutronData",  0x1,  1, 29); // Channel 5 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch6:NeutronData",  0x1,  1, 30); // Channel 6 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch7:NeutronData",  0x1,  1, 31); // Channel 7 seen neutron data  (0=yes,1=no)
    createStatusParam("OverTemp",         0x3,  1,  0); // Over temperature alarm       (0=no alarm,1=alarm [alarm])
    createStatusParam("UserTemp",         0x3,  1,  1); // User temperature alarm       (0=no alarm,1=alarm [alarm])
    createStatusParam("VccInt",           0x3,  1,  2); // VCCINT alarm                 (0=no alarm,1=alarm [alarm])
    createStatusParam("VccAux",           0x3,  1,  3); // VCCAUX alarm                 (0=no alarm,1=alarm [alarm])


    createCounterParam("Ch0:EventRate",   0x0, 32,  0); // Channel 0 event rate         (unit:cnts/s)
    createCounterParam("Ch1:EventRate",   0x1, 32,  0); // Channel 1 event rate         (unit:cnts/s)
    createCounterParam("Ch2:EventRate",   0x2, 32,  0); // Channel 2 event rate         (unit:cnts/s)
    createCounterParam("Ch3:EventRate",   0x3, 32,  0); // Channel 3 event rate         (unit:cnts/s)
    createCounterParam("Ch4:EventRate",   0x4, 32,  0); // Channel 4 event rate         (unit:cnts/s)
    createCounterParam("Ch5:EventRate",   0x5, 32,  0); // Channel 5 event rate         (unit:cnts/s)
    createCounterParam("Ch6:EventRate",   0x6, 32,  0); // Channel 6 event rate         (unit:cnts/s)
    createCounterParam("Ch7:EventRate",   0x7, 32,  0); // Channel 7 event rate         (unit:cnts/s)
    createCounterParam("SumEventRate",    0x8, 32,  0); // Total event rate             (unit:cnts/s)
    createCounterParam("PpsCount",        0x9, 32,  0); // Tracking PPS count           (unit:cnts/s)
    createCounterParam("NtpPpsCount",     0xA, 32,  0); // NTP PPS count                (unit:cnts/s)
    createCounterParam("PhaseError",      0xB, 32,  0); // Phase error                  (scale:9.411765,unit:ns)
    createCounterParam("EpochSeconds",    0xC, 32,  0); // EPOCH seconds                (unit:sec)
    createCounterParam("UTCSecond",       0xD,  6,  0); // UTCSecond                    (unit:sec)
    createCounterParam("UTCMinute",       0xD,  6,  6); // UTCMinute                    (unit:min)
    createCounterParam("UTCHour",         0xD,  5, 12); // UTCHour                      (unit:hour)
    createCounterParam("UTCDay",          0xD,  9, 17); // UTCDay                       (unit:day)
    createCounterParam("UTCYear",         0xD,  6, 26); // UTCYear                      (unit:year)
    createCounterParam("TotalMetaRate",   0xE, 32,  0); // Total Meta Rate              (unit:cnts/s)
    createCounterParam("UnusedCount0",    0xF, 32,  0); // UnusedCount0
}
