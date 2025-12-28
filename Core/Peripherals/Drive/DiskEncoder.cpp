// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskEncoder.h"
#include "FloppyDisk.h"
#include "ADFFactory.h"
#include "IMGFactory.h"
#include "STFactory.h"
#include "CoreError.h"
#include "DeviceError.h"

namespace vamiga {

void
DiskEncoder::encodeTrack(MutableByteView track, Track t, span<const u8> src)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize ssize = 1088;                      // MFM sector size in bytes
    const isize count = (isize)src.size() / bsize; // Number of sectors to encode

    if (ADF_DEBUG) fprintf(stderr, "Encoding Amiga track %ld with %ld sectors\n", t, count);
    assert(src.size() % bsize == 0);

    // Format track
    track.clear(0xAA);

    // Encode all sectors
    for (Sector s = 0; s < count; s++)
        encodeSector(track, s * ssize, t, s, src.subspan(s * bsize, bsize));

    // Compute a debug checksum
    if (ADF_DEBUG) fprintf(stderr, "Track %ld checksum = %x\n", t, track.fnv32());
}

void
DiskEncoder::encodeSector(MutableByteView track, isize offset, Track t, Sector s, span<const u8> data)
{
    const isize bsize = 512;   // Block size in bytes
    const isize ssize = 1088;  // MFM sector size in bytes

    if (ADF_DEBUG) fprintf(stderr, "Encoding sector %ld\n", s);
    assert(data.size() == bsize);

    // Block header layout:
    //
    //                         Start  Size   Value
    //     Bytes before SYNC   00      4     0xAA 0xAA 0xAA 0xAA
    //     SYNC mark           04      4     0x44 0x89 0x44 0x89
    //     Track & sector info 08      8     Odd/Even encoded
    //     Unused area         16     32     0xAA
    //     Block checksum      48      8     Odd/Even encoded
    //     Data checksum       56      8     Odd/Even encoded

    auto it = track.cyclic_begin(offset);

    // Bytes before SYNC
    it[0] = (it[-1] & 1) ? 0x2A : 0xAA;
    it[1] = 0xAA;
    it[2] = 0xAA;
    it[3] = 0xAA;

    // SYNC mark
    u16 sync = 0x4489;
    it[4] = HI_BYTE(sync);
    it[5] = LO_BYTE(sync);
    it[6] = HI_BYTE(sync);
    it[7] = LO_BYTE(sync);

    // Track and sector information
    u8 info[4] = { 0xFF, (u8)t, (u8)s, (u8)(11 - s) };
    FloppyDisk::encodeOddEven(&it[8], info, sizeof(info));

    // Unused area
    for (isize i = 16; i < 48; i++)
        it[i] = 0xAA;

    // Data
    FloppyDisk::encodeOddEven(&it[64], data);

    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 8; i < 48; i += 4) {
        bcheck[0] ^= it[i];
        bcheck[1] ^= it[i+1];
        bcheck[2] ^= it[i+2];
        bcheck[3] ^= it[i+3];
    }
    FloppyDisk::encodeOddEven(&it[48], bcheck, sizeof(bcheck));

    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 64; i < ssize; i += 4) {
        dcheck[0] ^= it[i];
        dcheck[1] ^= it[i+1];
        dcheck[2] ^= it[i+2];
        dcheck[3] ^= it[i+3];
    }
    FloppyDisk::encodeOddEven(&it[56], dcheck, sizeof(bcheck));

    // Add clock bits
    for(isize i = 8; i < ssize + 1; i++) {
        it[i] = FloppyDisk::addClockBits(it[i], it[i-1]);
    }
}

void
DiskEncoder::decodeTrack(ByteView track, Track t, span<u8> dst)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize count = (isize)dst.size() / bsize; // Number of sectors to decode

    if (ADF_DEBUG) fprintf(stderr, "Decoding track %ld\n", t);
    assert(dst.size() % bsize == 0);

    // Seek all sync marks
    std::vector<isize> sectorStart(count);
    isize nr = 0; isize index = 0;
    auto it = track.cyclic_begin();

    while (index < track.size() + 8 && nr < count) {

        // Scan MFM stream for $4489 $4489
        if (it[index++] != 0x44) continue;
        if (it[index++] != 0x89) continue;
        if (it[index++] != 0x44) continue;
        if (it[index++] != 0x89) continue;

        // Make sure it's not a DOS track
        if (it[index + 1] == 0x89) continue;

        sectorStart[nr++] = index;
    }

    if (ADF_DEBUG) fprintf(stderr, "Found %ld sectors (expected %ld)\n", nr, count);

    if (nr != count) {

        warn("Found %ld sectors, expected %ld. Aborting.\n", nr, count);
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    // Decode all sectors
    for (Sector s = 0; s < count; s++)
        decodeSector(track, sectorStart[s], t, s, dst.subspan(s * bsize, bsize));
}

