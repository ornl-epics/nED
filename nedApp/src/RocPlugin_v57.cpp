/* RocPlugin_v57.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "RocPlugin.h"

/**
 * @file RocPlugin_v57.cpp
 *
 * ROC 5.7 parameters
 *
 * The ROC 5.7 firmware is identical to ROC 5.5 except that it adds support for
 * Test Pattern Generator write and read commands, command to read Status counters
 * and rate meters, and a command to reset status counters.
 *
 */

void RocPlugin::createStatusParams_v57()
{
//    BLXXX:Det:RocXXX:| sig nam |                         | EPICS record description  | (bi and mbbi description)
    createStatusParam("Acquiring",        0x0,  1, 11); // Acquiring mode.               (0=not acquiring,1=acquiring)
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

    createStatusParam("Ch1:A:BaseAvg",    0x6, 15,  0); // A1 baseline average
    createStatusParam("Ch1:B:BaseAvg",    0x7, 15,  0); // B1 baseline average
    createStatusParam("Ch2:A:BaseAvg",    0x8, 15,  0); // A2 baseline average
    createStatusParam("Ch2:B:BaseAvg",    0x9, 15,  0); // B2 baseline average
    createStatusParam("Ch3:A:BaseAvg",    0xA, 15,  0); // A3 baseline average
    createStatusParam("Ch3:B:BaseAvg",    0xB, 15,  0); // B3 baseline average
    createStatusParam("Ch4:A:BaseAvg",    0xC, 15,  0); // A4 baseline average
    createStatusParam("Ch4:B:BaseAvg",    0xD, 15,  0); // B4 baseline average
    createStatusParam("Ch5:A:BaseAvg",    0xE, 15,  0); // A5 baseline average
    createStatusParam("Ch5:B:BaseAvg",    0xF, 15,  0); // B5 baseline average
    createStatusParam("Ch6:A:BaseAvg",    0x10,15,  0); // A6 baseline average
    createStatusParam("Ch6:B:BaseAvg",    0x11,15,  0); // B6 baseline average
    createStatusParam("Ch7:A:BaseAvg",    0x12,15,  0); // A7 baseline average
    createStatusParam("Ch7:B:BaseAvg",    0x13,15,  0); // B7 baseline average
    createStatusParam("Ch8:A:BaseAvg",    0x14,15,  0); // A8 baseline average
    createStatusParam("Ch8:B:BaseAvg",    0x15,15,  0); // B8 baseline average

    // ROC lies about the length of the packet. It returns 44 bytes of valid
    // data but the packet length is 64 bytes => 20 bytes or 10 registers
    // We only need to define the last one which defines the packet length
    createStatusParam("StatFill",         0x1F,15,  0);
}

void RocPlugin::createCounterParams_v57()
{
    createCounterParam("CntParity",        0x0, 16,  0); // LVDS parity error counter
    createCounterParam("CntUartParity",    0x1, 16,  0); // UART parity error counter
    createCounterParam("CntUartByte",      0x2, 16,  0); // UART byte error counter
    createCounterParam("CntMissClk",       0x3, 16,  0); // Link RX clock missing cnt
    createCounterParam("Ch0:CntFifoFull",  0x4, 16,  0); // Ch0 ADC FIFO full counter
    createCounterParam("Ch1:CntFifoFull",  0x5, 16,  0); // Ch1 ADC FIFO full counter
    createCounterParam("Ch2:CntFifoFull",  0x6, 16,  0); // Ch2 ADC FIFO full counter
    createCounterParam("Ch3:CntFifoFull",  0x7, 16,  0); // Ch3 ADC FIFO full counter
    createCounterParam("Ch4:CntFifoFull",  0x8, 16,  0); // Ch4 ADC FIFO full counter
    createCounterParam("Ch5:CntFifoFull",  0x9, 16,  0); // Ch5 ADC FIFO full counter
    createCounterParam("Ch6:CntFifoFull",  0xA, 16,  0); // Ch6 ADC FIFO full counter
    createCounterParam("Ch7:CntFifoFull",  0xB, 16,  0); // Ch7 ADC FIFO full counter
    createCounterParam("Ch0:CntDiscSum",   0xC, 16,  0); // Ch0 SUM discriminator rate    (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch1:CntDiscSum",   0xD, 16,  0); // Ch1 SUM discriminator rate    (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch2:CntDiscSum",   0xE, 16,  0); // Ch2 SUM discriminator rate    (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch3:CntDiscSum",   0xF, 16,  0); // Ch3 SUM discriminator rate    (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch4:CntDiscSum",   0x10, 16,  0); // Ch4 SUM discriminator rate   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch5:CntDiscSum",   0x11, 16,  0); // Ch5 SUM discriminator rate   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch6:CntDiscSum",   0x12, 16,  0); // Ch6 SUM discriminator rate   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch7:CntDiscSum",   0x13, 16,  0); // Ch7 SUM discriminator rate   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch0:CntRate",      0x14, 16,  0); // Ch0 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch1:CntRate",      0x15, 16,  0); // Ch1 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch2:CntRate",      0x16, 16,  0); // Ch2 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch3:CntRate",      0x17, 16,  0); // Ch3 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch4:CntRate",      0x18, 16,  0); // Ch4 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch5:CntRate",      0x19, 16,  0); // Ch5 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch6:CntRate",      0x1A, 16,  0); // Ch6 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("Ch7:CntRate",      0x1B, 16,  0); // Ch7 outrate counter   (calc:1000*A/52.4288,unit:counts/s)
    createCounterParam("CntOutRate",       0x1C, 16,  0); // Total outrate counter (calc:1000*A/52.4288,unit:counts/s,prec:0)
}

