// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IOUtils.h"
#include "FSDevice.h"
#include "MemUtils.h"
#include <climits>
#include <set>
#include <stack>

void
FSDevice::init(isize capacity)
{
    assert(blocks.empty());
    
    blocks.reserve(capacity);
    blocks.assign(capacity, 0);
}

void
FSDevice::init(FSDeviceDescriptor &layout)
{
    init((isize)layout.numBlocks);
    
    if constexpr (FS_DEBUG) { layout.dump(); }
    
    // Copy layout parameters from the descriptor
    numCyls    = layout.geometry.cylinders;
    numHeads   = layout.geometry.heads;
    numSectors = layout.geometry.sectors;
    bsize      = layout.geometry.bsize;
    numBlocks  = layout.numBlocks;
        
    // Copy file system parameters from the descriptor
    dos        = layout.dos;
    
    // Create partition
    // partition = new FSPartition(*this, layout);
    partition = new FSPartition();
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;
    
    initBlocks(layout);
    
    // Compute checksums for all blocks
    updateChecksums();
    
    // Set the current directory to '/'
    cd = rootBlock;
    
    // Do some consistency checking
    for (isize i = 0; i < numBlocks; i++) assert(blocks[i] != nullptr);
    
    // Print some debug information
    if constexpr (FS_DEBUG) { dump(dump::Summary); }
}

void
FSDevice::initBlocks(FSDeviceDescriptor &layout)
{
    // Do some consistency checking
    for (Block i = 0; i < numBlocks; i++) assert(blocks[i] == nullptr);
    
    // Create boot blocks
    blocks[0] = new FSBlock(*this, 0, FS_BOOT_BLOCK);
    blocks[1] = new FSBlock(*this, 1, FS_BOOT_BLOCK);

    // Create the root block
    FSBlock *rb = new FSBlock(*this, rootBlock, FS_ROOT_BLOCK);
    blocks[layout.rootBlock] = rb;
    
    // Create the bitmap blocks
    for (auto& ref : layout.bmBlocks) {
        
        blocks[ref] = new FSBlock(*this, ref, FS_BITMAP_BLOCK);
    }
    
    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (auto& ref : layout.bmExtBlocks) {
        
        blocks[ref] = new FSBlock(*this, ref, FS_BITMAP_EXT_BLOCK);
        pred->setNextBmExtBlockRef(ref);
        pred = blocks[ref];
    }
    
    // Add all bitmap block references
    rb->addBitmapBlockRefs(layout.bmBlocks);
    
    // Add free blocks
    for (Block i = 0; i < numBlocks; i++) {
        
        if (blocks[i] == nullptr) {
            blocks[i] = new FSBlock(*this, i, FS_EMPTY_BLOCK);
            markAsFree(i);
        }
    }
}

void
FSDevice::init(DiskDiameter dia, DiskDensity den)
{
    // Get a device descriptor
    auto descriptor = FSDeviceDescriptor(dia, den);
        
    // Create the device
    init(descriptor);
}

