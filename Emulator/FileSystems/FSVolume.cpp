// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSVolume.h"
#include <set>

FSVolume *
FSVolume::makeWithADF(ADFFile *adf, FSError *error)
{
    assert(adf != nullptr);

    // TODO: Determine file system type from ADF
    FSVolumeType type = FS_OFS;
    
    // Create volume
    FSVolume *volume = new FSVolume(type, adf->numBlocks(), 512);

    // Import file system from ADF
    if (!volume->importVolume(adf->getData(), adf->getSize(), error)) {
        delete volume;
        return nullptr;
    }
    
    return volume;
}

FSVolume *
FSVolume::makeWithHDF(HDFFile *hdf, FSError *error)
{
    assert(hdf != nullptr);

    // TODO: Determine file system type from HDF
    FSVolumeType type = FS_OFS;
    
    // REMOVE ASAP
    FSVolume *volume = new FSVolume(FS_OFS, 120000);

    // Create volume
    // FSVolume *volume = new FSVolume(type, hdf->numBlocks(), 512);

    // Import file system from HDF
    /*
    if (!volume->importVolume(hdf->getData(), hdf->getSize(), error)) {
        delete volume;
        return nullptr;
    }
    */
    
    return volume;
}

FSVolume *
FSVolume::make(FSVolumeType type, const char *name, const char *path, u32 capacity)
{
    FSVolume *volume = new FSVolume(type, capacity);
    volume->setName(FSName(name));

    // Try to import directory
    if (!volume->importDirectory(path)) { delete volume; return nullptr; }
    
    // Change to root directory and return
    volume->changeDir("/");
    return volume;
}

FSVolume *
FSVolume::make(FSVolumeType type, const char *name, const char *path)
{
    FSVolume *volume;
    
    // Try to fit the directory into files system with DD disk capacity
    if ((volume = make(type, name, path, 2 * 880))) return volume;

    // Try to fit the directory into files system with HD disk capacity
    if ((volume = make(type, name, path, 4 * 880))) return volume;

    return nullptr;
}

FSVolume::FSVolume(FSVolumeType t, u32 c, u32 s) :  type(t), capacity(c), bsize(s)
{
    debug(FS_DEBUG, "Creating FSVolume with %d blocks\n", c);
    
    blocks = new BlockPtr[capacity]();

    dsize = isOFS() ? bsize - 24 : bsize;

    // Determine the block locations
    u32 root             = rootBlockNr();
    u32 firstBitmapBlock = root + 1;
    u32 lastBitmapBlock  = firstBitmapBlock + requiredBitmapBlocks() - 1;
    u32 firstExtBlock    = lastBitmapBlock + 1;
    u32 lastExtBlock     = firstExtBlock + requiredBitmapExtensionBlocks() - 1;
    assert(root < capacity);

    // Add boot blocks
    blocks[0] = new FSBootBlock(*this, 0);
    blocks[1] = new FSBootBlock(*this, 1);
    
    // Add the root block
    FSRootBlock *rb = new FSRootBlock(*this, root);
    blocks[root] = rb;
        
    // Add bitmap blocks
    for (u32 ref = firstBitmapBlock; ref <= lastBitmapBlock; ref++) {
        blocks[ref] = new FSBitmapBlock(*this, ref);
        bmBlocks.push_back(ref);
    }

    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (u32 ref = firstExtBlock; ref <= lastExtBlock; ref++) {
        blocks[ref] = new FSBitmapExtBlock(*this, ref);
        bmExtensionBlocks.push_back(ref);
        pred->setNextBmExtBlockRef(ref);
        pred = blocks[ref];
    }

    // Add all bitmap block references
    rb->addBitmapBlockRefs(bmBlocks);
        
    // Add free blocks
    for (u32 i = 2; i < root; i++) {
        assert(blocks[i] == nullptr);
        blocks[i] = new FSEmptyBlock(*this, i);
        markAsFree(i);
    }
    for (u32 i = lastExtBlock + 1; i < capacity; i++) {
        assert(blocks[i] == nullptr);
        blocks[i] = new FSEmptyBlock(*this, i);
        markAsFree(i);
    }

    // Compute checksums for all blocks
    updateChecksums();
    
    // Set the current directory to '/'
    currentDir = rootBlockNr();
    
    // Do some final consistency checks
    assert(bitmapBlock() == blocks[rootBlockNr() + 1]);
    assert(rootBlock() == blocks[rootBlockNr()]);

    if (FS_DEBUG) {
        info();
        dump();
    }
}

