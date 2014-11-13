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
//    BLXXX:Det:RocXXX:| sig nam|                     | EPICS record description  | (bi and mbbi description)
    createStatusParam("ErrParity",   0x0,  1, 10); // LVDS parity error.            (0=no error,1=error)
    createStatusParam("Configured",  0x0,  1,  9); // Not configured                (0=configured,1=not configured)
    createStatusParam("Discovered",  0x0,  1,  8); // Configured                    (0=discovered,1=not discovered)
    createStatusParam("SysrstBHi",   0x0,  1,  7); // SYSRST_B Got HIGH             (0=no,1=yes)
    createStatusParam("SysrstBLo",   0x0,  1,  6); // SYSRST_B Got LOW              (0=no,1=yes)
    createStatusParam("TxenBHigh",   0x0,  1,  5); // TXEN_B Got HIGH               (0=no,1=yes)
    createStatusParam("TxenBLow",    0x0,  1,  4); // TXEN_B Got LOW                (0=no,1=yes)
    createStatusParam("TsyncHigh",   0x0,  1,  3); // TSYNC Got HIGH                (0=no,1=yes)
    createStatusParam("TsyncLow",    0x0,  1,  2); // TSYNC Got LOW                 (0=no,1=yes)
    createStatusParam("TclkHigh",    0x0,  1,  1); // TCLK Got HIGH                 (0=no,1=yes)
    createStatusParam("TclkLow",     0x0,  1,  0); // TCLK Got LOW                  (0=no,1=yes)

    createStatusParam("HVStatus",    0x1,  1,  1); // High Voltage Status bit       (0=not present,1=present)
    createStatusParam("AutoCorr",    0x1,  1,  0); // Auto correction enabled       (0=not enabled,1=enabled)

    createStatusParam("Ch2:Event",   0x2,  1, 13); // Chan2 got event               (0=no,1=yes)
    createStatusParam("Ch2:DiscHi",  0x2,  1, 12); // Chan2 disc high               (0=low,1=high)
    createStatusParam("Ch2:FifFul",  0x2,  1, 11); // Chan2 FIFO full               (0=not full,1=full)
    createStatusParam("Ch2:CalcEr",  0x2,  1, 10); // Chan2 calc out of range       (0=no,1=yes)
    createStatusParam("Ch2:DenNeg",  0x2,  1,  9); // Chan2 negative denom          (0=no,1=yes)
    createStatusParam("Ch2:CalcOv",  0x2,  1,  8); // Chan2 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch1:GotEv",   0x2,  1,  5); // Chan1 got event               (0=no,1=yes)
    createStatusParam("Ch1:DiscHi",  0x2,  1,  4); // Chan1 disc high               (0=low,1=high)
    createStatusParam("Ch1:FifFul",  0x2,  1,  3); // Chan1 FIFO full               (0=not full,1=full)
    createStatusParam("Ch1:CalcEr",  0x2,  1,  2); // Chan1 calc out of range       (0=no,1=yes)
    createStatusParam("Ch1:DenNeg",  0x2,  1,  1); // Chan1 negative denom          (0=no,1=yes)
    createStatusParam("Ch1:CalcOv",  0x2,  1,  0); // Chan1 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch4:GotEv",   0x3,  1, 13); // Chan4 got event               (0=no,1=yes)
    createStatusParam("Ch4:DiscHi",  0x3,  1, 12); // Chan4 disc high               (0=low,1=high)
    createStatusParam("Ch4:FifFul",  0x3,  1, 11); // Chan4 FIFO full               (0=not full,1=full)
    createStatusParam("Ch4:CalcEr",  0x3,  1, 10); // Chan4 calc out of range       (0=no,1=yes)
    createStatusParam("Ch4:DenNeg",  0x3,  1,  9); // Chan4 negative denom          (0=no,1=yes)
    createStatusParam("Ch4:CalcOv",  0x3,  1,  8); // Chan4 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch3:GotEv",   0x3,  1,  5); // Chan3 got event               (0=no,1=yes)
    createStatusParam("Ch3:DiscHi",  0x3,  1,  4); // Chan3 disc high               (0=low,1=high)
    createStatusParam("Ch3:FifFul",  0x3,  1,  3); // Chan3 FIFO full               (0=not full,1=full)
    createStatusParam("Ch3:CalcEr",  0x3,  1,  2); // Chan3 calc out of range       (0=no,1=yes)
    createStatusParam("Ch3:DenNeg",  0x3,  1,  1); // Chan3 negative denom          (0=no,1=yes)
    createStatusParam("Ch3:CalcOv",  0x3,  1,  0); // Chan3 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch6:GotEv",   0x4,  1, 13); // Chan6 got event               (0=no,1=yes)
    createStatusParam("Ch6:DiscHi",  0x4,  1, 12); // Chan6 disc high               (0=low,1=high)
    createStatusParam("Ch6:FifFul",  0x4,  1, 11); // Chan6 FIFO full               (0=not full,1=full)
    createStatusParam("Ch6:CalcEr",  0x4,  1, 10); // Chan6 calc out of range       (0=no,1=yes)
    createStatusParam("Ch6:DenNeg",  0x4,  1,  9); // Chan6 negative denom          (0=no,1=yes)
    createStatusParam("Ch6:CalcOv",  0x4,  1,  8); // Chan6 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch5:GotEv",   0x4,  1,  5); // Chan5 got event               (0=no,1=yes)
    createStatusParam("Ch5:DiscHi",  0x4,  1,  4); // Chan5 disc high               (0=low,1=high)
    createStatusParam("Ch5:FifFul",  0x4,  1,  3); // Chan5 FIFO full               (0=not full,1=full)
    createStatusParam("Ch5:CalcEr",  0x4,  1,  2); // Chan5 calc out of range       (0=no,1=yes)
    createStatusParam("Ch5:DenNeg",  0x4,  1,  1); // Chan5 negative denom          (0=no,1=yes)
    createStatusParam("Ch5:CalcOv",  0x4,  1,  0); // Chan5 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch8:GotEv",   0x5,  1, 13); // Chan8 got event               (0=no,1=yes)
    createStatusParam("Ch8:DiscHi",  0x5,  1, 12); // Chan8 disc high               (0=low,1=high)
    createStatusParam("Ch8:FifFul",  0x5,  1, 11); // Chan8 FIFO full               (0=not full,1=full)
    createStatusParam("Ch8:CalcEr",  0x5,  1, 10); // Chan8 calc out of range       (0=no,1=yes)
    createStatusParam("Ch8:DenNeg",  0x5,  1,  9); // Chan8 negative denom          (0=no,1=yes)
    createStatusParam("Ch8:CalcOv",  0x5,  1,  8); // Chan8 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("Ch7:GotEv",   0x5,  1,  5); // Chan7 got event               (0=no,1=yes)
    createStatusParam("Ch7:DiscHi",  0x5,  1,  4); // Chan7 disc high               (0=low,1=high)
    createStatusParam("Ch7:FifFul",  0x5,  1,  3); // Chan7 FIFO full               (0=not full,1=full)
    createStatusParam("Ch7:CalcEr",  0x5,  1,  2); // Chan7 calc out of range       (0=no,1=yes)
    createStatusParam("Ch7:DenNeg",  0x5,  1,  1); // Chan7 negative denom          (0=no,1=yes)
    createStatusParam("Ch7:CalcOv",  0x5,  1,  0); // Chan7 calc overflow           (0=no overflow,1=overflow)

    createStatusParam("A1:BaseAvg",  0x6, 15,  0); // A1 baseline average
    createStatusParam("B1:BaseAvg",  0x7, 15,  0); // B1 baseline average
    createStatusParam("A2:BaseAvg",  0x8, 15,  0); // A2 baseline average
    createStatusParam("B2:BaseAvg",  0x9, 15,  0); // B2 baseline average
    createStatusParam("A3:BaseAvg",  0xA, 15,  0); // A3 baseline average
    createStatusParam("B3:BaseAvg",  0xB, 15,  0); // B3 baseline average
    createStatusParam("A4:BaseAvg",  0xC, 15,  0); // A4 baseline average
    createStatusParam("B4:BaseAvg",  0xD, 15,  0); // B4 baseline average
    createStatusParam("A5:BaseAvg",  0xE, 15,  0); // A5 baseline average
    createStatusParam("B5:BaseAvg",  0xF, 15,  0); // B5 baseline average
    createStatusParam("A6:BaseAvg", 0x10, 15,  0); // A6 baseline average
    createStatusParam("B6:BaseAvg", 0x11, 15,  0); // B6 baseline average
    createStatusParam("A7:BaseAvg", 0x12, 15,  0); // A7 baseline average
    createStatusParam("B7:BaseAvg", 0x13, 15,  0); // B7 baseline average
    createStatusParam("A8:BaseAvg", 0x14, 15,  0); // A8 baseline average
    createStatusParam("B8:BaseAvg", 0x15, 15,  0); // B8 baseline average

    // ROC lies about the length of the packet. It returns 44 bytes of valid
    // data but the packet length is 64 bytes => 20 bytes or 10 registers
    // We only need to define the last one which defines the packet length
    createStatusParam("StatFill",   0x1F,15,  0);
}

