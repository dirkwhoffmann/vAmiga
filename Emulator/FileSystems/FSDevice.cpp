// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSDevice.h"
// #include <set>

FSDevice *
FSDevice::makeWithADF(ADFFile *adf, FSError *error)
{
    assert(adf != nullptr);

    // TODO: Determine file system type from ADF
    FSVolumeType type = FS_OFS;
    
    // Create file system
    FSDevice *volume = FSDevice::makeWithADF(adf, error);

    // Import file system from ADF
    if (!volume->importVolume(adf->getData(), adf->getSize(), error)) {
        delete volume;
        return nullptr;
    }
    
    return volume;
}

FSDevice *
FSDevice::makeWithHDF(HDFFile *hdf, FSError *error)
{
    assert(hdf != nullptr);

    // TODO: Determine file system type from HDF
    FSVolumeType type = FS_OFS;
    
    // Create file system
    FSDevice *volume = FSDevice::makeWithHDF(hdf, error);


    // Import file system from HDF
    /*
    if (!volume->importVolume(hdf->getData(), hdf->getSize(), error)) {
        delete volume;
        return nullptr;
    }
    */
    
    return volume;
}

FSDevice *
FSDevice::makeWithFormat(DiskType type, DiskDensity density)
{
    FSLayout layout = FSLayout(type, density);
    return new FSDevice(layout);
}

/*
FSDevice *
FSDevice::make(FSVolumeType type, u32 cyls, u32 heads, u32 sectors, u32 bsize)
{
    FSDevice *dev = new FSDevice();
        
    dev->type = type;
    dev->cylinders = cyls;
    dev->heads = heads;
    dev->sectors = sectors;
    dev->bsize = bsize;
    dev->capacity = cyls * heads * sectors;
    
    // Create the block storage
    dev->blocks = new BlockPtr[dev->capacity]();
    dev->dsize = dev->isOFS() ? bsize - 24 : bsize; // TODO: MOVE TO PARTITION

    // Create the first partition
    dev->part.push_back(FSPartition(0, cyls - 1, sectors * heads, 880));

    // Determine the block locations
    u32 root             = 880; // rootBlockNr();
    u32 firstBitmapBlock = root + 1;
    u32 lastBitmapBlock  = firstBitmapBlock + dev->requiredBitmapBlocks() - 1;
    u32 firstExtBlock    = lastBitmapBlock + 1;
    u32 lastExtBlock     = firstExtBlock + dev->requiredBitmapExtensionBlocks() - 1;
    assert(root < dev->capacity);

    // Add boot blocks
    dev->blocks[0] = new FSBootBlock(*dev, 0);
    dev->blocks[1] = new FSBootBlock(*dev, 1);
    
    // Add the root block
    FSRootBlock *rb = new FSRootBlock(*dev, root);
    dev->blocks[root] = rb;
        
    // Add bitmap blocks
    for (u32 ref = firstBitmapBlock; ref <= lastBitmapBlock; ref++) {
        dev->blocks[ref] = new FSBitmapBlock(*dev, ref);
        dev->part[0].bmBlocks.push_back(ref);
    }

    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (u32 ref = firstExtBlock; ref <= lastExtBlock; ref++) {
        dev->blocks[ref] = new FSBitmapExtBlock(*dev, ref);
        dev->part[0].bmExtBlocks.push_back(ref);
        pred->setNextBmExtBlockRef(ref);
        pred = dev->blocks[ref];
    }

    // Add all bitmap block references
    rb->addBitmapBlockRefs(dev->part[0].bmBlocks);
        
    // Add free blocks
    for (u32 i = 2; i < root; i++) {
        assert(dev->blocks[i] == nullptr);
        dev->blocks[i] = new FSEmptyBlock(*dev, i);
        dev->markAsFree(i);
    }
    for (u32 i = lastExtBlock + 1; i < dev->capacity; i++) {
        assert(dev->blocks[i] == nullptr);
        dev->blocks[i] = new FSEmptyBlock(*dev, i);
        dev->markAsFree(i);
    }

    // Compute checksums for all blocks
    dev->updateChecksums();
    
    // Set the current directory to '/'
    dev->cd = dev->part[0].rootBlock;

    if (FS_DEBUG) {
        printf("cd = %d\n", dev->cd);
        dev->info();
        dev->dump();
    }
    
    return dev; 
}
*/