void
DiskEncoder::decodeSector(ByteView track, isize offset, Track t, Sector s, span<u8> dst)
{
    const isize bsize = 512;

    if (ADF_DEBUG) fprintf(stderr, "Decoding sector %ld\n", s);
    assert(dst.size() == bsize);

    // Decode sector info
    u8 info[4];
    FloppyDisk::decodeOddEven(info, &track[offset], 4);

    // Only proceed if the sector number is valid
    u8 sector = info[2];
    if (sector != s) {
        warn("Invalid sector number %d. Aborting.\n", sector);
        throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
    }

    // Skip sector header
    auto *data = &track[offset + 56];

    // Decode sector data
    FloppyDisk::decodeOddEven(dst.data(), data, bsize);
}

//
// ADF
//

void
DiskEncoder::encode(const ADFFile &adf, FloppyDisk &disk)
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
    for (Track t = 0; t < tracks; t++) {
        encodeTrack(disk.byteView(t), t, adf.byteView(t));
    }

    // In debug mode, also run the decoder
    if (ADF_DEBUG) {

        auto adf = ADFFactory::make(disk);
        string tmp = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        adf->writeToFile(tmp);
    }
}

void
DiskEncoder::decodeTrack(ADFFile &adf, const class FloppyDisk &disk, Track t)
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
DiskEncoder::decodeSector(ADFFile &adf, u8 *dst, const u8 *src)
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


void DiskEncoder::decode(ADFFile &adf, const class FloppyDisk &disk)
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
    // const_cast<FloppyDisk &>(disk).repeatTracks();

    // Decode all tracks
    // for (Track t = 0; t < tracks; t++) decodeTrack(adf, disk, t);
    for (Track t = 0; t < tracks; t++)
        decodeTrack(disk.byteView(t), t, adf.byteView(t));
}

//
// IMG
//

void
DiskEncoder::encode(const class IMGFile &img, FloppyDisk &disk)
{
    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    isize tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS disk with %ld tracks\n", tracks);

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(img, disk, t);

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        auto tmp = IMGFactory::make(disk);
        if (IMG_DEBUG) fprintf(stderr, "Saving image to /tmp/debug.img for debugging\n");
        tmp->writeToFile("/tmp/tmp.img");
    }
}

void
DiskEncoder::decode(class IMGFile &img, const FloppyDisk &disk)
{
    long tracks = img.numTracks();

    if (IMG_DEBUG) fprintf(stderr, "Decoding DOS disk (%ld tracks)\n", tracks);

    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Make the MFM stream scannable beyond the track end
    const_cast<FloppyDisk &>(disk).repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(img, disk, t);
}

void
DiskEncoder::encodeTrack(const IMGFile &img, FloppyDisk &disk, Track t)
{
    isize sectors = img.numSectors();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS track %ld with %ld sectors\n", t, sectors);

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
    for (Sector s = 0; s < sectors; s++) encodeSector(img, disk, t, s);

    // Compute a checksum for debugging
    if (IMG_DEBUG) fprintf(stderr, "Track %ld checksum = %llx\n", t, disk.checksum(t));
}

