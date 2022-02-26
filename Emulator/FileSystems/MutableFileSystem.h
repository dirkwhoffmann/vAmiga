// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystem.h"

class MutableFileSystem : public FileSystem {
    
    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;

    
    //
    // Initializing
    //
    
public:

    MutableFileSystem(isize capacity) { init(capacity); }
    MutableFileSystem(FSDeviceDescriptor &layout) { init(layout); }
    MutableFileSystem(DiskDiameter dia, DiskDensity den) { init(dia, den); }
    MutableFileSystem(DiskDiameter dia, DiskDensity den, const string &path) { init(dia, den, path); }
    MutableFileSystem(const ADFFile &adf) throws { init(adf); }
    MutableFileSystem(const HDFFile &hdf, isize part) throws { init(hdf, part); }
    MutableFileSystem(Drive &drive) throws { init(drive); }
    MutableFileSystem(const HardDrive &drive, isize part) throws { init(drive, part); }
    MutableFileSystem(FSVolumeType type, const string &path) { init(type, path); }
    ~MutableFileSystem();
    
private:
    
    void init(isize capacity);
    void init(FSDeviceDescriptor &layout);
    void init(DiskDiameter type, DiskDensity density);
    void init(DiskDiameter type, DiskDensity density, const string &path);
    void init(const ADFFile &adf) throws;
    void init(const HDFFile &hdf, isize part) throws;
    void init(Drive &drive) throws;
    void init(const HardDrive &drive, isize part) throws;
    void init(FSVolumeType type, const string &path);

    void initBlocks(FSDeviceDescriptor &layout);


    //
    // Modifying the root block
    //

public:

    void setName(FSName name);
    
    
    //
    // Modifying boot blocks
    //
    
public:
    // Installs a boot block
    void makeBootable(BootBlockId id);

    // Removes a boot block virus from the current partition (if any)
    void killVirus();
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Returns the number of required blocks to store a file of certain size
    isize requiredDataBlocks(isize fileSize) const;
    isize requiredFileListBlocks(isize fileSize) const;
    isize requiredBlocks(isize fileSize) const;

    // Seeks a free block and marks it as allocated
    Block allocateBlock();
    Block allocateBlockAbove(Block nr);
    Block allocateBlockBelow(Block nr);

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
    
    // Adds a reference to the current directory
    void addHashRef(Block nr);
    void addHashRef(FSBlock *block);
    
    // Creates a new directory
    FSBlock *createDir(const string &name);

    // Creates a new file
    FSBlock *createFile(const string &name);
    FSBlock *createFile(const string &name, const u8 *buffer, isize size);
    FSBlock *createFile(const string &name, const string &str);
        
private:
    
    // Adds data bytes to a block
    isize addData(FSBlock &block, const u8 *buffer, isize size);
    
    
    //
    // Importing and exporting
    //
    
public:
        
    // Predicts the type of a block by analyzing its number and data
    // FSBlockType predictBlockType(Block nr, const u8 *buffer);

    // Imports the volume from a buffer compatible with the ADF format
    void importVolume(const u8 *src, isize size) throws;

    // Imports a directory from the host file system
    void importDirectory(const string &path, bool recursive = true) throws;
    void importDirectory(const fs::directory_entry &dir, bool recursive) throws;
    
    // Exports the volume to a buffer
    bool exportVolume(u8 *dst, isize size) const;
    bool exportVolume(u8 *dst, isize size, ErrorCode *error) const;

    // Exports a single block or a range of blocks
    bool exportBlock(Block nr, u8 *dst, isize size) const;
    bool exportBlock(Block nr, u8 *dst, isize size, ErrorCode *error) const;
    bool exportBlocks(Block first, Block last, u8 *dst, isize size) const;
    bool exportBlocks(Block first, Block last, u8 *dst, isize size, ErrorCode *error) const;

    // Exports the volume to a directory of the host file system
    void exportDirectory(const string &path, bool createDir = true) throws;
};
