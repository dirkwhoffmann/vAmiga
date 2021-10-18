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
#include "IO.h"
#include "Reflection.h"
#include <sstream>
#include <fstream>

/* All media files are organized in the class hierarchy displayed below. Two
 * abstract classes are involed: AmigaFile and DiskFile. AmigaFile provides
 * basic functionality for reading and writing files, streams, and buffers.
 * DiskFile provides an abstract interface for accessing media files that will
 * be mounted as a virtual floppy disk.
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
    u8 *data = nullptr;
    
    // The size of this file in bytes
    isize size = 0;
    

    //
    // Initializing
    //
    
public:

    virtual ~AmigaFile();
        
    void init(std::istream &stream) throws;
    void init(const string &path, std::istream &stream) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const string &path) throws;
    void init(FILE *file) throws;
    
    
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
            
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() const { return util::fnv_1a_64(data, size); }
        
    
    //
    // Flashing
    //
            
    // Copies the file contents into a buffer starting at the provided offset
    virtual void flash(u8 *buf, isize offset = 0) const;
    
    
    //
    // Serializing
    //
    
protected:
    
    virtual bool isCompatiblePath(const string &path) const = 0;
    virtual bool isCompatibleStream(std::istream &stream) const = 0;
    
    isize readFromStream(std::istream &stream) throws;
    isize readFromFile(const string &path) throws;
    isize readFromBuffer(const u8 *buf, isize len) throws;

public:
    
    isize writeToStream(std::ostream &stream) throws;
    isize writeToFile(const string &path) throws;
    isize writeToBuffer(u8 *buf) throws;
    
private:
    
    // Delegation methods
    virtual void finalizeRead() throws { };
    virtual void finalizeWrite() throws { };
};