void
DiskEncoder::encodeSector(const IMGFile &img, FloppyDisk &disk, Track t, Sector s)
{
    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap

    if (IMG_DEBUG) fprintf(stderr, "  Encoding DOS sector %ld\n", s);

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
    u16 crc = Hashable::crc16(&buf[12], 8);
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
    img.readBlock(&buf[60], t, s);

    // Compute and write CRC
    crc = Hashable::crc16(&buf[56], 516);
    buf[572] = HI_BYTE(crc);
    buf[573] = LO_BYTE(crc);

    // Write GAP
    for (usize i = 574; i < sizeof(buf); i++) { buf[i] = 0x4E; }

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
DiskEncoder::decodeTrack(IMGFile &img, const class FloppyDisk &disk, Track t)
{
    assert(t < disk.numTracks());

    long numSectors = 9;
    auto *src = disk.data.track[t];
    auto *dst = img.data.ptr + t * numSectors * 512;

    if (IMG_DEBUG) fprintf(stderr, "Decoding DOS track %ld\n", t);

    // Determine the start of all sectors contained in this track
    std::vector <isize> sectorStart(numSectors);
    for (isize i = 0; i < numSectors; i++) {
        sectorStart[i] = 0;
    }
    isize cnt = 0;
    for (usize i = 0; i < sizeof(disk.data.track[t]) - 16;) {

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
        if (IMG_DEBUG) fprintf(stderr, "c: %d h: %d r: %d n: %d\n", chrn.c, chrn.h, chrn.r, chrn.n);

        if (chrn.r >= 1 && chrn.r <= numSectors) {

            // Break the loop once we see the same sector twice
            if (sectorStart[chrn.r - 1] != 0) {
                break;
            }
            sectorStart[chrn.r - 1] = i + 88;
            cnt++;

        } else {
            throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
        }
    }

    if (cnt != numSectors) {
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    // Do some consistency checking
    for (isize i = 0; i < numSectors; i++) assert(sectorStart[i] != 0);

    // Decode all sectors
    for (Sector s = 0; s < numSectors; s++, dst += 512) {
        decodeSector(img, dst, src + sectorStart[s]);
    }
}

void
DiskEncoder::decodeSector(IMGFile &img, u8 *dst, const u8 *src)
{
    FloppyDisk::decodeMFM(dst, src, 512);
}


//
// ST (TODO: REUSE IMG CODE)
//

void
DiskEncoder::encode(const class STFile &img, FloppyDisk &disk)
{
    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    isize tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS disk with %ld tracks\n", tracks);

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(img, disk, t);

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        auto tmp = IMGFactory::make(disk);
        if (IMG_DEBUG) fprintf(stderr, "Saving image to /tmp/debug.img for debugging\n");
        tmp->writeToFile("/tmp/tmp.img");
    }
}

void
DiskEncoder::decode(class STFile &img, const FloppyDisk &disk)
{
    long tracks = img.numTracks();

    if (IMG_DEBUG) fprintf(stderr, "Decoding Atari ST disk (%ld tracks)\n", tracks);

    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Make the MFM stream scannable beyond the track end
    const_cast<FloppyDisk &>(disk).repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(img, disk, t);
}

void
DiskEncoder::encodeTrack(const STFile &img, FloppyDisk &disk, Track t)
{
    isize sectors = img.numSectors();
    if (IMG_DEBUG) fprintf(stderr, "Encoding Atari ST track %ld with %ld sectors\n", t, sectors);

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
    for (Sector s = 0; s < sectors; s++) encodeSector(img, disk, t, s);

    // Compute a checksum for debugging
    if (IMG_DEBUG) fprintf(stderr, "Track %ld checksum = %llx\n", t, disk.checksum(t));
}

void
DiskEncoder::encodeSector(const STFile &img, FloppyDisk &disk, Track t, Sector s)
{
    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap

    if (IMG_DEBUG) fprintf(stderr, "  Encoding Atari ST sector %ld\n", s);

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
    u16 crc = Hashable::crc16(&buf[12], 8);
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
    img.readBlock(&buf[60], t, s);

    // Compute and write CRC
    crc = Hashable::crc16(&buf[56], 516);
    buf[572] = HI_BYTE(crc);
    buf[573] = LO_BYTE(crc);

    // Write GAP
    for (usize i = 574; i < sizeof(buf); i++) { buf[i] = 0x4E; }

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
DiskEncoder::decodeTrack(STFile &img, const class FloppyDisk &disk, Track t)
{
    assert(t < disk.numTracks());

    long numSectors = 9;
    auto *src = disk.data.track[t];
    auto *dst = img.data.ptr + t * numSectors * 512;

    if (IMG_DEBUG) fprintf(stderr, "Decoding Atari ST track %ld\n", t);

    // Determine the start of all sectors contained in this track
    std::vector <isize> sectorStart(numSectors);
    for (isize i = 0; i < numSectors; i++) {
        sectorStart[i] = 0;
    }
    isize cnt = 0;
    for (usize i = 0; i < sizeof(disk.data.track[t]) - 16;) {

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
        if (IMG_DEBUG) fprintf(stderr, "c: %d h: %d r: %d n: %d\n", chrn.c, chrn.h, chrn.r, chrn.n);

        if (chrn.r >= 1 && chrn.r <= numSectors) {

            // Break the loop once we see the same sector twice
            if (sectorStart[chrn.r - 1] != 0) {
                break;
            }
            sectorStart[chrn.r - 1] = i + 88;
            cnt++;

        } else {
            throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
        }
    }

    if (cnt != numSectors) {
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    // Do some consistency checking
    for (isize i = 0; i < numSectors; i++) assert(sectorStart[i] != 0);

    // Decode all sectors
    for (Sector s = 0; s < numSectors; s++, dst += 512) {
        decodeSector(img, dst, src + sectorStart[s]);
    }
}

void
DiskEncoder::decodeSector(STFile &img, u8 *dst, const u8 *src)
{
    FloppyDisk::decodeMFM(dst, src, 512);
}


}