FSDevice *
FSDevice::make(DiskType type, DiskDensity density, const char *path)
{
    FSDevice *device = makeWithFormat(type, density);
    
    if (device) {
        
        // Try to import directory
        if (!device->importDirectory(path)) { delete device; return nullptr; }

        // Assign device name
        device->setName(FSName("Directory")); // TODO: Use last path component

        // Change to the root directory
        device->changeDir("/");
    }

    return device;
}

/*
FSDevice *
FSDevice::make(FSVolumeType type, const char *name, const char *path, u32 cylinders, u32 heads, u32 sectors)
{
    FSDevice *volume = FSDevice::make(type, cylinders, heads, sectors);
    volume->setName(FSName(name));

    // Try to import directory
    if (!volume->importDirectory(path)) { delete volume; return nullptr; }
    
    // Change to root directory and return
    volume->changeDir("/");
    return volume;
}
*/

FSDevice *
FSDevice::make(FSVolumeType type, const char *path)
{
    //     FSDevice *volume;
    
    // Try to fit the directory into files system with DD disk capacity
    // if ((volume = make(type, name, path, 80, 2, 11))) return volume;
    if (FSDevice *device = make(DISK_35, DISK_DD, path)) return device;

    // Try to fit the directory into files system with HD disk capacity
    // if ((volume = make(type, name, path, 80, 2, 22))) return volume;
    if (FSDevice *device = make(DISK_35, DISK_HD, path)) return device;

    return nullptr;
}

// FSDevice::FSDevice(FSVolumeType t, u32 c, u32 s) :  type(t), capacity(c), bsize(s)
/*
FSDevice::FSDevice(FSVolumeType type, u32 c, u32 h, u32 s, u32 bsize)
{
    debug(FS_DEBUG, "Creating FSDevice (%d x %d x %d x %d)\n", c, h, s, bsize);
    
    this->type = type;
    this->cylinders = c;
    this->heads = h;
    this->sectors = s;
    this->bsize = bsize;
    capacity = c * h * s;
    
    // Create the block storage
    blocks = new BlockPtr[capacity]();
    dsize = isOFS() ? bsize - 24 : bsize; // TODO: MOVE TO PARTITION

    // Create the first partition
    part.push_back(FSPartition(0, c-1, 880));

    // Determine the block locations
    u32 root             = 880; // rootBlockNr();
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
        part[0].bmBlocks.push_back(ref);
    }

    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (u32 ref = firstExtBlock; ref <= lastExtBlock; ref++) {
        blocks[ref] = new FSBitmapExtBlock(*this, ref);
        part[0].bmExtBlocks.push_back(ref);
        pred->setNextBmExtBlockRef(ref);
        pred = blocks[ref];
    }

    // Add all bitmap block references
    rb->addBitmapBlockRefs(part[0].bmBlocks);
        
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
    cd = part[0].rootBlock;

    if (FS_DEBUG) {
        debug("cd = %d\n", cd);
        info();
        dump();
    }
}
*/
 
FSDevice::FSDevice(FSLayout &l)
{
    if (FS_DEBUG) {
        debug("Creating FSDevice with layout:\n");
        l.dump();
    }
    
    this->layout = l;
    
    type = FS_OFS; // TODO: REMOVE: THIS IS A PARTITION PROPERTY
    cylinders = layout.cyls;
    heads = layout.heads;
    sectors = layout.sectors;
    bsize = layout.bsize;
    capacity = layout.blocks;
    
    // Create the block storage
    blocks = new BlockPtr[layout.blocks]();
        
    // Iterate through all partitions
    for (auto& it : layout.part) {
    
        u32 first = it.firstBlock;
        
        // Create boot blocks
        blocks[first] = new FSBootBlock(*this, first, it.dos);
        blocks[first + 1] = new FSBootBlock(*this, first + 1, it.dos);

        // Create the root block
        FSRootBlock *rb = new FSRootBlock(*this, it.rootBlock);
        blocks[it.rootBlock] = rb;
        
        // Create the bitmap blocks
        for (auto& bmb : it.bmBlocks) {
            
            debug("Creating bitmap block at %d\n", bmb);
            blocks[bmb] = new FSBitmapBlock(*this, bmb);
        }
        
        // Add bitmap extension blocks
        FSBlock *pred = rb;
        for (auto& ext : it.bmExtBlocks) {
            
            blocks[ext] = new FSBitmapExtBlock(*this, ext);
            pred->setNextBmExtBlockRef(ext);
            pred = blocks[ext];
        }
        
        // Add all bitmap block references
        rb->addBitmapBlockRefs(it.bmBlocks);
    }

    // Add free blocks
    for (u32 i = 0; i < layout.blocks; i++) {
        
        if (blocks[i] == nullptr) {
            blocks[i] = new FSEmptyBlock(*this, i);
            markAsFree(i); // TODO: MUST BE PARTITION SPECIFIC
        }
    }

    // Compute checksums for all blocks
    updateChecksums();
    
    // Set the current directory to '/'
    cd = layout.part[0].rootBlock;
    
    if (FS_DEBUG) {
        printf("cd = %d\n", cd);
        info();
        dump();
    }
}

