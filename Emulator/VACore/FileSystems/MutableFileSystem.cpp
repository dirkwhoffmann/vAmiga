// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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
MutableFileSystem::init(FileSystemDescriptor &layout, const fs::path &path)
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
    
    // Start allocating blocks at the middle of the disk
    ap = rootBlock;
    
    // Do some consistency checking
    for (isize i = 0; i < numBlocks(); i++) assert(blocks[i] != nullptr);
    
    // Print some debug information
    if (FS_DEBUG) { dump(Category::State); }
    
    // Import files if applicable
    if (!path.empty()) {
        
        // Add all files
        importDirectory(path);
        
        // Assign device name
        setName(FSName(path.filename().string()));
        
        // Change to the root directory
        changeDir("/");
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
    // Start from scratch
    init(isize(blocks.size()));
    
    // Do some consistency checking
    assert(numBlocks() > 2);
    for (isize i = 0; i < numBlocks(); i++) assert(blocks[i] == nullptr);
    
    // Create boot blocks
    blocks[0] = new FSBlock(*this, 0, FSBlockType::BOOT_BLOCK);
    blocks[1] = new FSBlock(*this, 1, FSBlockType::BOOT_BLOCK);
    
    // Create the root block
    assert(rootBlock != 0);
    FSBlock *rb = new FSBlock(*this, rootBlock, FSBlockType::ROOT_BLOCK);
    blocks[rootBlock] = rb;
    
    // Create bitmap blocks
    for (auto& ref : bmBlocks) {
        
        blocks[ref] = new FSBlock(*this, ref, FSBlockType::BITMAP_BLOCK);
    }
    
    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (auto& ref : bmExtBlocks) {
        
        blocks[ref] = new FSBlock(*this, ref, FSBlockType::BITMAP_EXT_BLOCK);
        pred->setNextBmExtBlockRef(ref);
        pred = blocks[ref];
    }
    
    // Add all bitmap block references
    rb->addBitmapBlockRefs(bmBlocks);
    
    // Add free blocks
    for (isize i = 0; i < numBlocks(); i++) {
        
        if (blocks[i] == nullptr) {
            
            blocks[i] = new FSBlock(*this, Block(i), FSBlockType::EMPTY_BLOCK);
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
    
    return 1 + numDataBlocks + numFileListBlocks;
}

bool
MutableFileSystem::allocatable(isize count) const
{
    Block i = ap;
    isize capacity = numBlocks();
    
    while (count > 0) {
        
        if (blocks[i]->type == FSBlockType::EMPTY_BLOCK) {
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
            throw CoreError(Fault::FS_OUT_OF_SPACE);
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
            throw CoreError(Fault::FS_OUT_OF_SPACE);
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
    assert(blocks[nr]);
    
    blocks[nr]->init(FSBlockType::EMPTY_BLOCK);
    markAsFree(nr);
}

void
MutableFileSystem::addFileListBlock(Block at, Block head, Block prev)
{
    FSBlock *prevBlock = blockPtr(prev);

    if (prevBlock) {
        
        blocks[at]->init(FSBlockType::FILELIST_BLOCK);
        blocks[at]->setFileHeaderRef(head);
        
        prevBlock->setNextListBlockRef(at);
    }
}

void
MutableFileSystem::addDataBlock(Block at, isize id, Block head, Block prev)
{
    FSBlock *prevBlock = blockPtr(prev);

    if (prevBlock) {
     
        blocks[at]->init(isOFS() ? FSBlockType::DATA_BLOCK_OFS : FSBlockType::DATA_BLOCK_FFS);
        blocks[at]->setDataBlockNr((Block)id);
        blocks[at]->setFileHeaderRef(head);
    
        prevBlock->setNextDataBlockRef(at);
    }
}

FSBlock *
MutableFileSystem::newUserDirBlock(const string &name)
{
    if (Block nr = allocate()) {

        blocks[nr]->init(FSBlockType::USERDIR_BLOCK);
        blocks[nr]->setName(FSName(name));
        return blockPtr(nr);
    }
 
    return nullptr;
}

FSBlock *
MutableFileSystem::newFileHeaderBlock(const string &name)
{
    if (Block nr = allocate()) {

        blocks[nr]->init(FSBlockType::FILEHEADER_BLOCK);
        blocks[nr]->setName(FSName(name));
        return blockPtr(nr);
    }
    
    return nullptr;
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
    assert(blocks[0]->type == FSBlockType::BOOT_BLOCK);
    assert(blocks[1]->type == FSBlockType::BOOT_BLOCK);

    blocks[0]->writeBootBlock(id, 0);
    blocks[1]->writeBootBlock(id, 1);
}

void
MutableFileSystem::killVirus()
{
    assert(blocks[0]->type == FSBlockType::BOOT_BLOCK);
    assert(blocks[1]->type == FSBlockType::BOOT_BLOCK);

    auto id = isOFS() ? BootBlockId::AMIGADOS_13 : isFFS() ? BootBlockId::AMIGADOS_20 : BootBlockId::NONE;

    if (id != BootBlockId::NONE) {
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
    
FSBlock *
MutableFileSystem::createDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = newUserDirBlock(name);

    if (block) {
        
        block->setParentDirRef(cdb->nr);
        addHashRef(block->nr);
        return block;
    }
    
    throw CoreError(Fault::FS_OUT_OF_SPACE);
}

FSBlock *
MutableFileSystem::createFile(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = newFileHeaderBlock(name);
    
    if (block) {
        
        block->setParentDirRef(cdb->nr);
        addHashRef(block->nr);
        return block;
    }
    
    throw CoreError(Fault::FS_OUT_OF_SPACE);
}

FSBlock *
MutableFileSystem::createFile(const string &name, const u8 *buf, isize size)
{
    assert(buf);
        
    // Compute the number of data block references held in a file header or list block
    const usize numRefs = ((bsize / 4) - 56);
    
    // Create a file header block
    FSBlock *fhb = createFile(name);
 
    // Set file size
    fhb->setFileSize(u32(size));

    // Allocate blocks
    std::vector<Block> listBlocks;
    std::vector<Block> dataBlocks;
    allocateFileBlocks(size, listBlocks, dataBlocks);
    
    for (usize i = 0; i < listBlocks.size(); i++) {

        // Add a list block
        addFileListBlock(listBlocks[i], fhb->nr, i == 0 ? fhb->nr : listBlocks[i-1]);
    }
 
    for (usize i = 0; i < dataBlocks.size(); i++) {
        
        // Add a data block
        addDataBlock(dataBlocks[i], i + 1, fhb->nr, i == 0 ? fhb->nr : dataBlocks[i-1]);

        // Determine the list block managing this data block
        FSBlock *lb = blockPtr((i < numRefs) ? fhb->nr : listBlocks[i / numRefs - 1]);
        
        // Link the data block
        lb->addDataBlockRef(dataBlocks[0], dataBlocks[i]);
                
        // Add data bytes
        isize written = addData(dataBlocks[i], buf, size);
        buf += written;
        size -= written;
    }

    return fhb;
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
MutableFileSystem::addData(Block nr, const u8 *buffer, isize size)
{
    FSBlock *block = blockPtr(nr);
    return block ? addData(*block, buffer, size) : 0;
}

isize
MutableFileSystem::addData(FSBlock &block, const u8 *buffer, isize size)
{
    isize count = 0;
    
    switch (block.type) {
            
        case FSBlockType::DATA_BLOCK_OFS:
            
            count = std::min(bsize - 24, size);
            std::memcpy(block.data.ptr + 24, buffer, count);
            block.setDataBytesInBlock((u32)count);
            break;

        case FSBlockType::DATA_BLOCK_FFS:

            count = std::min(bsize, size);
            std::memcpy(block.data.ptr, buffer, count);
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
    if (size % bsize != 0) throw CoreError(Fault::FS_WRONG_BSIZE);

    // Only proceed if the source buffer contains the right amount of data
    if (numBytes() != size) throw CoreError(Fault::FS_WRONG_CAPACITY);

    // Only proceed if all partitions contain a valid file system
    if (dos == FSVolumeType::NODOS) throw CoreError(Fault::FS_UNSUPPORTED);

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
    if (FS_DEBUG) { printDirectory(true); }
}

void
MutableFileSystem::importDirectory(const fs::path &path, bool recursive)
{
    fs::directory_entry dir;
    
    try { dir = fs::directory_entry(path); }
    catch (...) { throw CoreError(Fault::FILE_CANT_READ); }
    
    // Add all files
    importDirectory(dir, recursive);
        
    // Rectify the checksums of all blocks
    updateChecksums();
    
    // Change back to the root directory
    changeDir("/");
    
    // Verify the result
    if (FS_DEBUG) verify();
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
                changeDir("..");
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
        
        blocks[first + i]->exportBlock(dst + i * bsize, bsize);
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
        throw CoreError(Fault::FS_CANNOT_CREATE_DIR);
    }

    // Only proceed if the directory exists
    if (!util::isDirectory(path)) {
        throw CoreError(Fault::DIR_NOT_FOUND);
    }
    
    // Only proceed if path points to an empty directory
    if (util::numDirectoryItems(path) != 0) {
        throw CoreError(Fault::FS_DIR_NOT_EMPTY);
    }
    
    // Collect all files and directories
    std::vector<Block> items;
    collect(rootBlock, items);

    // Export all items
    for (auto const& i : items) {
        
        if (Fault error = blockPtr(i)->exportBlock(path.c_str()); error != Fault::OK) {
            throw CoreError(error);
        }
    }
    
    debug(FS_DEBUG, "Exported %zu items", items.size());
}

}
