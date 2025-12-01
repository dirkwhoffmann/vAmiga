// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DMSFile.h"

namespace vamiga {

class DMSFactory {

public:

    static std::unique_ptr<DMSFile> make(const fs::path &path);
    static std::unique_ptr<DMSFile> make(const u8 *buf, isize len);
};

}
