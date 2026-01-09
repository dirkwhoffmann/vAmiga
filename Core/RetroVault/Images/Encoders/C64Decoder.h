// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskDecoder.h"

namespace retro::vault {

class C64Decoder : public DiskDecoder {

    // Backing buffer
    std::vector<u8> bytes;

public:

    // Methods from DiskDecoder
    ByteView decodeTrack(TrackNr t, BitView src) override;

    // Returns the start offset of a SYNC mark (empty if not found)
    // optional<isize> trySeekSync(BitView track, isize offset = 0);
    // optional<isize> trySeekSync(BitView track, isize offset, isize max);

    // Returns the start offset of a sector (empty if not found)
    // optional<isize> trySeekSector(BitView track, SectorNr s, isize offset = 0);

    // Returns the start offset of a sector (throws if not found)
    // isize seekSector(BitView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    // std::unordered_map<isize, isize> seekSectors(BitView track);

    // Decodes a single sector
    void decodeSector(BitView sector, MutableByteView dst);

    // Returns a bit view on the data section of a sector (throws if not found)
    BitView seekSector(BitView track, SectorNr s, isize offset = 0);

    // Returns a bit view on the data section of a sector (empty if not found)
    optional<BitView> trySeekSector(BitView track, SectorNr s, isize offset = 0);

    // Computes the data section bit views for all sectors
    std::unordered_map<SectorNr, BitView> seekSectors(BitView track);

private:

    // Moves the iterator to the bit following the next sync mark
    bool seekSync(BitView track, BitView::cyclic_iterator &it);

    // Moves the iterator to the bit following the next header block sync mark
    bool seekHeaderSync(BitView track, BitView::cyclic_iterator &it);

    // Finds the start offsets of sectors on a track
    //
    // `wanted` specifies which sectors to locate. For example, { 7 } searches
    // only for sector 7. If empty, all sectors are found.
    //
    // `offset` specifies the bit position at which the search begins. If a
    // sector’s approximate position is already known, this can be used to
    // speed up the search.
    //
    // Returns a mapping from sector numbers to their start offsets. Each offset
    // is the bit position of the sector’s first data bit.

    std::unordered_map<SectorNr, BitView> seekSectors(BitView track,
                                                      std::span<const SectorNr> wanted,
                                                      isize offset = 0);

};

}
