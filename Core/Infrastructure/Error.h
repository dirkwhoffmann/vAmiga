// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ErrorTypes.h"

namespace vamiga {

struct AppException : public std::exception {
    
    // Payload
    i64 data;

    // Auxiliary information about the thrown errow
    string description;
    
    AppException(i64 d, const string &s) : data(d), description(s) { }
    AppException(i64 d) : data(d), description("") { }
    AppException(const string &s) : data(0), description(s) { }
    AppException() : data(0) { }
    
    const char *what() const noexcept override { return description.c_str(); }
};

struct AppError : public AppException
{
    AppError(Fault fault, const string &s);
    AppError(Fault fault, const char *s) : AppError(fault, string(s)) { };
    AppError(Fault fault, const fs::path &p) : AppError(fault, p.string()) { };
    AppError(Fault fault, std::integral auto v) : AppError(fault, std::to_string(v)) { };
    AppError(Fault fault) : AppError(fault, "") { }

    Fault fault() const { return Fault(data); }
    const char *what() const throw() override;
};

}
