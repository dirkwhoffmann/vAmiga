// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

/* The FileSystem class represents an Amiga file system (OFS or FFS).
 * It models a logical volume that can be created from either an ADF or an HDF.
 * In the case of an HDF, each partition can be converted into an independent
 * file system instance.
 *
 * The FileSystem class is organized as a layered architecture to separate
 * responsibilities and to enforce downward-only dependencies.
 *
 *  ---------------------
 * |     POSIX Layer     |    Layer 3:
 *  ---------------------
 *            |               Wraps a Layer-3 file system with all lower-level
 *            |               access functions hidden. It exposes a POSIX-like
 *            |               high-level API that provides operations such as
 *            |               open, close, read, write, and file handles.
 *            V
 *  -----------------------
 * | Path Resolution Layer |  Layer 2:
 *  -----------------------
 *            |               Resolves symbolic and relative paths into file
 *            |               system objects and canonicalizes paths. It depends
 *            |               only on the read/write layer.
 *            |
 *            V
 *  -----------------------
 * |       Node Layer      |  Layer 1:
 *  -----------------------
 *            |               Interprets storage blocks as files and directories
 *            |               according to OFS or FFS semantics. It allows the
 *            |               upper layers to create files, directories, and to
 *            |               modify metadata.
 *            V
 *  -----------------------
 * |  Block Storage Layer  |  Layer 0:
 *  -----------------------
 *                            Storage the actual block data.
 *
 */

#include "FSTypes.h"
#include "FSBlock.h"
#include "FSDescriptor.h"
#include "FSObjects.h"
#include "FSTree.h"
#include "FSStorage.h"
#include "FSDoctor.h"
#include "FSAllocator.h"
#include "FSImporter.h"
#include "FSExporter.h"
#include "Loggable.h"

#include "ADFFile.h"
#include "HDFFile.h"

namespace vamiga {

class ADFFile;
class HDFFile;
class FloppyDrive;
class HardDrive;

class FileSystem : public Loggable, public Dumpable, public Inspectable<FSInfo, Void> {

    friend struct FSBlock;
    friend class  FSComponent;
    friend class  FSDoctor;
    friend class  FSAllocator;
    friend struct FSHashTable;
    friend struct FSPartition;

    // Static file system properties
    FSTraits traits;

public:

    // Public Components
    FSDoctor doctor = FSDoctor(*this);
    FSImporter importer = FSImporter(*this);
    FSExporter exporter = FSExporter(*this);

private:

    // Private Components
    FSStorage storage = FSStorage(this);
    FSAllocator allocator = FSAllocator(*this);

    // Location of the root block
    Block rootBlock = 0;

    // Location of the current directory (TODO: MOVE TO POSIX LAYER)
    Block current = 0;

    // Location of bitmap blocks and extended bitmap blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;


    //
    // Initializing
    //
    
public:
    
    FileSystem() { stats = {}; };
    FileSystem(const FSDescriptor &layout, u8 *buf, isize len) : FileSystem() { init(layout, buf, len); }
    /*
    FileSystem(const MediaFile &file, isize part = 0) : FileSystem() { init(file, part); }
    FileSystem(const ADFFile &adf) : FileSystem() { init(adf); }
    FileSystem(const HDFFile &hdf, isize part = 0) : FileSystem() { init(hdf, part); }
    FileSystem(const FloppyDrive &dfn) : FileSystem() { init(dfn); }
    FileSystem(const HardDrive &hdn, isize part = 0) : FileSystem() { init(hdn, part); }
     */

    FileSystem(isize capacity, isize bsize = 512) { init(capacity, bsize); }
    FileSystem(const FSDescriptor &layout, const fs::path &path = {}) { init(layout, path); }
    FileSystem(Diameter dia, Density den, FSFormat dos, const fs::path &path = {}) { init(dia, den, dos, path); }

    FileSystem(const FileSystem&) = delete;
    FileSystem& operator=(const FileSystem&) = delete;
    
    virtual ~FileSystem();

    void init(const FSDescriptor &layout, u8 *buf, isize len);
    void init(const MediaFile &file, isize part);
    void init(const ADFFile &adf);
    void init(const HDFFile &hdf, isize part);
    void init(const FloppyDrive &dfn);
    void init(const HardDrive &hdn, isize part);

    void init(isize capacity, isize bsize = 512);
    void init(const FSDescriptor &layout, const fs::path &path = {});
    void init(Diameter dia, Density den, FSFormat dos, const fs::path &path = {});
    
    bool isInitialized() const noexcept;
    bool isFormatted() const noexcept;


    //
    // Methods from CoreObject
    //
    
protected:
    
