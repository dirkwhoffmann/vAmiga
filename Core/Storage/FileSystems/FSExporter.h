// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSExtension.h"

namespace vamiga {

class FSExporter final : public FSExtension {

public:

    using FSExtension::FSExtension;

    // Exports the volume to a buffer
    bool exportVolume(u8 *dst, isize size) const;
    bool exportVolume(u8 *dst, isize size, FSFault *error) const;

    // Exports a single block or a range of blocks to a buffer
    bool exportBlock(BlockNr nr, u8 *dst, isize size) const;
    bool exportBlock(BlockNr nr, u8 *dst, isize size, FSFault *error) const;
    bool exportBlocks(BlockNr first, BlockNr last, u8 *dst, isize size) const;
    bool exportBlocks(BlockNr first, BlockNr last, u8 *dst, isize size, FSFault *error) const;

    // Exports a single block or a range of blocks to a file
    void exportBlock(BlockNr nr, const fs::path &path) const;
    void exportBlocks(BlockNr first, BlockNr last, const fs::path &path) const;
    void exportBlocks(const fs::path &path) const;

    // Exports the volume to a buffer
    void exportFiles(BlockNr nr, const fs::path &path, bool recursive = true, bool contents = false) const;
    void exportFiles(const FSBlock &top, const fs::path &path, bool recursive = true, bool contents = false) const;
    void exportFiles(const fs::path &path, bool recursive = true, bool contents = false) const;
};

}
