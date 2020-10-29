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
}

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name) :
FSFileHeaderBlock(ref, nr)
{
    this->name = FSName(name);
}

void
FSFileHeaderBlock::printName()
{
    printf("%s", name.name);
}

void
FSFileHeaderBlock::printPath()
{
    FSBlock *ref = volume.block(getParent());
    if (ref) ref->printPath();
    printf("/");
    printName();
}

void
FSFileHeaderBlock::dump()
{
    FSFileBlock::dump();
    
    printf("        Name: "); printName(); printf("\n");
    printf("        Path: "); printPath(); printf("\n");
    printf("     Comment: "); comment.dump(); printf("\n");
    printf("     Created: "); created.dump(); printf("\n");
    printf("   File size: %d\n", fileSize);
}

bool
FSFileHeaderBlock::check(bool verbose)
{
    printf("FSFileHeaderBlock::check(%d)", verbose);

    bool result = FSFileBlock::check(verbose);
    return result;
}

void
FSFileHeaderBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);
    
    // Type
    write32(p, 2);
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Number of blocks in file header
    write32(p + 8, numDataBlocks);

    // First data block
    if (dataBlocks[0]) write32(p + 16, dataBlocks[0]);
    
    // Data block list
    for (int i = 0; i < numDataBlocks; i++) write32(p+308-4*i, dataBlocks[i]);
    
    // Protection status bits
    write32(p + 320, 0);
    
    // File size
    write32(p + 324, fileSize);
    
    // Comment as BCPL string
    comment.write(p + 328);
    
    // Creation date
    created.write(p + 420);
    
    // Directory name as BCPL string
    name.write(p + 432);
    
    // Next block with same hash
    write32(p + 496, nextTableBlock);

    // Block pointer to parent directory
    write32(p + 500, parent);
    
    // Subtype
    write32(p + 508, (u32)-3);
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

bool
FSFileHeaderBlock::append(u8 byte)
{
    FSDataBlock *block = volume.dataBlock(firstDataBlock);
    
    // If a data block already exist, append it there
    if (block) return block->append(byte);
    
    // Create the first data block
    block = volume.newDataBlock();
    if (block == nullptr) return false;
    
    // Link this block
    block->fileHeaderBlock = nr;
    firstDataBlock = block->nr;
    
    // Append data
    return block->append(byte);
}

bool
FSFileHeaderBlock::append(u8 *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (!append(buffer[i])) return false;
    }
    
    return true;
}
