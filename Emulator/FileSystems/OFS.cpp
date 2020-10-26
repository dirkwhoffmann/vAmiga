// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "OFS.h"
#include "Utils.h"

OFS::OFS(const char *name, long cap) : capacity(cap)
{
    setDescription("OFS");
    
    // Initialize block storage
    blocks = new BlockPtr[cap]();
    
    // Add a root block and a bitmap block
    addBlock(881, new BitmapBlock(capacity));
    addBlock(880, new RootBlock(name));
    
    debug("File system created (OFS)\n");
}

OFS::~OFS()
{
    delete [] blocks;
}

void
OFS::dump()
{
    debug("Block list:\n");
    
    for (size_t i = 0; i < capacity; i++)  {
        
        if (blocks[i] == nullptr) continue;
        
        msg("%d: %ld", i, blocks[i]->nr);
        msg(" (%s)\n", fsBlockTypeName(blocks[i]->type()));
                
        blocks[i]->dump(); 
    }
}

RootBlock *
OFS::rootBlock()
{
    assert(blocks[880] != nullptr);
    return (RootBlock *)blocks[880];
}

BitmapBlock *
OFS::bitmapBlock()
{
    assert(blocks[881] != nullptr);    
    return (BitmapBlock *)blocks[881];
}

void
OFS::addBlock(long nr, Block *block)
{
    assert(nr < capacity);
    
    // Remove old block if present
    assert(blocks[nr] == nullptr); 
    removeBlock(nr);
    
    // Add new block
    blocks[nr] = block;
    blocks[nr]->nr = nr;

    // Mark block as used
    bitmapBlock()->alloc(nr);
}

void
OFS::removeBlock(long nr)
{
    assert(nr < capacity);
    
    if (blocks[nr] == nullptr) return;
    
    // Delete block
    delete blocks[nr];
    blocks[nr] = nullptr;
    
    // Mark block as free
    bitmapBlock()->dealloc(nr);
}

long
OFS::freeBlock()
{
    for (long i = 882; i < capacity; i++) {

        if (blocks[i] == nullptr) {
            assert(!bitmapBlock()->isAllocated(i));
            return i;
        }
    }

    for (long i = 879; i > 1; i--) {

        if (blocks[i] == nullptr) {
            assert(!bitmapBlock()->isAllocated(i));
            return i;
        }
    }
    
    return -1;
}

void
OFS::installBootBlock(bool ffs)
{
    addBlock(0, new BootBlock(ffs));
}

bool
OFS::addTopLevelDir(const char *name)
{
    assert(name != nullptr);
    
    // Get a free block number
    long nr = freeBlock();
    if (nr == -1) return false;

    // Create block
    UserDirBlock *block = new UserDirBlock(name);
    debug("block = %p nr = %d\n", block, nr);
    
    // Add block at the free location
    addBlock(nr, block);
    
    // Link the new block with the root block
    block->parent = rootBlock();
    rootBlock()->hashTable.link(block);
    return true;
}

bool
OFS::addSubDir(const char *name, UserDirBlock *dir)
{
    assert(name != nullptr);
    
    // Get a free block number
    long nr = freeBlock();
    if (nr == -1) return false;

    // Create block
    UserDirBlock *block = new UserDirBlock(name);
    debug("block = %p nr = %d\n", block, nr);
    
    // Add block at the free location
    addBlock(nr, block);
    
    // Link the new block with the root block
    block->parent = dir;
    dir->hashTable.link(block);
    return true;
}

/*
UserDirBlock *
OFS::seekDirectory(FSHashTable &hashTable, const char *name)
{
    
}
 
 currentDir = nullptr (top level)
 
 changeDir(const char *dir) Cases: 'foo' '/' '..'
 
 */

void
OFS::writeAsDisk(u8 *dst, size_t length)
{
    assert(dst != nullptr);
    assert(length % 512 == 0);
 
    size_t sectorCnt = length / 512;
    assert(sectorCnt <= 2 * 84 * 11);

    debug("writeAsDisk(%x, %d) sectors: %d\n", dst, length, sectorCnt);
    dump();
        
    for (long i = 0; i < capacity; i++) {

        u8 *sector = dst + i * 512;
        
        // Only proceed if the disk has space for this block
        if (i >= sectorCnt) {
            printf("Skipping block %ld (>= %zu)\n", i, sectorCnt);
            continue;
        }

        // If the sector is unused, wipe it out
        if (blocks[i] == nullptr) {
            memset(sector, 0, 512);
            continue;
        }
        
        
        printf("Exporting block %ld\n", i);
        assert(blocks[i]->nr == i);
        blocks[i]->write(sector);
    }
    
    debug("writeAsDisk() DONE\n");
}
