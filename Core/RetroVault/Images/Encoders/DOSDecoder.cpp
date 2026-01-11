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
#include <unordered_set>

namespace retro::vault {

ByteView
DOSDecoder::decodeTrack(BitView track, TrackNr t, std::span<u8> out)
{
    loginfo(IMG_DEBUG, "Decoding DOS track %ld\n", t);

    // Find all sectors
    auto sectors    = seekSectorsNew(track);
    auto numSectors = isize(sectors.size());

    // Ensure the output buffer is large enough
    assert(isize(out.size()) >= numSectors * bsize);

    // Iterator through all sectors
    for (isize s = 0; s < numSectors; ++s) {

        if (!sectors.contains(s))
            throw DeviceError(DeviceError::SEEK_ERR);

        auto it = track.cyclic_begin() + sectors[s].lower;

        // Read sector data
        assert(sectors[s].size() == 1024*8);
        u8 mfm[1024]; for (isize i = 0; i < 1024; ++i) mfm[i] = it.readByte();

        // Decode data
        MFM::decodeMFM(out.data() + s * bsize, mfm, bsize);
    }

    return ByteView(out.data(), numSectors * bsize);
}

/*
ByteView
DOSDecoder::decodeTrack(BitView track, TrackNr t, std::span<u8> out)
{
    loginfo(IMG_DEBUG, "Decoding DOS track %ld\n", t);

    // Find all IDAM blocks
    auto offsets    = seekSectors(track.byteView());
    auto numSectors = isize(offsets.size());

    // Ensure the output buffer is large enough
    assert(isize(out.size()) >= numSectors * bsize);

    // Decode all sectors
    for (SectorNr s = 0; s < numSectors; s++) {

        if (!offsets.contains(s))
            throw DeviceError(DeviceError::SEEK_ERR,
                              "Sector " + std::to_string(s) + " not found");

        decodeSector(track.byteView(),
                     offsets[s],
                     MutableByteView(out.data() + s * bsize, bsize));
    }

    return ByteView(out.data(), numSectors * bsize);
}
*/

ByteView
DOSDecoder::decodeSector(BitView track, TrackNr t, SectorNr s, std::span<u8> out)
{
    loginfo(IMG_DEBUG, "Decoding DOS track %ld:%ld\n", t, s);

    // TODO
    return ByteView(nullptr, 0);
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

optional<Range<isize>>
DOSDecoder::seekSectorNew(BitView track, SectorNr s, isize offset)
{
    auto map = seekSectors(track, std::vector<SectorNr>{s}, offset);

    if (!map.contains(s))
        throw DeviceError(DeviceError::INVALID_SECTOR_NR);

    return map[s];
}

std::unordered_map<isize, Range<isize>>
DOSDecoder::seekSectorsNew(BitView track)
{
    return seekSectors(track, std::vector<SectorNr>{});
}

std::unordered_map<SectorNr, Range<isize>>
DOSDecoder::seekSectors(BitView track, std::span<const SectorNr> wanted, isize offset)
{
    constexpr u64 IDAM = u64(0x4489448944895554);
    constexpr u64 DAM  = u64(0x4489448944895545);

    std::unordered_map<SectorNr, Range<isize>> result;
    std::unordered_set<SectorNr> visited;

    // Loop until a sector header repeats or no sync marks are found
    for (auto it = track.cyclic_begin(offset);;) {

        // Move behind the next IDAM sync mark
        if (!track.forward(it, IDAM, 64)) throw DeviceError(DeviceError::SEEK_ERR);

        // Read the next 8 MFM bytes
        u8 mfm[8]; for (isize i = 0; i < 8; ++i) mfm[i] = it.readByte();

        // Decode the CHRN block
        u8 info[4]; MFM::decodeMFM(info, mfm, 4);

        // The sector number is encoded in the third byte, counting 1,2,...
        u8 s = info[2] - 1;

        // Break the loop if we've seen this sector before
        if (!visited.insert(s).second) break;

        // If the sector is requested...
        if (wanted.empty() || std::find(wanted.begin(), wanted.end(), s) != wanted.end()) {

            // Move behind the next IDAM sync mark
            if (!track.forward(it, DAM, 64)) throw DeviceError(DeviceError::SEEK_ERR);

            // Record the sector number
            result[s] = Range<isize>(it.offset(), it.offset() + 1024 * 8);

            // Check for early exit
            if (!wanted.empty() && result.size() == wanted.size()) break;
        }
    }
    return result;
}

}
