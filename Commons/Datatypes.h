// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <sys/types.h>

// String types
#ifdef __cplusplus
#include <string>
using std::string;
#endif

//
// Integer types
//

// Signed
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef ssize_t            isize;

// Unsigned
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef size_t             usize;


//
// Enumerations
//

/* The following macros 'enum_<type>' provides a way to make enumerations
 * easily accessible in Swift. All macros have two definitions, one for the
 * Swift side and one for the C side. Please note that the type mapping for
 * enum_long differs on both sides. On the Swift side, enums of this type are
 * mapped to type 'long' to make them accessible via the Swift standard type
 * 'Int'. On the C side all enums are mapped to long long. This ensures that
 * all enums have the same size, both on 32-bit and 64-bit architectures.
 */

#if defined(__SWIFT__)

// Definition for Swift
#define enum_open(_name, _type) \
typedef enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

#define enum_long(_name) enum_open(_name, long)
#define enum_u32(_name) enum_open(_name, u32)
#define enum_i8(_name) enum_open(_name, i8)

#else

// Definition for C
#define enum_long(_name) enum _name : long long
#define enum_u32(_name) enum _name : u32
#define enum_i8(_name) enum _name : i8

#endif
