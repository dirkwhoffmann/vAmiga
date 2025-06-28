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
#include "MutableFileSystem.h"
#include "MemUtils.h"
#include <climits>
#include <set>
#include <stack>

namespace vamiga {

void
MutableFileSystem::init(isize capacity)
{
    storage.init(capacity);
}

void
MutableFileSystem::init(const FileSystemDescriptor &layout, const fs::path &path)
{
    if (FS_DEBUG) { layout.dump(); }
    
    // Copy layout parameters
    dos         = layout.dos;
    bsize       = layout.bsize;
    numReserved = layout.numReserved;
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;

    // Create all blocks
    init(isize(layout.numBlocks));

    // Format the file system
    if (dos != FSVolumeType::NODOS) format();

    // Start allocating blocks at the middle of the disk
    ap = rootBlock;

    // Print some debug information
    if (FS_DEBUG) { dump(Category::State); }
    
    // Import files if applicable
    if (!path.empty()) {
        
        // Add all files
        import(oldRootDir(), path);

        // Assign device name
        setName(FSName(path.filename().string()));
    }
}

void
MutableFileSystem::init(Diameter dia, Density den, FSVolumeType dos, const fs::path &path)
{
    // Get a device descriptor
    auto descriptor = FileSystemDescriptor(dia, den, dos);
    
    // Create the device
    init(descriptor, path);
}

void
MutableFileSystem::format(FSVolumeType dos, string name)
{
    this->dos = dos;
    format(name);
}

void
MutableFileSystem::format(string name)
{
    if (!initialized()) throw AppError(Fault::FS_UNINITIALIZED);

    // Perform some consistency checks
    assert(numBlocks() > 2);
    assert(rootBlock > 0);

    // Trash all existing data
    storage.init(numBlocks());

    // Create boot blocks
    storage[0].init(FSBlockType::BOOT_BLOCK);
    storage[1].init(FSBlockType::BOOT_BLOCK);

    // Create the root block
    storage[rootBlock].init(FSBlockType::ROOT_BLOCK);

    // Create bitmap blocks
    for (auto& ref : bmBlocks) {
        
        // storage.write(ref, new FSBlock(this, ref, FSBlockType::BITMAP_BLOCK));
        storage[ref].init(FSBlockType::BITMAP_BLOCK);
    }

    // Add bitmap extension blocks
    Block pred = rootBlock;
    for (auto &ref : bmExtBlocks) {

        // storage.write(ref, new FSBlock(this, ref, FSBlockType::BITMAP_EXT_BLOCK));
        storage[ref].init(FSBlockType::BITMAP_EXT_BLOCK);
        storage[pred].setNextBmExtBlockRef(ref);
        pred = ref;
    }
    
    // Add all bitmap block references
    storage[rootBlock].addBitmapBlockRefs(bmBlocks);

    // Mark free blocks as free in the bitmap block
    for (isize i = 0; i < numBlocks(); i++) {
        if (storage.isEmpty(Block(i))) markAsFree(Block(i));
    }
    
    // Set the volume name
    if (name != "") setName(name);

    // Compute checksums for all blocks
    updateChecksums();

    // Set the current directory
    curr = rootBlock;
}

void
MutableFileSystem::setName(FSName name)
{
    if (auto *rb = storage.read(rootBlock, FSBlockType::ROOT_BLOCK); rb) {

        rb->setName(name);
        rb->updateChecksum();
    }
}

isize
MutableFileSystem::requiredDataBlocks(isize fileSize) const
{
    // Compute the capacity of a single data block
    isize numBytes = bsize - (isOFS() ? 24 : 0);
    
    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

isize
MutableFileSystem::requiredFileListBlocks(isize fileSize) const
{
    // Compute the required number of data blocks
    isize numBlocks = requiredDataBlocks(fileSize);
    
    // Compute the number of data block references in a single block
    isize numRefs = (bsize / 4) - 56;
    
    // Small files do not require any file list block
    if (numBlocks <= numRefs) return 0;
    
    // Compute the required number of additional file list blocks
    return (numBlocks - 1) / numRefs;
}

isize
MutableFileSystem::requiredBlocks(isize fileSize) const
{
    isize numDataBlocks = requiredDataBlocks(fileSize);
    isize numFileListBlocks = requiredFileListBlocks(fileSize);
    
    debug(FS_DEBUG, "Required file header blocks : %d\n",  1);
    debug(FS_DEBUG, "       Required data blocks : %ld\n", numDataBlocks);
    debug(FS_DEBUG, "  Required file list blocks : %ld\n", numFileListBlocks);
    
    return 1 + numDataBlocks + numFileListBlocks;
}

bool
MutableFileSystem::allocatable(isize count) const
{
    Block i = ap;
    isize capacity = numBlocks();
    
    while (count > 0) {

        if (storage.getType(Block(i)) == FSBlockType::EMPTY_BLOCK) {
            if (--count == 0) break;
        }
        
        i = (i + 1) % capacity;
        if (i == ap) return false;
    }
    
    return true;
}

Block
MutableFileSystem::allocate()
{
    Block i = ap;
    
    while (!isEmpty(i)) {
        
        if ((i = (i + 1) % numBlocks()) == ap) {
            
            debug(FS_DEBUG, "No more free blocks\n");
            throw AppError(Fault::FS_OUT_OF_SPACE);
        }
    }
    
    blockPtr(Block(i))->type = FSBlockType::UNKNOWN_BLOCK;
    markAsAllocated(Block(i));
    ap = (i + 1) % numBlocks();
    return (Block(i));
}

void
MutableFileSystem::allocate(isize count, std::vector<Block> &result)
{
    Block i = ap;
    
    // Try to find enough free blocks
    while (count > 0) {
        
        if (isEmpty(i)) {
            
            blockPtr(Block(i))->type = FSBlockType::UNKNOWN_BLOCK;
            result.push_back(Block(i));
            count--;
        }
                
        if ((i = (i + 1) % numBlocks()) == ap && count) {
        
            debug(FS_DEBUG, "No more free blocks\n");
            throw AppError(Fault::FS_OUT_OF_SPACE);
        }
    }
    
    // Success: Mark all blocks as allocated
    for (const auto &it : result) markAsAllocated(it);

    // Advance the allocation pointer
    ap = i;
}

void
MutableFileSystem::deallocateBlock(Block nr)
{
    assert(isBlockNumber(nr));

    storage[nr].init(FSBlockType::EMPTY_BLOCK);
    markAsFree(nr);
}

void
MutableFileSystem::addFileListBlock(Block at, Block head, Block prev)
{
    FSBlock *prevBlock = blockPtr(prev);

    if (prevBlock) {

        storage[at].init(FSBlockType::FILELIST_BLOCK);
        storage[at].setFileHeaderRef(head);

        prevBlock->setNextListBlockRef(at);
    }
}

void
MutableFileSystem::addDataBlock(Block at, isize id, Block head, Block prev)
{
    FSBlock *prevBlock = blockPtr(prev);

    if (prevBlock) {

        storage[at].init(isOFS() ? FSBlockType::DATA_BLOCK_OFS : FSBlockType::DATA_BLOCK_FFS);
        storage[at].setDataBlockNr((Block)id);
        storage[at].setFileHeaderRef(head);
        prevBlock->setNextDataBlockRef(at);
    }
}

FSBlock *
MutableFileSystem::newUserDirBlock(const FSName &name)
{
    if (Block nr = allocate()) {

        storage[nr].init(FSBlockType::USERDIR_BLOCK);
        storage[nr].setName(name);
        return blockPtr(nr);
    }
 
    return nullptr;
}

FSBlock *
MutableFileSystem::newFileHeaderBlock(const FSName &name)
{
    if (Block nr = allocate()) {

        storage[nr].init(FSBlockType::FILEHEADER_BLOCK);
        storage[nr].setName(name);
        return blockPtr(nr);
    }
    
    return nullptr;
}

void
MutableFileSystem::updateChecksums()
{
    for (isize i = 0; i < numBlocks(); i++) {
        storage[i].updateChecksum();
    }
}

void
MutableFileSystem::makeBootable(BootBlockId id)
{
    assert(storage.getType(0) == FSBlockType::BOOT_BLOCK);
    assert(storage.getType(1) == FSBlockType::BOOT_BLOCK);
    storage[0].writeBootBlock(id, 0);
    storage[1].writeBootBlock(id, 1);
}

void
MutableFileSystem::killVirus()
{
    assert(storage.getType(0) == FSBlockType::BOOT_BLOCK);
    assert(storage.getType(1) == FSBlockType::BOOT_BLOCK);

    auto id = isOFS() ? BootBlockId::AMIGADOS_13 : isFFS() ? BootBlockId::AMIGADOS_20 : BootBlockId::NONE;

    if (id != BootBlockId::NONE) {
        storage[0].writeBootBlock(id, 0);
        storage[1].writeBootBlock(id, 1);
    } else {
        std::memset(storage[0].data() + 4, 0, bsize - 4);
        std::memset(storage[1].data(), 0, bsize);
     }
}

void
MutableFileSystem::setAllocationBit(Block nr, bool value)
{
    isize byte, bit;
    
    if (FSBlock *bm = locateAllocationBit(nr, &byte, &bit)) {
        REPLACE_BIT(bm->data()[byte], bit, value);
    }
}

void
MutableFileSystem::rectifyAllocationMap()
{
    for (isize i = 0, max = numBlocks(); i < max; i++) {
        
        auto free = isFree(Block(i));
        auto empty = isEmpty(Block(i));

        if (empty && !free) {
            markAsFree(Block(i));
        }
        if (!empty && free) {
            markAsAllocated(Block(i));
        }
    }
}

FSBlock &
MutableFileSystem::createDir(const FSBlock &at, const FSName &name)
{
    if (at.isDirectory()) {

        if (FSBlock *block = newUserDirBlock(name); block) {

            block->setParentDirRef(at.nr);
            addToHashTable(at.nr, block->nr);
            return *block;
        }
        throw AppError(Fault::FS_OUT_OF_SPACE);
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, at.absName());
}

FSBlock &
MutableFileSystem::createFile(const FSBlock &at, const FSName &name)
{
    if (at.isDirectory()) {

        if (FSBlock *block = newFileHeaderBlock(name); block) {

            block->setParentDirRef(at.nr);
            addToHashTable(at.nr, block->nr);
            return *block;
        }
        throw AppError(Fault::FS_OUT_OF_SPACE);
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, at.absName());
}

FSBlock &
MutableFileSystem::createFile(const FSBlock &at, const FSName &name, const Buffer<u8> &buf)
{
    return createFile(at, name, buf.ptr, buf.size);
}

FSBlock &
MutableFileSystem::createFile(const FSBlock &at, const FSName &name, const u8 *buf, isize size)
{
    assert(buf);

    // Compute the number of data block references held in a file header or list block
    const usize numRefs = ((bsize / 4) - 56);

    // Create a file header block
    auto &file = createFile(at, name);

    // Set file size
    file.setFileSize(u32(size));

    // Allocate blocks
    std::vector<Block> listBlocks;
    std::vector<Block> dataBlocks;
    allocateFileBlocks(size, listBlocks, dataBlocks);

    for (usize i = 0; i < listBlocks.size(); i++) {

        // Add a list block
        addFileListBlock(listBlocks[i], file.nr, i == 0 ? file.nr : listBlocks[i-1]);
    }

    for (usize i = 0; i < dataBlocks.size(); i++) {

        // Add a data block
        addDataBlock(dataBlocks[i], i + 1, file.nr, i == 0 ? file.nr : dataBlocks[i-1]);

        // Determine the list block managing this data block
        FSBlock *lb = blockPtr((i < numRefs) ? file.nr : listBlocks[i / numRefs - 1]);

        // Link the data block
        lb->addDataBlockRef(dataBlocks[0], dataBlocks[i]);

        // Add data bytes
        isize written = addData(dataBlocks[i], buf, size);
        buf += written;
        size -= written;
    }

    return file;
}

FSBlock &
MutableFileSystem::createFile(const FSBlock &at, const FSName &name, const string &str)
{
    return createFile(at, name, (const u8 *)str.c_str(), (isize)str.size());
}

void
MutableFileSystem::rename(const FSNode &item, const FSName &name)
{
    // Renaming the root node renames the name of the file system
    if (item.isRoot()) { setName(name); return; }

    // For files and directories, reposition the item in the hash table
    move(item, item.parent(), name);
}

void
MutableFileSystem::move(const FSNode &item, const FSNode &dest, const FSName &name)
{
    if (!dest.isDirectory()) throw AppError(Fault::FS_NOT_A_DIRECTORY, dest.absName());

    // Remove the item from the hash table
    deleteFromHashTable(item);

    // Rename if a new name is provided
    if (name != "") item.ptr()->setName(name);

    // Add the item to the new hash table
    addToHashTable(dest.ref, item.ref);

    // Assign the new parent directory
    item.ptr()->setParentDirRef(dest.ref);
}

void
MutableFileSystem::copy(const FSBlock &item, const FSBlock &dest)
{
    copy(item, dest, item.pathName());
}

void
MutableFileSystem::copy(const FSBlock &item, const FSBlock &dest, const FSName &name)
{
    if (!item.isFile()) throw AppError(Fault::FS_NOT_A_FILE, item.absName());
    if (!dest.isDirectory()) throw AppError(Fault::FS_NOT_A_DIRECTORY, dest.absName());

    // Read the file
    Buffer<u8> buffer; item.writeData(buffer);

    // Recreate the file at the target location
    createFile(dest, name, buffer);
}

void
MutableFileSystem::deleteFile(const FSNode &item)
{
    if (!item.isFile()) return;

    // Collect all blocks occupied by this file
    auto blocks = dataBlocks(item);
    blocks.push_back(item.ref);

    // Remove the file from the hash table
    deleteFromHashTable(item);

    // Remove all blocks
    for (auto &it : blocks) {

        storage.erase(it);
        markAsFree(it);
    }
}

void
MutableFileSystem::addToHashTable(const FSBlock &item)
{
    addToHashTable(item.getParentDirRef(), item.nr);
}

void
MutableFileSystem::addToHashTable(Block parent, Block ref)
{
    FSBlock *pp = blockPtr(parent);
    if (!pp) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    if (!pp->hasHashTable()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    FSBlock *pr = blockPtr(ref);
    if (!pr) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    if (!pr->isHashable()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    // Read the linked list from the proper hash-table bucket
    u32 hash = pr->hashValue() % pp->hashTableSize();
    auto chain = hashBlockChain(pp->getHashRef(hash));

    // If the bucket is empty, make the reference the first entry
    if (chain.empty()) { pp->setHashRef(hash, ref); return; }

    // Otherwise, put the referecne at the end of the linked list
    blockPtr(chain.back())->setNextHashRef(ref);
}

void
MutableFileSystem::deleteFromHashTable(const FSNode &item)
{
    deleteFromHashTable(item.parent().ref, item.ref);
}

void
MutableFileSystem::deleteFromHashTable(Block parent, Block ref)
{
    FSBlock *pp = blockPtr(parent);
    if (!pp) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    if (!pp->hasHashTable()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    FSBlock *pr = blockPtr(ref);
    if (!pr) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    if (!pr->isHashable()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    // Read the linked list from the proper hash-table bucket
    u32 hash = pr->hashValue() % pp->hashTableSize();
    auto chain = hashBlockChain(pp->getHashRef(hash));

    // Find the element
    if (auto it = std::find(chain.begin(), chain.end(), ref); it != chain.end()) {


        auto pred = it != chain.begin() ? *(it - 1) : 0;
        auto succ = (it + 1) != chain.end() ? *(it + 1) : 0;

        // Remove the element from the list
        if (!pred) {
            pp->setHashRef(hash, succ);
        } else {
            blockPtr(pred)->setNextHashRef(succ);
        }
    }
}

/*
void
MutableFileSystem::addHashRef(const FSNode &at, Block nr)
{
    if (FSBlock *block = hashableBlockPtr(nr)) {
        addHashRef(at, block);
    }
}

void
MutableFileSystem::addHashRef(const FSNode &at, FSBlock *newBlock)
{
    // Only proceed if a hash table is present
    FSBlock *bp = blockPtr(at.ref);
    if (!bp || bp->hashTableSize() == 0) { return; }

    // Read the item at the proper hash table location
    u32 hash = newBlock->hashValue() % bp->hashTableSize();
    u32 ref = bp->getHashRef(hash);

    // If the slot is empty, put the reference there
    if (ref == 0) { bp->setHashRef(hash, newBlock->nr); return; }

    // Otherwise, put it into the last element of the block list chain
    FSBlock *last = lastHashBlockInChain(ref);
    if (last) last->setNextHashRef(newBlock->nr);
}
*/

isize
MutableFileSystem::addData(Block nr, const u8 *buf, isize size)
{
    FSBlock *block = blockPtr(nr);
    return block ? addData(*block, buf, size) : 0;
}

isize
MutableFileSystem::addData(FSBlock &block, const u8 *buf, isize size)
{
    isize count = 0;
    
    switch (block.type) {
            
        case FSBlockType::DATA_BLOCK_OFS:
            
            count = std::min(bsize - 24, size);
            std::memcpy(block.data() + 24, buf, count);
            block.setDataBytesInBlock((u32)count);
            break;

        case FSBlockType::DATA_BLOCK_FFS:

            count = std::min(bsize, size);
            std::memcpy(block.data(), buf, count);
            break;

        default:
            break;
    }
    
    return count;
}

void
MutableFileSystem::allocateFileBlocks(isize bytes, std::vector<Block> &listBlocks, std::vector<Block> &dataBlocks)
{
    isize numDataBlocks         = requiredDataBlocks(bytes);
    isize numListBlocks         = requiredFileListBlocks(bytes);
    isize refsPerBlock          = (bsize / 4) - 56;
    isize refsInHeaderBlock     = std::min(numDataBlocks, refsPerBlock);
    isize refsInListBlocks      = numDataBlocks - refsInHeaderBlock;
    isize refsInLastListBlock   = refsInListBlocks % refsPerBlock;
    
    debug(FS_DEBUG, "                   Data bytes : %ld\n", bytes);
    debug(FS_DEBUG, "         Required data blocks : %ld\n", numDataBlocks);
    debug(FS_DEBUG, "         Required list blocks : %ld\n", numListBlocks);
    debug(FS_DEBUG, "         References per block : %ld\n", refsPerBlock);
    debug(FS_DEBUG, "   References in header block : %ld\n", refsInHeaderBlock);
    debug(FS_DEBUG, "    References in list blocks : %ld\n", refsInListBlocks);
    debug(FS_DEBUG, "References in last list block : %ld\n", refsInLastListBlock);

    listBlocks.reserve(numListBlocks);
    dataBlocks.reserve(numDataBlocks);

    if (isOFS()) {

        // Header block -> Data blocks -> List block -> Data blocks ... List block -> Data blocks
        allocate(refsInHeaderBlock, dataBlocks);
        for (isize i = 0; i < numListBlocks; i++) {
            allocate(1, listBlocks);
            allocate(i < numListBlocks - 1 ? refsPerBlock : refsInLastListBlock, dataBlocks);
        }
    }
    
    if (isFFS()) {
        
        // Header block -> Data blocks -> All list block -> All remaining data blocks
        allocate(refsInHeaderBlock, dataBlocks);
        allocate(numListBlocks, listBlocks);
        allocate(refsInListBlocks, dataBlocks);
    }
}

void
MutableFileSystem::importVolume(const u8 *src, isize size)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) throw AppError(Fault::FS_WRONG_BSIZE);

    // Only proceed if the source buffer contains the right amount of data
    if (numBytes() != size) throw AppError(Fault::FS_WRONG_CAPACITY);

    // Only proceed if all partitions contain a valid file system
    if (dos == FSVolumeType::NODOS) throw AppError(Fault::FS_UNSUPPORTED);

    // Import all blocks
    for (isize i = 0; i < numBlocks(); i++) {
        
        const u8 *data = src + i * bsize;

        // Determine the type of the new block
        FSBlockType type = predictBlockType((Block)i, data);
        
        // Create new block
        storage[i].init(type);
        storage[i].importBlock(data, bsize);
    }
    
    // Print some debug information
    debug(FS_DEBUG, "Success\n");
}

void
MutableFileSystem::import(const FSNode &at, const fs::path &path, bool recursive, bool contents)
{
    fs::directory_entry dir;

    // Get the directory item
    try { dir = fs::directory_entry(path); } catch (...) {
        throw AppError(Fault::FILE_CANT_READ);
    }

    if (dir.is_directory() && contents) {

        // Add the directory contents
        for (const auto& it : fs::directory_iterator(dir)) import(at, it, recursive);

    } else {

        // Add the file or directory as a whole
        import(at, dir, recursive);
    }

    // Rectify the checksums of all blocks
    updateChecksums();

    // Verify the result
    if (FS_DEBUG) verify();
}

void
MutableFileSystem::import(const FSNode &at, const fs::directory_entry &entry, bool recursive)
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
            createFile(*at.ptr(), fsname, buffer.ptr, buffer.size);
        } else {
            createFile(*at.ptr(), fsname);
        }

    } else {

        debug(FS_DEBUG > 1, "Importing directory %s\n", fsname.c_str());

        // Create new directory
        auto &subdir = createDir(*at.ptr(), fsname);

        // Import all items
        for (const auto& it : fs::directory_iterator(entry)) {

            if (it.is_regular_file() || recursive) import(FSNode(this, subdir.nr), it, recursive);
        }
    }
}

bool
MutableFileSystem::exportVolume(u8 *dst, isize size) const
{
    return exportBlocks(0, (Block)(numBlocks() - 1), dst, size);
}

bool
MutableFileSystem::exportVolume(u8 *dst, isize size, Fault *err) const
{
    return exportBlocks(0, (Block)(numBlocks() - 1), dst, size, err);
}

bool
MutableFileSystem::exportBlock(Block nr, u8 *dst, isize size) const
{
    return exportBlocks(nr, nr, dst, size);
}

bool
MutableFileSystem::exportBlock(Block nr, u8 *dst, isize size, Fault *error) const
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
MutableFileSystem::exportBlocks(Block first, Block last, u8 *dst, isize size) const
{
    Fault error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == Fault::OK));
    return result;
}

