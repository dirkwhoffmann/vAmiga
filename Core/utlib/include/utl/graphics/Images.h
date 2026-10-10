// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"
#include "GraphicsTypes.h"

namespace utl {

// Writes an image to disk
void exportImage(const fs::path &path, const void *data, isize w, isize h,
                 ImgFormat imgFormat,
                 TexelFormat texelFormat = TexelFormat::ABGR);

}
