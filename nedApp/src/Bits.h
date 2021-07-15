/* Bits.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef BITS_H
#define BITS_H

#include <cinttypes>

/**
 * Class with common bitwise operations and other numerical hacks
 *
 * A lot of these operations are taken from community web site:
 * https://graphics.stanford.edu/~seander/bithacks.html
 */
class Bits {
    public:
        /**
         * Round 32-bit value to the next highest power of 2
         */
        static uint32_t roundUpPower2(uint32_t number) {
            number--;
            number |= number >> 1;
            number |= number >> 2;
            number |= number >> 4;
            number |= number >> 8;
            number |= number >> 16;
            number++;
            return number;
        }
};

#endif // BITS_H
