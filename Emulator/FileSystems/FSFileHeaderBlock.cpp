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
    
    setCreationDate(time(NULL));
}

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name) :
FSFileHeaderBlock(ref, nr)
{
    this->name = FSName(name);
    setName(FSName(name));
}

void
FSFileHeaderBlock::dump()
{
    printf("  Name (old): "); printName(); printf("\n");
    printf("        Name: %s\n", getName().name);
    printf("        Path: "); printPath(); printf("\n");
    printf("     Comment: %s\n", getComment().name);
    printf("     Created: "); dumpDate(getCreationDate()); printf("\n");
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

    memcpy(p, data, bsize);

    // Type
    write32(p, 2);
        
    // Block pointer to itself
    write32(p + 4, nr);

    // Number of data block references
    // write32(p + 8, numDataBlockRefs());

    // First data block
    write32(p + 16, firstDataBlock);
    
    // Data block list
    u8 *end = p + bsize - 51 * 4;
    for (int i = 0; i < numDataBlockRefs(); i++) write32(end - 4 * i, dataBlocks[i]);

    // Protection status bits
    write32(p + bsize - 48 * 4, protection);
    
    // File size
    write32(p + bsize - 47 * 4, fileSize);
        
    // Name as BCPL string
    name.write(p + bsize - 20 * 4);
    
    // Next block with same hash
    write32(p + bsize - 4 * 4, next);

    // Block pointer to parent directory
    write32(p + bsize - 3 * 4, parent);

    // Block pointer to first extension block
    // write32(p + bsize - 2 * 4, nextTableBlock);

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

FSName
FSFileHeaderBlock::getName()
{
    return FSName(data + bsize() - 20 * 4);
}

void
FSFileHeaderBlock::setName(FSName name)
{
    // name.write(data + bsize() - 20 * 4);
}

FSName
FSFileHeaderBlock::getComment()
{
    return FSName(data + bsize() - 46 * 4);
}

void
FSFileHeaderBlock::setComment(FSName name)
{
    name.write(data + bsize() - 46 * 4);
}

time_t
FSFileHeaderBlock::getCreationDate()
{
    return readTimeStamp(data + bsize() - 23 * 4);
}

void
FSFileHeaderBlock::setCreationDate(time_t t)
{
    writeTimeStamp(data + bsize() - 23 * 4, t);
}

void
FSFileHeaderBlock::setNextDataBlockRef(u32 ref)
{
    firstDataBlock = ref;
}

size_t
FSFileHeaderBlock::addData(const u8 *buffer, size_t size)
{
    printf("addData(%p,%zu)\n", buffer, size);

    assert(fileSize == 0);
    
    // Compute the required number of DataBlocks
    u32 bytes = volume.bytesInDataBlock();
    u32 numDataBlocks = (size + bytes - 1) / bytes;

    // Compute the required number of FileListBlocks
    u32 numDataListBlocks = 0;
    if (numDataBlocks > maxDataBlockRefs()) {
        numDataListBlocks = 1 + (numDataBlocks - maxDataBlockRefs()) / maxDataBlockRefs();
    }

    printf("Required DataBlocks: %d\n", numDataBlocks);
    printf("Required DataListBlocks: %d\n", numDataListBlocks);
    
    // TODO: Check if the volume has enough free space
    
    for (u32 ref = nr, i = 0; i < numDataListBlocks; i++) {

        // Add a new file list block
        ref = volume.addFileListBlock(nr, ref);
    }
    
    for (u32 ref = nr, i = 1; i <= numDataBlocks; i++) {

        // Add a new data block
        ref = volume.addDataBlock(i, nr, ref);

        // Add references to the new data block
        addDataBlockRef(ref);
        
        // Add data
        FSBlock *block = volume.block(ref);
        if (block) {
            size_t written = block->addData(buffer, size);
            fileSize += written;
            buffer += written;
            size -= written;
        }
    }

    return fileSize;
}


bool
FSFileHeaderBlock::addDataBlockRef(u32 ref)
{
    return addDataBlockRef(nr, ref);
}

bool
FSFileHeaderBlock::addDataBlockRef(u32 first, u32 ref)
{
    // If this block has space for more references, add it here
    if (numDataBlockRefs() < maxDataBlockRefs()) {

        if (numDataBlockRefs() == 0) setFirstDataBlockRef(ref);
        dataBlocks[numDataBlockRefs()] = ref;
        incDataBlockRefs();
        return true;
    }

    // Otherwise, add it to an extension block
    FSFileListBlock *item = getNextExtensionBlock();
    
    for (int i = 0; item && i < searchLimit; i++) {
        
        if (item->addDataBlockRef(first, ref)) return true;
        item = item->getNextExtensionBlock();
    }
    
    assert(false);
    return false;
}
