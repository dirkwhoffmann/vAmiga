// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BLOCKS_H
#define _FS_BLOCKS_H

#include "FSObjects.h"

struct FSBlock : AmigaObject {
        
    // The partition this block belongs to
    struct FSPartition &partition;
    
    // The sector number of this block
    u32 nr;
    
    // Outcome of the last integrity check (0 = OK, n = n-th corrupted block)
    u32 corrupted = 0;
        
    // The actual block data
    u8 *data = nullptr;

    
    //
    // Constructing
    //
    
    FSBlock(FSPartition &p, u32 nr) : partition(p) { this->nr = nr; }
    virtual ~FSBlock() { }

    static FSBlock *makeWithType(FSPartition &p, u32 nr, FSBlockType type);
    
    
    //
    // Querying block properties
    //

    // Returns the type of this block
    virtual FSBlockType type() = 0; 

    // Returns the size of this block in bytes (usually 512)
    u32 bsize();

    // Extract the file system type from the block header
    virtual FSVolumeType dos() { return FS_NODOS; }
    
    // Returns the role of a certain byte in this block
    virtual FSItemType itemType(u32 byte) { return FSI_UNKNOWN; }
    
    // Returns the type and subtype identifiers of this block
    virtual u32 typeID();
    virtual u32 subtypeID();
    
    
    //
    // Integrity checking
    //

    // Scans all long words in this block and returns the number of errors
    unsigned check(bool strict);

    // Checks the integrity of a certain byte in this block
    virtual FSError check(u32 pos, u8 *expected, bool strict) { return FS_OK; }
        
    
    //
    // Reading and writing block data
    //

    // Reads or writes a long word in Big Endian format
    static u32 read32(const u8 *p);
    static void write32(u8 *p, u32 value);
    static void inc32(u8 *p) { write32(p, read32(p) + 1); }
    static void dec32(u8 *p) { write32(p, read32(p) - 1); }

    // Computes the address of a long word inside the block
    u8 *addr32(int nr); 
    
    // Reads, writes, or modifies the n-th long word
    u32 get32(i32 n) { return read32(addr32(n)); }
    void set32(i32 n, u32 val) { write32(addr32(n), val); }
    void inc32(i32 n) { inc32(addr32(n)); }
    void dec32(i32 n) { dec32(addr32(n)); }

    // Returns the location of the checksum inside this block
    virtual u32 checksumLocation() { return (u32)-1; }
    
    // Computes a checksum for this block
    virtual u32 checksum();
    
    // Updates the checksum in this block
    void updateChecksum();
    
    
    //
    // Debugging
    //
    
    // Prints some debug information for this block
    virtual void dump() { };
    virtual void dumpData();

    
    //
    // Importing and exporting
    //
    
public:
    
    // Imports this block from a buffer (bsize must match the volume block size)
    virtual void importBlock(const u8 *src, size_t bsize);

    // Exports this block to a buffer (bsize must match the volume block size)
    virtual void exportBlock(u8 *dst, size_t bsize);
    
    // Exports this block to the host file system
    virtual FSError exportBlock(const char *path) { return FS_OK; }
        
                
    //
    // Geting and setting names and comments
    //
    
public:
    
    virtual FSName getName() { return FSName(""); }
    virtual void setName(FSName name) { }
    virtual bool isNamed(FSName &other) { return false; }

    virtual FSComment getComment() { return FSComment(""); }
    virtual void setComment(FSComment name) { }

    
    //
    // Getting and settting date and time
    //
    
    virtual FSTime getCreationDate() { return FSTime((time_t)0); }
    virtual void setCreationDate(FSTime t) { }

    virtual FSTime getModificationDate() { return FSTime((time_t)0); }
    virtual void setModificationDate(FSTime t) { }
    
    
    //
    // Getting and setting file properties
    //
    
    virtual u32 getProtectionBits() { return 0; }
    virtual void setProtectionBits(u32 val) { }

    virtual u32 getFileSize() { return 0; }
    virtual void setFileSize(u32 val) { }

    
    //
    // Chaining blocks
    //

    // Link to the parent directory block
    virtual u32 getParentDirRef() { return 0; }
    virtual void setParentDirRef(u32 ref) { }
    struct FSBlock *getParentDirBlock();
    
    // Link to the file header block
    virtual u32 getFileHeaderRef() { return 0; }
    virtual void setFileHeaderRef(u32 ref) { }
    struct FSFileHeaderBlock *getFileHeaderBlock();

    // Link to the next block with the same hash
    virtual u32 getNextHashRef() { return 0; }
    virtual void setNextHashRef(u32 ref) { }
    struct FSBlock *getNextHashBlock();

