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
    // Remove existing blocks (if any)
    for (auto &b : blocks) delete b;
    
    // Resize and initialize the block storage
    blocks.reserve(capacity);
    blocks.assign(capacity, nullptr);
}

void
MutableFileSystem::init(FileSystemDescriptor &layout)
{
    init((isize)layout.numBlocks);
    
    if (FS_DEBUG) { layout.dump(); }

    // Copy layout parameters
    dos         = layout.dos;
    bsize       = layout.bsize;
    numReserved = layout.numReserved;
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;

    // Create all blocks
    format();
    
    // Set the current directory to '/'
    cd = rootBlock;
    
    // Do some consistency checking
    for (isize i = 0; i < numBlocks(); i++) assert(blocks[i] != nullptr);
    
    // Print some debug information
    if (FS_DEBUG) { dump(Category::State); }
}

void
MutableFileSystem::init(Diameter dia, Density den, FSVolumeType dos)
{
    // Get a device descriptor
    auto descriptor = FileSystemDescriptor(dia, den, dos);

    // Create the device
    init(descriptor);
}

void
MutableFileSystem::init(Diameter dia, Density den, const std::filesystem::path &path)
{
    init(dia, den, FS_OFS);
    
    // Try to import directory
    importDirectory(path);
    
    // Assign device name
    setName(FSName("Directory")); // TODO: Use last path component

    // Compute checksums for all blocks
    updateChecksums();

    // Change to the root directory
    changeDir("/");
}

void
MutableFileSystem::init(FSVolumeType type, const std::filesystem::path &path)
{
    // Try to fit the directory into files system with DD disk capacity
    try { init(INCH_35, DENSITY_DD, path); return; } catch (...) { };

    // Try to fit the directory into files system with HD disk capacity
    init(INCH_35, DENSITY_HD, path);
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
    // Start from scratch
    init(isize(blocks.size()));

    // Do some consistency checking
    assert(numBlocks() > 2);
    for (isize i = 0; i < numBlocks(); i++) assert(blocks[i] == nullptr);

    // Create boot blocks
    blocks[0] = new FSBlock(*this, 0, FS_BOOT_BLOCK);
    blocks[1] = new FSBlock(*this, 1, FS_BOOT_BLOCK);

    // Create the root block
    assert(rootBlock != 0);
    FSBlock *rb = new FSBlock(*this, rootBlock, FS_ROOT_BLOCK);
    blocks[rootBlock] = rb;
    
    // Create bitmap blocks
    for (auto& ref : bmBlocks) {
        
        blocks[ref] = new FSBlock(*this, ref, FS_BITMAP_BLOCK);
    }
    
    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (auto& ref : bmExtBlocks) {
        
        blocks[ref] = new FSBlock(*this, ref, FS_BITMAP_EXT_BLOCK);
        pred->setNextBmExtBlockRef(ref);
        pred = blocks[ref];
    }
    
    // Add all bitmap block references
    rb->addBitmapBlockRefs(bmBlocks);
    
    // Add free blocks
    for (isize i = 0; i < numBlocks(); i++) {
        
        if (blocks[i] == nullptr) {
            blocks[i] = new FSBlock(*this, Block(i), FS_EMPTY_BLOCK);
            markAsFree(Block(i));
        }
    }
    
    // Set the volume name
    if (name != "") setName(name);
    
    // Compute checksums for all blocks
    updateChecksums();
}

void
MutableFileSystem::setName(FSName name)
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    assert(rb != nullptr);

    rb->setName(name);
    rb->updateChecksum();
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
    debug(FS_DEBUG, "                Free blocks : %ld\n", freeBlocks());
    
    return 1 + numDataBlocks + numFileListBlocks;
}

Block
MutableFileSystem::allocateBlock()
{
    if (Block nr = allocateBlockAbove(rootBlock)) return nr;
    if (Block nr = allocateBlockBelow(rootBlock)) return nr;

    return 0;
}

Block
MutableFileSystem::allocateBlockAbove(Block nr)
{
    assert(isBlockNumber(nr));
    
    for (isize i = nr + 1; i < numBlocks(); i++) {
        
        if (blocks[i]->type == FS_EMPTY_BLOCK) {
            
            markAsAllocated(Block(i));
            return (Block(i));
        }
    }
    return 0;
}

