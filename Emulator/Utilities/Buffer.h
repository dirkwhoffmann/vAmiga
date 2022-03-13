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

template <class T> struct Allocator {

    static constexpr isize maxCapacity = 512 * 1024 * 1024;
    
    T *&ptr;
    isize size;
    
    Allocator(T *&ptr) : ptr(ptr), size(0) { ptr = nullptr; }
    ~Allocator() { dealloc(); }
    void dealloc();
            
    // Queries the buffer state
    isize bytesize() const { return size * sizeof(T); }
    bool empty() const { return size == 0; }
    explicit operator bool() const { return empty(); }

    // Initializers
    void init(isize elements);
    void init(isize elements, T value);
    void init(const T *buf, isize elements);
    void init(const Allocator<T> &other);
    void init(const string &path);
    void init(const string &path, const string &name);

    // Resizes an existing buffer
    void resize(isize elements);
    void resize(isize elements, T pad);

    // Overwrites elements with a default value
    void clear(T value, isize offset, isize len);
    void clear(T value = 0, isize offset = 0) { clear(value, offset, size - offset); }
    
    // Imports or exports the buffer contents
    void copy(T *buf, isize offset, isize len) const;
    void copy(T *buf) const { copy(buf, 0, size); }

    // Replaces a byte or character sequence by another one
    void patch(const u8 *seq, const u8 *subst);
    void patch(const char *seq, const char *subst);

    // Computes a checksum of a certain kind
    u32 fnv32() const { return ptr ? util::fnv32((u8 *)ptr, bytesize()) : 0; }
    u64 fnv64() const { return ptr ? util::fnv64((u8 *)ptr, bytesize()) : 0; }
    u16 crc16() const { return ptr ? util::crc16((u8 *)ptr, bytesize()) : 0; }
    u32 crc32() const { return ptr ? util::crc32((u8 *)ptr, bytesize()) : 0; }
};

struct Buffer : public Allocator <u8> {
    
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
