// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FileSystem.h"
#include "utl/support.h"

namespace retro::vault::cbm {

optional<FSDirEntry>
FileSystem::trySeekEntry(const PETName<16> &path) const
{
    auto dir = readDir();

    for (auto &item : dir)
        if (item.getName() == path) return item;

    return {};
}

optional<BlockNr>
FileSystem::trySeek(const PETName<16> &path) const
{
    if (auto entry = trySeekEntry(path))
        return traits.blockNr(entry->firstBlock());

    return {};
}

FSDirEntry
FileSystem::seekEntry(const PETName<16> &path) const
{
    if (auto it = trySeekEntry(path)) return *it;
    throw FSError(FSError::FS_NOT_FOUND, path.str());
}

BlockNr
FileSystem::seek(const PETName<16> &path) const
{
    if (auto it = trySeek(path)) return *it;
    throw FSError(FSError::FS_NOT_FOUND, path.str());
}

/*
vector<BlockNr>
FileSystem::match(BlockNr top, const vector<FSPattern> &patterns)
{
    return {};
    vector<BlockNr> currentSet { top };

    for (const auto &pattern : patterns) {

        vector<BlockNr> nextSet;

        // No-ops
        if (pattern.glob == "" || pattern.glob == ".") {
            continue;
        }

        // Root traversal
        if (pattern.glob == ":" || pattern.glob == "/") {
            currentSet = { root() };
            continue;
        }

        // Parent traversal
        if (pattern.glob == "..") {
            for (auto blk : currentSet) {
                nextSet.push_back(fetch(blk).getParentDirRef());
            }
            currentSet = std::move(nextSet);
            continue;
        }

        // Pattern-based lookup
        for (auto blk : currentSet) {

            printf("  Seeking '%s' in '%s'\n",
                   pattern.glob.c_str(),
                   fetch(blk).absName().c_str());

            auto matches = searchdir(blk, pattern);
            for (auto m : matches) {
                printf("    Found %ld (%s)\n", m, fetch(m).absName().c_str());
                nextSet.push_back(m);
            }
        }

        if (nextSet.empty()) {
            printf("No matches for '%s'\n", pattern.glob.c_str());
            return {};
        }

        currentSet = std::move(nextSet);
    }

    return currentSet;
}
*/

vector<BlockNr>
FileSystem::match(const string &path)
{
    return {};
}

}
