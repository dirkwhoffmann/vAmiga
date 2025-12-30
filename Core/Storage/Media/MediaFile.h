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
#include "AnyFile.h"
#include <sstream>
#include <fstream>

namespace vamiga {

class MediaFile {

public:

    // The wrapped file
    unique_ptr<class AnyFile> file;

    MediaFile(unique_ptr<AnyFile> file) : file(std::move(file)) {}
    virtual ~MediaFile() = default;


    //
    // Static methods
    //

public:

    // Determines the type of a media file
    static FileType type(const fs::path &path);
    static FileType type(const AnyFile &file);

    // Factory methods (TODO: Return unique_ptr)
    static MediaFile *make(const fs::path &path);
    static MediaFile *make(const fs::path &path, FileType type);
    static MediaFile *make(const u8 *buf, isize len, FileType type);
    static MediaFile *make(class FileSystem &fs, FileType type);
    static MediaFile *make(class FloppyDriveAPI &drive, FileType type);
    static MediaFile *make(class HardDriveAPI &drive, FileType type);


    //
    // Public API
    //

public:

    // Returns the media type of this file
    FileType type() const;

    // Returns the size of this file
    isize getSize() const { return file->getSize(); }

    // Returns a textual representation of the file size
    // [[deprecated]] string getSizeAsString() const { return file->getSizeAsString(); }

    // Returns a pointer to the wrapped file
    const AnyFile *get() const { return file.get(); }
    AnyFile *get() { return file.get(); }

    // Returns a pointer to the file data
    u8 *getData() const { return file->getData(); }

    // Returns a fingerprint (hash value) for this file
    u64 fnv64() const { return file->fnv64(); }
    u32 crc32() const { return file->crc32(); }

    // Return a timestamp (if present)
    time_t timestamp() const { return time_t(0); } // TODO: ADD TO ANYFILE

    // Return the size of the preview image (only available for snapshot files)
    std::pair <isize,isize> previewImageSize() const;

    // Return a preview image (only available for snapshot files)
    const u32 *previewImageData() const ;

    // Handels data compression (only implemented by snapshot files)
    virtual Compressor compressor() const;
    virtual bool isCompressed() const;
    virtual void compress(Compressor method);
    virtual void uncompress();

    // Returns media information
    DiskInfo getDiskInfo() const;
    FloppyDiskInfo getFloppyDiskInfo() const;
    HDFInfo getHDFInfo() const;

    // Flashes the contents of the file into a buffer
    // void flash(u8 *buf, isize offset, isize len) { file->flash(buf, offset, len); }
    // void flash(u8 *buf, isize offset = 0) { file->flash(buf, offset); }


    //
    // Accessing raw data
    //

public:

    // Reads data from the file
    virtual u8 readByte(isize b, isize offset) const;
    virtual u8 readByte(isize t, isize s, isize offset) const;
    virtual void readSector(u8 *dst, isize b) const;
    virtual void readSector(u8 *dst, isize t, isize s) const;

    
    //
    // Serializing
    //

public:

    virtual isize writeToFile(const fs::path &path) const {
        return file->writeToFile(path);
    }
    virtual isize writeToFile(const fs::path &path, isize offset, isize length) const {
        return file->writeToFile(path, offset, length);
    }
};

}
