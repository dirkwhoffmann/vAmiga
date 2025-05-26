// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystem.h"

namespace vamiga {

/* The MutableFileSystem class extends the FileSystem class with functions for
 * modifiying the contents of the file system. It provides functions for
 * creating empty file systems of a certain type as well as functions for
 * creating files and directories.
 */
class MutableFileSystem : public FileSystem {
    
    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;

    // Next block to be allocated in allocBlock()
    Block tba = 0;
    
    
    //
    // Initializing
    //
    
public:

    using FileSystem::FileSystem;

    MutableFileSystem(isize capacity) { init(capacity); }
    MutableFileSystem(FileSystemDescriptor &layout, const fs::path &path = {}) { init(layout, path); }
    MutableFileSystem(Diameter dia, Density den, FSVolumeType dos, const fs::path &path = {}) { init(dia, den, dos, path); }

private:
    
    void init(isize capacity);
    void init(FileSystemDescriptor &layout, const fs::path &path);
    void init(Diameter dia, Density den, FSVolumeType dos, const fs::path &path);


    //
    // Formatting
    //

public:

    // Formats the volume
    void format(FSVolumeType dos, string name = "");
    void format(string name = "");
    
    // Assigns the volume name
    void setName(FSName name);
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Returns the number of required blocks to store a file of certain size
    isize requiredDataBlocks(isize fileSize) const;
    isize requiredFileListBlocks(isize fileSize) const;
    isize requiredBlocks(isize fileSize) const;

    // Returns true if at least 'count' free blocks are available
    bool allocatable(isize count) const;
    
    // Seeks a free block and marks it as allocated
    Block allocateBlock();

    // Deallocates a block
    void deallocateBlock(Block nr);

    // Adds a new block of a certain kind
    Block addFileListBlock(Block head, Block prev);
    Block addDataBlock(isize count, Block head, Block prev);
    
    // Creates a new block of a certain kind
    FSBlock *newUserDirBlock(const string &name);
    FSBlock *newFileHeaderBlock(const string &name);
    
    // Updates the checksums in all blocks
    void updateChecksums();
    
    
    //
    // Modifying boot blocks
    //
    
public:
    // Installs a boot block
    void makeBootable(BootBlockId id);

    // Removes a boot block virus from the current partition (if any)
    void killVirus();

    
    //
    // Editing the block allocation bitmap
    //

public:

    // Marks a block as allocated or free
    void markAsAllocated(Block nr) { setAllocationBit(nr, 0); }
    void markAsFree(Block nr) { setAllocationBit(nr, 1); }
    void setAllocationBit(Block nr, bool value);


    //
    // Managing directories and files
    //
    
public:

    // Creates a new directory
    FSBlock *createDir(const string &name);

    // Creates a new file
    FSBlock *createFile(const string &name);
    FSBlock *createFile(const string &name, const u8 *buffer, isize size);
    FSBlock *createFile(const string &name, const string &str);

private:
    
    // Adds a reference to the current directory
    void addHashRef(Block nr);
    void addHashRef(FSBlock *block);

    // Adds data bytes to a block
    isize addData(FSBlock &block, const u8 *buffer, isize size);
    
    
    //
    // Importing and exporting the volume
    //
    
public:

    // Imports the volume from a buffer compatible with the ADF format
    void importVolume(const u8 *src, isize size) throws;

    // Imports a directory from the host file system
    void importDirectory(const fs::path &path, bool recursive = true) throws;
    void importDirectory(const fs::directory_entry &dir, bool recursive) throws;
    
    // Exports the volume to a buffer
    bool exportVolume(u8 *dst, isize size) const;
    bool exportVolume(u8 *dst, isize size, Fault *error) const;

    // Exports a single block or a range of blocks
    bool exportBlock(Block nr, u8 *dst, isize size) const;
    bool exportBlock(Block nr, u8 *dst, isize size, Fault *error) const;
    bool exportBlocks(Block first, Block last, u8 *dst, isize size) const;
    bool exportBlocks(Block first, Block last, u8 *dst, isize size, Fault *error) const;

    // Exports the volume to a directory of the host file system
    void exportDirectory(const fs::path &path, bool createDir = true) const throws;
};

}
