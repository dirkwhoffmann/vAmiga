// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystem.h"
#include "utl/support/Streams.h"
#include "utl/support/Strings.h"
#include <climits>
#include <unordered_set>
#include <stack>
#include <algorithm>

namespace utl { using namespace support; }

namespace vamiga {

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

void
FileSystem::init(isize capacity, isize bsize)
{
    traits.blocks   = capacity;
    traits.bytes    = capacity * bsize;
    traits.bsize    = bsize;

    storage.init(capacity);

    if (isize(rootBlock) >= capacity) rootBlock = 0;
    if (isize(current) >= capacity) current = 0;
}

void
FileSystem::init(const FSDescriptor &layout, u8 *buf, isize len)
{
    assert(buf);
    assert(len == layout.numBlocks * 512);

    debug(FS_DEBUG, "Importing %ld blocks from buffer...\n", layout.numBlocks);

    // Check the consistency of the file system descriptor
    layout.checkCompatibility();

    // Only proceed if the volume is formatted
    if (layout.dos == FSFormat::NODOS) throw FSError(FSFault::FS_UNFORMATTED);

    // Copy layout parameters
    traits.dos      = layout.dos;
    traits.blocks   = layout.numBlocks;
    traits.bytes    = layout.numBlocks * layout.bsize;
    traits.bsize    = layout.bsize;
    traits.reserved = layout.numReserved;
    rootBlock       = layout.rootBlock;
    bmBlocks        = layout.bmBlocks;
    bmExtBlocks     = layout.bmExtBlocks;

    // Create all blocks
    storage.init(layout.numBlocks);

    for (isize i = 0; i < layout.numBlocks; i++) {

        const u8 *data = buf + i * traits.bsize;
        if (auto type = predictType((Block)i, data); type != FSBlockType::EMPTY) {

            // Create new block
            storage[i].init(type);

            // Import block data
            storage[i].importBlock(data, traits.bsize);

            // Emulate some errors for debugging
            /*
            auto *data = storage[i].data();
            for (isize i = 0; i < 20; i++) {
                data[rand() % 512] = rand() & 0xFF;
            }
            */
        }
    }

    // Set the current directory to '/'
    current = rootBlock;

    debug(FS_DEBUG, "Success\n");
}

void
FileSystem::init(const FSDescriptor &layout, const fs::path &path)
{
    if (FS_DEBUG) { layout.dump(); }

    // Create all blocks
    init(isize(layout.numBlocks));

    // Copy layout parameters
    traits.dos      = layout.dos;
    traits.reserved = layout.numReserved;
    rootBlock       = layout.rootBlock;
    bmBlocks        = layout.bmBlocks;
    bmExtBlocks     = layout.bmExtBlocks;

    // Format the file system
    format();

    // Start allocating blocks at the middle of the disk
    allocator.ap = rootBlock;

    // Print some debug information
    if (FS_DEBUG) { dumpState(); }

    // Import files if a path is given
    if (!path.empty()) {

        // Add all files
        importer.import(root(), path, true, true);

        // Assign device name
        setName(FSName(path.filename().string()));
    }
}

bool
FileSystem::isInitialized() const noexcept
{
    return blocks() > 0;
}

bool
FileSystem::isFormatted() const noexcept
{
    // Check if the file system is initialized
    if (!isInitialized()) return false;

    // Check the DOS type
    if (traits.dos == FSFormat::NODOS) return false;

    // Check if the root block is present
    if (!storage.read(rootBlock, FSBlockType::ROOT)) return false;

    return true;
}

/*
void
FileSystem::_dump(Category category, std::ostream &os) const noexcept
{
    using namespace util;

    auto stat = getStat();

    switch (category) {

        case Category::Info:

            os << "Type   Size             Used    Free    Full  Name" << std::endl;
            [[fallthrough]];

        case Category::State:
        {
            auto size = std::to_string(stat.numBlocks) + " (x " + std::to_string(traits.bsize) + ")";

            if (isFormatted()) {

                os << std::setw(5) << std::left << ("DOS" + std::to_string(isize(traits.dos)));
                os << "  ";
                os << std::setw(15) << std::left << std::setfill(' ') << size;
                os << "  ";
                os << std::setw(6) << std::left << std::setfill(' ') << stat.usedBlocks;
                os << "  ";
                os << std::setw(6) << std::left << std::setfill(' ') << stat.freeBlocks;
                os << "  ";
                os << std::setw(3) << std::right << std::setfill(' ') << isize(stat.fill);
                os << "%  ";
                os << stat.name.c_str() << std::endl;

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
            break;
        }
        case Category::Properties:
        {
            os << tab("Name");
            os << stat.name.cpp_str() << std::endl;
            os << tab("Created");
            os << stat.bDate.str() << std::endl;
            os << tab("Modified");
            os << stat.mDate.str() << std::endl;
            os << tab("Boot block");
            os << getBootBlockName() << std::endl;
            os << tab("Capacity");
            os << util::byteCountAsString(stat.numBlocks * traits.bsize) << std::endl;
            os << tab("Block size");
            os << dec(traits.bsize) << " Bytes" << std::endl;
            os << tab("Blocks");
            os << dec(stat.numBlocks) << std::endl;
            os << tab("Used");
            os << dec(stat.usedBlocks);
            os << tab("Free");
            os << dec(stat.freeBlocks);
            os << " (" <<  std::fixed << std::setprecision(2) << stat.fill << "%)" << std::endl;
            os << tab("Root block");
            os << dec(rootBlock) << std::endl;
            os << tab("Bitmap blocks");
            for (auto& it : bmBlocks) { os << dec(it) << " "; }
            os << std::endl;
            os << util::tab("Extension blocks");
            for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
            os << std::endl;
            break;
        }
        case Category::Blocks:

            storage.dump(Category::Blocks, os);
            break;

        default:
            break;
    }
}
*/

void
FileSystem::dumpInfo(std::ostream &os) const noexcept
{
    os << "Type   Size             Used    Free    Full  Name" << std::endl;
    dumpState(os);
}

void
FileSystem::dumpState(std::ostream &os) const noexcept
{
    using namespace utl::support;

    auto st = stat();

    auto size = std::to_string(traits.blocks) + " (x " + std::to_string(traits.bsize) + ")";

    if (isFormatted()) {

        os << std::setw(5) << std::left << ("DOS" + std::to_string(isize(traits.dos)));
        os << "  ";
        os << std::setw(15) << std::left << std::setfill(' ') << size;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << st.usedBlocks;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << st.freeBlocks;
        os << "  ";
        os << std::setw(3) << std::right << std::setfill(' ') << isize(st.fill);
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
    using namespace utl::support;

    auto st = stat();
    auto bst = bootStat();

    os << tab("Name");
    os << st.name.cpp_str() << std::endl;
    os << tab("Created");
    os << st.bDate.str() << std::endl;
    os << tab("Modified");
    os << st.mDate.str() << std::endl;
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
    os << " (" <<  std::fixed << std::setprecision(2) << st.fill << "%)" << std::endl;
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
    storage.dump(os);
}

FSStat
FileSystem::stat() const noexcept
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT);