FSVolume::~FSVolume()
{
    for (u32 i = 0; i < capacity; i++) {
        delete blocks[i];
    }
    delete [] blocks;
}

void
FSVolume::info()
{
    msg("Type   Size          Used   Free   Full   Name\n");
    msg("DOS%ld  ",     getType());
    msg("%5d (x %3d) ", numBlocks(), bsize);
    msg("%5d  ",        usedBlocks());
    msg("%5d   ",       freeBlocks());
    msg("%3d%%   ",     (int)(100.0 * usedBlocks() / freeBlocks()));
    msg("%s\n",         getName().c_str());
    msg("\n");
}

void
FSVolume::dump()
{
    msg("                  Name : %s\n", getName().c_str());
    msg("           File system : DOS%ld (%s)\n", getType(), sFSVolumeType(getType()));
    msg("\n");
    msg("  Bytes per data block : %d\n", getDataBlockCapacity());
    msg(" Bits per bitmap block : %d\n", getAllocBitsInBitmapBlock());
    msg("     Bitmap block refs : %d (in root block)\n", bitmapRefsInRootBlock());
    msg("                         %d (in ext block)\n", bitmapRefsInBitmapExtensionBlock());
    msg("Required bitmap blocks : %d\n", requiredBitmapBlocks());
    msg(" Required bmExt blocks : %d\n", requiredBitmapExtensionBlocks());
    msg("\n");
    msg(" Bitmap blocks : ");
    for (auto& it : bmBlocks) { msg("%d ", it); }
    msg("\n");
    msg("  BmExt blocks : ");
    for (auto& it : bmExtensionBlocks) { msg("%d ", it); }
    msg("\n\n");

    for (size_t i = 0; i < capacity; i++)  {
        
        if (blocks[i]->type() == FS_EMPTY_BLOCK) continue;
        
        msg("\nBlock %d (%d):", i, blocks[i]->nr);
        msg(" %s\n", sFSBlockType(blocks[i]->type()));
                
        blocks[i]->dump(); 
    }
}

