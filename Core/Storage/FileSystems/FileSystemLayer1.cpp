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

    require::initialized(*this);

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
    require::directory(at);

    // Error out if the file already exists
    if (searchdir(at, name)) throw(FSError(fault::FS_EXISTS, name.cpp_str()));

    FSBlock &block = newUserDirBlock(name);
    block.setParentDirRef(at.nr);
    addToHashTable(at.nr, block.nr);

    return block;
}

void
FileSystem::rmdir(FSBlock &at)
{
    require::emptyDirectory(at);

    deleteFromHashTable(at);
    reclaim(at);
}

FSBlock *
FileSystem::searchdir(const FSBlock &at, const FSName &name)
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
    require::notExist(at, fhb.name());

    // Wire
    fhb.setParentDirRef(at.nr);
    addToHashTable(at.nr, fhb.nr);
}

void
FileSystem::unlink(const FSBlock &node)
{
    require::fileOrDirectory(node);

    // Unwire
    deleteFromHashTable(node);
}

void
FileSystem::addToHashTable(const FSBlock &item)
{
    addToHashTable(item.getParentDirRef(), item.nr);
}

void
FileSystem::addToHashTable(Block parent, Block ref)
{
    FSBlock *pp = read(parent);
    if (!pp) throw FSError(fault::FS_OUT_OF_RANGE);
    if (!pp->hasHashTable()) throw FSError(fault::FS_WRONG_BLOCK_TYPE);

    FSBlock *pr = read(ref);
    if (!pr) throw FSError(fault::FS_OUT_OF_RANGE);
    if (!pr->isHashable()) throw FSError(fault::FS_WRONG_BLOCK_TYPE);

    // Read the linked list from the proper hash-table bucket
    u32 hash = pr->hashValue() % pp->hashTableSize();
    auto chain = collectHashedBlocks(pp->nr, hash);

    if (chain.empty()) {

        // If the bucket is empty, make the reference the first entry
        pp->setHashRef(hash, ref);
        pp->updateChecksum();

    } else {

        // Otherwise, put the reference at the end of the linked list
        read(chain.back())->setNextHashRef(ref);
        read(chain.back())->updateChecksum();
    }
}

void
FileSystem::deleteFromHashTable(const FSBlock &item)
{
    deleteFromHashTable(item.getParentDirRef(), item.nr);
}

void
FileSystem::deleteFromHashTable(Block parent, Block ref)
{
    FSBlock *pp = read(parent);
    if (!pp) throw FSError(fault::FS_OUT_OF_RANGE);
    if (!pp->hasHashTable()) throw FSError(fault::FS_WRONG_BLOCK_TYPE);

    FSBlock *pr = read(ref);
    if (!pr) throw FSError(fault::FS_OUT_OF_RANGE);
    if (!pr->isHashable()) throw FSError(fault::FS_WRONG_BLOCK_TYPE);

    // Read the linked list from the proper hash-table bucket
    u32 hash = pr->hashValue() % pp->hashTableSize();
    auto chain = collectHashedBlocks(pp->nr, hash);

    // Find the element
    if (auto it = std::find(chain.begin(), chain.end(), ref); it != chain.end()) {

        auto pred = it != chain.begin() ? *(it - 1) : 0;
        auto succ = (it + 1) != chain.end() ? *(it + 1) : 0;

        // Remove the element from the list
        if (!pred) {

            pp->setHashRef(hash, succ);
            pp->updateChecksum();

        } else {

            read(pred)->setNextHashRef(succ);
            read(pred)->updateChecksum();
        }
    }
}

FSBlock &
FileSystem::createFile(FSBlock &at, const FSName &name)
{
    require::directory(at);

    FSBlock &fhb = newFileHeaderBlock(name);

    try {

        link(at, fhb);
        return fhb;

    } catch(...) {

        allocator.deallocateBlock(fhb.nr);
        throw;
    }
}

FSBlock &
FileSystem::createFile(FSBlock &at, const FSName &name, const u8 *buf, isize size)
{
    // Create an empty file
    auto &fhb = createFile(at, name);

    // Add data
    return replace(fhb, buf, size);
}

