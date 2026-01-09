// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "C64Decoder.h"
#include "DeviceError.h"
#include "GCR.h"
#include "utl/support/Bits.h"
#include <unordered_set>

namespace retro::vault {

static constexpr isize bsize  = 256;

struct SYNC
{
    isize ones = 0;

    bool sync(bool bit) {

        bool result = ones == 50;
        ones = bit ? ones + 1 : 0;
        return result;
    }
};

ByteView
C64Decoder::decodeTrack(BitView track, TrackNr t)
{
    loginfo(IMG_DEBUG, "Decoding C64 track %ld\n", t);

    // Setup the backing buffer
    if (trackbuffer.empty()) trackbuffer.resize(16384);

    // Find all sectors
    auto sectors = seekSectors(track);
    auto numSectors = isize(sectors.size());

    // Decode all sectors
    for (isize s = 0; s < numSectors; ++s) {

        if (!sectors.contains(s))
            throw DeviceError(DeviceError::SEEK_ERR);

        decodeSector(sectors[s],
                     MutableByteView(trackbuffer.data() + s * bsize, bsize));
    }

    return ByteView(trackbuffer.data(), numSectors * bsize);
}

ByteView
C64Decoder::decodeSector(BitView track, TrackNr t, SectorNr s)
{
    loginfo(IMG_DEBUG, "Decoding C64 track %ld:%ld\n", t, s);

    // Setup the backing buffer
    if (sectorBuffer.empty()) sectorBuffer.resize(bsize);

    // Find sector
    auto sector = seekSector(track, s);

    // Skip sync mark + sector header
    isize offset = 40 + 10;

    // Decode sector data
    for (isize i = 0; i < bsize; ++i) {

        sectorBuffer[i] = GCR::decodeGcr(sector, offset);
        offset += 10;
    }

    return ByteView(sectorBuffer);
}

void
C64Decoder::decodeSector(BitView sector, MutableByteView dst)
{
    assert(dst.size() == bsize);

    // Skip sync mark + sector header
    isize offset = 40 + 10;

    // Decode sector data
    for (isize i = 0; i < bsize; ++i) {

        dst[i] = GCR::decodeGcr(sector, offset);
        offset += 10;
    }
}

bool
C64Decoder::seekSync(BitView track, BitView::cyclic_iterator &it)
{
    for (isize i = 0, ones = 0; i < track.size() + 50; ++i, ++it) {

        if (it[0] == 0 && ones >= 50)
            return true;

        ones = it[0] == 1 ? ones + 1 : 0;
    }

    return false;
}

bool
C64Decoder::seekHeaderSync(BitView track, BitView::cyclic_iterator &it)
{
    for (isize i = 0, ones = 0; i < track.size() + 50; ++i, ++it) {

        if (it[0] == 0 && ones >= 50) {

            // $08 indicates a header block
            if (auto id = GCR::decodeGcr(track, it.offset()); id == 0x08) {
                return true;
            }
        }
        ones = it[0] == 1 ? ones + 1 : 0;
    }

    return false;
}

BitView
C64Decoder::seekSector(BitView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::INVALID_SECTOR_NR);
}

optional<BitView>
C64Decoder::trySeekSector(BitView track, SectorNr s, isize offset)
{
    auto map = seekSectors(track, std::span<const SectorNr>(&s, 1), offset);

    if (auto it = map.find(s); it != map.end())
        return it->second;

    return std::nullopt;
}

std::unordered_map<SectorNr, BitView>
C64Decoder::seekSectors(BitView track)
{
    return seekSectors(track, std::vector<SectorNr>(), 0);
}

std::unordered_map<SectorNr, BitView>
C64Decoder::seekSectors(BitView track, std::span<const SectorNr> wanted, isize offset)
{
    std::unordered_set<SectorNr> visited;
    std::unordered_map<SectorNr, BitView> result;

    // Loop until a sector header repeats or no sync marks are found
    for (auto it = track.cyclic_begin(offset);;) {

        // Move to the next header block
        if (!seekHeaderSync(track, it)) break;

        // Skip id and checksum
        it += 2 * GCR::bitsPerByte;

        // The next byte stores the sector number
        SectorNr nr = GCR::decodeGcr(track, it.offset());

        // Bail out if we've seen this sector before
        if (!visited.insert(nr).second) break;

        // Record the sector if requested
        if (wanted.empty() || std::find(wanted.begin(), wanted.end(), nr) != wanted.end()) {

            // Move beyond the next sync mark
            if (!seekSync(track, it)) break;

            // The next byte contains the block id
            if (auto id = GCR::decodeGcr(track, it.offset()); id == 0x07) {

                // Skip id
                it += GCR::bitsPerByte;

                // At this point, the offset references the first data bit
                result[nr] = track.subview(it.offset(), GCR::bitsPerByte * 256);

                // Check for early exit
                if (!wanted.empty() && result.size() == wanted.size()) break;
            }
        }
    }

    return result;
}

/*
isize
C64Decoder::seekSector(BitView track, SectorNr s, isize offset)
{
    if (auto result = trySeekSector(track, s, offset))
        return *result;

    throw DeviceError(DeviceError::INVALID_SECTOR_NR);
}

bool
C64Decoder::seekSectorHeader(BitView track, BitView::cyclic_iterator &it)
{
    constexpr isize syncMarkLen = 50;

    isize max = track.size() + syncMarkLen;
    for (isize i = 0, ones = 0; i < max; ++i, ++it) {

        if (ones == 50) return true;
        ones = it[i] ? ones + 1 : 0;
    }
}
*/

}

