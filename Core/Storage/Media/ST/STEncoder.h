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
#include "FloppyDisk.h"

namespace vamiga {

class STEncoder {

public:

    static void encode(const STFile &source, FloppyDisk &target);
    static void decode(STFile &target, const FloppyDisk &source);
};

}
