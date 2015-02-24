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

#define ALIGN_UP(number, boundary)      (((number) + (boundary) - 1) & ~((boundary) - 1))

/**
 * Determine file size.
 *
 * @param[in] path Relative or absolute path to regular file.
 * @param[out] size Calculated size.
 * @retval true on success
 * @retval false on failure, possibly file doesn't exist.
 */
bool fileSize(const std::string &path, uint32_t &size);

#endif // NED_COMMON_HPP
