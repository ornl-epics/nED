/* Common.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "Common.h"

#include <fstream>
#include <sstream>

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

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
