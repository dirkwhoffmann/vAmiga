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

DiskGeometry::DiskGeometry(isize c, isize h, isize s, isize b)
{
    cylinders = c;
    heads = h;
    sectors = s;
    bsize = b;
}

DiskGeometry::DiskGeometry(DiskDiameter type, DiskDensity density)
{
    if (type == INCH_525 && density == DISK_DD) {
        
        cylinders = 40;
        heads = 2;
        sectors = 11;
        bsize = 512;
        return;
    }
    if (type == INCH_35 && density == DISK_DD) {
        
        cylinders = 80;
        heads = 2;
        sectors = 11;
        bsize = 512;
        return;
    }
    if (type == INCH_35 && density == DISK_HD) {
        
        cylinders = 80;
        heads = 2;
        sectors = 22;
        bsize = 512;
        return;
    }
    
    fatalError;
}

FSDeviceDescriptor::FSDeviceDescriptor(DiskDiameter type, DiskDensity density, FSVolumeType dos)
{
    init(type, density, dos);
}

FSDeviceDescriptor::FSDeviceDescriptor(const DiskGeometry &geometry, FSVolumeType dos)
{
    init(geometry, dos);
}

void
FSDeviceDescriptor::init(DiskDiameter type, DiskDensity density, FSVolumeType dos)
{
    init(DiskGeometry(type, density), dos);
}

void
FSDeviceDescriptor::init(const DiskGeometry &geometry, FSVolumeType dos)
{
    this->geometry = geometry;

    numBlocks = geometry.numBlocks();
    numReserved = 2;
    
    // Determine the location of the root block
    Block root = (Block)(numBlocks / 2);

    // Add the partition
    partitions.push_back(FSPartitionDescriptor(dos, 0, geometry.upperCyl(), root));

    // Determine number of bitmap blocks
    isize bitsPerBlock = (geometry.bsize - 4) * 8;
    isize neededBlocks = (numBlocks + bitsPerBlock - 1) / bitsPerBlock;
    
    // TODO: CREATE BITMAP EXTENSION BLOCKS IF THE NUMBER EXCEEDS 25
    assert(neededBlocks <= 25);
    
    // Add all bitmap blocks
    for (isize i = 0; i < neededBlocks; i++) {
        partitions[0].bmBlocks.push_back(Block(root + 1 + i));
    }
}

void
FSDeviceDescriptor::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
        
        os << tab("Cylinders");
        os << dec(geometry.cylinders) << std::endl;
        os << tab("Heads");
        os << dec(geometry.heads) << std::endl;
        os << tab("Sectors");
        os << dec(geometry.sectors) << std::endl;
        os << tab("BSize");
        os << dec(geometry.bsize) << std::endl;
        os << tab("Blocks");
        os << dec(numBlocks) << std::endl;
        os << tab("Reserved");
        os << dec(numReserved) << std::endl;
    
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
        for (auto& it : bmBlocks) { os << dec(it) << " "; }; os << std::endl;
        os << tab("Extension blocks");
        for (auto& it : bmExtBlocks) { os << dec(it) << " "; }; os << std::endl;
    }
}
