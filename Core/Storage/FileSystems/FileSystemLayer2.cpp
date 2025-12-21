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
    current = rootBlock;
}

void
FileSystem::setName(const FSName &name)
{
    if (auto &rb = fetch(rootBlock); rb.isRoot()) {

        rb.mutate().setName(name);
        rb.mutate().updateChecksum();
    }
}

void
FileSystem::makeBootable(BootBlockId id)
{
    assert(cache.getType(0) == FSBlockType::BOOT);
    assert(cache.getType(1) == FSBlockType::BOOT);
    fetch(0).mutate().writeBootBlock(id, 0);
    fetch(1).mutate().writeBootBlock(id, 1);
}

void
FileSystem::killVirus()
{
    assert(cache.getType(0) == FSBlockType::BOOT);
    assert(cache.getType(1) == FSBlockType::BOOT);

    if (bootStat().hasVirus) {

        auto id =
        traits.ofs() ? BootBlockId::AMIGADOS_13 :
        traits.ffs() ? BootBlockId::AMIGADOS_20 : BootBlockId::NONE;

        if (id != BootBlockId::NONE) {
            fetch(0).mutate().writeBootBlock(id, 0);
            fetch(1).mutate().writeBootBlock(id, 1);
        } else {
            std::memset(fetch(0).mutate().data() + 4, 0, traits.bsize - 4);
            std::memset(fetch(1).mutate().data(), 0, traits.bsize);
        }
    }
}

BlockNr
FileSystem::mkdir(BlockNr at, const FSName &name)
{
    require.directory(at);

    // Error out if the file already exists
    if (searchdir(at, name)) throw(FSError(FSError::FS_EXISTS, name.cpp_str()));

    FSBlock &block = newUserDirBlock(name);
    block.setParentDirRef(at);
    addToHashTable(at, block.nr);

    return block.nr;
}

void
FileSystem::rmdir(BlockNr at)
{
    require.emptyDirectory(at);

    deleteFromHashTable(at);
    reclaim(at);
}

optional<BlockNr>
FileSystem::searchdir(BlockNr at, const FSName &name)
{
    std::unordered_set<BlockNr> visited;

    // Only proceed if a hash table is present
    auto &top = fetch(at);
    if (!top.hasHashTable()) return {};

    // Compute the table position and read the item
    u32 hash = name.hashValue(traits.dos) % top.hashTableSize();
    u32 ref = top.getHashRef(hash);

    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {

        auto *block = tryFetch(ref, { FSBlockType::USERDIR, FSBlockType::FILEHEADER });
        if (block == nullptr) break;

        if (block->isNamed(name)) return block->nr;

        visited.insert(ref);
        ref = block->getNextHashRef();
    }

    return {};
}

void
FileSystem::link(BlockNr at, BlockNr fhb)
{
    require.directory(at);

    // Read the file heade block
    auto &fhbBlk = fetch(fhb);

    // Only proceed if the file does not yet exist
    if (searchdir(at, fhbBlk.name())) throw FSError(FSError::FS_EXISTS);

    // Wire up
    fhbBlk.mutate().setParentDirRef(at);
    addToHashTable(at, fhb);
}

void
FileSystem::unlink(BlockNr node)
{
    require.fileOrDirectory(node);

    // Unwire
    deleteFromHashTable(node);
}

void
FileSystem::addToHashTable(BlockNr parent, BlockNr ref)
{
    auto &pp = fetch(parent);
    if (!pp.hasHashTable()) throw FSError(FSError::FS_WRONG_BLOCK_TYPE);

    auto &pr = fetch(ref);
    if (!pr.isHashable()) throw FSError(FSError::FS_WRONG_BLOCK_TYPE);

    // Read the linked list from the proper hash-table bucket
    u32 hash = pr.hashValue() % pp.hashTableSize();
    auto chain = collectHashedBlocks(pp.nr, hash);

    if (chain.empty()) {

        // If the bucket is empty, make the reference the first entry
        pp.mutate().setHashRef(hash, ref);
        pp.mutate().updateChecksum();

    } else {

        // Otherwise, put the reference at the end of the linked list
        auto &back = fetch(chain.back());
        back.mutate().setNextHashRef(ref);
        back.mutate().updateChecksum();
    }
}

