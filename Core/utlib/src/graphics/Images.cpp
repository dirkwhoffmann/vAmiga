// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/graphics/Images.h"
#include "utl/graphics/Colors.h"
#include <bit>
#include <vector>
#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

#pragma GCC diagnostic pop

namespace utl {

namespace {

// The texel format whose bytes appear as R,G,B,A in memory
constexpr TexelFormat nativeFormat =
    std::endian::native == std::endian::little ? TexelFormat::ABGR : TexelFormat::RGBA;

constexpr int jpgQuality = 90;

// Repacks a buffer of texels from format F into the native format
template <TexelFormat F> void
convert(const u32 *src, u32 *dst, size_t count)
{
    for (size_t i = 0; i < count; i++) {

        GpuColor<F> c(src[i]);
        dst[i] = GpuColor<nativeFormat>(c.r(), c.g(), c.b(), c.a()).rawValue;
    }
}

void
append(void *context, void *data, int size)
{
    auto *out = static_cast<std::vector<u8> *>(context);
    auto *bytes = static_cast<const u8 *>(data);
    out->insert(out->end(), bytes, bytes + size);
}

}

void
exportImage(const fs::path &path, const void *data, isize w, isize h,
            ImgFormat imgFormat, TexelFormat texelFormat)
{
    if (!data || w <= 0 || h <= 0)
        throw Error(0, "exportImage: invalid image data or dimensions");

    auto count = size_t(w) * size_t(h);
    auto *pixels = static_cast<const u32 *>(data);

    // Convert into a temporary buffer if the texels aren't in stb's layout
    std::vector<u32> tmp;
    if (texelFormat != nativeFormat) {

        tmp.resize(count);
        switch (texelFormat) {

            case TexelFormat::ABGR: convert<TexelFormat::ABGR>(pixels, tmp.data(), count); break;
            case TexelFormat::ARGB: convert<TexelFormat::ARGB>(pixels, tmp.data(), count); break;
            case TexelFormat::RGBA: convert<TexelFormat::RGBA>(pixels, tmp.data(), count); break;
        }
        pixels = tmp.data();
    }

    // Encode into memory and write the file ourselves. This keeps non-ASCII
    // paths working, which stb's fopen-based writers don't guarantee.
    std::vector<u8> out;
    int ok = 0;
    int ww = int(w), hh = int(h);

    switch (imgFormat) {

        case ImgFormat::PNG:
            ok = stbi_write_png_to_func(append, &out, ww, hh, 4, pixels, ww * 4);
            break;
        case ImgFormat::BMP:
            ok = stbi_write_bmp_to_func(append, &out, ww, hh, 4, pixels);
            break;
        case ImgFormat::TGA:
            ok = stbi_write_tga_to_func(append, &out, ww, hh, 4, pixels);
            break;
        case ImgFormat::JPG:
            ok = stbi_write_jpg_to_func(append, &out, ww, hh, 4, pixels, jpgQuality);
            break;
    }
    if (!ok || out.empty())
        throw Error(0, "exportImage: image encoding failed");

    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char *>(out.data()), std::streamsize(out.size()));
    if (!file)
        throw Error(0, "exportImage: can't write " + path.string());
}

}
