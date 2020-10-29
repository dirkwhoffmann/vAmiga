// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"
#include "Utils.h"

FSVolume::FSVolume(const char *name, u32 c, u32 s) : capacity(c), bsize(s)
{
    setDescription("Volume");
        
    // Initialize block storage
    blocks = new BlockPtr[capacity];
    for (u32 i = 0; i < capacity; i++) blocks[i] = new FSBlock(*this, i);
        
    // Install the bitmap block
    u32 bitmap = bitmapBlockNr();
    assert(bitmap < capacity);
    delete blocks[bitmap];
    blocks[bitmap] = new FSBitmapBlock(*this, bitmap);
    assert(bitmapBlock() == blocks[bitmap]);
    bitmapBlock()->alloc(bitmap);

    // Install the root block
    u32 root = rootBlockNr();
    assert(root < capacity);
    delete blocks[root];
    blocks[root] = new FSRootBlock(*this, root, name);
    assert(rootBlock() == blocks[root]);
    bitmapBlock()->alloc(root);

    // Set the current directory to '/'
    currentDir = rootBlockNr();
    
    debug("Volume created\n");
}

FSVolume::~FSVolume()
{
    delete [] blocks;
}

void
FSVolume::dump()
{
    debug("Volume: (%s)\n", type == OFS ? "OFS" : "FFS");
    
    debug("Block list:\n");
    
    for (size_t i = 0; i < capacity; i++)  {
        
        if (blocks[i]->type() == FS_EMPTY_BLOCK) continue;
        
        msg("Block %d (%d):", i, blocks[i]->nr);
        msg(" %s\n", fsBlockTypeName(blocks[i]->type()));
                
        blocks[i]->dump(); 
    }
}

