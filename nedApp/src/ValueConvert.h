/* ValueConvert.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef VALUE_CONVERT_H
#define VALUE_CONVERT_H

#include <stdint.h>

/**
 * Base value converter class.
 */
class BaseConvert {
    public:

        BaseConvert();

        virtual int fromRaw(uint32_t value, uint8_t bits=32) const = 0;

        virtual uint32_t toRaw(int value, uint8_t bits=32) const = 0;

        virtual bool checkBounds(int value, uint8_t bits=32) const = 0;
};

class UnsignConvert : public BaseConvert {
    public:
        UnsignConvert();

        int fromRaw(uint32_t value, uint8_t bits=32) const;

        uint32_t toRaw(int value, uint8_t bits=32) const;

        bool checkBounds(int value, uint8_t bits=32) const;
};

class Sign2sComplementConvert : public BaseConvert {
    public:
        Sign2sComplementConvert();

        int fromRaw(uint32_t value, uint8_t bits=32) const;

        uint32_t toRaw(int value, uint8_t bits=32) const;

        bool checkBounds(int value, uint8_t bits=32) const;
};

class SignMagnitudeConvert : public BaseConvert {
    public:
        SignMagnitudeConvert();

        int fromRaw(uint32_t value, uint8_t bits=32) const;

        uint32_t toRaw(int value, uint8_t bits=32) const;

        bool checkBounds(int value, uint8_t bits=32) const;
};

#endif // VALUE_CONVERT_H
