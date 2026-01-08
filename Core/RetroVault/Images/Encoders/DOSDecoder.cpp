// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DOSDecoder.h"
#include "DeviceError.h"
#include "MFM.h"
#include "utl/support/Bits.h"
#include <array>

namespace retro::vault {

namespace Decoder { DOSDecoder dos; }

ByteView
DOSDecoder::decodeTrack(TrackNr t, BitView src)
{
    loginfo(IMG_DEBUG, "Decoding DOS track %ld\n", t);

    // Setup the backing buffer
    if (!decoded) decoded = make_unique<u8>(16384);

    // Find all IDAM blocks
    auto offsets    = seekSectors(src.byteView());
    auto numSectors = isize(offsets.size());

    // Decode all sectors
    for (SectorNr s = 0; s < numSectors; s++) {

        if (!offsets.contains(s))
            throw DeviceError(DeviceError::SEEK_ERR,
                              "Sector " + std::to_string(s) + " not found");

        decodeSector(src.byteView(),
                     offsets[s],
                     MutableByteView(decoded.get() + s * bsize, bsize));
    }

    return ByteView(decoded.get(), numSectors * bsize);
}

void
DOSDecoder::decodeSector(ByteView track, isize offset, MutableByteView dst)
{
    const isize bsize = 512;
    assert(dst.size() == bsize);

    loginfo(MFM_DEBUG, "Decoding DOS sector at offset %ld\n", offset);

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
    throw DeviceError(DeviceError::SEEK_ERR,
                      "No DAM found for IDAM at " + std::to_string(offset));
}

optional<isize>
DOSDecoder::trySeekSector(ByteView track, SectorNr s, isize offset)
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
DOSDecoder::seekSector(ByteView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::SEEK_ERR,
                      "Sector " + std::to_string(s) + " not found");
}

std::unordered_map<isize, isize>
DOSDecoder::seekSectors(ByteView track)
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
        loginfo(IMG_DEBUG, "c: %d h: %d r: %d n: %d\n", chrn.c, chrn.h, chrn.r, chrn.n);

        if (chrn.r >= 1 && chrn.r <= numSectors) {

            // Break the loop once we see the same sector twice
            if (result.contains(chrn.r - 1)) break;

            result[chrn.r - 1] = it.offset();

        } else {
            throw DeviceError(DeviceError::INVALID_SECTOR_NR);
        }
    }

    if (result.size() != numSectors) {
        throw DeviceError(DeviceError::DSK_WRONG_SECTOR_CNT);
    }

    return result;
}

}
