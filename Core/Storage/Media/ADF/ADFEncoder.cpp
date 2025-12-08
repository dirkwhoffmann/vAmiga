// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADFEncoder.h"
#include "ADFFactory.h"
#include "DeviceError.h"

namespace vamiga {

void
ADFEncoder::encode(const ADFFile &adf, FloppyDisk &disk)
{
    if (disk.getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    isize tracks = adf.numTracks();
    if (ADF_DEBUG) fprintf(stderr, "Encoding Amiga disk with %ld tracks\n", tracks);

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(adf, disk, t);

    // In debug mode, also run the decoder
    if (ADF_DEBUG) {

        auto tmp = ADFFactory::make(disk);
        string name = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", name.c_str());
        tmp->writeToFile(name);
    }
}

void
ADFEncoder::encodeTrack(const ADFFile &adf, FloppyDisk &disk, Track t)
{
    isize sectors = adf.numSectors();
    if (ADF_DEBUG) fprintf(stderr, "Encoding Amiga track %ld with %ld sectors\n", t, sectors);

    // Format track
    disk.clearTrack(t, 0xAA);

    // Encode all sectors
    for (Sector s = 0; s < sectors; s++) encodeSector(adf, disk, t, s);

    // Rectify the first clock bit (where the buffer wraps over)
    if (disk.readBit(t, disk.length.track[t] * 8 - 1)) {
        disk.writeBit(t, 0, 0);
    }

    // Compute a debug checksum
    if (ADF_DEBUG) fprintf(stderr, "Track %ld checksum = %x\n",
                           t, Hashable::fnv32(disk.data.track[t], disk.length.track[t]));
}

void
ADFEncoder::encodeSector(const ADFFile &adf, FloppyDisk &disk, Track t, Sector s)
{
    assert(t < disk.numTracks());

    if (ADF_DEBUG) fprintf(stderr, "Encoding sector %ld\n", s);

    // Block header layout:
    //
    //                         Start  Size   Value
    //     Bytes before SYNC   00      4     0xAA 0xAA 0xAA 0xAA
    //     SYNC mark           04      4     0x44 0x89 0x44 0x89
    //     Track & sector info 08      8     Odd/Even encoded
    //     Unused area         16     32     0xAA
    //     Block checksum      48      8     Odd/Even encoded
    //     Data checksum       56      8     Odd/Even encoded

    // Determine the start of this sector
    u8 *p = disk.data.track[t] + (s * 1088);

    // Bytes before SYNC
    p[0] = (p[-1] & 1) ? 0x2A : 0xAA;
    p[1] = 0xAA;
    p[2] = 0xAA;
    p[3] = 0xAA;

    // SYNC mark
    u16 sync = 0x4489;
    p[4] = HI_BYTE(sync);
    p[5] = LO_BYTE(sync);
    p[6] = HI_BYTE(sync);
    p[7] = LO_BYTE(sync);

    // Track and sector information
    u8 info[4] = { 0xFF, (u8)t, (u8)s, (u8)(11 - s) };
    FloppyDisk::encodeOddEven(&p[8], info, sizeof(info));

    // Unused area
    for (isize i = 16; i < 48; i++)
        p[i] = 0xAA;

    // Data
    u8 bytes[512];
    adf.readSector(bytes, t, s);
    FloppyDisk::encodeOddEven(&p[64], bytes, sizeof(bytes));

    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    FloppyDisk::encodeOddEven(&p[48], bcheck, sizeof(bcheck));

    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    FloppyDisk::encodeOddEven(&p[56], dcheck, sizeof(bcheck));

    // Add clock bits
    for(isize i = 8; i < 1088; i++) {
        p[i] = FloppyDisk::addClockBits(p[i], p[i-1]);
    }
}

void
ADFEncoder::decode(ADFFile &adf, const class FloppyDisk &disk)
{
    if (ADF_DEBUG) fprintf(stderr, "ADFFile::decodeDisk\n");
    long tracks = adf.numTracks();

    if (ADF_DEBUG) fprintf(stderr, "Decoding Amiga disk with %ld tracks\n", tracks);

    if (disk.getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Make the MFM stream scannable beyond the track end
    // TODO: THINK ABOUT OF DECODING WITHOUT MODIFYING THE DISK
    const_cast<FloppyDisk &>(disk).repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(adf, disk, t);
}

void
ADFEncoder::decodeTrack(ADFFile &adf, const class FloppyDisk &disk, Track t)
{
    long sectors = adf.numSectors();

    if (ADF_DEBUG) fprintf(stderr, "Decoding track %ld\n", t);

    auto *src = disk.data.track[t];
    auto *dst = adf.data.ptr + t * sectors * 512;

    // Seek all sync marks
    std::vector<isize> sectorStart(sectors);
    isize nr = 0; usize index = 0;

    while (index < sizeof(disk.data.track[t]) && nr < sectors) {

        // Scan MFM stream for $4489 $4489
        if (src[index++] != 0x44) continue;
        if (src[index++] != 0x89) continue;
        if (src[index++] != 0x44) continue;
        if (src[index++] != 0x89) continue;

        // Make sure it's not a DOS track
        if (src[index + 1] == 0x89) continue;

        sectorStart[nr++] = index;
    }

    if (ADF_DEBUG) fprintf(stderr, "Found %ld sectors (expected %ld)\n", nr, sectors);

    if (nr != sectors) {

        warn("Found %ld sectors, expected %ld. Aborting.\n", nr, sectors);
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    // Decode all sectors
    for (Sector s = 0; s < sectors; s++) {
        decodeSector(adf, dst, src + sectorStart[s]);
    }
}

void
ADFEncoder::decodeSector(ADFFile &adf, u8 *dst, const u8 *src)
{
    assert(dst != nullptr);
    assert(src != nullptr);

    // Decode sector info
    u8 info[4];
    FloppyDisk::decodeOddEven(info, src, 4);

    // Only proceed if the sector number is valid
    u8 sector = info[2];
    if (sector >= adf.numSectors()) {
        warn("Invalid sector number %d. Aborting.\n", sector);
        throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
    }

    // Skip sector header
    src += 56;

    // Decode sector data
    FloppyDisk::decodeOddEven(dst + sector * 512, src, 512);
}

}
