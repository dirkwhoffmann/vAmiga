// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include <bit>

namespace util {

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

// Returns the big endian representation of an integer value
template<typename T> T bigEndian(T x);

template<>
inline uint16_t bigEndian(uint16_t x)
 {
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return SWAP16(x);
    }
}

template<>
inline uint32_t bigEndian(uint32_t x)
 {
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return SWAP32(x);
    }
}

template<>
inline uint64_t bigEndian(uint64_t x)
{
    if constexpr (std::endian::native == std::endian::big) {
        return x;
    } else {
        return SWAP64(x);
    }
}

// Checks if a certain memory area is all zero
bool isZero(const u8 *ptr, usize size);

// Prints a hex dump of a buffer to the console
void hexdump(u8 *p, isize size, isize cols, isize pad);
void hexdump(u8 *p, isize size, isize cols = 32);
void hexdumpWords(u8 *p, isize size, isize cols = 32);
void hexdumpLongwords(u8 *p, isize size, isize cols = 32);

}
