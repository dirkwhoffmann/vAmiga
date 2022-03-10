// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MemUtils.h"
#include "string.h"
#include "IOUtils.h"
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
    // printf("Allocator::dealloc()\n");
    
    assert((size == 0) == (ptr == nullptr));

    if (ptr) {
        
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
        
        size = bytes;
        ptr = new u8[size];
        
    } catch (...) {
        
        size = 0;
        ptr = nullptr;
    }
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
    // Only proceed if the file exists
    if (!util::fileExists(path)) {
        dealloc(); return; // TODO: Throw exception
    }
    
    // Open stream in binary mode
    std::ifstream stream(path, std::ifstream::binary);
    
    if (!stream) {
        dealloc(); return; // TODO: Throw exception
    }
    
    // Assign the buffer the proper size
    auto length = streamLength(stream);
    init(length);

    // Read data from stream
    stream.read((char *)ptr, size);

    if (!stream) {
        dealloc(); return; // TODO: Throw exception
    }
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

bool isZero(const u8 *ptr, usize size)
{
    for (usize i = 0; i < size; i++) {
        if (ptr[i]) return false;
    }
    return true;
}

void replace(u8 *p, isize size, const u8 *sequence, const u8 *substitute)
{
    replace((char *)p, size, (char *)sequence, (char *)substitute);
}

void replace(char *p, isize size, const char *sequence, const char *substitute)
{
    assert(p);
    assert(sequence);
    assert(substitute);
    assert(strlen(sequence) == strlen(substitute));

    auto len = strlen(sequence);
        
    for (isize i = 0; i < size - isize(len); i++) {

        if (strncmp(p + i, sequence, len) == 0) {
            
            memcpy((void *)(p + i), (void *)substitute, len);
            return;
        }
    }
    assert(false);
}

void readAscii(const u8 *buf, isize len, char *result, char pad)
{
    assert(buf);
    assert(result);
    
    for (isize i = 0; i < len; i++) {
        
        result[i] = isprint(int(buf[i])) ? char(buf[i]) : pad;
    }
    result[len] = 0;
}


void hexdump(u8 *p, isize size, isize cols, isize pad)
{
    while (size) {
        
        isize cnt = std::min(size, cols);
        for (isize x = 0; x < cnt; x++) {
            fprintf(stderr, "%02X %s", p[x], ((x + 1) % pad) == 0 ? " " : "");
        }
        
        size -= cnt;
        p += cnt;
        
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void hexdump(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, cols);
}

void hexdumpWords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 2);
}

void hexdumpLongwords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 4);
}

}
