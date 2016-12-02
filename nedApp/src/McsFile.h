/* McsFile.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef nED_MCS_FILE_H
#define nED_MCS_FILE_H

#include <stdint.h>
#include <string>

/**
 * Class to read data from Intel HEX mcs file.
 *
 * Intel HEX is a file format that conveys binary information in ASCII text form.
 * This class supports only multiple of 4 byte data. It allows seamless reading
 * of binary data from mcs file. Rollback functionality is added for resending
 * last packet, since fseek() can not be easily used and partial records (one
 * line) are not supported.
 */
class McsFile {
    public:
        /**
         * Constructor only initializes some internal variables.
         */
        McsFile();

        /**
         * Destructor deallocated buffer.
         */
        ~McsFile();

        /**
         * Import Intel HEX file into object buffer.
         *
         * Read, parse and check the MCS file and save raw data into a buffer.
         * Non-data records are skipped. Record address is ignored. Record
         * checksum is properly checked. Buffer size is automatically adjusted.
         * After successful import, data is stored into m_buffer, m_bufferSize
         * stores current m_buffer size and m_length tells the actual data in
         * the buffer.
         *
         * @param[in] filepath Path to the MCS file
         * @return true on success, false on any error (CRC, length, parse)
         */
        bool import(const std::string &filepath);

        /**
         * Ask for total binary data size in bytes.
         */
        uint32_t getSize();

        /**
         * Read some data from mcs file.
         *
         * Read up to nCount bytes from file and put it into data buffer.
         *
         * @param[out] data User provided buffer where to store data.
         * @param[in] nCount Size of buffer in bytes.
         * @retval Number of bytes read.
         */
        uint32_t read(uint32_t offset, char *data, uint32_t nCount);

    private: /* variables */
        char *m_buffer;         //!< Buffer holding image raw data
        uint32_t m_bufferSize;  //!< Buffer size in bytes
        uint32_t m_length;      //!< Size of binary data in bytes
};

#endif // nED_MCS_FILE_H
