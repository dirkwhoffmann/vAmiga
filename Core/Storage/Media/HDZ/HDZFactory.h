// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HDZFile.h"

namespace vamiga {

class HDZFactory {

public:

    static std::unique_ptr<HDZFile> make(const fs::path &path);
    static std::unique_ptr<HDZFile> make(const u8 *buf, isize len);
    static std::unique_ptr<HDZFile> make(const class HDFFile &hdf);
    static std::unique_ptr<HDZFile> make(const class HardDrive &hd);
    // static std::unique_ptr<HDZFile> make(const std::unique_ptr<HDFFile>& adf);
};

}