FSErrorReport
FSVolume::check(bool strict)
{
    long total = 0, min = LONG_MAX, max = 0;
    
    // Analyze all blocks
    for (u32 i = 0; i < capacity; i++) {
         
        if (blocks[i]->check(strict) > 0) {
            min = MIN(min, i);
            max = MAX(max, i);
            blocks[i]->corrupted = ++total;
        } else {
            blocks[i]->corrupted = 0;
        }
    }

    // Record findings
    FSErrorReport result;
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

FSError
FSVolume::check(u32 blockNr, u32 pos, u8 *expected, bool strict)
{
    return blocks[blockNr]->check(pos, expected, strict);
}

FSError
FSVolume::checkBlockType(u32 nr, FSBlockType type)
{
    return checkBlockType(nr, type, type);
}

FSError
FSVolume::checkBlockType(u32 nr, FSBlockType type, FSBlockType altType)
{
    FSBlockType t = blockType(nr);
    
    if (t != type && t != altType) {
        
        switch (t) {
                
            case FS_EMPTY_BLOCK:      return FS_PTR_TO_EMPTY_BLOCK;
            case FS_BOOT_BLOCK:       return FS_PTR_TO_BOOT_BLOCK;
            case FS_ROOT_BLOCK:       return FS_PTR_TO_ROOT_BLOCK;
            case FS_BITMAP_BLOCK:     return FS_PTR_TO_BITMAP_BLOCK;
            case FS_BITMAP_EXT_BLOCK: return FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FS_USERDIR_BLOCK:    return FS_PTR_TO_USERDIR_BLOCK;
            case FS_FILEHEADER_BLOCK: return FS_PTR_TO_FILEHEADER_BLOCK;
            case FS_FILELIST_BLOCK:   return FS_PTR_TO_FILEHEADER_BLOCK;
            case FS_DATA_BLOCK:       return FS_PTR_TO_DATA_BLOCK;
            default:                  return FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return FS_OK;
}

u32
FSVolume::getCorrupted(u32 blockNr)
{
    return block(blockNr) ? blocks[blockNr]->corrupted : 0;
}

bool
FSVolume::isCorrupted(u32 blockNr, u32 n)
{
    for (u32 i = 0, cnt = 0; i < capacity; i++) {
        
        if (isCorrupted(i)) {
            cnt++;
            if (blockNr == i) return cnt == n;
        }
    }
    return false;
}

u32
FSVolume::nextCorrupted(u32 blockNr)
{
    long i = (long)blockNr;
    while (++i < capacity) { if (isCorrupted(i)) return i; }
    return blockNr;
}

u32
FSVolume::prevCorrupted(u32 blockNr)
{
    long i = (long)blockNr - 1;
    while (i-- >= 0) { if (isCorrupted(i)) return i; }
    return blockNr;
}

u32
FSVolume::seekCorruptedBlock(u32 n)
{
    for (u32 i = 0, cnt = 0; i < capacity; i++) {

        if (isCorrupted(i)) {
            cnt++;
            if (cnt == n) return i;
        }
    }
    return (u32)-1;
}

FSBlockType
FSVolume::guessBlockType(u32 nr, const u8 *buffer)
{
    assert(buffer != nullptr);

    // Take care of blocks that can be identified by number
    if (nr <= 1) return FS_BOOT_BLOCK;
    if (nr == bitmapBlockNr()) return FS_BITMAP_BLOCK;
    
    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize - 4);

    if (type == 2 && subtype == 1) return FS_ROOT_BLOCK;
    if (type == 2 && subtype == 2) return FS_USERDIR_BLOCK;
    if (type == 2 && subtype == (u32)-3) return FS_FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FS_FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS()) {
        if (type == 8) return FS_DATA_BLOCK;
    } else {
        for (u32 i = 0; i < bsize; i++) if (buffer[i]) return FS_DATA_BLOCK;
    }
    
    return FS_EMPTY_BLOCK;
}

bool
FSVolume::isOFS()
{
    return
    type == FS_OFS ||
    type == FS_OFS_INTL ||
    type == FS_OFS_DC ||
    type == FS_OFS_LNFS;
}

bool
FSVolume::isFFS()
{
    return
    type == FS_FFS ||
    type == FS_FFS_INTL ||
    type == FS_FFS_DC ||
    type == FS_FFS_LNFS;
}

u32
FSVolume::getAllocBitsInBitmapBlock()
{
    return (bsize - 4) * 8;
}

u32
FSVolume::bitmapRefsInRootBlock()
{
    return 25;
}

u32
FSVolume::bitmapRefsInBitmapExtensionBlock()
{
    return (bsize / 4) - 1;
}

u32
FSVolume::requiredBitmapBlocks()
{
    u32 allocationBitsPerBlock = (bsize - 4) * 8;
    return (capacity + allocationBitsPerBlock - 1) / allocationBitsPerBlock;
}

u32
FSVolume::requiredBitmapExtensionBlocks()
{
    u32 numBlocks = requiredBitmapBlocks();
    
    // The first 25 blocks fit into the root block
    if (numBlocks <= bitmapRefsInRootBlock()) return 0;
    
    numBlocks -= 25;
    u32 refsPerBlock = bitmapRefsInBitmapExtensionBlock();
    return (numBlocks + refsPerBlock - 1) / refsPerBlock;
}

u32
FSVolume::getDataBlockCapacity()
{
    if (isOFS()) {
        return bsize - OFSDataBlock::headerSize();
    } else {
        return bsize - FFSDataBlock::headerSize();
    }
}

u32
FSVolume::freeBlocks()
{
    u32 result = 0;
    
    for (size_t i = 0; i < capacity; i++)  {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) result++;
    }
    
    return result;
}