bool
FSVolume::check(bool verbose)
{
    bool result = true;
    
    if (verbose) fprintf(stderr, "Checking volume...\n");
    
    for (u32 i = 0; i < capacity; i++) {
                
        if (blocks[i]->type() == FS_EMPTY_BLOCK) continue;

        if (verbose) {
            fprintf(stderr, "Inspecting block %d (%s) ...\n",
                    i, fsBlockTypeName(blocks[i]->type()));
        }

        result &= blocks[i]->check(verbose);
    }
    
    printf("The volume is %s.\n", result ? "sound" : "corrupted");
    return result;
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

u32
FSVolume::allocateBlock()
{
    u32 result;
    
    // Search for a free block above the root block
    if ((result = allocateAbove(rootBlockNr()))) { return result; }

    // Search for a free block below the root block
    if ((result = allocateBelow(rootBlockNr()))) { return result; }
    
    return 0;
}

u32
FSVolume::allocateBlock(u32 start, int incr)
{
    for (long i = start; i > 0 && i < capacity; i += incr) {

        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            assert(!bitmapBlock()->isAllocated(i));
            bitmapBlock()->alloc(i);
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
        blocks[ref] = new FSBlock(*this, ref);
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

FSFileListBlock *
FSVolume::newFileListBlock()
{
    u32 ref = allocateBlock();
    if (!ref) return nullptr;
    
    blocks[ref] = new FSFileListBlock(*this, ref);
    return (FSFileListBlock *)blocks[ref];
}

FSDataBlock *
FSVolume::newDataBlock()
{
    u32 ref = allocateBlock();
    if (!ref) return nullptr;
    
    blocks[ref] = new FSDataBlock(*this, ref);
    return (FSDataBlock *)blocks[ref];
}

/*
void
FSVolume::replaceBlock(long nr, FSBlock *block)
{
    assert(nr < capacity);
    assert(blocks[nr] != nullptr);
    
    delete blocks[nr];
    blocks[nr] = block;
    blocks[nr]->nr = nr;
}
*/

/*
void
FSVolume::addBlock(long nr, FSBlock *block)
{
    assert(nr < capacity);
    
    // Remove old block
    removeBlock(nr);
    
    // Add new block
    blocks[nr] = block;
    blocks[nr]->nr = nr;

    // Mark block as used
    bitmapBlock()->alloc(nr);
}
*/

/*
void
FSVolume::removeBlock(long nr)
{
    assert(nr < capacity);
    assert(blocks[nr] != nullptr);
    
    // Delete block
    delete blocks[nr];
    blocks[nr] = nullptr;
    
    // Mark block as free
    bitmapBlock()->dealloc(nr);
}
*/

/*
long
FSVolume::freeBlock()
{
    // Search for a free block above the root block
    for (long i = rootBlockNr() + 2; i < capacity; i++) {

        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            assert(!bitmapBlock()->isAllocated(i));
            return i;
        }
    }

    // Search for a free block below the root block
    for (long i = rootBlockNr() - 1; i > 1; i--) {

        if (blocks[i]->type() == FS_EMPTY_BLOCK) {
            assert(!bitmapBlock()->isAllocated(i));
            return i;
        }
    }
    
    return -1;
}
*/

void
FSVolume::installBootBlock()
{
    debug("installBootBlock()");
    delete blocks[0];
    blocks[0] = new FSBootBlock(*this, 0);
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

bool
FSVolume::changeDir(const char *name)
{
    assert(name != nullptr);

    FSBlock *cdb = currentDirBlock();
    
    if (strcmp(name, "..") == 0) {
        
        u32 parent = cdb->getParent();
        if (parent == 0) return false;
        
        // Go one level up
        currentDir = parent;
        return true;
    }
    
    FSBlock *subdir = cdb->seek(name);
    if (!subdir) {
        printf("Unable to find subdirectory %s\n", name);
        return false;
    }
    
    // Go one level down
    currentDir = subdir->nr;
    
    printf("New current directory (%d %d %d): ", currentDir, currentDirBlock()->nr, currentDirBlock()->type());
    currentDirBlock()->printPath();
    printf("\n");
    
    return true;
}

FSUserDirBlock *
FSVolume::makeDir(const char *name)
{
    debug("makeDir(%s)\n", name);
    
    FSBlock *cdb = currentDirBlock();
    FSUserDirBlock *block = newUserDirBlock(name);
    block->setParent(cdb->nr);

    return cdb->addHashBlock(block) ? block : nullptr;
}

FSFileHeaderBlock *
FSVolume::makeFile(const char *name)
{
    debug("makeFile(%s)\n", name);

    FSBlock *cdb = currentDirBlock();
    FSFileHeaderBlock *block = newFileHeaderBlock(name);
    block->setParent(cdb->nr);
    
    return cdb->addHashBlock(block) ? block : nullptr;
}

FSBlock *
FSVolume::seek(const char *name)
{
    debug("seekItem(%s)\n", name);
    
    FSBlock *cdb = currentDirBlock();
    return cdb->seek(name);
}

FSUserDirBlock *
FSVolume::seekDir(const char *name)
{
    FSBlock *block = seek(name);

    if (!block || block->type() != FS_USERDIR_BLOCK) return nullptr;
    
    return (FSUserDirBlock *)block;
}

FSFileHeaderBlock *
FSVolume::seekFile(const char *name)
{
    FSBlock *block = seek(name);

    if (!block || block->type() != FS_FILEHEADER_BLOCK) return nullptr;

    return (FSFileHeaderBlock *)block;
}

void
FSVolume::writeAsDisk(u8 *dst, size_t length)
{
    assert(dst != nullptr);
    assert(length % 512 == 0);
 
    size_t sectorCnt = length / 512;
    assert(sectorCnt <= 2 * 84 * 11);

    debug("writeAsDisk(%x, %d) sectors: %d\n", dst, length, sectorCnt);
    dump();
        
    for (int i = 0; i < capacity; i++) {

        u8 *sector = dst + i * 512;
        
        // Only proceed if the disk has space for this block
        if (i >= sectorCnt) {
            printf("Skipping block %d (>= %zu)\n", i, sectorCnt);
            continue;
        }
        
        if (blocks[i]->type() != FS_EMPTY_BLOCK) debug("Exporting block %ld\n", i);
        assert(blocks[i]->nr == i);
        blocks[i]->write(sector);
    }
    
    debug("writeAsDisk() DONE\n");
}

OFSVolume::OFSVolume(const char *name) : FSVolume(name, 2*880, 512)
{
    type = OFS;
}

FFSVolume::FFSVolume(const char *name) : FSVolume(name, 2*880, 512)
{
    type = FFS;
}
