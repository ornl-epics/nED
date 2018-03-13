/* Event.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Event.h"
#include <cstring>

namespace Event {

namespace BNL {
    Diag& Diag::operator=(const Raw &raw) {
        // Use memcpy() for efficiency
        memcpy(this, &raw, sizeof(Raw));
        x = 0;
        y = 0;
        corrected_x = 0.0;
        corrected_y = 0.0;
        position = raw.position;
        mapped_pixelid = 0;

        return *this;
    }

}; // namespace BNL

}; // namespace Event
