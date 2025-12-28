// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DMSEncoder.h"
#include "DMSFile.h"
#include "DiskEncoder.h"

namespace vamiga {

void
DMSEncoder::encode(const class DMSFile &source, FloppyDisk &target)
{
    DiskEncoder::encode(source.getADF(), target);
}

}
