// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreObject.h"
#include "FSTypes.h"
#include "FSObjects.h"
#include "Buffer.h"
#include "IOUtils.h"
#include "BootBlockImage.h"
#include "Aliases.h"

namespace vamiga {

using util::Buffer;

struct FSBlock : CoreObject {

    friend class FSDoctor;
    
    // The file system this block belongs to
    class FileSystem *fs = nullptr;

    // The type of this block
    FSBlockType type = FSBlockType::UNKNOWN;

    // The sector number of this block
    Block nr = 0;
    
private:

    // Block data
    u8 *bdata = nullptr;

    
    //
    // Constructing
    //

public:

    FSBlock(const FSBlock&) = delete;             // Copy constructor
    FSBlock& operator=(const FSBlock&) = delete;  // Copy assignment
    FSBlock(FSBlock&&) = delete;                  // Move constructor
    FSBlock& operator=(FSBlock&&) = delete;       // Move assignment

    FSBlock(FileSystem *ref, Block nr, FSBlockType t);
    ~FSBlock();

    void init(FSBlockType t);

    static FSBlock *make(FileSystem *ref, Block nr, FSBlockType type) throws;
    static std::vector<Block> refs(const std::vector<const FSBlock *> blocks);


    //
    // Methods from CoreObject
    //
    
protected:
    
    const char *objectName() const override;
    void _dump(Category category, std::ostream &os) const override;
    
    
    //
    // Querying block properties
    //

public:

    // Informs about the block type
    bool is(FSBlockType type) const;
    bool isRoot() const;
    bool isFile() const;
    bool isDirectory() const;
    bool isRegular() const;
    bool isData() const;

    FSName name() const;
    string cppName() const;
    string absName() const;
    string relName() const;
    string relName(const FSBlock &top) const;

    // Experimental
    string acabsName() const;
    string acrelName() const;

    // Converts the path to a host path
    fs::path sanitizedPath() const;

    // Checks if the path matches a search pattern
    bool matches(const FSPattern &pattern) const;
    
    // Returns the size of this block in bytes (usually 512)
    isize bsize() const;

    // Returns the number of data bytes stored in this block
    isize dsize() const;

    // Returns the role of a certain byte in this block
    FSItemType itemType(isize byte) const;
    
    // Returns the type and subtype identifiers of this block
    u32 typeID() const;
    u32 subtypeID() const;
    
        
    //
    // Reading and writing block data
    //

    // Provides the data of a block
    u8 *data();
    const u8 *data() const;

    // Reads or writes a long word in Big Endian format
    static u32 read32(const u8 *p);
    static void write32(u8 *p, u32 value);
    static void inc32(u8 *p) { write32(p, read32(p) + 1); }
    static void dec32(u8 *p) { write32(p, read32(p) - 1); }

    // Computes the address of a long word inside the block
    const u8 *addr32(isize nr) const;
    u8 *addr32(isize nr);

    // Reads, writes, or modifies the n-th long word
    u32 get32(isize n) const { return read32(addr32(n)); }
    void set32(isize n, u32 val) { write32(addr32(n), val); }
    void inc32(isize n) { inc32(addr32(n)); }
    void dec32(isize n) { dec32(addr32(n)); }

    // Returns the location of the checksum inside this block
    isize checksumLocation() const;
    
    // Computes a checksum for this block
    u32 checksum() const;
    
    // Updates the checksum in this block
    void updateChecksum();
    
private:

    u32 checksumStandard() const;
    u32 checksumBootBlock() const;


    //
    // Printing
    //

public:

    void hexDump(std::ostream &os, const util::DumpOpt &opt);

    // Experimental
    static string rangeString(const std::vector<Block> &vec);


    //
    // Debugging
    //
    
public:
    
    // Prints some debug information for this block
    // void dump(std::ostream &os) const;

    
    //
    // Importing and exporting
    //
    
public:
    
    // Imports this block from a buffer (bsize must match the volume block size)
    void importBlock(const u8 *src, isize bsize);

    // Exports this block to a buffer (bsize must match the volume block size)
    void exportBlock(u8 *dst, isize bsize) const;

    // Exports this block to the host file system
    Fault exportBlock(const fs::path &path) const;

private:
    
    Fault exportUserDirBlock(const fs::path &path) const;
    Fault exportFileHeaderBlock(const fs::path &path) const;


    //
    // Geting and setting names and comments
    //
    
public:
    
    bool hasName() const;
    FSName getName() const;
    void setName(FSName name);
    bool isNamed(const FSName &other) const;

    FSComment getComment() const;
    void setComment(FSComment name);

    
    //
    // Getting and settting date and time
    //
    
    FSTime getCreationDate() const;
    void setCreationDate(FSTime t);

    FSTime getModificationDate() const;
    void setModificationDate(FSTime t);
    
    
    //
    // Getting and setting file properties
    //
    
