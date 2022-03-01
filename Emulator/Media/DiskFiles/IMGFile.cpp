// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IMGFile.h"
#include "Checksum.h"
#include "FloppyDisk.h"
#include "IOUtils.h"

bool
IMGFile::isCompatible(const string &path)
{
    auto suffix = util::uppercased(util::extractSuffix(path));
    return suffix == "IMG";
}

bool
IMGFile::isCompatible(std::istream &stream)
{
    isize length = util::streamLength(stream);
    
    // There are no magic bytes. We can only check the buffer size
    return length == IMGSIZE_35_DD;
}

void
IMGFile::init(Diameter dia, Density den)
{
    // We only support 3.5"DD disks at the moment
    if (dia == INCH_35 && den == DENSITY_DD) {

        size = 9 * 160 * 512;
        data = new u8[size]();

    } else {

        throw VAError(ERROR_DISK_INVALID_LAYOUT);
    }
}

void
IMGFile::init(FloppyDisk &disk)
{
    init(INCH_35, DENSITY_DD);
    decodeDisk(disk);
}

isize
IMGFile::numCyls() const
{
    return 80;
}

isize
IMGFile::numHeads() const
{
    return 2;
}

isize
IMGFile::numSectors() const
{
    return 9;
}

void
IMGFile::encodeDisk(FloppyDisk &disk) const
{
    if (disk.getDiameter() != getDiameter()) {
        throw VAError(ERROR_DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDensity()) {
        throw VAError(ERROR_DISK_INVALID_DENSITY);
    }

    isize tracks = numTracks();
    debug(IMG_DEBUG, "Encoding DOS disk with %ld tracks\n", tracks);

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(disk, t);

    // In debug mode, also run the decoder
    if constexpr (IMG_DEBUG) {
        
        IMGFile tmp(disk);
        debug(IMG_DEBUG, "Saving image to /tmp/debug.img for debugging\n");
        tmp.writeToFile("/tmp/tmp.img");
    }
}

void
IMGFile::encodeTrack(FloppyDisk &disk, Track t) const
{
    isize sectors = numSectors();
    debug(IMG_DEBUG, "Encoding DOS track %ld with %ld sectors\n", t, sectors);

    u8 *p = disk.data.track[t];

    // Clear track
    disk.clearTrack(t, 0x92, 0x54);

    // Encode track header
    p += 82;                                        // GAP
    for (isize i = 0; i < 24; i++) { p[i] = 0xAA; } // SYNC
    p += 24;
    p[0] = 0x52; p[1] = 0x24;                       // IAM
    p[2] = 0x52; p[3] = 0x24;
    p[4] = 0x52; p[5] = 0x24;
    p[6] = 0x55; p[7] = 0x52;
    p += 8;
    p += 80;                                        // GAP
        
    // Encode all sectors
    for (Sector s = 0; s < sectors; s++) encodeSector(disk, t, s);
    
    // Compute a checksum for debugging
    debug(IMG_DEBUG, "Track %ld checksum = %x\n",
          t, util::fnv_1a_32(disk.data.track[t], disk.length.track[t]));
}

void
IMGFile::encodeSector(FloppyDisk &disk, Track t, Sector s) const
{
    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap
        
    debug(IMG_DEBUG, "  Encoding DOS sector %ld\n", s);
    
    // Write SYNC
    for (isize i = 0; i < 12; i++) { buf[i] = 0x00; }
    
    // Write IDAM
    buf[12] = 0xA1;
    buf[13] = 0xA1;
    buf[14] = 0xA1;
    buf[15] = 0xFE;
    
    // Write CHRN
    buf[16] = (u8)(t / 2);
    buf[17] = (u8)(t % 2);
    buf[18] = (u8)(s + 1);
    buf[19] = 2;
    
    // Compute and write CRC
    u16 crc = util::crc16(&buf[12], 8);
    buf[20] = HI_BYTE(crc);
    buf[21] = LO_BYTE(crc);

    // Write GAP
    for (isize i = 22; i < 44; i++) { buf[i] = 0x4E; }

    // Write SYNC
    for (isize i = 44; i < 56; i++) { buf[i] = 0x00; }

    // Write DATA AM
    buf[56] = 0xA1;
    buf[57] = 0xA1;
    buf[58] = 0xA1;
    buf[59] = 0xFB;

    // Write DATA
    readSector(&buf[60], t, s);
    
    // Compute and write CRC
    crc = util::crc16(&buf[56], 516);
    buf[572] = HI_BYTE(crc);
    buf[573] = LO_BYTE(crc);

    // Write GAP
    for (isize i = 574; i < isizeof(buf); i++) { buf[i] = 0x4E; }

    // Determine the start of this sector
    u8 *p = disk.data.track[t] + 194 + s * 1300;

    // Create the MFM data stream
    FloppyDisk::encodeMFM(p, buf, sizeof(buf));
    FloppyDisk::addClockBits(p, 2 * sizeof(buf));
    
    // Remove certain clock bits in IDAM block
    p[2*12+1] &= 0xDF;
    p[2*13+1] &= 0xDF;
    p[2*14+1] &= 0xDF;
    
    // Remove certain clock bits in DATA AM block
    p[2*56+1] &= 0xDF;
    p[2*57+1] &= 0xDF;
    p[2*58+1] &= 0xDF;
}

void
IMGFile::decodeDisk(FloppyDisk &disk)
{
    long tracks = numTracks();
    
    debug(IMG_DEBUG, "Decoding DOS disk (%ld tracks)\n", tracks);
    
    if (disk.getDiameter() != getDiameter()) {
        throw VAError(ERROR_DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDensity()) {
        throw VAError(ERROR_DISK_INVALID_DENSITY);
    }
    
    // Make the MFM stream scannable beyond the track end
    disk.repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(disk, t);
}

void
IMGFile::decodeTrack(FloppyDisk &disk, Track t)
{
    assert(t < disk.numTracks());
        
    long numSectors = 9;
    u8 *src = disk.data.track[t];
    u8 *dst = data + t * numSectors * 512;
    
    debug(IMG_DEBUG, "Decoding DOS track %ld\n", t);

    // Determine the start of all sectors contained in this track
    std::vector <isize> sectorStart(numSectors);
    for (isize i = 0; i < numSectors; i++) {
        sectorStart[i] = 0;
    }
    isize cnt = 0;
    for (isize i = 0; i < isizeof(disk.data.track[t]) - 16;) {
        
        // Seek IDAM block
        if (src[i++] != 0x44) continue;
        if (src[i++] != 0x89) continue;
        if (src[i++] != 0x44) continue;
        if (src[i++] != 0x89) continue;
        if (src[i++] != 0x44) continue;
        if (src[i++] != 0x89) continue;
        if (src[i++] != 0x55) continue;
        if (src[i++] != 0x54) continue;

        // Decode CHRN block
        struct { u8 c; u8 h; u8 r; u8 n; } chrn;
        FloppyDisk::decodeMFM((u8 *)&chrn, &src[i], 4);
        debug(IMG_DEBUG, "c: %d h: %d r: %d n: %d\n", chrn.c, chrn.h, chrn.r, chrn.n);
        
        if (chrn.r >= 1 && chrn.r <= numSectors) {
            
            // Break the loop once we see the same sector twice
            if (sectorStart[chrn.r - 1] != 0) {
                break;
            }
            sectorStart[chrn.r - 1] = i + 88;
            cnt++;

        } else {
            warn("Invalid sector number %d. Aborting.\n", chrn.r);
            throw VAError(ERROR_DISK_INVALID_SECTOR_NUMBER);
        }
    }

    if (cnt != numSectors) {
        warn("Found %ld sectors, expected %ld. Aborting.\n", cnt, numSectors);
        throw VAError(ERROR_DISK_WRONG_SECTOR_COUNT);
    }
        
    // Do some consistency checking
    for (isize i = 0; i < numSectors; i++) assert(sectorStart[i] != 0);
    
    // Decode all sectors
    for (Sector s = 0; s < numSectors; s++, dst += 512) {
        decodeSector(dst, src + sectorStart[s]);
    }
}

void
IMGFile::decodeSector(u8 *dst, u8 *src)
{
    FloppyDisk::decodeMFM(dst, src, 512);
}