Block
MutableFileSystem::allocateBlockBelow(Block nr)
{
    assert(isBlockNumber(nr));

    for (i64 i = (i64)nr - 1; i >= 0; i--) {
        
        if (blocks[i]->type == FS_EMPTY_BLOCK) {
            
            markAsAllocated(Block(i));
            return (Block(i));
        }
    }
    return 0;
}

void
MutableFileSystem::deallocateBlock(Block nr)
{
    assert(isBlockNumber(nr));
    assert(blocks[nr]);
    
    delete blocks[nr];
    blocks[nr] = new FSBlock(*this, nr, FS_EMPTY_BLOCK);
    markAsFree(nr);
}

Block
MutableFileSystem::addFileListBlock(Block head, Block prev)
{
    FSBlock *prevBlock = blockPtr(prev);
    if (!prevBlock) return 0;
    
    Block nr = allocateBlock();
    if (!nr) return 0;
    
    blocks[nr] = new FSBlock(*this, nr, FS_FILELIST_BLOCK);
    blocks[nr]->setFileHeaderRef(head);
    prevBlock->setNextListBlockRef(nr);
    
    return nr;
}

Block
MutableFileSystem::addDataBlock(isize count, Block head, Block prev)
{
    FSBlock *prevBlock = blockPtr(prev);
    if (!prevBlock) return 0;

    Block nr = allocateBlock();
    if (!nr) return 0;

    FSBlock *newBlock;
    if (isOFS()) {
        newBlock = new FSBlock(*this, nr, FS_DATA_BLOCK_OFS);
    } else {
        newBlock = new FSBlock(*this, nr, FS_DATA_BLOCK_FFS);
    }
    
    blocks[nr] = newBlock;
    newBlock->setDataBlockNr((Block)count);
    newBlock->setFileHeaderRef(head);
    prevBlock->setNextDataBlockRef(nr);
    
    return nr;
}

FSBlock *
MutableFileSystem::newUserDirBlock(const string &name)
{
    FSBlock *block = nullptr;
    
    if (Block nr = allocateBlock()) {

        block = new FSBlock(*this, nr, FS_USERDIR_BLOCK);
        block->setName(FSName(name));
        blocks[nr] = block;
    }
    
    return block;
}

FSBlock *
MutableFileSystem::newFileHeaderBlock(const string &name)
{
    FSBlock *block = nullptr;
    
    if (Block nr = allocateBlock()) {

        block = new FSBlock(*this, nr, FS_FILEHEADER_BLOCK);
        block->setName(FSName(name));
        blocks[nr] = block;
    }
    
    return block;
}

void
MutableFileSystem::updateChecksums()
{
    for (isize i = 0; i < numBlocks(); i++) {
        blocks[i]->updateChecksum();
    }
}

void
MutableFileSystem::makeBootable(BootBlockId id)
{
    assert(blocks[0]->type == FS_BOOT_BLOCK);
    assert(blocks[1]->type == FS_BOOT_BLOCK);

    blocks[0]->writeBootBlock(id, 0);
    blocks[1]->writeBootBlock(id, 1);
}

void
MutableFileSystem::killVirus()
{
    assert(blocks[0]->type == FS_BOOT_BLOCK);
    assert(blocks[1]->type == FS_BOOT_BLOCK);

    auto id = isOFS() ? BB_AMIGADOS_13 : isFFS() ? BB_AMIGADOS_20 : BB_NONE;

    if (id != BB_NONE) {
        blocks[0]->writeBootBlock(id, 0);
        blocks[1]->writeBootBlock(id, 1);
    } else {
        std::memset(blocks[0]->data.ptr + 4, 0, bsize - 4);
        std::memset(blocks[1]->data.ptr, 0, bsize);
    }
}

void
MutableFileSystem::setAllocationBit(Block nr, bool value)
{
    isize byte, bit;
    
    if (FSBlock *bm = locateAllocationBit(nr, &byte, &bit)) {
        REPLACE_BIT(bm->data[byte], bit, value);
    }
}