void
FSDevice::init(DiskDiameter dia, DiskDensity den, const string &path)
{
    init(dia, den);
    
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
FSDevice::init(ADFFile &adf)
{
    // Get a device descriptor for the ADF
    FSDeviceDescriptor descriptor = adf.layout();
        
    // Create the device
    init(descriptor);

    // Import file system from ADF
    importVolume(adf.data, adf.size);
}

void
FSDevice::init(HDFFile &hdf, isize partition)
{
    printf("Getting layout for partition %ld\n", partition);
    
    // Get a device descriptor for the HDF
    // FSDeviceDescriptor descriptor = hdf.layout();
    auto descriptor = hdf.layoutOfPartition(partition);
    descriptor.dump();
    
    printf("Done\n");

    // Only proceed if the HDF is formatted
    if (descriptor.dos == FS_NODOS) throw VAError(ERROR_HDR_UNPARTITIONED);
    
    // Create the device
    init(descriptor);

    // Import file system from HDF
    auto *ptr = hdf.dataForPartition(partition);
    auto diff = ptr - hdf.data;
    printf("Skipping %ld.%ld blocks\n", diff / 512, diff % 512);
    
    // importVolume(hdf.data, hdf.size);
    importVolume(ptr, descriptor.numBlocks * 512);
}

void
FSDevice::init(class Drive &drive)
{
    auto adf = ADFFile(drive);
    init(adf);
}

void
FSDevice::init(const class HardDrive &drive, isize partition)
{
    auto hdf = HDFFile(drive);
    init(hdf, partition);
}

void
FSDevice::init(FSVolumeType type, const string &path)
{
    // Try to fit the directory into files system with DD disk capacity
    try { init(INCH_35, DISK_DD, path); return; } catch (...) { };

    // Try to fit the directory into files system with HD disk capacity
    init(INCH_35, DISK_HD, path);
}

FSDevice::~FSDevice()
{
    delete partition;
    for (auto &b : blocks) delete b;
}

void
FSDevice::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;

    if (category & dump::Summary) {
        
        auto total = numBlocks;
        auto used = usedBlocks();
        auto free = freeBlocks();
        auto fill = (isize)(100.0 * used / total);
        
        os << "DOS" << dec(dos);
        os << "   ";
        os << std::setw(6) << std::left << std::setfill(' ') << total;
        os << " (x ";
        os << std::setw(3) << std::left << std::setfill(' ') << bsize;
        os << ")  ";
        os << std::setw(6) << std::left << std::setfill(' ') << used;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << free;
        os << "  ";
        os << std::setw(3) << std::right << std::setfill(' ') << fill;
        os << "%  ";
        os << getName().c_str() << std::endl;
    }
    
    if (category & dump::Partitions) {
        
        os << tab("Root block");
        os << dec(rootBlock) << std::endl;
        os << tab("Bitmap blocks");
        for (auto& it : bmBlocks) { os << dec(it) << " "; }
        os << std::endl;
        os << util::tab("Extension blocks");
        for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
        os << std::endl;
    }

    if (category & dump::Blocks) {
                
        for (isize i = 0; i < numBlocks; i++)  {
            
            if (blocks[i]->type == FS_EMPTY_BLOCK) continue;
            
            msg("\nBlock %ld (%d):", i, blocks[i]->nr);
            msg(" %s\n", FSBlockTypeEnum::key(blocks[i]->type));
            
            blocks[i]->dump();
        }
    }
}

DiskGeometry
FSDevice::getGeometry() const
{
    DiskGeometry result;
    
    result.cylinders = numCyls;
    result.heads = numHeads;
    result.sectors = numSectors;
    result.bsize = bsize;
    
    return result;
}

isize
FSDevice::freeBlocks() const
{
    isize result = 0;
    
    for (isize i = 0; i < numBlocks; i++) {
        if (isFree((Block)i)) result++;
    }

    return result;
}

isize
FSDevice::usedBlocks() const
{
    return numBlocks - freeBlocks();
}

isize
FSDevice::freeBytes() const
{
    return freeBlocks() * bsize;
}

isize
FSDevice::usedBytes() const
{
    return usedBlocks() * bsize;
}

FSName
FSDevice::getName() const
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getName() : FSName("");
}

void
FSDevice::setName(FSName name)
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    assert(rb != nullptr);

    rb->setName(name);
}

void
FSDevice::makeBootable(BootBlockId id)
{
    assert(blocks[0]->type == FS_BOOT_BLOCK);
    assert(blocks[1]->type == FS_BOOT_BLOCK);

    blocks[0]->writeBootBlock(id, 0);
    blocks[1]->writeBootBlock(id, 1);
}

void
FSDevice::killVirus()
{
    assert(blocks[0]->type == FS_BOOT_BLOCK);
    assert(blocks[1]->type == FS_BOOT_BLOCK);

    auto id = isOFS() ? BB_AMIGADOS_13 : isFFS() ? BB_AMIGADOS_20 : BB_NONE;

    if (id != BB_NONE) {
        blocks[0]->writeBootBlock(id, 0);
        blocks[1]->writeBootBlock(id, 1);
    } else {
        std::memset(blocks[0]->data + 4, 0, bsize - 4);
        std::memset(blocks[1]->data, 0, bsize);
    }
}

FSBlockType
FSDevice::blockType(Block nr)
{
    return blockPtr(nr) ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
}

FSItemType
FSDevice::itemType(Block nr, isize pos) const
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FSDevice::blockPtr(Block nr) const
{
    return nr < blocks.size() ? blocks[nr] : nullptr;
}