    // const char *objectName() const noexcept override { return "FileSystem"; }
    void _dump(Category category, std::ostream &os) const noexcept override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(FSInfo &result) const noexcept override;
    // void cacheStats(FSStats &result) const noexcept override;


    //
    // Querying properties
    //

public:

    // Returns static file system properties
    const FSTraits &getTraits() const noexcept { return traits; }

    // Returns capacity information
    isize numBlocks() const noexcept { return storage.numBlocks(); }
    [[deprecated]] isize numBytes() const noexcept { return storage.numBytes(); }
    [[deprecated]] isize blockSize() const noexcept { return storage.blockSize(); }

    // Returns usage information and root metadata
    FSStat getStat() const noexcept;

    // Get information about the file system
    FSAttr attr(Block nr) const;
    FSAttr attr(const FSBlock &fhd) const;


    // Reports usage information
    /*
    [[deprecated]] isize freeBlocks() const noexcept { return storage.freeBlocks(); }
    [[deprecated]] isize usedBlocks() const noexcept { return storage.usedBlocks(); }
    [[deprecated]] isize freeBytes() const noexcept { return storage.freeBytes(); }
    [[deprecated]] isize usedBytes() const noexcept { return storage.usedBytes(); }
    */

    // Analyzes the root block
    /*
    [[deprecated]] FSName getName() const noexcept;
    [[deprecated]] FSTime getCreationDate() const noexcept;
    [[deprecated]] FSTime getModificationDate() const noexcept;
    */

    // Analyzes the boot block
    string getBootBlockName() const noexcept;
    BootBlockType bootBlockType() const noexcept;
    bool hasVirus() const noexcept { return bootBlockType() == BootBlockType::VIRUS; }


    //
    // Querying block properties
    //

public:

    // Returns the type of a certain block or a block item
    FSBlockType typeOf(Block nr) const noexcept;
    bool is(Block nr, FSBlockType t) const noexcept { return typeOf(nr) == t; }
    FSItemType typeOf(Block nr, isize pos) const noexcept;

    // Convenience wrappers
    bool isEmpty(Block nr) const noexcept { return typeOf(nr) == FSBlockType::EMPTY; }

    // Predicts the type of a block
    FSBlockType predictType(Block nr, const u8 *buf) const noexcept;


    //
    // Accessing the block storage
    //
    
public:

    // Returns a block pointer or null if the block does not exist
    FSBlock *read(Block nr) noexcept;
    FSBlock *read(Block nr, FSBlockType type) noexcept;
    FSBlock *read(Block nr, std::vector<FSBlockType> types) noexcept;
    const FSBlock *read(Block nr) const noexcept;
    const FSBlock *read(Block nr, FSBlockType type) const noexcept;
    const FSBlock *read(Block nr, std::vector<FSBlockType> types) const noexcept;

    // Returns a reference to a stored block (throws on error)
    FSBlock &at(Block nr);
    FSBlock &at(Block nr, FSBlockType type);
    FSBlock &at(Block nr, std::vector<FSBlockType> types);
    const FSBlock &at(Block nr) const;
    const FSBlock &at(Block nr, FSBlockType type) const;
    const FSBlock &at(Block nr, std::vector<FSBlockType> types) const;

    // Operator overload
    FSBlock &operator[](size_t nr);
    const FSBlock &operator[](size_t nr) const;


    //
    // Managing files and directories
    //

public:

    // Returns the root of the directory tree
    FSBlock &root() { return at(rootBlock); }
    const FSBlock &root() const { return at(rootBlock); }

    // Returns the working directory (TODO: MOVE TO POSIX LAYER)
    FSBlock &pwd() { return at(current); }
    const FSBlock &pwd() const { return at(current); }

    // Returns the parent directory
    FSBlock &parent(const FSBlock &block);
    FSBlock *parent(const FSBlock *block) noexcept;
    const FSBlock &parent(const FSBlock &block) const;
    const FSBlock *parent(const FSBlock *block) const noexcept;

    // Changes the working directory (TODO: MOVE TO POSIX LAYER)
    void cd(const FSName &name);
    void cd(const FSBlock &path);
    void cd(const string &path);

    // Checks if a an item exists in the directory tree
    bool exists(const FSBlock &top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(pwd(), path); }

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock *seekPtr(const FSBlock *top, const FSName &name) noexcept;
    FSBlock *seekPtr(const FSBlock *top, const fs::path &name) noexcept;
    FSBlock *seekPtr(const FSBlock *top, const string &name) noexcept;
    const FSBlock *seekPtr(const FSBlock *top, const FSName &name) const noexcept;
    const FSBlock *seekPtr(const FSBlock *top, const fs::path &name) const noexcept;
    const FSBlock *seekPtr(const FSBlock *top, const string &name) const noexcept;

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock &seek(const FSBlock &top, const FSName &name);
    FSBlock &seek(const FSBlock &top, const fs::path &name);
    FSBlock &seek(const FSBlock &top, const string &name);
    const FSBlock &seek(const FSBlock &top, const FSName &name) const;
    const FSBlock &seek(const FSBlock &top, const fs::path &name) const;
    const FSBlock &seek(const FSBlock &top, const string &name) const;