bool
MutableFileSystem::exportBlocks(Block first, Block last, u8 *dst, isize size, Fault *err) const
{
    assert(last < (Block)numBlocks());
    assert(first <= last);
    assert(dst);
    
    isize count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %ld blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        if (err) *err = Fault::FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * bsize != size) {
        if (err) *err = Fault::FS_WRONG_CAPACITY;
        return false;
    }

    // Wipe out the target buffer
    std::memset(dst, 0, size);
    
    // Export all blocks
    for (isize i = 0; i < count; i++) {

        const_cast<FSBlock *>(storage.read(Block(first + i)))->exportBlock(dst + i * bsize, bsize);
        // blocks[first + i]->exportBlock(dst + i * bsize, bsize);
    }

    debug(FS_DEBUG, "Success\n");

    if (err) *err = Fault::OK;
    return true;
}

void
MutableFileSystem::exportDirectory(const fs::path &path, bool createDir) const
{
    // Try to create the directory if it doesn't exist
    if (!util::isDirectory(path) && createDir && !util::createDirectory(path)) {
        throw AppError(Fault::FS_CANNOT_CREATE_DIR);
    }

    // Only proceed if the directory exists
    if (!util::isDirectory(path)) {
        throw AppError(Fault::DIR_NOT_FOUND);
    }
    
    // Only proceed if path points to an empty directory
    if (util::numDirectoryItems(path) != 0) {
        throw AppError(Fault::FS_DIR_NOT_EMPTY);
    }
    
    // Collect all files and directories
    auto items = oldRootDir().collect();
    /*
    std::vector<Block> items;
    collect(rootDir(), items);
    */

    // Export all items
    for (auto const& i : items) {

        if (Fault error = i.ptr()->exportBlock(path.c_str()); error != Fault::OK) {
            throw AppError(error);
        }
        /*
        if (Fault error = blockPtr(i)->exportBlock(path.c_str()); error != Fault::OK) {
            throw AppError(error);
        }
        */
    }
    
    debug(FS_DEBUG, "Exported %zu items", items.size());
}

}
