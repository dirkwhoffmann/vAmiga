// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/abilities/Reflectable.h"

namespace vamiga {

using namespace utl;

namespace fault {

enum : long
{
    FS_OK,
    FS_UNKNOWN,

    // General
    FS_OUT_OF_RANGE,
    FS_INVALID_PATH,
    FS_INVALID_REGEX,
    FS_NOT_A_DIRECTORY,
    FS_NOT_A_FILE,
    FS_NOT_A_FILE_OR_DIRECTORY,
    FS_NOT_FOUND,
    FS_EXISTS,
    FS_CANNOT_OPEN,

    FS_UNFORMATTED,
    FS_UNSUPPORTED,
    FS_READ_ONLY,
    FS_WRONG_BSIZE,
    FS_WRONG_CAPACITY,
    FS_WRONG_DOS_TYPE,
    FS_WRONG_BLOCK_TYPE,
    FS_HAS_CYCLES,
    FS_CORRUPTED,

    // Import
    FS_OUT_OF_SPACE,

    // Export error
    FS_DIR_NOT_EMPTY,
    FS_CANNOT_CREATE_DIR,
    FS_CANNOT_CREATE_FILE
};

struct FSFaultEnum : Reflectable<FSFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FS_CANNOT_CREATE_FILE;

    static const char *_key(long value)
    {
        switch (value) {

            case FS_OK:                       return "OK";
            case FS_UNKNOWN:                  return "UNKNOWN";

            case FS_OUT_OF_RANGE:             return "OUT_OF_RANGE";
            case FS_INVALID_PATH:             return "INVALID_PATH";
            case FS_INVALID_REGEX:            return "INVALID_REGEX";
            case FS_NOT_A_DIRECTORY:          return "NOT_A_DIRECTORY";
            case FS_NOT_A_FILE:               return "NOT_A_FILE";
            case FS_NOT_A_FILE_OR_DIRECTORY:  return "NOT_A_FILE_OR_DIRECTORY";
            case FS_NOT_FOUND:                return "NOT_FOUND";
            case FS_EXISTS:                   return "EXISTS";
            case FS_CANNOT_OPEN:              return "CANNOT_OPEN";

            case FS_UNFORMATTED:              return "UNFORMATTED";
            case FS_UNSUPPORTED:              return "UNSUPPORTED";
            case FS_READ_ONLY:                return "READ_ONLY";
            case FS_WRONG_BSIZE:              return "WRONG_BSIZE";
            case FS_WRONG_CAPACITY:           return "WRONG_CAPACITY";
            case FS_WRONG_DOS_TYPE:           return "WRONG_DOS_TYPE";
            case FS_WRONG_BLOCK_TYPE:         return "WRONG_BLOCK_TYPE";
            case FS_HAS_CYCLES:               return "HAS_CYCLES";
            case FS_CORRUPTED:                return "CORRUPTED";

            case FS_OUT_OF_SPACE:             return "OUT_OF_SPACE";

            case FS_DIR_NOT_EMPTY:            return "DIR_NOT_EMPTY";
            case FS_CANNOT_CREATE_DIR:        return "CANNOT_CREATE_DIR";
            case FS_CANNOT_CREATE_FILE:       return "CANNOT_CREATE_FILE";
        }
        return "???";
    }

    static const char *help(long value)
    {
        return "";
    }
};

}

using FSFault = long;

class FSError : public utl::Error {

public:

    FSError(long fault, const std::string &s = "");
    FSError(long fault, const std::filesystem::path &p) : FSError(fault, p.string()) { };
    FSError(long fault, std::integral auto v) : FSError(fault, std::to_string(v)) { };
};

}
