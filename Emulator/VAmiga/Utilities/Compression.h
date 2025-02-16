// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmigaConfig.h"
#include "BasicTypes.h"

namespace vamiga::util {

void rle(u8 *uncompressed, isize len, std::vector<u8> &result, isize n = 2);
void unrle(u8 *compressed, isize len, std::vector<u8> &result, isize n = 2);

void gzip(u8 *uncompressed, isize len, std::vector<u8> &result);
void gunzip(u8 *compressed, isize len, std::vector<u8> &result, isize sizeEstimate = 0);

void lz4(u8 *uncompressed, isize len, std::vector<u8> &result);
void unlz4(u8 *compressed, isize len, std::vector<u8> &result, isize sizeEstimate = 0);
}
