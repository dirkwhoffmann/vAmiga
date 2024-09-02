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
#include "MediaFile.h"
#include "Checksum.h"
#include "IOUtils.h"
#include "Buffer.h"
#include "Reflection.h"
#include <sstream>
#include <fstream>

namespace vamiga {

using util::Buffer;

/* All media files are organized in the class hierarchy displayed below. Two
 * abstract classes are involed: AmigaFile, DiskFile, and FloppyFile.
 * AmigaFile provides basic functionalities for reading and writing files,
 * streams, and buffers. DiskFile provides an abstract interface for accessing
 * files that represent hard disks and floppy disks. FloppyFile is the base
 * class of all floppy disk file formats.
 *
 *  ------------
 * | AmigaFile  |
 *  ------------
 *       |
 *       |-----------------------------------------------------------
 *       |       |           |           |           |               |
 *       |  ----------   ---------   ---------   ---------   -----------------
 *       | | Snapshot | | Script  | | HDFFile | | RomFile | | ExtendedRomFile |
 *       |  ----------   ---------   ---------   ---------   -----------------
 *       |
 *  ------------
 * |  DiskFile  |
 *  ------------
 *       |
 *       |------
 *       |      |
 *       |  ---------
 *       | | HDFFile |
 *       |  ---------

 *  --------------
 * |  FloppyFile  |
 *  --------------
 *       |
 *       |-----------------------------------------------------------
 *       |           |            |           |           |          |
 *   ---------   ----------   ---------   ---------   ---------   ---------
 *  | ADFFile | | EADFFile | | IMGFile | | DMSFile | | EXEFile | | Folder  |
 *   ---------   ----------   ---------   ---------   ---------   ---------
 */

class AmigaFile : public CoreObject, public MediaFile {

public:
    
    // Physical location of this file
    std::filesystem::path path;

    // The raw data of this file
    Buffer<u8> data;
    
    
    //
    // Initializing
    //
    
public:

    virtual ~AmigaFile();

    // void init(std::istream &stream) throws;
    // void init(const std::filesystem::path &path, std::istream &stream) throws;
    void init(isize len) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const Buffer<u8> &buffer) throws;
    void init(const string &str) throws;
    void init(const std::filesystem::path &path) throws;
    // void init(FILE *file) throws;
    
    explicit operator bool() const { return data.ptr != nullptr; }

    
    //
    // Methods from CoreObject
    //
    
    /*
private:
    
    void _dump(Category category, std::ostream& os) const override { }
     */


    //
    // Methods from MediaFile
    //
    
public:

    virtual isize getSize() const override { return data.size; }
    virtual u8 *getData() const override { return data.ptr; }
    virtual u64 fnv64() const override { return data.fnv64(); }
    virtual u32 crc32() const override { return data.crc32(); }

    
    //
    // Flashing
    //

    // Copies the file contents into a buffer
    virtual void flash(u8 *buf, isize offset, isize len) const override;
    virtual void flash(u8 *buf, isize offset = 0) const override;

    
    //
    // Serializing
    //
    
protected:
    
    virtual bool isCompatiblePath(const std::filesystem::path &path) const = 0;
    virtual bool isCompatibleBuffer(const u8 *buf, isize len) = 0;
    bool isCompatibleBuffer(const Buffer<u8> &buffer);

    // isize readFromStream(std::istream &stream) throws override;
    // isize readFromFile(const std::filesystem::path &path) throws override;
    isize readFromBuffer(const u8 *buf, isize len) throws override;
    isize readFromBuffer(const Buffer<u8> &buffer) throws;

public:
    
    isize writeToStream(std::ostream &stream, isize offset, isize len) throws;
    isize writeToFile(const std::filesystem::path &path, isize offset, isize len) throws;
    isize writeToBuffer(u8 *buf, isize offset, isize len) throws;
    isize writeToBuffer(Buffer<u8> &buffer, isize offset, isize len) throws;

    isize writeToStream(std::ostream &stream) throws override;
    isize writeToFile(const std::filesystem::path &path) throws override;
    isize writePartitionToFile(const std::filesystem::path &path, isize partition) throws override;
    isize writeToBuffer(u8 *buf) throws override;
    isize writeToBuffer(Buffer<u8> &buffer) throws;

private:
    
    // Delegation methods
    virtual void finalizeRead() throws { };
    virtual void finalizeWrite() throws { };
};

}
