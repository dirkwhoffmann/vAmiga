// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace utl {

struct ScriptInterruption: Exception {

    using Exception::Exception;
};

struct RSError : public Error {

    static constexpr long OK                    = 0;
    static constexpr long GENERIC               = 1;
    static constexpr long SYNTAX_ERROR          = 2;
    static constexpr long TOO_FEW_ARGUMENTS     = 3;
    static constexpr long TOO_MANY_ARGUMENTS    = 4;
    static constexpr long MISSING_FLAG          = 5;
    static constexpr long UNKNOWN_FLAG          = 6;
    static constexpr long MISSING_KEY_VALUE     = 7;
    static constexpr long UNKNOWN_KEY_VALUE     = 8;

    const char *errstr() const noexcept override {

        switch (payload) {

            case OK:                    return "OK";
            case GENERIC:               return "GENERIC";
            case SYNTAX_ERROR:          return "SYNTAX_ERROR";
            case TOO_FEW_ARGUMENTS:     return "TOO_FEW_ARGUMENTS";
            case TOO_MANY_ARGUMENTS:    return "TOO_MANY_ARGUMENTS";
            case MISSING_FLAG:          return "MISSING_FLAG";
            case UNKNOWN_FLAG:          return "UNKNOWN_FLAG";
            case MISSING_KEY_VALUE:     return "MISSING_KEY_VALUE";
            case UNKNOWN_KEY_VALUE:     return "UNKNOWN_KEY_VALUE";

            default:
                return "UNKNOWN";
        }
    }

    explicit RSError(long fault, const std::string &msg = "") : Error(fault) {

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

    explicit RSError(long fault, const char *str) : RSError(fault, string(str)) { }
    explicit RSError(long fault, const fs::path &path) : RSError(fault, path.string()) { }
};

}
