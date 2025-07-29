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

class MutableFileSystem : public FileSystem {

    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;

    // Allocation pointer (used by the allocator to select the next block)
    Block ap = 0;


    //
    // Initializing
    //

public:

    using FileSystem::FileSystem;

    MutableFileSystem(isize capacity, isize bsize = 512) { init(capacity, bsize); }
    MutableFileSystem(const FSDescriptor &layout, const fs::path &path = {}) { init(layout, path); }
    MutableFileSystem(Diameter dia, Density den, FSFormat dos, const fs::path &path = {}) { init(dia, den, dos, path); }

    using FileSystem::init;
    void init(isize capacity, isize bsize = 512);
    void init(const FSDescriptor &layout, const fs::path &path = {});
    void init(Diameter dia, Density den, FSFormat dos, const fs::path &path = {});


    //
    // Formatting
    //

public:

    // Formats the volume
    void format(string name = "");
    void format(FSFormat dos, string name = "");

    // Assigns the volume name
    void setName(FSName name);
    void setName(string name) { setName(FSName(name)); }


    //
    // Creating and deleting blocks
    //

public:

    // Returns true if at least 'count' free blocks are available
    bool allocatable(isize count) const;

    // Seeks a free block and marks it as allocated
    Block allocate();

    // Allocates multiple blocks
    void allocate(isize count, std::vector<Block> &result);

    // Deallocates a block
    void deallocateBlock(Block nr);

    // Updates the checksums in all blocks
    void updateChecksums() noexcept;

private:

    // Returns the number of required blocks to store a file of certain size
    isize requiredDataBlocks(isize fileSize) const;
    isize requiredFileListBlocks(isize fileSize) const;
    isize requiredBlocks(isize fileSize) const;

    // Adds a new block of a certain kind
    void addFileListBlock(Block at, Block head, Block prev);
    void addDataBlock(Block at, isize id, Block head, Block prev);

    // Creates a new block of a certain kind
    FSBlock *newUserDirBlock(const FSName &name);
    FSBlock *newFileHeaderBlock(const FSName &name);


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

    // Rectifies the block allocation map
    void rectifyAllocationMap();


    //
    // Managing directories and files
    //

public:

    // Creates a new directory
    FSBlock &createDir(FSBlock &at, const FSName &name);

    // Creates a new file
    FSBlock &createFile(FSBlock &at, const FSName &name);
    FSBlock &createFile(FSBlock &at, const FSName &name, const Buffer<u8> &buf);
    FSBlock &createFile(FSBlock &at, const FSName &name, const u8 *buf, isize size);
    FSBlock &createFile(FSBlock &at, const FSName &name, const string &str);

    // Renames a file or directory
    void rename(FSBlock &item, const FSName &name);

    // Moves a file or directory to another location
    void move(FSBlock &item, const FSBlock &dest, const FSName &name = "");

    // Copies a file
    void copy(const FSBlock &item, FSBlock &dest);
    void copy(const FSBlock &item, FSBlock &dest, const FSName &name);

    // Delete a file
    void deleteFile(const FSBlock &at);

private:

    // Adds a hash-table entry for a given item
    void addToHashTable(const FSBlock &item);
    void addToHashTable(Block parent, Block ref);

    // Removes the hash-table entry for a given item
    void deleteFromHashTable(const FSBlock &item);
    void deleteFromHashTable(Block parent, Block ref);

    // Adds bytes to a data block
    isize addData(Block nr, const u8 *buf, isize size);
    isize addData(FSBlock &block, const u8 *buf, isize size);

    // Allocates all blocks needed for a file
    void allocateFileBlocks(isize bytes, std::vector<Block> &listBlocks, std::vector<Block> &dataBlocks);


    //
    // Importing and exporting the volume
    //

public:

    // Imports the volume from a buffer compatible with the ADF or HDF format
    void importVolume(const u8 *src, isize size) throws;

    // Imports files and folders from the host file system
    void import(const fs::path &path, bool recursive = true, bool contents = false) throws;
    void import(FSBlock &top, const fs::path &path, bool recursive = true, bool contents = false) throws;

    // Imports a single block
    void importBlock(Block nr, const fs::path &path);

    // Exports the volume to a buffer
    bool exportVolume(u8 *dst, isize size) const;
    bool exportVolume(u8 *dst, isize size, Fault *error) const;

    // Exports a single block or a range of blocks to a buffer
    bool exportBlock(Block nr, u8 *dst, isize size) const;
    bool exportBlock(Block nr, u8 *dst, isize size, Fault *error) const;
    bool exportBlocks(Block first, Block last, u8 *dst, isize size) const;
    bool exportBlocks(Block first, Block last, u8 *dst, isize size, Fault *error) const;

    // Exports a single block or a range of blocks to a file
    void exportBlock(Block nr, const fs::path &path) const;
    void exportBlocks(Block first, Block last, const fs::path &path) const;
    void exportBlocks(const fs::path &path) const;

    // Exports the volume to a buffer
    void exportFiles(Block nr, const fs::path &path, bool recursive = true, bool contents = false) const;
    void exportFiles(const FSBlock &top, const fs::path &path, bool recursive = true, bool contents = false) const;
    void exportFiles(const fs::path &path, bool recursive = true, bool contents = false) const;

private:

    void import(FSBlock &top, const fs::directory_entry &dir, bool recursive) throws;
};

}
