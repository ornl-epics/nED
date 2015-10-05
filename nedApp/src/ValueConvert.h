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
    protected:
        uint8_t m_bits;
    public:

        BaseConvert(uint8_t bits);

        virtual int fromRaw(uint32_t value) = 0;

        virtual uint32_t toRaw(int value) = 0;

        virtual bool checkBounds(int value) = 0;
};

class UnsignConvert : public BaseConvert {
    public:
        UnsignConvert(uint8_t bits);

        int fromRaw(uint32_t value);

        uint32_t toRaw(int value);

        bool checkBounds(int value);
};

class Sign2sComplementConvert : public BaseConvert {
    public:
        Sign2sComplementConvert(uint8_t bits);

        int fromRaw(uint32_t value);

        uint32_t toRaw(int value);

        bool checkBounds(int value);
};

class SignMagnitudeConvert : public BaseConvert {
    public:
        SignMagnitudeConvert(uint8_t bits);

        int fromRaw(uint32_t value);

        uint32_t toRaw(int value);

        bool checkBounds(int value);
};

#endif // VALUE_CONVERT_H
