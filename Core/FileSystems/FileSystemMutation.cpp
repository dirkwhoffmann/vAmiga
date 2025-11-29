// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IOUtils.h"
#include "FileSystem.h"
#include "Host.h"
#include "MemUtils.h"
#include <climits>
#include <unordered_set>
#include <stack>

namespace vamiga {


/*
void
FileSystem::init(Diameter dia, Density den, FSFormat dos, const fs::path &path)
{
    // Get a device descriptor
    auto descriptor = FSDescriptor(dia, den, dos);
    
    // Create the device
    init(descriptor, path);
}
*/



/*
isize
FileSystem::requiredDataBlocks(isize fileSize) const
{
    // Compute the capacity of a single data block
    isize numBytes = traits.bsize - (traits.ofs() ? 24 : 0);

    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

isize
FileSystem::requiredFileListBlocks(isize fileSize) const
{
    // Compute the required number of data blocks
    isize numBlocks = requiredDataBlocks(fileSize);
    
    // Compute the number of data block references in a single block
    isize numRefs = (traits.bsize / 4) - 56;

    // Small files do not require any file list block
    if (numBlocks <= numRefs) return 0;
    
    // Compute the required number of additional file list blocks
    return (numBlocks - 1) / numRefs;
}

isize
FileSystem::requiredBlocks(isize fileSize) const
{
    isize numDataBlocks = requiredDataBlocks(fileSize);
    isize numFileListBlocks = requiredFileListBlocks(fileSize);
    
    debug(FS_DEBUG, "Required file header blocks : %d\n",  1);
    debug(FS_DEBUG, "       Required data blocks : %ld\n", numDataBlocks);
    debug(FS_DEBUG, "  Required file list blocks : %ld\n", numFileListBlocks);
    
    return 1 + numDataBlocks + numFileListBlocks;
}
*/

#if 0
bool
FileSystem::allocatable(isize count) const
{
    Block i = ap;
    isize capacity = numBlocks();
    
    while (count > 0) {

        if (storage.getType(Block(i)) == FSBlockType::EMPTY) {
            if (--count == 0) break;
        }
        
        i = (i + 1) % capacity;
        if (i == ap) return false;
    }
    
    return true;
}

Block
FileSystem::allocate()
{
    Block i = ap;
    
    while (!isEmpty(i)) {
        
        if ((i = (i + 1) % numBlocks()) == ap) {
            
            debug(FS_DEBUG, "No more free blocks\n");
            throw AppError(Fault::FS_OUT_OF_SPACE);
        }
    }
    
    read(i)->type = FSBlockType::UNKNOWN;
    allocator.markAsAllocated(i);
    ap = (i + 1) % numBlocks();
    return i;
}

void
FileSystem::allocate(isize count, std::vector<Block> &result, std::vector<Block> prealloc)
{
    /* Allocate multiple blocks and return them in `result`.
     *
     * Parameters:
     *
     * count    – number of blocks to allocate
     * result   – vector to store the allocated blocks
     * prealloc – optional list of pre-allocated blocks. If not empty, these
     *            blocks are used first: the allocator moves blocks from
     *            `prealloc` into `result` until `prealloc` is empty.
     *            Remaining blocks (if any) are allocated normally.
     *
     * Notes:
     *
     * - The function does not modify `prealloc` outside of moving blocks.
     * - Guarantees that `result` contains exactly `count` blocks upon return.
     */

    // Step 1: Use pre-allocated blocks first
    while (!prealloc.empty() && count > 0) {

        result.push_back(prealloc.back());
        prealloc.pop_back();
        count--;
    }

    // Step 2: Allocate remaining blocks from free space
    Block i = ap;
    while (count > 0) {

        if (isEmpty(i)) {

            read(i)->type = FSBlockType::UNKNOWN;
            result.push_back(i);
            count--;
        }

        // Move to the next block
        i = (i + 1) % numBlocks();

        // Fail if we looped all the way and still need blocks
        if (i == ap && count > 0) {

            debug(FS_DEBUG, "No more free blocks\n");
            throw AppError(Fault::FS_OUT_OF_SPACE);
        }
    }

    // Step 3: Mark all blocks as allocated
    for (const auto &b : result) allocator.markAsAllocated(b);

    // Step 4: Advance allocation pointer
    ap = i;
}

void
FileSystem::deallocateBlock(Block nr)
{
    storage[nr].init(FSBlockType::EMPTY);
    allocator.markAsFree(nr);
}

void
FileSystem::deallocateBlocks(const std::vector<Block> &nrs)
{
    for (Block nr : nrs) { deallocateBlock(nr); }
}
#endif


/*
void
FileSystem::setAllocationBit(Block nr, bool value)
{
    isize byte, bit;
    
    if (FSBlock *bm = locateAllocationBit(nr, &byte, &bit)) {
        REPLACE_BIT(bm->data()[byte], bit, value);
    }
}

void
FileSystem::rectifyAllocationMap()
{
    for (isize i = 0, max = numBlocks(); i < max; i++) {
        
        auto free = isUnallocated(Block(i));
        auto empty = isEmpty(Block(i));

        if (empty && !free) {
            markAsFree(Block(i));
        }
        if (!empty && free) {
            markAsAllocated(Block(i));
        }
    }
}
*/

/*
FSBlock &
FileSystem::link(FSBlock &at, const FSName &name)
{
    FSBlock &fhb = newFileHeaderBlock(name);

    try {
        link(at, name, fhb);
    } catch(...) {
        allocator.deallocateBlock(fhb.nr);
        throw;
    }

    return fhb;
}

void
FileSystem::link(FSBlock &at, const FSName &name, FSBlock &fhb)
{
    // Files can only be linked to directories
    if (!at.isDirectory()) throw AppError(Fault::FS_NOT_A_DIRECTORY);

    // Error out if the file already exists
    if (seekPtr(&at, name)) throw(AppError(Fault::FS_EXISTS, name.cpp_str()));

    // Wire up
    fhb.setParentDirRef(at.nr);
    addToHashTable(at.nr, fhb.nr);
}
*/

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
FileSystem::addToHashTable(const FSBlock &item)
{
    addToHashTable(item.getParentDirRef(), item.nr);
}

void
FileSystem::addToHashTable(Block parent, Block ref)
{
    FSBlock *pp = read(parent);
    if (!pp) throw AppError(Fault::FS_OUT_OF_RANGE);
    if (!pp->hasHashTable()) throw AppError(Fault::FS_WRONG_BLOCK_TYPE);

    FSBlock *pr = read(ref);
    if (!pr) throw AppError(Fault::FS_OUT_OF_RANGE);
    if (!pr->isHashable()) throw AppError(Fault::FS_WRONG_BLOCK_TYPE);

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
    if (!pp) throw AppError(Fault::FS_OUT_OF_RANGE);
    if (!pp->hasHashTable()) throw AppError(Fault::FS_WRONG_BLOCK_TYPE);

    FSBlock *pr = read(ref);
    if (!pr) throw AppError(Fault::FS_OUT_OF_RANGE);
    if (!pr->isHashable()) throw AppError(Fault::FS_WRONG_BLOCK_TYPE);

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

/*
void
FileSystem::importVolume(const u8 *src, isize size)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % traits.bsize != 0) throw AppError(Fault::FS_WRONG_BSIZE);

    // Only proceed if the source buffer contains the right amount of data
    if (numBytes() != size) throw AppError(Fault::FS_WRONG_CAPACITY);

    // Only proceed if all partitions contain a valid file system
    if (traits.dos == FSFormat::NODOS) throw AppError(Fault::FS_UNSUPPORTED);

    // Import all blocks
    for (isize i = 0; i < numBlocks(); i++) {
        
        const u8 *data = src + i * traits.bsize;

        // Determine the type of the new block
        if (FSBlockType type = predictType((Block)i, data); type != FSBlockType::EMPTY) {

            // Create new block
            storage[i].init(type);
            storage[i].importBlock(data, traits.bsize);
        }
    }
    
    // Print some debug information
    debug(FS_DEBUG, "Success\n");
}

void
FileSystem::import(const fs::path &path, bool recursive, bool contents)
{
    import(pwd(), path, recursive, contents);
}

void
FileSystem::import(FSBlock &top, const fs::path &path, bool recursive, bool contents)
{
    fs::directory_entry dir;

    // Get the directory item
    try { dir = fs::directory_entry(path); } catch (...) {
        throw AppError(Fault::FILE_CANT_READ);
    }

    if (dir.is_directory() && contents) {

        // Add the directory contents
        for (const auto& it : fs::directory_iterator(dir)) import(top, it, recursive);

    } else {

        // Add the file or directory as a whole
        import(top, dir, recursive);
    }

    // Rectify the checksums of all blocks
    updateChecksums();

    // Verify the result
    if (FS_DEBUG) doctor.xray(true, std::cout, false);
}

void
FileSystem::import(FSBlock &top, const fs::directory_entry &entry, bool recursive)
{
    auto isHidden = [&](const fs::path &path) {

        string s = path.filename().string();
        return !s.empty() && s[0] == '.';
    };

    const auto path = entry.path().string();
    const auto name = entry.path().filename();
    FSName fsname = FSName(name);

    // Skip hidden files
    if (isHidden(name)) return;

    if (entry.is_regular_file()) {

        debug(FS_DEBUG > 1, "  Importing file %s\n", path.c_str());

        Buffer<u8> buffer(entry.path());
        if (buffer) {
            createFile(top, fsname, buffer.ptr, buffer.size);
        } else {
            createFile(top, fsname);
        }

    } else {

        debug(FS_DEBUG > 1, "Importing directory %s\n", fsname.c_str());

        // Create new directory
        auto &subdir = createDir(top, fsname);

        // Import all items
        for (const auto& it : fs::directory_iterator(entry)) {

            if (it.is_regular_file() || recursive) import(subdir, it, recursive);
        }
    }
}

bool
FileSystem::exportVolume(u8 *dst, isize size) const
{
    return exportBlocks(0, (Block)(numBlocks() - 1), dst, size);
}

bool
FileSystem::exportVolume(u8 *dst, isize size, Fault *err) const
{
    return exportBlocks(0, (Block)(numBlocks() - 1), dst, size, err);
}

bool
FileSystem::exportBlock(Block nr, u8 *dst, isize size) const
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FileSystem::exportBlock(Block nr, u8 *dst, isize size, Fault *error) const
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FileSystem::exportBlocks(Block first, Block last, u8 *dst, isize size) const
{
    Fault error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == Fault::OK));
    return result;
}

bool
FileSystem::exportBlocks(Block first, Block last, u8 *dst, isize size, Fault *err) const
{
    assert(last < (Block)numBlocks());
    assert(first <= last);
    assert(dst);
    
    isize count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %ld blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % traits.bsize != 0) {
        if (err) *err = Fault::FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * traits.bsize != size) {
        if (err) *err = Fault::FS_WRONG_CAPACITY;
        return false;
    }

    // Wipe out the target buffer
    std::memset(dst, 0, size);
    
    // Export all blocks
    for (auto &block: storage.keys(first, last)) {

        storage.read(block)->exportBlock(dst + (block - first) * traits.bsize, traits.bsize);
    }

    debug(FS_DEBUG, "Success\n");
    if (err) *err = Fault::OK;
    return true;
}

void
FileSystem::exportBlock(Block nr, const fs::path &path) const
{
    exportBlocks(nr, nr, path);
}

void
FileSystem::exportBlocks(Block first, Block last, const fs::path &path) const
{
    std::ofstream stream(path, std::ios::binary);

    if (!stream.is_open()) {
        throw AppError(Fault::FILE_CANT_CREATE, path);
    }

    for (Block i = first; i <= last; i++) {

        auto *data = at(i).data();
        stream.write((const char *)data, traits.bsize);
    }

    if (!stream) {
        throw AppError(Fault::FILE_CANT_WRITE, path);
    }
}

void
FileSystem::exportBlocks(const fs::path &path) const
{
    if (traits.blocks) {
        exportBlocks(0, Block(traits.blocks - 1), path);
    }
}

void
FileSystem::exportFiles(Block nr, const fs::path &path, bool recursive, bool contents) const
{
    const FSBlock *block = read(nr);
    exportFiles(*block, path, recursive, contents);

}

void
FileSystem::exportFiles(const FSBlock &item, const fs::path &path, bool recursive, bool contents) const
{
    fs::path hostPath;

    if (item.isDirectory()) {

        hostPath = contents ? path : path / item.cppName();
        if (!fs::exists(hostPath)) fs::create_directories(hostPath);

    } else if (item.isFile())  {

        hostPath = fs::is_directory(path) ? path / item.cppName() : path;
    }

    debug(FS_DEBUG, "Exporting %s to %s\n", item.absName().c_str(), hostPath.string().c_str());
    FSTree tree(item, { .recursive = recursive });
    tree.save(hostPath, { .recursive = recursive });
}

void
FileSystem::exportFiles(const fs::path &path, bool recursive, bool contents) const
{
    exportFiles(pwd(), path, recursive, contents);
}

void
FileSystem::importBlock(Block nr, const fs::path &path)
{
    std::ifstream stream(path, std::ios::binary);

    if (!stream.is_open()) {
        throw AppError(Fault::FILE_CANT_READ, path);
    }

    auto *data = at(nr).data();
    stream.read((char *)data, traits.bsize);

    if (!stream) {
        throw AppError(Fault::FILE_CANT_READ, path);
    }
}
*/

}
