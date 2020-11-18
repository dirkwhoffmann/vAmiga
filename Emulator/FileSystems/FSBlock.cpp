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

FSBlock *
FSBlock::makeWithType(FSVolume &ref, u32 nr, FSBlockType type)
{
    switch (type) {

        case FS_EMPTY_BLOCK: return new FSEmptyBlock(ref, nr);
        case FS_BOOT_BLOCK: return new FSBootBlock(ref, nr);
        case FS_ROOT_BLOCK: return new FSRootBlock(ref, nr);
        case FS_BITMAP_BLOCK: return new FSBitmapBlock(ref, nr);
        case FS_USERDIR_BLOCK: return new FSUserDirBlock(ref, nr);
        case FS_FILEHEADER_BLOCK: return new FSFileHeaderBlock(ref, nr);
        case FS_FILELIST_BLOCK: return new FSFileListBlock(ref, nr);
        case FS_DATA_BLOCK: return new FSDataBlock(ref, nr);

        default: return nullptr;
    }
}

u32
FSBlock::typeID()
{
    return get32(0);
}

u32
FSBlock::subtypeID()
{
    return get32((volume.bsize / 4) - 1);
}

bool
FSBlock::check(long *numErrors)
{
    long errors = 0;
    
    for (u32 i = 0; i < volume.bsize / 4; i += 4) {
        if (check(i) != FS_OK) errors++;
    }
    
    if (numErrors) *numErrors = errors;
    return errors == 0;
}

u8 *
FSBlock::addr(int nr)
{
    return (data + 4 * nr) + (nr < 0 ? volume.bsize : 0);
}

u32
FSBlock::read32(const u8 *p)
{
    return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}
    
void
FSBlock::write32(u8 *p, u32 value)
{
    p[0] = (value >> 24) & 0xFF;
    p[1] = (value >> 16) & 0xFF;
    p[2] = (value >>  8) & 0xFF;
    p[3] = (value >>  0) & 0xFF;
}

char *
FSBlock::assemblePath()
{
    FSBlock *parent = getParentBlock();
    if (!parent) return strdup("");
    
    FSName name = getName();
    
    char *prefix = parent->assemblePath();
    char *result = new char [strlen(prefix) + strlen(name.cStr) + 2];

    strcpy(result, prefix);
    strcat(result, "/");
    strcat(result, name.cStr);

    delete [] prefix;
    return result;
}

void
FSBlock::printPath()
{
    char *path = assemblePath();
    printf("%s", path);
    delete [] path;
}

u32
FSBlock::checksum()
{
    // TODO: Skip fields storing the actual checksum
    u32 result = 0;
    u32 numLongWords = volume.bsize / 4;
    
    for (u32 i = 0; i < numLongWords; i++) {
        result += get32(i);
    }
    
    return ~result + 1;
}

bool
FSBlock::check(bool verbose)
{
    return assertSelfRef(nr, verbose);
}

bool
FSBlock::assertNotNull(u32 ref, bool verbose)
{
    if (ref != 0) return true;
    
    if (verbose) fprintf(stderr, "Block reference is missing.\n");
    return false;
}

bool
FSBlock::assertInRange(u32 ref, bool verbose)
{
    if (volume.isBlockNumber(ref)) return true;

    if (verbose) fprintf(stderr, "Block reference %d is invalid\n", ref);
    return false;
}

bool
FSBlock::assertHasType(u32 ref, FSBlockType type, bool verbose)
{
    return assertHasType(ref, type, type, verbose);
}

bool
FSBlock::assertHasType(u32 ref, FSBlockType type1, FSBlockType type2, bool verbose)
{
    assert(isFSBlockType(type1));
    assert(isFSBlockType(type2));

    FSBlock *block = volume.block(ref);
    FSBlockType type = block ? block->type() : FS_EMPTY_BLOCK;
    
    if (!isFSBlockType(type)) {
        if (verbose) fprintf(stderr, "Block type %ld is not a known type.\n", type);
        return false;
    }
    
    if (block && (type == type1 || type == type2)) return true;
    
    if (verbose && type1 == type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s.\n",
                ref,
                sFSBlockType(type),
                sFSBlockType(type1));
    }
    
    if (verbose && type1 != type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s or %s.\n",
                ref,
                sFSBlockType(type),
                sFSBlockType(type1),
                sFSBlockType(type2));
    }

    return false;
}

