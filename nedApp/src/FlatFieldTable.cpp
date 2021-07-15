/* FlatFieldTable.cpp
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "FlatFieldTable.h"

#include <fstream>

FlatFieldTable::FlatFieldTable()
: sizeX(0)
, sizeY(0)
, position_id(0)
, pixel_offset(0)
, type(TYPE_INVALID)
{}

bool FlatFieldTable::import(const std::string &path)
{
    importError.clear();
    std::ifstream infile(path.c_str());

    if (infile.fail()) {
        importError = "Failed to open input file";
        return false;
    }

    if (infile.eof()) {
        importError = "Empty file";
        return false;
    }

    if (parseHeaders(infile, sizeX, sizeY, position_id, pixel_offset, type, path) == false) {
        // error already reported
        return false;
    }

    // TODO: should we enforce some upper limit?
    data.resize(sizeX, std::vector<double>(sizeY, 0.0));

    // Correction factors should follow as doubles, X rows and Y columns.
    // Table size must match the one defined in header.
    for (uint32_t i = 0; i < sizeX; i++) {
        for (uint32_t j = 0; j < sizeY; j++) {
            double value;
            infile >> value;
            data[i][j] = value;

            if (infile.eof()) {
                importError = "Premature end of file";
                type = TYPE_INVALID;
                return false;
            }
        }
    }

    return true;
}

std::string &FlatFieldTable::getImportError()
{
     return importError;
}


bool FlatFieldTable::parseHeader(std::ifstream &infile, std::string &key, std::string &value)
{
    std::string line;

    while (std::getline(infile, line)) {
        // Skip leading blanks
        size_t pos = line.find_first_not_of(' ');

        // Eat empty lines
        if (pos == std::string::npos)
            continue;

        line = line.substr(pos);

        // Put line back when not a header line
        if (line.at(0) != '#') {
            infile.seekg(-1 * (line.length() + 1), std::ifstream::cur);
            break;
        }

        // Skip # and blanks
        pos = line.find_first_not_of(" \t#");

        // Empty comment line
        if (pos == std::string::npos)
            continue;

        line = line.substr(pos);

        // Parse comment line, stripped text before ':' is key, value is after ':'
        pos = line.find_first_of(':');
        if (pos == std::string::npos) {
            key = line;
            value = "";
        } else {
            key = line.substr(0, line.find_last_not_of(" \t", pos));
            pos = line.find_first_not_of(" \t:", pos);
            if (pos == std::string::npos) {
                value = "";
            } else {
                value = line.substr(pos);
            }
        }
        return true;
    }

    return false;
}

bool FlatFieldTable::parseHeaders(
    std::ifstream &infile, uint32_t &size_x, uint32_t &size_y, uint32_t &position_id_,
    uint32_t &pixel_offset_, FlatFieldTable::Type_t &type, const std::string &path)
{
    size_x = 0;
    size_y = 0;
    position_id_ = 0xFFFFFFFF;
    pixel_offset_ = 0xFFFFFFFF;
    type = TYPE_INVALID;
    int version = 0;
    uint32_t nHeaders = 0;

    std::string key, value;

    while (parseHeader(infile, key, value)) {
        if (key == "Format") {
            if (sscanf(value.c_str(), "%d", &version) != 1) {
                importError = "Failed to read file format version";
                return false;
            }
            nHeaders++;
        } else if (key == "Size") {
            if (sscanf(value.c_str(), "%ux%u", &size_x, &size_y) != 2) {
                importError = "Failed to read table size";
                return false;
            }
            nHeaders++;
        } else if (key == "Type") {
            char dim;
            char direction[8];
            if (sscanf(value.c_str(), "Photo Sum %c %5s", &dim, direction) == 2) {
                if (dim == 'x' || dim == 'X') {
                    if (std::string(direction) == "upper") {
                        type = TYPE_X_PS_UP;
                    } else if (std::string(direction) == "lower") {
                        type = TYPE_X_PS_LOW;
                    } else {
                        importError = "Invalid photo sum table type";
                        return false;
                    }
                } else if (dim == 'y' || dim == 'Y') {
                    //LOG_WARN("Y photo sum table not implemented");
                } else {
                    importError = "Unknown photo sum dimension";
                    return false;
                }
            } else if (sscanf(value.c_str(), "FlatField %c", &dim) == 1) {
                if (dim == 'x' || dim == 'X') {
                    type = TYPE_X_CORR;
                } else if (dim == 'y' || dim == 'Y') {
                    type = TYPE_Y_CORR;
                } else {
                    importError = "Failed to parse table type";
                    return false;
                }
            } else {
                importError = "Failed to read table type";
                return false;
            }
            nHeaders++;
        } else if (key == "Position") {
            if (sscanf(value.c_str(), "%u", &position_id_) != 1) {
                importError = "Failed to read position id";
                return false;
            } else if (position_id_ & ~0x7FFFFFFF) {
                importError = "Invalid position id %u";
                return false;
            }
            nHeaders++;
        } else if (key == "Pixel offset") {
            if (sscanf(value.c_str(), "%u", &pixel_offset_) != 1) {
                importError = "Failed to read pixel offset";
                return false;
            } else if (pixel_offset_ & ~0x7FFFFFFF) {
                importError = "Invalid pixel offset %u";
                return false;
            }
            nHeaders++;
        }
    }

    if (nHeaders == 0) {
        importError = "Unknown file format";
        return false;
    }

    // Make sure we got all required headers
    if (version == 0) {
        importError = "Invalid file format, missing version";
        return false;
    }
    if (version != 1) {
        importError = "Invalid file format, unsupported format version";
        return false;
    }
    if (size_x == 0 || size_y == 0) {
        importError = "Invalid file format, missing table size header";
        return false;
    }
    if (type == TYPE_INVALID) {
        importError = "Invalid file format, missing table type header";
        return false;
    }
    if (position_id == 0xFFFFFFFF) {
        importError = "Invalid file format, missing position id header";
        return false;
    }

    return true;
}