FSDevice::~FSDevice()
{
    for (u32 i = 0; i < capacity; i++) {
        delete blocks[i];
    }
    delete [] blocks;
}

void
FSDevice::info()
{
    msg("Type   Size          Used   Free   Full   Name\n");
    for (auto& p : layout.part) {
        msg("DOS%ld  ",     fileSystem(p));
        msg("%5d (x %3d) ", numBlocks(p), bsize);
        msg("%5d  ",        usedBlocks(p));
        msg("%5d   ",       freeBlocks(p));
        msg("%3d%%   ",     (int)(100.0 * usedBlocks(p) / numBlocks(p)));
        msg("%s\n",         getName(p).c_str());
        msg("\n");
    }
}

void
FSDevice::dump()
{
    msg("                  Name : %s\n", getName().c_str());
    msg("           File system : DOS%ld (%s)\n", getType(), sFSVolumeType(getType()));
    msg("\n");
    msg("  Bytes per data block : %d\n", getDataBlockCapacity());
    msg(" Bits per bitmap block : %d\n", getAllocBitsInBitmapBlock());
    msg("     Bitmap block refs : %d (in root block)\n", bitmapRefsInRootBlock());
    msg("                         %d (in ext block)\n", bitmapRefsInBitmapExtensionBlock());
    msg("\n");
    
    for (size_t i = 0; i < part.size(); i++) {

        FSVolumeType dos = fileSystem(part[i]);
        msg("           Partition %d : DOS%ld (%s)\n", i, dos, sFSVolumeType(dos));
        part[i].dump();
    }
    msg("\n");

    layout.dump();
    msg("\n");

    for (size_t i = 0; i < capacity; i++)  {
        
        if (blocks[i]->type() == FS_EMPTY_BLOCK) continue;
        
        msg("\nBlock %d (%d):", i, blocks[i]->nr);
        msg(" %s\n", sFSBlockType(blocks[i]->type()));
                
        blocks[i]->dump(); 
    }
}

