// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADZEncoder.h"
#include "ADFEncoder.h"
#include "Error.h"

namespace vamiga {

void
ADZEncoder::encode(const ADZFile &adz, FloppyDisk &disk)
{
    ADFEncoder::encode(adz.adf, disk);
}

}
