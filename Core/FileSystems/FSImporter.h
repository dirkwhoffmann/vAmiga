// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSComponent.h"

namespace vamiga {

class FSImporter final : public FSComponent {

public:

    using FSComponent::FSComponent;

    // Imports the volume from a buffer compatible with the ADF or HDF format
    void importVolume(const u8 *src, isize size) throws;

    // Imports files and folders from the host file system
    void import(const fs::path &path, bool recursive = true, bool contents = false) throws;
    void import(FSBlock &top, const fs::path &path, bool recursive = true, bool contents = false) throws;

    // Imports a single block
    void importBlock(Block nr, const fs::path &path);

    // Updates the checksums of all blocks
    void updateChecksums() noexcept;

private:

    void import(FSBlock &top, const fs::directory_entry &dir, bool recursive) throws;
};

}
