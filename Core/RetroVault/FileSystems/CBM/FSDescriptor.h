// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystems/CBM/FSTypes.h"
#include "DeviceDescriptors.h"

namespace retro::vault::cbm {

/* To create a FileSystem, several layout parameters need to to be provided.
 * This is done by passing a FSDescriptor which contains the necessary
 * information.
 */
struct FSDescriptor {

    // Capacity of the file system in blocks
    isize numBlocks = 0;
    
    // Size of a block in bytes
    isize bsize = 256;

    // Location of the root block
    BlockNr rootBlock = 0;

    // Initializing
    FSDescriptor() { };
    FSDescriptor(isize numBlocks);
    FSDescriptor(const GeometryDescriptor &geometry);
    FSDescriptor(const PartitionDescriptor &des);
    FSDescriptor(Diameter dia, Density den);
    
    void init(isize numBlocks);
    void init(const GeometryDescriptor &geometry);
    void init(const PartitionDescriptor &des);
    void init(Diameter type, Density density);

    // Computed values
    isize numBytes() const { return numBlocks * bsize; }
    
    // Prints debug information
    void dump() const;
    void dump(std::ostream &os) const;
    
    // Throws an exception if the descriptor contains unsupported values
    void checkCompatibility() const;
};

}
