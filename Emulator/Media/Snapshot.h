// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"
#include "Constants.h"

namespace vamiga {

struct Thumbnail {
    
    // Image size
    i32 width, height;
    
    // Raw texture data
    u32 screen[(HPIXELS / 2) * (VPIXELS / 1)];
    
    // Creation date and time
    time_t timestamp;
    
    // Takes a screenshot from a given Amiga
    void take(class Amiga &amiga, isize dx = 2, isize dy = 1);
};

struct SnapshotHeader {
    
    // Magic bytes ('V','A','S','N','A','P')
    char magic[6];
    
    // Version number (major.minor.subminor['b'beta])
    u8 major;
    u8 minor;
    u8 subminor;
    u8 beta;

    // Indicates if the snapshot contents is stored in compressed form
    bool compressed;

    // Size of this snapshot when uncompressed
    i32 rawSize;
    
    // Preview image
    Thumbnail screenshot;
};

class Snapshot : public AmigaFile {
    
public:
    
    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    
    //
    // Initializing
    //
    
    Snapshot(const Snapshot &other) throws { init(other.data.ptr, other.data.size); }
    Snapshot(const std::filesystem::path &path) throws { init(path); }
    Snapshot(const u8 *buf, isize len) throws { init(buf, len); }
    Snapshot(isize capacity);
    Snapshot(Amiga &amiga);
    
    const char *objectName() const override { return "Snapshot"; }
    
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_SNAPSHOT; }
    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }
    void finalizeRead() throws override;
    
    
    //
    // Accessing
    //
    
public:
    
    std::pair <isize,isize> previewImageSize() const override;
    const u32 *previewImageData() const override;
    time_t timestamp() const override;
    
    // Checks the snapshot version number
    bool isTooOld() const;
    bool isTooNew() const;
    bool isBeta() const;
    bool matches() { return !isTooOld() && !isTooNew(); }
    
    // Returns a pointer to the snapshot header
    SnapshotHeader *getHeader() const { return (SnapshotHeader *)data.ptr; }
    
    // Returns a pointer to the thumbnail image
    const Thumbnail &getThumbnail() const { return getHeader()->screenshot; }
    
    // Returns pointer to the core data
    u8 *getData() const override { return data.ptr + sizeof(SnapshotHeader); }
    
    // Takes a screenshot
    void takeScreenshot(Amiga &amiga);
    

    //
    // Compressing
    //

    // Indicates whether the snapshot is compressed
    bool isCompressed() const override { return getHeader()->compressed; }

    // Compresses or uncompresses the snapshot
    void compress() override;
    void uncompress() override;
};

}
