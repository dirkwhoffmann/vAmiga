// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "RSError.h"

namespace utl {

RSError::RSError(long fault, const std::string &msg) : Error(fault) {

    switch (fault) {

        case GENERIC:
            set_msg(msg);
            break;

        case SYNTAX_ERROR:
            set_msg("Syntax error: " + msg);
            break;

        case TOO_FEW_ARGUMENTS:
            set_msg(msg + ": Too few arguments");
            break;

        case TOO_MANY_ARGUMENTS:
            set_msg(msg + ": Too many arguments");
            break;

        case MISSING_FLAG:
            set_msg("Missing flag: " + msg);
            break;

        case UNKNOWN_FLAG:
            set_msg("Unknown flag: " + msg);
            break;

        case MISSING_KEY_VALUE:
            set_msg("Missing key-value pair: " + msg);
            break;

        case UNKNOWN_KEY_VALUE:
            set_msg("Unknown key-value pair: " + msg);
            break;

        default:
            set_msg("RSError " + std::to_string(fault) + " (" + errstr() + ")");
    }
}

}