FSBlock *
MutableFileSystem::createDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);
    
    return block;
}

FSBlock *
MutableFileSystem::createFile(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = newFileHeaderBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);

    return block;
}

FSBlock *
MutableFileSystem::createFile(const string &name, const u8 *buf, isize size)
{
    assert(buf);

    FSBlock *block = createFile(name);
    
    if (block) {
        assert(block->type == FS_FILEHEADER_BLOCK);
        addData(*block, buf, size);
    }
    
    return block;
}

FSBlock *
MutableFileSystem::createFile(const string &name, const string &str)
{
    return createFile(name, (const u8 *)str.c_str(), (isize)str.size());
}

void
MutableFileSystem::addHashRef(Block nr)
{
    if (FSBlock *block = hashableBlockPtr(nr)) {
        addHashRef(block);
    }
}

void
MutableFileSystem::addHashRef(FSBlock *newBlock)
{
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) { return; }

    // Read the item at the proper hash table location
    u32 hash = newBlock->hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);

    // If the slot is empty, put the reference there
    if (ref == 0) { cdb->setHashRef(hash, newBlock->nr); return; }

    // Otherwise, put it into the last element of the block list chain
    FSBlock *last = lastHashBlockInChain(ref);
    if (last) last->setNextHashRef(newBlock->nr);
}

isize
MutableFileSystem::addData(FSBlock &block, const u8 *buffer, isize size)
{
    auto nr = block.nr;
    
    switch (block.type) {
            
        case FS_FILEHEADER_BLOCK:
        {
            assert(block.getFileSize() == 0);

            // Compute the required number of blocks
            isize numDataBlocks = requiredDataBlocks(size);
            isize numListBlocks = requiredFileListBlocks(size);
            
            debug(FS_DEBUG, "Required data blocks : %ld\n", numDataBlocks);
            debug(FS_DEBUG, "Required list blocks : %ld\n", numListBlocks);
            debug(FS_DEBUG, "         Free blocks : %ld\n", freeBlocks());
            
            if (freeBlocks() < numDataBlocks + numListBlocks) {
                warn("Not enough free blocks\n");
                return 0;
            }
            
            for (Block ref = nr, i = 0; i < (Block)numListBlocks; i++) {

                // Add a new file list block
                ref = addFileListBlock(nr, ref);
            }
            
            for (Block ref = nr, i = 1; i <= (Block)numDataBlocks; i++) {

                // Add a new data block
                ref = addDataBlock(i, nr, ref);

                // Add references to the new data block
                block.addDataBlockRef(ref, ref);
                
                // Add data
                FSBlock *ptr = blockPtr(ref);
                if (ptr) {
                    isize written = addData(*ptr, buffer, size);
                    block.setFileSize((u32)(block.getFileSize() + written));
                    buffer += written;
                    size -= written;
                }
            }

            return block.getFileSize();
        }
        case FS_DATA_BLOCK_OFS:
        {
            isize count = std::min(bsize - 24, size);

            std::memcpy(block.data.ptr + 24, buffer, count);
            block.setDataBytesInBlock((u32)count);
            
            return count;
        }
        case FS_DATA_BLOCK_FFS:
        {
            isize count = std::min(bsize, size);
            
            std::memcpy(block.data.ptr, buffer, count);
            
            return count;
        }
        default:
            return 0;
    }
}

void
MutableFileSystem::importVolume(const u8 *src, isize size)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) throw Error(VAERROR_FS_WRONG_BSIZE);

    // Only proceed if the source buffer contains the right amount of data
    if (numBytes() != size) throw Error(VAERROR_FS_WRONG_CAPACITY);

    // Only proceed if all partitions contain a valid file system
    if (dos == FS_NODOS) throw Error(VAERROR_FS_UNSUPPORTED);

    // Import all blocks
    for (isize i = 0; i < numBlocks(); i++) {
        
        const u8 *data = src + i * bsize;

        // Determine the type of the new block
        FSBlockType type = predictBlockType((Block)i, data);
        
        // Create new block
        FSBlock *newBlock = FSBlock::make(*this, (Block)i, type);

        // Import block data
        newBlock->importBlock(data, bsize);

        // Replace the existing block
        assert(blocks[i] != nullptr);
        delete blocks[i];
        blocks[i] = newBlock;
    }
    
    // Print some debug information
    debug(FS_DEBUG, "Success\n");
    // info();
    // dump();
    // util::hexdump(blocks[0]->data, 512);
    printDirectory(true);
}

