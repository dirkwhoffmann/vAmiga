// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"
#include "FSTypes.h"
#include "FSObjects.h"
#include "BootBlockImage.h"
#include <vector>

struct FSBlock : AmigaObject {
        
    // The partition this block belongs to
    struct FSPartition &partition;

    // The type of this block
    FSBlockType type = FS_UNKNOWN_BLOCK;
        
    // The sector number of this block
    Block nr;
    
    // Outcome of the latest integrity check (0 = OK, n = n-th corrupted block)
    isize corrupted = 0;
        
    // The actual block data
    u8 *data = nullptr;

    
    //
    // Constructing
    //
    
    FSBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSBlock();

    static FSBlock *make(FSPartition &p, Block nr, FSBlockType type) throws;

    
    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override;
    void _dump(dump::Category category, std::ostream& os) const override { }
    
    
    //
    // Querying block properties
    //

public:
    
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
    // Integrity checking
    //

    // Scans all long words in this block and returns the number of errors
    isize check(bool strict) const;

    // Checks the integrity of a certain byte in this block
    ErrorCode check(isize pos, u8 *expected, bool strict) const;
        
    
    //
    // Reading and writing block data
    //

    // Reads or writes a long word in Big Endian format
    static u32 read32(const u8 *p);
    static void write32(u8 *p, u32 value);
    static void inc32(u8 *p) { write32(p, read32(p) + 1); }
    static void dec32(u8 *p) { write32(p, read32(p) - 1); }

    // Computes the address of a long word inside the block
    u8 *addr32(isize nr) const;
    
    // Reads, writes, or modifies the n-th long word
    u32 get32(isize n) const { return read32(addr32(n)); }
    void set32(isize n, u32 val) const { write32(addr32(n), val); }
    void inc32(isize n) const { inc32(addr32(n)); }
    void dec32(isize n) const { dec32(addr32(n)); }

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
    // Debugging
    //
    
public:
    
    // Prints some debug information for this block
    void dump() const;
    void dumpData() const;

    
    //
    // Importing and exporting
    //
    
public:
    
    // Imports this block from a buffer (bsize must match the volume block size)
    void importBlock(const u8 *src, isize bsize);

    // Exports this block to a buffer (bsize must match the volume block size)
    void exportBlock(u8 *dst, isize bsize);
    
    // Exports this block to the host file system
    ErrorCode exportBlock(const string &path);
        
private:
    
    ErrorCode exportUserDirBlock(const string &path);
    ErrorCode exportFileHeaderBlock(const string &path);

                
    //
    // Geting and setting names and comments
    //
    
public:
    
    FSName getName() const;
    void setName(FSName name);
    bool isNamed(FSName &other) const;

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

    u32 getFileSize() const;
    void setFileSize(u32 val);

    
    //
    // Chaining blocks
    //

    // Link to the parent directory block
    Block getParentDirRef() const;
    void setParentDirRef(Block ref);
    struct FSBlock *getParentDirBlock();
    
    // Link to the file header block
    Block getFileHeaderRef() const;
    void setFileHeaderRef(Block ref);
    FSBlock *getFileHeaderBlock();

    // Link to the next block with the same hash
    Block getNextHashRef() const;
    void setNextHashRef(Block ref);
    struct FSBlock *getNextHashBlock();

    // Link to the next extension block
    Block getNextListBlockRef() const;
    void setNextListBlockRef(Block ref);
    FSBlock *getNextListBlock();
    
    // Link to the next bitmap extension block
    Block getNextBmExtBlockRef() const;
    void setNextBmExtBlockRef(Block ref);
    FSBlock *getNextBmExtBlock();
    
    // Link to the first data block
    Block getFirstDataBlockRef() const;
    void setFirstDataBlockRef(Block ref);
    FSBlock *getFirstDataBlock();

    Block getDataBlockRef(isize nr) const;
    void setDataBlockRef(isize nr, Block ref);

    // Link to the next data block
    Block getNextDataBlockRef() const;
    void setNextDataBlockRef(Block ref);
    FSBlock *getNextDataBlock();

        
    //
    // Working with hash tables
    //
    
    // Returns the hash table size
    isize hashTableSize() const;

    // Returns a hash value for this block
    u32 hashValue() const;

    // Looks up an item in the hash table
    u32 getHashRef(u32 nr) const;
    void setHashRef(u32 nr, u32 ref);

    // Dumps the contents of the hash table for debugging
    void dumpHashTable() const;


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
    Block getBmBlockRef(isize nr) const;
    void setBmBlockRef(isize nr, Block ref);

    
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
        
    // Adds a data block reference to this block
    bool addDataBlockRef(Block ref);
    bool addDataBlockRef(u32 first, u32 ref);
    
    // Gets or sets the number of data bytes stored in this block
    u32 getDataBytesInBlock() const;
    void setDataBytesInBlock(u32 val);

    // Adds data bytes to this block
    isize addData(const u8 *buffer, isize size);
    
    
    //
    // Exporting
    //
    
    isize writeData(FILE *file);
    isize writeData(FILE *file, isize size);
};

typedef FSBlock* BlockPtr;


//
// Convenience macros used inside the check() methods
//

#define EXPECT_BYTE(exp) { \
if (value != (exp)) { *expected = (exp); return ERROR_FS_EXPECTED_VALUE; } }

#define EXPECT_LONGWORD(exp) { \
if ((byte % 4) == 0 && BYTE3(value) != BYTE3((u32)exp)) \
    { *expected = (BYTE3((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } \
if ((byte % 4) == 1 && BYTE2(value) != BYTE2((u32)exp)) \
    { *expected = (BYTE2((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } \
if ((byte % 4) == 2 && BYTE1(value) != BYTE1((u32)exp)) \
    { *expected = (BYTE1((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } \
if ((byte % 4) == 3 && BYTE0(value) != BYTE0((u32)exp)) \
    { *expected = (BYTE0((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } }

#define EXPECT_CHECKSUM EXPECT_LONGWORD(checksum())

#define EXPECT_LESS_OR_EQUAL(exp) { \
if (value > (u32)exp) \
{ *expected = (u8)(exp); return ERROR_FS_EXPECTED_SMALLER_VALUE; } }

#define EXPECT_DOS_REVISION { \
if (!FSVolumeTypeEnum::isValid(value)) return ERROR_FS_EXPECTED_DOS_REVISION; }

#define EXPECT_REF { \
if (!partition.dev.block(value)) return ERROR_FS_EXPECTED_REF; }

#define EXPECT_SELFREF { \
if (value != nr) return ERROR_FS_EXPECTED_SELFREF; }

#define EXPECT_FILEHEADER_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_FILEHEADER_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_HASH_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_FILEHEADER_BLOCK, FS_USERDIR_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_ROOT_BLOCK, FS_USERDIR_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_FILELIST_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_FILELIST_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_BITMAP_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_REF { \
if (value) { EXPECT_BITMAP_REF } }

#define EXPECT_BITMAP_EXT_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_BITMAP_EXT_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_DATA_BLOCK_OFS, FS_DATA_BLOCK_FFS); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return ERROR_FS_EXPECTED_DATABLOCK_NR; }

#define EXPECT_HASHTABLE_SIZE { \
if (value != 72) return ERROR_FS_INVALID_HASHTABLE_SIZE; }
