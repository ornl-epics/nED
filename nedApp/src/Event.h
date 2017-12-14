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

    /**
     * Used by ACPC in normal mode.
     */
    typedef struct {
        uint32_t tof;
        uint32_t position_index;    //!< Detector position, masked with calculated pixel id
        uint32_t position_x;        //!< Q24.8 format
        uint32_t position_y;        //!< Q24.8 format
        uint32_t photo_sum_x;       //!< Q15.17 format
        uint32_t photo_sum_y;       //!< Q15.17 format
    } AcpcXyPs;

    /**
     * AROC raw event.
     */
    typedef struct {
        uint32_t time_of_flight;
        uint32_t position_index;
        uint32_t sample_a[8];
        uint32_t sample_b[8];
    } ArocRaw;
};

#endif // EVENT_H
