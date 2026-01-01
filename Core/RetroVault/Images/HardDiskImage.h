// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskImage.h"

namespace vamiga {

class HardDiskImage : public DiskImage {

public:

    static optional<ImageInfo> about(const fs::path& url);
    
    // Hard disk factory
    static std::unique_ptr<HardDiskImage> make(const fs::path &path);

    // Informs about the contained partitions
    virtual isize numPartitions() const = 0;
    virtual Range<isize> partition(isize nr) const = 0;

    // Exports a single partition
    isize writePartitionToStream(std::ostream &stream, isize nr) const;
    isize writePartitionToFile(const fs::path &path, isize nr) const;
};

}
