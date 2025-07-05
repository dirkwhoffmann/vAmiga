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
#include "Checksum.h"
#include "Macros.h"
#include <bit>
#include <functional>
#include <cstdint>
#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace vamiga::util {

// Reverses the byte ordering in an integer value
#ifdef _MSC_VER
#define SWAP16 _byteswap_ushort
#define SWAP32 _byteswap_ulong
#define SWAP64 _byteswap_uint64
#else
#define SWAP16  __builtin_bswap16
#define SWAP32  __builtin_bswap32
#define SWAP64  __builtin_bswap64
#endif

//
// Byte order
//

// Returns the big endian representation of an integer value
template<typename T> T bigEndian(T x);

template<>
inline u16 bigEndian(u16 x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return SWAP16(x);
    }
}

template<>
inline u32 bigEndian(u32 x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return SWAP32(x);
    }
}

template<>
inline u64 bigEndian(u64 x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return SWAP64(x);
    }
}

//
// Bit counting
//

#ifdef _MSC_VER
inline isize popcount(u32 x) { return isize(__popcnt(u32(x))); }
#else
inline isize popcount(u32 x) { return isize(__builtin_popcount(u32(x))); }
#endif


//
// Memory content
//

// Reads a value in big-endian format
#define R8BE(a)  (*(u8 *)(a))
#define R16BE(a) HI_LO(*(u8 *)(a), *((u8 *)(a)+1))
#define R32BE(a) HI_HI_LO_LO(*(u8 *)(a), *((u8 *)(a)+1), *((u8 *)(a)+2), *((u8 *)(a)+3))

// Writes a value in big-endian format
#define W8BE(a,v)  { *(u8 *)(a) = (v); }
#define W16BE(a,v) { *(u8 *)(a) = HI_BYTE(v); *((u8 *)(a)+1) = LO_BYTE(v); }
#define W32BE(a,v) { W16BE(a,HI_WORD(v)); W16BE((a)+2,LO_WORD(v)); }

// Checks if a certain memory area is all zero
bool isZero(const u8 *ptr, isize size);

// Replaces the first occurence of a byte or character sequence by another one
void replace(u8 *p, isize size, const u8 *sequence, const u8 *substitute);
void replace(char *p, isize size, const char *sequence, const char *substitute);

// Extracts all readable ASCII characters from a buffer
void readAscii(const u8 *buf, isize len, char *result, char fill = '.');

// Prints a hex dump of a buffer to the console (DEPRECATED)
void hexdump(u8 *p, isize size, isize cols, isize pad);
void hexdump(u8 *p, isize size, isize cols = 32);
void hexdumpWords(u8 *p, isize size, isize cols = 32);
void hexdumpLongwords(u8 *p, isize size, isize cols = 32);

// Dumps memory data in customizable formats
struct DumpOpt
{
    // const char *fmt;
    isize base;
    isize size;
    isize prefix;
    isize columns;
    isize lines;
    bool tail;
    bool nr;
    bool offset;
    bool ascii;
};
void dump(std::ostream &os, const DumpOpt &opt, std::function<isize(isize,isize)>);
void dump(std::ostream &os, const DumpOpt &opt, std::function<isize(isize,isize)>, const char *fmt);
void dump(std::ostream &os, const DumpOpt &opt, u8 *buf, isize len);
void dump(std::ostream &os, const DumpOpt &opt, u8 *buf, isize len, const char *fmt);
 
}
