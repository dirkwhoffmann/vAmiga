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

using namespace utl::abilities;

enum class FSFault : long
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

    FS_UNINITIALIZED,
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

struct FSFaultEnum : Reflectable<FSFaultEnum, FSFault>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FSFault::FS_CANNOT_CREATE_FILE);

    static const char *_key(FSFault value)
    {
        switch (value) {

            case FSFault::FS_OK:                       return "OK";
            case FSFault::FS_UNKNOWN:                  return "UNKNOWN";

            case FSFault::FS_OUT_OF_RANGE:             return "OUT_OF_RANGE";
            case FSFault::FS_INVALID_PATH:             return "INVALID_PATH";
            case FSFault::FS_INVALID_REGEX:            return "INVALID_REGEX";
            case FSFault::FS_NOT_A_DIRECTORY:          return "NOT_A_DIRECTORY";
            case FSFault::FS_NOT_A_FILE:               return "NOT_A_FILE";
            case FSFault::FS_NOT_A_FILE_OR_DIRECTORY:  return "NOT_A_FILE_OR_DIRECTORY";
            case FSFault::FS_NOT_FOUND:                return "NOT_FOUND";
            case FSFault::FS_EXISTS:                   return "EXISTS";
            case FSFault::FS_CANNOT_OPEN:              return "CANNOT_OPEN";

            case FSFault::FS_UNINITIALIZED:            return "UNINITIALIZED";
            case FSFault::FS_UNFORMATTED:              return "UNFORMATTED";
            case FSFault::FS_UNSUPPORTED:              return "UNSUPPORTED";
            case FSFault::FS_READ_ONLY:                return "READ_ONLY";
            case FSFault::FS_WRONG_BSIZE:              return "WRONG_BSIZE";
            case FSFault::FS_WRONG_CAPACITY:           return "WRONG_CAPACITY";
            case FSFault::FS_WRONG_DOS_TYPE:           return "WRONG_DOS_TYPE";
            case FSFault::FS_WRONG_BLOCK_TYPE:         return "WRONG_BLOCK_TYPE";
            case FSFault::FS_HAS_CYCLES:               return "HAS_CYCLES";
            case FSFault::FS_CORRUPTED:                return "CORRUPTED";

            case FSFault::FS_OUT_OF_SPACE:             return "OUT_OF_SPACE";

            case FSFault::FS_DIR_NOT_EMPTY:            return "DIR_NOT_EMPTY";
            case FSFault::FS_CANNOT_CREATE_DIR:        return "CANNOT_CREATE_DIR";
            case FSFault::FS_CANNOT_CREATE_FILE:       return "CANNOT_CREATE_FILE";
        }
        return "???";
    }

    static const char *help(FSFault value)
    {
        return "";
    }
};

class FSError : public utl::Exception
{
public:

    FSError(FSFault fault, const std::string &s);
    FSError(FSFault fault, const char *s) : FSError(fault, std::string(s)) { };
    FSError(FSFault fault, const std::filesystem::path &p) : FSError(fault, p.string()) { };
    FSError(FSFault fault, std::integral auto v) : FSError(fault, std::to_string(v)) { };
    FSError(FSFault fault) : FSError(fault, "") { }

    FSFault fault() const { return *payload<FSFault>(); }
};

}