void RocPlugin::createConfigParams_v54()
{
//    BLXXX:Det:RocXXX:| sig nam|                                  | EPICS record description | (bi and mbbi description)
    createConfigParam("Ch1:PosIdx",   '1', 0x0,  32, 0, 0);     // Chan1 position index
    createConfigParam("Ch2:PosIdx",   '1', 0x2,  32, 0, 256);   // Chan2 position index
    createConfigParam("Ch3:PosIdx",   '1', 0x4,  32, 0, 512);   // Chan3 position index
    createConfigParam("Ch4:PosIdx",   '1', 0x6,  32, 0, 768);   // Chan4 position index
    createConfigParam("Ch5:PosIdx",   '1', 0x8,  32, 0, 1024);  // Chan5 position index
    createConfigParam("Ch6:PosIdx",   '1', 0xA,  32, 0, 1280);  // Chan6 position index
    createConfigParam("Ch7:PosIdx",   '1', 0xC,  32, 0, 1536);  // Chan7 position index
    createConfigParam("Ch8:PosIdx",   '1', 0xE,  32, 0, 1792);  // Chan8 position index

    createConfigParam("Ch1:A:InOf",   '2', 0x0,  12, 0, 100);   // Chan1 A input offset
    createConfigParam("Ch2:A:InOf",   '2', 0x1,  12, 0, 100);   // Chan2 A input offset
    createConfigParam("Ch3:A:InOf",   '2', 0x2,  12, 0, 100);   // Chan3 A input offset
    createConfigParam("Ch4:A:InOf",   '2', 0x3,  12, 0, 100);   // Chan4 A input offset
    createConfigParam("Ch5:A:InOf",   '2', 0x4,  12, 0, 100);   // Chan5 A input offset
    createConfigParam("Ch6:A:InOf",   '2', 0x5,  12, 0, 100);   // Chan6 A input offset
    createConfigParam("Ch7:A:InOf",   '2', 0x6,  12, 0, 100);   // Chan7 A input offset
    createConfigParam("Ch8:A:InOf",   '2', 0x7,  12, 0, 100);   // Chan8 A input offset
    createConfigParam("Ch1:B:InOf",   '2', 0x8,  12, 0, 100);   // Chan1 B input offset
    createConfigParam("Ch2:B:InOf",   '2', 0x9,  12, 0, 100);   // Chan2 B input offset
    createConfigParam("Ch3:B:InOf",   '2', 0xA,  12, 0, 100);   // Chan3 B input offset
    createConfigParam("Ch4:B:InOf",   '2', 0xB,  12, 0, 100);   // Chan4 B input offset
    createConfigParam("Ch5:B:InOf",   '2', 0xC,  12, 0, 100);   // Chan5 B input offset
    createConfigParam("Ch6:B:InOf",   '2', 0xD,  12, 0, 100);   // Chan6 B input offset
    createConfigParam("Ch7:B:InOf",   '2', 0xE,  12, 0, 100);   // Chan7 B input offset
    createConfigParam("Ch8:B:InOf",   '2', 0xF,  12, 0, 100);   // Chan8 B input offset

    createConfigParam("Ch1:A:AdcO",   '2', 0x10, 12, 0, 100);   // Chan1 A ADC offset
    createConfigParam("Ch2:A:AdcO",   '2', 0x11, 12, 0, 100);   // Chan2 A ADC offset
    createConfigParam("Ch3:A:AdcO",   '2', 0x12, 12, 0, 100);   // Chan3 A ADC offset
    createConfigParam("Ch4:A:AdcO",   '2', 0x13, 12, 0, 100);   // Chan4 A ADC offset
    createConfigParam("Ch5:A:AdcO",   '2', 0x14, 12, 0, 100);   // Chan5 A ADC offset
    createConfigParam("Ch6:A:AdcO",   '2', 0x15, 12, 0, 100);   // Chan6 A ADC offset
    createConfigParam("Ch7:A:AdcO",   '2', 0x16, 12, 0, 100);   // Chan7 A ADC offset
    createConfigParam("Ch8:A:AdcO",   '2', 0x17, 12, 0, 100);   // Chan8 A ADC offset
    createConfigParam("Ch1:B:AdcO",   '2', 0x18, 12, 0, 100);   // Chan1 B ADC offset
    createConfigParam("Ch2:B:AdcO",   '2', 0x19, 12, 0, 100);   // Chan2 B ADC offset
    createConfigParam("Ch3:B:AdcO",   '2', 0x1A, 12, 0, 100);   // Chan3 B ADC offset
    createConfigParam("Ch4:B:AdcO",   '2', 0x1B, 12, 0, 100);   // Chan4 B ADC offset
    createConfigParam("Ch5:B:AdcO",   '2', 0x1C, 12, 0, 100);   // Chan5 B ADC offset
    createConfigParam("Ch6:B:AdcO",   '2', 0x1D, 12, 0, 100);   // Chan6 B ADC offset
    createConfigParam("Ch7:B:AdcO",   '2', 0x1E, 12, 0, 100);   // Chan7 B ADC offset
    createConfigParam("Ch8:B:AdcO",   '2', 0x1F, 12, 0, 100);   // Chan8 B ADC offset

    createConfigParam("Ch1:A:Thrs",   '2', 0x20, 12, 0, 400);   // Chan1 A threshold
    createConfigParam("Ch2:A:Thrs",   '2', 0x21, 12, 0, 400);   // Chan2 A threshold
    createConfigParam("Ch3:A:Thrs",   '2', 0x22, 12, 0, 400);   // Chan3 A threshold
    createConfigParam("Ch4:A:Thrs",   '2', 0x23, 12, 0, 400);   // Chan4 A threshold
    createConfigParam("Ch5:A:Thrs",   '2', 0x24, 12, 0, 400);   // Chan5 A threshold
    createConfigParam("Ch6:A:Thrs",   '2', 0x25, 12, 0, 400);   // Chan6 A threshold
    createConfigParam("Ch7:A:Thrs",   '2', 0x26, 12, 0, 400);   // Chan7 A threshold
    createConfigParam("Ch8:A:Thrs",   '2', 0x27, 12, 0, 400);   // Chan8 A threshold
    createConfigParam("Ch1:B:Thrs",   '2', 0x28, 12, 0, 400);   // Chan1 B threshold
    createConfigParam("Ch2:B:Thrs",   '2', 0x29, 12, 0, 400);   // Chan2 B threshold
    createConfigParam("Ch3:B:Thrs",   '2', 0x2A, 12, 0, 400);   // Chan3 B threshold
    createConfigParam("Ch4:B:Thrs",   '2', 0x2B, 12, 0, 400);   // Chan4 B threshold
    createConfigParam("Ch5:B:Thrs",   '2', 0x2C, 12, 0, 400);   // Chan5 B threshold
    createConfigParam("Ch6:B:Thrs",   '2', 0x2D, 12, 0, 400);   // Chan6 B threshold
    createConfigParam("Ch7:B:Thrs",   '2', 0x2E, 12, 0, 400);   // Chan7 B threshold
    createConfigParam("Ch8:B:Thrs",   '2', 0x2F, 12, 0, 400);   // Chan8 B threshold

    createConfigParam("Ch1:PosThr",   '2', 0x30, 12, 0, 400);   // Chan1 positive threshold
    createConfigParam("Ch2:PosThr",   '2', 0x31, 12, 0, 400);   // Chan2 positive threshold
    createConfigParam("Ch3:PosThr",   '2', 0x32, 12, 0, 400);   // Chan3 positive threshold
    createConfigParam("Ch4:PosThr",   '2', 0x33, 12, 0, 400);   // Chan4 positive threshold
    createConfigParam("Ch5:PosThr",   '2', 0x34, 12, 0, 400);   // Chan5 positive threshold
    createConfigParam("Ch6:PosThr",   '2', 0x35, 12, 0, 400);   // Chan6 positive threshold
    createConfigParam("Ch7:PosThr",   '2', 0x36, 12, 0, 400);   // Chan7 positive threshold
    createConfigParam("Ch8:PosThr",   '2', 0x37, 12, 0, 400);   // Chan8 positive threshold
    createConfigParam("Ch1:A:FSca",   '2', 0x38, 12, 0, 10);    // Chan1 A full scale
    createConfigParam("Ch2:A:FSca",   '2', 0x39, 12, 0, 10);    // Chan2 A full scale
    createConfigParam("Ch3:A:FSca",   '2', 0x3A, 12, 0, 10);    // Chan3 A full scale
    createConfigParam("Ch4:A:FSca",   '2', 0x3B, 12, 0, 10);    // Chan4 A full scale
    createConfigParam("Ch5:A:FSca",   '2', 0x3C, 12, 0, 10);    // Chan5 A full scale
    createConfigParam("Ch6:A:FSca",   '2', 0x3D, 12, 0, 10);    // Chan6 A full scale
    createConfigParam("Ch7:A:FSca",   '2', 0x3E, 12, 0, 10);    // Chan7 A full scale
    createConfigParam("Ch8:A:FSca",   '2', 0x3F, 12, 0, 10);    // Chan8 A full scale

    createConfigParam("Ch1:B:FSca",   '2', 0x40, 12, 0, 10);    // Chan1 B full scale
    createConfigParam("Ch2:B:FSca",   '2', 0x41, 12, 0, 10);    // Chan2 B full scale
    createConfigParam("Ch3:B:FSca",   '2', 0x42, 12, 0, 10);    // Chan3 B full scale
    createConfigParam("Ch4:B:FSca",   '2', 0x43, 12, 0, 10);    // Chan4 B full scale
    createConfigParam("Ch5:B:FSca",   '2', 0x44, 12, 0, 10);    // Chan5 B full scale
    createConfigParam("Ch6:B:FSca",   '2', 0x45, 12, 0, 10);    // Chan6 B full scale
    createConfigParam("Ch7:B:FSca",   '2', 0x46, 12, 0, 10);    // Chan7 B full scale
    createConfigParam("Ch8:B:FSca",   '2', 0x47, 12, 0, 10);    // Chan8 B full scale
    createConfigParam("Ch1:A:GAdj",   '2', 0x48, 12, 0, 0);     // Chan1 A gain adjust
    createConfigParam("Ch2:A:GAdj",   '2', 0x49, 12, 0, 0);     // Chan2 A gain adjust
    createConfigParam("Ch3:A:GAdj",   '2', 0x4A, 12, 0, 0);     // Chan3 A gain adjust
    createConfigParam("Ch4:A:GAdj",   '2', 0x4B, 12, 0, 0);     // Chan4 A gain adjust
    createConfigParam("Ch5:A:GAdj",   '2', 0x4C, 12, 0, 0);     // Chan5 A gain adjust
    createConfigParam("Ch6:A:GAdj",   '2', 0x4D, 12, 0, 0);     // Chan6 A gain adjust
    createConfigParam("Ch7:A:GAdj",   '2', 0x4E, 12, 0, 0);     // Chan7 A gain adjust
    createConfigParam("Ch8:A:GAdj",   '2', 0x4F, 12, 0, 0);     // Chan8 A gain adjust

    createConfigParam("Ch1:B:GAdj",   '2', 0x50, 12, 0, 0);     // Chan1 B gain adjust
    createConfigParam("Ch2:B:GAdj",   '2', 0x51, 12, 0, 0);     // Chan2 B gain adjust
    createConfigParam("Ch3:B:GAdj",   '2', 0x52, 12, 0, 0);     // Chan3 B gain adjust
    createConfigParam("Ch4:B:GAdj",   '2', 0x53, 12, 0, 0);     // Chan4 B gain adjust
    createConfigParam("Ch5:B:GAdj",   '2', 0x54, 12, 0, 0);     // Chan5 B gain adjust
    createConfigParam("Ch6:B:GAdj",   '2', 0x55, 12, 0, 0);     // Chan6 B gain adjust
    createConfigParam("Ch7:B:GAdj",   '2', 0x56, 12, 0, 0);     // Chan7 B gain adjust
    createConfigParam("Ch8:B:GAdj",   '2', 0x57, 12, 0, 0);     // Chan8 B gain adjust

    createConfigParam("Ch1:A:Scal",   '3', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch2:A:Scal",   '3', 0x1,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch3:A:Scal",   '3', 0x2,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch4:A:Scal",   '3', 0x3,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch5:A:Scal",   '3', 0x4,  12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch6:A:Scal",   '3', 0x5,  12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch7:A:Scal",   '3', 0x6,  12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch8:A:Scal",   '3', 0x7,  12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch1:B:Scal",   '3', 0x8,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch2:B:Scal",   '3', 0x9,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch3:B:Scal",   '3', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch4:B:Scal",   '3', 0xB,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch5:B:Scal",   '3', 0xC,  12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch6:B:Scal",   '3', 0xD,  12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch7:B:Scal",   '3', 0xE,  12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch8:B:Scal",   '3', 0xF,  12, 0, 2048);  // Chan8 B scale

    createConfigParam("Ch1:A:Offs",   '3', 0x10, 12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch2:A:Offs",   '3', 0x11, 12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch3:A:Offs",   '3', 0x12, 12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch4:A:Offs",   '3', 0x13, 12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch5:A:Offs",   '3', 0x14, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch6:A:Offs",   '3', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch7:A:Offs",   '3', 0x16, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch8:A:Offs",   '3', 0x17, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch1:B:Offs",   '3', 0x18, 12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:B:Offs",   '3', 0x19, 12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:B:Offs",   '3', 0x1A, 12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:B:Offs",   '3', 0x1B, 12, 0, 1600);  // Chan4 B offset
    createConfigParam("Ch5:B:Offs",   '3', 0x1C, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:B:Offs",   '3', 0x1D, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:B:Offs",   '3', 0x1E, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:B:Offs",   '3', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleF",   '4', 0x0,  12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleF",   '4', 0x1,  12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleF",   '4', 0x2,  12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleF",   '4', 0x3,  12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleF",   '4', 0x4,  12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleF",   '4', 0x5,  12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleF",   '4', 0x6,  12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleF",   '4', 0x7,  12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1:A:BasV",   'C', 0x0,  12, 0, 100);   // Chan1 A baseline value
    createConfigParam("Ch2:A:BasV",   'C', 0x1,  12, 0, 100);   // Chan2 A baseline value
    createConfigParam("Ch3:A:BasV",   'C', 0x2,  12, 0, 100);   // Chan3 A baseline value
    createConfigParam("Ch4:A:BasV",   'C', 0x3,  12, 0, 100);   // Chan4 A baseline value
    createConfigParam("Ch5:A:BasV",   'C', 0x4,  12, 0, 100);   // Chan5 A baseline value
    createConfigParam("Ch6:A:BasV",   'C', 0x5,  12, 0, 100);   // Chan6 A baseline value
    createConfigParam("Ch7:A:BasV",   'C', 0x6,  12, 0, 100);   // Chan7 A baseline value
    createConfigParam("Ch8:A:BasV",   'C', 0x7,  12, 0, 100);   // Chan8 A baseline value
    createConfigParam("Ch1:B:BasV",   'C', 0x8,  12, 0, 100);   // Chan1 B baseline value
    createConfigParam("Ch2:B:BasV",   'C', 0x9,  12, 0, 100);   // Chan2 B baseline value
    createConfigParam("Ch3:B:BasV",   'C', 0xA,  12, 0, 100);   // Chan3 B baseline value
    createConfigParam("Ch4:B:BasV",   'C', 0xB,  12, 0, 100);   // Chan4 B baseline value
    createConfigParam("Ch5:B:BasV",   'C', 0xC,  12, 0, 100);   // Chan5 B baseline value
    createConfigParam("Ch6:B:BasV",   'C', 0xD,  12, 0, 100);   // Chan6 B baseline value
    createConfigParam("Ch7:B:BasV",   'C', 0xE,  12, 0, 100);   // Chan7 B baseline value
    createConfigParam("Ch8:B:BasV",   'C', 0xF,  12, 0, 100);   // Chan8 B baseline value

    // There's an unused part of C section in v5.4 C:0x10 - C:0x1F
    createConfigParam("ConfigFill",   'C', 0x1F, 16, 0, 0);     // Config filler

    createConfigParam("Ch1:A:SaMi",   'D', 0x0,  12, 0, 100);   // Chan1 A sample minimum
    createConfigParam("Ch2:A:SaMi",   'D', 0x1,  12, 0, 100);   // Chan2 A sample minimum
    createConfigParam("Ch3:A:SaMi",   'D', 0x2,  12, 0, 100);   // Chan3 A sample minimum
    createConfigParam("Ch4:A:SaMi",   'D', 0x3,  12, 0, 100);   // Chan4 A sample minimum
    createConfigParam("Ch5:A:SaMi",   'D', 0x4,  12, 0, 100);   // Chan5 A sample minimum
    createConfigParam("Ch6:A:SaMi",   'D', 0x5,  12, 0, 100);   // Chan6 A sample minimum
    createConfigParam("Ch7:A:SaMi",   'D', 0x6,  12, 0, 100);   // Chan7 A sample minimum
    createConfigParam("Ch8:A:SaMi",   'D', 0x7,  12, 0, 100);   // Chan8 A sample minimum
    createConfigParam("Ch1:B:SaMi",   'D', 0x8,  12, 0, 100);   // Chan1 B sample minimum
    createConfigParam("Ch2:B:SaMi",   'D', 0x9,  12, 0, 100);   // Chan2 B sample minimum
    createConfigParam("Ch3:B:SaMi",   'D', 0xA,  12, 0, 100);   // Chan3 B sample minimum
    createConfigParam("Ch4:B:SaMi",   'D', 0xB,  12, 0, 100);   // Chan4 B sample minimum
    createConfigParam("Ch5:B:SaMi",   'D', 0xC,  12, 0, 100);   // Chan5 B sample minimum
    createConfigParam("Ch6:B:SaMi",   'D', 0xD,  12, 0, 100);   // Chan6 B sample minimum
    createConfigParam("Ch7:B:SaMi",   'D', 0xE,  12, 0, 100);   // Chan7 B sample minimum
    createConfigParam("Ch8:B:SaMi",   'D', 0xF,  12, 0, 100);   // Chan8 B sample minimum

    createConfigParam("Ch1:A:SaMa",   'D', 0x10, 12, 0, 1000);  // Chan1 A sample maximum
    createConfigParam("Ch2:A:SaMa",   'D', 0x11, 12, 0, 1000);  // Chan2 A sample maximum
    createConfigParam("Ch3:A:SaMa",   'D', 0x12, 12, 0, 1000);  // Chan3 A sample maximum
    createConfigParam("Ch4:A:SaMa",   'D', 0x13, 12, 0, 1000);  // Chan4 A sample maximum
    createConfigParam("Ch5:A:SaMa",   'D', 0x14, 12, 0, 1000);  // Chan5 A sample maximum
    createConfigParam("Ch6:A:SaMa",   'D', 0x15, 12, 0, 1000);  // Chan6 A sample maximum
    createConfigParam("Ch7:A:SaMa",   'D', 0x16, 12, 0, 1000);  // Chan7 A sample maximum
    createConfigParam("Ch8:A:SaMa",   'D', 0x17, 12, 0, 1000);  // Chan8 A sample maximum
    createConfigParam("Ch1:B:SaMa",   'D', 0x18, 12, 0, 1000);  // Chan1 B sample maximum
    createConfigParam("Ch2:B:SaMa",   'D', 0x19, 12, 0, 1000);  // Chan2 B sample maximum
    createConfigParam("Ch3:B:SaMa",   'D', 0x1A, 12, 0, 1000);  // Chan3 B sample maximum
    createConfigParam("Ch4:B:SaMa",   'D', 0x1B, 12, 0, 1000);  // Chan4 B sample maximum
    createConfigParam("Ch5:B:SaMa",   'D', 0x1C, 12, 0, 1000);  // Chan5 B sample maximum
    createConfigParam("Ch6:B:SaMa",   'D', 0x1D, 12, 0, 1000);  // Chan6 B sample maximum
    createConfigParam("Ch7:B:SaMa",   'D', 0x1E, 12, 0, 1000);  // Chan7 B sample maximum
    createConfigParam("Ch8:B:SaMa",   'D', 0x1F, 12, 0, 1000);  // Chan8 B sample maximum

    createConfigParam("Ch8:B:SlMa",   'D', 0x20, 12, 0, 0);     // Maximum slope

    createConfigParam("Ch1:En",       'E', 0x0,  1,  0, 1);     // Chan1 enable                 (0=disable,1=enable)
    createConfigParam("Ch2:En",       'E', 0x0,  1,  1, 1);     // Chan2 enable                 (0=disable,1=enable)
    createConfigParam("Ch3:En",       'E', 0x0,  1,  2, 1);     // Chan3 enable                 (0=disable,1=enable)
    createConfigParam("Ch4:En",       'E', 0x0,  1,  3, 1);     // Chan4 enable                 (0=disable,1=enable)
    createConfigParam("Ch5:En",       'E', 0x0,  1,  4, 1);     // Chan5 enable                 (0=disable,1=enable)
    createConfigParam("Ch6:En",       'E', 0x0,  1,  5, 1);     // Chan6 enable                 (0=disable,1=enable)
    createConfigParam("Ch7:En",       'E', 0x0,  1,  6, 1);     // Chan7 enable                 (0=disable,1=enable)
    createConfigParam("Ch8:En",       'E', 0x0,  1,  7, 1);     // Chan8 enable                 (0=disable,1=enable)

    createConfigParam("Ch1:VerbEn",   'E', 0x0,  1,  8, 0);     // Chan1 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch2:VerbEn",   'E', 0x0,  1,  9, 0);     // Chan2 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch3:VerbEn",   'E', 0x0,  1, 10, 0);     // Chan3 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch4:VerbEn",   'E', 0x0,  1, 11, 0);     // Chan4 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch5:VerbEn",   'E', 0x0,  1, 12, 0);     // Chan5 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch6:VerbEn",   'E', 0x0,  1, 13, 0);     // Chan6 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch7:VerbEn",   'E', 0x0,  1, 14, 0);     // Chan7 verbose enable         (0=disable,1=enable)
    createConfigParam("Ch8:VerbEn",   'E', 0x0,  1, 15, 0);     // Chan8 verbose enable         (0=disable,1=enable)

    createConfigParam("TimeVetoLo",   'E', 0x1,  32, 0, 0);     // Timestamp veto low
    createConfigParam("TimeVetoHi",   'E', 0x3,  32, 0, 0);     // Timestamp veto high
    createConfigParam("FakTrigDly",   'E', 0x5,  16, 0, 20000); // Fake trigger delay
    createConfigParam("MinAdcVal",    'E', 0x6,  16, 0, 100);   // Minimum ADC
    createConfigParam("MaxAdcVal",    'E', 0x7,  16, 0, 1000);  // Maximum ADC
    createConfigParam("IntRelease",   'E', 0x8,  16, 0, -6);    // Integrator releas pnt [-6:-1]
    createConfigParam("MinDisWidt",   'E', 0x9,  12, 0, 1);     // Minimum discriminator width
    createConfigParam("Sample1",      'E', 0xA,  4,  0, 2);     // Sample 1 point [0:10]
    createConfigParam("Sample2",      'E', 0xB,  6,  0, 14);    // Sample 1 point [2:50]
    createConfigParam("TsyncDelay",   'E', 0xC,  32, 0, 0);     // TSYNC delay

    createConfigParam("LvdsRate",     'F', 0x0,  1, 15, 1);     // LVDS output rate             (0=20Mhz,1=40Mhz)
    createConfigParam("AutoCorSam",   'F', 0x0,  1, 14, 0);     // Auto correction sample mode  (0=TSYNC,1=fake trigger)
    createConfigParam("EdgeDetEn",    'F', 0x0,  1, 13, 1);     // Edge detect enable           (0=disable,1=enable)
    createConfigParam("MstDiscSel",   'F', 0x0,  2, 11, 0);     // Master discriminator select  (0=SUM discr,1=A discr,2=B discr,3=all)
    createConfigParam("AcqEn",        'F', 0x0,  1, 10, 1);     // ROC enable acquisition       (0=disable,1=enable)
    createConfigParam("AutoCorrEn",   'F', 0x0,  1, 9,  0);     // Auto correction mode         (0=enabled,1=disabled)
    createConfigParam("HighResMod",   'F', 0x0,  1, 8,  1);     // High resolution mode         (0=low res 0-127,1=high res 0-255)
    createConfigParam("OutputMode",   'F', 0x0,  2, 6,  0);     // Output mode                  (0=normal,1=raw,2=extended)
    createConfigParam("AcqMode",      'F', 0x0,  2, 4,  0);     // Output mode                  (0=normal,1=verbose,2=reserved,3=fake trigger)
    createConfigParam("TxEnable",     'F', 0x0,  1, 3,  1);     // TX enable                    (0=external,1=always enabled)
    createConfigParam("TsyncSel",     'F', 0x0,  1, 2,  0);     // TSYNC select                 (0=external,1=internal 60Hz)
    createConfigParam("TclkSel",      'F', 0x0,  1, 1,  0);     // TCLK select                  (0=external,1=internal 10MHz)
    createConfigParam("Reset",        'F', 0x0,  1, 0,  0);     // Reset enable                 (0=disable,1=enable)
}
