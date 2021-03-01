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

// Shortcuts for being used in combination with the '<<' stream operator
#define DEC std::dec
#define HEX8 std::hex << "0x" << std::setw(2) << std::setfill('0')
#define HEX16 std::hex << "0x" << std::setw(4) << std::setfill('0')
#define HEX32 std::hex << "0x" << std::setw(8) << std::setfill('0')
#define HEX64 std::hex << "0x" << std::setw(16) << std::setfill('0')
#define TAB(x) std::left << std::setw(x)
#define YESNO(x) ((x) ? "yes" : "no")
#define ONOFF(x) ((x) ? "on" : "off")
#define HILO(x) ((x) ? "high" : "low")
#define ISENABLED(x) ((x) ? "enabled" : "disabled")
#define ISSET(x) ((x) ? "set" : "not set")
#define EMULATED(x) ((x) ? "emulated" : "not emulated")
#define DUMP(x) std::setw(24) << std::right << std::setfill(' ') << (x) << " : "

string lowercased(const string& s);
string uppercased(const string& s);
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

/*
#define enum_long(_name) enum _name : long long
#define enum_u32(_name) enum _name : u32
#define enum_i8(_name) enum _name : i8
*/

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
