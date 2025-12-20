// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSContract.h"
#include "FileSystem.h"

namespace vamiga {

void
FSRequire::inRange(BlockNr nr)
{
    if (nr >= fs.getTraits().blocks) {
        throw FSError(FSError::FS_OUT_OF_RANGE);
    }
}

void
FSRequire::file(BlockNr nr)
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t != FSBlockType::FILEHEADER) {
        throw FSError(FSError::FS_NOT_A_FILE);
    }
}

void
FSRequire::fileOrDirectory(BlockNr nr)
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t != FSBlockType::ROOT && t != FSBlockType::USERDIR && t != FSBlockType::FILEHEADER) {
        throw FSError(FSError::FS_NOT_A_FILE);
    }
}

void
FSRequire::directory(BlockNr nr)
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t != FSBlockType::ROOT && t != FSBlockType::USERDIR) {
        throw FSError(FSError::FS_NOT_A_DIRECTORY);
    }
}

void
FSRequire::notRoot(BlockNr nr)
{
    inRange(nr);
    auto t = fs.typeOf(nr);
    if (t == FSBlockType::ROOT) {
        throw FSError(FSError::FS_INVALID_PATH);
    }
}

void
FSRequire::emptyDirectory(BlockNr nr)
{
    directory(nr);
    auto &node = fs.fetch(nr);
    if (FSTree(node, { .recursive = false }).size() != 0) {
        throw FSError(FSError::FS_DIR_NOT_EMPTY);
    }
}

void
FSRequire::notExist(BlockNr nr, const FSName &name)
{
    directory(nr);
    auto &node = fs.fetch(nr);
    if (node.fs->searchdir(node.nr, name)) throw FSError(FSError::FS_EXISTS);
}

void
FSEnsure::inRange(BlockNr nr)
{
    assert(nr < fs.getTraits().blocks);
}

}
