/* ValueConvert.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 * @date September 2015
 */

#include "ValueConvert.h"

#include <epicsAlgorithm.h>

BaseConvert::BaseConvert()
{}

UnsignConvert::UnsignConvert()
: BaseConvert()
{}

int UnsignConvert::fromRaw(uint32_t value, uint8_t bits) const
{
    return (int)value;
}

uint32_t UnsignConvert::toRaw(int value, uint8_t bits) const
{
    if (checkBounds(value, bits) == false)
        return 0;

    int mask = (1ULL << bits) - 1;
    return (uint32_t)value & mask;
}

bool UnsignConvert::checkBounds(int value, uint8_t bits) const
{
    bits = (bits == 32 ? 31 : bits);
    int max = (0x1ULL << bits) - 1;
    int min = 0;
    return (min <= value && value <= max);
}

Sign2sComplementConvert::Sign2sComplementConvert()
: BaseConvert()
{}

int Sign2sComplementConvert::fromRaw(uint32_t value, uint8_t bits) const
{
    int mask = ((1ULL << bits) - 1);
    int sign = 1U << (bits - 1);
    return ((value &mask) ^ sign) - sign;
}

uint32_t Sign2sComplementConvert::toRaw(int value, uint8_t bits) const
{
    if (checkBounds(value, bits) == false)
        return 0;

    int mask = (1ULL << bits) - 1;
    return (uint32_t)value & mask;
}

bool Sign2sComplementConvert::checkBounds(int value, uint8_t bits) const
{
    int max = (0x1ULL << (bits - 1)) - 1;
    int min = -1 * (0x1ULL << (bits - 1));
    return (min <= value && value <= max);
}

SignMagnitudeConvert::SignMagnitudeConvert()
: BaseConvert()
{}

int SignMagnitudeConvert::fromRaw(uint32_t value, uint8_t bits) const
{
    value &= ((1ULL << bits) - 1);

    int sign = (value & (1U << (bits - 1)));
    if (sign != 0) {
        int mask = (1U << bits) - 1;
        value = (sign | ~(value & mask)) + 1;
    }

    return value;
}

uint32_t SignMagnitudeConvert::toRaw(int value, uint8_t bits) const
{
    if (checkBounds(value, bits) == false)
        return 0;

    int sign = (value & (1U << (bits - 1)));
    if (sign != 0)
        value = sign | ~(value - 1);

    return value;
}

bool SignMagnitudeConvert::checkBounds(int value, uint8_t bits) const
{
    int max = (0x1ULL << (bits - 1)) - 1;
    int min = -1 * max;
    return (min <= value && value <= max);
}


Hex2DecConvert::Hex2DecConvert()
: BaseConvert()
{}

int Hex2DecConvert::fromRaw(uint32_t value, uint8_t bits) const
{
    int converted = 0;
    int mult = 1;
    for (int i = 0; i < 4; i++) {
        int byte = value & 0xFF;
        converted += mult * ((byte/16)*10 + (byte%16));
        mult *= 100;
        value >>= 8;
    }
    return converted;
}

uint32_t Hex2DecConvert::toRaw(int value, uint8_t bits) const
{
    if (checkBounds(value, bits) == false)
        return 0;

    int converted = 0;
    for (int i = 0; i < 4; i++) {
        int temp = value % 100;
        converted |= ((temp/10)*16 + temp%10) << (8*i);
        value /= 100;
    }
    return converted;
}

bool Hex2DecConvert::checkBounds(int value, uint8_t bits) const
{
    int max = (0x1ULL << (bits - 1)) - 1;
    int min = -1 * max;
    return (min <= value && value <= max);
}

Hex2DecConvert2K::Hex2DecConvert2K()
: Hex2DecConvert()
{}

int Hex2DecConvert2K::fromRaw(uint32_t value, uint8_t bits) const
{
    return Hex2DecConvert::fromRaw(value, bits) + 2000;
}

uint32_t Hex2DecConvert2K::toRaw(int value, uint8_t bits) const
{
    return Hex2DecConvert::toRaw(value-2000, bits);
}
