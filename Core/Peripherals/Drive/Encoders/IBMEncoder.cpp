// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IBMEncoder.h"
#include "DeviceError.h"
#include "MFM.h"
#include "utl/support/Bits.h"

namespace vamiga {

namespace Encoder { IBMEncoder ibm; }

void
IBMEncoder::encodeTrack(MutableByteView track, TrackNr t, ByteView src)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize ssize = 1300;                      // MFM sector size in bytes
    const isize count = (isize)src.size() / bsize; // Number of sectors to encode

    if constexpr (IMG_DEBUG) fprintf(stderr, "Encoding DOS track %ld with %ld sectors\n", t, count);
    assert(src.size() % bsize == 0);

    // Format track
    track.clear( std::array<u8,2> { 0x92, 0x54 } );

    auto p = track.cyclic_begin();

    // Encode track header
    p = p + 82;                                         // GAP
    for (isize i = 0; i < 24; i++) { p[i] = 0xAA; }     // SYNC
    p = p + 24;
    p[0] = 0x52; p[1] = 0x24;                           // IAM
    p[2] = 0x52; p[3] = 0x24;
    p[4] = 0x52; p[5] = 0x24;
    p[6] = 0x55; p[7] = 0x52;
    p = p + 8;
    p = p + 80;                                         // GAP

    // Encode all sectors
    for (SectorNr s = 0; s < count; s++)
        encodeSector(track, 194 + s * ssize, t, s, ByteView(src.subspan(s * bsize, bsize)));

    // Compute a debug checksum
    if constexpr (IMG_DEBUG) fprintf(stderr, "Track %ld checksum = %x\n", t, track.fnv32());
}

