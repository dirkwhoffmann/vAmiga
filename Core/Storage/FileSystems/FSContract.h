// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSObjects.h"

namespace vamiga {

using namespace utl;

struct FSRequire {

    const class FileSystem &fs;

    void inRange(BlockNr nr);
    void file(BlockNr nr);
    void directory(BlockNr nr);
    void fileOrDirectory(BlockNr nr);
    void notRoot(BlockNr nr);
    void emptyDirectory(BlockNr nr);
    void notExist(BlockNr nr, const FSName &name);
};

struct FSEnsure {

    const class FileSystem &fs;

    void inRange(BlockNr nr);
};

}
