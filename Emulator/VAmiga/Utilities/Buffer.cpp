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
Allocator<T>::compress(isize n, isize offset)
{
    /* This function performs a run-length encoding according to the following
     * scheme:
     *
     * k < n:   AA ... AA B   ->   AA ... AA B
     *          <-- k -->          <-- k -->
     *
     * k >= n:  AA ... AA B   ->   AA ... AA [max] ... [max] [l] B
     *          <-- k -->          <-- n --> <--- sum = k-n --->
     *
     *                             with max = std::numeric_limits<T>::max()
     *
     * Note: For k = n, the second case implies:
     *
     *          AA ... AA B   ->   AA ... AA [0] B
     *          <-- n -->          <-- n -->
     *
     * In this case, compression has a negative effect. Examples:
     *
     *      ABBCCCDDDDEEEEE   ->   ABB0CC1DD2EE3
     *         AABBCCDDEEFF   ->   AA0BB0CC0DD0EE0FF0
     */

    const auto max = isize(std::numeric_limits<T>::max());
    T prev = 0;
    isize repetitions = 0;

    std::vector<T> vec;
    vec.reserve(size);

    auto encode = [&](T element, isize count) {
        
        // Encode up to n symbols in plain text
        for (isize k = 0; k < std::min(n, count); k++) vec.push_back(element);
        
        count -= n;
        
        // Append the run length sequence
        while (count >= 0) {
            
            auto runlength = std::min(count, max);
            vec.push_back(T(runlength));
            count -= runlength;
            
            if (runlength != max) break;
        }
    };

    // Skip everything up to the offset position
    vec.insert(vec.end(), ptr, ptr + std::min(offset, size));
    
    // Perform run-length encoding
    for (isize i = offset; i < size; i++) {

        if (ptr[i] == prev) {

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
Allocator<T>::uncompress(isize n, isize offset, isize expectedSize)
{
    const auto max = isize(std::numeric_limits<T>::max());
    T prev = 0;
    isize repetitions = 0;
    std::vector<T> vec;

    // Speed up by starting with a big enough container
    if (expectedSize) vec.reserve(expectedSize);

    // Skip everything up to the offset position
    vec.insert(vec.end(), ptr, ptr + std::min(offset, size));
    
    for (isize i = offset; i < size; i++) {

        vec.push_back(ptr[i]);
        repetitions = prev != ptr[i] ? 1 : repetitions + 1;
        prev = ptr[i];

        if (repetitions == n) {

            while (i < size - 1) {
                
                auto runlength = isize(ptr[++i]);
                vec.insert(vec.end(), runlength, prev);
                if (runlength != max) break;
            }
            repetitions = 0;
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
template void Allocator<T>::uncompress(isize, isize, isize);

INSTANTIATE_ALLOCATOR(u8)
INSTANTIATE_ALLOCATOR(u32)
INSTANTIATE_ALLOCATOR(u64)
INSTANTIATE_ALLOCATOR(isize)
INSTANTIATE_ALLOCATOR(float)
INSTANTIATE_ALLOCATOR(bool)

}
