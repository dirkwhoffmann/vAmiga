// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSDescriptor.h"
#include "IOUtils.h"

namespace vamiga {

FSDescriptor::FSDescriptor(isize numBlocks, FSFormat dos)
{
    init(numBlocks, dos);
}

FSDescriptor::FSDescriptor(Diameter dia, Density den, FSFormat dos)
{
    init(dia, den, dos);
}

FSDescriptor::FSDescriptor(const GeometryDescriptor &geometry, FSFormat dos)
{
    init(geometry, dos);
}

FSDescriptor::FSDescriptor(const PartitionDescriptor &des)
{
    init(des);
}


void
FSDescriptor::init(isize numBlocks, FSFormat dos)
{
    // Copy parameters
    this->numBlocks = numBlocks;
    this->numReserved = 2;
    this->dos = dos;

    // Determine the location of the root block
    isize highKey = numBlocks - 1;
    isize rootKey = (numReserved + highKey) / 2; 
    rootBlock = Block(rootKey);

    assert(rootKey == numBlocks / 2);

    // Determine the number of required bitmap blocks
    isize bitsPerBlock = (bsize - 4) * 8;
    isize neededBlocks = (numBlocks + bitsPerBlock - 1) / bitsPerBlock;
    isize bmKey = rootKey + 1;

    // Add bitmap blocks
    for (isize i = 0; i < neededBlocks; i++) {
        bmBlocks.push_back(Block(bmKey++));
    }
    
    // Add bitmap extension blocks (the first 25 references are stored in the root block)
    for (; neededBlocks - 25 > 0; neededBlocks -= 127) {
        bmExtBlocks.push_back(Block(bmKey++));
    }
}

void
FSDescriptor::init(const GeometryDescriptor &geometry, FSFormat dos)
{
    init(geometry.numBlocks(), dos);
}

void
FSDescriptor::init(const PartitionDescriptor &des)
{

    init(des.numBlocks(), dos);
}

void
FSDescriptor::init(Diameter dia, Density den, FSFormat dos)
{
    init(GeometryDescriptor(dia, den), dos);
}


void
FSDescriptor::dump() const
{
    dump(std::cout);
}

void
FSDescriptor::dump(std::ostream &os) const
{
    using namespace util;
    
    os << tab("Blocks");
    os << dec(numBlocks) << std::endl;
    os << tab("BSize");
    os << dec(bsize) << std::endl;
    os << tab("Reserved");
    os << dec(numReserved) << std::endl;
    os << tab("DOS version");
    os << FSFormatEnum::key(dos) << std::endl;
    os << tab("Root block");
    os << dec(rootBlock) << std::endl;
    os << tab("Bitmap blocks");
    for (auto& it : bmBlocks) { os << dec(it) << " "; }; os << std::endl;
    os << tab("Extension blocks");
    for (auto& it : bmExtBlocks) { os << dec(it) << " "; }; os << std::endl;
}

void
FSDescriptor::checkCompatibility() const
{
    if (numBytes() > MB(504) || FORCE_FS_WRONG_CAPACITY) {
        throw AppError(Fault::FS_WRONG_CAPACITY);
    }
    if (bsize != 512 || FORCE_FS_WRONG_BSIZE) {
        throw AppError(Fault::FS_WRONG_BSIZE);
    }
    if (!FSFormatEnum::isValid(dos) || FORCE_FS_WRONG_DOS_TYPE) {
        throw AppError(Fault::FS_WRONG_DOS_TYPE);
    }
    if (isize(rootBlock) >= numBlocks) {
        throw AppError(Fault::FS_OUT_OF_RANGE);
    }
}

GeometryDescriptor::GeometryDescriptor(isize c, isize h, isize s, isize b)
{
    cylinders = c;
    heads = h;
    sectors = s;
    bsize = b;
}

GeometryDescriptor::GeometryDescriptor(isize size)
{
    // Create a default geometry for the provide size

    bsize = 512;
    sectors = 32;
    heads = 1;
    
    auto tsize = bsize * sectors;
    cylinders = (size / tsize) + (size % tsize ? 1 : 0);
    
    while (cylinders > 1024) {
        
        cylinders = (cylinders + 1) / 2;
        heads = heads * 2;
    }
}

GeometryDescriptor::GeometryDescriptor(Diameter type, Density density)
{
    if (type == Diameter::INCH_525 && density == Density::SD) {

        cylinders = 40;
        heads = 2;
        sectors = 11;
        bsize = 512;
        return;
    }
    if (type == Diameter::INCH_35 && density == Density::DD) {
        
        cylinders = 80;
        heads = 2;
        sectors = 11;
        bsize = 512;
        return;
    }
    if (type == Diameter::INCH_35 && density == Density::HD) {
        
        cylinders = 80;
        heads = 2;
        sectors = 22;
        bsize = 512;
        return;
    }
    
    fatalError;
}

}
