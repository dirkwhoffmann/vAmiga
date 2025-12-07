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

namespace fault {

enum : long
{
    IO_CANT_READ,
    IO_CANT_WRITE,
    IO_CANT_CREATE
};

struct IOFaultEnum : Reflectable<IOFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(IO_CANT_CREATE);

    static const char *_key(long value) {

        switch (value) {

            case IO_CANT_READ:     return "IO_CANT_READ";
            case IO_CANT_WRITE:    return "IO_CANT_WRITE";
            case IO_CANT_CREATE:   return "IO_CANT_CREATE";
        }
        return "???";
    }
};

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
