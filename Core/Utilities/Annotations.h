// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

//
// Compiler directives
//

#if defined(__clang__)
    #define alwaysinline __attribute__((always_inline))
#elif defined(__GNUC__) || defined(__GNUG__)
    #define alwaysinline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define alwaysinline __forceinline
#elif
    #define alwaysinline inline
#endif


//
// Syntactic sugar
//

/* This macro marks a function a throwing.
 * It is used to mark all methods that use the exception mechanism to signal
 * error conditions instead of returning error codes. The keyword is used for
 * documentary purposes only as C++ implicitly marks all functions with
 * noexcept(false) by default.
 */
#define throws noexcept(false)
