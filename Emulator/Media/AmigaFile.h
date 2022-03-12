// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFileTypes.h"
#include "AmigaObject.h"
#include "Checksum.h"
#include "IOUtils.h"
#include "Buffer.h"
#include "Reflection.h"
#include <sstream>
#include <fstream>

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
 *       |           |           |           |            |          |
 *   ---------   ---------   ---------   ---------    ---------  ---------
 *  | ADFFile | | EXTFile | | IMGFile | | DMSFile | | EXEFile | | Folder  |
 *   ---------   ---------   ---------   ---------    ---------  ---------
 */

class AmigaFile : public AmigaObject {
    
public:
    
    // Physical location of this file
    string path;
    
    // The raw data of this file
    util::Buffer data;
    
    
    //
    // Initializing
    //
    
public:

    virtual ~AmigaFile();
        
    void init(std::istream &stream) throws;
    void init(const string &path, std::istream &stream) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const util::Buffer &buffer) throws;
    void init(const string &path) throws;
    void init(FILE *file) throws;
    
    explicit operator bool() const { return data.ptr != nullptr; }

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    void _dump(dump::Category category, std::ostream& os) const override { }

    
    //
    // Accessing
    //
    
public:
    
    // Determines the type of an arbitrary file on file
    static FileType type(const string &path);
    
    // Returns the type of this file
    virtual FileType type() const { return FILETYPE_UNKNOWN; }
            
    // Returns a textual description of the file size
    virtual string sizeAsString();
    
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() const { return data.fnv64(); }
        
    
    //
    // Flashing
    //
            
    // Copies the file contents into a buffer
    virtual void flash(u8 *buf, isize offset, isize len) const;
    virtual void flash(u8 *buf, isize offset) const;
    virtual void flash(u8 *buf) const;

    
    //
    // Serializing
    //
    
protected:
    
    virtual bool isCompatiblePath(const string &path) const = 0;
    virtual bool isCompatibleStream(std::istream &stream) const = 0;
    
    isize readFromStream(std::istream &stream) throws;
    isize readFromFile(const string &path) throws;
    isize readFromBuffer(const u8 *buf, isize len) throws;
    isize readFromBuffer(const util::Buffer &buffer) throws;

public:
    
    isize writeToStream(std::ostream &stream, isize offset, isize len) throws;
    isize writeToFile(const string &path, isize offset, isize len) throws;
    isize writeToBuffer(u8 *buf, isize offset, isize len) throws;
    isize writeToBuffer(util::Buffer &buffer, isize offset, isize len) throws;

    isize writeToStream(std::ostream &stream) throws;
    isize writeToFile(const string &path) throws;
    isize writeToBuffer(u8 *buf) throws;
    isize writeToBuffer(util::Buffer &buffer) throws;

private:
    
    // Delegation methods
    virtual void finalizeRead() throws { };
    virtual void finalizeWrite() throws { };
};
