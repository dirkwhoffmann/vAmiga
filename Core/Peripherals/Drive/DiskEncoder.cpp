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
DiskEncoder::encodeMFM(u8 *dst, const u8 *src, isize count)
{
    for(isize i = 0; i < count; i++) {

        auto mfm =
        ((src[i] & 0b10000000) << 7) |
        ((src[i] & 0b01000000) << 6) |
        ((src[i] & 0b00100000) << 5) |
        ((src[i] & 0b00010000) << 4) |
        ((src[i] & 0b00001000) << 3) |
        ((src[i] & 0b00000100) << 2) |
        ((src[i] & 0b00000010) << 1) |
        ((src[i] & 0b00000001) << 0);

        dst[2*i+0] = HI_BYTE(mfm);
        dst[2*i+1] = LO_BYTE(mfm);
    }
}

void
DiskEncoder::decodeMFM(u8 *dst, const u8 *src, isize count)
{
    for(isize i = 0; i < count; i++) {

        u16 mfm = HI_LO(src[2*i], src[2*i+1]);

        auto decoded =
        ((mfm & 0b0100000000000000) >> 7) |
        ((mfm & 0b0001000000000000) >> 6) |
        ((mfm & 0b0000010000000000) >> 5) |
        ((mfm & 0b0000000100000000) >> 4) |
        ((mfm & 0b0000000001000000) >> 3) |
        ((mfm & 0b0000000000010000) >> 2) |
        ((mfm & 0b0000000000000100) >> 1) |
        ((mfm & 0b0000000000000001) >> 0);

        dst[i] = (u8)decoded;
    }
}

void
DiskEncoder::encodeOddEven(u8 *dst, const u8 *src, isize count)
{
    // Encode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (src[i] >> 1) & 0x55;

    // Encode even bits
    for(isize i = 0; i < count; i++)
        dst[i + count] = src[i] & 0x55;
}

void
DiskEncoder::decodeOddEven(u8 *dst, const u8 *src, isize count)
{
    // Decode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (u8)((src[i] & 0x55) << 1);

    // Decode even bits
    for(isize i = 0; i < count; i++)
        dst[i] |= src[i + count] & 0x55;
}

void
DiskEncoder::addClockBits(u8 *dst, isize count)
{
    for (isize i = 0; i < count; i++) {
        dst[i] = addClockBits(dst[i], dst[i-1]);
    }
}

u8
DiskEncoder::addClockBits(u8 value, u8 previous)
{
    // Clear all previously set clock bits
    value &= 0x55;

    // Compute clock bits (clock bit values are inverted)
    u8 lShifted = (u8)(value << 1);
    u8 rShifted = (u8)(value >> 1 | previous << 7);
    u8 cBitsInv = (u8)(lShifted | rShifted);

    // Reverse the computed clock bits
    u8 cBits = cBitsInv ^ 0xAA;

    // Return original value with the clock bits added
    return value | cBits;
}

void
DiskEncoder::encodeAmigaTrack(MutableByteView track, TrackNr t, ByteView src)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize ssize = 1088;                      // MFM sector size in bytes
    const isize count = (isize)src.size() / bsize; // Number of sectors to encode

    if (ADF_DEBUG) fprintf(stderr, "Encoding Amiga track %ld with %ld sectors\n", t, count);
    assert(src.size() % bsize == 0);

    // Format track
    track.clear(0xAA);

    // Encode all sectors
    for (SectorNr s = 0; s < count; s++)
        encodeAmigaSector(track, s * ssize, t, s, src.subspan(s * bsize, bsize));

    // Compute a debug checksum
    if (ADF_DEBUG) fprintf(stderr, "Track %ld checksum = %x\n", t, track.fnv32());
}

void
DiskEncoder::encodeAmigaSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView data)
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
    encodeOddEven(&it[8], info, sizeof(info));

    // Unused area
    for (isize i = 16; i < 48; i++)
        it[i] = 0xAA;

    // Data
    encodeOddEven(&it[64], data.data(), bsize);

    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 8; i < 48; i += 4) {
        bcheck[0] ^= it[i];
        bcheck[1] ^= it[i+1];
        bcheck[2] ^= it[i+2];
        bcheck[3] ^= it[i+3];
    }
    encodeOddEven(&it[48], bcheck, sizeof(bcheck));

    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 64; i < ssize; i += 4) {
        dcheck[0] ^= it[i];
        dcheck[1] ^= it[i+1];
        dcheck[2] ^= it[i+2];
        dcheck[3] ^= it[i+3];
    }
    encodeOddEven(&it[56], dcheck, sizeof(dcheck));

    // Add clock bits
    for(isize i = 8; i < ssize + 1; i++) {
        it[i] = addClockBits(it[i], it[i-1]);
    }
}

