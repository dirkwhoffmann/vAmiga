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
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "MutableFileSystem.h"
#include "IOUtils.h"

namespace vamiga {

const std::vector<string> EADFFile::extAdfHeaders =
{
    "UAE--ADF",
    "UAE-1ADF"
};

bool
EADFFile::isCompatible(const std::filesystem::path &path)
{
    return true;
}

bool
EADFFile::isCompatible(const u8 *buf, isize len)
{
    for (auto &header : extAdfHeaders) {

        if (util::matchingBufferHeader(buf, header)) return true;
    }
    return false;
}

bool
EADFFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
EADFFile::init(FloppyDisk &disk)
{
    auto numTracks = disk.numTracks();
    
    auto length = 0;

    length += 12;               // File header
    length += 12 * numTracks;   // Track headers
    
    for (isize i = 0; i < numTracks; i++) {
        length += disk.length.track[i];
    }
    
    data.init(length);
    decodeDisk(disk);
}

void
EADFFile::init(FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw Error(VAERROR_DISK_MISSING);
    init(*drive.disk);
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
    return adf ? adf.numSectors() : 0;
}

void
EADFFile::finalizeRead()
{
    isize numTracks = storedTracks();
    
    if (std::strcmp((char *)data.ptr, "UAE-1ADF") != 0) {
        
        warn("Only UAE-1ADF files are supported\n");
        throw Error(VAERROR_EXT_FACTOR5);
    }
    
    if (numTracks < 160 || numTracks > 168) {

        warn("Invalid number of tracks\n");
        throw Error(VAERROR_EXT_CORRUPTED);
    }

    if (data.size < proposedHeaderSize() || data.size != proposedFileSize()) {
        
        warn("File size mismatch\n");
        throw Error(VAERROR_EXT_CORRUPTED);
    }

    for (isize i = 0; i < numTracks; i++) {

        if (typeOfTrack(i) != 0 && typeOfTrack(i) != 1) {
            
            warn("Unsupported track format\n");
            throw Error(VAERROR_EXT_INCOMPATIBLE);
        }

        if (typeOfTrack(i) == 0) {

            if (usedBitsForTrack(i) != 11 * 512 * 8) {

                warn("Unsupported standard track size\n");
                throw Error(VAERROR_EXT_CORRUPTED);
            }
        }

        if (usedBitsForTrack(i) > availableBytesForTrack(i) * 8) {
            
            warn("Corrupted length information\n");
            throw Error(VAERROR_EXT_CORRUPTED);
        }

        if (usedBitsForTrack(i) % 8) {
            
            warn("Track length is not a multiple of 8\n");
            throw Error(VAERROR_EXT_INCOMPATIBLE);
        }
    }
    
    /* Try to convert the file to a standard ADF. The conversion will fail if
     * the extended ADF does not contain a standard Amiga disk.
     */
    try {

        // Convert the extended ADF to a disk
        auto disk = FloppyDisk(*this);

        // Convert the disk to a standard ADF
        adf.init(getDescriptor());
        adf.decodeDisk(disk);

    } catch (...) { }
}

FSVolumeType
EADFFile::getDos() const
{
    return adf ? adf.getDos() : FS_NODOS;
}

Diameter
EADFFile::getDiameter() const
{
    return INCH_35;
}

Density
EADFFile::getDensity() const
{
    isize bitsInLargestTrack = 0;
    
    for (isize i = 0; i < storedTracks(); i++) {
        bitsInLargestTrack = std::max(bitsInLargestTrack, usedBitsForTrack(i));
    }
    
    return bitsInLargestTrack < 16000 * 8 ? DENSITY_DD : DENSITY_HD;
}

void
EADFFile::encodeDisk(class FloppyDisk &disk) const
{
    assert(!data.empty());

    isize tracks = storedTracks();
    debug(MFM_DEBUG, "Encoding Amiga disk with %ld tracks\n", tracks);

    // Create an empty ADF
    ADFFile adf(getDescriptor());

    // Wipe out all data
    disk.clearDisk(0);

    // Encode all standard tracks
    for (Track t = 0; t < tracks; t++) encodeStandardTrack(adf, t);

    // Convert the ADF to a disk
    disk.encodeDisk(adf);

    // Encode all extended tracks
    for (Track t = 0; t < tracks; t++) encodeExtendedTrack(disk, t);
}

void
EADFFile::encodeStandardTrack(ADFFile &adf, Track t) const
{
    if (typeOfTrack(t) == 0) {

        debug(MFM_DEBUG, "Encoding standard track %ld\n", t);

        auto numBits = usedBitsForTrack(t);
        assert(numBits % 8 == 0);

        auto ptr = adf.data.ptr + t * 11 * 512;
        std::memcpy(ptr, trackData(t), size_t(numBits / 8));
    }
}

void
EADFFile::encodeExtendedTrack(class FloppyDisk &disk, Track t) const
{
    if (typeOfTrack(t) == 1) {

        debug(MFM_DEBUG, "Encoding extended track %ld\n", t);

        auto numBits = usedBitsForTrack(t);
        assert(numBits % 8 == 0);

        std::memcpy(disk.data.track[t], trackData(t), size_t(numBits / 8));
        disk.length.track[t] = i32(numBits / 8);
    }
}

void
EADFFile::decodeDisk(FloppyDisk &disk)
{
    assert(!data.empty());
    
    u8 *p = data.ptr;
    auto numTracks = disk.numTracks();
    
    // Magic bytes
    p[0] = 'U'; p[1] = 'A'; p[2] = 'E'; p[3] = '-';
    p[4] = '1'; p[5] = 'A'; p[6] = 'D'; p[7] = 'F';
    
    // Reserved
    assert(p[8] == 0);
    assert(p[9] == 0);
    
    // Number of tracks
    assert(p[10] == 0);
    p[11] = (u8)numTracks;
    
    p += 12;
    
    // Track headers
    for (Track t = 0; t < numTracks; t++, p += 12) {

        auto bytes = disk.length.track[t];
        auto bits = 8 * bytes;
        
        // Reserved
        assert(p[0] == 0);
        assert(p[1] == 0);

        // Type
        assert(p[2] == 0);
        p[3] = 1;
        
        // Track space in bytes
        p[4] = BYTE3(bytes);
        p[5] = BYTE2(bytes);
        p[6] = BYTE1(bytes);
        p[7] = BYTE0(bytes);

        // Track length in bits
        p[8] = BYTE3(bits);
        p[9] = BYTE2(bits);
        p[10] = BYTE1(bits);
        p[11] = BYTE0(bits);
    }
    
    // Track headers
    for (Track t = 0; t < numTracks; t++) {
        
        auto bytes = disk.length.track[t];
        
        for (isize i = 0; i < bytes; i++, p++) {
            *p = disk.data.track[t][i];
        }
    }
    
    debug(ADF_DEBUG, "Wrote %td bytes\n", p - data.ptr);
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