void
FileSystem::deleteFromHashTable(BlockNr ref)
{
    auto &pr = fetch(ref);
    if (!pr.isHashable()) throw FSError(FSError::FS_WRONG_BLOCK_TYPE);

    auto &pp = fetch(pr.getParentDirRef());
    if (!pp.hasHashTable()) throw FSError(FSError::FS_WRONG_BLOCK_TYPE);

    // Read the linked list from the proper hash-table bucket
    u32 hash = pr.hashValue() % pp.hashTableSize();
    auto chain = collectHashedBlocks(pp.nr, hash);

    // Find the element
    if (auto it = std::find(chain.begin(), chain.end(), ref); it != chain.end()) {

        auto pred = it != chain.begin() ? *(it - 1) : 0;
        auto succ = (it + 1) != chain.end() ? *(it + 1) : 0;

        // Remove the element from the list
        if (!pred) {

            pp.mutate().setHashRef(hash, succ);
            pp.mutate().updateChecksum();

        } else {

            fetch(pred).mutate().setNextHashRef(succ);
            fetch(pred).mutate().updateChecksum();
        }
    }
}

BlockNr
FileSystem::createFile(BlockNr at, const FSName &name)
{
    require.directory(at);

    FSBlock &fhb = newFileHeaderBlock(name);

    try {

        link(at, fhb.nr);
        return fhb.nr;

    } catch(...) {

        allocator.deallocateBlock(fhb.nr);
        throw;
    }
}

BlockNr
FileSystem::createFile(BlockNr at, const FSName &name, const u8 *buf, isize size)
{
    // Create an empty file
    auto fhb = createFile(at, name);

    // Add data
    replace(fhb, buf, size);

    // Return the number of the file header block
    return fhb;
}

BlockNr
FileSystem::createFile(BlockNr at, const FSName &name, const Buffer<u8> &buf)
{
    return createFile(at, name, buf.ptr, buf.size);
}

