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

//
// Syntactic sugar
//

/** @brief  Marks a function a throwing.
 *  It is used to mark all methods that use the exception mechanism to signal
 *  error conditions instead of returning error codes. The keyword is used for
 *  documentary purposes only as C++ implicitly marks all functions with
 *  noexcept(false) by default.
 *  @note   It is used in favor of classic throw lists since the latter causes
 *          the compiler to embed unwanted runtime checks in the code.
 */
#define throws noexcept(false)

}
