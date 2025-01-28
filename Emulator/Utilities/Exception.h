// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"
#include <exception>

namespace vamiga::util {

/** @brief  Base class for all emulator exceptions
 */
struct Exception : public std::exception {
    
    //! @brief Error code
    i64 data;

    //! @brief Auxiliary information about the thrown errow
    string description;
    
    Exception(i64 d, const string &s) : data(d), description(s) { }
    Exception(i64 d) : data(d), description("") { }
    Exception(const string &s) : data(0), description(s) { }
    Exception() : data(0) { }
    
    const char *what() const noexcept override { return description.c_str(); }
};

}
