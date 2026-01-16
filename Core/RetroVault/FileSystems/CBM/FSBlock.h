// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystems/CBM/FSService.h"
#include "FileSystems/CBM/FSTypes.h"
#include "FileSystems/CBM/FSError.h"
#include "FileSystems/CBM/FSObjects.h"
#include "utl/storage.h"
#include "utl/abilities/Dumpable.h"

namespace retro::vault::cbm {

using utl::Buffer;
using utl::DumpOpt;

struct FSBlock : Loggable, Hashable, Dumpable {

    // The file system this block belongs to (DEPRECATED)
    class FileSystem *fs = nullptr;

    // The storage this block belongs to
    class FSCache &cache;

    // The type of this block
    FSBlockType type = FSBlockType::UNKNOWN;

    // The number of this block
    BlockNr nr = 0;

    // Cached block data
    Buffer<u8> dataCache;


    //
    // Constructing
    //

public:

    FSBlock(const FSBlock&) = delete;             // Copy constructor
    FSBlock& operator=(const FSBlock&) = delete;  // Copy assignment
    FSBlock(FSBlock&&) = delete;                  // Move constructor
    FSBlock& operator=(FSBlock&&) = delete;       // Move assignment

    FSBlock(FileSystem *ref, BlockNr nr);
    FSBlock(FileSystem *ref, BlockNr nr, FSBlockType t);
    ~FSBlock();

    void init(FSBlockType t);

    // static FSBlock *make(FileSystem *ref, BlockNr nr, FSBlockType type);
    static std::vector<BlockNr> refs(const std::vector<const FSBlock *> blocks);


    //
    // Methods from Hashable
    //

    u64 hash(HashAlgorithm algorithm) const override {

        return dataCache.empty() ? 0 : dataCache.hash(algorithm);
    }


    //
    // Methods from Dumpable
    //

    Dumpable::DataProvider dataProvider() const override {

        if (dataCache.empty()) {
            return [&](isize offset, isize bytes) { return offset < bsize() ? 0 : -1; };
        } else {
            return dataCache.dataProvider();
        }
    }


    //
    // Printing debug information
    //
    
public:

    // const char *objectName() const;
    void dumpInfo(std::ostream &os) const;
    void dumpStorage(std::ostream &os) const;
    void dumpBlocks(std::ostream &os) const;

    
    //
    // Querying block properties
    //

public:

    // Informs about the block type
    bool is(FSBlockType type) const;
    bool isEmpty() const;
    bool isBAM() const;
    /*
    bool isDirectory() const;
    bool isRegular() const;
    bool isData() const;
    */

    // [[deprecated]] string relName(BlockNr top) const;

    // Experimental
    //string acabsName() const;
    // string acrelName() const;

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
    [[deprecated]] u32 typeID() const;
    [[deprecated]] u32 subtypeID() const;

    // Returns the track / sector link stored in the fist two bytes
    TSLink tsLink() const { auto *p = data(); return TSLink {p[0],p[1]}; }


    //
    // Reading and writing block data
    //

    // Provides the data of a block
    u8 *data();
    const u8 *data() const;

    // Grants write access for this block
    FSBlock &mutate() const;

    // Marks this block as dirty in the block cache
    void invalidate();

    // Writes the block back to the block device
    void flush();

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
    void set32(isize n, isize val) { write32(addr32(n), u32(val)); }
    void inc32(isize n) { inc32(addr32(n)); }
    void dec32(isize n) { dec32(addr32(n)); }


    //
    // Printing
    //

public:

    // void hexDump(std::ostream &os, const DumpOpt &opt) const;

    // Experimental
    static string rangeString(const std::vector<BlockNr> &vec);


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
    FSFault exportBlock(const fs::path &path) const;

private:
    
    FSFault exportUserDirBlock(const fs::path &path) const;
    FSFault exportFileHeaderBlock(const fs::path &path) const;


    //
    // Geting and setting names
    //
    
public:
    
    bool hasName() const;
    PETName<16> getName() const;
    void setName(PETName<16> name);

    
    //
    // Exporting
    //
    
    isize writeData(std::ostream &os) const;
    isize writeData(std::ostream &os, isize size) const;
    isize writeData(Buffer<u8> &buf, isize offset, isize count) const;
    isize extractData(Buffer<u8> &buf) const;

    
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

/*
inline std::function<bool(const FSBlock &, const FSBlock &)> dafa = [](const FSBlock &b1, const FSBlock &b2)
{
    if ( b1.isDirectory() && !b2.isDirectory()) return true;
    if (!b1.isDirectory() &&  b2.isDirectory()) return false;
    return b1.getName() < b2.getName();
};

inline std::function<bool(const FSBlock *, const FSBlock *)> dafaPtr = [](const FSBlock *b1, const FSBlock *b2)
{
    if ( b1->isDirectory() && !b2->isDirectory()) return true;
    if (!b1->isDirectory() &&  b2->isDirectory()) return false;
    return b1->getName() < b2->getName();
};

inline std::function<bool(const FSBlock &, const FSBlock &)> alpha = [](const FSBlock &b1, const FSBlock &b2)
{
    return b1.getName() < b2.getName();
};

inline std::function<bool(const FSBlock *, const FSBlock *)> alphaPtr = [](const FSBlock *b1, const FSBlock *b2)
{
    return b1->getName() < b2->getName();
};

*/

/*
inline std::function<bool(const FSBlock &, const FSBlock &)> none = nullptr;
inline std::function<bool(const FSBlock *, const FSBlock *)> nonePtr = nullptr;
*/

}

/*
namespace accept {

inline std::function<bool(const FSBlock &)> all = [](const FSBlock &b1)
{
    return true;
};

inline std::function<bool(const FSBlock &)> files = [](const FSBlock &b)
{
    return b.isFile();
};

inline std::function<bool(const FSBlock &)> directories = [](const FSBlock &b)
{
    return b.isDirectory();
};

inline std::function<bool(const FSBlock &)> pattern(const FSPattern &p)
{
    return [p](const FSBlock &b) { return false; };
    // return [p](const FSBlock &b) { return p.match(b.name()); };
};

}
*/

}
