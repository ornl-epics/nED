/* Common.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include <fstream>

bool fileSize(const std::string &path, uint32_t &size)
{
    std::ifstream file(path, std::ifstream::in);

    if (file.fail() == false) {
        file.seekg(0, file.end);
        size = file.tellg();
        return true;
    }

    return false;
}