bool
FSBlock::assertSelfRef(u32 ref, bool verbose)
{
    if (ref == nr && volume.block(ref) == this) return true;

    if (ref != nr && verbose) {
        fprintf(stderr, "%d is not a self-reference.\n", ref);
    }
    
    if (volume.block(ref) != this && verbose) {
        fprintf(stderr, "Array element %d references an invalid block\n", ref);
    }
    
    return false;
}

void
FSBlock::importBlock(const u8 *src, size_t bsize)
{
    assert(bsize == volume.bsize);
    assert(src != nullptr);
    assert(data != nullptr);
    memcpy(data, src, bsize);
}

void
FSBlock::exportBlock(u8 *dst, size_t bsize)
{
    assert(bsize == volume.bsize);
            
    // Rectify the checksum
    updateChecksum();

    // Export the block
    assert(dst != nullptr);
    assert(data != nullptr);
    memcpy(dst, data, bsize);
}

FSBlock *
FSBlock::getParentBlock()
{
    u32 ref = getParentDirRef();
    return ref ? volume.block(ref) : nullptr;
}

FSFileHeaderBlock *
FSBlock::getFileHeaderBlock()
{
    u32 ref = getFileHeaderRef();
    return ref ? volume.fileHeaderBlock(ref) : nullptr;
}

FSDataBlock *
FSBlock::getFirstDataBlock()
{
    u32 ref = getFirstDataBlockRef();
    return ref ? volume.dataBlock(ref) : nullptr;
}

FSDataBlock *
FSBlock::getNextDataBlock()
{
    u32 ref = getNextDataBlockRef();
    return ref ? volume.dataBlock(ref) : nullptr;
}

FSBlock *
FSBlock::getNextHashBlock()
{
    u32 ref = getNextHashRef();
    return ref ? volume.block(ref) : nullptr;
}

FSFileListBlock *
FSBlock::getNextExtensionBlock()
{
    u32 ref = getNextListBlockRef();
    return ref ? volume.fileListBlock(ref) : nullptr;
}

u32
FSBlock::hashLookup(u32 nr)
{
    return (nr < hashTableSize()) ? get32(6 + nr) : 0;
}

FSBlock *
FSBlock::hashLookup(FSName name)
{
    // Don't call this function if no hash table is present
    assert(hashTableSize() != 0);

    // Compute hash value and table position
    u32 hash = name.hashValue() % hashTableSize();
    
    // Read the entry
    u32 blockRef = hashLookup(hash);
    FSBlock *block = blockRef ? volume.block(blockRef) : nullptr;
    
    // Traverse the linked list until the item has been found
    for (int i = 0; block && i < searchLimit; i++) {

        if (block->isNamed(name)) return block;
        block = block->getNextHashBlock();
    }

    return nullptr;
}

void
FSBlock::addToHashTable(u32 ref)
{
    FSBlock *block = volume.block(ref);
    if (block == nullptr) return;
    
    // Don't call this function if no hash table is present
    assert(hashTableSize() != 0);
        
    // Compute hash value and table position
    u32 hash = block->hashValue() % hashTableSize();
    u8 *tableEntry = data + 24 + 4 * hash;
    
    // If the hash table slot is empty, put the reference there
    if (read32(tableEntry) == 0) { write32(tableEntry, ref); return; }
    
    // Otherwise, add the reference at the end of the linked list
    if (auto item = volume.block(read32(tableEntry))) {
        
        for (int i = 0; i < searchLimit; i++) {
            
            if (item->getNextHashBlock() == nullptr) {
                item->setNextHashRef(ref);
                return;
            }
            
            item = item->getNextHashBlock();
        }
    }
}

bool
FSBlock::checkHashTable(bool verbose)
{
    bool result = true;
    
    for (u32 i = 0; i < hashTableSize(); i++) {
        
        if (u32 ref = read32(data + 24 + 4 * i)) {
            result &= assertInRange(ref, verbose);
            result &= assertHasType(ref, FS_USERDIR_BLOCK, FS_FILEHEADER_BLOCK, verbose);
        }
    }
    return result;
}

FSError
FSBlock::checkHashTableItem(u32 item)
{
    if (u32 ref = get32(6 + item)) {
    
        if (!volume.block(ref)) {
            return FS_BLOCK_REF_OUT_OF_RANGE;
        }
        if (!volume.fileHeaderBlock(ref) && !volume.userDirBlock(ref)) {
            return FS_BLOCK_REF_TYPE_MISMATCH;
        }
    }
    
    return FS_OK;
}

void
FSBlock::dumpHashTable()
{
    for (u32 i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            printf("%d: %d ", i, value);
        }
    }
}

u32
FSBlock::getMaxDataBlockRefs()
{
    return volume.bsize / 4 - 56;
}