FSBlock &
FileSystem::createFile(FSBlock &at, const FSName &name, const Buffer<u8> &buf)
{
    return createFile(at, name, buf.ptr, buf.size);
}

FSBlock &
FileSystem::createFile(FSBlock &top, const FSName &name, const string &str)
{
    return createFile(top, name, (const u8 *)str.c_str(), (isize)str.size());
}

void
FileSystem::rm(const FSBlock &node)
{
    // Remove the file from its parent directory
    unlink(node);

    // Reclaim all associated storage blocks
    reclaim(node);
}

void
FileSystem::rename(FSBlock &item, const FSName &name)
{
    // Renaming the root updates the file system name
    if (item.isRoot()) { setName(name); return; }

    // For regular items, relocate entry in the parent directory
    move(item, *item.getParentDirBlock(), name);
}

void
FileSystem::move(FSBlock &item, FSBlock &dest)
{
    move (item, dest, item.name());
}

void
FileSystem::move(FSBlock &item, FSBlock &dest, const FSName &name)
{
    require::fileOrDirectory(item);
    require::notExist(dest, name);

    // Detach the item from its current parent
    unlink(item);

    // Apply new name if provided
    item.setName(name);

    // Insert into the destination directory
    link(dest, item);
}

void
FileSystem::copy(const FSBlock &item, FSBlock &dest)
{
    copy(item, dest, item.cppName());
}

void
FileSystem::copy(const FSBlock &item, FSBlock &dest, const FSName &name)
{
    require::file(item);
    require::directory(dest);

    // Read source file
    Buffer<u8> buffer; item.extractData(buffer);

    // Create file at destination
    createFile(dest, name, buffer);
}

void
FileSystem::resize(FSBlock &at, isize size)
{
    // Extract file data
    Buffer<u8> buffer; at.extractData(buffer);

    // Adjust size (pads with zero when growing)
    buffer.resize(size, 0);

    // Write resized data back
    replace(at, buffer);
}

void
FileSystem::replace(FSBlock &at, const Buffer<u8> &data)
{
    // Collect all blocks occupied by this file
    auto listBlocks = collectListBlocks(at.nr);
    auto dataBlocks = collectDataBlocks(at.nr);

    // Update the file contents
    replace(at, data.ptr, data.size, listBlocks, dataBlocks);
}

FSBlock &
FileSystem::replace(FSBlock &fhb,
                       const u8 *buf, isize size,
                       std::vector<Block> listBlocks,
                       std::vector<Block> dataBlocks)
{
    // Number of data block references held in a file header or list block
    const isize numRefs = ((traits.bsize / 4) - 56);

    // Start with a clean reference area
    fhb.setNextListBlockRef(0);
    fhb.setNextDataBlockRef(0);
    for (isize i = 0; i < numRefs; i++) fhb.setDataBlockRef(i, 0);

    // Set file size
    fhb.setFileSize(u32(size));

    // Allocate blocks
    allocator.allocateFileBlocks(size, listBlocks, dataBlocks);

    for (usize i = 0; i < listBlocks.size(); i++) {

        // Add a list block
        addFileListBlock(listBlocks[i], fhb.nr, i == 0 ? fhb.nr : listBlocks[i-1]);
    }

    for (isize i = 0; i < (isize)dataBlocks.size(); i++) {

        // Add a data block
        addDataBlock(dataBlocks[i], i + 1, fhb.nr, i == 0 ? fhb.nr : dataBlocks[i-1]);

        // Determine the list block managing this data block
        FSBlock *lb = read((i < numRefs) ? fhb.nr : listBlocks[i / numRefs - 1]);

        // Link the data block
        lb->addDataBlockRef(dataBlocks[0], dataBlocks[i]);

        // Add data bytes
        isize written = addData(dataBlocks[i], buf, size);
        buf += written;
        size -= written;
    }

    // Rectify checksums
    for (auto &it : listBlocks) { at(it).updateChecksum(); }
    for (auto &it : dataBlocks) { at(it).updateChecksum(); }
    fhb.updateChecksum();

    return fhb;
}

