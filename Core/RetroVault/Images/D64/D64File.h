// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDiskImage.h"

namespace retro::vault::image {

// class FileSystem;

class D64File : public FloppyDiskImage {

public:

    // D64 files come in six different sizes
    static constexpr isize D64_683_SECTORS     = 174848;
    static constexpr isize D64_683_SECTORS_ECC = 175531;
    static constexpr isize D64_768_SECTORS     = 196608;
    static constexpr isize D64_768_SECTORS_ECC = 197376;
    static constexpr isize D64_802_SECTORS     = 205312;
    static constexpr isize D64_802_SECTORS_ECC = 206114;

    static optional<ImageInfo> about(const fs::path &path);


    //
    // Initializing
    //

public:

    explicit D64File();
    explicit D64File(const fs::path &path) { init(path); }
    explicit D64File(isize len) { init(len); }
    explicit D64File(const u8 *buf, isize len) { init(buf, len); }

    using FloppyDiskImage::init;


    //
    // Methods from AnyImage
    //

public:

    bool validateURL(const fs::path& path) const noexcept override {
        return about(path).has_value();
    }
    
    ImageType type() const noexcept override { return ImageType::FLOPPY; }
    ImageFormat format() const noexcept override { return ImageFormat::D64; }
    std::vector<string> describe() const noexcept override;
    

    //
    // Methods from DiskImage
    //

public:

    FSFamily fsFamily() const noexcept override { return FSFamily::CBM; }


    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return 256; }


    //
    // Methods from DiskImage
    //

public:

    isize numCyls() const noexcept override;
    isize numHeads() const noexcept override;
    isize numSectors(isize) const noexcept override;


    //
    // Methods from FloppyDiskImage
    //

public:

    Diameter getDiameter() const noexcept override;
    Density getDensity() const noexcept override;


    //
    // Querying disk properties
    //

public:

    // Returns true if the file contains error correction codes
    bool hasEcc() const noexcept;

    // Returns the error correction codes (if any)
    optional<std::span<const u8>> ecc() const noexcept;


    // Returns a file system descriptor for this volume
    // struct FSDescriptor getFileSystemDescriptor() const;


    //
    // Formatting
    //

public:

    // void formatDisk(FSFormat fs, BootBlockId id, string name);
};

}
