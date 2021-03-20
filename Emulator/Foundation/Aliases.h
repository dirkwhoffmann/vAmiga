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

//
// Strings
//

#ifdef __cplusplus
#include <string>
using std::string;

#endif


//
// Integer types
//

// Signed integers
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef ssize_t            isize;

// Unsigned integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef size_t             usize;

// Signed alternative for the sizeof keyword
#define isizeof(x) (isize)(sizeof(x))

// Cycles
typedef i64 Cycle;            // Master cycle units
typedef i64 CPUCycle;         // CPU cycle units
typedef i64 CIACycle;         // CIA cycle units
typedef i64 DMACycle;         // DMA cycle units


// Pixels
typedef i16 Pixel;


// Floppy disks
typedef i16 Side;
typedef i16 Cylinder;
typedef i16 Track;
typedef i16 Sector;


// File Systems
typedef u32 Block;


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
#define enum_generic(_name, _type) \
typedef enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

#define enum_long(_name) enum_generic(_name, long)
#define enum_u32(_name) enum_generic(_name, u32)
#define enum_i8(_name) enum_generic(_name, i8)

#else

// Definition for C
#define enum_generic(_name, _type) \
typedef _type _name; \
enum : _type

#define enum_long(_name) enum_generic(_name, long long)
#define enum_u32(_name) enum_generic(_name, u32)
#define enum_i8(_name) enum_generic(_name, i8)

#endif


//
// Optimizing code
//

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)


//
// Code compatibility
//

/* The following macro can be used to disable clang sanitizer checks. It has
 * been added to make the code compatible with gcc which doesn't recognize
 * the 'no_sanitize' keyword.
 */
#if defined(__clang__)
#define NO_SANITIZE(x) __attribute__((no_sanitize(x)))
#else
#define NO_SANITIZE(x)
#endif


//
// Syntactic sugar
//

/* The following keyword is used for documentary purposes only. It is used to
 * mark all methods that use the exception mechanism to signal error conditions
 * instead of returning error codes. It is used in favor of classic throw
 * lists, since the latter cause the compiler to embed unwanted runtime checks
 * in the code.
 */
#define throws
