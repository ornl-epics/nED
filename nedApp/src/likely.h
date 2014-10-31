/* PixelMapPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef LIKELY_H_
#define LIKELY_H_

#if defined(__GNUC__) && __GNUC__ >= 3
    #define likely(x)   __builtin_expect((x), 1)
    #define unlikely(x) __builtin_expect((x), 0)
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif

#endif /* LIKELY_H_ */
