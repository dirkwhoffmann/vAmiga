// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MediaFileTypes.h"
#include "FloppyDiskTypes.h"
#include "AmigaTypes.h"
#include <sstream>
#include <fstream>

namespace vamiga {

class MediaFile {

public:

    virtual ~MediaFile() = default;


    //
    // Static methods
    //

    // Determines the type of an arbitrary file on disk
    static FileType type(const fs::path &path);

    // Factory methods
    static MediaFile *make(const fs::path &path);
    static MediaFile *make(const fs::path &path, FileType type);
    static MediaFile *make(const u8 *buf, isize len, FileType type);
    static MediaFile *make(class MutableFileSystem &fs, FileType type);
    static MediaFile *make(class FloppyDriveAPI &drive, FileType type);
    static MediaFile *make(class HardDriveAPI &drive, FileType type);


    //
    // Methods
    //

    // Returns the media type of this file
    virtual FileType type() const { return FileType::UNKNOWN; }

    // Returns the size of this file
    virtual isize getSize() const = 0;

    // Returns a textual representation of the file size
    virtual string getSizeAsString() const;

    // Returns a pointer to the file data
    virtual u8 *getData() const = 0;

    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv64() const = 0;
    virtual u32 crc32() const = 0;

    // Return a timestamp (if present)
    virtual time_t timestamp() const { return time_t(0); }

    // Return the size of the preview image (only available for snapshot files)
    virtual std::pair <isize,isize> previewImageSize() const { return { 0, 0 }; }

    // Return a preview image (only available for snapshot files)
    virtual const u32 *previewImageData() const { return nullptr; }

    // Handels data compression (only implemented by snapshot files)
    virtual Compressor compressor() const { return Compressor::NONE; }
    virtual bool isCompressed() const { return compressor() != Compressor::NONE; }
    virtual void compress(Compressor method) { }
    virtual void uncompress() { }

    // Returns media information
    DiskInfo getDiskInfo() const;
    FloppyDiskInfo getFloppyDiskInfo() const;
    HDFInfo getHDFInfo() const;

    // Flashes the contents of the file into a buffer
    virtual void flash(u8 *buf, isize offset, isize len) const = 0;
    virtual void flash(u8 *buf, isize offset = 0) const = 0;
    

    //
    // Accessing raw data
    //

public:

    // Reads data from the file
    virtual u8 readByte(isize b, isize offset) const { return 0; }
    virtual u8 readByte(isize t, isize s, isize offset) const { return 0; }
    virtual void readSector(u8 *dst, isize b) const { }
    virtual void readSector(u8 *dst, isize t, isize s) const { }

    // Generates a hex dump for some sector data
    virtual string hexdump(isize b, isize offset, isize len) const { return ""; }
    virtual string hexdump(isize t, isize s, isize offset, isize len) const { return ""; }
    virtual string hexdump(isize c, isize h, isize s, isize offset, isize len) const { return ""; }

    // Generates an ASCII dump for some sector data
    virtual string asciidump(isize b, isize offset, isize len) const { return ""; }
    virtual string asciidump(isize t, isize s, isize offset, isize len) const { return ""; }
    virtual string asciidump(isize c, isize h, isize s, isize offset, isize len) const { return ""; }

    //
    // Serializing
    //

public:

    virtual isize readFromBuffer(const u8 *buf, isize len) = 0;

    virtual isize writeToStream(std::ostream &stream) const = 0;
    virtual isize writeToFile(const fs::path &path) const = 0;
    virtual isize writePartitionToFile(const fs::path &path, isize partition) const = 0;
    virtual isize writeToBuffer(u8 *buf) const = 0;
};

}
