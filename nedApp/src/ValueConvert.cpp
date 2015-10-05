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

BaseConvert::BaseConvert(uint8_t bits)
: m_bits(bits)
{}

UnsignConvert::UnsignConvert(uint8_t bits)
: BaseConvert(bits)
{}

int UnsignConvert::fromRaw(uint32_t value)
{
    return (int)value;
}

uint32_t UnsignConvert::toRaw(int value)
{
    if (checkBounds(value) == false)
        return 0;

    int mask = (1ULL << m_bits) - 1;
    return (uint32_t)value & mask;
}

bool UnsignConvert::checkBounds(int value)
{
    uint8_t bits = (m_bits == 32 ? 31 : m_bits);
    int max = (0x1ULL << bits) - 1;
    int min = 0;
    return (min <= value && value <= max);
}

Sign2sComplementConvert::Sign2sComplementConvert(uint8_t bits)
: BaseConvert(bits)
{}

int Sign2sComplementConvert::fromRaw(uint32_t value)
{
    int mask = ((1ULL << m_bits) - 1);
    int sign = 1U << (m_bits - 1);
    return ((value &mask) ^ sign) - sign;
}

uint32_t Sign2sComplementConvert::toRaw(int value)
{
    if (checkBounds(value) == false)
        return 0;

    int mask = (1ULL << m_bits) - 1;
    return (uint32_t)value & mask;
}

bool Sign2sComplementConvert::checkBounds(int value)
{
    int max = (0x1ULL << (m_bits - 1)) - 1;
    int min = -1 * (0x1ULL << (m_bits - 1));
    return (min <= value && value <= max);
}

SignMagnitudeConvert::SignMagnitudeConvert(uint8_t bits)
: BaseConvert(bits)
{}

int SignMagnitudeConvert::fromRaw(uint32_t value)
{
    value &= ((1ULL << m_bits) - 1);

    int sign = (value & (1U << (m_bits - 1)));
    if (sign != 0) {
        int mask = (1U << m_bits) - 1;
        value = (sign | ~(value & mask)) + 1;
    }

    return value;
}

uint32_t SignMagnitudeConvert::toRaw(int value)
{
    if (checkBounds(value) == false)
        return 0; 

    int sign = (value & (1U << (m_bits - 1)));
    if (sign != 0)
        value = sign | ~(value - 1);

    return value;
}

bool SignMagnitudeConvert::checkBounds(int value)
{
    int max = (0x1ULL << (m_bits - 1)) - 1;
    int min = -1 * max;
    return (min <= value && value <= max);
}

