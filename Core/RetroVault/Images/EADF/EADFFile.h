// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ADFFile.h"

namespace retro::vault::image {

/* This class represents a file in the extended ADF format. The file layout is
 * organized as follows:
 *
 *   1. Header section:
 *
 *      8 bytes : ASCII signature "UAE-1ADF"
 *      2 bytes : Reserved
 *      2 bytes : Number of tracks (typically 2 × 80 = 160)
 *
 *   2. Track header section (one entry per track):
 *
 *      2 bytes : Reserved
 *      2 bytes : Track type
 *                0 = Standard AmigaDOS track
 *                1 = Raw MFM data (upper byte = number of disk revolutions − 1)
 *      4 bytes : Available space for the track, in bytes (must be even)
 *      4 bytes : Track length, in bits
 *
 *   3. Track data section:
 *
 *      Raw track data for each track, stored consecutively.
 *
 * Note:
 * There exists a related format identified by the header "UAE--ADF". This
 * variant was introduced by Factor 5 to distribute Turrican disk images and
 * appears to be the only known use of this format. It is not supported by the
 * emulator and will not be supported in the future.
 */

class EADFFile : public FloppyDiskImage {

    using MFMTrack = std::vector<u8>;
    mutable std::vector<MFMTrack> mfmTracks;

    // Accepted header signatures
    static const std::vector<string> extAdfHeaders;

public:

    static optional<ImageInfo> about(const fs::path &path);


    //
    // Initializing
    //

public:

    explicit EADFFile(isize len) { init(len); }
    explicit EADFFile(const fs::path &path) { init(path); }
    explicit EADFFile(const u8 *buf, isize len) { init(buf, len); }


    //
    // Methods from AnyImage
    //

public:

    bool validateURL(const fs::path& path) const noexcept override {
        return about(path).has_value();
    }
    
    ImageType type() const noexcept override { return ImageType::FLOPPY; }
    ImageFormat format() const noexcept override { return ImageFormat::EADF; }
    std::vector<string> describe() const noexcept override;
    
    void didLoad() override;


    //
    // Methods from DiskImage
    //

public:

    FSFamily fsFamily() const noexcept override { return FSFamily::AMIGA; }


    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return 512; }


    //
    // Methods from DiskImage
    //

public:

    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors(isize) const override { return numSectors(); }
    isize numSectors() const;


    //
    // Methods from FloppyDiskImage
    //

public:

    Diameter getDiameter() const noexcept override;
    Density getDensity() const noexcept override;

    BitView encode(TrackNr t) const override;
    void decode(TrackNr t, BitView bits) override;

private:

    MFMTrack& ensureMFMTrack(TrackNr t) const;
    BitView encodeStandardTrack(TrackNr t) const;
    BitView encodeExtendedTrack(TrackNr t) const;


    //
    // Scanning the raw data
    //
    
public:
    
    isize storedTracks() const;
    isize typeOfTrack(isize nr) const;
    isize availableBytesForTrack(isize nr) const;
    isize usedBitsForTrack(isize nr) const;
    isize proposedHeaderSize() const;
    isize proposedFileSize() const;
    
    // Returns a pointer to the first data byte of a certain track
    u8 *trackData(isize nr) const;
};

}
