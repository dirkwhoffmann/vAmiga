// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EXTFile.h"
#include "Disk.h"
#include "Drive.h"
#include "FSDevice.h"
#include "IOUtils.h"

const std::vector<string> EXTFile::extAdfHeaders =
{
    "UAE--ADF",
    "UAE-1ADF"
};

EXTFile::~EXTFile()
{
    if (adf) delete adf;
}

bool
EXTFile::isCompatible(const string &path)
{
    return true;
}

bool
EXTFile::isCompatible(std::istream &stream)
{
    for (auto &header : extAdfHeaders) {
        if (util::matchingStreamHeader(stream, header)) return true;
    }

    return false;
}

void
EXTFile::init(Disk &disk)
{
    auto numTracks = disk.numTracks();
    
    size = 12;              // File header
    size += 12 * numTracks; // Track headers
    
    for (isize i = 0; i < numTracks; i++) {
        size += disk.length.track[i];
    }
    
    data = new u8[size]();
        
    decodeDisk(disk);
}

void
EXTFile::init(Drive &drive)
{
    if (drive.disk == nullptr) throw VAError(ERROR_DISK_MISSING);
    init(*drive.disk);
}

void
EXTFile::finalizeRead()
{
    isize numTracks = storedTracks();
    
    if (std::strcmp((char *)data, "UAE-1ADF") != 0) {
        
        warn("UAE-1ADF files are not supported\n");
        throw VAError(ERROR_EXT_FACTOR5);
    }
    
    if (numTracks < 160 || numTracks > 168) {

        warn("Invalid number of tracks\n");
        throw VAError(ERROR_EXT_CORRUPTED);
    }

    if (size < proposedHeaderSize() || size != proposedFileSize()) {
        
        warn("File size mismatch\n");
        throw VAError(ERROR_EXT_CORRUPTED);
    }

    for (isize i = 0; i < numTracks; i++) {
        
        if (typeOfTrack(i) != 1) {
            
            warn("Only MFM encoded tracks are supported yet\n");
            throw VAError(ERROR_EXT_INCOMPATIBLE);
        }

        if (usedBitsForTrack(i) > availableBytesForTrack(i) * 8) {
            
            warn("Corrupted length information\n");
            throw VAError(ERROR_EXT_CORRUPTED);
        }

        if (usedBitsForTrack(i) % 8) {
            
            warn("Track length is not a multiple of 8\n");
            throw VAError(ERROR_EXT_INCOMPATIBLE);
        }
    }
    
    /* Try to convert the file to a standard ADF. The conversion will fail if
     * the extended ADF does not contain a standard Amiga disk.
     */
    try {
                
        // Convert the extended ADF to a disk
        auto disk = Disk(*this);

        // Convert the disk to a standard ADF
        adf = new ADFFile(disk);
        
    } catch (...) { }    
}

FSVolumeType
EXTFile::getDos() const
{
    return adf ? adf->getDos() : FS_NODOS;
}

DiskDiameter
EXTFile::getDiskDiameter() const
{
    return INCH_35;
}

DiskDensity
EXTFile::getDiskDensity() const
{
    isize bitsInLargestTrack = 0;
    
    for (isize i = 0; i < storedTracks(); i++) {
        bitsInLargestTrack = std::max(bitsInLargestTrack, usedBitsForTrack(i));
    }
    
    return bitsInLargestTrack < 16000 * 8 ? DISK_DD : DISK_HD;
}

isize
EXTFile::numSides() const
{
    return 2;
}

isize
EXTFile::numCyls() const
{
   return (storedTracks() + 1) / 2;
}

isize
EXTFile::numTracks() const
{
    return storedTracks();
}

isize
EXTFile::numSectors() const
{
    return adf ? adf->numSectors() : 0;
}

void
EXTFile::encodeDisk(class Disk &disk) const
{
    assert(size);
    assert(data);
    
    isize tracks = storedTracks();
    debug(MFM_DEBUG, "Encoding Amiga disk with %ld tracks\n", tracks);

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(disk, t);
}

void
EXTFile::encodeTrack(class Disk &disk, Track t) const
{
    auto numTracks = storedTracks();
    
    for (isize i = 0; i < numTracks; i++) {

        debug(MFM_DEBUG, "Encoding track %ld\n", i);
        
        auto numBits = usedBitsForTrack(i);
        assert(numBits % 8 == 0);
        
        std::memcpy(disk.data.track[i], trackData(i), numBits / 8);
        disk.length.track[i] = (i32)(numBits / 8);
    }
}

void
EXTFile::decodeDisk(Disk &disk)
{
    assert(size);
    assert(data);
    
    u8 *p = data;
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
    
    debug(ADF_DEBUG, "Wrote %td bytes\n", p - data);
}

isize
EXTFile::storedTracks() const
{
    assert(data);

    return HI_LO(data[10], data[11]);
}

isize
EXTFile::typeOfTrack(isize nr) const
{
    assert(data);
    
    u8 *p = data + 12 + 12 * nr + 2;
    return HI_LO(p[0], p[1]);
}

isize
EXTFile::availableBytesForTrack(isize nr) const
{
    assert(data);
    
    u8 *p = data + 12 + 12 * nr + 4;
    return HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
}

isize
EXTFile::usedBitsForTrack(isize nr) const
{
    assert(data);
    
    u8 *p = data + 12 + 12 * nr + 8;
    return HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
}

isize
EXTFile::proposedHeaderSize() const
{
    assert(data);
    
    return 12 + 12 * storedTracks();
}

isize
EXTFile::proposedFileSize() const
{
    assert(data);

    isize result = proposedHeaderSize();
    
    for (isize i = 0; i < storedTracks(); i++) {
        result += availableBytesForTrack(i);
    }
    
    return result;
}

u8 *
EXTFile::trackData(isize nr) const
{
    assert(data);
    
    u8 *p = data + proposedHeaderSize();
    
    for (isize i = 0; i < nr; i++) {
        p += availableBytesForTrack(i);
    }
    
    return p;
}