bool
FSVolume::isFree(u32 ref)
{
    u32 block, byte, bit;
    
    if (locateAllocationBit(ref, &block, &byte, &bit)) {
        if (FSBitmapBlock *bm = bitmapBlock(block)) {
            
            return GET_BIT(bm->data[byte], bit);
        }
    }
    return false;
}

void
FSVolume::mark(u32 ref, bool alloc)
{
    u32 block, byte, bit;
    
    if (locateAllocationBit(ref, &block, &byte, &bit)) {
        if (FSBitmapBlock *bm = bitmapBlock(block)) {
            
            // 0 = allocated, 1 = free
            alloc ? CLR_BIT(bm->data[byte], bit) : SET_BIT(bm->data[byte], bit);
        }
    }
}

bool
FSVolume::locateAllocationBit(u32 ref, u32 *block, u32 *byte, u32 *bit)
{
    assert(ref >= 2 && ref < capacity);
    
    // The first two blocks are not part the map (they are always allocated)
    ref -= 2;
    
    // Locate the bitmap block
    u32 nr = ref /  getAllocBitsInBitmapBlock();
    if (nr >= bmBlocks.size()) {
        warn("Allocation bit is located in a non-existent bitmap block %d\n", nr);
        return false;
    }
    u32 rBlock = bmBlocks[nr];
    assert(bitmapBlock(rBlock));

    // Locate the byte position (the long word ordering will be inversed)
    u32 rByte = (nr / 8);
    
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
    
    *block = rBlock;
    *byte  = rByte;
    *bit   = ref % 8;
    
    return true;
}

FSBlockType
FSVolume::blockType(u32 nr)
{
    return block(nr) ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
}

