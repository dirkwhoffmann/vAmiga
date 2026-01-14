// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystems/CBM/FSObjects.h"

namespace retro::vault::cbm {

using namespace utl;

struct FSRequire {

    const class FileSystem &fs;

    void isFormatted() const;
    void inRange(BlockNr nr) const;
    void file(BlockNr nr) const;
    void directory(BlockNr nr) const;
    void fileOrDirectory(BlockNr nr) const;
    void notRoot(BlockNr nr) const;
    void emptyDirectory() const;
    void notExist(BlockNr nr, const FSName &name) const;
};

struct FSEnsure {

    const class FileSystem &fs;

    void isFormatted() const;
    void inRange(BlockNr nr) const;
};

}
