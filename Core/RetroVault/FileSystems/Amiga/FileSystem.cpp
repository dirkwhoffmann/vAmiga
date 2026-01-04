// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/Amiga/FileSystem.h"
#include "utl/io.h"
#include "utl/support/Strings.h"
#include <climits>
#include <unordered_set>
#include <stack>
#include <algorithm>

namespace retro::vault::amigafs {

bool
FSTraits::adf() const
{
    auto size = blocks * bsize;

    return
    size == 901120   ||   //  880 KB (DD)
    size == 912384   ||   //  891 KB (DD + 1 cyl)
    size == 923648   ||   //  902 KB (DD + 2 cyls)
    size == 934912   ||   //  913 KB (DD + 3 cyls)
    size == 946176   ||   //  924 KB (DD + 4 cyls)
    size == 1802240;      // 1760 KB (HD)
}

FileSystem::FileSystem(Volume &vol) : cache(*this, vol)
{
    loginfo(FS_DEBUG, "Creating file system...\n");

    auto layout = FSDescriptor(vol.capacity());

    // Check consistency (may throw)
    layout.checkCompatibility();

    // Predict the file system
    traits.dos = cache.predictDOS(vol);

    // Copy layout parameters
    traits.blocks   = layout.numBlocks;
    traits.bytes    = layout.numBlocks * layout.bsize;
    traits.bsize    = layout.bsize;
    traits.reserved = layout.numReserved;
    rootBlock       = layout.rootBlock;
    bmBlocks        = layout.bmBlocks;
    bmExtBlocks     = layout.bmExtBlocks;

    if constexpr (debug::FS_DEBUG) dumpState();

    // Set the current directory to '/'
    current = rootBlock;

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

        os << std::setw(5) << std::left << ("DOS" + std::to_string(isize(traits.dos)));
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
    auto bst  = bootStat();
    auto fill = 100.0 * st.usedBlocks / st.blocks;

    os << tab("Name");
    os << st.name << std::endl;
    os << tab("Created");
    os << st.btime << std::endl;
    os << tab("Modified");
    os << st.mtime << std::endl;
    os << tab("Boot block");
    os << bst.name << std::endl;
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
    os << tab("Root block");
    os << dec(rootBlock) << std::endl;
    os << tab("Bitmap blocks");
    for (auto& it : bmBlocks) { os << dec(it) << " "; }
    os << std::endl;
    os << tab("Extension blocks");
    for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
    os << std::endl;
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

    // Check if the root block is present
    return fetch(rootBlock).is(FSBlockType::ROOT);
}

FSPosixStat
FileSystem::stat() const noexcept
{
    auto &rb = fetch(rootBlock);

    FSPosixStat result = {

        .bsize          = traits.bsize,
        .blocks         = traits.blocks,
        .freeBlocks     = cache.freeBlocks(),
        .usedBlocks     = cache.usedBlocks(),
        .btime          = rb.isRoot() ? rb.getCreationDate().time() : FSTime().time(),
        .mtime          = rb.isRoot() ? rb.getModificationDate().time() : FSTime().time(),
        .blockReads     = 0, // Not yet supported
        .blockWrites    = 0, // Not yet supported
    };

    return result;
}

FSBootStat
FileSystem::bootStat() const noexcept
{
    auto bb = FSBootBlockImage(cache[0].data(), cache[1].data());

    FSBootStat result = {

        .name = bb.name,
        .type = bb.type,
        .hasVirus = bb.type == BootBlockType::VIRUS
    };

    return result;
}

FSPosixAttr
FileSystem::attr(BlockNr nr) const
{
    auto &fhd   = fetch(nr);
    auto size   = isize(fhd.getFileSize());
    auto blocks = allocator.requiredBlocks(size);

    FSPosixAttr result = {

        .size   = size,
        .blocks = blocks,
        .prot   = fhd.getProtectionBits(),
        .isDir  = fhd.isDirectory(),
        .btime  = fhd.getCreationDate().time(),
        .atime  = fhd.getModificationDate().time(),
        .mtime  = fhd.getModificationDate().time(),
        .ctime  = fhd.getCreationDate().time()
    };

    return result;
}

}