void
IBMEncoder::encodeSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView data)
{
    const isize bsize = 512;   // Block size in bytes

    if constexpr (ADF_DEBUG) fprintf(stderr, "Encoding sector %ld\n", s);
    assert(data.size() == bsize);

    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap

    // Write SYNC
    for (isize i = 0; i < 12; ++i) { buf[i] = 0x00; }

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
    for (isize i = 22; i < 44; ++i) { buf[i] = 0x4E; }

    // Write SYNC
    for (isize i = 44; i < 56; ++i) { buf[i] = 0x00; }

    // Write DATA AM
    buf[56] = 0xA1;
    buf[57] = 0xA1;
    buf[58] = 0xA1;
    buf[59] = 0xFB;

    // Write DATA
    for (isize i = 0; i < bsize; ++i) { buf[60 + i] = data[i]; }

    // Compute and write CRC
    crc = Hashable::crc16(&buf[56], 516);
    buf[572] = HI_BYTE(crc);
    buf[573] = LO_BYTE(crc);

    // Write GAP
    for (usize i = 574; i < sizeof(buf); ++i) { buf[i] = 0x4E; }

    // Compute the start of this sector
    u8 *p = track.data() + offset;

    // Create the MFM data stream
    MFM::encodeMFM(p, buf, sizeof(buf));
    MFM::addClockBits(p, 2 * sizeof(buf));

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
IBMEncoder::decodeTrack(ByteView track, TrackNr t, MutableByteView dst)
{
    const isize bsize = 512;                       // Block size in bytes
    const isize count = (isize)dst.size() / bsize; // Number of sectors to decode

    if constexpr (IMG_DEBUG) fprintf(stderr, "Decoding DOS track %ld\n", t);
    assert(dst.size() % bsize == 0);

    // Find all IDAM blocks
    auto offsets = seekSectors(track);

    // Decode all sectors
    for (SectorNr s = 0; s < count; s++) {

        if (!offsets.contains(s))
            throw DeviceError(DeviceError::DEV_SEEK_ERR,
                              "Sector " + std::to_string(s) + " not found");

        auto *secData = dst.data() + s * bsize;
        decodeSector(track, offsets[s], MutableByteView(span<u8>(secData, bsize)));
    }
}

void
IBMEncoder::decodeSector(ByteView track, isize offset, MutableByteView dst)
{
    const isize bsize = 512;
    assert(dst.size() == bsize);

    if constexpr (MFM_DEBUG) fprintf(stderr, "Decoding DOS sector at offset %ld\n", offset);

    // Initialize an iterator at the position of the IDAM block
    auto it = track.cyclic_begin(offset);

    // Seek the DAM block
    for (isize i = 0; i < 256; ++i, ++it) {

        if (it[0] != 0x44) continue;
        if (it[1] != 0x89) continue;
        if (it[2] != 0x44) continue;
        if (it[3] != 0x89) continue;
        if (it[4] != 0x44) continue;
        if (it[5] != 0x89) continue;
        if (it[6] != 0x55) continue;
        if (it[7] != 0x45) continue;
        it = it + 8;

        // Read the MFM-encoded block data
        u8 data[1024];
        for (usize j = 0; j < sizeof(data); j++) data[j] = *(it++);

        // Decode the block
        MFM::decodeMFM(dst.data(), data, bsize);

        return;
    }
    throw DeviceError(DeviceError::DEV_SEEK_ERR,
                      "No DAM found for IDAM at " + std::to_string(offset));
}

optional<isize>
IBMEncoder::trySeekSector(ByteView track, SectorNr s, isize offset)
{
    constexpr isize syncMarkLen = 8;

    // Search through all IDAM blocks...
    auto it = track.cyclic_begin(offset);
    for (isize i = 0; i < track.size() + syncMarkLen; ++i, ++it) {

        // Scan MFM stream for the IDAM marker
        if (it[0] != 0x44) continue;
        if (it[1] != 0x89) continue;
        if (it[2] != 0x44) continue;
        if (it[3] != 0x89) continue;
        if (it[4] != 0x44) continue;
        if (it[5] != 0x89) continue;
        if (it[6] != 0x55) continue;
        if (it[7] != 0x54) continue;

        // Decode CHRN block
        struct { u8 c, h, r, n; } chrn;
        MFM::decodeMFM((u8 *)&chrn, &it[8], 4);

        if (chrn.r - 1 == s) return it.offset();
    }
    return { };
}

isize
IBMEncoder::seekSector(ByteView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::DEV_SEEK_ERR,
                      "Sector " + std::to_string(s) + " not found");
}

std::unordered_map<isize, isize>
IBMEncoder::seekSectors(ByteView track)
{
    constexpr isize numSectors  = 9;
    constexpr isize syncMarkLen = 8;

    std::unordered_map<isize, isize> result;

    // Search through all IDAM blocks...
    auto it = track.cyclic_begin();
    for (isize i = 0; i < track.size() + syncMarkLen; ++i, ++it) {

        // Scan MFM stream for the IDAM marker
        if (it[0] != 0x44) continue;
        if (it[1] != 0x89) continue;
        if (it[2] != 0x44) continue;
        if (it[3] != 0x89) continue;
        if (it[4] != 0x44) continue;
        if (it[5] != 0x89) continue;
        if (it[6] != 0x55) continue;
        if (it[7] != 0x54) continue;

        // Decode CHRN block
        struct { u8 c, h, r, n; } chrn;
        MFM::decodeMFM((u8 *)&chrn, &it[8], 4);
        if constexpr (IMG_DEBUG) fprintf(stderr, "c: %d h: %d r: %d n: %d\n", chrn.c, chrn.h, chrn.r, chrn.n);

        if (chrn.r >= 1 && chrn.r <= numSectors) {

            // Break the loop once we see the same sector twice
            if (result.contains(chrn.r - 1)) break;

            result[chrn.r - 1] = it.offset();

        } else {
            throw DeviceError(DeviceError::DSK_INVALID_SECTOR_NUMBER);
        }
    }

    if (result.size() != numSectors) {
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_COUNT);
    }

    return result;
}

}
