/* Event.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

namespace Event {
    /**
     * Structure representing single TOF,pixel event
     */
    typedef struct {
        uint32_t tof;
        uint32_t pixelid;
    } Pixel;
};

#endif // EVENT_H
