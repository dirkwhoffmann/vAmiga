// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types.h"

namespace utl {

class Compressible {

public:
    
    static void gzip(u8 *buffer, isize len, std::vector<u8> &result);
    static void gunzip(u8 *buffer, isize len, std::vector<u8> &result, isize sizeEstimate = 0);

    static void lz4(u8 *buffer, isize len, std::vector<u8> &result);
    static void unlz4(u8 *buffer, isize len, std::vector<u8> &result, isize sizeEstimate = 0);

    static void rle2(u8 *buffer, isize len, std::vector<u8> &result);
    static void unrle2(u8 *buffer, isize len, std::vector<u8> &result, isize sizeEstimate = 0);

    static void rle3(u8 *buffer, isize len, std::vector<u8> &result);
    static void unrle3(u8 *buffer, isize len, std::vector<u8> &result, isize sizeEstimate = 0);

    static void rle(isize n, u8 *buffer, isize len, std::vector<u8> &result, isize sizeEstimate = 0);
    static void unrle(isize n, u8 *buffer, isize len, std::vector<u8> &result, isize sizeEstimate = 0);
};

}
