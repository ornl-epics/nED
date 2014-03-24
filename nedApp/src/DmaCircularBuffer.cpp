#include "DmaCircularBuffer.h"

#include <occlib.h>

DmaCircularBuffer::DmaCircularBuffer(struct occ_handle *occ)
    : m_occ(occ)
{
}

DmaCircularBuffer::~DmaCircularBuffer()
{
}

void DmaCircularBuffer::wait(void **data, uint32_t *len)
{
    size_t l;
    int status = occ_data_wait(m_occ, data, &l, 0);
    if (status != 0) {
        // TODO: error reporting
        *len = 0;
    }
    *len = l;
}

void DmaCircularBuffer::consume(uint32_t len)
{
    int status = occ_data_ack(m_occ, len);
    if (status != 0) {
        // TODO: error reporting
    }
}

bool DmaCircularBuffer::empty()
{
    int status;
    void *data;
    size_t len;

    status = occ_data_wait(m_occ, &data, &len, 1);
    return (status == 0 && len > 0);
}
