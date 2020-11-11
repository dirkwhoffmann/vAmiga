// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

u32
FSBlock::checksum(u8 *p)
{
    assert(p != nullptr);
    
    u32 result = 0;

    for (int i = 0; i < 512; i += 4, p += 4) {
        result += HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
    }
    
    return ~result + 1;
}

u32
FSBlock::read32(u8 *p)
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

u32
FSBlock::bsize()
{
    return volume.bsize;
}

char *
FSBlock::assemblePath()
{
    FSBlock *parent = getParent() ? volume.block(getParent()) : nullptr;
    if (!parent) return strdup("");
    
    char *prefix = parent->assemblePath();
    char *result = new char [strlen(prefix) + strlen(getName()) + 2];

    strcpy(result, prefix);
    strcat(result, "/");
    strcat(result, getName());

    delete [] prefix;
    return result;
}

void
FSBlock::printName()
{
    printf("%s", getName());
}

void
FSBlock::printPath()
{
    char *path = assemblePath();
    printf("%s", path);
    delete [] path;
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
                fsBlockTypeName(type),
                fsBlockTypeName(type1));
    }
    
    if (verbose && type1 != type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s or %s.\n",
                ref,
                fsBlockTypeName(type),
                fsBlockTypeName(type1),
                fsBlockTypeName(type2));
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
FSBlock::importBlock(u8 *p, size_t bsize)
{
    assert(bsize == volume.bsize);
}

void
FSBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(bsize == volume.bsize);
    
    if (!data) {
        assert(type() == FS_EMPTY_BLOCK);
        
        memset(p, 0, bsize);
        
        // Write header
        /*
        p[0] = 'D';
        p[1] = 'O';
        p[2] = 'S';
        p[3] = volume.isOFS() ? 0 : 1;
        */
    
    } else {
        
        printf("Exporting block %d (%zu bytes) (generic code)\n", nr, bsize);
        memcpy(p, data, bsize);
    }
}

FSBlock *
FSBlock::getNextHashBlock()
{
    return getNextHashRef() ? volume.block(getNextHashRef()) : nullptr;
}

FSBlock *
FSBlock::lookup(FSName name)
{
    // Don't call this function if no hash table is present
    assert(hashTableSize() != 0);

    // Compute hash value and table position
    u32 hash = name.hashValue();
    u8 *tableEntry = data + 24 + 4 * hash;
    
    // Read the entry
    u32 blockRef = read32(tableEntry);
    FSBlock *block = blockRef ? volume.block(blockRef) : nullptr;
    
    // Traverse the linked list until the item has been found
    for (int i = 0; block && i < searchLimit; i++) {

        if (block->matches(name)) return block;
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

void
FSBlock::dumpHashTable()
{
    for (int i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            printf("%d: %d ", i, value);
        }
    }
    printf("\n");
}
