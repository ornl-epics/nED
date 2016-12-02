/* FlatFieldTable.h
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef FLAT_FIELD_TABLE_H
#define FLAT_FIELD_TABLE_H

#include <cinttypes>
#include <string>
#include <vector>

struct FlatFieldTable {
    public: // variables
        typedef enum {
            TYPE_INVALID,
            TYPE_X_CORR,
            TYPE_Y_CORR,
            TYPE_X_PS_LOW,
            TYPE_X_PS_UP,
        } Type_t;

        uint32_t sizeX;
        uint32_t sizeY;
        uint32_t position_id;
        uint32_t pixel_offset;
        Type_t type;

        std::vector< std::vector<double> > data;

    private: // variables
        std::string importError;
    public:
        FlatFieldTable();

        /**
         * Parse the flat-field file and populate the tables.
         *
         * All correction tables are single ASCII file. Correction table file
         * starts with mandatory header and optional comments. Header and
         * comment lines start with character '#'. Header lines are key,value
         * tupples separated by ':'. Unknown header lines are treated as
         * comment. Valid header keys are:
         * - Format
         * - Size
         * - Type
         * - Position
         *
         * ASCII file has several advantages over binary file. It's cross
         * platform independent, it's easy to read and debug, it can be saved
         * to version control repository more efficiently due to compression.
         *
         * Example file:
         * @code {.txt}
         * # FlatField correction table: X correction
         * # File created on 2015-02-09 09:28:06
         * # Format version: 1
         * # Size: 3x3
         * # Table dimension: X
         * # Position id: 17
         *
         * -0.12 +6.81 +3.55
         * +0.44 +5.09 +12.11
         * +0.79 -9.10 -3.61
         * @endcode
         *
         * @param[in] path Relative or absolute path to a file to parse.
         * @param[out] tableSize Both table dimensions must match this one,
         *                       initialized to size_x if 0.
         * @return true on success
         */
        bool import(const std::string &path);

        std::string &getImportError();

    private:

        /**
         * Parse single header from the current file position.
         *
         * @param[in] infile Opened file
         * @param[out] key Text up to the first ':' character in line
         * @param[out] value Text after the first ':' character in line
         * @return true when parsed, false otherwise.
         */
        bool parseHeader(std::ifstream &infile, std::string &key, std::string &value);

        /**
         * Parse all headers from the current file position.
         *
         * Process all lines starting with # character and tries to parse them
         * to obtain required fields. Blank lines are truncated. Header line
         * can be of any. Function succeeds only if it finds all required
         * headers and parses them. Required header lines:
         * - Format version: <ver>
         * - Table size: <X>x<Y>
         * - Table type: Correction <x or y>
         * - Position id: <id>
         *
         * @param[in] infile Opened file to parse headers from.
         * @param[out] size_x X dimension size
         * @param[out] size_y Y dimension size
         * @param[out] position_id Camera position id
         * @param[out] pixel_offset Pixel id offset
         * @param[out] type Type of imported table
         * @param[in] path File path name used for error reporting
         * @retval MAP_ERR_BAD_FORMAT Unrecognized file format
         * @retval MAP_ERR_PARSE File seems to be expected format but parse error.
         * @retval MAP_ERR_NONE Header parsed.
         */
        bool parseHeaders(std::ifstream &infile, uint32_t &size_x, uint32_t &size_y, uint32_t &position_id_, uint32_t &pixel_offset_, Type_t &type, const std::string &path);
};

#endif // FLAT_FIELD_TABLE_H
