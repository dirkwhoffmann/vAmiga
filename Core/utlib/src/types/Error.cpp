// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/types/Error.h"

namespace utl {

IOError::IOError(long fault, const std::string &msg) : Error()
{
     switch (fault) {

         case fault::io::DIR_NOT_FOUND:
             set_msg("Folder \"" + msg + "\" not found.");
             break;

         case fault::io::DIR_ACCESS_DENIED:
             set_msg("Unable to access folder \"" + msg + "\". Permission denied.");
             break;
             
         case fault::io::DIR_CANT_CREATE:
             set_msg("Failed to create folder \"" + msg + "\".");
             break;

         case fault::io::DIR_NOT_EMPTY:
             set_msg("Folder \"" + msg + "\" is not empty.");
             break;

         case fault::io::FILE_NOT_FOUND:
             set_msg("File \"" + msg + "\" not empty.");
             break;

         case fault::io::FILE_EXISTS:
             set_msg("File \"" + msg + "\" exists.");
             break;

         case fault::io::FILE_IS_DIRECTORY:
             set_msg("File \"" + msg + "\" is a directory.");
             break;

         case fault::io::FILE_ACCESS_DENIED:
             set_msg("Unable to access file \"" + msg + "\". Permission denied.");
             break;

         case fault::io::FILE_TYPE_MISMATCH:
             set_msg("File \"" + msg + "\" does not match its type.");
             break;

         case fault::io::FILE_TYPE_UNSUPPORTED:
             set_msg("Unsupported file type: \"" + msg + "\".");
             break;

         case fault::io::FILE_CANT_READ:
             set_msg("Failed to read from file \"" + msg + "\".");
             break;

         case fault::io::FILE_CANT_WRITE:
             set_msg("Failed to write to file \"" + msg + "\".");
             break;

         case fault::io::FILE_CANT_CREATE:
             set_msg("Failed to create file \"" + msg + "\".");
             break;

         default:
             set_msg("IOError " + std::to_string(fault) +
                     " (" + std::string(fault::io::IOFaultEnum::key(fault)) + ")");
    }
}

NewParseError::NewParseError(long fault, const std::string &msg) : Error()
{
    switch (fault) {

        case fault::parse::PARSE_BOOL_ERROR:
            set_msg(msg + " must be true or false.");
            break;

        case fault::parse::PARSE_ON_OFF_ERROR:
            set_msg(msg + " must be on or off.");
            break;

        case fault::parse::PARSE_NUM_ERROR:
            set_msg(msg + " is not a number.");
            break;

        case fault::parse::PARSE_ENUM_ERROR:
            set_msg(msg + " is not a valid key.");
            break;

        default:
            set_msg("ParseError " + std::to_string(fault) +
                    " (" + std::string(fault::parse::ParseFaultEnum::key(fault)) + ")");
    }
}

}