FSItemType
FSVolume::itemType(u32 nr, u32 pos)
{
    return block(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FSVolume::block(u32 nr)
{
    if (nr < capacity) {
        return blocks[nr];
    } else {
        return nullptr;
    }
}

FSBootBlock *
FSVolume::bootBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() != FS_BOOT_BLOCK)
    {
        return (FSBootBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSRootBlock *
FSVolume::rootBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_ROOT_BLOCK) {
        return (FSRootBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSBitmapBlock *
FSVolume::bitmapBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_BITMAP_BLOCK) {
        return (FSBitmapBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSBitmapExtBlock *
FSVolume::bitmapExtBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_BITMAP_EXT_BLOCK) {
        return (FSBitmapExtBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSUserDirBlock *
FSVolume::userDirBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_USERDIR_BLOCK) {
        return (FSUserDirBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSFileHeaderBlock *
FSVolume::fileHeaderBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_FILEHEADER_BLOCK) {
        return (FSFileHeaderBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSFileListBlock *
FSVolume::fileListBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_FILELIST_BLOCK) {
        return (FSFileListBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSDataBlock *
FSVolume::dataBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_DATA_BLOCK) {
        return (FSDataBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSBlock *
FSVolume::hashableBlock(u32 nr)
{
    FSBlockType type = nr < capacity ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
    
    if (type == FS_USERDIR_BLOCK || type == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    } else {
        return nullptr;
    }
}

u32
FSVolume::allocateBlock()
{
    // Search for a free block above the root block
    for (long i = rootBlockNr() + 1; i < capacity; i++) {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            markAsAllocated(i);
            return i;
        }
    }

    // Search for a free block below the root block
    for (long i = rootBlockNr() - 1; i >= 2; i--) {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            markAsAllocated(i);
            return i;
        }
    }

    return 0;
}

void
FSVolume::deallocateBlock(u32 ref)
{
    FSBlock *b = block(ref);
    if (b == nullptr) return;
    
    if (b->type() != FS_EMPTY_BLOCK) {
        delete b;
        blocks[ref] = new FSEmptyBlock(*this, ref);
        markAsFree(ref);
    }
}

FSUserDirBlock *
FSVolume::newUserDirBlock(const char *name)
{
    u32 ref = allocateBlock();
    if (!ref) return nullptr;
    
    blocks[ref] = new FSUserDirBlock(*this, ref, name);
    return (FSUserDirBlock *)blocks[ref];
}

FSFileHeaderBlock *
FSVolume::newFileHeaderBlock(const char *name)
{
    u32 ref = allocateBlock();
    if (!ref) return nullptr;
    
    blocks[ref] = new FSFileHeaderBlock(*this, ref, name);
    return (FSFileHeaderBlock *)blocks[ref];
}

u32
FSVolume::addFileListBlock(u32 head, u32 prev)
{
    FSBlock *prevBlock = block(prev);
    if (!prevBlock) return 0;
    
    u32 ref = allocateBlock();
    if (!ref) return 0;
    
    blocks[ref] = new FSFileListBlock(*this, ref);
    blocks[ref]->setFileHeaderRef(head);
    prevBlock->setNextListBlockRef(ref);
    
    return ref;
}

u32
FSVolume::addDataBlock(u32 count, u32 head, u32 prev)
{
    FSBlock *prevBlock = block(prev);
    if (!prevBlock) return 0;

    u32 ref = allocateBlock();
    if (!ref) return 0;

    FSDataBlock *newBlock;
    if (isOFS()) {
        newBlock = new OFSDataBlock(*this, ref);
    } else {
        newBlock = new FFSDataBlock(*this, ref);
    }
    
    blocks[ref] = newBlock;
    newBlock->setDataBlockNr(count);
    newBlock->setFileHeaderRef(head);
    prevBlock->setNextDataBlockRef(ref);
    
    return ref;
}

void
FSVolume::installBootBlock()
{
    assert(blocks[0]->type() == FS_BOOT_BLOCK);
    assert(blocks[1]->type() == FS_BOOT_BLOCK);

    ((FSBootBlock *)blocks[0])->writeBootCode();
    ((FSBootBlock *)blocks[1])->writeBootCode();
}

void
FSVolume::updateChecksums()
{
    for (u32 i = 0; i < capacity; i++) {
        blocks[i]->updateChecksum();
    }
}

FSBlock *
FSVolume::currentDirBlock()
{
    FSBlock *cdb = block(currentDir);
    
    if (cdb) {
        if (cdb->type() == FS_ROOT_BLOCK || cdb->type() == FS_USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    currentDir = rootBlockNr();
    return rootBlock(); 
}

FSBlock *
FSVolume::changeDir(const char *name)
{
    assert(name != nullptr);

    FSBlock *cdb = currentDirBlock();

    if (strcmp(name, "/") == 0) {
                
        // Move to top level
        currentDir = rootBlockNr();
        return currentDirBlock();
    }

    if (strcmp(name, "..") == 0) {
                
        // Move one level up
        currentDir = cdb->getParentDirRef();
        return currentDirBlock();
    }
    
    FSBlock *subdir = seekDir(name);
    if (subdir == nullptr) return cdb;
    
    // Move one level down
    currentDir = subdir->nr;
    return currentDirBlock();
}

string
FSVolume::getPath(FSBlock *block)
{
    string result = "";
    std::set<u32> visited;
 
    while(block) {

        // Break the loop if this block has an invalid type
        if (!hashableBlock(block->nr)) break;

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
FSVolume::makeDir(const char *name)
{
    FSBlock *cdb = currentDirBlock();
    FSUserDirBlock *block = newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);
    
    return block;
}

FSBlock *
FSVolume::makeFile(const char *name)
{
    assert(name != nullptr);
 
    FSBlock *cdb = currentDirBlock();
    FSFileHeaderBlock *block = newFileHeaderBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);

    return block;
}

FSBlock *
FSVolume::makeFile(const char *name, const u8 *buffer, size_t size)
{
    assert(buffer != nullptr);

    FSBlock *block = makeFile(name);
    
    if (block) {
        assert(block->type() == FS_FILEHEADER_BLOCK);
        ((FSFileHeaderBlock *)block)->addData(buffer, size);
    }
    
    return block;
}

FSBlock *
FSVolume::makeFile(const char *name, const char *str)
{
    assert(str != nullptr);
    
    return makeFile(name, (const u8 *)str, strlen(str));
}

u32
FSVolume::seekRef(FSName name)
{
    std::set<u32> visited;
    
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) return 0;
    
    // Compute the table position and read the item
    u32 hash = name.hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);
    
    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {
        
        FSBlock *item = hashableBlock(ref);
        if (item == nullptr) break;
        
        if (item->isNamed(name)) return item->nr;

        visited.insert(ref);
        ref = item->getNextHashRef();
    }

    return 0;
}

void
FSVolume::addHashRef(u32 ref)
{
    if (FSBlock *block = hashableBlock(ref)) {
        addHashRef(block);
    }
}

void
FSVolume::addHashRef(FSBlock *newBlock)
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
FSVolume::printDirectory(bool recursive)
{
    std::vector<u32> items;
    collect(currentDir, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%d items\n", items.size());
}


FSBlock *
FSVolume::lastHashBlockInChain(u32 start)
{
    FSBlock *block = hashableBlock(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FSVolume::lastHashBlockInChain(FSBlock *block)
{
    std::set<u32> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextHashBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block =next;
    }
    return nullptr;
}

FSBlock *
FSVolume::lastFileListBlockInChain(u32 start)
{
    FSBlock *block = fileListBlock(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FSVolume::lastFileListBlockInChain(FSBlock *block)
{
    std::set<u32> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSFileListBlock *next = block->getNextListBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block = next;
    }
    return nullptr;
}

FSError
FSVolume::collect(u32 ref, std::vector<u32> &result, bool recursive)
{
    std::stack<u32> remainingItems;
    std::set<u32> visited;
    
    // Start with the items in this block
    collectHashedRefs(ref, remainingItems, visited);
    
    // Move the collected items to the result list
    while (remainingItems.size() > 0) {
        
        u32 item = remainingItems.top();
        remainingItems.pop();
        result.push_back(item);

        // Add subdirectory items to the queue
        if (userDirBlock(item) && recursive) {
            collectHashedRefs(item, remainingItems, visited);
        }
    }

    return FS_OK;
}

FSError
FSVolume::collectHashedRefs(u32 ref, std::stack<u32> &result, std::set<u32> &visited)
{
    if (FSBlock *b = block(ref)) {
        
        // Walk through the hash table in reverse order
        for (long i = (long)b->hashTableSize(); i >= 0; i--) {
            collectRefsWithSameHashValue(b->getHashRef(i), result, visited);
        }
    }
    
    return FS_OK;
}

FSError
FSVolume::collectRefsWithSameHashValue(u32 ref, std::stack<u32> &result, std::set<u32> &visited)
{
    std::stack<u32> refs;
    
    // Walk down the linked list
    for (FSBlock *b = hashableBlock(ref); b; b = b->getNextHashBlock()) {

        // Break the loop if we've already seen this block
        if (visited.find(b->nr) != visited.end()) return FS_HAS_CYCLES;
        visited.insert(b->nr);

        refs.push(b->nr);
    }
  
    // Push the collected elements onto the result stack
    while (refs.size() > 0) { result.push(refs.top()); refs.pop(); }
    
    return FS_OK;
}

u8
FSVolume::readByte(u32 block, u32 offset)
{
    assert(offset < bsize);

    if (block < capacity) {
        return blocks[block]->data ? blocks[block]->data[offset] : 0;
    }
    
    return 0;
}

bool
FSVolume::importVolume(const u8 *src, size_t size)
{
    FSError error;
    bool result = importVolume(src, size, &error);
    
    assert(result == (error == FS_OK));
    return result;
}

bool
FSVolume::importVolume(const u8 *src, size_t size, FSError *error)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        *error = FS_WRONG_BSIZE; return false;
    }
    // Only proceed if the source buffer contains the right amount of data
    if (capacity * bsize != size) {
        *error = FS_WRONG_CAPACITY; return false;
    }
    // Only proceed if the buffer contains a file system
    if (src[0] != 'D' || src[1] != 'O' || src[2] != 'S') {
        *error = FS_UNKNOWN; return false;
    }
    // Only proceed if the provided file system is supported
    if (src[3] > 1) {
        *error = FS_UNSUPPORTED; return false;
    }
    
    // Set the version number
    type = (FSVolumeType)src[3];
    
    // Import all blocks
    for (u32 i = 0; i < capacity; i++) {
        
        // Determine the type of the new block
        FSBlockType type = guessBlockType(i, src + i * bsize);
        
        // Create the new block
        FSBlock *newBlock = FSBlock::makeWithType(*this, i, type);
        if (newBlock == nullptr) return false;

        // Import the block data
        const u8 *p = src + i * bsize;
        newBlock->importBlock(p, bsize);

        // Replace the existing block
        assert(blocks[i] != nullptr);
        delete blocks[i];
        blocks[i] = newBlock;
    }
    
    *error = FS_OK;
    debug(FS_DEBUG, "Success\n");
    printDirectory(true);
    return true;
}

bool
FSVolume::exportVolume(u8 *dst, size_t size)
{
    return exportBlocks(0, capacity - 1, dst, size);
}

bool
FSVolume::exportVolume(u8 *dst, size_t size, FSError *error)
{
    return exportBlocks(0, capacity - 1, dst, size, error);
}

bool
FSVolume::exportBlock(u32 nr, u8 *dst, size_t size)
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSVolume::exportBlock(u32 nr, u8 *dst, size_t size, FSError *error)
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSVolume::exportBlocks(u32 first, u32 last, u8 *dst, size_t size)
{
    FSError error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == FS_OK));
    return result;
}

bool FSVolume::exportBlocks(u32 first, u32 last, u8 *dst, size_t size, FSError *error)
{
    assert(first < capacity);
    assert(last < capacity);
    assert(first <= last);
    assert(dst != nullptr);
    
    u32 count = last - first;
    
    debug(FS_DEBUG, "Exporting %d blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) { *error = FS_WRONG_BSIZE; return false; }

    // Only proceed if the source buffer contains the right amount of data
    if (count * bsize != size) { *error = FS_WRONG_CAPACITY; return false; }
        
    // Wipe out the target buffer
    memset(dst, 0, size);
    
    // Export all blocks
    for (u32 i = 0; i < count; i++) {
        
        blocks[first + i]->exportBlock(dst + i * bsize, bsize);
    }

    *error = FS_OK;
    debug(FS_DEBUG, "Success\n");
    return true;
}

bool
FSVolume::importDirectory(const char *path, bool recursive)
{
    assert(path != nullptr);

    DIR *dir;
    
    if ((dir = opendir(path))) {
        
        bool result = importDirectory(path, dir, recursive);
        closedir(dir);
        return result;
    }

    warn("Error opening directory %s\n", path);
    return false;
}

bool
FSVolume::importDirectory(const char *path, DIR *dir, bool recursive)
{
    assert(dir != nullptr);
    
    struct dirent *item;
    bool result = true;
    
    while ((item = readdir(dir))) {

        // Skip '.', '..' and all hidden files
        if (item->d_name[0] == '.') continue;

        // Assemble file name
        char *name = new char [strlen(path) + strlen(item->d_name) + 2];
        strcpy(name, path);
        strcat(name, "/");
        strcat(name, item->d_name);

        msg("importDirectory: Processing %s\n", name);
        
        if (item->d_type == DT_DIR) {
            
            // Add directory
            result &= makeDir(item->d_name) != nullptr;
            if (recursive && result) {
                changeDir(item->d_name);
                result &= importDirectory(name, recursive);
            }
            
        } else {
            
            // Add file
            u8 *buffer; long size;
            if (loadFile(name, &buffer, &size)) {
                FSBlock *file = makeFile(item->d_name, buffer, size);
                // result &= file ? (file->append(buffer, size)) : false;
                result &= file != nullptr;
                delete(buffer);
            }
        }
        
        delete [] name;
    }

    return result;
}

FSError
FSVolume::exportDirectory(const char *path)
{
    assert(path != nullptr);
        
    // Only proceed if path points to an empty directory
    long numItems = numDirectoryItems(path);
    if (numItems != 0) return FS_DIRECTORY_NOT_EMPTY;
    
    // Collect files and directories
    std::vector<u32> items;
    collect(currentDir, items);
    
    // Export all items
    for (auto const& i : items) {
        if (FSError error = block(i)->exportBlock(path); error != FS_OK) {
            msg("Export error: %d\n", error);
            return error; 
        }
    }
    
    msg("Exported %d items", items.size());
    return FS_OK;
}
