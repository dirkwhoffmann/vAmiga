// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EADFFile.h"
#include "DiskEncoder.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "AmigaEncoder.h"
#include "FileSystems/Amiga/FileSystem.h"
#include "ImageError.h"
#include "utl/io.h"

namespace retro::vault::image {

using vamiga::FloppyDisk;

const std::vector<string> EADFFile::extAdfHeaders =
{
    "UAE--ADF",
    "UAE-1ADF"
};

optional<ImageInfo>
EADFFile::about(const fs::path &path)
{
    for (auto &header : extAdfHeaders) {

        if (utl::matchingFileHeader(path, header))
            return {{ ImageType::FLOPPY, ImageFormat::EADF }};
    }
    return {};
}

isize
EADFFile::numCyls() const
{
    return (storedTracks() + 1) / 2;
}

isize
EADFFile::numHeads() const
{
    return 2;
}

isize
EADFFile::numSectors() const
{
    switch (getDensity()) {

        case Density::DD:   return 11;
        case Density::HD:   return 22;

        default:
            return 0;
    }
}

std::vector<string>
EADFFile::describe() const noexcept
{
    return {
        "Amiga Floppy Disk",
        "Extended ADF format",
        ""
    };
}

void
EADFFile::didLoad()
{
    isize numTracks = storedTracks();
    
    if (std::strcmp((char *)data.ptr, "UAE-1ADF") != 0) {
        
        logwarn("Only UAE-1ADF files are supported\n");
        throw ImageError(ImageError::EXT_FACTOR5);
    }
    
    if (numTracks < 160 || numTracks > 168) {

        logwarn("Invalid number of tracks\n");
        throw ImageError(ImageError::EXT_CORRUPTED);
    }

    if (data.size < proposedHeaderSize() || data.size != proposedFileSize()) {
        
        logwarn("File size mismatch\n");
        throw ImageError(ImageError::EXT_CORRUPTED);
    }

    for (isize i = 0; i < numTracks; i++) {

        if (typeOfTrack(i) != 0 && typeOfTrack(i) != 1) {
            
            logwarn("Unsupported track format\n");
            throw ImageError(ImageError::EXT_INCOMPATIBLE);
        }

        if (typeOfTrack(i) == 0) {

            if (usedBitsForTrack(i) != 11 * 512 * 8) {

                logwarn("Unsupported standard track size\n");
                throw ImageError(ImageError::EXT_CORRUPTED);
            }
        }

        if (usedBitsForTrack(i) > availableBytesForTrack(i) * 8) {
            
            logwarn("Corrupted length information\n");
            throw ImageError(ImageError::EXT_CORRUPTED);
        }
    }

    mfmTracks.resize(numTracks);
}

Diameter
EADFFile::getDiameter() const noexcept
{
    return Diameter::INCH_35;
}

Density
EADFFile::getDensity() const noexcept
{
    isize bitsInLargestTrack = 0;
    
    for (isize i = 0; i < storedTracks(); i++) {
        bitsInLargestTrack = std::max(bitsInLargestTrack, usedBitsForTrack(i));
    }
    
    return bitsInLargestTrack < 16000 * 8 ? Density::DD : Density::HD;
}

BitView
EADFFile::encode(TrackNr t) const
{
    switch (typeOfTrack(t)) {

        case 0: return encodeStandardTrack(t);
        case 1: return encodeExtendedTrack(t);

        default:
            throw ImageError(ImageError::EXT_CORRUPTED,
                             "Invalid track type: " + std::to_string(t));
    }
}

EADFFile::MFMTrack &
EADFFile::ensureMFMTrack(TrackNr t) const
{
    static constexpr isize MFMSectorSize = 1088;
    static constexpr isize MFMTrackBytes = 22 * MFMSectorSize;

    validateTrackNr(t);
    assert(t < isize(mfmTracks.size()));

    // Access the MFM data cache
    auto &track = mfmTracks.at(t);

    // Resize if necessary
    if (isize(track.size()) != MFMTrackBytes) track.resize(MFMTrackBytes);

    return track;
}

BitView
EADFFile::encodeStandardTrack(TrackNr t) const
{
    loginfo(MFM_DEBUG, "Encoding standard track %ld\n", t);

    auto &track = ensureMFMTrack(t);

    // Create views
    auto dataByteView = ByteView(trackData(t), availableBytesForTrack(t));
    auto mfmByteView  = MutableByteView(track.data(), track.size());

    // Encode the track
    Encoder::amiga.encodeTrack(mfmByteView, t, dataByteView);

    // Return a bit view for the cached MFM data
    return BitView(mfmByteView.data(), usedBitsForTrack(t));
}

BitView
EADFFile::encodeExtendedTrack(TrackNr t) const
{
    loginfo(MFM_DEBUG, "Encoding extended track %ld\n", t);
    validateTrackNr(t);

    return BitView(trackData(t), usedBitsForTrack(t));
}

void
EADFFile::decode(TrackNr t, BitView bits)
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

isize
EADFFile::storedTracks() const
{
    assert(!data.empty());

    return HI_LO(data[10], data[11]);
}

isize
EADFFile::typeOfTrack(isize nr) const
{
    assert(!data.empty());
    
    u8 *p = data.ptr + 12 + 12 * nr + 2;
    return HI_LO(p[0], p[1]);
}

isize
EADFFile::availableBytesForTrack(isize nr) const
{
    assert(!data.empty());
    
    u8 *p = data.ptr + 12 + 12 * nr + 4;
    return HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
}

isize
EADFFile::usedBitsForTrack(isize nr) const
{
    assert(!data.empty());
    
    u8 *p = data.ptr + 12 + 12 * nr + 8;
    return HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
}

isize
EADFFile::proposedHeaderSize() const
{
    assert(!data.empty());
    
    return 12 + 12 * storedTracks();
}

isize
EADFFile::proposedFileSize() const
{
    assert(!data.empty());

    isize result = proposedHeaderSize();
    
    for (isize i = 0; i < storedTracks(); i++) {
        result += availableBytesForTrack(i);
    }
    
    return result;
}

u8 *
EADFFile::trackData(isize nr) const
{
    assert(!data.empty());
    
    u8 *p = data.ptr + proposedHeaderSize();
    
    for (isize i = 0; i < nr; i++) {
        p += availableBytesForTrack(i);
    }
    
    return p;
}

}
