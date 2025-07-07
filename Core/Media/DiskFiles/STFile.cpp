// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "STFile.h"
#include "Checksum.h"
#include "FloppyDisk.h"
#include "IOUtils.h"

namespace vamiga {

bool
STFile::isCompatible(const fs::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".ST";
}

bool
STFile::isCompatible(const u8 *buf, isize len)
{
    // There are no magic bytes. We can only check the buffer size
    return len == STSIZE_35_DD;
}

bool
STFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
STFile::init(Diameter dia, Density den)
{
    // We only support 3.5"DD disks at the moment
    if (dia == Diameter::INCH_35 && den == Density::DD) {

        data.init(9 * 160 * 512);

    } else {

        throw AppError(Fault::DISK_INVALID_LAYOUT);
    }
}

void
STFile::init(FloppyDisk &disk)
{
    init(Diameter::INCH_35, Density::DD);
    decodeDisk(disk);
}

isize
STFile::numCyls() const
{
    return 80;
}

isize
STFile::numHeads() const
{
    return 2;
}

isize
STFile::numSectors() const
{
    return 9;
}

void
STFile::encodeDisk(FloppyDisk &disk) const
{
    if (disk.getDiameter() != getDiameter()) {
        throw AppError(Fault::DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDensity()) {
        throw AppError(Fault::DISK_INVALID_DENSITY);
    }

    isize tracks = numTracks();
    debug(IMG_DEBUG, "Encoding AtariST disk with %ld tracks\n", tracks);

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(disk, t);

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        STFile tmp(disk);
        debug(IMG_DEBUG, "Saving image to /tmp/debug.img for debugging\n");
        tmp.writeToFile("/tmp/tmp.img");
    }
}

void
STFile::encodeTrack(FloppyDisk &disk, Track t) const
{
    isize sectors = numSectors();
    debug(IMG_DEBUG, "Encoding AtariST track %ld with %ld sectors\n", t, sectors);

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
    debug(IMG_DEBUG, "Track %ld checksum = %llx\n", t, disk.checksum(t));
}

void
STFile::encodeSector(FloppyDisk &disk, Track t, Sector s) const
{
    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap

    debug(IMG_DEBUG, "  Encoding AtariST sector %ld\n", s);

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
    u8 *p = disk.data.track[t] + 194 + s * 2 * sizeof(buf);
    // u8 *p = disk.data.track[t] + 194 + s * 1310;
    debug(IMG_DEBUG, "  Range: %ld - %lu / %d\n", 
          isize(p - disk.data.track[t]), isize(p - disk.data.track[t] + 2*sizeof(buf)), disk.length.track[t]);

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
STFile::decodeDisk(const FloppyDisk &disk)
{
    long tracks = numTracks();

    debug(IMG_DEBUG, "Decoding DOS disk (%ld tracks)\n", tracks);

    if (disk.getDiameter() != getDiameter()) {
        throw AppError(Fault::DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDensity()) {
        throw AppError(Fault::DISK_INVALID_DENSITY);
    }

    // Make a copy of the disk which can modify
    auto diskCopy = disk;

    // Make the MFM stream scannable beyond the track end
    // TODO: THINK ABOUT OF DECODING WITHOUT MODIFYING THE DISK
    const_cast<FloppyDisk &>(disk).repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(disk, t);
}

void
STFile::decodeTrack(const FloppyDisk &disk, Track t)
{
    assert(t < disk.numTracks());

    long numSectors = 9;
    auto *src = disk.data.track[t];
    auto *dst = data.ptr + t * numSectors * 512;

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
            throw AppError(Fault::DISK_INVALID_SECTOR_NUMBER);
        }
    }

    if (cnt != numSectors) {
        throw AppError(Fault::DISK_WRONG_SECTOR_COUNT);
    }

    // Do some consistency checking
    for (isize i = 0; i < numSectors; i++) assert(sectorStart[i] != 0);

    // Decode all sectors
    for (Sector s = 0; s < numSectors; s++, dst += 512) {
        decodeSector(dst, src + sectorStart[s]);
    }
}

void
STFile::decodeSector(u8 *dst, const u8 *src)
{
    FloppyDisk::decodeMFM(dst, src, 512);
}

}
