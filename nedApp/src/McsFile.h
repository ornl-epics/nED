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
         * Opens the file identified by argument filepath.
         *
         * Opens and verifies firmware image Intel HEX mcs file. It returns
         * false on any error detected, like CRC, short file, parsing error.
         * It detects firmware image size which can be queried with progress()
         * function.
         * Internally the object reads entire MCS file into a buffer when opened.
         * Afterwards it reads directly from the buffer.
         *
         * @param[in] filepath Path to mcs file.
         */
        bool open(const std::string &filepath);

        /**
         * Closes the file currently associated with the object.
         *
         * It closes the file and resets all internal variables. Reading more
         * data at this point is not possible.
         */
        void close();

        /**
         * Returns whether the object is currently associated to a file.
         *
         * Returns true after succesfully opened file until it has been
         * explicitly closed.
         */
        bool isOpen();

        /**
         * Return true if there is no more data in file.
         */
        bool hasData();

        /**
         * Read some data from mcs file.
         *
         * Read up to nCount bytes from file and put it into data buffer.
         * Keep track of how many data has been read the last time, so that it
         * can be reverted.
         *
         * @param[out] data User provided buffer where to store data.
         * @param[in] nCount Size of buffer in bytes.
         * @param[out] nActual Number of bytes read.
         * @retval true on success
         * @retval false on file reading error, not in progress or no more data
         */
        bool read(char *data, uint32_t nCount, uint32_t &nActual);

        /**
         * Last data was accepted by remote party.
         */
        void confirmLastData();

        /**
         * Last data needs to be re-sent.
         */
        void revertLastData();

        /**
         * Get total firmware image length and current progress in bytes.
         */
        void progress(uint32_t &read, uint32_t &total);

    private: /* variables */
        char *m_buffer;         //!< Buffer holding image raw data
        uint32_t m_bufferSize;  //!< Buffer size in bytes
        uint32_t m_length;      //!< Number of dwords of data in mcs file
        uint32_t m_read;        //!< Number of dwords already read
        uint32_t m_confirmed;   //!< Number of dwords confirmed by remote party

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
};

#endif // nED_MCS_FILE_H
