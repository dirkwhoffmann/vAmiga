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

namespace retro::vault::cbm {

FSBlockType
FileSystem::predictType(BlockNr nr, const u8 *buf) const noexcept
{
    if (buf) {

        auto ts = traits.tsLink(nr);

        // Track 18 contains the BAM and the directory blocks
        if (ts.t == 18) return ts.s == 0 ? FSBlockType::BAM : FSBlockType::USERDIR;

        // Check if this block is a data block
        for (isize i = 0; i < traits.bsize; i++) if (buf[i]) return FSBlockType::DATA;
    }

    return FSBlockType::EMPTY;
}

const FSBlock *
FileSystem::tryFetch(TSLink ts) const noexcept
{
    if (ts.t < 0 || ts.t > traits.numTracks())
        return nullptr;

    return tryFetch(traits.blockNr(ts));
}

void
FileSystem::flush()
{
    cache.flush();
}

}
