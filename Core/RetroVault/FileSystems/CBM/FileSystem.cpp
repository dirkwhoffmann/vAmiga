// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FileSystem.h"
#include "utl/io.h"
#include "utl/support/Strings.h"
#include <climits>
#include <unordered_set>
#include <stack>
#include <algorithm>

namespace retro::vault::cbm {

FileSystem::FileSystem(Volume &vol) : cache(*this, vol)
{
    loginfo(FS_DEBUG, "Creating file system...\n");

    auto layout = FSDescriptor(vol.capacity());

    // Check consistency (may throw)
    layout.checkCompatibility();

    // Derive persistant file system properties
    traits.init(cache.predictDOS(vol), vol.capacity());

    // Predict the file system
    /*
    traits.dos = cache.predictDOS(vol);

    // Copy layout parameters
    traits.blocks   = layout.numBlocks;
    traits.bytes    = layout.numBlocks * layout.bsize;
    traits.bsize    = layout.bsize;
    rootBlock       = layout.rootBlock;
    */

    if constexpr (debug::FS_DEBUG) dumpState();

    loginfo(FS_DEBUG, "Success\n");
}

void
FileSystem::dumpInfo(std::ostream &os) const noexcept
{
    os << "Type   Size             Used    Free    Full  Name" << std::endl;
    dumpState(os);
}

void
FileSystem::dumpState(std::ostream &os) const noexcept
{
    using namespace utl;

    auto st = stat();

    auto size = std::to_string(traits.blocks) + " (x " + std::to_string(traits.bsize) + ")";

    if (isFormatted()) {

        auto fill = 100.0 * st.usedBlocks / st.blocks;

        os << std::setw(5) << std::left << FSFormatEnum::key(traits.dos);
        os << "  ";
        os << std::setw(15) << std::left << std::setfill(' ') << size;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << st.usedBlocks;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << st.freeBlocks;
        os << "  ";
        os << std::setw(3) << std::right << std::setfill(' ') << isize(fill);
        os << "%  ";
        os << st.name.c_str() << std::endl;

    } else {

        os << std::setw(5) << std::left << "NODOS";
        os << "  ";
        os << std::setw(15) << std::left << std::setfill(' ') << size;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << "--";
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << "--";
        os << "  ";
        os << std::setw(3) << std::left << std::setfill(' ') << "--";
        os << "   ";
        os << "--" << std::endl;
    }
}

void
FileSystem::dumpProps(std::ostream &os) const noexcept
{
    using namespace utl;

    auto st   = stat();
    auto fill = 100.0 * st.usedBlocks / st.blocks;

    os << tab("Name");
    os << st.name << std::endl;
    os << tab("Capacity");
    os << utl::byteCountAsString(traits.blocks * traits.bsize) << std::endl;
    os << tab("Block size");
    os << dec(traits.bsize) << " Bytes" << std::endl;
    os << tab("Blocks");
    os << dec(traits.blocks) << std::endl;
    os << tab("Used");
    os << dec(st.usedBlocks);
    os << tab("Free");
    os << dec(st.freeBlocks);
    os << " (" <<  std::fixed << std::setprecision(2) << fill << "%)" << std::endl;
    os << tab("BAM");
    os << dec(bamBlock) << std::endl;
}

void
FileSystem::dumpBlocks(std::ostream &os) const noexcept
{
    cache.dump(os);
}

bool
FileSystem::isFormatted() const noexcept
{
    // Check the DOS type
    if (traits.dos == FSFormat::NODOS) return false;

    // Check if the BAM is present
    return fetch(bamBlock).is(FSBlockType::BAM);
}

FSStat
FileSystem::stat() const noexcept
{
    auto &bam = fetch(bamBlock);

    FSStat result = {

        .name           = bam.getName().str(),
        .bsize          = traits.bsize,
        .blocks         = traits.blocks,
        .freeBlocks     = cache.freeBlocks(),
        .usedBlocks     = cache.usedBlocks(),
        .blockReads     = 0, // Not yet supported
        .blockWrites    = 0, // Not yet supported
    };

    return result;
}

FSAttr
FileSystem::attr(BlockNr nr) const
{
    auto size   = isize(fetch(nr).getFileSize());
    auto blocks = allocator.requiredBlocks(size);

    FSAttr result = {

        .size   = size,
        .blocks = blocks,
    };

    return result;
}

}
