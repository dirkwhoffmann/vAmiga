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

FSVolume *
FSVolume::make(FSVolumeType type, const char *name, const char *path, u32 capacity)
{
    FSVolume *volume = new FSVolume(type, name, capacity);

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

FSVolume::FSVolume(FSVolumeType t, const char *name, u32 c, u32 s) :  type(t), capacity(c), bsize(s)
{
    assert(capacity == 2 * 880 || capacity == 4 * 880);

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
}

FSVolume::~FSVolume()
{
    delete [] blocks;
}

void
FSVolume::info()
{
    msg("Type   Size          Used   Free   Full   Name\n");
    msg("DOS%d  ", type == OFS ? 0 : 1);
    msg("%5d (x %3d) ", totalBlocks(), bsize);
    msg("%5d  ", usedBlocks());
    msg("%5d   ", freeBlocks());
    msg("%3d%%   ", (int)(100.0 * usedBlocks() / freeBlocks()));
    msg("%s\n", getName());
}

void
FSVolume::dump()
{
    msg("Volume: (%s)\n", type == OFS ? "OFS" : "FFS");
    
    for (size_t i = 0; i < capacity; i++)  {
        
        if (blocks[i]->type() == FS_EMPTY_BLOCK) continue;
        
        msg("\nBlock %d (%d):", i, blocks[i]->nr);
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
    
    if (verbose) {
        fprintf(stderr, "The volume is %s.\n", result ? "OK" : "corrupted");
    }
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
    for (long i = start; i >= 2 && i < capacity; i += incr) {

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
        bitmapBlock()->dealloc(ref);
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

void
FSVolume::installBootBlock()
{
    delete blocks[0];
    delete blocks[1];

    blocks[0] = new FSBootBlock(*this, 0);
    blocks[1] = new FSBootBlock(*this, 1);
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
        currentDir = cdb->getParent();
        return currentDirBlock();
    }
    
    FSBlock *subdir = cdb->seek(name);
    if (!subdir) return cdb;
    
    // Move one level down
    currentDir = subdir->nr;
    return currentDirBlock();
}

FSBlock *
FSVolume::makeDir(const char *name)
{
    FSBlock *cdb = currentDirBlock();
    FSUserDirBlock *block = newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParent(cdb->nr);
    return cdb->addHashBlock(block) ? block : nullptr;
}

FSBlock *
FSVolume::makeFile(const char *name)
{
    FSBlock *cdb = currentDirBlock();
    FSFileHeaderBlock *block = newFileHeaderBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParent(cdb->nr);
    return cdb->addHashBlock(block) ? block : nullptr;
}

FSBlock *
FSVolume::seek(const char *name)
{
    FSBlock *cdb = currentDirBlock();
    return cdb->seek(name);
}

FSBlock *
FSVolume::seekDir(const char *name)
{
    FSBlock *block = seek(name);

    if (!block || block->type() != FS_USERDIR_BLOCK) return nullptr;
    return block;
}

FSBlock *
FSVolume::seekFile(const char *name)
{
    FSBlock *block = seek(name);

    if (!block || block->type() != FS_FILEHEADER_BLOCK) return nullptr;
    return block;
}

int
FSVolume::walk(bool recursive)
{
    return walk(currentDirBlock(), &FSVolume::listWalker, 0, recursive);
}

int
FSVolume::walk(FSBlock *dir, int(FSVolume::*walker)(FSBlock *, int), int value, bool recursive)
{
    assert(dir != nullptr);
        
    FSHashTable *hashTable = dir->getHashTable();
    if (hashTable) {
        
        for (int i = 0; i < hashTable->hashTableSize; i++) {
            
            FSBlock *item = block(hashTable->hashTable[i]);
            while (item) {

                if (item->type() == FS_USERDIR_BLOCK) {

                    value = (this->*walker)(item, value);
                    if (recursive) value = walk(item, walker, value, recursive);
                }
                if (item->type() == FS_FILEHEADER_BLOCK) {

                    value = (this->*walker)(item, value);
                }
                
                item = item->getNext() ? block(item->getNext()) : nullptr;
            }
        }
    }
    return value;
}

int
FSVolume::listWalker(FSBlock *block, int value)
{
    if (block->type() == FS_USERDIR_BLOCK) {
        msg("%6s  ", "(DIR)");
    } else {
        msg("%6d  ", block->getSize());
    }
    
    // Convert the internally stored time diff to an absolute time_t value
    time_t time = block->getCreationDate();
    tm *t = localtime(&time);

    msg("%04d-%02d-%02d  ", 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday);
    msg("%02d:%02d:%02d  ", t->tm_hour, t->tm_min, t->tm_sec);
    // msg("%s ", block->getName());
    block->printPath();
    msg("\n");

    return value + 1;
}

bool
FSVolume::exportVolume(u8 *dst, size_t size)
{
    assert(dst != nullptr);
    assert(size % bsize == 0);

    debug("Exporting file system with %d blocks\n", capacity);

    // Only proceed if the buffer is large enough
    if (size / bsize < capacity) {
        debug("Buffer is too small (%d blocks)\n", bsize < capacity);
        return false;
    }
        
    // Wipe out the target buffer
    memset(dst, 0, size);
    
    // Export all blocks
    for (int i = 0; i < capacity; i++) {
        blocks[i]->exportBlock(dst + i * bsize, bsize);
    }
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
                FSBlock *file = makeFile(item->d_name);
                result &= file ? (file->append(buffer, size)) : false;
                delete(buffer);
            }
        }
        
        delete [] name;
    }

    return result;
}

/*
OFSVolume::OFSVolume(const char *name, u32 capacity) : FSVolume(OFS, name, capacity)
{
}

FFSVolume::FFSVolume(const char *name, u32 capacity) : FSVolume(FFS, name, capacity)
{
}
*/