    // Seeks all items satisfying a predicate
    std::vector<const FSBlock *> find(const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock *root, const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock &root, const FSOpt &opt) const;
    std::vector<Block> find(Block root, const FSOpt &opt) const;

    // Seeks all items with a pattern-matching name
    std::vector<const FSBlock *> find(const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock *top, const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock &top, const FSPattern &pattern) const;
    std::vector<Block> find(Block root, const FSPattern &pattern) const;

    // Collects all items with a pattern-matching path
    std::vector<const FSBlock *> match(const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock *top, const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock &top, const FSPattern &pattern) const;
    std::vector<Block> match(Block root, const FSPattern &pattern) const;

private:

    std::vector<const FSBlock *> find(const FSBlock *top, const FSOpt &opt,
                                      std::unordered_set<Block> &visited) const;

    std::vector<const FSBlock *> match(const FSBlock *top,
                                       std::vector<FSPattern> pattern) const;


    //
    // Traversing linked lists
    //

public:
    
    // Follows a linked list and collects all blocks
    std::vector<const FSBlock *> collect(const FSBlock &block,
                                         std::function<const FSBlock *(const FSBlock *)> next) const;
    std::vector<Block> collect(const Block nr,
                               std::function<const FSBlock *(const FSBlock *)> next) const;

    // Collects blocks of a certain type
    std::vector<const FSBlock *> collectDataBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectListBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block, isize bucket) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block) const;
    std::vector<Block> collectDataBlocks(Block nr) const;
    std::vector<Block> collectListBlocks(Block nr) const;
    std::vector<Block> collectHashedBlocks(Block nr, isize bucket) const;
    std::vector<Block> collectHashedBlocks(Block nr) const;


    //
    // Argument checkers
    //

public:

    void require_initialized() const;
    void require_formatted() const;
    void require_file_or_directory(const FSBlock &block) const;

    void ensureFile(const FSBlock &node);
    void ensureFileOrDirectory(const FSBlock &node);
    void ensureDirectory(const FSBlock &node);
    void ensureNotRoot(const FSBlock &node);
    void ensureEmptyDirectory(const FSBlock &node);


    //
    // LAYER 2: WRITE
    //

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

private:

    // Adds a new block of a certain kind
    void addFileListBlock(Block at, Block head, Block prev);
    void addDataBlock(Block at, isize id, Block head, Block prev);

    // Creates a new block of a certain kind
    FSBlock &newUserDirBlock(const FSName &name);
    FSBlock &newFileHeaderBlock(const FSName &name);


    //
    // Managing the boot blocks
    //

public:

    // Installs a boot block
    void makeBootable(BootBlockId id);

    // Removes a boot block virus from the current partition (if any)
    void killVirus();


    //
    // Creating files and directories
    //

public:

    // Creates a new directory
    FSBlock &createDir(FSBlock &at, const FSName &name);

    // Creates a directory entry
    FSBlock &link(FSBlock &at, const FSName &name); // DEPRECATED
    void link(FSBlock &at, const FSName &name, FSBlock &fhb);  // DEPRECATED
    void link(FSBlock &at, FSBlock &fhb);

    // Removes a directory entry
    void unlink(const FSBlock &fhb);

    // Frees the file header block and all related data blocks
    void reclaim(const FSBlock &fhb);

    // Creates a new file
    FSBlock &createFile(FSBlock &at, const FSName &name);
    FSBlock &createFile(FSBlock &at, const FSName &name, const Buffer<u8> &buf);
    FSBlock &createFile(FSBlock &at, const FSName &name, const string &str);
    FSBlock &createFile(FSBlock &at, const FSName &name, const u8 *buf, isize size);

private:

    // FSBlock &createFile(FSBlock &at, FSBlock &fhb, const u8 *buf, isize size);
    FSBlock &replace(FSBlock &fhb,
                     const u8 *buf, isize size,
                     std::vector<Block> listBlocks = {},
                     std::vector<Block> dataBlocks = {});

public:

    // Changes the size of an existing file, pads with 0
    void resize(FSBlock &at, isize size);

    // Changes the size and cotents of an existing file
    void replace(FSBlock &at, const Buffer<u8> &data);

    // Update file contents with new data

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
};

}