    FSStat result = {

        .traits     = traits,

        .freeBlocks = storage.freeBlocks(),
        .freeBytes  = storage.freeBytes(),
        .usedBlocks = storage.usedBlocks(),
        .usedBytes  = storage.usedBytes(),
        .fill       = double(100) * storage.usedBlocks() / storage.numBlocks(),

        .name       = rb ? rb->getName() : FSName(""),
        .bDate      = rb ? rb->getCreationDate() : FSTime(),
        .mDate      = rb ? rb->getModificationDate() : FSTime(),

        .reads      = 0, // Not yet supported
        .writes     = 0, // Not yet supported
    };

    return result;
}

FSBootStat
FileSystem::bootStat() const noexcept
{
    auto bb = BootBlockImage(storage[0].data(), storage[1].data());

    FSBootStat result = {

        .name = bb.name,
        .type = bb.type,
        .hasVirus = bb.type == BootBlockType::VIRUS
    };

    return result;
}

/*
string
FileSystem::getBootBlockName() const noexcept
{
    return BootBlockImage(storage[0].data(), storage[1].data()).name;
}

BootBlockType
FileSystem::bootBlockType() const noexcept
{
    return BootBlockImage(storage[0].data(), storage[1].data()).type;
}
*/

FSAttr
FileSystem::attr(Block nr) const
{
    return attr(at(nr));
}

FSAttr
FileSystem::attr(const FSBlock &fhd) const
{
    isize size = fhd.getFileSize();
    isize blocks = allocator.requiredBlocks(size);

    FSAttr result = {

        .size   = size,
        .blocks = blocks,
        .prot   = fhd.getProtectionBits(),
        .isDir  = fhd.isDirectory(),
        .ctime  = fhd.getCreationDate(),
        .mtime  = fhd.getModificationDate()
    };

    return result;
}








namespace require {

void
initialized(const FileSystem &fs)
{
    if (!fs.isInitialized()) throw FSError(FSFault::FS_UNINITIALIZED);
}

void
formatted(const FileSystem &fs)
{
    if (!fs.isInitialized()) throw FSError(FSFault::FS_UNINITIALIZED);
    if (!fs.isFormatted()) throw FSError(FSFault::FS_UNFORMATTED);
}

void
file(const FSBlock &node)
{
    if (!node.isFile()) throw FSError(FSFault::FS_NOT_A_FILE);
}

void
fileOrDirectory(const FSBlock &node)
{
    if (!node.isRegular()) throw FSError(FSFault::FS_NOT_A_FILE_OR_DIRECTORY);
}

void
directory(const FSBlock &node)
{
    if (!node.isDirectory()) throw FSError(FSFault::FS_NOT_A_DIRECTORY);
}

void
notRoot(const FSBlock &node)
{
    if (node.isRoot()) throw FSError(FSFault::FS_INVALID_PATH);
}

void
emptyDirectory(const FSBlock &node)
{
    directory(node);

    if (FSTree(node, { .recursive = false }).size() != 0) {
        throw FSError(FSFault::FS_DIR_NOT_EMPTY);
    }
}

void
notExist(const FSBlock &node, const FSName &name)
{
    directory(node);
    if (node.fs->searchdir(node, name) != nullptr) throw FSError(FSFault::FS_EXISTS);
}

}}