FSErrorReport
FSDevice::check(bool strict)
{
    long total = 0, min = LONG_MAX, max = 0;
    
    // Analyze all blocks
    for (u32 i = 0; i < capacity; i++) {

        if (blocks[i]->type() == FS_EMPTY_BLOCK && !isFree(i)) {
            debug(FS_DEBUG, "Empty block %d is marked as allocated\n", i);
        }
        if (blocks[i]->type() != FS_EMPTY_BLOCK && isFree(i)) {
            debug(FS_DEBUG, "Non-empty block %d is marked as free\n", i);
        }

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
FSDevice::check(u32 blockNr, u32 pos, u8 *expected, bool strict)
{
    return blocks[blockNr]->check(pos, expected, strict);
}

FSError
FSDevice::checkBlockType(u32 nr, FSBlockType type)
{
    return checkBlockType(nr, type, type);
}

FSError
FSDevice::checkBlockType(u32 nr, FSBlockType type, FSBlockType altType)
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
            case FS_FILELIST_BLOCK:   return FS_PTR_TO_FILELIST_BLOCK;
            case FS_DATA_BLOCK_OFS:   return FS_PTR_TO_DATA_BLOCK;
            case FS_DATA_BLOCK_FFS:   return FS_PTR_TO_DATA_BLOCK;
            default:                  return FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return FS_OK;
}

u32
FSDevice::getCorrupted(u32 blockNr)
{
    return block(blockNr) ? blocks[blockNr]->corrupted : 0;
}

bool
FSDevice::isCorrupted(u32 blockNr, u32 n)
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
FSDevice::nextCorrupted(u32 blockNr)
{
    long i = (long)blockNr;
    while (++i < capacity) { if (isCorrupted(i)) return i; }
    return blockNr;
}

u32
FSDevice::prevCorrupted(u32 blockNr)
{
    long i = (long)blockNr - 1;
    while (i-- >= 0) { if (isCorrupted(i)) return i; }
    return blockNr;
}

u32
FSDevice::seekCorruptedBlock(u32 n)
{
    for (u32 i = 0, cnt = 0; i < capacity; i++) {

        if (isCorrupted(i)) {
            cnt++;
            if (cnt == n) return i;
        }
    }
    return (u32)-1;
}

u32
FSDevice::partitionForBlock(u32 ref)
{
    for (u32 i = 0; i < layout.part.size(); i++) {

        FSPartition &part = layout.part[i];
        if (ref >= part.firstBlock && ref <= part.lastBlock) return i;
    }
    
    assert(false);
    return 0;
}

FSName
FSDevice::getName(FSPartition &part)
{
    FSRootBlock *rb = rootBlock(part.rootBlock);
    assert(rb != nullptr);
    
    return rb->getName();
}

void
FSDevice::setName(FSPartition &part, FSName name)
{
    FSRootBlock *rb = rootBlock(part.rootBlock);
    assert(rb != nullptr);

    rb->setName(name);
}

FSVolumeType
FSDevice::fileSystem(FSPartition &part)
{
    FSBlock *b = block(part.firstBlock);

    return b ? b->fileSystem() : FS_NONE;
}

FSBlockType
FSDevice::predictBlockType(u32 nr, const u8 *buffer)
{
    assert(buffer != nullptr);

    // Determine the partition the belongs to
    FSPartition &p = part[partitionForBlock(nr)];
    
    // Is it a boot block?
    if (nr == p.firstBlock + 0) return FS_BOOT_BLOCK;
    if (nr == p.firstBlock + 1) return FS_BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(p.bmBlocks.begin(), p.bmBlocks.end(), nr) != p.bmBlocks.end())
        return FS_BITMAP_BLOCK;
    
    // is it a bitmap extension block?
    if (std::find(p.bmExtBlocks.begin(), p.bmExtBlocks.end(), nr) != p.bmExtBlocks.end())
        return FS_BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize - 4);

    if (type == 2  && subtype == 1)       return FS_ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FS_USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FS_FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FS_FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS(p)) {
        if (type == 8) return FS_DATA_BLOCK_OFS;
    } else {
        for (u32 i = 0; i < bsize; i++) if (buffer[i]) return FS_DATA_BLOCK_FFS;
    }
    
    return FS_EMPTY_BLOCK;
}

bool
FSDevice::isOFS()
{
    return
    type == FS_OFS ||
    type == FS_OFS_INTL ||
    type == FS_OFS_DC ||
    type == FS_OFS_LNFS;
}

bool
FSDevice::isFFS()
{
    return
    type == FS_FFS ||
    type == FS_FFS_INTL ||
    type == FS_FFS_DC ||
    type == FS_FFS_LNFS;
}

u32
FSDevice::getAllocBitsInBitmapBlock()
{
    return (bsize - 4) * 8;
}

u32
FSDevice::getDataBlockCapacity()
{
    if (isOFS()) {
        return bsize - OFSDataBlock::headerSize();
    } else {
        return bsize - FFSDataBlock::headerSize();
    }
}

u32
FSDevice::bitmapRefsInRootBlock()
{
    return 25;
}

u32
FSDevice::bitmapRefsInBitmapExtensionBlock()
{
    return (bsize / 4) - 1;
}

u32
FSDevice::numBlocks(FSPartition &p)
{
    return p.lastBlock - p.firstBlock + 1;
}

u32
FSDevice::freeBlocks(FSPartition &p)
{
    u32 result = 0;
    
    /*
    for (size_t i = p.firstBlock; i <= p.lastBlock; i++) {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) result++;
    }
    */
    for (size_t i = p.firstBlock; i <= p.lastBlock; i++) {
        if (isFree(i)) result++;
    }

    return result;
}

u32
FSDevice::usedBlocks(FSPartition &p)
{
    return numBlocks(p) - freeBlocks(p);
}

u32
FSDevice::totalBytes(FSPartition &p)
{
    return numBlocks(p) * bsize;
}

u32
FSDevice::freeBytes(FSPartition &p)
{
    return freeBlocks(p) * bsize;
}

