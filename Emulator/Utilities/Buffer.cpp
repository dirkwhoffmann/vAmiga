// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Buffer.h"
#include "IOUtils.h"
#include "MemUtils.h"
#include <fstream>

namespace util {

Allocator::Allocator(u8 *&ptr) : ptr(ptr)
{
    ptr = nullptr;
    size = 0;
}

void
Allocator::dealloc()
{
    assert((size == 0) == (ptr == nullptr));

    if (ptr) {
 
        // printf("Freeing %ld bytes at %p\n", size, (void *)ptr);

        delete [] ptr;
        ptr = nullptr;
        size = 0;
    }
}

void
Allocator::init(isize bytes)
{
    // printf("Allocator::init(%ld)\n", bytes);

    assert(usize(bytes) <= maxCapacity);
    assert((size == 0) == (ptr == nullptr));

    if (size != bytes) try {
        
        dealloc();
        
        if (bytes) {

            size = bytes;
            ptr = new u8[size];
        }
        
    } catch (...) {
        
        size = 0;
        ptr = nullptr;
    }
}

void
Allocator::init(isize bytes, u8 value)
{
    init(bytes);
    if (ptr) memset(ptr, value, size); 
}

void
Allocator::init(const u8 *buf, isize len)
{
    assert(buf);
    
    init(len);
    if (ptr) memcpy((void *)ptr, (const void *)buf, len);
}

void
Allocator::init(const Allocator &other)
{
    init(other.ptr, other.size);
}

void
Allocator::init(const string &path)
{
    // Open stream in binary mode
    std::ifstream stream(path, std::ifstream::binary);
    
    if (stream) {
        
        // Assign the buffer the proper size
        init(streamLength(stream));
        
        // Read data from stream
        stream.read((char *)ptr, size);
        return;
    }
    
    dealloc();
}

void
Allocator::init(const string &path, const string &name)
{
    init(path + "/" + name);
}

void
Allocator::clear(u8 value, isize offset)
{
    assert(offset >= 0 && offset <= size);
    if (ptr) memset((void *)(ptr + offset), value, size - offset);
}

void
Allocator::resize(isize bytes)
{
    // printf("Allocator::resize(%ld)\n", bytes);

    assert((size == 0) == (ptr == nullptr));

    if (size != bytes) {
        
        if (bytes == 0) {
            
            dealloc();
            
        } else try {
            
            auto newPtr = new u8[bytes];
            memcpy(newPtr, ptr, std::min(size, bytes));
            dealloc();
            ptr = (u8 *)newPtr;
            size = bytes;
            
        } catch (...) {
            
            size = 0;
            ptr = nullptr;
        }
    }
}

void
Allocator::resize(isize bytes, u8 value)
{
    assert((size == 0) == (ptr == nullptr));

    auto gap = bytes > size ? bytes - size : 0;
    
    resize(bytes);
    if (ptr) memset((void *)(ptr + size - gap), value, gap);
}

/*
void
Allocator::read(const u8 *buf, isize len)
{
    // printf("Allocator::init(%p,%ld)\n", (void *)buf, len);

    assert((size == 0) == (ptr == nullptr));
    assert(buf);
    
    resize(len);
    if (ptr) memcpy((void *)ptr, (const void *)buf, len);
}
*/

void
Allocator::copy(u8 *buf, isize offset, isize len) const
{
    // printf("Allocator::init(%p,%ld,%ld)\n", (void *)buf, offset, len);

    assert(buf);
    assert((size == 0) == (ptr == nullptr));
    assert(offset + len <= size);
    
    if (ptr) memcpy((void *)buf, (void *)(ptr + offset), len);
}

void
Allocator::patch(const u8 *seq, const u8 *subst)
{
    if (ptr) util::replace(ptr, size, seq, subst);
}

void
Allocator::patch(const char *seq, const char *subst)
{
    if (ptr) util::replace((char *)ptr, size, seq, subst);
}

}
