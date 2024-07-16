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
#include <sstream>
#include <fstream>
#include <filesystem>

namespace vamiga {

namespace fs = ::std::filesystem;

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
    static MediaFile *make(struct FloppyDriveAPI &drive, FileType type);


    //
    // Methods
    //

    // Returns the media type of this file
    virtual FileType type() const { return FILETYPE_UNKNOWN; }

    // Returns the size of this file
    virtual isize getSize() const = 0;

    // Returns a pointer to the file data
    virtual u8 *getData() const = 0;

    // Returns the logical name of this file
    // virtual string name() const = 0;

    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv64() const = 0;
    virtual u32 crc32() const = 0;

    // Return a timestamp (if present)
    virtual time_t timestamp() const { return time_t(0); }

    // Return the size of the preview image (only available for snapshot files)
    virtual std::pair <isize,isize> previewImageSize() const { return { 0, 0 }; }

    // Return a preview image (only available for snapshot files)
    virtual const u32 *previewImageData() const { return nullptr; }

    //
    virtual void flash(u8 *buf, isize offset, isize len) const = 0;
    virtual void flash(u8 *buf, isize offset = 0) const = 0;


    //
    // Serializing
    //

public:

    virtual isize readFromStream(std::istream &stream) = 0;
    virtual isize readFromFile(const string &path) = 0;
    virtual isize readFromBuffer(const u8 *buf, isize len) = 0;

    virtual isize writeToStream(std::ostream &stream) = 0;
    virtual isize writeToFile(const string &path) = 0;
    virtual isize writeToBuffer(u8 *buf) = 0;
};

}