void RocPlugin::createConfigParams_v57()
{
//    BLXXX:Det:RocXXX:| sig nam |                                    | EPICS record description  | (bi and mbbi description)
    createConfigParam("Ch1:PositionIdx",  '1', 0x0,  32, 0, 0);     // Chan1 position index
    createConfigParam("Ch2:PositionIdx",  '1', 0x2,  32, 0, 256);   // Chan2 position index
    createConfigParam("Ch3:PositionIdx",  '1', 0x4,  32, 0, 512);   // Chan3 position index
    createConfigParam("Ch4:PositionIdx",  '1', 0x6,  32, 0, 768);   // Chan4 position index
    createConfigParam("Ch5:PositionIdx",  '1', 0x8,  32, 0, 1024);  // Chan5 position index
    createConfigParam("Ch6:PositionIdx",  '1', 0xA,  32, 0, 1280);  // Chan6 position index
    createConfigParam("Ch7:PositionIdx",  '1', 0xC,  32, 0, 1536);  // Chan7 position index
    createConfigParam("Ch8:PositionIdx",  '1', 0xE,  32, 0, 1792);  // Chan8 position index

    createConfigParam("Ch1:A:InOffset",   '2', 0x0,  12, 0, 100);   // Chan1 A input offset
    createConfigParam("Ch2:A:InOffset",   '2', 0x1,  12, 0, 100);   // Chan2 A input offset
    createConfigParam("Ch3:A:InOffset",   '2', 0x2,  12, 0, 100);   // Chan3 A input offset
    createConfigParam("Ch4:A:InOffset",   '2', 0x3,  12, 0, 100);   // Chan4 A input offset
    createConfigParam("Ch5:A:InOffset",   '2', 0x4,  12, 0, 100);   // Chan5 A input offset
    createConfigParam("Ch6:A:InOffset",   '2', 0x5,  12, 0, 100);   // Chan6 A input offset
    createConfigParam("Ch7:A:InOffset",   '2', 0x6,  12, 0, 100);   // Chan7 A input offset
    createConfigParam("Ch8:A:InOffset",   '2', 0x7,  12, 0, 100);   // Chan8 A input offset
    createConfigParam("Ch1:B:InOffset",   '2', 0x8,  12, 0, 100);   // Chan1 B input offset
    createConfigParam("Ch2:B:InOffset",   '2', 0x9,  12, 0, 100);   // Chan2 B input offset
    createConfigParam("Ch3:B:InOffset",   '2', 0xA,  12, 0, 100);   // Chan3 B input offset
    createConfigParam("Ch4:B:InOffset",   '2', 0xB,  12, 0, 100);   // Chan4 B input offset
    createConfigParam("Ch5:B:InOffset",   '2', 0xC,  12, 0, 100);   // Chan5 B input offset
    createConfigParam("Ch6:B:InOffset",   '2', 0xD,  12, 0, 100);   // Chan6 B input offset
    createConfigParam("Ch7:B:InOffset",   '2', 0xE,  12, 0, 100);   // Chan7 B input offset
    createConfigParam("Ch8:B:InOffset",   '2', 0xF,  12, 0, 100);   // Chan8 B input offset

    createConfigParam("Ch1:A:AdcOffset",  '2', 0x10, 12, 0, 100);   // Chan1 A ADC offset
    createConfigParam("Ch2:A:AdcOffset",  '2', 0x11, 12, 0, 100);   // Chan2 A ADC offset
    createConfigParam("Ch3:A:AdcOffset",  '2', 0x12, 12, 0, 100);   // Chan3 A ADC offset
    createConfigParam("Ch4:A:AdcOffset",  '2', 0x13, 12, 0, 100);   // Chan4 A ADC offset
    createConfigParam("Ch5:A:AdcOffset",  '2', 0x14, 12, 0, 100);   // Chan5 A ADC offset
    createConfigParam("Ch6:A:AdcOffset",  '2', 0x15, 12, 0, 100);   // Chan6 A ADC offset
    createConfigParam("Ch7:A:AdcOffset",  '2', 0x16, 12, 0, 100);   // Chan7 A ADC offset
    createConfigParam("Ch8:A:AdcOffset",  '2', 0x17, 12, 0, 100);   // Chan8 A ADC offset
    createConfigParam("Ch1:B:AdcOffset",  '2', 0x18, 12, 0, 100);   // Chan1 B ADC offset
    createConfigParam("Ch2:B:AdcOffset",  '2', 0x19, 12, 0, 100);   // Chan2 B ADC offset
    createConfigParam("Ch3:B:AdcOffset",  '2', 0x1A, 12, 0, 100);   // Chan3 B ADC offset
    createConfigParam("Ch4:B:AdcOffset",  '2', 0x1B, 12, 0, 100);   // Chan4 B ADC offset
    createConfigParam("Ch5:B:AdcOffset",  '2', 0x1C, 12, 0, 100);   // Chan5 B ADC offset
    createConfigParam("Ch6:B:AdcOffset",  '2', 0x1D, 12, 0, 100);   // Chan6 B ADC offset
    createConfigParam("Ch7:B:AdcOffset",  '2', 0x1E, 12, 0, 100);   // Chan7 B ADC offset
    createConfigParam("Ch8:B:AdcOffset",  '2', 0x1F, 12, 0, 100);   // Chan8 B ADC offset

    createConfigParam("Ch1:A:Threshold",  '2', 0x20, 12, 0, 400);   // Chan1 A threshold
    createConfigParam("Ch2:A:Threshold",  '2', 0x21, 12, 0, 400);   // Chan2 A threshold
    createConfigParam("Ch3:A:Threshold",  '2', 0x22, 12, 0, 400);   // Chan3 A threshold
    createConfigParam("Ch4:A:Threshold",  '2', 0x23, 12, 0, 400);   // Chan4 A threshold
    createConfigParam("Ch5:A:Threshold",  '2', 0x24, 12, 0, 400);   // Chan5 A threshold
    createConfigParam("Ch6:A:Threshold",  '2', 0x25, 12, 0, 400);   // Chan6 A threshold
    createConfigParam("Ch7:A:Threshold",  '2', 0x26, 12, 0, 400);   // Chan7 A threshold
    createConfigParam("Ch8:A:Threshold",  '2', 0x27, 12, 0, 400);   // Chan8 A threshold
    createConfigParam("Ch1:B:Threshold",  '2', 0x28, 12, 0, 400);   // Chan1 B threshold
    createConfigParam("Ch2:B:Threshold",  '2', 0x29, 12, 0, 400);   // Chan2 B threshold
    createConfigParam("Ch3:B:Threshold",  '2', 0x2A, 12, 0, 400);   // Chan3 B threshold
    createConfigParam("Ch4:B:Threshold",  '2', 0x2B, 12, 0, 400);   // Chan4 B threshold
    createConfigParam("Ch5:B:Threshold",  '2', 0x2C, 12, 0, 400);   // Chan5 B threshold
    createConfigParam("Ch6:B:Threshold",  '2', 0x2D, 12, 0, 400);   // Chan6 B threshold
    createConfigParam("Ch7:B:Threshold",  '2', 0x2E, 12, 0, 400);   // Chan7 B threshold
    createConfigParam("Ch8:B:Threshold",  '2', 0x2F, 12, 0, 400);   // Chan8 B threshold

    createConfigParam("Ch1:PosThreshold", '2', 0x30, 12, 0, 400);   // Chan1 positive threshold
    createConfigParam("Ch2:PosThreshold", '2', 0x31, 12, 0, 400);   // Chan2 positive threshold
    createConfigParam("Ch3:PosThreshold", '2', 0x32, 12, 0, 400);   // Chan3 positive threshold
    createConfigParam("Ch4:PosThreshold", '2', 0x33, 12, 0, 400);   // Chan4 positive threshold
    createConfigParam("Ch5:PosThreshold", '2', 0x34, 12, 0, 400);   // Chan5 positive threshold
    createConfigParam("Ch6:PosThreshold", '2', 0x35, 12, 0, 400);   // Chan6 positive threshold
    createConfigParam("Ch7:PosThreshold", '2', 0x36, 12, 0, 400);   // Chan7 positive threshold
    createConfigParam("Ch8:PosThreshold", '2', 0x37, 12, 0, 400);   // Chan8 positive threshold
    createConfigParam("Ch1:A:FullScale",  '2', 0x38, 12, 0, 10);    // Chan1 A full scale
    createConfigParam("Ch2:A:FullScale",  '2', 0x39, 12, 0, 10);    // Chan2 A full scale
    createConfigParam("Ch3:A:FullScale",  '2', 0x3A, 12, 0, 10);    // Chan3 A full scale
    createConfigParam("Ch4:A:FullScale",  '2', 0x3B, 12, 0, 10);    // Chan4 A full scale
    createConfigParam("Ch5:A:FullScale",  '2', 0x3C, 12, 0, 10);    // Chan5 A full scale
    createConfigParam("Ch6:A:FullScale",  '2', 0x3D, 12, 0, 10);    // Chan6 A full scale
    createConfigParam("Ch7:A:FullScale",  '2', 0x3E, 12, 0, 10);    // Chan7 A full scale
    createConfigParam("Ch8:A:FullScale",  '2', 0x3F, 12, 0, 10);    // Chan8 A full scale

    createConfigParam("Ch1:B:FullScale",  '2', 0x40, 12, 0, 10);    // Chan1 B full scale
    createConfigParam("Ch2:B:FullScale",  '2', 0x41, 12, 0, 10);    // Chan2 B full scale
    createConfigParam("Ch3:B:FullScale",  '2', 0x42, 12, 0, 10);    // Chan3 B full scale
    createConfigParam("Ch4:B:FullScale",  '2', 0x43, 12, 0, 10);    // Chan4 B full scale
    createConfigParam("Ch5:B:FullScale",  '2', 0x44, 12, 0, 10);    // Chan5 B full scale
    createConfigParam("Ch6:B:FullScale",  '2', 0x45, 12, 0, 10);    // Chan6 B full scale
    createConfigParam("Ch7:B:FullScale",  '2', 0x46, 12, 0, 10);    // Chan7 B full scale
    createConfigParam("Ch8:B:FullScale",  '2', 0x47, 12, 0, 10);    // Chan8 B full scale
    createConfigParam("Ch1:A:GainAdj",    '2', 0x48, 12, 0, 0);     // Chan1 A gain adjust
    createConfigParam("Ch2:A:GainAdj",    '2', 0x49, 12, 0, 0);     // Chan2 A gain adjust
    createConfigParam("Ch3:A:GainAdj",    '2', 0x4A, 12, 0, 0);     // Chan3 A gain adjust
    createConfigParam("Ch4:A:GainAdj",    '2', 0x4B, 12, 0, 0);     // Chan4 A gain adjust
    createConfigParam("Ch5:A:GainAdj",    '2', 0x4C, 12, 0, 0);     // Chan5 A gain adjust
    createConfigParam("Ch6:A:GainAdj",    '2', 0x4D, 12, 0, 0);     // Chan6 A gain adjust
    createConfigParam("Ch7:A:GainAdj",    '2', 0x4E, 12, 0, 0);     // Chan7 A gain adjust
    createConfigParam("Ch8:A:GainAdj",    '2', 0x4F, 12, 0, 0);     // Chan8 A gain adjust

    createConfigParam("Ch1:B:GainAdj",    '2', 0x50, 12, 0, 0);     // Chan1 B gain adjust
    createConfigParam("Ch2:B:GainAdj",    '2', 0x51, 12, 0, 0);     // Chan2 B gain adjust
    createConfigParam("Ch3:B:GainAdj",    '2', 0x52, 12, 0, 0);     // Chan3 B gain adjust
    createConfigParam("Ch4:B:GainAdj",    '2', 0x53, 12, 0, 0);     // Chan4 B gain adjust
    createConfigParam("Ch5:B:GainAdj",    '2', 0x54, 12, 0, 0);     // Chan5 B gain adjust
    createConfigParam("Ch6:B:GainAdj",    '2', 0x55, 12, 0, 0);     // Chan6 B gain adjust
    createConfigParam("Ch7:B:GainAdj",    '2', 0x56, 12, 0, 0);     // Chan7 B gain adjust
    createConfigParam("Ch8:B:GainAdj",    '2', 0x57, 12, 0, 0);     // Chan8 B gain adjust

    createConfigParam("Ch1:A:Scale",      '3', 0x0,  12, 0, 2048);  // Chan1 A scale
    createConfigParam("Ch1:A:Offset",     '3', 0x1,  12, 0, 1600);  // Chan1 A offset
    createConfigParam("Ch1:B:Scale",      '3', 0x2,  12, 0, 2048);  // Chan1 B scale
    createConfigParam("Ch1:B:Offset",     '3', 0x3,  12, 0, 1600);  // Chan1 B offset
    createConfigParam("Ch2:A:Scale",      '3', 0x4,  12, 0, 2048);  // Chan2 A scale
    createConfigParam("Ch2:A:Offset",     '3', 0x5,  12, 0, 1600);  // Chan2 A offset
    createConfigParam("Ch2:B:Scale",      '3', 0x6,  12, 0, 2048);  // Chan2 B scale
    createConfigParam("Ch2:B:Offset",     '3', 0x7,  12, 0, 1600);  // Chan2 B offset
    createConfigParam("Ch3:A:Scale",      '3', 0x8,  12, 0, 2048);  // Chan3 A scale
    createConfigParam("Ch3:A:Offset",     '3', 0x9,  12, 0, 1600);  // Chan3 A offset
    createConfigParam("Ch3:B:Scale",      '3', 0xA,  12, 0, 2048);  // Chan3 B scale
    createConfigParam("Ch3:B:Offset",     '3', 0xB,  12, 0, 1600);  // Chan3 B offset
    createConfigParam("Ch4:A:Scale",      '3', 0xC,  12, 0, 2048);  // Chan4 A scale
    createConfigParam("Ch4:A:Offset",     '3', 0xD,  12, 0, 1600);  // Chan4 A offset
    createConfigParam("Ch4:B:Scale",      '3', 0xE,  12, 0, 2048);  // Chan4 B scale
    createConfigParam("Ch4:B:Offset",     '3', 0xF,  12, 0, 1600);  // Chan4 B offset

    createConfigParam("Ch5:A:Scale",      '3', 0x10, 12, 0, 2048);  // Chan5 A scale
    createConfigParam("Ch5:A:Offset",     '3', 0x11, 12, 0, 1600);  // Chan5 A offset
    createConfigParam("Ch5:B:Scale",      '3', 0x12, 12, 0, 2048);  // Chan5 B scale
    createConfigParam("Ch5:B:Offset",     '3', 0x13, 12, 0, 1600);  // Chan5 B offset
    createConfigParam("Ch6:A:Scale",      '3', 0x14, 12, 0, 2048);  // Chan6 A scale
    createConfigParam("Ch6:A:Offset",     '3', 0x15, 12, 0, 1600);  // Chan6 A offset
    createConfigParam("Ch6:B:Scale",      '3', 0x16, 12, 0, 2048);  // Chan6 B scale
    createConfigParam("Ch6:B:Offset",     '3', 0x17, 12, 0, 1600);  // Chan6 B offset
    createConfigParam("Ch7:A:Scale",      '3', 0x18, 12, 0, 2048);  // Chan7 A scale
    createConfigParam("Ch7:A:Offset",     '3', 0x19, 12, 0, 1600);  // Chan7 A offset
    createConfigParam("Ch7:B:Scale",      '3', 0x1A, 12, 0, 2048);  // Chan7 B scale
    createConfigParam("Ch7:B:Offset",     '3', 0x1B, 12, 0, 1600);  // Chan7 B offset
    createConfigParam("Ch8:A:Scale",      '3', 0x1C, 12, 0, 2048);  // Chan8 A scale
    createConfigParam("Ch8:A:Offset",     '3', 0x1D, 12, 0, 1600);  // Chan8 A offset
    createConfigParam("Ch8:B:Scale",      '3', 0x1E, 12, 0, 2048);  // Chan8 B scale
    createConfigParam("Ch8:B:Offset",     '3', 0x1F, 12, 0, 1600);  // Chan8 B offset

    createConfigParam("Ch1:ScaleFact",    '3', 0x20, 12, 0, 2048);  // Chan1 scale factor
    createConfigParam("Ch2:ScaleFact",    '3', 0x21, 12, 0, 2048);  // Chan2 scale factor
    createConfigParam("Ch3:ScaleFact",    '3', 0x22, 12, 0, 2048);  // Chan3 scale factor
    createConfigParam("Ch4:ScaleFact",    '3', 0x23, 12, 0, 2048);  // Chan4 scale factor
    createConfigParam("Ch5:ScaleFact",    '3', 0x24, 12, 0, 2048);  // Chan5 scale factor
    createConfigParam("Ch6:ScaleFact",    '3', 0x25, 12, 0, 2048);  // Chan6 scale factor
    createConfigParam("Ch7:ScaleFact",    '3', 0x26, 12, 0, 2048);  // Chan7 scale factor
    createConfigParam("Ch8:ScaleFact",    '3', 0x27, 12, 0, 2048);  // Chan8 scale factor

    createConfigParam("Ch1:A:Baseline",   'C', 0x0,  12, 0, 100);   // Chan1 A baseline value
    createConfigParam("Ch1:B:Baseline",   'C', 0x1,  12, 0, 100);   // Chan1 B baseline value
    createConfigParam("Ch2:A:Baseline",   'C', 0x2,  12, 0, 100);   // Chan2 A baseline value
    createConfigParam("Ch2:B:Baseline",   'C', 0x3,  12, 0, 100);   // Chan2 B baseline value
    createConfigParam("Ch3:A:Baseline",   'C', 0x4,  12, 0, 100);   // Chan3 A baseline value
    createConfigParam("Ch3:B:Baseline",   'C', 0x5,  12, 0, 100);   // Chan3 B baseline value
    createConfigParam("Ch4:A:Baseline",   'C', 0x6,  12, 0, 100);   // Chan4 A baseline value
    createConfigParam("Ch4:B:Baseline",   'C', 0x7,  12, 0, 100);   // Chan4 B baseline value
    createConfigParam("Ch5:A:Baseline",   'C', 0x8,  12, 0, 100);   // Chan5 A baseline value
    createConfigParam("Ch5:B:Baseline",   'C', 0x9,  12, 0, 100);   // Chan5 B baseline value
    createConfigParam("Ch6:A:Baseline",   'C', 0xA,  12, 0, 100);   // Chan6 A baseline value
    createConfigParam("Ch6:B:Baseline",   'C', 0xB,  12, 0, 100);   // Chan6 B baseline value
    createConfigParam("Ch7:A:Baseline",   'C', 0xC,  12, 0, 100);   // Chan7 A baseline value
    createConfigParam("Ch7:B:Baseline",   'C', 0xD,  12, 0, 100);   // Chan7 B baseline value
    createConfigParam("Ch8:A:Baseline",   'C', 0xE,  12, 0, 100);   // Chan8 A baseline value
    createConfigParam("Ch8:B:Baseline",   'C', 0xF,  12, 0, 100);   // Chan8 B baseline value

    // There's an unused part of C section in v5.4 C:0x10 - C:0x1F
    createConfigParam("ConfigFillerC",    'C', 0x1F, 16, 0, 0);     // Config filler

    createConfigParam("Ch1:A:SampleMin",  'D', 0x0,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch2:A:SampleMin",  'D', 0x1,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch3:A:SampleMin",  'D', 0x2,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch4:A:SampleMin",  'D', 0x3,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch5:A:SampleMin",  'D', 0x4,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch6:A:SampleMin",  'D', 0x5,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch7:A:SampleMin",  'D', 0x6,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch8:A:SampleMin",  'D', 0x7,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch1:B:SampleMin",  'D', 0x8,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch2:B:SampleMin",  'D', 0x9,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch3:B:SampleMin",  'D', 0xA,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch4:B:SampleMin",  'D', 0xB,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch5:B:SampleMin",  'D', 0xC,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch6:B:SampleMin",  'D', 0xD,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch7:B:SampleMin",  'D', 0xE,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch8:B:SampleMin",  'D', 0xF,  12, 0, 100);   // *** UNUSED BY FIRMWARE ***

    createConfigParam("Ch1:A:SampleMax",  'D', 0x10, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch2:A:SampleMax",  'D', 0x11, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch3:A:SampleMax",  'D', 0x12, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch4:A:SampleMax",  'D', 0x13, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch5:A:SampleMax",  'D', 0x14, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch6:A:SampleMax",  'D', 0x15, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch7:A:SampleMax",  'D', 0x16, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch8:A:SampleMax",  'D', 0x17, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch1:B:SampleMax",  'D', 0x18, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch2:B:SampleMax",  'D', 0x19, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch3:B:SampleMax",  'D', 0x1A, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch4:B:SampleMax",  'D', 0x1B, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch5:B:SampleMax",  'D', 0x1C, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch6:B:SampleMax",  'D', 0x1D, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch7:B:SampleMax",  'D', 0x1E, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***
    createConfigParam("Ch8:B:SampleMax",  'D', 0x1F, 12, 0, 1000);  // *** UNUSED BY FIRMWARE ***

    createConfigParam("MaximumSlope",     'D', 0x20, 12, 0, 0);     // *** UNUSED BY FIRMWARE ***

    createConfigParam("Ch1:Enable",       'E', 0x0,  1,  0, 1);     // Chan1 enable                  (0=disable,1=enable)
    createConfigParam("Ch2:Enable",       'E', 0x0,  1,  1, 1);     // Chan2 enable                  (0=disable,1=enable)
    createConfigParam("Ch3:Enable",       'E', 0x0,  1,  2, 1);     // Chan3 enable                  (0=disable,1=enable)
    createConfigParam("Ch4:Enable",       'E', 0x0,  1,  3, 1);     // Chan4 enable                  (0=disable,1=enable)
    createConfigParam("Ch5:Enable",       'E', 0x0,  1,  4, 1);     // Chan5 enable                  (0=disable,1=enable)
    createConfigParam("Ch6:Enable",       'E', 0x0,  1,  5, 1);     // Chan6 enable                  (0=disable,1=enable)
    createConfigParam("Ch7:Enable",       'E', 0x0,  1,  6, 1);     // Chan7 enable                  (0=disable,1=enable)
    createConfigParam("Ch8:Enable",       'E', 0x0,  1,  7, 1);     // Chan8 enable                  (0=disable,1=enable)

    createConfigParam("Ch1:VerboseEn",    'E', 0x0,  1,  8, 0);     // Chan1 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch2:VerboseEn",    'E', 0x0,  1,  9, 0);     // Chan2 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch3:VerboseEn",    'E', 0x0,  1, 10, 0);     // Chan3 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch4:VerboseEn",    'E', 0x0,  1, 11, 0);     // Chan4 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch5:VerboseEn",    'E', 0x0,  1, 12, 0);     // Chan5 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch6:VerboseEn",    'E', 0x0,  1, 13, 0);     // Chan6 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch7:VerboseEn",    'E', 0x0,  1, 14, 0);     // Chan7 verbose enable          (0=disable,1=enable)
    createConfigParam("Ch8:VerboseEn",    'E', 0x0,  1, 15, 0);     // Chan8 verbose enable          (0=disable,1=enable)

    createConfigParam("TimeVetoLow",      'E', 0x1,  32, 0, 0);     // Timestamp veto low
    createConfigParam("TimeVetoHigh",     'E', 0x3,  32, 0, 0);     // Timestamp veto high
    createConfigParam("FakeTrigDelay",    'E', 0x5,  16, 0, 20000); // Fake trigger delay
    createConfigParam("MinAdc",           'E', 0x6,  12, 0, 100);   // Minimum ADC
    createConfigParam("MaxAdc",           'E', 0x7,  12, 0, 1000);  // Maximum ADC
    createConfigParam("IntRelease",       'E', 0x8,  8,  0, 65530); // Integrator release point
    createConfigParam("MinDiscrimWidth",  'E', 0x9,  8,  0, 1);     // Minimum discriminator width
    createConfigParam("Sample1",          'E', 0xA,  4,  0, 2);     // Sample 1 point [0:10]
    createConfigParam("Sample2",          'E', 0xB,  6,  0, 14);    // Sample 1 point [2:50]
    createConfigParam("TsyncDelay",       'E', 0xC,  32, 0, 0);     // TSYNC delay

    createConfigParam("LvdsRate",         'F', 0x0,  1, 15, 1);     // *** UNUSED BY FIRMWARE ***    (0=20Mhz,1=40Mhz)
    createConfigParam("AutoCorrectMode",  'F', 0x0,  1, 14, 0);     // *** UNUSED BY FIRMWARE ***    (0=TSYNC,1=fake trigger)
    createConfigParam("EdgeDetectEn",     'F', 0x0,  1, 13, 1);     // *** UNUSED BY FIRMWARE ***    (0=disable,1=enable)
    createConfigParam("MastDiscrimMode",  'F', 0x0,  2, 11, 0);     // *** UNUSED BY FIRMWARE ***    (0=SUM discr,1=A discr,2=B discr,3=all)
    createConfigParam("AcquireEn",        'F', 0x0,  1, 10, 1);     // *** UNUSED BY FIRMWARE ***    (0=disable,1=enable)
    createConfigParam("AutoCorrectionEn", 'F', 0x0,  1, 9,  0);     // Auto correction mode          (0=enabled,1=disabled)
    createConfigParam("HighResMode",      'F', 0x0,  1, 8,  1);     // High resolution mode          (0=low res 0-127,1=high res 0-255)
    createConfigParam("OutputMode",       'F', 0x0,  2, 6,  0);     // Output mode                   (0=normal,1=raw,2=extended)
    createConfigParam("AcquireMode",      'F', 0x0,  2, 4,  0);     // Acquire mode                  (0=normal,1=verbose,2=fakedata,3=trigger)
    createConfigParam("TxEnable",         'F', 0x0,  1, 3,  1);     // TX enable                     (0=external,1=always enabled)
    createConfigParam("TsyncSelect",      'F', 0x0,  1, 2,  0);     // TSYNC select                  (0=external,1=internal 60Hz)
    createConfigParam("TclkSelect",       'F', 0x0,  1, 1,  0);     // TCLK select                   (0=external,1=internal 10MHz)
    createConfigParam("Reset",            'F', 0x0,  1, 0,  0);     // Reset enable                  (0=disable,1=enable)

    createConfigParam("TestPatternEn",    'F', 0x1,  1, 15, 0);     // Test pattern enable           (0=disable,1=enable)
    createConfigParam("TestPatternDebug", 'F', 0x1,  3, 12, 0);     // Engineering Use only
    createConfigParam("TestPatternId",    'F', 0x1, 12, 0,  0);     // Test pattern id
    createConfigParam("TestPatternRate",  'F', 0x2, 16, 0,  0);     // Test pattern rate             (65535=153 ev/s,9999=1 Kev/s,4999=2 Kev/s,1999=5 Kev/s,999=10 Kev/s,399=25 Kev/s,199=50 Kev/s,99=100 Kev/s,13=800 Kev/s,9=1 Mev/s,4=2 Mev/s,1=5 Mev/s,0=10 Mev/s)
}