FSBlock &
FileSystem::newUserDirBlock(const FSName &name)
{
    Block nr = allocator.allocate();

    storage[nr].init(FSBlockType::USERDIR);
    storage[nr].setName(name);
    return at(nr);
}

FSBlock &
FileSystem::newFileHeaderBlock(const FSName &name)
{
    Block nr = allocator.allocate();

    storage[nr].init(FSBlockType::FILEHEADER);
    storage[nr].setName(name);
    return at(nr);
}

void
FileSystem::addFileListBlock(Block at, Block head, Block prev)
{
    if (auto *prevBlock = read(prev); prevBlock) {

        storage[at].init(FSBlockType::FILELIST);
        storage[at].setFileHeaderRef(head);

        prevBlock->setNextListBlockRef(at);
    }
}

void
FileSystem::addDataBlock(Block at, isize id, Block head, Block prev)
{
    if (auto *prevBlock = read(prev); prevBlock) {

        storage[at].init(traits.ofs() ? FSBlockType::DATA_OFS : FSBlockType::DATA_FFS);
        storage[at].setDataBlockNr((Block)id);
        storage[at].setFileHeaderRef(head);
        prevBlock->setNextDataBlockRef(at);
    }
}

isize
FileSystem::addData(Block nr, const u8 *buf, isize size)
{
    FSBlock *block = read(nr);
    return block ? addData(*block, buf, size) : 0;
}

isize
FileSystem::addData(FSBlock &block, const u8 *buf, isize size)
{
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
FileSystem::reclaim(const FSBlock &node)
{
    if (node.isDirectory()) {

        // Remove user directory block
        storage.erase(node.nr); allocator.markAsFree(node.nr);
        return;
    }

    if (node.isFile()) {

        // Collect all blocks occupied by this file
        auto dataBlocks = collectDataBlocks(node.nr);
        auto listBlocks = collectListBlocks(node.nr);

        // Remove all blocks
        storage.erase(node.nr); allocator.markAsFree(node.nr);
        for (auto &it : dataBlocks) { storage.erase(it); allocator.markAsFree(it); }
        for (auto &it : listBlocks) { storage.erase(it); allocator.markAsFree(it); }
        return;
    }

    throw FSError(fault::FS_NOT_A_FILE_OR_DIRECTORY, node.absName());
}

std::vector<const FSBlock *>
FileSystem::collect(const FSBlock &node, std::function<const FSBlock *(const FSBlock *)> next) const
{
    std::vector<const FSBlock *> result;
    std::unordered_set<Block> visited;

    for (auto block = read(node.nr); block != nullptr; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

std::vector<Block>
FileSystem::collect(const Block nr, std::function<const FSBlock *(FSBlock const *)> next) const
{
    std::vector<Block> result;
    std::unordered_set<Block> visited;

    for (auto block = read(nr); block; block = next(block)) {

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

std::vector<Block>
FileSystem::collectDataBlocks(Block ref) const
{
    std::vector<Block> result;

    if (auto *ptr = read(ref)) {
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

std::vector<Block>
FileSystem::collectListBlocks(const Block ref) const
{
    std::vector<Block> result;

    if (auto *ptr = read(ref)) {
        for (auto &it: collectDataBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<Block>
FileSystem::collectHashedBlocks(Block ref, isize bucket) const
{
    std::vector<Block> result;

    if (auto *ptr = read(ref)) {
        for (auto &it: collectHashedBlocks(*ptr, bucket)) result.push_back(it->nr);
    }
    return result;
}

std::vector<const FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node, isize bucket) const
{
    auto first = node.getHashRef((u32)bucket);
    if (auto *ptr = read(first, { FSBlockType::USERDIR, FSBlockType::FILEHEADER }); ptr) {
        return collect(*ptr, [&](auto *p) { return p->getNextHashBlock(); });
    } else {
        return {};
    }
}

std::vector<Block>
FileSystem::collectHashedBlocks(Block ref) const
{
    std::vector<Block> result;
    if (auto *ptr = read(ref)) {
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
