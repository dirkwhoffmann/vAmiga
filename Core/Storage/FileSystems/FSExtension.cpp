// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSExtension.h"
#include "FileSystem.h"

namespace vamiga {

FSExtension::FSExtension(FileSystem& fs) :

fs(fs),
traits(fs.traits),
storage(fs.storage),
doctor(fs.doctor)

{ }

}
