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
#include <cstring>
#include <span>

namespace retro::vault::cbm {

void
FileSystem::format(FSFormat dos) {

    // Assign the new DOS type
    traits.dos = dos;
    if (dos == FSFormat::NODOS) return;

    // Perform some consistency checks
    assert(blocks() > 2);
    assert(rootBlock > 0);

    // Create boot blocks
    cache.modify(0).init(FSBlockType::BOOT);
    cache.modify(1).init(FSBlockType::BOOT);

    // Wipe out all other blocks
    for (isize i = 2; i < traits.blocks; i++) {
        (*this)[i].mutate().init(FSBlockType::EMPTY);
    }

    // Create the root block
    (*this)[rootBlock].mutate().init(FSBlockType::ROOT);

    // Create bitmap blocks
    for (auto& ref : bmBlocks) {

        // storage.write(ref, new FSBlock(this, ref, FSBlockType::BITMAP_BLOCK));
        cache.modify(ref).init(FSBlockType::BITMAP);
    }

    // Add bitmap extension blocks
    BlockNr pred = rootBlock;
    for (auto &ref : bmExtBlocks) {

        // storage.write(ref, new FSBlock(this, ref, FSBlockType::BITMAP_EXT_BLOCK));
        (*this)[ref].mutate().init(FSBlockType::BITMAP_EXT);
        (*this)[pred].mutate().setNextBmExtBlockRef(ref);
        pred = ref;
    }

    // Add all bitmap block references
    (*this)[rootBlock].mutate().addBitmapBlockRefs(bmBlocks);

    // Mark free blocks as free in the bitmap block
    // TODO: SPEED THIS UP
    for (isize i = 0; i < blocks(); i++) {
        if (cache.isEmpty(BlockNr(i))) allocator.markAsFree(BlockNr(i));
    }

    // Rectify checksums
    fetch(0).mutate().updateChecksum();
    fetch(1).mutate().updateChecksum();
    (*this)[rootBlock].mutate().updateChecksum();
    for (auto& ref : bmBlocks) { (*this)[ref].mutate().updateChecksum(); }
    for (auto& ref : bmExtBlocks) { (*this)[ref].mutate().updateChecksum(); }

    // Set the current directory
    // current = rootBlock;
}

void
FileSystem::setName(const PETName<16> &name)
{
    if (auto bam = tryFetchBAM()) {

        bam->mutate().setName(name);
        bam->mutate().updateChecksum();
    }
}

optional<BlockNr>
FileSystem::searchdir(BlockNr at, const PETName<16> &name) const
{
    for (auto &entry: readDir()) {

        if (entry.getName() == name)
            return traits.blockNr(entry.firstBlock());
    }
    return {};
}

vector<BlockNr>
FileSystem::searchdir(BlockNr at, const FSPattern &pattern) const
{
    vector<BlockNr> result;

    for (auto &entry: readDir()) {

        if (pattern.match(entry.getName().str())) {
            if (auto b = traits.blockNr(entry.firstBlock())) {
                result.push_back(*b);
            }
        }
    }
    return result;
}

void
FileSystem::link(const FSDirEntry &entry)
{
    auto dir = readDir();

    // Find a free slot
    for (auto &slot : dir) {

        if (slot.deleted()) {

            slot = entry;
            writeDir(dir);
            return;
        }
    }

    // Append at the end
    dir.push_back(entry);
    writeDir(dir);
}

void
FileSystem::unlink(BlockNr node)
{
    auto ts  = traits.tsLink(node);
    auto dir = readDir();

    for (auto &slot : dir) {

        if (slot.firstDataTrack == ts.t && slot.firstDataSector == ts.s) {

            slot = FSDirEntry();
            writeDir(dir);
        }
    }
}

vector<FSDirEntry>
FileSystem::readDir() const 
{
    auto dirBlocks = collectDirBlocks();

    vector<FSDirEntry> result;
    result.reserve(dirBlocks.size() * 8);

    // Iterate through all directory blocks
    for (auto block : dirBlocks) {

        auto *data = fetch(block).data();

        // Each directory block contains 8 directory entries
        for (int i = 0; i < 8; i++) {

            FSDirEntry entry(std::span(data + i * 0x20, 0x20));
            result.push_back(entry);
        }
    }

    return result;
}

void
FileSystem::writeDir(const vector<FSDirEntry> &dir)
{
    static constexpr u8 interleave[18] = {
        1, 4, 7, 10, 13, 16, 2, 5, 8, 11, 14, 17, 3, 6, 9, 12, 15, 18
    };

    // A directory contains up to 144 files
    if (dir.size() > 144) throw FSError(FSError::FS_OUT_OF_SPACE);

    // Compute the number of required directory blocks
    auto numDirBlocks = (dir.size() + 7) / 8;

    for (usize b = 0, i = 0; b < numDirBlocks; ++b) {

        auto &block = fetch(TSLink{18,interleave[b]}).mutate();
        auto *data  = block.data();

        for (isize j = 0; j < 8; ++j, ++i) {

            // Write directory entry
            FSDirEntry *entry = (FSDirEntry *)data + j;
            if (i < dir.size()) {
                memcpy(entry, &dir[i], sizeof(FSDirEntry));
            } else {
                memset(entry, 0, sizeof(FSDirEntry));
            }
        }

        // Write TS link
        data[0] = (b + 1 < numDirBlocks) ? 18 : 0;
        data[1] = (b + 1 < numDirBlocks) ? interleave[b + 1] : 0;
    }
}

BlockNr
FileSystem::createFile(const PETName<16> &name)
{
    return createFile(name, nullptr, 0);
}

BlockNr
FileSystem::createFile(const PETName<16> &name, const u8 *buf, isize size)
{
    // Allocate required blocks
    auto blocks = allocator.allocate(std::min(isize(1), (size + 253) / 254));
    auto first  = traits.tsLink(blocks[0]);

    // Add data
    if (buf) replace(blocks, buf, size);

    // Create a directory entry
    FSDirEntry entry;
    entry.firstDataTrack  = u8(first.t);
    entry.firstDataSector = u8(first.s);

    // Add the file to the directory
    // TODO

    return blocks[0];
}

BlockNr
FileSystem::createFile(const PETName<16> &name, const Buffer<u8> &buf)
{
    return createFile(name, buf.ptr, buf.size);
}

BlockNr
FileSystem::createFile(const PETName<16> &name, const string &str)
{
    return createFile(name, (const u8 *)str.c_str(), (isize)str.size());
}

void
FileSystem::rm(BlockNr node)
{
    // Remove the file from its parent directory
    unlink(node);

    // Reclaim all associated storage blocks
    reclaim(node);
}

void
FileSystem::rename(BlockNr item, const PETName<16> &name)
{
    auto &block = fetch(item);

    // Renaming the root updates the file system name
    if (block.isRoot()) { setName(name); return; }

    // For regular items, relocate entry in the parent directory
    // move(item, block.getParentDirRef(), name);

    // TODO
    assert(false);
}

/*
void
FileSystem::move(BlockNr item, BlockNr dest)
{
    move (item, dest, fetch(item).name());
}

void
FileSystem::move(BlockNr item, BlockNr dest, const FSName &name)
{
    require.fileOrDirectory(item);
    require.notExist(dest, name);

    // Detach the item from its current parent
    unlink(item);

    // Apply new name if provided
    fetch(item).mutate().setName(name);

    // Insert into the destination directory
    link(dest, item);
}
*/

/*
void
FileSystem::copy(BlockNr item, BlockNr dest)
{
    copy(item, dest, fetch(item).name());
}

void
FileSystem::copy(BlockNr item, BlockNr dest, const FSName &name)
{
    require.file(item);
    require.directory(dest);

    // Read source file
    Buffer<u8> buffer; fetch(item).extractData(buffer);

    // Create file at destination
    createFile(dest, name, buffer);
}
*/

void
FileSystem::resize(BlockNr at, isize size)
{
    // Extract file data
    Buffer<u8> buffer; fetch(at).extractData(buffer);

    // Adjust size (pads with zero when growing)
    buffer.resize(size, 0);

    // Write resized data back
    replace(at, buffer);
}

void
FileSystem::replace(BlockNr at, const u8 *buf, isize size)
{
    // Collect all blocks occupied by this file
    auto blocks = collectDataBlocks(at);

    // Compute how many blocks we need
    auto needed = allocator.requiredBlocks(size);

    // Allocate additional blocks if necessary
    if (needed > isize(blocks.size())) {

        auto more = allocator.allocate(needed - isize(blocks.size()));
        blocks.insert(blocks.end(), more.begin(), more.end());
    }

    // Update the file contents
    replace(blocks, buf, size);
}

void
FileSystem::replace(BlockNr at, const Buffer<u8> &data)
{
    replace(at, data.ptr, data.size);
}

void
FileSystem::replace(BlockNr at, const string &str)
{
    replace(at, (const u8 *)str.c_str(), (isize)str.size());
}

void
FileSystem::replace(std::vector<BlockNr> blocks, const u8 *buf, isize size)
{
    for (usize i = 0; i < blocks.size() && size > 0; ++i) {

        auto &block = fetch(blocks[i]).mutate();
        auto written = std::min(size, isize(254));
        std::memcpy(block.data(), buf, written);
        buf += written;
        size -= written;
    }

    assert(size == 0);
}

/*
BlockNr
FileSystem::newUserDirBlock(const PETName<16> &name)
{
    BlockNr nr = allocator.allocate();

    auto &node = fetch(nr).mutate();
    node.init(FSBlockType::USERDIR);
    node.setName(name);

    return nr;
}

BlockNr
FileSystem::newFileHeaderBlock(const PETName<16> &name)
{
    BlockNr nr = allocator.allocate();

    auto &node = fetch(nr).mutate();
    node.init(FSBlockType::FILEHEADER);
    node.setName(name);

    return nr;
}

void
FileSystem::addFileListBlock(BlockNr at, BlockNr head, BlockNr prev)
{
    auto &node = fetch(at).mutate();
    auto &prevNode = fetch(prev).mutate();

    node.init(FSBlockType::FILELIST);
    node.setFileHeaderRef(head);

    prevNode.setNextListBlockRef(at);
}

void
FileSystem::addDataBlock(BlockNr at, BlockNr id, BlockNr head, BlockNr prev)
{
    auto &node = fetch(at).mutate();
    auto &prevNode = fetch(prev).mutate();

    node.init(FSBlockType::DATA);
    node.setDataBlockNr(id);
    node.setFileHeaderRef(head);

    prevNode.setNextDataBlockRef(at);
}

isize
FileSystem::addData(BlockNr nr, const u8 *buf, isize size)
{
    auto &block = fetch(nr).mutate();
    isize count = 0;

    switch (block.type) {

        case FSBlockType::DATA:

            count = std::min(traits.bsize, size);
            std::memcpy(block.data(), buf, count);
            break;

        default:
            break;
    }

    return count;
}
*/

void
FileSystem::reclaim(BlockNr fhb)
{
    /*
    auto &node = fetch(fhb);

    if (node.isDirectory()) {

        // Remove user directory block
        cache.erase(node.nr); allocator.markAsFree(node.nr);
        return;
    }

    if (node.isFile()) {

        // Collect all blocks occupied by this file
        auto dataBlocks = collectDataBlocks(node.nr);
        auto listBlocks = collectListBlocks(node.nr);

        // Remove all blocks
        cache.erase(node.nr); allocator.markAsFree(node.nr);
        for (auto &it : dataBlocks) { cache.erase(it); allocator.markAsFree(it); }
        for (auto &it : listBlocks) { cache.erase(it); allocator.markAsFree(it); }
        return;
    }

    throw FSError(FSError::FS_NOT_A_FILE_OR_DIRECTORY);
    */
}

vector<BlockNr>
FileSystem::collectDirBlocks() const
{
    return collect(*traits.blockNr(18, 1), [&](const FSBlock *node) {
        return tryFetch(node->tsLink());
    });
}

vector<BlockNr>
FileSystem::collectDataBlocks(BlockNr ref) const
{
    return collect(ref, [&](const FSBlock *node) {
        return tryFetch(node->tsLink());
    });
}

vector<BlockNr>
FileSystem::collectDataBlocks(const FSDirEntry &entry) const
{
    if (auto b = traits.blockNr(entry.firstBlock()))
        return collectDataBlocks(*b);

    return {};
}

std::vector<const FSBlock *>
FileSystem::collect(const FSBlock &node, BlockIterator succ) const noexcept
{
    std::vector<const FSBlock *> result;
    std::unordered_set<BlockNr> visited;

    for (auto *block = tryFetch(node.nr); block != nullptr; block = succ(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

void
FileSystem::chainBlocks(TSLink from, TSLink to)
{
    auto &block = fetch(from).mutate();
    auto *data  = block.data();

    data[0] = u8(to.t);
    data[1] = u8(to.s);
}

std::vector<BlockNr>
FileSystem::collect(const BlockNr nr, BlockIterator succ) const noexcept
{
    std::vector<BlockNr> result;
    std::unordered_set<BlockNr> visited;

    for (auto *block = tryFetch(nr); block; block = succ(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block->nr);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

/*
std::vector<const FSBlock *>
FileSystem::collectDataBlocks(const FSBlock &node) const
{
    return collect(node, [&](const FSBlock *node) {
        return tryFetch(node->tsLink());
    });
}

std::vector<const FSBlock *>
FileSystem::collectListBlocks(const FSBlock &node) const
{
    std::vector<const FSBlock *> result;

    if (auto *ptr = node.getNextListBlock()) {
        result = collect(*ptr, [&](auto *block) { return block->getNextListBlock(); });
    }
    return result;
}

std::vector<BlockNr>
FileSystem::collectListBlocks(const BlockNr ref) const
{
    std::vector<BlockNr> result;

    if (auto *ptr = tryFetch(ref)) {
        for (auto &it: collectDataBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<BlockNr>
FileSystem::collectHashedBlocks(BlockNr ref, isize bucket) const
{
    std::vector<BlockNr> result;

    if (auto *ptr = tryFetch(ref)) {
        for (auto &it: collectHashedBlocks(*ptr, bucket)) result.push_back(it->nr);
    }
    return result;
}

std::vector<const FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node, isize bucket) const
{
    auto first = node.getHashRef((u32)bucket);
    if (auto *ptr = tryFetch(first, { FSBlockType::USERDIR, FSBlockType::FILEHEADER }); ptr) {
        return collect(*ptr, [&](auto *p) { return p->getNextHashBlock(); });
    } else {
        return {};
    }
}

std::vector<BlockNr>
FileSystem::collectHashedBlocks(BlockNr ref) const
{
    std::vector<BlockNr> result;
    if (auto *ptr = tryFetch(ref)) {
        for (auto &it: collectHashedBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<const FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node) const
{
    std::vector<const FSBlock *> result;

    // Walk through all hash table buckets in reverse order
    for (isize i = (isize)node.hashTableSize() - 1; i >= 0; i--) {
        for (auto &it : collectHashedBlocks(node, i)) result.push_back(it);
    }
    return result;
}
*/

}