FSBlock *
FSDevice::bootBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BOOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::rootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_ROOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::bitmapBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::bitmapExtBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_EXT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::userDirBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_USERDIR_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::fileHeaderBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::fileListBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILELIST_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::dataBlockPtr(Block nr)
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;

    if (t == FS_DATA_BLOCK_OFS || t == FS_DATA_BLOCK_FFS) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::hashableBlockPtr(Block nr)
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
    
    if (t == FS_USERDIR_BLOCK || t == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

isize
FSDevice::requiredDataBlocks(isize fileSize) const
{
    // Compute the capacity of a single data block
    isize numBytes = bsize - (isOFS() ? 24 : 0);

    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

isize
FSDevice::requiredFileListBlocks(isize fileSize) const
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
FSDevice::requiredBlocks(isize fileSize) const
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
FSDevice::allocateBlock()
{
    if (Block nr = allocateBlockAbove(rootBlock)) return nr;
    if (Block nr = allocateBlockBelow(rootBlock)) return nr;

    return 0;
}

Block
FSDevice::allocateBlockAbove(Block nr)
{
    assert(nr >= 0 && nr < numBlocks);
    
    for (i64 i = (i64)nr + 1; i < numBlocks; i++) {
        if (blocks[i]->type == FS_EMPTY_BLOCK) {
            markAsAllocated((Block)i);
            return (Block)i;
        }
    }
    return 0;
}

Block
FSDevice::allocateBlockBelow(Block nr)
{
    assert(nr >= 0 && nr < numBlocks);

    for (i64 i = (i64)nr - 1; i >= 0; i--) {
        if (blocks[i]->type == FS_EMPTY_BLOCK) {
            markAsAllocated((Block)i);
            return (Block)i;
        }
    }
    return 0;
}

void
FSDevice::deallocateBlock(Block nr)
{
    assert(nr >= 0 && nr < numBlocks);
    assert(blocks[nr]);
    
    delete blocks[nr];
    blocks[nr] = new FSBlock(*this, nr, FS_EMPTY_BLOCK);
    markAsFree(nr);
}

Block
FSDevice::addFileListBlock(Block head, Block prev)
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
FSDevice::addDataBlock(isize count, Block head, Block prev)
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
FSDevice::newUserDirBlock(const string &name)
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
FSDevice::newFileHeaderBlock(const string &name)
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
FSDevice::updateChecksums()
{
    for (isize i = 0; i < numBlocks; i++) {
        blocks[i]->updateChecksum();
    }
}

FSBlock *
FSDevice::bmBlockForBlock(Block nr)
{
    assert(nr >= 2 && (isize)nr < numBlocks);
        
    // Locate the bitmap block
    isize bitsPerBlock = (bsize - 4) * 8;
    isize bmNr = (nr - 2) / bitsPerBlock;

    if (bmNr >= (isize)bmBlocks.size()) {
        warn("Allocation bit is located in non-existent bitmap block %ld\n", bmNr);
        return nullptr;
    }

    return bitmapBlockPtr(bmBlocks[bmNr]);
}

bool
FSDevice::isFree(Block nr) const
{
    assert(nr >= 0 && nr < numBlocks);

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    FSBlock *bm = locateAllocationBit(nr, &byte, &bit);
        
    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

void
FSDevice::setAllocationBit(Block nr, bool value)
{
    isize byte, bit;
    
    if (FSBlock *bm = locateAllocationBit(nr, &byte, &bit)) {
        REPLACE_BIT(bm->data[byte], bit, value);
    }
}

FSBlock *
FSDevice::locateAllocationBit(Block nr, isize *byte, isize *bit) const
{
    assert(nr >= 0 && nr < numBlocks);

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;
    
    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm;
    bm = (bmNr < (isize)bmBlocks.size()) ? bitmapBlockPtr(bmBlocks[bmNr]) : nullptr;
    if (bm == nullptr) {
        warn("Failed to lookup allocation bit for block %d\n", nr);
        warn("bmNr = %ld\n", bmNr);
        return nullptr;
    }
    
    // Locate the byte position (note: the long word ordering will be reversed)
    nr = nr % bitsPerBlock;
    isize rByte = nr / 8;
    
    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < bsize);
    
    *byte = rByte;
    *bit = nr % 8;
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}

FSBlock *
FSDevice::currentDirBlock()
{
    FSBlock *cdb = blockPtr(cd);
    
    if (cdb) {
        if (cdb->type == FS_ROOT_BLOCK || cdb->type == FS_USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    cd = rootBlock;
    return blockPtr(cd);
}

FSBlock *
FSDevice::changeDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();

    if (name == "/") {
                
        // Move to top level
        cd = rootBlock;
        return currentDirBlock();
    }

    if (name == "..") {
                
        // Move one level up
        cd = cdb->getParentDirRef();
        return currentDirBlock();
    }
    
    FSBlock *subdir = seekDir(name);
    if (subdir == nullptr) return cdb;
    
    // Move one level down
    cd = subdir->nr;
    return currentDirBlock();
}

string
FSDevice::getPath(FSBlock *block)
{
    string result = "";
    std::set<Block> visited;
 
    while(block) {

        // Break the loop if this block has an invalid type
        if (!hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.find(block->nr) != visited.end()) break;
        
        // Add the block to the set of visited blocks
        visited.insert(block->nr);
                
        // Expand the path
        string name = block->getName().c_str();
        result = (result == "") ? name : name + "/" + result;
        
        // Continue with the parent block
        block = block->getParentDirBlock();
    }
    
    return result;
}

FSBlock *
FSDevice::createDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);
    
    return block;
}

FSBlock *
FSDevice::createFile(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = newFileHeaderBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);

    return block;
}