void
DiskEncoder::decodeAmigaTrack(ByteView track, TrackNr t, MutableByteView dst)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize count = (isize)dst.size() / bsize; // Number of sectors to decode

    if (ADF_DEBUG) fprintf(stderr, "Decoding track %ld\n", t);
    assert(dst.size() % bsize == 0);

    // Find all sectors
    auto offsets = seekSectors(track);

    for (isize i = 0; i < count; ++i) {
        if (!offsets.contains(i)) {
            warn("Sector %ld not found. Aborting.\n", i);
            throw DeviceError(DeviceError::DEV_SEEK_ERR);
        }
    }
    if (isize(offsets.size()) != count) {
        warn("Found %zd sectors, expected %ld. Aborting.\n", offsets.size(), count);
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    // Decode all sectors
    for (SectorNr s = 0; s < count; s++) {

        if (!offsets.contains(s)) {
            warn("Sector %ld not found. Aborting.\n", s);
            throw DeviceError(DeviceError::DEV_SEEK_ERR);
        }

        auto *secData = dst.data() + s * bsize;
        decodeAmigaSector(track, offsets[s], span<u8>(secData, bsize));
    }

/*
    // Find all sync marks
    auto it = track.cyclic_begin();
    std::vector<isize> syncMarks;
    syncMarks.reserve(count);

    constexpr isize syncMarkLen = 4;
    for (isize i = 0; i < track.size() + syncMarkLen; ++i, ++it) {

        // Scan MFM stream for $4489 $4489
        if (it[0] != 0x44) continue;
        if (it[1] != 0x89) continue;
        if (it[2] != 0x44) continue;
        if (it[3] != 0x89) continue;

        // Make sure it's not a DOS track
        if (it[5] == 0x89) continue;

        syncMarks.push_back(it.offset());
    }

    if (ADF_DEBUG) fprintf(stderr, "Found %zd sectors (expected %ld)\n", syncMarks.size(), count);

    // Check that the track contains the proper amount of sync marks
    if (isize(syncMarks.size()) != count) {

        warn("Found %zd sectors, expected %ld. Aborting.\n", syncMarks.size(), count);
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    // Decode all sectors
    for (SectorNr s = 0; s < count; s++)
        decodeAmigaSector(track, syncMarks[s], dst);
*/
}

void
DiskEncoder::decodeAmigaSector(ByteView track, isize offset, MutableByteView dst)
{
    const isize bsize = 512;
    assert(dst.size() == bsize);

    if (MFM_DEBUG) fprintf(stderr, "Decoding sector at offset %ld\n", offset);

    // Skip sync mark
    offset += 4;

    /*
    // Decode sector info
    u8 info[4]; decodeOddEven(info, &track[offset], 4);

    // Extract the sector number
    auto secNr = SectorNr(info[2]);

    if (MFM_DEBUG) fprintf(stderr, "Decoding sector %ld\n", secNr);
    */

    // Check that the sector number is valid
    /*
    if (secNr * bsize + bsize > dst.size()) {

        warn("Invalid sector number %ld. Aborting.\n", secNr);
        throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
    }
    */

    // Determine the destination address
    // auto *secData = dst.data() + secNr * bsize;

    // Determine the source address (adding 56 skips the sector header)
    auto *mfmData = &track[offset + 56];

    // Decode sector data
    decodeOddEven(dst.data(), mfmData, bsize);
}

optional<isize>
DiskEncoder::trySeekSector(ByteView track, SectorNr s, isize offset)
{
    constexpr isize syncMarkLen = 4;

    // Search through all sync marks...
    auto it = track.cyclic_begin(offset);

   for (isize i = 0; i < track.size() + syncMarkLen; ++i, ++it) {

        // Scan MFM stream for $4489 $4489
        if (it[0] != 0x44) continue;
        if (it[1] != 0x89) continue;
        if (it[2] != 0x44) continue;
        if (it[3] != 0x89) continue;

        // Make sure it's not a DOS track
        if (it[5] == 0x89) continue;

        // Decode track & sector info
        u8 info[4]; decodeOddEven(info, &it[4], 4);

        // Check if the sector number matches
        if (info[2] == s) return it.offset();
    }

    return {};
}

