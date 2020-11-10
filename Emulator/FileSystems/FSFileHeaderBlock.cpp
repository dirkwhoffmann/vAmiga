// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr) : FSFileBlock(ref, nr)
{
    data = new u8[ref.bsize]();
}

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name) :
FSFileHeaderBlock(ref, nr)
{
    this->name = FSName(name);
}

void
FSFileHeaderBlock::dump()
{
    printf("        Name: "); printName(); printf("\n");
    printf("        Path: "); printPath(); printf("\n");
    printf("     Comment: "); comment.dump(); printf("\n");
    printf("     Created: "); created.dump(); printf("\n");
    printf("        Next: %d\n", next);
    printf("   File size: %d\n", fileSize);

    FSFileBlock::dump();
}

bool
FSFileHeaderBlock::check(bool verbose)
{
    bool result = FSFileBlock::check(verbose);
    return result;
}

void
FSFileHeaderBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);

    // Type
    write32(p, 2);
        
    // Block pointer to itself
    write32(p + 4, nr);

    // Number of data block references
    write32(p + 8, numDataBlocks);

    // First data block
    write32(p + 16, firstDataBlock);
    
    // Data block list
    u8 *end = p + bsize - 51 * 4;
    for (int i = 0; i < numDataBlocks; i++) write32(end - 4 * i, dataBlocks[i]);

    // Protection status bits
    write32(p + bsize - 48 * 4, protection);
    
    // File size
    write32(p + bsize - 47 * 4, fileSize);
    
    // Comment as BCPL string
    comment.write(p + bsize - 46 * 4);
    
    // Creation date
    created.write(p + bsize - 23 * 4);
    
    // Name as BCPL string
    name.write(p + bsize - 20 * 4);
    
    // Next block with same hash
    write32(p + bsize - 4 * 4, next);

    // Block pointer to parent directory
    write32(p + bsize - 3 * 4, parent);

    // Block pointer to first extension block
    write32(p + bsize - 2 * 4, nextTableBlock);

    // Subtype
    write32(p + bsize - 1 * 4, (u32)-3);
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

void
FSFileHeaderBlock::setNext(u32 ref)
{
    if (!volume.isBlockNumber(ref)) return;
    
    if (next) {
        volume.block(next)->setNext(ref);
    } else {
        next = ref;
    }
}

bool
FSFileHeaderBlock::append(const u8 *buffer, size_t size)
{
    // If a data block exists, append the buffer there
    FSDataBlock *block = volume.dataBlock(firstDataBlock);
    if (block) return block->append(buffer, size);
    
    // Otherwise, create the first data block
    block = addDataBlock();
    if (block == nullptr) return false;

    // Connect the new block
    firstDataBlock = block->nr;
    block->fileHeaderBlock = nr;
    
    // Append data
    return block->append(buffer, size);
}

bool
FSFileHeaderBlock::append(const char *string)
{
    return append((u8 *)string, strlen(string));
}