FSBlock *
FSDevice::createFile(const string &name, const u8 *buf, isize size)
{
    assert(buf);

    FSBlock *block = createFile(name);
    
    if (block) {
        assert(block->type == FS_FILEHEADER_BLOCK);
        block->addData(buf, size);
    }
    
    return block;
}

FSBlock *
FSDevice::createFile(const string &name, const string &str)
{
    return createFile(name, (const u8 *)str.c_str(), (isize)str.size());
}

Block
FSDevice::seekRef(FSName name)
{
    std::set<Block> visited;
    
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) return 0;
    
    // Compute the table position and read the item
    u32 hash = name.hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);
    
    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {
        
        FSBlock *item = hashableBlockPtr(ref);
        if (item == nullptr) break;
        
        if (item->isNamed(name)) return item->nr;

        visited.insert(ref);
        ref = item->getNextHashRef();
    }

    return 0;
}

void
FSDevice::addHashRef(Block nr)
{
    if (FSBlock *block = hashableBlockPtr(nr)) {
        addHashRef(block);
    }
}

void
FSDevice::addHashRef(FSBlock *newBlock)
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

void
FSDevice::printDirectory(bool recursive)
{
    std::vector<Block> items;
    collect(cd, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%zu items\n", items.size());
}


FSBlock *
FSDevice::lastHashBlockInChain(Block start)
{
    FSBlock *block = hashableBlockPtr(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastHashBlockInChain(FSBlock *block)
{
    std::set<Block> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextHashBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block =next;
    }
    return nullptr;
}

FSBlock *
FSDevice::lastFileListBlockInChain(Block start)
{
    FSBlock *block = fileListBlockPtr(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastFileListBlockInChain(FSBlock *block)
{
    std::set<Block> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextListBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block = next;
    }
    return nullptr;
}

void
FSDevice::collect(Block nr, std::vector<Block> &result, bool recursive)
{
    std::stack<Block> remainingItems;
    std::set<Block> visited;
    
    // Start with the items in this block
    collectHashedRefs(nr, remainingItems, visited);
    
    // Move the collected items to the result list
    while (remainingItems.size() > 0) {
        
        Block item = remainingItems.top();
        remainingItems.pop();
        result.push_back(item);

        // Add subdirectory items to the queue
        if (userDirBlockPtr(item) && recursive) {
            collectHashedRefs(item, remainingItems, visited);
        }
    }
}

void
FSDevice::collectHashedRefs(Block nr,
                            std::stack<Block> &result, std::set<Block> &visited)
{
    if (FSBlock *b = blockPtr(nr)) {
        
        // Walk through the hash table in reverse order
        for (isize i = (isize)b->hashTableSize(); i >= 0; i--) {
            collectRefsWithSameHashValue(b->getHashRef((u32)i), result, visited);
        }
    }
}

void
FSDevice::collectRefsWithSameHashValue(Block nr,
                                       std::stack<Block> &result, std::set<Block> &visited)
{
    std::stack<Block> refs;
    
    // Walk down the linked list
    for (FSBlock *b = hashableBlockPtr(nr); b; b = b->getNextHashBlock()) {

        // Only proceed if we haven't seen this block yet
        if (visited.find(b->nr) != visited.end()) throw VAError(ERROR_FS_HAS_CYCLES);

        visited.insert(b->nr);
        refs.push(b->nr);
    }
  
    // Push the collected elements onto the result stack
    while (refs.size() > 0) { result.push(refs.top()); refs.pop(); }
}

FSErrorReport
FSDevice::check(bool strict) const
{
    FSErrorReport result = { };

    isize total = 0, min = INT_MAX, max = 0;
    
    // Analyze the allocation table
    for (Block i = 0; i < numBlocks; i++) {

        FSBlock *block = blocks[i];
        if (block->type == FS_EMPTY_BLOCK && !isFree((Block)i)) {
            result.bitmapErrors++;
            debug(FS_DEBUG, "Empty block %d is marked as allocated\n", i);
        }
        if (block->type != FS_EMPTY_BLOCK && isFree((Block)i)) {
            result.bitmapErrors++;
            debug(FS_DEBUG, "Non-empty block %d is marked as free\n", i);
        }
    }

    // Analyze all blocks
    for (isize i = 0; i < numBlocks; i++) {

        if (blocks[i]->check(strict) > 0) {
            min = std::min(min, i);
            max = std::max(max, i);
            blocks[i]->corrupted = ++total;
        } else {
            blocks[i]->corrupted = 0;
        }
    }

    // Record findings
    if (total) {
        result.corruptedBlocks = total;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    } else {
        result.corruptedBlocks = 0;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    }
    
    return result;
}

ErrorCode
FSDevice::check(Block nr, isize pos, u8 *expected, bool strict) const
{
    return blocks[nr]->check(pos, expected, strict);
}

ErrorCode
FSDevice::checkBlockType(Block nr, FSBlockType type)
{
    return checkBlockType(nr, type, type);
}

ErrorCode
FSDevice::checkBlockType(Block nr, FSBlockType type, FSBlockType altType)
{
    FSBlockType t = blockType(nr);
    
    if (t != type && t != altType) {
        
        switch (t) {
                
            case FS_EMPTY_BLOCK:      return ERROR_FS_PTR_TO_EMPTY_BLOCK;
            case FS_BOOT_BLOCK:       return ERROR_FS_PTR_TO_BOOT_BLOCK;
            case FS_ROOT_BLOCK:       return ERROR_FS_PTR_TO_ROOT_BLOCK;
            case FS_BITMAP_BLOCK:     return ERROR_FS_PTR_TO_BITMAP_BLOCK;
            case FS_BITMAP_EXT_BLOCK: return ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FS_USERDIR_BLOCK:    return ERROR_FS_PTR_TO_USERDIR_BLOCK;
            case FS_FILEHEADER_BLOCK: return ERROR_FS_PTR_TO_FILEHEADER_BLOCK;
            case FS_FILELIST_BLOCK:   return ERROR_FS_PTR_TO_FILELIST_BLOCK;
            case FS_DATA_BLOCK_OFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            case FS_DATA_BLOCK_FFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            default:                  return ERROR_FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return ERROR_OK;
}

isize
FSDevice::getCorrupted(Block nr)
{
    return blockPtr(nr) ? blocks[nr]->corrupted : 0;
}

bool
FSDevice::isCorrupted(Block nr, isize n)
{
    for (isize i = 0, cnt = 0; i < numBlocks; i++) {
        
        if (isCorrupted((Block)i)) {
            cnt++;
            if ((i64)nr == i) return cnt == n;
        }
    }
    return false;
}

Block
FSDevice::nextCorrupted(Block nr)
{
    isize i = (isize)nr;
    while (++i < numBlocks) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FSDevice::prevCorrupted(Block nr)
{
    isize i = (isize)nr - 1;
    while (i-- >= 0) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FSDevice::seekCorruptedBlock(isize n)
{
    for (isize i = 0, cnt = 0; i < numBlocks; i++) {

        if (isCorrupted((Block)i)) {
            cnt++;
            if (cnt == n) return (Block)i;
        }
    }
    return (Block)-1;
}

u8
FSDevice::readByte(Block nr, isize offset) const
{
    assert(offset < bsize);

    if (nr < (Block)numBlocks) {
        return blocks[nr]->data ? blocks[nr]->data[offset] : 0;
    }
    
    return 0;
}

FSBlockType
FSDevice::predictBlockType(Block nr, const u8 *buffer)
{
    assert(buffer != nullptr);
    
    // Is it a boot block?
    if (nr == 0 || nr == 1) return FS_BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FS_BITMAP_BLOCK;
    
    // is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FS_BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize - 4);

    if (type == 2  && subtype == 1)       return FS_ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FS_USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FS_FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FS_FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS()) {
        if (type == 8) return FS_DATA_BLOCK_OFS;
    } else {
        for (isize i = 0; i < bsize; i++) if (buffer[i]) return FS_DATA_BLOCK_FFS;
    }
    
    return FS_EMPTY_BLOCK;
}

void
FSDevice::importVolume(const u8 *src, isize size)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) throw VAError(ERROR_FS_WRONG_BSIZE);

    // Only proceed if the source buffer contains the right amount of data
    if (numBlocks * bsize != size) throw VAError(ERROR_FS_WRONG_CAPACITY);

    // Only proceed if all partitions contain a valid file system
    if (dos == FS_NODOS) throw VAError(ERROR_FS_UNSUPPORTED);
        
    // Import all blocks
    for (isize i = 0; i < numBlocks; i++) {
        
        const u8 *data = src + i * bsize;
        
        // Get the partition this block belongs to
        // FSPartition &p = blocks[i]->partition;
        
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

bool
FSDevice::exportVolume(u8 *dst, isize size) const
{
    return exportBlocks(0, (Block)(numBlocks - 1), dst, size);
}

bool
FSDevice::exportVolume(u8 *dst, isize size, ErrorCode *err) const
{
    return exportBlocks(0, (Block)(numBlocks - 1), dst, size, err);
}

bool
FSDevice::exportBlock(Block nr, u8 *dst, isize size) const
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSDevice::exportBlock(Block nr, u8 *dst, isize size, ErrorCode *error) const
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSDevice::exportBlocks(Block first, Block last, u8 *dst, isize size) const
{
    ErrorCode error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == ERROR_OK));
    return result;
}

bool
FSDevice::exportBlocks(Block first, Block last, u8 *dst, isize size, ErrorCode *err) const
{
    assert(last < (Block)numBlocks);
    assert(first <= last);
    assert(dst);
    
    isize count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %ld blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        if (err) *err = ERROR_FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * bsize != size) {
        if (err) *err = ERROR_FS_WRONG_CAPACITY;
        return false;
    }
        
    // Wipe out the target buffer
    std::memset(dst, 0, size);
    
    // Export all blocks
    for (isize i = 0; i < count; i++) {
        
        blocks[first + i]->exportBlock(dst + i * bsize, bsize);
    }

    debug(FS_DEBUG, "Success\n");

    if (err) *err = ERROR_OK;
    return true;
}

#include <iostream>

void
FSDevice::importDirectory(const string &path, bool recursive)
{
    fs::directory_entry dir;
    
    try { dir = fs::directory_entry(path); }
    catch (...) { throw VAError(ERROR_FILE_CANT_READ); }
    
    importDirectory(dir, recursive);
}

void
FSDevice::importDirectory(const fs::directory_entry &dir, bool recursive)
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
            u8 *buffer; isize size;
            if (util::loadFile(string(path), &buffer, &size)) {
                
                createFile(name, buffer, size);
                delete [] (buffer);
            }
        }
    }
}

void
FSDevice::exportDirectory(const string &path, bool createDir)
{
    // Try to create the directory if it doesn't exist
    if (!util::isDirectory(path) && createDir && !util::createDirectory(path)) {
        throw VAError(ERROR_FS_CANNOT_CREATE_DIR);
    }

    // Only proceed if the directory exists
    if (!util::isDirectory(path)) {
        throw VAError(ERROR_DIR_NOT_FOUND);
    }
    // Only proceed if path points to an empty directory
    if (util::numDirectoryItems(path) != 0) {
        throw VAError(ERROR_FS_DIR_NOT_EMPTY);
    }
    
    // Collect all files and directories
    std::vector<Block> items;
    collect(cd, items);
        
    // Export all items
    for (auto const& i : items) {
        if (ErrorCode error = blockPtr(i)->exportBlock(path.c_str()); error != ERROR_OK) {
            throw VAError(error);
        }
    }
    
    debug(FS_DEBUG, "Exported %zu items", items.size());
}