isize
DiskEncoder::seekSector(ByteView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
}

std::unordered_map<isize, isize>
DiskEncoder::seekSectors(ByteView track)
{
    constexpr isize syncMarkLen = 4;
    std::unordered_map<isize, isize> result;

    // Search through all sync marks...
    auto it = track.cyclic_begin();

    for (isize i = 0; i < track.size() + syncMarkLen; ++i, ++it) {

        // Scan MFM stream for $4489 $4489
        if (it[0] != 0x44) continue;
        if (it[1] != 0x89) continue;
        if (it[2] != 0x44) continue;
        if (it[3] != 0x89) continue;

        // Make sure it's not a DOS track
        if (it[5] == 0x89) continue;

        // Decode track & sector info (info[2] = sector number)
        u8 info[4]; decodeOddEven(info, &it[4], 4);

        // Record the offset
        result[info[2]] = it.offset();
    }

    return result;
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
    for (TrackNr t = 0; t < tracks; t++) {
        encodeAmigaTrack(disk.byteView(t), t, adf.byteView(t));
    }

    // In debug mode, also run the decoder
    if (ADF_DEBUG) {

        auto adf = ADFFactory::make(disk);
        string tmp = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        adf->writeToFile(tmp);
    }
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
    for (TrackNr t = 0; t < tracks; t++)
        decodeAmigaTrack(disk.byteView(t), t, adf.byteView(t));
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
    for (TrackNr t = 0; t < tracks; t++) encodeTrack(img, disk, t);

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
    for (TrackNr t = 0; t < tracks; t++) decodeTrack(img, disk, t);
}

void
DiskEncoder::encodeTrack(const IMGFile &img, FloppyDisk &disk, TrackNr t)
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
    for (SectorNr s = 0; s < sectors; s++) encodeSector(img, disk, t, s);

    // Compute a checksum for debugging
    if (IMG_DEBUG) fprintf(stderr, "Track %ld checksum = %llx\n", t, disk.checksum(t));
}

void
DiskEncoder::encodeSector(const IMGFile &img, FloppyDisk &disk, TrackNr t, SectorNr s)
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
    encodeMFM(p, buf, sizeof(buf));
    addClockBits(p, 2 * sizeof(buf));

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
DiskEncoder::decodeTrack(IMGFile &img, const class FloppyDisk &disk, TrackNr t)
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
        decodeMFM((u8 *)&chrn, &src[i], 4);
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
    for (SectorNr s = 0; s < numSectors; s++, dst += 512) {
        decodeSector(img, dst, src + sectorStart[s]);
    }
}

void
DiskEncoder::decodeSector(IMGFile &img, u8 *dst, const u8 *src)
{
    decodeMFM(dst, src, 512);
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
    for (TrackNr t = 0; t < tracks; t++) encodeTrack(img, disk, t);

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
    for (TrackNr t = 0; t < tracks; t++) decodeTrack(img, disk, t);
}

void
DiskEncoder::encodeTrack(const STFile &img, FloppyDisk &disk, TrackNr t)
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
    for (SectorNr s = 0; s < sectors; s++) encodeSector(img, disk, t, s);

    // Compute a checksum for debugging
    if (IMG_DEBUG) fprintf(stderr, "Track %ld checksum = %llx\n", t, disk.checksum(t));
}

void
DiskEncoder::encodeSector(const STFile &img, FloppyDisk &disk, TrackNr t, SectorNr s)
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
    encodeMFM(p, buf, sizeof(buf));
    addClockBits(p, 2 * sizeof(buf));

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
DiskEncoder::decodeTrack(STFile &img, const class FloppyDisk &disk, TrackNr t)
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
        decodeMFM((u8 *)&chrn, &src[i], 4);
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
    for (SectorNr s = 0; s < numSectors; s++, dst += 512) {
        decodeSector(img, dst, src + sectorStart[s]);
    }
}

void
DiskEncoder::decodeSector(STFile &img, u8 *dst, const u8 *src)
{
    decodeMFM(dst, src, 512);
}


}

