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

}
