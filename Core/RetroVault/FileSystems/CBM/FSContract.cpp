// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FileSystems/CBM/FSContract.h"
#include "FileSystems/CBM/FileSystem.h"

namespace retro::vault::cbm {

void
FSRequire::isFormatted() const
{
    if (!fs.isFormatted()) {
        throw FSError(FSError::FS_UNFORMATTED);
    }
}

void
FSRequire::inRange(BlockNr nr) const
{
    if (isize(nr) >= fs.getTraits().blocks) {
        throw FSError(FSError::FS_OUT_OF_RANGE);
    }
}

/*
void
FSRequire::file(BlockNr nr) const
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t != FSBlockType::FILEHEADER) {
        throw FSError(FSError::FS_NOT_A_FILE);
    }
}

void
FSRequire::fileOrDirectory(BlockNr nr) const
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t != FSBlockType::ROOT && t != FSBlockType::USERDIR && t != FSBlockType::FILEHEADER) {
        throw FSError(FSError::FS_NOT_A_FILE);
    }
}

void
FSRequire::directory(BlockNr nr) const
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t != FSBlockType::ROOT && t != FSBlockType::USERDIR) {
        throw FSError(FSError::FS_NOT_A_DIRECTORY);
    }
}

void
FSRequire::notRoot(BlockNr nr) const
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t == FSBlockType::ROOT) {
        throw FSError(FSError::FS_INVALID_PATH);
    }
}
*/

void
FSRequire::emptyDirectory() const
{
    if (fs.numItems() != 0) {
        throw FSError(FSError::FS_DIR_NOT_EMPTY);
    }
}

void
FSRequire::exists(const PETName<16> &name) const
{
    if (auto result = fs.searchDir(name); !result.has_value())
        throw FSError(FSError::FS_NOT_FOUND);
}

void
FSRequire::notExist(const PETName<16> &name) const
{
    if (auto result = fs.searchDir(name); result.has_value())
        throw FSError(FSError::FS_EXISTS);
}

void
FSEnsure::isFormatted() const
{
    assert(fs.isFormatted());
}

void
FSEnsure::inRange(BlockNr nr) const
{
    assert(isize(nr) < fs.getTraits().blocks);
}

}
