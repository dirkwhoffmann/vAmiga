// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/types/UtlError.h"

namespace utl {

void
IOError::init(std::any payload, std::string msg)
{
    auto fault = std::any_cast<long>(payload);

     switch (fault)
     {
         case fault::IO_ENOENT:
             set_msg(msg + " does not exist.");
             break;

         case fault::IO_EEXIST:
             set_msg(msg + " already exists.");
             break;

         case fault::IO_ENOTDIR:
             set_msg(msg + " is not a directory.");
             break;

         case fault::IO_EISDIR:
             set_msg(msg + " is a directory.");
             break;

         case fault::IO_EACCES:
             set_msg("Permission denied: " + msg);
             break;

         case fault::IO_EPERM:
             set_msg("Operation not permitted: " + msg);
             break;

         case fault::IO_ENOSPC:
             set_msg("No space left on device while accessing " + msg);
             break;

         case fault::IO_EFBIG:
             set_msg("File too large: " + msg);
             break;

         case fault::IO_EIO:
             set_msg("I/O error while accessing " + msg);
             break;

         case fault::IO_ESPIPE:
             set_msg("Invalid seek on: " + msg);
             break;

         case fault::IO_EAGAIN:
             set_msg("Resource temporarily unavailable: " + msg);
             break;

         case fault::IO_EILSEQ:
             set_msg("Illegal byte sequence in: " + msg);
             break;

         default:
             set_msg("IOError " + std::to_string(fault) +
                     " (" + std::string(fault::IOFaultEnum::key(fault)) + ")");
    }
}

void
NewParseError::init(std::any payload, std::string msg)
{
    auto fault = std::any_cast<long>(payload);

    switch (fault)
    {
        case fault::PARSE_BOOL_ERROR:
            set_msg(msg + " must be true or false.");
            break;

        case fault::PARSE_ON_OFF_ERROR:
            set_msg(msg + " must be on or off.");
            break;

        case fault::PARSE_NUM_ERROR:
            set_msg(msg + " is not a number.");
            break;

        case fault::PARSE_ENUM_ERROR:
            set_msg(msg + " is not a valid key.");
            break;

        default:
            set_msg("ParseError " + std::to_string(fault) +
                    " (" + std::string(fault::IOFaultEnum::key(fault)) + ")");
    }
}

}
