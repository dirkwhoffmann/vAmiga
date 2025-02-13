// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Compression.h"

#ifdef USE_ZLIB
#include <zlib.h>
#endif

namespace vamiga::util {

void
rle(u8 *uncompressed, isize len, std::vector<u8> &result, isize n)
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

    const auto max = isize(std::numeric_limits<u8>::max());
    u8 prev = 0;
    isize repetitions = 0;

    // For speedup: Start with a container with a decent capacity
    if (result.capacity() == 0) result.reserve(len);

    auto encode = [&](u8 element, isize count) {
        
        // Encode up to n symbols in plain text
        for (isize k = 0; k < std::min(n, count); k++) result.push_back(element);
        
        count -= n;
        
        // Append the run length sequence
        while (count >= 0) {
            
            auto runlength = std::min(count, max);
            result.push_back(u8(runlength));
            count -= runlength;
            
            if (runlength != max) break;
        }
    };
    
    // Perform run-length encoding
    for (isize i = 0; i < len; i++) {

        if (uncompressed[i] == prev) {

            repetitions++;

        } else {

            encode(prev, repetitions);
            prev = uncompressed[i];
            repetitions = 1;
        }
    }
    encode(prev, repetitions);
}

void
unrle(u8 *uncompressed, isize len, std::vector<u8> &result, isize n)
{
    const auto max = isize(std::numeric_limits<u8>::max());
    u8 prev = 0;
    isize repetitions = 0;

    // For speedup: Start with a container with a decent capacity
    if (result.capacity() == 0) result.reserve(2 * len);
    
    for (isize i = 0; i < len; i++) {

        result.push_back(uncompressed[i]);
        repetitions = prev != uncompressed[i] ? 1 : repetitions + 1;
        prev = uncompressed[i];

        if (repetitions == n) {

            while (i < len - 1) {
                
                auto runlength = isize(uncompressed[++i]);
                result.insert(result.end(), runlength, prev);
                if (runlength != max) break;
            }
            repetitions = 0;
        }
    }
}

#ifdef USE_ZLIB

void
gzip(u8 *uncompressed, isize len, std::vector<u8> &result)
{
    // Simulate an error if requested
    if (FORCE_ZLIB_ERROR) throw std::runtime_error("Forced zlib error.");
    
    // Only proceed if there is anything to zip
    if (len == 0) return;
    
    // Remember the initial length of the result vector
    auto initialLen = result.size();
    
    // Resize the target buffer
    result.resize(initialLen + len + len / 2 + 256);
    
    // Configure the zlib stream
    z_stream zs {
        
        .next_in   = (Bytef *)uncompressed,
        .avail_in  = (uInt)len,
        .next_out  = (Bytef *)(result.data()) + initialLen,
        .avail_out = (uInt)result.size(),
    };
    
    // Select the gzip format by choosing adequate window bits
    constexpr int windowBits = MAX_WBITS | 16;

    // Initialize the zlib stream
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib.");
    }
    
    // Run the zlib decompressor
    if (auto ret = deflate(&zs, Z_FINISH); ret != Z_STREAM_END) {
        
        deflateEnd(&zs);
        throw std::runtime_error("Zlib error " + std::to_string(ret));
    }
    
    // Reduce the target buffer to the correct size
    result.resize(initialLen + zs.total_out);
    deflateEnd(&zs);
}

void
gunzip(u8 *compressed, isize len, std::vector<u8> &result, isize sizeEstimate)
{
    // Simulate an error if requested
    if (FORCE_ZLIB_ERROR) throw std::runtime_error("Forced zlib error.");
    
    // Only proceed if there is anything to unzip
    if (len == 0) return;
    
    // Remember the initial length of the result vector
    auto initialLen = result.size();
    
    // Configure the zlib stream
    z_stream zs {
        
        .next_in   = (Bytef *)compressed,
        .avail_in  = (uInt)len,
    };
    
    // Select the gzip format by choosing adequate window bits
    constexpr int windowBits = MAX_WBITS | 16;
    
    // Initialize the zlib stream
    if (inflateInit2(&zs, windowBits) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib.");
    }
    
    // For speedup: Estimate the size and reserve elements
    result.reserve(initialLen + (sizeEstimate ? sizeEstimate : 2 * len));

    // Decompress in smaller chunks
    std::vector<uint8_t> buffer(8192); int ret;
    
    do {
        
        zs.next_out = buffer.data();
        zs.avail_out = static_cast<uInt>(buffer.size());
        
        switch (ret = inflate(&zs, Z_NO_FLUSH)) {
                
            case Z_ERRNO:
            case Z_STREAM_ERROR:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
            case Z_BUF_ERROR:
            case Z_VERSION_ERROR:
                
                inflateEnd(&zs);
                throw std::runtime_error("Zlib error " + std::to_string(ret));
                
            default:
                break;
        }
        
        auto count = buffer.size() - zs.avail_out;
        result.insert(result.end(), buffer.begin(), buffer.begin() + count);
        
    } while (ret != Z_STREAM_END);
    
    inflateEnd(&zs);
}

#else

void gzip(u8 *uncompressed, isize len, std::vector<u8> &result) {
    throw std::runtime_error("No zlib support.");
}
void gunzip(u8 *compressed, isize len, std::vector<u8> &result, isize sizeEstimate) {
    throw std::runtime_error("No zlib support.");
}

#endif

}
