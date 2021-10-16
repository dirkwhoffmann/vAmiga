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
#include "IO.h"

const std::vector<string> EXTFile::extAdfHeaders =
{
    "UAE--ADF",
    "UAE-1ADF"
};

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
    debug(true, "EXTFILE::init\n");
    
    auto numTracks = disk.numTracks();
    
    size = 12;              // File header
    size += 12 * numTracks; // Track headers
    
    for (isize i = 0; i < numTracks; i++) {
        size += disk.length.track[i];
    }
    
    data = new u8[size]();
    
    printf("Allocated %zd bytes\n", size);
    
    decodeDisk(disk);
}

void
EXTFile::init(Drive &drive)
{
    if (drive.disk == nullptr) throw VAError(ERROR_DISK_MISSING);
    init(*drive.disk);
}

isize
EXTFile::readFromStream(std::istream &stream)
{
    throw VAError(ERROR_NO_EXTADF_SUPPORT);
}

void
EXTFile::decodeDisk(Disk &disk)
{
    assert(size);
    assert(data);
    
    u8 *p = data;
    auto numTracks = disk.numTracks();
    
    // Magic bytes
    std::strcpy((char *)p, "UAE-1ADF");

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
    
    printf("Wrote %zd bytes\n", p - data);
}
