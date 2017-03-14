/* Common.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef NED_COMMON_HPP
#define NED_COMMON_HPP

#include <string>
#include <vector>

#define ALIGN_UP(number, boundary)      (((number) + (boundary) - 1) & ~((boundary) - 1))
#define HEX_BYTE_TO_DEC(a)              ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

namespace Common {

/**
 * Determine file size.
 *
 * @param[in] path Relative or absolute path to regular file.
 * @param[out] size Calculated size.
 * @retval true on success
 * @retval false on failure, possibly file doesn't exist.
 */
bool fileSize(const std::string &path, uint32_t &size);

/**
 * Check if path is directory.
 *
 * @param[in] path Relative or absolute path to regular file.
 * @return true when path is directory, false otherwise or when path does not exist.
 */
bool isDir(const std::string &path);

/**
 * Split a string and return a vector of tokens.
 *
 * String is split by delim character. Empty tokens are included in the output.
 * @param[in] string to be split
 * @param[in] delim character to be used
 * @return the new elements vector
 */
std::vector<std::string> split(const std::string &s, char delim);

/**
 * Split string and modify vector in place.
 *
 * @param[in] string to be split
 * @param[in] delim character to be used
 * @param[out] elements vector
 * @return the elements vector for ease of use
 */
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

}; // namespace Common


#endif // NED_COMMON_HPP