BlockNr
FileSystem::createFile(BlockNr top, const FSName &name, const string &str)
{
    return createFile(top, name, (const u8 *)str.c_str(), (isize)str.size());
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
FileSystem::rename(BlockNr item, const FSName &name)
{
    auto &block = fetch(item);

    // Renaming the root updates the file system name
    if (block.isRoot()) { setName(name); return; }

    // For regular items, relocate entry in the parent directory
    move(item, block.getParentDirRef(), name);
}

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
    auto listBlocks = collectListBlocks(at);
    auto dataBlocks = collectDataBlocks(at);

    // Update the file contents
    replace(at, buf, size, listBlocks, dataBlocks);
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

BlockNr
FileSystem::replace(BlockNr fhb,
                    const u8 *buf, isize size,
                    std::vector<BlockNr> listBlocks,
                    std::vector<BlockNr> dataBlocks)
{
    auto &fhbNode = fetch(fhb).mutate();

    // Number of data block references held in a file header or list block
    const isize numRefs = ((traits.bsize / 4) - 56);

    // Start with a clean reference area
    fhbNode.setNextListBlockRef(0);
    fhbNode.setNextDataBlockRef(0);
    for (isize i = 0; i < numRefs; i++) fhbNode.setDataBlockRef(i, 0);

    // Set file size
    fhbNode.setFileSize(u32(size));

    // Allocate blocks
    allocator.allocateFileBlocks(size, listBlocks, dataBlocks);

    for (usize i = 0; i < listBlocks.size(); i++) {

        // Add a list block
        addFileListBlock(listBlocks[i], fhb, i == 0 ? fhb : listBlocks[i-1]);
    }

    for (isize i = 0; i < (isize)dataBlocks.size(); i++) {

        // Add a data block
        addDataBlock(dataBlocks[i], i + 1, fhb, i == 0 ? fhb : dataBlocks[i-1]);

        // Determine the list block managing this data block
        auto &lb = fetch((i < numRefs) ? fhb : listBlocks[i / numRefs - 1]);

        // Link the data block
        lb.mutate().addDataBlockRef(dataBlocks[0], dataBlocks[i]);

        // Add data bytes
        isize written = addData(dataBlocks[i], buf, size);
        buf += written;
        size -= written;
    }

    // Rectify checksums
    for (auto &it : listBlocks) { fetch(it).mutate().updateChecksum(); }
    for (auto &it : dataBlocks) { fetch(it).mutate().updateChecksum(); }
    fhbNode.updateChecksum();

    return fhb;
}

FSBlock &
FileSystem::newUserDirBlock(const FSName &name)
{
    BlockNr nr = allocator.allocate();

    auto &node = fetch(nr).mutate();
    node.init(FSBlockType::USERDIR);
    node.setName(name);

    return node;
}

FSBlock &
FileSystem::newFileHeaderBlock(const FSName &name)
{
    BlockNr nr = allocator.allocate();

    auto &node = fetch(nr).mutate();
    node.init(FSBlockType::FILEHEADER);
    node.setName(name);

    return node;
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
FileSystem::addDataBlock(BlockNr at, isize id, BlockNr head, BlockNr prev)
{
    auto &node = fetch(at).mutate();
    auto &prevNode = fetch(prev).mutate();

    node.init(traits.ofs() ? FSBlockType::DATA_OFS : FSBlockType::DATA_FFS);
    node.setDataBlockNr((BlockNr)id);
    node.setFileHeaderRef(head);

    prevNode.setNextDataBlockRef(at);
}

isize
FileSystem::addData(BlockNr nr, const u8 *buf, isize size)
{
    auto &block = fetch(nr).mutate();
    isize count = 0;

    switch (block.type) {

        case FSBlockType::DATA_OFS:

            count = std::min(traits.bsize - 24, size);
            std::memcpy(block.data() + 24, buf, count);
            block.setDataBytesInBlock((u32)count);
            block.updateChecksum();
            break;

        case FSBlockType::DATA_FFS:

            count = std::min(traits.bsize, size);
            std::memcpy(block.data(), buf, count);
            break;

        default:
            break;
    }

    return count;
}

void
FileSystem::reclaim(BlockNr fhb)
{
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

    throw FSError(FSError::FS_NOT_A_FILE_OR_DIRECTORY, node.absName());
}

std::vector<const FSBlock *>
FileSystem::collect(const FSBlock &node, std::function<const FSBlock *(const FSBlock *)> next) const
{
    std::vector<const FSBlock *> result;
    std::unordered_set<BlockNr> visited;

    for (auto *block = tryFetch(node.nr); block != nullptr; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

std::vector<BlockNr>
FileSystem::collect(const BlockNr nr, std::function<const FSBlock *(FSBlock const *)> next) const
{
    std::vector<BlockNr> result;
    std::unordered_set<BlockNr> visited;

    for (auto *block = tryFetch(nr); block; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block->nr);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

std::vector<const FSBlock *>
FileSystem::collectDataBlocks(const FSBlock &node) const
{
    // Gather all blocks containing data block references
    auto blocks = collectListBlocks(node);
    blocks.push_back(&node);

    // Setup the result vector
    std::vector<const FSBlock *> result;
    result.reserve(blocks.size() * node.getMaxDataBlockRefs());

    // Crawl through blocks and collect all data block references
    for (auto &it : blocks) {

        isize num = std::min(it->getNumDataBlockRefs(), it->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {
            if (auto *ptr = it->getDataBlock(i); ptr) {
                result.push_back(ptr);
            }
        }
    }
    return result;
}

std::vector<BlockNr>
FileSystem::collectDataBlocks(BlockNr ref) const
{
    std::vector<BlockNr> result;

    if (auto *ptr = tryFetch(ref)) {
        for (auto &it: collectDataBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
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

}