void
MutableFileSystem::importDirectory(const std::filesystem::path &path, bool recursive)
{
    fs::directory_entry dir;
    
    try { dir = fs::directory_entry(path); }
    catch (...) { throw Error(VAERROR_FILE_CANT_READ); }
    
    importDirectory(dir, recursive);
}

void
MutableFileSystem::importDirectory(const fs::directory_entry &dir, bool recursive)
{
    for (const auto& entry : fs::directory_iterator(dir)) {
        
        const auto path = entry.path().string();
        const auto name = entry.path().filename().string();

        // Skip all hidden files
        if (name[0] == '.') continue;

        debug(FS_DEBUG, "Importing %s\n", path.c_str());

        if (entry.is_directory()) {
            
            // Add directory
            if(createDir(name) && recursive) {

                changeDir(name);
                importDirectory(entry, recursive);
            }
        }

        if (entry.is_regular_file()) {
            
            // Add file
            Buffer<u8> buffer(path);
            if (buffer) createFile(name, buffer.ptr, buffer.size);
        }
    }
}

bool
MutableFileSystem::exportVolume(u8 *dst, isize size) const
{
    return exportBlocks(0, (Block)(numBlocks() - 1), dst, size);
}

bool
MutableFileSystem::exportVolume(u8 *dst, isize size, ErrorCode *err) const
{
    return exportBlocks(0, (Block)(numBlocks() - 1), dst, size, err);
}

bool
MutableFileSystem::exportBlock(Block nr, u8 *dst, isize size) const
{
    return exportBlocks(nr, nr, dst, size);
}

bool
MutableFileSystem::exportBlock(Block nr, u8 *dst, isize size, ErrorCode *error) const
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
MutableFileSystem::exportBlocks(Block first, Block last, u8 *dst, isize size) const
{
    ErrorCode error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == VAERROR_OK));
    return result;
}

bool
MutableFileSystem::exportBlocks(Block first, Block last, u8 *dst, isize size, ErrorCode *err) const
{
    assert(last < (Block)numBlocks());
    assert(first <= last);
    assert(dst);
    
    isize count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %ld blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        if (err) *err = VAERROR_FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * bsize != size) {
        if (err) *err = VAERROR_FS_WRONG_CAPACITY;
        return false;
    }

    // Wipe out the target buffer
    std::memset(dst, 0, size);
    
    // Export all blocks
    for (isize i = 0; i < count; i++) {
        
        blocks[first + i]->exportBlock(dst + i * bsize, bsize);
    }

    debug(FS_DEBUG, "Success\n");

    if (err) *err = VAERROR_OK;
    return true;
}

void
MutableFileSystem::exportDirectory(const std::filesystem::path &path, bool createDir) const
{
    // Try to create the directory if it doesn't exist
    if (!util::isDirectory(path) && createDir && !util::createDirectory(path)) {
        throw Error(VAERROR_FS_CANNOT_CREATE_DIR);
    }

    // Only proceed if the directory exists
    if (!util::isDirectory(path)) {
        throw Error(VAERROR_DIR_NOT_FOUND);
    }
    
    // Only proceed if path points to an empty directory
    if (util::numDirectoryItems(path) != 0) {
        throw Error(VAERROR_FS_DIR_NOT_EMPTY);
    }
    
    // Collect all files and directories
    std::vector<Block> items;
    collect(cd, items);

    // Export all items
    for (auto const& i : items) {
        
        if (ErrorCode error = blockPtr(i)->exportBlock(path.c_str()); error != VAERROR_OK) {
            throw Error(error);
        }
    }
    
    debug(FS_DEBUG, "Exported %zu items", items.size());
}

}
