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
#include <filesystem>

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
    AppError(Fault f, const string &s);
    AppError(Fault f, const char *s) : AppError(f, string(s)) { };
    AppError(Fault f, const fs::path &p) : AppError(f, p.string()) { };
    AppError(Fault f, std::integral auto v) : AppError(f, std::to_string(v)) { };
    AppError(Fault f) : AppError(f, "") { }
    
    const char *what() const throw() override;
    Fault fault() const { return Fault(data); }
};

}
