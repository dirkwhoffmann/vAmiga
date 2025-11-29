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

namespace vamiga {

void
FileSystem::format(string name)
{
    format(traits.dos);
}

void
FileSystem::format(FSFormat dos, string name){

    require_initialized();

    traits.dos = dos;
    if (dos == FSFormat::NODOS) return;

    // Perform some consistency checks
    assert(blocks() > 2);
    assert(rootBlock > 0);

    // Trash all existing data
    storage.init(blocks());

    // Create boot blocks
    storage[0].init(FSBlockType::BOOT);
    storage[1].init(FSBlockType::BOOT);

    // Create the root block
    storage[rootBlock].init(FSBlockType::ROOT);

    // Create bitmap blocks
    for (auto& ref : bmBlocks) {

        // storage.write(ref, new FSBlock(this, ref, FSBlockType::BITMAP_BLOCK));
        storage[ref].init(FSBlockType::BITMAP);
    }

    // Add bitmap extension blocks
    Block pred = rootBlock;
    for (auto &ref : bmExtBlocks) {

        // storage.write(ref, new FSBlock(this, ref, FSBlockType::BITMAP_EXT_BLOCK));
        storage[ref].init(FSBlockType::BITMAP_EXT);
        storage[pred].setNextBmExtBlockRef(ref);
        pred = ref;
    }

    // Add all bitmap block references
    storage[rootBlock].addBitmapBlockRefs(bmBlocks);

    // Mark free blocks as free in the bitmap block
    // TODO: SPEED THIS UP
    for (isize i = 0; i < blocks(); i++) {
        if (storage.isEmpty(Block(i))) allocator.markAsFree(Block(i));
    }

    // Set the volume name
    if (name != "") setName(name);

    // Rectify checksums
    storage[0].updateChecksum();
    storage[1].updateChecksum();
    storage[rootBlock].updateChecksum();
    for (auto& ref : bmBlocks) { storage[ref].updateChecksum(); }
    for (auto& ref : bmExtBlocks) { storage[ref].updateChecksum(); }

    // Set the current directory
    current = rootBlock;
}

void
FileSystem::setName(FSName name)
{
    if (auto *rb = storage.read(rootBlock, FSBlockType::ROOT); rb) {

        rb->setName(name);
        rb->updateChecksum();
    }
}

void
FileSystem::makeBootable(BootBlockId id)
{
    assert(storage.getType(0) == FSBlockType::BOOT);
    assert(storage.getType(1) == FSBlockType::BOOT);
    storage[0].writeBootBlock(id, 0);
    storage[1].writeBootBlock(id, 1);
}

void
FileSystem::killVirus()
{
    assert(storage.getType(0) == FSBlockType::BOOT);
    assert(storage.getType(1) == FSBlockType::BOOT);

    if (bootStat().hasVirus) {

        auto id =
        traits.ofs() ? BootBlockId::AMIGADOS_13 :
        traits.ffs() ? BootBlockId::AMIGADOS_20 : BootBlockId::NONE;

        if (id != BootBlockId::NONE) {
            storage[0].writeBootBlock(id, 0);
            storage[1].writeBootBlock(id, 1);
        } else {
            std::memset(storage[0].data() + 4, 0, traits.bsize - 4);
            std::memset(storage[1].data(), 0, traits.bsize);
        }
    }
}

FSBlock &
FileSystem::mkdir(FSBlock &at, const FSName &name)
{
    ensureDirectory(at);

    // Error out if the file already exists
    if (searchDir(at, name)) throw(AppError(Fault::FS_EXISTS, name.cpp_str()));

    FSBlock &block = newUserDirBlock(name);
    block.setParentDirRef(at.nr);
    addToHashTable(at.nr, block.nr);

    return block;
}

void
FileSystem::rmdir(FSBlock &at, const FSName &name)
{
    ensureEmptyDirectory(at);

    deleteFromHashTable(at);
    reclaim(at);
}

FSBlock *
FileSystem::searchDir(const FSBlock &at, const FSName &name)
{
    std::unordered_set<Block> visited;

    // Only proceed if a hash table is present
    if (!at.hasHashTable()) return nullptr;

    // Compute the table position and read the item
    u32 hash = name.hashValue(traits.dos) % at.hashTableSize();
    u32 ref = at.getHashRef(hash);

    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {

        auto *block = read(ref, { FSBlockType::USERDIR, FSBlockType::FILEHEADER });
        if (block == nullptr) break;

        if (block->isNamed(name)) return block;

        visited.insert(ref);
        ref = block->getNextHashRef();
    }

    return nullptr;
}

void
FileSystem::link(FSBlock &at, FSBlock &fhb)
{
    ensureNotExist(at, fhb.name());

    // Wire
    fhb.setParentDirRef(at.nr);
    addToHashTable(at.nr, fhb.nr);
}

void
FileSystem::unlink(const FSBlock &node)
{
    ensureFileOrDirectory(node);

    // Unwire
    deleteFromHashTable(node);
}

}