u32
FSDevice::usedBytes(FSPartition &p)
{
    return usedBlocks(p) * bsize;
}

/*
u32
FSDevice::freeBlocks()
{
    u32 result = 0;
    
    for (size_t i = 0; i < capacity; i++)  {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) result++;
    }
    
    return result;
}
*/

bool
FSDevice::isFree(u32 ref)
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
FSDevice::mark(u32 ref, bool alloc)
{
    u32 block, byte, bit;
    
    if (locateAllocationBit(ref, &block, &byte, &bit)) {
        if (FSBitmapBlock *bm = bitmapBlock(block)) {
            
            // 0 = allocated, 1 = free
            alloc ? CLR_BIT(bm->data[byte], bit) : SET_BIT(bm->data[byte], bit);
        }
    }
}

/*
void
FSDevice::locateBitmapBlocks(const u8 *buffer)
{
    assert(buffer != nullptr);

    //
    // DEPRECATED. MOVE TO HDF
    //
    
    std::vector<u32> &bmBlocks = part[0].bmBlocks;
    std::vector<u32> &bmExtBlocks = part[0].bmExtBlocks;

    bmBlocks.clear();
    bmExtBlocks.clear();
    
    // Start at the root block location
    u32 ref = 880; // TODO: THIS WON'T WORK FOR HARD DRIVES
    u32 cnt = 25;
    u32 offset = bsize - 49 * 4;
    
    while (ref) {

        const u8 *p = buffer + (ref * bsize) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (u32 i = 0; i < cnt; i++, p += 4) {
            if (u32 bmb = FFSDataBlock::read32(p)) {
                if (bmb < capacity) {debug("Adding %d\n", bmb); bmBlocks.push_back(bmb);}
            } else {
                return;
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FFSDataBlock::read32(p))) {
            if (ref < capacity) bmExtBlocks.push_back(ref);
            cnt = (bsize / 4) - 1;
            offset = 0;
        }
    }
}
*/

bool
FSDevice::locateAllocationBit(u32 ref, u32 *block, u32 *byte, u32 *bit)
{
    assert(ref < layout.blocks);
    
    // Select the correct partition
    FSPartition &p = layout.part[partitionForBlock(ref)];
    auto &bmBlocks = p.bmBlocks;
    
    // Make 'rel' relative to the partition start
    ref -= p.firstBlock;

    // The first two blocks are not part the map (they are always allocated)
    if (ref < 2) return false;
    
    // Locate the bitmap block
    u32 nr = ref / getAllocBitsInBitmapBlock();
    if (nr >= bmBlocks.size()) {
        warn("Allocation bit is located in a non-existent bitmap block %d\n", nr);
        return false;
    }
    u32 rBlock = bmBlocks[nr];
    assert(bitmapBlock(rBlock));

    // Locate the byte position (the long word ordering will be inversed)
    u32 rByte = (ref / 8);
    
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
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, *block, *byte, *bit);

    return true;
}

FSBlockType
FSDevice::blockType(u32 nr)
{
    return block(nr) ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
}

