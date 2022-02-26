// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "Disk.h"
#include "AmigaObject.h"
#include "FSObjects.h"
#include "FSBlock.h"
#include "Geometry.h"

/* To create a FileSystem, several layout parameters need to to be provided.
 * This is done by passing a FileSystemDescriptor which contains the necessary
 * information.
 *
 * A FileSystemDescriptor can be obtained in several ways. If a descriptor for
 * a floppy disk is needed, it can be created by specifiying the form factor
 * and density of the disk. In addition, a suitabe descriptors can be extracted
 * directly from an ADF or HDF.
 */
struct FileSystemDescriptor {

    // The capacity of this file system in blocks
    isize numBlocks = 0;
    
    // Number of reserved blocks
    isize numReserved = 0;
        
    // File system type
    FSVolumeType dos = FS_NODOS;
            
    // Location of the root block
    Block rootBlock = 0;
    
    // References to all bitmap blocks and bitmap extension blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;
};

/* OLD:
 * To create a FSDevice, the layout parameters of the represendet device have
 * to be provided. This is done by passing a FSPartitionDescriptor which
 * provides information about the physical and logical properties of the device.
 *
 * FSDeviceDescriptors can be obtained in several ways. If a descriptor for
 * a floppy disk is needed, it can be created by specifiying the form factor
 * and density of the disk. In addition, a suitabe device constructor can be
 * extracted directly from an ADF or HDF.
 */

struct FSDeviceDescriptor : AmigaObject {
    
    // Device geometry
    DiskGeometry geometry;
    
    i64 numBlocks = 0;    // DEPRECATED. MAKE IT A COMPUTED VALUE IN geometry
    
    // Number of reserved blocks
    isize numReserved = 0;
        
    // File system type
    FSVolumeType dos = FS_NODOS;
            
    // Location of the root block
    Block rootBlock = 0;
    
    // References to all bitmap blocks and bitmap extension blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;
    
    
    //
    // Initializing
    //
    
    FSDeviceDescriptor() { };
    FSDeviceDescriptor(DiskDiameter type, DiskDensity density, FSVolumeType dos = FS_OFS);
    FSDeviceDescriptor(const DiskGeometry &geometry, FSVolumeType dos = FS_OFS);

private:
    
    void init(DiskDiameter type, DiskDensity density, FSVolumeType dos);
    void init(const DiskGeometry &geometry, FSVolumeType dos);
    
    
    //
    // Methods from AmigaObject
    //

private:
    
    const char *getDescription() const override { return "FSLayout"; }
    void _dump(dump::Category category, std::ostream& os) const override;
};
