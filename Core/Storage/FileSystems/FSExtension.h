// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "Loggable.h"

namespace vamiga {

class FSExtension : public Loggable {

public:

    class FileSystem &fs;
    const FSTraits &traits;
    class FSStorage &storage;
    class FSDoctor &doctor;

    explicit FSExtension(FileSystem& fs);
};

}
