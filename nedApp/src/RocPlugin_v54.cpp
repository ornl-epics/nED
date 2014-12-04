/* RocPlugin_v54.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "RocPlugin.h"

/**
 * @file RocPlugin_v54.cpp
 *
 * ROC 5.4 parameters
 *
 * ROC 5.4 firmware seems to be an update of 5.2, adding baseline support.
 * The status registers are all different though, unfortunately the AcquireStatus
 * register is no longer available. There are other quirks as well. The version
 * response includes additional 'vendor' information. The READ_CONFIG response
 * reports bad packet length - v5.5 fixes that.
 */

void RocPlugin::createStatusParams_v54()
{
//    BLXXX:Det:RocXXX:| sig nam |                         | EPICS record description  | (bi and mbbi description)
    createStatusParam("ErrParity",        0x0,  1, 10); // LVDS parity error.            (0=no error,1=error)
    createStatusParam("Configured",       0x0,  1,  9); // Not configured                (0=configured,1=not configured)
    createStatusParam("Discovered",       0x0,  1,  8); // Configured                    (0=discovered,1=not discovered)
    createStatusParam("SysrstBHigh",      0x0,  1,  7); // SYSRST_B Got HIGH             (0=no,1=yes)
    createStatusParam("SysrstBLow",       0x0,  1,  6); // SYSRST_B Got LOW              (0=no,1=yes)
    createStatusParam("TxenBHigh",        0x0,  1,  5); // TXEN_B Got HIGH               (0=no,1=yes)
    createStatusParam("TxenBLow",         0x0,  1,  4); // TXEN_B Got LOW                (0=no,1=yes)
    createStatusParam("TsyncHigh",        0x0,  1,  3); // TSYNC Got HIGH                (0=no,1=yes)
    createStatusParam("TsyncLow",         0x0,  1,  2); // TSYNC Got LOW                 (0=no,1=yes)
    createStatusParam("TclkHigh",         0x0,  1,  1); // TCLK Got HIGH                 (0=no,1=yes)
    createStatusParam("TclkLow",          0x0,  1,  0); // TCLK Got LOW                  (0=no,1=yes)

    createStatusParam("HVStatus",         0x1,  1,  1); // High Voltage Status bit       (0=not present,1=present)
    createStatusParam("AutoCorrection",   0x1,  1,  0); // Auto correction enabled       (0=disabled,1=enabled)

    createStatusParam("Ch2:Event",        0x2,  1, 13); // Chan2 got event               (0=no,1=yes)
    createStatusParam("Ch2:DiscHigh",     0x2,  1, 12); // Chan2 disc high               (0=low,1=high)
    createStatusParam("Ch2:FifoFull",     0x2,  1, 11); // Chan2 FIFO full               (0=not full,1=full)
    createStatusParam("Ch2:CalcErr",      0x2,  1, 10); // Chan2 calc out of range       (0=no,1=yes)
    createStatusParam("Ch2:NegDenom",     0x2,  1,  9); // Chan2 negative denom          (0=no,1=yes)
    createStatusParam("Ch2:CalcOverflow", 0x2,  1,  8); // Chan2 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch1:Event",        0x2,  1,  5); // Chan1 got event               (0=no,1=yes)
    createStatusParam("Ch1:DiscHigh",     0x2,  1,  4); // Chan1 disc high               (0=low,1=high)
    createStatusParam("Ch1:FifoFull",     0x2,  1,  3); // Chan1 FIFO full               (0=not full,1=full)
    createStatusParam("Ch1:CalcErr",      0x2,  1,  2); // Chan1 calc out of range       (0=no,1=yes)
    createStatusParam("Ch1:NegDenom",     0x2,  1,  1); // Chan1 negative denom          (0=no,1=yes)
    createStatusParam("Ch1:CalcOverflow", 0x2,  1,  0); // Chan1 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch4:Event",        0x3,  1, 13); // Chan4 got event               (0=no,1=yes)
    createStatusParam("Ch4:DiscHigh",     0x3,  1, 12); // Chan4 disc high               (0=low,1=high)
    createStatusParam("Ch4:FifoFull",     0x3,  1, 11); // Chan4 FIFO full               (0=not full,1=full)
    createStatusParam("Ch4:CalcErr",      0x3,  1, 10); // Chan4 calc out of range       (0=no,1=yes)
    createStatusParam("Ch4:NegDenom",     0x3,  1,  9); // Chan4 negative denom          (0=no,1=yes)
    createStatusParam("Ch4:CalcOverflow", 0x3,  1,  8); // Chan4 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch3:Event",        0x3,  1,  5); // Chan3 got event               (0=no,1=yes)
    createStatusParam("Ch3:DiscHigh",     0x3,  1,  4); // Chan3 disc high               (0=low,1=high)
    createStatusParam("Ch3:FifoFull",     0x3,  1,  3); // Chan3 FIFO full               (0=not full,1=full)
    createStatusParam("Ch3:CalcErr",      0x3,  1,  2); // Chan3 calc out of range       (0=no,1=yes)
    createStatusParam("Ch3:NegDenom",     0x3,  1,  1); // Chan3 negative denom          (0=no,1=yes)
    createStatusParam("Ch3:CalcOverflow", 0x3,  1,  0); // Chan3 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch6:Event",        0x4,  1, 13); // Chan6 got event               (0=no,1=yes)
    createStatusParam("Ch6:DiscHigh",     0x4,  1, 12); // Chan6 disc high               (0=low,1=high)
    createStatusParam("Ch6:FifoFull",     0x4,  1, 11); // Chan6 FIFO full               (0=not full,1=full)
    createStatusParam("Ch6:CalcErr",      0x4,  1, 10); // Chan6 calc out of range       (0=no,1=yes)
    createStatusParam("Ch6:NegDenom",     0x4,  1,  9); // Chan6 negative denom          (0=no,1=yes)
    createStatusParam("Ch6:CalcOverflow", 0x4,  1,  8); // Chan6 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch5:Event",        0x4,  1,  5); // Chan5 got event               (0=no,1=yes)
    createStatusParam("Ch5:DiscHigh",     0x4,  1,  4); // Chan5 disc high               (0=low,1=high)
    createStatusParam("Ch5:FifoFull",     0x4,  1,  3); // Chan5 FIFO full               (0=not full,1=full)
    createStatusParam("Ch5:CalcErr",      0x4,  1,  2); // Chan5 calc out of range       (0=no,1=yes)
    createStatusParam("Ch5:NegDenom",     0x4,  1,  1); // Chan5 negative denom          (0=no,1=yes)
    createStatusParam("Ch5:CalcOverflow", 0x4,  1,  0); // Chan5 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch8:Event",        0x5,  1, 13); // Chan8 got event               (0=no,1=yes)
    createStatusParam("Ch8:DiscHigh",     0x5,  1, 12); // Chan8 disc high               (0=low,1=high)
    createStatusParam("Ch8:FifoFull",     0x5,  1, 11); // Chan8 FIFO full               (0=not full,1=full)
    createStatusParam("Ch8:CalcErr",      0x5,  1, 10); // Chan8 calc out of range       (0=no,1=yes)
    createStatusParam("Ch8:NegDenom",     0x5,  1,  9); // Chan8 negative denom          (0=no,1=yes)
    createStatusParam("Ch8:CalcOverflow", 0x5,  1,  8); // Chan8 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch7:Event",        0x5,  1,  5); // Chan7 got event               (0=no,1=yes)
    createStatusParam("Ch7:DiscHigh",     0x5,  1,  4); // Chan7 disc high               (0=low,1=high)
    createStatusParam("Ch7:FifoFull",     0x5,  1,  3); // Chan7 FIFO full               (0=not full,1=full)
    createStatusParam("Ch7:CalcErr",      0x5,  1,  2); // Chan7 calc out of range       (0=no,1=yes)
    createStatusParam("Ch7:NegDenom",     0x5,  1,  1); // Chan7 negative denom          (0=no,1=yes)
    createStatusParam("Ch7:CalcOverflow", 0x5,  1,  0); // Chan7 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("A1BaseAvg",        0x6, 15,  0); // A1 baseline average
    createStatusParam("B1BaseAvg",        0x7, 15,  0); // B1 baseline average
    createStatusParam("A2BaseAvg",        0x8, 15,  0); // A2 baseline average
    createStatusParam("B2BaseAvg",        0x9, 15,  0); // B2 baseline average
    createStatusParam("A3BaseAvg",        0xA, 15,  0); // A3 baseline average
    createStatusParam("B3BaseAvg",        0xB, 15,  0); // B3 baseline average
    createStatusParam("A4BaseAvg",        0xC, 15,  0); // A4 baseline average
    createStatusParam("B4BaseAvg",        0xD, 15,  0); // B4 baseline average
    createStatusParam("A5BaseAvg",        0xE, 15,  0); // A5 baseline average
    createStatusParam("B5BaseAvg",        0xF, 15,  0); // B5 baseline average
    createStatusParam("A6BaseAvg",        0x10,15,  0); // A6 baseline average
    createStatusParam("B6BaseAvg",        0x11,15,  0); // B6 baseline average
    createStatusParam("A7BaseAvg",        0x12,15,  0); // A7 baseline average
    createStatusParam("B7BaseAvg",        0x13,15,  0); // B7 baseline average
    createStatusParam("A8BaseAvg",        0x14,15,  0); // A8 baseline average
    createStatusParam("B8BaseAvg",        0x15,15,  0); // B8 baseline average

    // ROC lies about the length of the packet. It returns 44 bytes of valid
    // data but the packet length is 64 bytes => 20 bytes or 10 registers
    // We only need to define the last one which defines the packet length
    createStatusParam("StatFill",         0x1F,15,  0);
}

