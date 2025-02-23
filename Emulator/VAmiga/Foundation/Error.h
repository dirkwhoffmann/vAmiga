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

struct CoreException : public std::exception {
    
    // Payload
    i64 data;

    // Auxiliary information about the thrown errow
    string description;
    
    CoreException(i64 d, const string &s) : data(d), description(s) { }
    CoreException(i64 d) : data(d), description("") { }
    CoreException(const string &s) : data(0), description(s) { }
    CoreException() : data(0) { }
    
    const char *what() const noexcept override { return description.c_str(); }
};

struct CoreError : public CoreException
{
    CoreError(Fault f, const string &s);
    CoreError(Fault f, const char *s) : CoreError(f, string(s)) { };
    CoreError(Fault f, const fs::path &p) : CoreError(f, p.string()) { };
    CoreError(Fault f, std::integral auto v) : CoreError(f, std::to_string(v)) { };
    CoreError(Fault f) : CoreError(f, "") { }
    
    const char *what() const throw() override;
    Fault fault() const { return Fault(data); }
};

}
