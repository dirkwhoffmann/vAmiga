// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSDescriptors.h"
#include "IOUtils.h"

FSDeviceDescriptor::FSDeviceDescriptor()
{
    
}

FSDeviceDescriptor::FSDeviceDescriptor(DiskDiameter type, DiskDensity density, FSVolumeType dos)
{
    /* TODO: REPLACE BY init(...)
     * Then, add init(isize numCyls, isize numSectors, FSVolumeType dos) and call it
     */
    if (type == INCH_525 && density == DISK_DD) {
        numCyls = 40; numSectors = 11;

    } else if (type == INCH_35 && density == DISK_DD) {
        numCyls = 80; numSectors = 11;
    
    } else if (type == INCH_35 && density == DISK_HD) {
        numCyls = 80; numSectors = 22;

    } else {
        fatalError;
    }

    numHeads    = 2;
    numBlocks   = numCyls * numHeads * numSectors;
    numReserved = 2;
    bsize       = 512;
    
    // Determine the location of the root block and the bitmap block
    Block root   = (Block)(numBlocks / 2);
    Block bitmap = root + 1;

    partitions.push_back(FSPartitionDescriptor(dos, 0, numCyls - 1, root));
    partitions[0].bmBlocks.push_back(bitmap);
}

void
FSDeviceDescriptor::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
        
        os << tab("Cylinders");
        os << dec(numCyls) << std::endl;
        os << tab("Heads");
        os << dec(numHeads) << std::endl;
        os << tab("Sectors");
        os << dec(numSectors) << std::endl;
        os << tab("Blocks");
        os << dec(numBlocks) << std::endl;
        os << tab("Reserved");
        os << dec(numReserved) << std::endl;
        os << tab("BSize");
        os << dec(bsize) << std::endl;
    
        for (auto& p : partitions) { p.dump(category, os); }
    }
}

FSPartitionDescriptor::FSPartitionDescriptor(FSVolumeType dos,
                                             isize firstCyl, isize lastCyl,
                                             Block root)
{
    this->dos = dos;
    this->lowCyl = firstCyl;
    this->highCyl = lastCyl;
    this->rootBlock = root;

    assert(bmBlocks.size() == 0);
    assert(bmExtBlocks.size() == 0);
}

void
FSPartitionDescriptor::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
        
        os << tab("Partition");
        os << dec(lowCyl) << " - " << dec(highCyl) << std::endl;
        os << tab("File system");
        os << FSVolumeTypeEnum::key(dos) << std::endl;
        os << tab("Root block");
        os << dec(rootBlock) << std::endl;
        os << tab("Bitmap blocks");
        for (auto& it : bmBlocks) { os << dec(it) << " "; }
        os << tab("Extension blocks");
        for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
    }
}
