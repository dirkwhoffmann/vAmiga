// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <sys/types.h>
#include <stdint.h>

//
// Booleans
//

#ifndef __cplusplus
#include <stdbool.h>
#endif


//
// Strings
//

#ifdef __cplusplus
#include <string>
#include <cstring>
using std::string;
#endif


//
// Filesystem
//

#ifdef __cplusplus
#include <filesystem>
namespace vamiga { namespace fs = std::filesystem; }
#endif


//
// Integers
//

// Signed integers
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef signed long        isize;

// Unsigned integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef unsigned long      usize;


//
// Optionals
//

#ifdef __cplusplus
#include <optional>
using std::optional;
#endif


//
// Enumerations
//

/* The following macros 'enum_<type>' provide a way to make enumerations
 * easily accessible in Swift. All macros have two definitions, one for the
 * Swift side and one for the C side. Please note that the type mapping for
 * enum_long differs on both sides. On the Swift side, enums of this type are
 * mapped to 'long enums' to make them accessible via the Swift standard type
 * 'Int'. On the C side all enums are mapped to 'enum-less longs' to make them
 * easily serializable.
 */

#if defined(__SWIFT__)

// Definition for Swift
#define enum_generic(_name, _type) \
typedef enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

#define enum_long(_name) enum_generic(_name, long)
#define enum_i8(_name) enum_generic(_name, i8)

#else

// Definition for C
#define enum_generic(_name, _type) \
typedef _type _name; \
enum : _type

#define enum_long(_name) enum_generic(_name, long)
#define enum_i8(_name) enum_generic(_name, i8)

#endif
