// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FSDescriptor.h"
#include "FileSystems/CBM/FSError.h"
#include "BlockDevice.h"
#include "ADFFile.h"
#include "utl/io.h"

namespace retro::vault::cbm {

using retro::vault::image::ADFFile;

FSDescriptor::FSDescriptor(isize numBlocks)
{
    init(numBlocks);
}

FSDescriptor::FSDescriptor(Diameter dia, Density den)
{
    init(dia, den);
}

FSDescriptor::FSDescriptor(const GeometryDescriptor &geometry)
{
    init(geometry);
}

FSDescriptor::FSDescriptor(const PartitionDescriptor &des)
{
    init(des);
}


void
FSDescriptor::init(isize numBlocks)
{
    // Copy parameters
    this->numBlocks = numBlocks;

    // Determine the location of the BAM (18:0)
    rootBlock = 357;
}

void
FSDescriptor::init(const GeometryDescriptor &geometry)
{
    init(geometry.numBlocks());
}

void
FSDescriptor::init(const PartitionDescriptor &des)
{

    init(des.numBlocks());
}

void
FSDescriptor::init(Diameter dia, Density den)
{
    init(GeometryDescriptor(dia, den));
}


void
FSDescriptor::dump() const
{
    dump(std::cout);
}

void
FSDescriptor::dump(std::ostream &os) const
{
    using namespace utl;

    os << tab("Blocks");
    os << dec(numBlocks) << std::endl;
    os << tab("Block size");
    os << dec(bsize) << std::endl;
    os << tab("BAM location");
    os << dec(rootBlock) << std::endl;
}

void
FSDescriptor::checkCompatibility() const
{
    if constexpr (force::FS_WRONG_CAPACITY)
        throw FSError(FSError::FS_WRONG_CAPACITY);

    if constexpr (force::FS_WRONG_BSIZE)
        throw FSError(FSError::FS_WRONG_BSIZE);

    if (numBlocks != 683 && numBlocks != 768 && numBlocks != 802)
        throw FSError(FSError::FS_WRONG_CAPACITY);

    if (bsize != 256)
        throw FSError(FSError::FS_WRONG_BSIZE);

    if (isize(rootBlock) >= numBlocks)
        throw FSError(FSError::FS_OUT_OF_RANGE);
}

}
