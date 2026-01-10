// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaDecoder.h"
#include "DeviceError.h"
#include "MFM.h"
#include "utl/support/Bits.h"

namespace retro::vault {

static constexpr isize bsize  = 512;

ByteView
AmigaDecoder::decodeTrack(BitView track, TrackNr t, std::span<u8> out)
{
    loginfo(IMG_DEBUG, "Decoding Amiga track %ld\n", t);

    // Find all sectors
    auto offsets    = seekSectors(track.byteView());
    auto numSectors = isize(offsets.size());

    // Ensure the output buffer is large enough
    assert(isize(out.size()) >= numSectors * bsize);

    // Decode all sectors
    for (isize s = 0; s < numSectors; ++s) {

        auto bytes = decodeSector(track, t, s);
        memcpy(out.data() + s * bsize, bytes.data(), bsize);
    }

    return ByteView(out.data(), numSectors * bsize);
}

ByteView
AmigaDecoder::decodeSector(BitView track, TrackNr t, SectorNr s, std::span<u8> out)
{
    loginfo(IMG_DEBUG, "Decoding Amiga sector %ld:%ld\n", t, s);

    // Ensure the output buffer is large enough
    assert(isize(out.size()) >= bsize);

    // Find sector
    auto sector = seekSector(track.byteView(), s);

    // Skip sync mark + sector header
    isize offset = sector + 4 + 56;

    // Determine the source address
    auto *mfmData = &track.byteView()[offset];

    // Decode sector data
    MFM::decodeOddEven(out.data(), mfmData, bsize);

    return ByteView(out);
}

optional<isize>
AmigaDecoder::trySeekSector(ByteView track, SectorNr s, isize offset)
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
AmigaDecoder::seekSector(ByteView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::SEEK_ERR);
}

std::unordered_map<isize, isize>
AmigaDecoder::seekSectors(ByteView track)
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

