// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EADFFile.h"
#include "ADFFactory.h"
#include "DiskEncoder.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "FileSystem.h"
#include "MediaError.h"
#include "utl/io.h"

namespace vamiga {

const std::vector<string> EADFFile::extAdfHeaders =
{
    "UAE--ADF",
    "UAE-1ADF"
};

bool
EADFFile::isCompatible(const fs::path &path)
{
    for (auto &header : extAdfHeaders) {

        if (utl::matchingFileHeader(path, header)) return true;
    }
    return false;
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
    return adf.empty() ? 0 : adf.numSectors();
}

void
EADFFile::finalizeRead()
{
    isize numTracks = storedTracks();
    
    if (std::strcmp((char *)data.ptr, "UAE-1ADF") != 0) {
        
        warn("Only UAE-1ADF files are supported\n");
        throw MediaError(MediaError::EXT_FACTOR5);
    }
    
    if (numTracks < 160 || numTracks > 168) {

        warn("Invalid number of tracks\n");
        throw MediaError(MediaError::EXT_CORRUPTED);
    }

    if (data.size < proposedHeaderSize() || data.size != proposedFileSize()) {
        
        warn("File size mismatch\n");
        throw MediaError(MediaError::EXT_CORRUPTED);
    }

    for (isize i = 0; i < numTracks; i++) {

        if (typeOfTrack(i) != 0 && typeOfTrack(i) != 1) {
            
            warn("Unsupported track format\n");
            throw MediaError(MediaError::EXT_INCOMPATIBLE);
        }

        if (typeOfTrack(i) == 0) {

            if (usedBitsForTrack(i) != 11 * 512 * 8) {

                warn("Unsupported standard track size\n");
                throw MediaError(MediaError::EXT_CORRUPTED);
            }
        }

        if (usedBitsForTrack(i) > availableBytesForTrack(i) * 8) {
            
            warn("Corrupted length information\n");
            throw MediaError(MediaError::EXT_CORRUPTED);
        }

        if (usedBitsForTrack(i) % 8) {
            
            warn("Truncating track (bit count is not a multiple of 8)\n");
            // throw MediaError(MediaError::EXT_INCOMPATIBLE);
            W32BE(data.ptr + 12 + 12 * i + 8, usedBitsForTrack(i) & ~7);
        }
    }
    
    /* Try to convert the file to a standard ADF. The conversion will fail if
     * the extended ADF does not contain a standard Amiga disk.
     */
    try {

        // Convert the extended ADF to a disk
        auto disk = FloppyDisk(*this);

        // Convert the disk to a standard ADF
        
        adf = *ADFFactory::make(numTracks * 11 * 512);
        disk.decode(adf);

    } catch (...) { }
}

FSFormat
EADFFile::getDos() const
{
    return adf.empty() ? FSFormat::NODOS : adf.getDos();
}

Diameter
EADFFile::getDiameter() const
{
    return Diameter::INCH_35;
}

Density
EADFFile::getDensity() const
{
    isize bitsInLargestTrack = 0;
    
    for (isize i = 0; i < storedTracks(); i++) {
        bitsInLargestTrack = std::max(bitsInLargestTrack, usedBitsForTrack(i));
    }
    
    return bitsInLargestTrack < 16000 * 8 ? Density::DD : Density::HD;
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
