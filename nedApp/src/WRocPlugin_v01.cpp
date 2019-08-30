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
    createRegParam("VERSION", "HwRev",  true, 0,  8,  0);   // Hardware revision        (low:3,high:5)
    createRegParam("VERSION", "HwVer",  true, 0,  8,  8);   // Hardware version         (low:1,high:3)
    createRegParam("VERSION", "FwRev",  true, 0,  8, 16);   // Firmware revision        (low:0,high:2)
    createRegParam("VERSION", "FwVer",  true, 0,  8, 24);   // Firmware version         (low:-1,high:1)
    createRegParam("VERSION", "FwYear", true, 1, 16,  0, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwDay",  true, 1,  8, 16, 0, CONV_HEX2DEC);
    createRegParam("VERSION", "FwMonth",true, 1,  8, 24, 0, CONV_HEX2DEC);

    createConfigParam("Protocol",     '1', 0,  1, 0, 0);    // LVDS Protocol Select   (0=new,1=legacy)
    createConfigParam("TcTclkMode",   '1', 0,  1, 1, 1);    // T&C TCLK Source Select (0=internal,1=external)
    createConfigParam("Ch0:Enable",   '1', 1,  1, 0, 1);    // Chan0 Enable           (0=disable,1=enable)
    createConfigParam("Ch1:Enable",   '1', 1,  1, 1, 1);    // Chan1 Enable           (0=disable,1=enable)
    createConfigParam("Ch2:Enable",   '1', 1,  1, 2, 1);    // Chan2 Enable           (0=disable,1=enable)
    createConfigParam("Ch3:Enable",   '1', 1,  1, 3, 1);    // Chan3 Enable           (0=disable,1=enable)
    createConfigParam("Ch4:Enable",   '1', 1,  1, 4, 1);    // Chan4 Enable           (0=disable,1=enable)
    createConfigParam("Ch5:Enable",   '1', 1,  1, 5, 1);    // Chan5 Enable           (0=disable,1=enable)
    createConfigParam("Ch6:Enable",   '1', 1,  1, 6, 1);    // Chan6 Enable           (0=disable,1=enable)
    createConfigParam("Ch7:Enable",   '1', 1,  1, 7, 1);    // Chan7 Enable           (0=disable,1=enable)
    createConfigParam("Ch0:Loopback", '1', 1,  1, 8, 0);    // Chan0 Loopback         (0=disable,1=enable)
    createConfigParam("Ch1:Loopback", '1', 1,  1, 9, 0);    // Chan1 Loopback         (0=disable,1=enable)
    createConfigParam("Ch2:Loopback", '1', 1,  1,10, 0);    // Chan2 Loopback         (0=disable,1=enable)
    createConfigParam("Ch3:Loopback", '1', 1,  1,11, 0);    // Chan3 Loopback         (0=disable,1=enable)
    createConfigParam("Ch4:Loopback", '1', 1,  1,12, 0);    // Chan4 Loopback         (0=disable,1=enable)
    createConfigParam("Ch5:Loopback", '1', 1,  1,13, 0);    // Chan5 Loopback         (0=disable,1=enable)
    createConfigParam("Ch6:Loopback", '1', 1,  1,14, 0);    // Chan6 Loopback         (0=disable,1=enable)
    createConfigParam("Ch7:Loopback", '1', 1,  1,15, 0);    // Chan7 Loopback         (0=disable,1=enable)
    createConfigParam("Ch0:TestData", '1', 1,  1,16, 0);    // Chan0 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch1:TestData", '1', 1,  1,17, 0);    // Chan1 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch2:TestData", '1', 1,  1,18, 0);    // Chan2 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch3:TestData", '1', 1,  1,19, 0);    // Chan3 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch4:TestData", '1', 1,  1,20, 0);    // Chan4 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch5:TestData", '1', 1,  1,21, 0);    // Chan5 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch6:TestData", '1', 1,  1,22, 0);    // Chan6 Test Enable      (0=disable,1=enable)
    createConfigParam("Ch7:TestData", '1', 1,  1,23, 0);    // Chan7 Test Enable      (0=disable,1=enable)
    createConfigParam("TestPeriod",   '1', 1,  8,24, 1000); // Test Period            (offset:1260,scale:1280,unit:ns)

    createStatusParam("Ch0:Sfp",          0,  1, 0);        // SFP0 status            (0=present,1=not present [alarm])
    createStatusParam("Ch1:Sfp",          0,  1, 1);        // SFP1 status            (0=present,1=not present [alarm])
    createStatusParam("Ch2:Sfp",          0,  1, 2);        // SFP2 status            (0=present,1=not present [alarm])
    createStatusParam("Ch3:Sfp",          0,  1, 3);        // SFP3 status            (0=present,1=not present [alarm])
    createStatusParam("Ch4:Sfp",          0,  1, 4);        // SFP4 status            (0=present,1=not present [alarm])
    createStatusParam("Ch5:Sfp",          0,  1, 5);        // SFP5 status            (0=present,1=not present [alarm])
    createStatusParam("Ch6:Sfp",          0,  1, 6);        // SFP6 status            (0=present,1=not present [alarm])
    createStatusParam("Ch7:Sfp",          0,  1, 7);        // SFP7 status            (0=present,1=not present [alarm])
    createStatusParam("OverTemp",         0,  1, 8);        // Over temperature alarm (0=normal,1=over [alarm])
    createStatusParam("UserTemp",         0,  1, 9);        // User temperature alarm (0=normal,1=over [alarm])
    createStatusParam("VCCInt",           0,  1, 10);       // VCCINT alarm           (0=no alarm,1=alarm [alarm])
    createStatusParam("VCCAux",           0,  1, 11);       // VCCAUX alarm           (0=no alarm,1=alarm [alarm])
    createStatusParam("Acquiring",        0,  1, 12);       // Acquiring data         (0=not acquiring,1=acquiring, archive:monitor)
    createStatusParam("Ch0:Enabled",      0,  1, 13);       // Chan0 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch1:Enabled",      0,  1, 14);       // Chan1 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch2:Enabled",      0,  1, 15);       // Chan2 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch3:Enabled",      0,  1, 16);       // Chan3 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch4:Enabled",      0,  1, 17);       // Chan4 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch5:Enabled",      0,  1, 18);       // Chan5 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch6:Enabled",      0,  1, 19);       // Chan6 enabled          (0=disabled,1=enabled)
    createStatusParam("Ch7:Enabled",      0,  1, 20);       // Chan7 enabled          (0=disabled,1=enabled)
    createStatusParam("Configured",       0,  1, 21);       // Module configured      (0=not configured [alarm],1=configured)

    createCounterParam("Ch0:CntPackets",0,  16,  0);        // Chan0 num RX packets
    createCounterParam("Ch1:CntPackets",0,  16, 16);        // Chan1 num RX packets
    createCounterParam("Ch2:CntPackets",1,  16,  0);        // Chan2 num RX packets
    createCounterParam("Ch3:CntPackets",1,  16, 16);        // Chan3 num RX packets
    createCounterParam("Ch4:CntPackets",2,  16,  0);        // Chan4 num RX packets
    createCounterParam("Ch5:CntPackets",2,  16, 16);        // Chan5 num RX packets
    createCounterParam("Ch6:CntPackets",3,  16,  0);        // Chan6 num RX packets
    createCounterParam("Ch7:CntPackets",3,  16, 16);        // Chan7 num RX packets
}
