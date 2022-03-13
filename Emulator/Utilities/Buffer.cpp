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

template <class T> void
Allocator<T>::dealloc()
{
    assert((size == 0) == (ptr == nullptr));

    if (ptr) {
 
        delete [] ptr;
        ptr = nullptr;
        size = 0;
    }
}

template <class T> void
Allocator<T>::init(isize elements)
{
    assert(usize(elements) <= maxCapacity);
    assert((size == 0) == (ptr == nullptr));

    if (size != elements) try {
        
        dealloc();
        
        if (elements) {

            size = elements;
            ptr = new T[size];
        }
        
    } catch (...) {
        
        size = 0;
        ptr = nullptr;
    }
}

template <class T> void
Allocator<T>::init(isize elements, T value)
{
    init(elements);
    
    if (ptr) {
        
        for (isize i = 0; i < size; i++) {
            ptr[i] = value;
        }
    }
}

template <class T> void
Allocator<T>::init(const T *buf, isize elements)
{
    assert(buf);
    
    init(elements);
    
    if (ptr) {
        
        for (isize i = 0; i < size; i++) {
            ptr[i] = buf[i];
        }
    }
}

template <class T> void
Allocator<T>::init(const Allocator<T> &other)
{
    init(other.ptr, other.size);
}

template <class T> void
Allocator<T>::init(const string &path)
{
    // Open stream in binary mode
    std::ifstream stream(path, std::ifstream::binary);
    
    // Return an empty buffer if the stream could not be opened
    if (!stream) { dealloc(); return; }
        
    // Get the stream length in bytes
    auto length = streamLength(stream);
    
    // Create a buffer of proper size
    init(length / sizeof(T) + (length % sizeof(T) ? 1 : 0));
    
    // Read from stream
    stream.read((char *)ptr, length);
}

template <class T> void
Allocator<T>::init(const string &path, const string &name)
{
    init(path + "/" + name);
}

template <class T> void
Allocator<T>::resize(isize elements)
{
    assert((size == 0) == (ptr == nullptr));

    if (size != elements) {
        
        if (elements == 0) {
            
            dealloc();
            
        } else try {
            
            auto newPtr = new T[elements];
            copy(newPtr, 0, std::min(size, elements));
            dealloc();
            ptr = (u8 *)newPtr;
            size = elements;
            
        } catch (...) {
            
            size = 0;
            ptr = nullptr;
        }
    }
}

template <class T> void
Allocator<T>::resize(isize elements, T pad)
{
    auto gap = elements > size ? elements - size : 0;
    
    resize(elements);
    clear(pad, elements - gap, gap);
}

template <class T> void
Allocator<T>::clear(T value, isize offset, isize len)
{
    assert((size == 0) == (ptr == nullptr));
    assert(offset >= 0 && len >= 0 && offset + len <= size);
    
    if (ptr) {
        
        for (isize i = 0; i < len; i++) {
            ptr[i + offset] = value;
        }
    }
}

template <class T> void
Allocator<T>::copy(T *buf, isize offset, isize len) const
{
    assert(buf);
    assert((size == 0) == (ptr == nullptr));
    assert(offset >= 0 && len >= 0 && offset + len <= size);
    
    if (ptr) {
        
        for (isize i = 0; i < len; i++) {
            buf[i] = ptr[i + offset];
        }
    }
}

template <class T> void
Allocator<T>::patch(const u8 *seq, const u8 *subst)
{
    if (ptr) util::replace(ptr, bytesize(), seq, subst);
}

template <class T> void
Allocator<T>::patch(const char *seq, const char *subst)
{
    if (ptr) util::replace((char *)ptr, bytesize(), seq, subst);
}

//
// Template instantiations
//

template void Allocator<u8>::init(isize bytes);
template void Allocator<u8>::init(isize bytes, u8 value);
template void Allocator<u8>::init(const u8 *buf, isize len);
template void Allocator<u8>::init(const Allocator<u8> &other);
template void Allocator<u8>::init(const string &path);
template void Allocator<u8>::init(const string &path, const string &name);
template void Allocator<u8>::resize(isize elements);
template void Allocator<u8>::resize(isize elements, u8 value);
template void Allocator<u8>::clear(u8 value, isize offset);
template void Allocator<u8>::copy(u8 *buf, isize offset, isize len) const;
template void Allocator<u8>::patch(const u8 *seq, const u8 *subst);
template void Allocator<u8>::patch(const char *seq, const char *subst);
}
