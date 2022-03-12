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

namespace util {

struct Allocator {

    static constexpr isize maxCapacity = 512 * 1024 * 1024;
    
    u8 *&ptr;
    isize size;
    
    Allocator(u8 *&ptr);
    ~Allocator() { dealloc(); }

    void dealloc();
            
    // Allocates or releases memory
    void init(isize bytes);
    void init(isize bytes, u8 value);
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
    
    // Operator overloads
    explicit operator bool() const { return ptr != nullptr; }
};

struct Buffer : public Allocator {
    
    u8 *ptr = nullptr;
    
    Buffer() : Allocator(ptr) { };
    
    Buffer(isize bytes) : Allocator(ptr) { init(bytes); }
    Buffer(isize bytes, u8 value) : Allocator(ptr) { init(bytes, value); }
    Buffer(const u8 *buf, isize len) : Allocator(ptr) { init(buf, len); }
    Buffer(const string &path) : Allocator(ptr) { init(path); }
    Buffer(const string &path, const string &name) : Allocator(ptr) { init(path, name); }
    
    u8 operator [] (isize i) const { return ptr[i]; }
    u8 &operator [] (isize i) { return ptr[i]; }
};

}