void RocPlugin::createConfigParams_v54()
{
    // TODO: minor changes between v52 and v54, but are not yet addressed

//    BLXXX:Det:RocXXX:| sig nam |                                    | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:PositionIdx", '1', 0x0,  32, 0, 0);     // Chan1 position index
    createConfigParam("Ch2:PositionIdx", '1', 0x2,  32, 0, 256);   // Chan2 position index
    createConfigParam("Ch3:PositionIdx", '1', 0x4,  32, 0, 512);   // Chan3 position index
    createConfigParam("Ch4:PositionIdx", '1', 0x6,  32, 0, 768);   // Chan4 position index
    createConfigParam("Ch5:PositionIdx", '1', 0x8,  32, 0, 1024);  // Chan5 position index
    createConfigParam("Ch6:PositionIdx", '1', 0xA,  32, 0, 1280);  // Chan6 position index
    createConfigParam("Ch7:PositionIdx", '1', 0xC,  32, 0, 1536);  // Chan7 position index
    createConfigParam("Ch8:PositionIdx", '1', 0xE,  32, 0, 1792);  // Chan8 position index

    createConfigParam("Ch1A:InOffset",   '2', 0x0,  12, 0, 100);   // Chan1 A input offset
    createConfigParam("Ch2A:InOffset",   '2', 0x1,  12, 0, 100);   // Chan2 A input offset
    createConfigParam("Ch3A:InOffset",   '2', 0x2,  12, 0, 100);   // Chan3 A input offset
    createConfigParam("Ch4A:InOffset",   '2', 0x3,  12, 0, 100);   // Chan4 A input offset
    createConfigParam("Ch5A:InOffset",   '2', 0x4,  12, 0, 100);   // Chan5 A input offset
    createConfigParam("Ch6A:InOffset",   '2', 0x5,  12, 0, 100);   // Chan6 A input offset
    createConfigParam("Ch7A:InOffset",   '2', 0x6,  12, 0, 100);   // Chan7 A input offset
    createConfigParam("Ch8A:InOffset",   '2', 0x7,  12, 0, 100);   // Chan8 A input offset
    createConfigParam("Ch1B:InOffset",   '2', 0x8,  12, 0, 100);   // Chan1 B input offset
    createConfigParam("Ch2B:InOffset",   '2', 0x9,  12, 0, 100);   // Chan2 B input offset
    createConfigParam("Ch3B:InOffset",   '2', 0xA,  12, 0, 100);   // Chan3 B input offset
    createConfigParam("Ch4B:InOffset",   '2', 0xB,  12, 0, 100);   // Chan4 B input offset
    createConfigParam("Ch5B:InOffset",   '2', 0xC,  12, 0, 100);   // Chan5 B input offset
    createConfigParam("Ch6B:InOffset",   '2', 0xD,  12, 0, 100);   // Chan6 B input offset
    createConfigParam("Ch7B:InOffset",   '2', 0xE,  12, 0, 100);   // Chan7 B input offset
    createConfigParam("Ch8B:InOffset",   '2', 0xF,  12, 0, 100);   // Chan8 B input offset

    createConfigParam("Ch1A:AdcOffset",  '2', 0x10, 12, 0, 100);   // Chan1 A ADC offset
    createConfigParam("Ch2A:AdcOffset",  '2', 0x11, 12, 0, 100);   // Chan2 A ADC offset
    createConfigParam("Ch3A:AdcOffset",  '2', 0x12, 12, 0, 100);   // Chan3 A ADC offset
    createConfigParam("Ch4A:AdcOffset",  '2', 0x13, 12, 0, 100);   // Chan4 A ADC offset
    createConfigParam("Ch5A:AdcOffset",  '2', 0x14, 12, 0, 100);   // Chan5 A ADC offset
    createConfigParam("Ch6A:AdcOffset",  '2', 0x15, 12, 0, 100);   // Chan6 A ADC offset
    createConfigParam("Ch7A:AdcOffset",  '2', 0x16, 12, 0, 100);   // Chan7 A ADC offset
    createConfigParam("Ch8A:AdcOffset",  '2', 0x17, 12, 0, 100);   // Chan8 A ADC offset
    createConfigParam("Ch1B:AdcOffset",  '2', 0x18, 12, 0, 100);   // Chan1 B ADC offset
    createConfigParam("Ch2B:AdcOffset",  '2', 0x19, 12, 0, 100);   // Chan2 B ADC offset
    createConfigParam("Ch3B:AdcOffset",  '2', 0x1A, 12, 0, 100);   // Chan3 B ADC offset
    createConfigParam("Ch4B:AdcOffset",  '2', 0x1B, 12, 0, 100);   // Chan4 B ADC offset
    createConfigParam("Ch5B:AdcOffset",  '2', 0x1C, 12, 0, 100);   // Chan5 B ADC offset
    createConfigParam("Ch6B:AdcOffset",  '2', 0x1D, 12, 0, 100);   // Chan6 B ADC offset
    createConfigParam("Ch7B:AdcOffset",  '2', 0x1E, 12, 0, 100);   // Chan7 B ADC offset
    createConfigParam("Ch8B:AdcOffset",  '2', 0x1F, 12, 0, 100);   // Chan8 B ADC offset

    createConfigParam("Ch1A:Threshold",  '2', 0x20, 12, 0, 400);   // Chan1 A threshold
    createConfigParam("Ch2A:Threshold",  '2', 0x21, 12, 0, 400);   // Chan2 A threshold
    createConfigParam("Ch3A:Threshold",  '2', 0x22, 12, 0, 400);   // Chan3 A threshold
    createConfigParam("Ch4A:Threshold",  '2', 0x23, 12, 0, 400);   // Chan4 A threshold
    createConfigParam("Ch5A:Threshold",  '2', 0x24, 12, 0, 400);   // Chan5 A threshold
    createConfigParam("Ch6A:Threshold",  '2', 0x25, 12, 0, 400);   // Chan6 A threshold
    createConfigParam("Ch7A:Threshold",  '2', 0x26, 12, 0, 400);   // Chan7 A threshold
    createConfigParam("Ch8A:Threshold",  '2', 0x27, 12, 0, 400);   // Chan8 A threshold
    createConfigParam("Ch1B:Threshold",  '2', 0x28, 12, 0, 400);   // Chan1 B threshold
    createConfigParam("Ch2B:Threshold",  '2', 0x29, 12, 0, 400);   // Chan2 B threshold
    createConfigParam("Ch3B:Threshold",  '2', 0x2A, 12, 0, 400);   // Chan3 B threshold
    createConfigParam("Ch4B:Threshold",  '2', 0x2B, 12, 0, 400);   // Chan4 B threshold
    createConfigParam("Ch5B:Threshold",  '2', 0x2C, 12, 0, 400);   // Chan5 B threshold
    createConfigParam("Ch6B:Threshold",  '2', 0x2D, 12, 0, 400);   // Chan6 B threshold
    createConfigParam("Ch7B:Threshold",  '2', 0x2E, 12, 0, 400);   // Chan7 B threshold
    createConfigParam("Ch8B:Threshold",  '2', 0x2F, 12, 0, 400);   // Chan8 B threshold

    createConfigParam("Ch1:PosThresold", '2', 0x30, 12, 0, 400);   // Chan1 positive threshold
    createConfigParam("Ch2:PosThresold", '2', 0x31, 12, 0, 400);   // Chan2 positive threshold
    createConfigParam("Ch3:PosThresold", '2', 0x32, 12, 0, 400);   // Chan3 positive threshold
    createConfigParam("Ch4:PosThresold", '2', 0x33, 12, 0, 400);   // Chan4 positive threshold
    createConfigParam("Ch5:PosThresold", '2', 0x34, 12, 0, 400);   // Chan5 positive threshold
    createConfigParam("Ch6:PosThresold", '2', 0x35, 12, 0, 400);   // Chan6 positive threshold
    createConfigParam("Ch7:PosThresold", '2', 0x36, 12, 0, 400);   // Chan7 positive threshold
    createConfigParam("Ch8:PosThresold", '2', 0x37, 12, 0, 400);   // Chan8 positive threshold
    createConfigParam("Ch1A:FullScale",  '2', 0x38, 12, 0, 10);    // Chan1 A full scale
    createConfigParam("Ch2A:FullScale",  '2', 0x39, 12, 0, 10);    // Chan2 A full scale
    createConfigParam("Ch3A:FullScale",  '2', 0x3A, 12, 0, 10);    // Chan3 A full scale
    createConfigParam("Ch4A:FullScale",  '2', 0x3B, 12, 0, 10);    // Chan4 A full scale
    createConfigParam("Ch5A:FullScale",  '2', 0x3C, 12, 0, 10);    // Chan5 A full scale
    createConfigParam("Ch6A:FullScale",  '2', 0x3D, 12, 0, 10);    // Chan6 A full scale
    createConfigParam("Ch7A:FullScale",  '2', 0x3E, 12, 0, 10);    // Chan7 A full scale
    createConfigParam("Ch8A:FullScale",  '2', 0x3F, 12, 0, 10);    // Chan8 A full scale

    createConfigParam("Ch1B:FullScale",  '2', 0x40, 12, 0, 10);    // Chan1 B full scale
    createConfigParam("Ch2B:FullScale",  '2', 0x41, 12, 0, 10);    // Chan2 B full scale
    createConfigParam("Ch3B:FullScale",  '2', 0x42, 12, 0, 10);    // Chan3 B full scale
    createConfigParam("Ch4B:FullScale",  '2', 0x43, 12, 0, 10);    // Chan4 B full scale
    createConfigParam("Ch5B:FullScale",  '2', 0x44, 12, 0, 10);    // Chan5 B full scale
    createConfigParam("Ch6B:FullScale",  '2', 0x45, 12, 0, 10);    // Chan6 B full scale
    createConfigParam("Ch7B:FullScale",  '2', 0x46, 12, 0, 10);    // Chan7 B full scale
    createConfigParam("Ch8B:FullScale",  '2', 0x47, 12, 0, 10);    // Chan8 B full scale
    createConfigParam("Ch1A:GainAdj",    '2', 0x48, 12, 0, 0);     // Chan1 A gain adjust
    createConfigParam("Ch2A:GainAdj",    '2', 0x49, 12, 0, 0);     // Chan2 A gain adjust
    createConfigParam("Ch3A:GainAdj",    '2', 0x4A, 12, 0, 0);     // Chan3 A gain adjust
    createConfigParam("Ch4A:GainAdj",    '2', 0x4B, 12, 0, 0);     // Chan4 A gain adjust
    createConfigParam("Ch5A:GainAdj",    '2', 0x4C, 12, 0, 0);     // Chan5 A gain adjust
    createConfigParam("Ch6A:GainAdj",    '2', 0x4D, 12, 0, 0);     // Chan6 A gain adjust
    createConfigParam("Ch7A:GainAdj",    '2', 0x4E, 12, 0, 0);     // Chan7 A gain adjust
    createConfigParam("Ch8A:GainAdj",    '2', 0x4F, 12, 0, 0);     // Chan8 A gain adjust

    createConfigParam("Ch1B:GainAdj",    '2', 0x50, 12, 0, 0);     // Chan1 B gain adjust
    createConfigParam("Ch2B:GainAdj",    '2', 0x51, 12, 0, 0);     // Chan2 B gain adjust
    createConfigParam("Ch3B:GainAdj",    '2', 0x52, 12, 0, 0);     // Chan3 B gain adjust
    createConfigParam("Ch4B:GainAdj",    '2', 0x53, 12, 0, 0);     // Chan4 B gain adjust
    createConfigParam("Ch5B:GainAdj",    '2', 0x54, 12, 0, 0);     // Chan5 B gain adjust
    createConfigParam("Ch6B:GainAdj",    '2', 0x55, 12, 0, 0);     // Chan6 B gain adjust
    createConfigParam("Ch7B:GainAdj",    '2', 0x56, 12, 0, 0);     // Chan7 B gain adjust
    createConfigParam("Ch8B:GainAdj",    '2', 0x57, 12, 0, 0);     // Chan8 B gain adjust

    createConfigParam("Ch1A:Scale",      '3', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch2A:Scale",      '3', 0x1,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch3A:Scale",      '3', 0x2,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch4A:Scale",      '3', 0x3,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch5A:Scale",      '3', 0x4,  12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch6A:Scale",      '3', 0x5,  12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch7A:Scale",      '3', 0x6,  12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch8A:Scale",      '3', 0x7,  12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch1B:Scale",      '3', 0x8,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch2B:Scale",      '3', 0x9,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch3B:Scale",      '3', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch4B:Scale",      '3', 0xB,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch5B:Scale",      '3', 0xC,  12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch6B:Scale",      '3', 0xD,  12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch7B:Scale",      '3', 0xE,  12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch8B:Scale",      '3', 0xF,  12, 0, 2048);  // Chan8 B scale

    createConfigParam("Ch1A:Offset",     '3', 0x10, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch2A:Offset",     '3', 0x11, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch3A:Offset",     '3', 0x12, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch4A:Offset",     '3', 0x13, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch5A:Offset",     '3', 0x14, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch6A:Offset",     '3', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch7A:Offset",     '3', 0x16, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch8A:Offset",     '3', 0x17, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch1B:Offset",     '3', 0x18, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2B:Offset",     '3', 0x19, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3B:Offset",     '3', 0x1A, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4B:Offset",     '3', 0x1B, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5B:Offset",     '3', 0x1C, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6B:Offset",     '3', 0x1D, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7B:Offset",     '3', 0x1E, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8B:Offset",     '3', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFactor", '4', 0x0,  12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFactor", '4', 0x1,  12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFactor", '4', 0x2,  12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFactor", '4', 0x3,  12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFactor", '4', 0x4,  12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFactor", '4', 0x5,  12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFactor", '4', 0x6,  12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFactor", '4', 0x7,  12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1A:Baseline",   'C', 0x0,  12, 0, 100);   // Chan1 A baseline value
    createConfigParam("Ch2A:Baseline",   'C', 0x1,  12, 0, 100);   // Chan2 A baseline value
    createConfigParam("Ch3A:Baseline",   'C', 0x2,  12, 0, 100);   // Chan3 A baseline value
    createConfigParam("Ch4A:Baseline",   'C', 0x3,  12, 0, 100);   // Chan4 A baseline value
    createConfigParam("Ch5A:Baseline",   'C', 0x4,  12, 0, 100);   // Chan5 A baseline value
    createConfigParam("Ch6A:Baseline",   'C', 0x5,  12, 0, 100);   // Chan6 A baseline value
    createConfigParam("Ch7A:Baseline",   'C', 0x6,  12, 0, 100);   // Chan7 A baseline value
    createConfigParam("Ch8A:Baseline",   'C', 0x7,  12, 0, 100);   // Chan8 A baseline value
    createConfigParam("Ch1B:Baseline",   'C', 0x8,  12, 0, 100);   // Chan1 B baseline value
    createConfigParam("Ch2B:Baseline",   'C', 0x9,  12, 0, 100);   // Chan2 B baseline value
    createConfigParam("Ch3B:Baseline",   'C', 0xA,  12, 0, 100);   // Chan3 B baseline value
    createConfigParam("Ch4B:Baseline",   'C', 0xB,  12, 0, 100);   // Chan4 B baseline value
    createConfigParam("Ch5B:Baseline",   'C', 0xC,  12, 0, 100);   // Chan5 B baseline value
    createConfigParam("Ch6B:Baseline",   'C', 0xD,  12, 0, 100);   // Chan6 B baseline value
    createConfigParam("Ch7B:Baseline",   'C', 0xE,  12, 0, 100);   // Chan7 B baseline value
    createConfigParam("Ch8B:Baseline",   'C', 0xF,  12, 0, 100);   // Chan8 B baseline value

    // There's an unused part of C section in v5.4 C:0x10 - C:0x1F
    createConfigParam("ConfigFiller",  'C', 0x1F, 16, 0, 0);     // Config filler

    createConfigParam("Ch1A:SampleMin",  'D', 0x0,  12, 0, 100);   // Chan1 A sample minimum
    createConfigParam("Ch2A:SampleMin",  'D', 0x1,  12, 0, 100);   // Chan2 A sample minimum
    createConfigParam("Ch3A:SampleMin",  'D', 0x2,  12, 0, 100);   // Chan3 A sample minimum
    createConfigParam("Ch4A:SampleMin",  'D', 0x3,  12, 0, 100);   // Chan4 A sample minimum
    createConfigParam("Ch5A:SampleMin",  'D', 0x4,  12, 0, 100);   // Chan5 A sample minimum
    createConfigParam("Ch6A:SampleMin",  'D', 0x5,  12, 0, 100);   // Chan6 A sample minimum
    createConfigParam("Ch7A:SampleMin",  'D', 0x6,  12, 0, 100);   // Chan7 A sample minimum
    createConfigParam("Ch8A:SampleMin",  'D', 0x7,  12, 0, 100);   // Chan8 A sample minimum
    createConfigParam("Ch1B:SampleMin",  'D', 0x8,  12, 0, 100);   // Chan1 B sample minimum
    createConfigParam("Ch2B:SampleMin",  'D', 0x9,  12, 0, 100);   // Chan2 B sample minimum
    createConfigParam("Ch3B:SampleMin",  'D', 0xA,  12, 0, 100);   // Chan3 B sample minimum
    createConfigParam("Ch4B:SampleMin",  'D', 0xB,  12, 0, 100);   // Chan4 B sample minimum
    createConfigParam("Ch5B:SampleMin",  'D', 0xC,  12, 0, 100);   // Chan5 B sample minimum
    createConfigParam("Ch6B:SampleMin",  'D', 0xD,  12, 0, 100);   // Chan6 B sample minimum
    createConfigParam("Ch7B:SampleMin",  'D', 0xE,  12, 0, 100);   // Chan7 B sample minimum
    createConfigParam("Ch8B:SampleMin",  'D', 0xF,  12, 0, 100);   // Chan8 B sample minimum

    createConfigParam("Ch1A:SampleMax",  'D', 0x10, 12, 0, 1000);  // Chan1 A sample maximum
    createConfigParam("Ch2A:SampleMax",  'D', 0x11, 12, 0, 1000);  // Chan2 A sample maximum
    createConfigParam("Ch3A:SampleMax",  'D', 0x12, 12, 0, 1000);  // Chan3 A sample maximum
    createConfigParam("Ch4A:SampleMax",  'D', 0x13, 12, 0, 1000);  // Chan4 A sample maximum
    createConfigParam("Ch5A:SampleMax",  'D', 0x14, 12, 0, 1000);  // Chan5 A sample maximum
    createConfigParam("Ch6A:SampleMax",  'D', 0x15, 12, 0, 1000);  // Chan6 A sample maximum
    createConfigParam("Ch7A:SampleMax",  'D', 0x16, 12, 0, 1000);  // Chan7 A sample maximum
    createConfigParam("Ch8A:SampleMax",  'D', 0x17, 12, 0, 1000);  // Chan8 A sample maximum
    createConfigParam("Ch1B:SampleMax",  'D', 0x18, 12, 0, 1000);  // Chan1 B sample maximum
    createConfigParam("Ch2B:SampleMax",  'D', 0x19, 12, 0, 1000);  // Chan2 B sample maximum
    createConfigParam("Ch3B:SampleMax",  'D', 0x1A, 12, 0, 1000);  // Chan3 B sample maximum
    createConfigParam("Ch4B:SampleMax",  'D', 0x1B, 12, 0, 1000);  // Chan4 B sample maximum
    createConfigParam("Ch5B:SampleMax",  'D', 0x1C, 12, 0, 1000);  // Chan5 B sample maximum
    createConfigParam("Ch6B:SampleMax",  'D', 0x1D, 12, 0, 1000);  // Chan6 B sample maximum
    createConfigParam("Ch7B:SampleMax",  'D', 0x1E, 12, 0, 1000);  // Chan7 B sample maximum
    createConfigParam("Ch8B:SampleMax",  'D', 0x1F, 12, 0, 1000);  // Chan8 B sample maximum

    createConfigParam("Ch8B:SlopeMax",   'D', 0x20, 12, 0, 0);     // Maximum slope

    createConfigParam("Ch1:Enable",      'E', 0x0,  1,  0, 1);     // Chan1 enable                  (0=disable,1=enable)
    createConfigParam("Ch2:Enable",      'E', 0x0,  1,  1, 1);     // Chan2 enable                  (0=disable,1=enable)
    createConfigParam("Ch3:Enable",      'E', 0x0,  1,  2, 1);     // Chan3 enable                  (0=disable,1=enable)
    createConfigParam("Ch4:Enable",      'E', 0x0,  1,  3, 1);     // Chan4 enable                  (0=disable,1=enable)
    createConfigParam("Ch5:Enable",      'E', 0x0,  1,  4, 1);     // Chan5 enable                  (0=disable,1=enable)
    createConfigParam("Ch6:Enable",      'E', 0x0,  1,  5, 1);     // Chan6 enable                  (0=disable,1=enable)
    createConfigParam("Ch7:Enable",      'E', 0x0,  1,  6, 1);     // Chan7 enable                  (0=disable,1=enable)
    createConfigParam("Ch8:Enable",      'E', 0x0,  1,  7, 1);     // Chan8 enable                  (0=disable,1=enable)

    createConfigParam("Ch1:VerboseEn",   'E', 0x0,  1,  8, 0);     // Chan1 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch2:VerboseEn",   'E', 0x0,  1,  9, 0);     // Chan2 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch3:VerboseEn",   'E', 0x0,  1, 10, 0);     // Chan3 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch4:VerboseEn",   'E', 0x0,  1, 11, 0);     // Chan4 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch5:VerboseEn",   'E', 0x0,  1, 12, 0);     // Chan5 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch6:VerboseEn",   'E', 0x0,  1, 13, 0);     // Chan6 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch7:VerboseEn",   'E', 0x0,  1, 14, 0);     // Chan7 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch8:VerboseEn",   'E', 0x0,  1, 15, 0);     // Chan8 verbose enable          (0=disable,1=enable)

    createConfigParam("TimeVetoLow",     'E', 0x1,  32, 0, 0);     // Timestamp veto low
    createConfigParam("TimeVetoHigh",    'E', 0x3,  32, 0, 0);     // Timestamp veto high
    createConfigParam("FakeTrigDelay",   'E', 0x5,  16, 0, 20000); // Fake trigger delay
    createConfigParam("MinAdc",          'E', 0x6,  16, 0, 100);   // Minimum ADC
    createConfigParam("MaxAdc",          'E', 0x7,  16, 0, 1000);  // Maximum ADC
    createConfigParam("IntRelease",      'E', 0x8,  16, 0, -6);    // Integrator release point
    createConfigParam("MinDiscrimWidth", 'E', 0x9,  12, 0, 1);     // Minimum discriminator width
    createConfigParam("Sample1",         'E', 0xA,  4,  0, 2);     // Sample 1 point [0:10]
    createConfigParam("Sample2",         'E', 0xB,  6,  0, 14);    // Sample 1 point [2:50]
    createConfigParam("TsyncDelay",      'E', 0xC,  32, 0, 0);     // TSYNC delay

    createConfigParam("LvdsRate",        'F', 0x0,  1, 15, 1);     // LVDS output rate              (0=20Mhz,1=40Mhz)
    createConfigParam("AutoCorrSam",     'F', 0x0,  1, 14, 0);     // Auto correction sample mode   (0=TSYNC,1=fake trigger)
    createConfigParam("EdgeDetecEn",     'F', 0x0,  1, 13, 1);     // Edge detect enable            (0=disable,1=enable)
    createConfigParam("MastDiscSel",     'F', 0x0,  2, 11, 0);     // Master discriminator select   (0=SUM discr,1=A discr,2=B discr,3=all)
    createConfigParam("EnableAcq",       'F', 0x0,  1, 10, 1);     // ROC enable acquisition        (0=disable,1=enable)
    createConfigParam("AutoCorrEn",      'F', 0x0,  1, 9,  0);     // Auto correction mode          (0=enabled,1=disabled)
    createConfigParam("HighResMode",     'F', 0x0,  1, 8,  1);     // High resolution mode          (0=low res 0-127,1=high res 0-255)
    createConfigParam("OutputMode",      'F', 0x0,  2, 6,  0);     // Output mode                   (0=normal,1=raw,2=extended)
    createConfigParam("AcqMode",         'F', 0x0,  2, 4,  0);     // Output mode                   (0=normal,1=verbose,2=fake trigger)
    createConfigParam("TxEnable",        'F', 0x0,  1, 3,  1);     // TX enable                     (0=external,1=always enabled)
    createConfigParam("TsyncSel",        'F', 0x0,  1, 2,  0);     // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TclkSel",         'F', 0x0,  1, 1,  0);     // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("Reset",           'F', 0x0,  1, 0,  0);     // Reset enable                  (0=disable,1=enable)
}
