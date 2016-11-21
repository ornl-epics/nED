/* McsFile.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"
#include "McsFile.h"

#include <cassert>
#include <cstring>
#include <cstdio>

#include <algorithm>

#define MCS_ALLOC_UNIT              (1024*1024)

McsFile::McsFile()
    : m_buffer(NULL)
    , m_bufferSize(0)
    , m_length(0)
    , m_read(0)
    , m_confirmed(0)
{
}

McsFile::~McsFile()
{
    if (m_buffer)
        free(m_buffer);
}

bool McsFile::open(const std::string &filepath)
{
    m_read = 0;
    m_confirmed = 0;

    if (m_length != 0)
        return false;

    if (import(filepath) == false)
        return false;

    return true;
}

void McsFile::close()
{
    m_length = 0;
}

bool McsFile::isOpen()
{
    return (m_length > 0);
}

bool McsFile::hasData()
{
    return (m_confirmed < m_length);
}

bool McsFile::read(char *data, uint32_t nCount, uint32_t &nActual) {
    nActual = 0;
    if (!isOpen() || !hasData())
        return false;

    assert(m_confirmed == m_read);

    nActual = std::min(m_length - m_read, nCount);
    memcpy(data, &m_buffer[m_read], nActual);
    m_read += nActual;
    return true;
}

void McsFile::confirmLastData()
{
    m_confirmed = m_read;
}

void McsFile::revertLastData()
{
    m_read = m_confirmed;
}

void McsFile::progress(uint32_t &read, uint32_t &total)
{
    read = m_read;
    total = m_length;
}

bool McsFile::import(const std::string &filepath) {
    bool ret = false;

    // Open the file
    FILE *fp = fopen(filepath.c_str(), "r");
    if (fp == NULL)
        return false;

    m_length = 0;
    while (true) {
        uint32_t count = 0;
        uint32_t address = 0;
        uint32_t record = 0;
        uint32_t chksum1 = 0;
        uint32_t chksum2 = 0;

        // Read in the header
        int n = fscanf(fp, ":%02x%04x%02x", &count, &address, &record);
        if (n != 3) {
            // EOF?
            if (n == -1 && feof(fp)) {
                ret = true;
            }
            break;
        }

        // Allocate more memory, even the first time
        if ((m_bufferSize - m_length) < count) {
            m_bufferSize += MCS_ALLOC_UNIT;
            m_buffer = (char *)realloc(m_buffer, m_bufferSize);
            if (!m_buffer) {
                break;
            }
        }

        // Skip non-data records
        if (record != 0) {
            while (fgetc(fp) != '\n');
            continue;
        }

        // Calculate checksum on header
        chksum1 += count;
        chksum1 +=  address       & 0xFF;
        chksum1 += (address >> 8) & 0xFF;
        chksum1 += record;

        // Copy data to buffer, calculate checksum
        for (uint32_t i = 0; i < count; i++) {
            uint32_t byte;
            if (fscanf(fp, "%02x", &byte) != 1) {
                break;
            }
            byte &= 0xFF;
            chksum1 += byte;
            m_buffer[m_length++] = (char)byte;
        }

        // Checksum to compare against
        if (fscanf(fp, "%02x", &chksum2) != 1) {
            break;
        }

        // Two's complement calculated checksum, compare to saved one
        chksum1 = (0xFFFFFFFF - chksum1 + 1) & 0xFF;
        if (chksum1 != chksum2) {
            break;
        }

        // Eat remaining characters in line
        while (fgetc(fp) != '\n');
    }

    fclose(fp);

    if (ret == false) {
        if (m_buffer != NULL)
            free(m_buffer);
        m_buffer = NULL;
        m_bufferSize = 0;
        m_length = 0;
    }

    return ret;
}
