// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types/UtlError.h"
#include "utl/abilities/Reflectable.h"
#include <system_error>

using namespace utl::abilities;

namespace utl {

namespace fault {

enum : long
{
    // File/path issues
    IO_ENOENT          = long(std::errc::no_such_file_or_directory),
    IO_EEXIST          = long(std::errc::file_exists),
    IO_ENOTDIR         = long(std::errc::not_a_directory),
    IO_EISDIR          = long(std::errc::is_a_directory),

    // Permissions
    IO_EACCES          = long(std::errc::permission_denied),
    IO_EPERM           = long(std::errc::operation_not_permitted),

    // Disk / I/O errors
    IO_ENOSPC          = long(std::errc::no_space_on_device),
    IO_EFBIG           = long(std::errc::file_too_large),
    IO_EIO             = long(std::errc::io_error),

    // Seeking
    IO_ESPIPE          = long(std::errc::invalid_seek),

    // Resource issues
    IO_EAGAIN          = long(std::errc::resource_unavailable_try_again),

    // Encoding
    IO_EILSEQ          = long(std::errc::illegal_byte_sequence)
};

struct IOFaultEnum : Reflectable<IOFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(IO_EILSEQ);

    static const char *_key(long value) {

        switch (value) {

                // File/path issues
            case IO_ENOENT:          return "IO_ENOENT";
            case IO_EEXIST:          return "IO_EEXIST";
            case IO_ENOTDIR:         return "IO_ENOTDIR";
            case IO_EISDIR:          return "IO_EISDIR";

                // Permissions
            case IO_EACCES:          return "IO_EACCES";
            case IO_EPERM:           return "IO_EPERM";

                // Disk / I/O errors
            case IO_ENOSPC:          return "IO_ENOSPC";
            case IO_EFBIG:           return "IO_EFBIG";
            case IO_EIO:             return "IO_EIO";

                // Seeking
            case IO_ESPIPE:          return "IO_ESPIPE";

                // Resource issues
            case IO_EAGAIN:          return "IO_EAGAIN";

                // Encoding
            case IO_EILSEQ:          return "IO_EILSEQ";
        }
        return "???";
    }

    static const char *help(long value) {

        return "";
    }
};

enum : long
{
    PARSE_UNKNOWN,
    PARSE_BOOL_ERROR,
    PARSE_ON_OFF_ERROR,
    PARSE_NUM_ERROR,
    PARSE_ENUM_ERROR
};

struct ParseFaultEnum : Reflectable<ParseFaultEnum, long>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(fault::PARSE_ENUM_ERROR);

    static const char *_key(long value)
    {
        switch (value) {

            case fault::PARSE_UNKNOWN:       return "PARSE_UNKNOWN";
            case fault::PARSE_BOOL_ERROR:    return "PARSE_BOOL_ERROR";
            case fault::PARSE_ON_OFF_ERROR:  return "PARSE_ON_OFF_ERROR";
            case fault::PARSE_NUM_ERROR:     return "PARSE_NUM_ERROR";
            case fault::PARSE_ENUM_ERROR:    return "PARSE_ENUM_ERROR";
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
};

struct NewParseError : public Error {

    NewParseError(long d, const std::string &s);
};

}
