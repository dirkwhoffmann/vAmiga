// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "FloppyDisk.h"
#include "Error.h"
#include "FSObjects.h"
#include "FSBlock.h"
#include "DriveDescriptors.h"

namespace vamiga {

/* To create a FileSystem, several layout parameters need to to be provided.
 * This is done by passing a FSDescriptor which contains the necessary
 * information.
 *
 * A FSDescriptor can be obtained in several ways. If a descriptor for
 * a floppy disk is needed, it can be created by specifiying the form factor
 * and density of the disk. In addition, a suitabe descriptors can be extracted
 * directly from an ADF or HDF.
 */
struct FSDescriptor {

    // Capacity of the file system in blocks
    isize numBlocks = 0;
    
    // Size of a block in bytes
    isize bsize = 512;
    
    // Number of reserved blocks
    isize numReserved = 0;

    // File system type
    FSFormat dos = FSFormat::NODOS;

    // Location of the root block
    Block rootBlock = 0;
    
    // References to all bitmap blocks and bitmap extension blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;
    
    // Initializing
    FSDescriptor() { };
    FSDescriptor(isize numBlocks, FSFormat dos);
    FSDescriptor(const GeometryDescriptor &geometry, FSFormat dos);
    FSDescriptor(const PartitionDescriptor &des);
    FSDescriptor(Diameter dia, Density den, FSFormat dos);
    
    void init(isize numBlocks, FSFormat dos);
    void init(const GeometryDescriptor &geometry, FSFormat dos);
    void init(const PartitionDescriptor &des);
    void init(Diameter type, Density density, FSFormat dos);

    // Computed values
    isize numBytes() const { return numBlocks * bsize; }
    
    // Prints debug information
    void dump() const;
    void dump(std::ostream &os) const;
    
    // Throws an exception if the descriptor contains unsupported values
    void checkCompatibility() const;
};

}
