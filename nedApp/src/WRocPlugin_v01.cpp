/* WRocPlugin_v01.cpp
 *
 * Copyright (c) 2019 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "WRocPlugin.h"

/**
 * @file WRocPlugin_v01.cpp
 *
 * WROC 0.1 parameters
 */

void WRocPlugin::createParams_v01()
{
//    createRegParam("VERSION", "ModuleType", true, 0,  8, 0);   // Module type               (low:46,high:48)
    createRegParam("VERSION", "HwRev",      true, 0,  8,  0);   // Hardware revision        (low:3,high:5)
    createRegParam("VERSION", "HwVer",  true, 0,  8,  8);   // Hardware version         (low:1,high:3)
    createRegParam("VERSION", "FwRev",  true, 0,  8, 16);   // Firmware revision        (low:0,high:2)
    createRegParam("VERSION", "FwVer",  true, 0,  8, 24);   // Firmware version         (low:-1,high:1)
    createRegParam("VERSION", "FwYear", true, 1, 16,  0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwDay",  true, 1,  8, 16, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwMonth",true, 1,  8, 24, 0, CONV_HEX2DEC);

    createConfigParam("NewFormat",  '1', 0,  1, 0, 0);    
    createConfigParam("ChannelEn0", '1', 1,  1, 0, 1);
    createConfigParam("ChannelEn1", '1', 1,  1, 1, 1);
    createConfigParam("ChannelEn2", '1', 1,  1, 2, 1);
    createConfigParam("ChannelEn3", '1', 1,  1, 3, 1);
    createConfigParam("ChannelEn4", '1', 1,  1, 4, 1);
    createConfigParam("ChannelEn5", '1', 1,  1, 5, 1);
    createConfigParam("ChannelEn6", '1', 1,  1, 6, 1);
    createConfigParam("ChannelEn7", '1', 1,  1, 7, 1);
    createConfigParam("LoopbackEn0",'1', 1,  1, 8, 0);
    createConfigParam("LoopbackEn1",'1', 1,  1, 9, 0);
    createConfigParam("LoopbackEn2",'1', 1,  1,10, 0);
    createConfigParam("LoopbackEn3",'1', 1,  1,11, 0);
    createConfigParam("LoopbackEn4",'1', 1,  1,12, 0);
    createConfigParam("LoopbackEn5",'1', 1,  1,13, 0);
    createConfigParam("LoopbackEn6",'1', 1,  1,14, 0);
    createConfigParam("LoopbackEn7",'1', 1,  1,15, 0);
    createConfigParam("TestEn0",    '1', 1,  1,16, 0);
    createConfigParam("TestEn1",    '1', 1,  1,17, 0);
    createConfigParam("TestEn2",    '1', 1,  1,18, 0);
    createConfigParam("TestEn3",    '1', 1,  1,19, 0);
    createConfigParam("TestEn4",    '1', 1,  1,20, 0);
    createConfigParam("TestEn5",    '1', 1,  1,21, 0);
    createConfigParam("TestEn6",    '1', 1,  1,22, 0);
    createConfigParam("TestEn7",    '1', 1,  1,23, 0);
    createConfigParam("TestRate",   '1', 1,  8,24, 100);

    createStatusParam("SfpLoss0",      0,  1, 0);
    createStatusParam("SfpLoss1",      0,  1, 1);
    createStatusParam("SfpLoss2",      0,  1, 2);
    createStatusParam("SfpLoss3",      0,  1, 3);
    createStatusParam("SfpLoss4",      0,  1, 4);
    createStatusParam("SfpLoss5",      0,  1, 5);
    createStatusParam("SfpLoss6",      0,  1, 6);
    createStatusParam("SfpLoss7",      0,  1, 7);
    createStatusParam("OverTempAlarm", 0,  1, 8);
    createStatusParam("UserTempAlarm", 0,  1, 9);
    createStatusParam("VCCIntAlarm",   0,  1, 10);
    createStatusParam("VCCAuxAlarm",   0,  1, 11);
    createStatusParam("Run",           0,  1, 12);
    createStatusParam("ChannActive0",  0,  1, 13);
    createStatusParam("ChannActive1",  0,  1, 14);
    createStatusParam("ChannActive2",  0,  1, 15);
    createStatusParam("ChannActive3",  0,  1, 16);
    createStatusParam("ChannActive4",  0,  1, 17);
    createStatusParam("ChannActive5",  0,  1, 18);
    createStatusParam("ChannActive6",  0,  1, 19);
    createStatusParam("ChannActive7",  0,  1, 20);
    createStatusParam("Configured",    0,  1, 21);

    createCounterParam("Chann0",  0,  16, 0);
    createCounterParam("Chann1",  0,  16, 16);
    createCounterParam("Chann2",  1,  16, 0);
    createCounterParam("Chann3",  1,  16, 16);
    createCounterParam("Chann4",  2,  16, 0);
    createCounterParam("Chann5",  2,  16, 16);
    createCounterParam("Chann6",  3,  16, 0);
    createCounterParam("Chann7",  3,  16, 16);
}
