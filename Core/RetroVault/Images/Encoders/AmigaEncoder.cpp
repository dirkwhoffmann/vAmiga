// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaEncoder.h"
#include "DeviceError.h"
#include "MFM.h"
#include "utl/support/Bits.h"

namespace retro::vault::image {

namespace Encoder { AmigaEncoder amiga; }

BitView
AmigaEncoder::encodeTrack(TrackNr t, ByteView src)
{
    // Determine the number of sectors to encode
    const isize count = (isize)src.size() / bsize;
    if (count >= maxsec) throw DeviceError(DeviceError::DSK_WRONG_SECTOR_CNT);

    loginfo(ADF_DEBUG, "Encoding Amiga track %ld with %ld sectors\n", t, count);
    assert(src.size() % bsize == 0);

    // Start with a clean track
    auto view = MutableByteView(mfmBuffer, count * ssize);
    view.clear(0xAA);

    // Encode all sectors
    for (SectorNr s = 0; s < count; s++)
        encodeSector(view, s * ssize, t, s, ByteView(src.subspan(s * bsize, bsize)));

    // Compute a debug checksum
    loginfo(IMG_DEBUG, "Track %ld checksum = %x\n", t, view.fnv32());

    return BitView(view.data(), view.size() * 8);
}

void
AmigaEncoder::encodeSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView data)
{
    const isize bsize = 512;   // Block size in bytes
    const isize ssize = 1088;  // MFM sector size in bytes

    loginfo(ADF_DEBUG, "Encoding Amiga sector %ld\n", s);
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
    MFM::encodeOddEven(&it[8], info, sizeof(info));

    // Unused area
    for (isize i = 16; i < 48; i++)
        it[i] = 0xAA;

    // Data
    MFM::encodeOddEven(&it[64], data.data(), bsize);

    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 8; i < 48; i += 4) {
        bcheck[0] ^= it[i];
        bcheck[1] ^= it[i+1];
        bcheck[2] ^= it[i+2];
        bcheck[3] ^= it[i+3];
    }
    MFM::encodeOddEven(&it[48], bcheck, sizeof(bcheck));

    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 64; i < ssize; i += 4) {
        dcheck[0] ^= it[i];
        dcheck[1] ^= it[i+1];
        dcheck[2] ^= it[i+2];
        dcheck[3] ^= it[i+3];
    }
    MFM::encodeOddEven(&it[56], dcheck, sizeof(dcheck));

    // Add clock bits
    for(isize i = 8; i < ssize + 1; i++) {
        it[i] = MFM::addClockBits(it[i], it[i-1]);
    }
}

void
AmigaEncoder::decodeTrack(ByteView track, TrackNr t, MutableByteView dst)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize count = (isize)dst.size() / bsize; // Number of sectors to decode

    loginfo(MFM_DEBUG, "Decoding Amiga track %ld\n", t);
    assert(dst.size() % bsize == 0);

    // Find all sectors
    auto offsets = seekSectors(track);

    // Decode all sectors
    for (SectorNr s = 0; s < count; s++) {

        if (!offsets.contains(s))
            throw DeviceError(DeviceError::SEEK_ERR);

        auto *secData = dst.data() + s * bsize;
        decodeSector(track, offsets[s], MutableByteView(span<u8>(secData, bsize)));
    }
}

void
AmigaEncoder::decodeSector(ByteView track, isize offset, MutableByteView dst)
{
    const isize bsize = 512;
    assert(dst.size() == bsize);

    if constexpr (debug::MFM_DEBUG) fprintf(stderr, "Decoding Amiga sector at offset %ld\n", offset);

    // Skip sync mark + sector header
    offset += 4 + 56;

    // Determine the source address
    auto *mfmData = &track[offset];

    // Decode sector data
    MFM::decodeOddEven(dst.data(), mfmData, bsize);
}

optional<isize>
AmigaEncoder::trySeekSector(ByteView track, SectorNr s, isize offset)
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
        u8 info[4]; MFM::decodeOddEven(info, &it[4], 4);

        // Check if the sector number matches
        if (info[2] == s) return it.offset();
    }

    return {};
}

isize
AmigaEncoder::seekSector(ByteView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::INVALID_SECTOR_NR);
}

std::unordered_map<isize, isize>
AmigaEncoder::seekSectors(ByteView track)
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
        u8 info[4]; MFM::decodeOddEven(info, &it[4], 4);

        // Record the offset
        result[info[2]] = it.offset();
    }

    return result;
}

}
