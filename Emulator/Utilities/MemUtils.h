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
#include "Checksum.h"
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

//
// Memory management
//

struct Allocator {

    static constexpr isize maxCapacity = 512 * 1024 * 1024;
    
    u8 *&ptr;
    isize size;
    
    Allocator(u8 *&ptr);
    ~Allocator() { dealloc(); }

    void dealloc();
            
    // Allocates or releases memory
    void init(isize bytes);
    void init(const u8 *buf, isize len);
    void init(const Allocator &other);
    void init(const string &path);
    void init(const string &path, const string &name);

    // Resizes an existing buffer
    void resize(isize bytes);
    void resize(isize bytes, u8 value);

    // Queries the buffer state
    bool empty() const { return size == 0; }

    // Overwrites all elements with a default value
    void clear(u8 value = 0, isize offset = 0);
    
    // Imports or exports the buffer contents
    void copy(u8 *buf, isize offset, isize len) const;
    void copy(u8 *buf) const { copy(buf, 0, size); }

    // Replaces a byte or character sequence by another one
    void patch(const u8 *seq, const u8 *subst);
    void patch(const char *seq, const char *subst);

    // Computes a checksum of a certain kind
    u32 fnv32() const { return ptr ? util::fnv32(ptr, size) : 0; }
    u64 fnv64() const { return ptr ? util::fnv64(ptr, size) : 0; }
    u16 crc16() const { return ptr ? util::crc16(ptr, size) : 0; }
    u32 crc32() const { return ptr ? util::crc32(ptr, size) : 0; }
};

struct Buffer {
    
    u8 *ptr;
    Allocator allocator = Allocator(ptr);

    void dealloc() { allocator.dealloc(); }
    void init(isize bytes) { allocator.init(bytes); }
    void init(const u8 *buf, isize len) { allocator.init(buf, len); }
    void init(const Buffer &other) { allocator.init(other.allocator); }
    void init(const string &path) { allocator.init(path); }
    void init(const string &path, const string &name) { allocator.init(path, name); }
    void resize(isize bytes) { allocator.resize(bytes); }
    void resize(isize bytes, u8 value) { allocator.resize(bytes, value); }
    bool empty() const { return allocator.empty(); }
    void clear(u8 value = 0, isize offset = 0) { allocator.clear(value, offset); }
    void copy(u8 *buf, isize offset, isize len) const { allocator.copy(buf, offset, len); }
    void copy(u8 *buf) { allocator.copy(buf); } const
    void patch(const u8 *seq, const u8 *subst) { allocator.patch(seq, subst); }
    void patch(const char *seq, const char *subst) { allocator.patch(seq, subst); }
    
    u32 fnv32() const { return allocator.fnv32(); }
    u64 fnv64() const { return allocator.fnv64(); }
    u16 crc16() const { return allocator.crc16(); }
    u32 crc32() const { return allocator.crc32(); }
    
    isize size() const { return allocator.size; }
    
    u8 operator [] (isize i) const { return ptr[i]; }
    u8 &operator [] (isize i) { return ptr[i]; }
};


//
// Byte order
//

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

//
// Memory content
//

// Checks if a certain memory area is all zero
bool isZero(const u8 *ptr, usize size);

// Replaces the first occurence of a byte or character sequence by another one
void replace(u8 *p, isize size, const u8 *sequence, const u8 *substitute);
void replace(char *p, isize size, const char *sequence, const char *substitute);

// Extracts all readable ASCII characters from a buffer
void readAscii(const u8 *buf, isize len, char *result, char fill = '.');

// Prints a hex dump of a buffer to the console
void hexdump(u8 *p, isize size, isize cols, isize pad);
void hexdump(u8 *p, isize size, isize cols = 32);
void hexdumpWords(u8 *p, isize size, isize cols = 32);
void hexdumpLongwords(u8 *p, isize size, isize cols = 32);

}
