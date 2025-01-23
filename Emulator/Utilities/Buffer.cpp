// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Buffer.h"
#include "IOUtils.h"
#include "MemUtils.h"
#include "Chrono.h"
#include <fstream>

namespace vamiga::util {

template <class T> Allocator<T>&
Allocator<T>::operator= (const Allocator<T>& other)
{
    // Reallocate buffer if needed
    if (size != other.size) alloc(other.size);
    assert(size == other.size);
    
    // Copy buffer
    if (size) memcpy(ptr, other.ptr, size);
    return *this;
}

template <class T> void
Allocator<T>::alloc(isize elements)
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
Allocator<T>::init(isize elements, T value)
{
    alloc(elements);
    
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
    
    alloc(elements);
    
    if (ptr) {
        
        for (isize i = 0; i < size; i++) {
            ptr[i] = buf[i];
        }
    }
}

template <class T> void
Allocator<T>::init(const string &str)
{
    init((const T *)str.c_str(), isize(str.length() / sizeof(T)));
}

template <class T> void
Allocator<T>::init(const Allocator<T> &other)
{
    init(other.ptr, other.size);
}

template <class T> void
Allocator<T>::init(const std::vector<T> &vector)
{
    isize vecsize = isize(vector.size());

    alloc(vecsize);
    for (isize i = 0; i < vecsize; i++) ptr[i] = vector[i];
}

template <class T> void
Allocator<T>::init(const std::filesystem::path &path)
{
    // Open stream in binary mode
    std::ifstream stream(path, std::ifstream::binary);
    
    // Return an empty buffer if the stream could not be opened
    if (!stream) { dealloc(); return; }
    
    // Read file contents into a string stream
    std::ostringstream sstr(std::ios::binary);
    sstr << stream.rdbuf();

    // Call the proper init delegate
    init(sstr.str());
}

template <class T> void
Allocator<T>::init(const std::filesystem::path &path, const string &name)
{
    init(path / name);
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
            ptr = (T *)newPtr;
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
    if (ptr) util::replace((u8 *)ptr, bytesize(), seq, subst);
}

template <class T> void
Allocator<T>::patch(const char *seq, const char *subst)
{
    if (ptr) util::replace((char *)ptr, bytesize(), seq, subst);
}

template <class T> void
Allocator<T>::compress(isize n, isize offset, isize compressedSize)
{
    const auto maxChunkSize = isize(std::numeric_limits<T>::max());
    
    T prev = 0;
    isize repetitions = 0;
 
    // Make a copy of the original buffer
    Buffer<T> buf(ptr, size);
    
    // Resize the buffer to the target size
    resize(compressedSize);
    
    isize j = std::min(offset, size);
    auto encode = [&](T element, isize count) {
        
        for (isize k = 0; k < std::min(count, n); k++) ptr[j++] = element;
        if (count >= n) ptr[j++] = T(count - n);
    };
    
    // Perform run-length encoding
    for (isize i = offset; i < buf.size; i++) {
        
        if (buf[i] == prev && repetitions < maxChunkSize) {
            
            repetitions++;
            
        } else {
            
            encode(prev, repetitions);
            prev = buf[i];
            repetitions = 1;
        }
    }
    encode(prev, repetitions);
}

template <class T> isize
Allocator<T>::compressedSize(isize n, isize offset)
{
    const auto maxChunkSize = isize(std::numeric_limits<T>::max());

    T prev = 0;
    isize repetitions = 0;
    isize count = std::min(offset, size);
    
    for (isize i = offset; i < size; i++) {
        
        if (ptr[i] == prev && repetitions < maxChunkSize) {
            
            repetitions++;
            
        } else {
            
            count += std::min(repetitions, n) + (repetitions >= n ? 1 : 0);
            prev = ptr[i];
            repetitions = 1;
        }
    }
    return count + std::min(repetitions, n) + (repetitions >= n ? 1 : 0);
}

template <class T> void
Allocator<T>::compress_old(isize n, isize offset)
{
    T prev = 0;
    isize repetitions = 0;
    std::vector<T> vec;
    vec.reserve(size);
    
    auto encode = [&](T element, isize count) {
        
        for (isize i = 0; i < std::min(count, n); i++) vec.push_back(element);
        if (count >= n) vec.push_back(T(count - n));
    };
    
    // Skip everything up to the offset position
    for (isize i = 0; i < std::min(offset, size); i++) vec.push_back(ptr[i]);
    
    // Perform run-length encoding
    auto maxChunkSize = isize(std::numeric_limits<T>::max());
    for (isize i = offset; i < size; i++) {
        
        if (ptr[i] == prev && repetitions < maxChunkSize) {
            
            repetitions++;
            
        } else {
            
            encode(prev, repetitions);
            prev = ptr[i];
            repetitions = 1;
        }
    }
    encode(prev, repetitions);
    
    // Replace old data
    init(vec);
}

template <class T> void
Allocator<T>::uncompress(isize n, isize offset, isize uncompressedSize)
{
    T prev = 0;
    isize repetitions = 0;
    
    // Make a copy of the original buffer
    Buffer<T> compressed(ptr, size);
    
    // Resize the buffer to the target size
    resize(uncompressedSize);
    
    // Copy elements one by one up to the offset position
    for (isize k = 0; k < std::min(offset, compressed.size); k++) ptr[k] = compressed[k];
    
    // Decode the rest
    for (isize i = offset, j = offset; i < compressed.size; i++) {

        ptr[j++] = compressed[i];
        repetitions = prev != compressed[i] ? 1 : repetitions + 1;
        prev = compressed[i];
        
        // 'n' matching symbols in a row indicate that a run-length follows
        if (repetitions == n && i < compressed.size - 1) {
            
            auto missing = isize(compressed[++i]);
            for (isize k = 0; k < missing; k++) ptr[j++] = prev;
            repetitions = 0;
        }
    }
}

template <class T> isize
Allocator<T>::uncompressedSize(isize n, isize offset)
{
    T prev = 0;
    isize repetitions = 0;
    isize count = std::min(offset, size);
    
    for (isize i = offset; i < size; i++) {
        
        count++;
        repetitions = prev != ptr[i] ? 1 : repetitions + 1;
        prev = ptr[i];
        
        if (repetitions == n && i < size - 1) {
            
            count += isize(ptr[++i]);
            repetitions = 0;
        }
    }
    return count;
}

template <class T> void
Allocator<T>::uncompress_old(isize n, isize offset, isize expectedSize)
{
    T prev = 0;
    isize repetitions = 0;
    
    std::vector<T> vec;
    
    { util::StopWatch watch("OLD CODE");
        
        prev = 0;
        repetitions = 0;
        
        
        // Speed up by starting with a big enough container
        if (expectedSize) vec.reserve(expectedSize);
        
        auto decode_old = [&](T element, isize count) {
            
            for (isize i = 0; i < count; i++) vec.push_back(element);
        };
        
        // Skip everything up to the offset position
        for (isize i = 0; i < std::min(offset, size); i++) vec.push_back(ptr[i]);
        
        for (isize i = offset; i < size; i++) {
            
            vec.push_back(ptr[i]);
            repetitions = prev != ptr[i] ? 1 : repetitions + 1;
            prev = ptr[i];
            
            if (repetitions == n && i < size - 1) {
                
                decode_old(prev, isize(ptr[++i]));
                repetitions = 0;
            }
        }
    }
    
    // Replace old data
    init(vec);
}


//
// Template instantiations
//

#define INSTANTIATE_ALLOCATOR(T) \
template Allocator<T>& Allocator<T>::operator=(const Allocator<T>& other); \
template void Allocator<T>::alloc(isize bytes); \
template void Allocator<T>::dealloc(); \
template void Allocator<T>::init(isize bytes, T value); \
template void Allocator<T>::init(const T *buf, isize len); \
template void Allocator<T>::init(const Allocator<T> &other); \
template void Allocator<T>::init(const std::filesystem::path &path); \
template void Allocator<T>::init(const std::filesystem::path &path, const string &name); \
template void Allocator<T>::resize(isize elements); \
template void Allocator<T>::resize(isize elements, T value); \
template void Allocator<T>::clear(T value, isize offset, isize len); \
template void Allocator<T>::copy(T *buf, isize offset, isize len) const; \
template void Allocator<T>::patch(const u8 *seq, const u8 *subst); \
template void Allocator<T>::patch(const char *seq, const char *subst); \
template void Allocator<T>::compress(isize, isize); \
template void Allocator<T>::compress_old(isize, isize); \
template isize Allocator<T>::compressedSize(isize, isize); \
template void Allocator<T>::uncompress(isize, isize, isize); \
template isize Allocator<T>::uncompressedSize(isize, isize); \
template void Allocator<T>::uncompress_old(isize, isize, isize);

INSTANTIATE_ALLOCATOR(u8)
INSTANTIATE_ALLOCATOR(u32)
INSTANTIATE_ALLOCATOR(u64)
INSTANTIATE_ALLOCATOR(isize)
INSTANTIATE_ALLOCATOR(float)
INSTANTIATE_ALLOCATOR(bool)

}