FSItemType
FSDevice::itemType(u32 nr, u32 pos)
{
    return block(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FSDevice::block(u32 nr)
{
    if (nr < capacity) {
        return blocks[nr];
    } else {
        return nullptr;
    }
}

FSBootBlock *
FSDevice::bootBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() != FS_BOOT_BLOCK)
    {
        return (FSBootBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSRootBlock *
FSDevice::rootBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_ROOT_BLOCK) {
        return (FSRootBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSBitmapBlock *
FSDevice::bitmapBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_BITMAP_BLOCK) {
        return (FSBitmapBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSBitmapExtBlock *
FSDevice::bitmapExtBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_BITMAP_EXT_BLOCK) {
        return (FSBitmapExtBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSUserDirBlock *
FSDevice::userDirBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_USERDIR_BLOCK) {
        return (FSUserDirBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSFileHeaderBlock *
FSDevice::fileHeaderBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_FILEHEADER_BLOCK) {
        return (FSFileHeaderBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSFileListBlock *
FSDevice::fileListBlock(u32 nr)
{
    if (nr < capacity && blocks[nr]->type() == FS_FILELIST_BLOCK) {
        return (FSFileListBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSDataBlock *
FSDevice::dataBlock(u32 nr)
{
    FSBlockType t = blocks[nr]->type();
    if (nr < capacity && (t == FS_DATA_BLOCK_OFS || t == FS_DATA_BLOCK_FFS)) {
        return (FSDataBlock *)blocks[nr];
    } else {
        return nullptr;
    }
}

FSBlock *
FSDevice::hashableBlock(u32 nr)
{
    FSBlockType type = nr < capacity ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
    
    if (type == FS_USERDIR_BLOCK || type == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    } else {
        return nullptr;
    }
}

u32
FSDevice::allocateBlock(FSPartition &part)
{
    if (u32 ref = allocateBlockAbove(part, part.rootBlock)) return ref;
    if (u32 ref = allocateBlockBelow(part, part.rootBlock)) return ref;

    return 0;
}

u32
FSDevice::allocateBlockAbove(FSPartition &part, u32 ref)
{
    for (u32 i = ref + 1; i <= part.lastBlock; i++) {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            markAsAllocated(i);
            return i;
        }
    }
    return 0;
}

u32
FSDevice::allocateBlockBelow(FSPartition &part, u32 ref)
{
    for (long i = (long)ref - 1; i >= part.firstBlock; i--) {
        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            markAsAllocated(i);
            return i;
        }
    }
    return 0;
}

void
FSDevice::deallocateBlock(u32 ref)
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
FSDevice::newUserDirBlock(FSPartition &p, const char *name)
{
    u32 ref = allocateBlock(p);
    if (!ref) return nullptr;
    
    blocks[ref] = new FSUserDirBlock(*this, ref, name);
    return (FSUserDirBlock *)blocks[ref];
}

FSFileHeaderBlock *
FSDevice::newFileHeaderBlock(FSPartition &p, const char *name)
{
    u32 ref = allocateBlock(p);
    if (!ref) return nullptr;
    
    blocks[ref] = new FSFileHeaderBlock(*this, ref, name);
    return (FSFileHeaderBlock *)blocks[ref];
}

u32
FSDevice::addFileListBlock(u32 head, u32 prev)
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
FSDevice::addDataBlock(u32 count, u32 head, u32 prev)
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
FSDevice::makeBootable(FSPartition &part, FSBootCode bootCode)
{
    u32 first = part.firstBlock;
    
    assert(blocks[first + 0]->type() == FS_BOOT_BLOCK);
    assert(blocks[first + 1]->type() == FS_BOOT_BLOCK);

    ((FSBootBlock *)blocks[first + 0])->writeBootCode(bootCode, 0);
    ((FSBootBlock *)blocks[first + 1])->writeBootCode(bootCode, 1);
}

void
FSDevice::updateChecksums()
{
    for (u32 i = 0; i < capacity; i++) {
        blocks[i]->updateChecksum();
    }
}

FSBlock *
FSDevice::currentDirBlock()
{
    FSBlock *cdb = block(cd);
    
    if (cdb) {
        if (cdb->type() == FS_ROOT_BLOCK || cdb->type() == FS_USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    cd = part[cp].rootBlock;
    return block(cd);
}

FSBlock *
FSDevice::changeDir(const char *name)
{
    assert(name != nullptr);

    FSBlock *cdb = currentDirBlock();

    if (strcmp(name, "/") == 0) {
                
        // Move to top level
        cd = layout.part[cp].rootBlock;
        return currentDirBlock();
    }

    if (strcmp(name, "..") == 0) {
                
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
FSDevice::makeDir(const char *name)
{
    FSBlock *cdb = currentDirBlock();
    FSUserDirBlock *block = newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);
    
    return block;
}

FSBlock *
FSDevice::makeFile(const char *name)
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
FSDevice::makeFile(const char *name, const u8 *buffer, size_t size)
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
FSDevice::makeFile(const char *name, const char *str)
{
    assert(str != nullptr);
    
    return makeFile(name, (const u8 *)str, strlen(str));
}

u32
FSDevice::requiredDataBlocks(FSPartition &p, size_t fileSize)
{
    // Compute the capacity of a single data block
    u32 numBytes = bsize - (isOFS(p) ? OFSDataBlock::headerSize() : 0);

    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

u32
FSDevice::requiredFileListBlocks(FSPartition &p, size_t fileSize)
{
    // Compute the required number of data blocks
    u32 numBlocks = requiredDataBlocks(p, fileSize);
    
    // Compute the number of data block references in a single block
    u32 numRefs = (bsize / 4) - 56;

    // Small files do not require any file list block
    if (numBlocks <= numRefs) return 0;

    // Compute the required number of additional file list blocks
    return (numBlocks - 1) / numRefs;
}

u32
FSDevice::requiredBlocks(FSPartition &p, size_t fileSize)
{
    u32 numDataBlocks = requiredDataBlocks(p, fileSize);
    u32 numFileListBlocks = requiredFileListBlocks(p, fileSize);
    
    if (FS_DEBUG) {
        debug("Required file header blocks : %d\n", 1);
        debug("       Required data blocks : %d\n", numDataBlocks);
        debug("  Required file list blocks : %d\n", numFileListBlocks);
        debug("                Free blocks : %d\n", freeBlocks());
    }
    
    return 1 + numDataBlocks + numFileListBlocks;
}

u32
FSDevice::seekRef(FSName name)
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
FSDevice::addHashRef(u32 ref)
{
    if (FSBlock *block = hashableBlock(ref)) {
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
    std::vector<u32> items;
    collect(cd, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%d items\n", items.size());
}


FSBlock *
FSDevice::lastHashBlockInChain(u32 start)
{
    FSBlock *block = hashableBlock(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastHashBlockInChain(FSBlock *block)
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
FSDevice::lastFileListBlockInChain(u32 start)
{
    FSBlock *block = fileListBlock(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastFileListBlockInChain(FSBlock *block)
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
FSDevice::collect(u32 ref, std::vector<u32> &result, bool recursive)
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
FSDevice::collectHashedRefs(u32 ref, std::stack<u32> &result, std::set<u32> &visited)
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
FSDevice::collectRefsWithSameHashValue(u32 ref, std::stack<u32> &result, std::set<u32> &visited)
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
FSDevice::readByte(u32 block, u32 offset)
{
    assert(offset < bsize);

    if (block < capacity) {
        return blocks[block]->data ? blocks[block]->data[offset] : 0;
    }
    
    return 0;
}

bool
FSDevice::importVolume(const u8 *src, size_t size)
{
    FSError error;
    bool result = importVolume(src, size, &error);
    
    assert(result == (error == FS_OK));
    return result;
}

bool
FSDevice::importVolume(const u8 *src, size_t size, FSError *error)
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
    if (src[3] > 7) {
        *error = FS_UNSUPPORTED; return false;
    }

    FSVolumeType dos = (FSVolumeType)src[3];

    // Set the version number
    // type = (FSVolumeType)src[3];
    
    // Scan the buffer for bitmap blocks and bitmap extension blocks
    // locateBitmapBlocks(src);
    assert(false);
    
    // Import all blocks
    for (u32 i = 0; i < capacity; i++) {
        
        // Determine the type of the new block
        FSBlockType type = predictBlockType(i, src + i * bsize);
        
        // Create the new block
        FSBlock *newBlock = FSBlock::makeWithType(*this, i, type, dos);
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
    info();
    dump();
    printDirectory(true);
    return true;
}

bool
FSDevice::exportVolume(u8 *dst, size_t size)
{
    return exportBlocks(0, capacity - 1, dst, size);
}

bool
FSDevice::exportVolume(u8 *dst, size_t size, FSError *error)
{
    return exportBlocks(0, capacity - 1, dst, size, error);
}

bool
FSDevice::exportBlock(u32 nr, u8 *dst, size_t size)
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSDevice::exportBlock(u32 nr, u8 *dst, size_t size, FSError *error)
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSDevice::exportBlocks(u32 first, u32 last, u8 *dst, size_t size)
{
    FSError error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == FS_OK));
    return result;
}

bool FSDevice::exportBlocks(u32 first, u32 last, u8 *dst, size_t size, FSError *error)
{
    assert(first < capacity);
    assert(last < capacity);
    assert(first <= last);
    assert(dst != nullptr);
    
    u32 count = last - first + 1;
    
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
FSDevice::importDirectory(const char *path, bool recursive)
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
FSDevice::importDirectory(const char *path, DIR *dir, bool recursive)
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
FSDevice::exportDirectory(const char *path)
{
    assert(path != nullptr);
        
    // Only proceed if path points to an empty directory
    long numItems = numDirectoryItems(path);
    if (numItems != 0) return FS_DIRECTORY_NOT_EMPTY;
    
    // Collect files and directories
    std::vector<u32> items;
    collect(cd, items);
    
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