    // Link to the next extension block
    virtual u32 getNextListBlockRef() { return 0; }
    virtual void setNextListBlockRef(u32 ref) { }
    struct FSFileListBlock *getNextListBlock();

    // Link to the next bitmap extension block
    virtual u32 getNextBmExtBlockRef() { return 0; }
    virtual void setNextBmExtBlockRef(u32 ref) { }
    struct FSBitmapExtBlock *getNextBmExtBlock();
    
    // Link to the first data block
    virtual u32 getFirstDataBlockRef() { return 0; }
    virtual void setFirstDataBlockRef(u32 ref) { }
    struct FSDataBlock *getFirstDataBlock();

    // Link to the next data block
    virtual u32 getNextDataBlockRef() { return 0; }
    virtual void setNextDataBlockRef(u32 ref) { }
    struct FSDataBlock *getNextDataBlock();

        
    //
    // Working with hash tables
    //
    
    // Returns the hash table size
    virtual u32 hashTableSize() { return 0; }

    // Returns a hash value for this block
    virtual u32 hashValue() { return 0; }

    // Looks up an item in the hash table
    u32 getHashRef(u32 nr);
    void setHashRef(u32 nr, u32 ref);

    // Dumps the contents of the hash table for debugging
    void dumpHashTable();


    //
    // Working with bitmap blocks
    //

    
    
    //
    // Working with data blocks
    //
    
    // Returns the maximum number of storable data block references
    u32 getMaxDataBlockRefs();

    // Gets or sets the number of data block references in this block
    virtual u32 getNumDataBlockRefs() { return 0; }
    virtual void setNumDataBlockRefs(u32 val) { }
    virtual void incNumDataBlockRefs() { }

    // Adds a data block reference to this block
    virtual bool addDataBlockRef(u32 first, u32 ref) { return false; }

    // Adds data bytes to this block
    virtual size_t addData(const u8 *buffer, size_t size) { return 0; }
};

typedef FSBlock* BlockPtr;


//
// Convenience macros used inside the check() methods
//

#define EXPECT_BYTE(exp) { \
if (value != (exp)) { *expected = (exp); return FS_EXPECTED_VALUE; } }

#define EXPECT_LONGWORD(exp) { \
if ((byte % 4) == 0 && BYTE3(value) != BYTE3((u32)exp)) \
    { *expected = (BYTE3((u32)exp)); return FS_EXPECTED_VALUE; } \
if ((byte % 4) == 1 && BYTE2(value) != BYTE2((u32)exp)) \
    { *expected = (BYTE2((u32)exp)); return FS_EXPECTED_VALUE; } \
if ((byte % 4) == 2 && BYTE1(value) != BYTE1((u32)exp)) \
    { *expected = (BYTE1((u32)exp)); return FS_EXPECTED_VALUE; } \
if ((byte % 4) == 3 && BYTE0(value) != BYTE0((u32)exp)) \
    { *expected = (BYTE0((u32)exp)); return FS_EXPECTED_VALUE; } }

#define EXPECT_CHECKSUM EXPECT_LONGWORD(checksum())

#define EXPECT_LESS_OR_EQUAL(exp) { \
if (value > exp) \
{ *expected = (u8)(exp); return FS_EXPECTED_SMALLER_VALUE; } }

#define EXPECT_DOS_REVISION { \
if (!isFSVolumeType(value)) return FS_EXPECTED_DOS_REVISION; }

#define EXPECT_REF { \
if (!partition.dev.block(value)) return FS_EXPECTED_REF; }

#define EXPECT_SELFREF { \
if (value != nr) return FS_EXPECTED_SELFREF; }

#define EXPECT_FILEHEADER_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_FILEHEADER_BLOCK); e != FS_OK) return e; }

#define EXPECT_HASH_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_FILEHEADER_BLOCK, FS_USERDIR_BLOCK); e != FS_OK) return e; }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_ROOT_BLOCK, FS_USERDIR_BLOCK); e != FS_OK) return e; }

#define EXPECT_FILELIST_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_FILELIST_BLOCK); e != FS_OK) return e; }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_BITMAP_BLOCK); e != FS_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_REF { \
if (value) { EXPECT_BITMAP_REF } }

#define EXPECT_BITMAP_EXT_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_BITMAP_EXT_BLOCK); e != FS_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (FSError e = partition.dev.checkBlockType(value, FS_DATA_BLOCK_OFS, FS_DATA_BLOCK_FFS); e != FS_OK) return e; }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return FS_EXPECTED_DATABLOCK_NR; }

#define EXPECT_HASHTABLE_SIZE { \
if (value != 72) return FS_INVALID_HASHTABLE_SIZE; }

#endif
