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

void
FileSystemDescriptor::dump() const
{
    dump(std::cout);
}

void
FileSystemDescriptor::dump(std::ostream& os) const
{
    using namespace util;
    
    os << tab("Blocks");
    os << dec(numBlocks) << std::endl;
    os << tab("BSize");
    os << dec(bsize) << std::endl;
    os << tab("Reserved");
    os << dec(numReserved) << std::endl;
    os << tab("DOS version");
    os << FSVolumeTypeEnum::key(dos) << std::endl;
    os << tab("Root block");
    os << dec(rootBlock) << std::endl;
    os << tab("Bitmap blocks");
    for (auto& it : bmBlocks) { os << dec(it) << " "; }; os << std::endl;
    os << tab("Extension blocks");
    for (auto& it : bmExtBlocks) { os << dec(it) << " "; }; os << std::endl;
}

void
FileSystemDescriptor::checkCompatibility() const
{
    if (numBytes() > MB(504) || FORCE_FS_WRONG_CAPACITY) {
        throw VAError(ERROR_FS_WRONG_CAPACITY);
    }
    if (bsize != 512 || FORCE_FS_WRONG_BSIZE) {
        throw VAError(ERROR_FS_WRONG_BSIZE);
    }
    if (!FSVolumeTypeEnum::isValid(dos) || FORCE_FS_WRONG_DOS_TYPE) {
        throw VAError(ERROR_FS_WRONG_DOS_TYPE);
    }
}

DiskGeometry::DiskGeometry(isize c, isize h, isize s, isize b)
{
    cylinders = c;
    heads = h;
    sectors = s;
    bsize = b;
}

DiskGeometry::DiskGeometry(isize size)
{
    // Create a default geometry for the provide size
    bsize = 512;
    sectors = 32;
    heads = 1;
    cylinders = (size / bsize) + (size % bsize) ? 1 : 0;
    while (cylinders > 1024) { cylinders /= 2; heads *= 2; }
}

DiskGeometry::DiskGeometry(Diameter type, Density density)
{
    if (type == INCH_525 && density == DENSITY_DD) {
        
        cylinders = 40;
        heads = 2;
        sectors = 11;
        bsize = 512;
        return;
    }
    if (type == INCH_35 && density == DENSITY_DD) {
        
        cylinders = 80;
        heads = 2;
        sectors = 11;
        bsize = 512;
        return;
    }
    if (type == INCH_35 && density == DENSITY_HD) {
        
        cylinders = 80;
        heads = 2;
        sectors = 22;
        bsize = 512;
        return;
    }
    
    fatalError;
}

FSDeviceDescriptor::FSDeviceDescriptor(Diameter type, Density density, FSVolumeType dos)
{
    init(type, density, dos);
}

FSDeviceDescriptor::FSDeviceDescriptor(const DiskGeometry &geometry, FSVolumeType dos)
{
    init(geometry, dos);
}

void
FSDeviceDescriptor::init(Diameter type, Density density, FSVolumeType dos)
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
    this->dos = dos;
    this->rootBlock = root;
    // partition = FSPartitionDescriptor(dos, 0, geometry.upperCyl(), root);

    // Determine number of bitmap blocks
    isize bitsPerBlock = (geometry.bsize - 4) * 8;
    isize neededBlocks = (numBlocks + bitsPerBlock - 1) / bitsPerBlock;
    
    // TODO: CREATE BITMAP EXTENSION BLOCKS IF THE NUMBER EXCEEDS 25
    assert(neededBlocks <= 25);
    
    // Add all bitmap blocks
    for (isize i = 0; i < neededBlocks; i++) {
        bmBlocks.push_back(Block(root + 1 + i));
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
