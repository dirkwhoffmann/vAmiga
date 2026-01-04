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
#include "utl/abilities/Loggable.h"

namespace retro::amigafs {

class FileSystem;

class FSService : public Loggable {

public:

    FileSystem &fs;
    const FSTraits &traits;

    explicit FSService(FileSystem& fs);
};

}
