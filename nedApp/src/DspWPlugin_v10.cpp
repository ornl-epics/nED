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
    createConfigParam("Ch1:LoopEn",       '1', 0x1,  1, 15, 0);  // Channel 1 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch2:LoopEn",       '1', 0x1,  1, 16, 0);  // Channel 2 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch3:LoopEn",       '1', 0x1,  1, 17, 0);  // Channel 3 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch4:LoopEn",       '1', 0x1,  1, 18, 0);  // Channel 4 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch5:LoopEn",       '1', 0x1,  1, 19, 0);  // Channel 5 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch6:LoopEn",       '1', 0x1,  1, 20, 0);  // Channel 6 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch7:LoopEn",       '1', 0x1,  1, 21, 0);  // Channel 7 loopback enable  (0=disable,1=enable)
    createConfigParam("Ch8:LoopEn",       '1', 0x1,  1, 22, 0);  // Channel 8 loopback enable  (0=disable,1=enable)
    createConfigParam("Opt1:Enable",      '1', 0x3,  1,  0, 1);  // Optical 1 TX enable        (0=enable,1=disable)
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

//      BLXXX:Det:DspX:| sig nam|                     | EPICS record description | (bi and mbbi description)
    createStatusParam("Status1",       0x0, 32,  0); // RTDL frame CRC errors count
    createStatusParam("Status2",       0x0, 32,  0); // RTDL frame CRC errors count
    createStatusParam("Status3",       0x0, 32,  0); // RTDL frame CRC errors count
    createStatusParam("Status4",       0x1, 32,  0); // RTDL frame CRC errors count
}
