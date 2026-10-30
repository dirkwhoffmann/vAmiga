// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "AnyFile.h"
#include "Constants.h"
#include "AmigaTypes.h"

namespace vamiga {

/// A screenshot that is stored inside a snapshot.
struct Thumbnail {
    
    // Image size
    i32 width;                           ///< Image width
    i32 height;                          ///< Image height
    
    /// Raw texture data
    u32 screen[(HPIXELS / 2) * (VPIXELS / 1)];
    
    /// Creation date and time
    time_t timestamp;
    
    /// Takes a screenshot from the current texture
    void take(class Amiga &amiga, isize dx = 2, isize dy = 1);
};

/// Header of a snapshot file.
struct SnapshotHeader {
    
    /// Magic bytes ('V','A','S','N','A','P')
    char magic[6];
    
    // Version number (major.minor.subminor['b'beta])
    u8 major;                            ///< Major version number
    u8 minor;                            ///< Minor version number
    u8 subminor;                         ///< Subminor version number
    u8 beta;                             ///< Beta release number

    /// Applied compression method
    u8 compressor;

    /// Size of this snapshot when uncompressed
    i32 rawSize;
    
    /// Preview image
    Thumbnail screenshot;
};

/// A snapshot of the emulator state.
class Snapshot : public AnyFile {
    
public:
    
    /// Checks whether a file contains a snapshot
    static bool isCompatible(const fs::path &path);

    
    //
    // Initializing
    //
    
    /// Creates a copy of a snapshot
    Snapshot(const Snapshot &other) { init(other.data.ptr, other.data.size); }
    /// Reads a snapshot from a file
    Snapshot(const fs::path &path) { init(path); }
    /// Reads a snapshot from a memory buffer
    Snapshot(const u8 *buf, isize len) { init(buf, len); }
    /// Creates an empty snapshot with the given capacity
    Snapshot(isize capacity);
    /// Takes a snapshot of the emulator state
    Snapshot(Amiga &amiga);
    /// Takes a snapshot of the emulator state and compresses it
    Snapshot(Amiga &amiga, utl::Compressor compressor);
    
    
    //
    // Methods from AnyFile
    //
    
    /// Returns true if the given path points to a snapshot file
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    /// Called after the snapshot has been loaded
    void didLoad() override;
    
    
    //
    // Accessing
    //
    
public:
    
    /// Returns the size of the preview image (width, height)
    std::pair <isize,isize> previewImageSize() const;
    /// Returns the pixel data of the preview image
    const u32 *previewImageData() const;
    /// Returns the time the snapshot was taken
    time_t timestamp() const;
    
    // Checks the snapshot version number
    /// Returns true if the snapshot was created by an older, incompatible version
    bool isTooOld() const;
    /// Returns true if the snapshot was created by a newer version
    bool isTooNew() const;
    /// Returns true if the snapshot was created by a beta version
    bool isBeta() const;
    /// Returns true if the snapshot can be restored by this version
    bool matches() { return !isTooOld() && !isTooNew(); }
    
    // Returns a pointer to the snapshot header
    /// Returns a pointer to the snapshot header
    SnapshotHeader *getHeader() const { return (SnapshotHeader *)data.ptr; }
    
    // Returns a pointer to the thumbnail image
    /// Returns the preview image
    const Thumbnail &getThumbnail() const { return getHeader()->screenshot; }
    
    // Returns pointer to the core data
    // u8 *getData() const override { return data.ptr + sizeof(SnapshotHeader); }
    
    // Takes a screenshot
    /// Records a screenshot
    void takeScreenshot(Amiga &amiga);
    

    //
    // Compressing
    //

    // Returns the compression method
    /// Returns the compression method
    utl::Compressor compressor() const { return utl::Compressor(getHeader()->compressor); }

    // Convenience wrapper
    /// Returns true if the snapshot is compressed
    bool isCompressed() const { return compressor() != utl::Compressor::NONE; }

    // Compresses or uncompresses the snapshot
    /// Compresses the snapshot with the given method
    void compress(utl::Compressor method);
    /// Uncompresses the snapshot
    void uncompress();
};

}
