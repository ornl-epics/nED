/* DmaCircularBuffer.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "DmaCircularBuffer.h"

#include <occlib.h>

DmaCircularBuffer::DmaCircularBuffer(struct occ_handle *occ)
    : m_occ(occ)
{
}

DmaCircularBuffer::~DmaCircularBuffer()
{
}

int DmaCircularBuffer::wait(void **data, uint32_t *len, double timeout)
{
    size_t l = 0;
    int status = occ_data_wait(m_occ, data, &l, timeout*1000);
    *len = l;
    return status;
}

int DmaCircularBuffer::consume(uint32_t len)
{
    int ret = occ_data_ack(m_occ, len);

    if (ret == 0)
        (void)BaseCircularBuffer::consume(len);

    return ret;
}

bool DmaCircularBuffer::empty()
{
    int status;
    void *data;
    size_t len;

    status = occ_data_wait(m_occ, &data, &len, 1);
    return (status == 0 && len > 0);
}
