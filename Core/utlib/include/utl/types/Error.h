// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types/Error.h"
#include "utl/abilities/Reflectable.h"
#include <system_error>

using namespace utl::abilities;

namespace utl {

namespace fault::io {

enum : long
{
    DIR_NOT_FOUND,        ///< Directory does not exist
    DIR_ACCESS_DENIED,    ///< File access denied
    DIR_CANT_CREATE,      ///< Unable to create a directory
    DIR_NOT_EMPTY,        ///< Directory is not empty
    FILE_NOT_FOUND,       ///< File not found error
    FILE_EXISTS,          ///< File already exists
    FILE_IS_DIRECTORY,    ///< The file is a directory
    FILE_ACCESS_DENIED,   ///< File access denied
    FILE_TYPE_MISMATCH,   ///< File type mismatch
    FILE_TYPE_UNSUPPORTED,///< Unsupported file type
    FILE_CANT_READ,       ///< Can't read from file
    FILE_CANT_WRITE,      ///< Can't write to file
    FILE_CANT_CREATE,     ///< Can't create file
};

struct IOFaultEnum : Reflectable<IOFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FILE_CANT_CREATE);

    static const char *_key(long value) {

        switch (value) {

            case DIR_NOT_FOUND:               return "DIR_NOT_FOUND";
            case DIR_ACCESS_DENIED:           return "DIR_ACCESS_DENIED";
            case DIR_CANT_CREATE:             return "DIR_CANT_CREATE";
            case DIR_NOT_EMPTY:               return "DIR_NOT_EMPTY";
            case FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case FILE_EXISTS:                 return "FILE_EXISTS";
            case FILE_IS_DIRECTORY:           return "FILE_IS_DIRECtORY";
            case FILE_ACCESS_DENIED:          return "FILE_ACCESS_DENIED";
            case FILE_TYPE_MISMATCH:          return "FILE_TYPE_MISMATCH";
            case FILE_TYPE_UNSUPPORTED:       return "FILE_TYPE_UNSUPPORTED";
            case FILE_CANT_READ:              return "FILE_CANT_READ";
            case FILE_CANT_WRITE:             return "FILE_CANT_WRITE";
            case FILE_CANT_CREATE:            return "FILE_CANT_CREATE";
        }
        return "???";
    }
};

}

namespace fault::parse {

enum : long
{
    PARSE_BOOL_ERROR,
    PARSE_ON_OFF_ERROR,
    PARSE_NUM_ERROR,
    PARSE_ENUM_ERROR
};

struct ParseFaultEnum : Reflectable<ParseFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = PARSE_ENUM_ERROR;

    static const char *_key(long value)
    {
        switch (value) {

            case PARSE_BOOL_ERROR:    return "PARSE_BOOL_ERROR";
            case PARSE_ON_OFF_ERROR:  return "PARSE_ON_OFF_ERROR";
            case PARSE_NUM_ERROR:     return "PARSE_NUM_ERROR";
            case PARSE_ENUM_ERROR:    return "PARSE_ENUM_ERROR";
        }
        return "???";
    }

    static const char *help(long value)
    {
        return "";
    }
};

}

class Error : public GenericException<long> {

public:

    Error(long d = 0, const std::string &s = "") : GenericException<long>(d, s) { }
    long fault() const { return _payload; }
};

struct IOError : public Error {

    IOError(long d, const std::string &s);
    IOError(long d, const std::filesystem::path &p) : IOError(d, p.string()) { }
};

struct NewParseError : public Error {

    NewParseError(long d, const std::string &s);
    NewParseError(long d, const std::filesystem::path &p) : NewParseError(d, p.string()) { }
};

}