    u32 getProtectionBits() const;
    void setProtectionBits(u32 val);
    string getProtectionBitString() const;

    u32 getFileSize() const;
    void setFileSize(u32 val);


    //
    // Getting and setting meta information
    //

    bool hasHeaderKey() const;
    u32 getHeaderKey() const;
    void setHeaderKey(u32 val);

    bool hasChecksum() const;
    u32 getChecksum() const;
    void setChecksum(u32 val);


    //
    // Chaining blocks
    //

    // Link to the parent directory block
    Block getParentDirRef() const;
    void setParentDirRef(Block ref);
    struct FSBlock *getParentDirBlock() const;
    
    // Link to the file header block
    Block getFileHeaderRef() const;
    void setFileHeaderRef(Block ref);
    FSBlock *getFileHeaderBlock() const;

    // Link to the next block with the same hash
    Block getNextHashRef() const;
    void setNextHashRef(Block ref);
    struct FSBlock *getNextHashBlock() const;

    // Link to the next extension block
    Block getNextListBlockRef() const;
    void setNextListBlockRef(Block ref);
    FSBlock *getNextListBlock() const;
    
    // Link to the next bitmap extension block
    Block getNextBmExtBlockRef() const;
    void setNextBmExtBlockRef(Block ref);
    FSBlock *getNextBmExtBlock() const;
    
    // Link to the first data block
    Block getFirstDataBlockRef() const;
    void setFirstDataBlockRef(Block ref);
    FSBlock *getFirstDataBlock() const;

    Block getDataBlockRef(isize nr) const;
    void setDataBlockRef(isize nr, Block ref);
    FSBlock *getDataBlock(isize nr) const;

    // Link to the next data block
    Block getNextDataBlockRef() const;
    void setNextDataBlockRef(Block ref);
    FSBlock *getNextDataBlock() const;


    //
    // Working with hash tables
    //

    // Returns true if this block can be stored in a hash list
    bool isHashable() const;

    // Returns the hash table size
    isize hashTableSize() const;
    bool hasHashTable() const { return hashTableSize() != 0; }

    // Returns a hash value for this block
    u32 hashValue() const;

    // Looks up an item in the hash table
    u32 getHashRef(u32 nr) const;
    void setHashRef(u32 nr, u32 ref);

 
    //
    // Working with boot blocks
    //

    void writeBootBlock(BootBlockId id, isize page);
    
    
    //
    // Working with bitmap blocks
    //

    // Adds bitmap block references to the root block or an extension block
    bool addBitmapBlockRefs(std::vector<Block> &refs);
    void addBitmapBlockRefs(std::vector<Block> &refs,
                            std::vector<Block>::iterator &it);
    
    //Gets or sets a link to a bitmap block
    isize numBmBlockRefs() const;
    Block getBmBlockRef(isize nr) const;
    void setBmBlockRef(isize nr, Block ref);
    std::vector<Block> getBmBlockRefs() const;


    //
    // Working with data blocks
    //
    
    // Gets or sets the data block number
    u32 getDataBlockNr() const;
    void setDataBlockNr(u32 val);

    // Returns the maximum number of storable data block references
    isize getMaxDataBlockRefs() const;

    // Gets or sets the number of data block references in this block
    isize getNumDataBlockRefs() const;
    void setNumDataBlockRefs(u32 val);
    void incNumDataBlockRefs();
    std::vector<Block> getDataBlockRefs() const;

    // Adds a data block reference to this block
    bool addDataBlockRef(Block ref);
    void addDataBlockRef(u32 first, u32 ref);
    
    // Gets or sets the number of data bytes stored in this block
    u32 getDataBytesInBlock() const;
    void setDataBytesInBlock(u32 val);

    
    //
    // Exporting
    //
    
    isize writeData(std::ostream &os) const;
    isize writeData(std::ostream &os, isize size) const;
    isize extractData(Buffer<u8> &buf) const;
    isize writeData(Buffer<u8> &buf, isize offset, isize count) const;

    
    //
    // Importing
    //
    
    isize overwriteData(Buffer<u8> &buf);
    isize overwriteData(Buffer<u8> &buf, isize offset, isize count);
};

typedef FSBlock* BlockPtr;


//
// Comparison function used for sorting
//

namespace sort {

inline std::function<bool(const FSBlock &, const FSBlock &)> dafa = [](const FSBlock &b1, const FSBlock &b2) {

    if ( b1.isDirectory() && !b2.isDirectory()) return true;
    if (!b1.isDirectory() &&  b2.isDirectory()) return false;
    return b1.getName() < b2.getName();
};

inline std::function<bool(const FSBlock &, const FSBlock &)> alpha = [](const FSBlock &b1, const FSBlock &b2) {

    return b1.getName() < b2.getName();
};

inline std::function<bool(const FSBlock &, const FSBlock &)> none = nullptr;

}

}
