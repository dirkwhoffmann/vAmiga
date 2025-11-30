// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "STFile.h"

namespace vamiga {

class STFactory {

public:

    static std::unique_ptr<STFile> make(const fs::path &path);
    static std::unique_ptr<STFile> make(const u8 *buf, isize len);
    static std::unique_ptr<STFile> make(Diameter dia, Density den);
    static std::unique_ptr<STFile> make(const class FloppyDisk &disk);
    static std::unique_ptr<STFile> make(const class FloppyDrive &drive);
};

}
