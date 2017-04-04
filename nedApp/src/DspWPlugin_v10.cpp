/* DspWPlugin_v10.cpp
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DspWPlugin.h"

void DspWPlugin::createParams_v10() {
//      BLXXX:Det:DspX:| sig nam|                                 | EPICS record description | (bi and mbbi description)
    createConfigParam("TestPatPeriod",    '1', 0x0,  8,  0, 15); // Pixel id offset
    createConfigParam("TsyncLimit",       '1', 0x0, 24,  8, 166666); // TSYNC limit            (scale:100,unit:ns)
    createConfigParam("Ch1:Enable",       '1', 0x1,  1,  0, 1);  // Channel 1 enable           (0=disable,1=enable)
    createConfigParam("Ch2:Enable",       '1', 0x1,  1,  1, 1);  // Channel 2 enable           (0=disable,1=enable)
    createConfigParam("Ch3:Enable",       '1', 0x1,  1,  2, 1);  // Channel 3 enable           (0=disable,1=enable)
    createConfigParam("Ch4:Enable",       '1', 0x1,  1,  3, 1);  // Channel 4 enable           (0=disable,1=enable)
    createConfigParam("Ch5:Enable",       '1', 0x1,  1,  4, 1);  // Channel 5 enable           (0=disable,1=enable)
    createConfigParam("Ch6:Enable",       '1', 0x1,  1,  5, 1);  // Channel 6 enable           (0=disable,1=enable)
    createConfigParam("Ch7:Enable",       '1', 0x1,  1,  6, 1);  // Channel 7 enable           (0=disable,1=enable)
    createConfigParam("Ch8:Enable",       '1', 0x1,  1,  7, 1);  // Channel 8 enable           (0=disable,1=enable)
    createConfigParam("Ch1:TestEn",       '1', 0x1,  1,  8, 0);  // Channel 1 test enable      (0=disable,1=enable)
    createConfigParam("Ch2:TestEn",       '1', 0x1,  1,  9, 0);  // Channel 2 test enable      (0=disable,1=enable)
    createConfigParam("Ch3:TestEn",       '1', 0x1,  1, 10, 0);  // Channel 3 test enable      (0=disable,1=enable)
    createConfigParam("Ch4:TestEn",       '1', 0x1,  1, 11, 0);  // Channel 4 test enable      (0=disable,1=enable)
    createConfigParam("Ch5:TestEn",       '1', 0x1,  1, 12, 0);  // Channel 5 test enable      (0=disable,1=enable)
    createConfigParam("Ch6:TestEn",       '1', 0x1,  1, 13, 0);  // Channel 6 test enable      (0=disable,1=enable)
    createConfigParam("Ch7:TestEn",       '1', 0x1,  1, 14, 0);  // Channel 7 test enable      (0=disable,1=enable)
    createConfigParam("Ch8:TestEn",       '1', 0x1,  1, 15, 0);  // Channel 8 test enable      (0=disable,1=enable)
    createConfigParam("Ch1:LoopEn",       '1', 0x1,  1, 16, 0);  // Channel 1 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch2:LoopEn",       '1', 0x1,  1, 17, 0);  // Channel 2 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch3:LoopEn",       '1', 0x1,  1, 18, 0);  // Channel 3 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch4:LoopEn",       '1', 0x1,  1, 19, 0);  // Channel 4 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch5:LoopEn",       '1', 0x1,  1, 20, 0);  // Channel 5 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch6:LoopEn",       '1', 0x1,  1, 21, 0);  // Channel 6 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch7:LoopEn",       '1', 0x1,  1, 22, 0);  // Channel 7 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch8:LoopEn",       '1', 0x1,  1, 23, 0);  // Channel 8 loopback enable  (0=disable,1=enable)
    createConfigParam("RunMode",          '1', 0x1,  3, 24, 0);  // Run mode                   (0=histogram,1=stroboscopic,2=test,3=loopback)
    createConfigParam("RateCounterSel",   '1', 0x2,  4, 27, 0);  // Rate counter select        (0=segment0,1=segment1,2=segment2,3=segment3,4=segment4,5=segment5,6=segment6=,7=segement7,8=neutrons,9=rollover,10=rejected)
    createConfigParam("Opt2:Enable",      '1', 0x3,  1,  1, 1);  // Optical 2 TX enable        (0=enable,1=disable)
    createConfigParam("Opt3:Enable",      '1', 0x3,  1,  2, 1);  // Optical 3 TX enable        (0=enable,1=disable)
    createConfigParam("Opt4:Enable",      '1', 0x3,  1,  3, 1);  // Optical 4 TX enable        (0=enable,1=disable)
    createConfigParam("Ch1:OptEn",        '1', 0x3,  1,  4, 0);  // Channel 1 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch2:OptEn",        '1', 0x3,  1,  5, 0);  // Channel 2 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch3:OptEn",        '1', 0x3,  1,  6, 0);  // Channel 3 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch4:OptEn",        '1', 0x3,  1,  7, 0);  // Channel 4 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch5:OptEn",        '1', 0x3,  1,  8, 0);  // Channel 5 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch6:OptEn",        '1', 0x3,  1,  9, 0);  // Channel 6 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch7:OptEn",        '1', 0x3,  1, 10, 0);  // Channel 7 optical TX en    (0=enable,1=disable)
    createConfigParam("Ch8:OptEn",        '1', 0x3,  1, 11, 0);  // Channel 8 optical TX en    (0=enable,1=disable)
    createConfigParam("CrcNackEn",        '1', 0x3,  1, 20, 0);  // CRC NACK enable            (0=disable,1=enable)
    createConfigParam("MaxDataPktSize",   '1', 0x3,  3, 21, 0);  // Max data packet size       (0=1000 bytes, 1=2000 bytes, 3=3000 bytes)

//      BLXXX:Det:DspX:| sig nam|                          | EPICS record description | (bi and mbbi description)
    createStatusParam("OccLinkStatus",    0x0,  1,  0); // OCC link status              (0=connected,1=disconnected)
    createStatusParam("OdbLinkStatus",    0x0,  1,  1); // ODB link status              (0=connected,1=disconnected)
    createStatusParam("Aux1LinkStatus",   0x0,  1,  2); // AUX1 link status             (0=connected,1=disconnected)
    createStatusParam("Aux2LinkStatus",   0x0,  1,  3); // AUX2 link status             (0=connected,1=disconnected)
    createStatusParam("Ch1:LinkStatus",   0x0,  1,  4); // Channel 1 link status        (0=connected,1=disconnected)
    createStatusParam("Ch2:LinkStatus",   0x0,  1,  5); // Channel 2 link status        (0=connected,1=disconnected)
    createStatusParam("Ch3:LinkStatus",   0x0,  1,  6); // Channel 3 link status        (0=connected,1=disconnected)
    createStatusParam("Ch4:LinkStatus",   0x0,  1,  7); // Channel 4 link status        (0=connected,1=disconnected)
    createStatusParam("Ch5:LinkStatus",   0x0,  1,  8); // Channel 5 link status        (0=connected,1=disconnected)
    createStatusParam("Ch6:LinkStatus",   0x0,  1,  9); // Channel 6 link status        (0=connected,1=disconnected)
    createStatusParam("Ch7:LinkStatus",   0x0,  1, 10); // Channel 7 link status        (0=connected,1=disconnected)
    createStatusParam("Ch8:LinkStatus",   0x0,  1, 11); // Channel 8 link status        (0=connected,1=disconnected)
    createStatusParam("NtpStatus",        0x0,  1, 12); // NTP timing status            (0=present,1=absent)
    createStatusParam("Ch1:MetaData",     0x0,  1, 16); // Channel 1 seen meta data     (0=yes,1=no)
    createStatusParam("Ch2:MetaData",     0x0,  1, 17); // Channel 2 seen meta data     (0=yes,1=no)
    createStatusParam("Ch3:MetaData",     0x0,  1, 18); // Channel 3 seen meta data     (0=yes,1=no)
    createStatusParam("Ch4:MetaData",     0x0,  1, 19); // Channel 4 seen meta data     (0=yes,1=no)
    createStatusParam("Ch5:MetaData",     0x0,  1, 20); // Channel 5 seen meta data     (0=yes,1=no)
    createStatusParam("Ch6:MetaData",     0x0,  1, 21); // Channel 6 seen meta data     (0=yes,1=no)
    createStatusParam("Ch7:MetaData",     0x0,  1, 22); // Channel 7 seen meta data     (0=yes,1=no)
    createStatusParam("Ch8:MetaData",     0x0,  1, 23); // Channel 8 seen meta data     (0=yes,1=no)
    createStatusParam("Ch1:NeutronData",  0x0,  1, 24); // Channel 1 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch2:NeutronData",  0x0,  1, 25); // Channel 2 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch3:NeutronData",  0x0,  1, 26); // Channel 3 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch4:NeutronData",  0x0,  1, 27); // Channel 4 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch5:NeutronData",  0x0,  1, 28); // Channel 5 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch6:NeutronData",  0x0,  1, 29); // Channel 6 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch7:NeutronData",  0x0,  1, 30); // Channel 7 seen neutron data  (0=yes,1=no)
    createStatusParam("Ch8:NeutronData",  0x0,  1, 31); // Channel 8 seen neutron data  (0=yes,1=no)
    createStatusParam("Acquiring",        0x1,  1,  0); // Acquiring data               (0=not acquiring,1=acquiring, archive:monitor)
    createConfigParam("RunModeRB",        0x1,  3,  1); // Run mode                     (0=histogram,1=stroboscopic,2=test,3=loopback)
    createConfigParam("RateCounterSelRB", 0x1,  4,  4); // Rate counter select          (0=segment0,1=segment1,2=segment2,3=segment3,4=segment4,5=segment5,6=segment6=,7=segement7,8=neutrons,9=rollover,10=rejected)
    createConfigParam("RateCounter",      0x2, 32,  0); // Selected rate counter
    createConfigParam("OverTemp",         0x3,  1,  0); // Over temperature alarm       (0=no alarm,1=alarm)
    createConfigParam("UserTemp",         0x3,  1,  1); // User temperature alarm       (0=no alarm,1=alarm)
    createConfigParam("VccInt",           0x3,  1,  2); // VCCINT alarm                 (0=no alarm,1=alarm)
    createConfigParam("VccAux",           0x3,  1,  3); // VCCAUX alarm                 (0=no alarm,1=alarm)

    createCounterParam("Ch1:EventRate",   0x0, 16,  0); // Channel 1 event rate
    createCounterParam("Ch2:EventRate",   0x0, 16, 16); // Channel 2 event rate
    createCounterParam("Ch3:EventRate",   0x1, 16,  0); // Channel 3 event rate
    createCounterParam("Ch4:EventRate",   0x1, 16, 16); // Channel 4 event rate
    createCounterParam("Ch5:EventRate",   0x2, 16,  0); // Channel 5 event rate
    createCounterParam("Ch6:EventRate",   0x2, 16, 16); // Channel 6 event rate
    createCounterParam("Ch7:EventRate",   0x3, 16,  0); // Channel 7 event rate
    createCounterParam("Ch8:EventRate",   0x3, 16, 16); // Channel 8 event rate
}
