// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "D64File.h"
#include "GCR.h"
#include "utl/io.h"
#include "utl/support/Strings.h"
#include <format>

namespace retro::vault::image {

BitView
D64File::encode(TrackNr t) const
{
    loginfo(IMG_DEBUG, "Encoding D64 track %ld\n", t);
    validateTrackNr(t);

    /* Naming scheme:
     *
     * TrackNr    0     1     2     3           68     69           82     83
     *         -----------------------------------------------------------------
     * C64     |  1  | 1.5 |  2  | 2.5 | ... |  35  | 35.5 | ... |  42  | 42.5 |
     *         -----------------------------------------------------------------
     */
    auto track = MutableByteView(gcrbuffer, sizeof(gcrbuffer));

    // Format track
    track.clear(0x55);

    // Get track infos
    auto &defaults = trackDefaults(t);

    // Create a bit view with proper length
    auto view = MutableBitView(gcrbuffer, defaults.lengthInBits);

    // Compute start position inside the bit view
    auto offset = isize(view.size() * defaults.stagger);

    // For each sector in this track ...
    isize totalBits = 0;
    for (SectorNr s = 0; s < defaults.sectors; ++s) {

        isize encodedBits = encodeSector(view, t, s, offset);
        offset += encodedBits;
        totalBits += encodedBits;
    }

    if constexpr (debug::IMG_DEBUG) {

        loginfo(IMG_DEBUG,
                "\nTrack size: %ld Encoded: %ld Checksum: %x\n",
                view.size(), totalBits, view.byteView().fnv32());
    }
    return view;
}

isize
D64File::encodeSector(MutableBitView &view, TrackNr t, SectorNr s, isize offset) const
{
    loginfo(IMG_DEBUG, "%ld (%ld) ", s, offset);

    auto ts = TS{t,s};
    validateTS(ts);

    isize b = bindex(ts);
    validateBlockNr(b);

    auto head      = offset;
    auto &defaults = trackDefaults(t);
    auto errorCode = getErrorCode(b);

    // Get disk id and compute checksum
    auto *bam = data.ptr + 357 * bsize();
    u8 id1 = bam[0xA2];
    u8 id2 = bam[0xA3];
    u8 checksum = (u8)(id1 ^ id2 ^ (t + 1) ^ s); // Header checksum byte

    // SYNC (0xFF 0xFF 0xFF 0xFF 0xFF)
    if (errorCode == 0x3) {
        view.setBytes(head, std::vector<u8>(5, 0x00)); // HEADER_CHECKSUM_ERROR
    } else {
        view.setBytes(head, std::vector<u8>(5, 0xFF));
    }
    head += 40;

    // Header ID
    if (errorCode == 0x2) {
        GCR::encodeGcr(view, head, 0x00); // HEADER_BLOCK_NOT_FOUND_ERROR
    } else {
        GCR::encodeGcr(view, head, 0x08);
    }
    head += 10;

    // Checksum
    if (errorCode == 0x9) {
        GCR::encodeGcr(view, head, checksum ^ 0xFF); // HEADER_BLOCK_CHECKSUM_ERROR
    } else {
        GCR::encodeGcr(view, head, checksum);
    }
    head += 10;

    // Sector and track number
    GCR::encodeGcr(view, head, u8(s));
    head += 10;
    GCR::encodeGcr(view, head, u8(t + 1));
    head += 10;

    // Disk ID (two bytes)
    if (errorCode == 0xB) {
        GCR::encodeGcr(view, head, id2 ^ 0xFF); // DISK_ID_MISMATCH_ERROR
        head += 10;
        GCR::encodeGcr(view, head, id1 ^ 0xFF); // DISK_ID_MISMATCH_ERROR
    } else {
        GCR::encodeGcr(view, head, id2);
        head += 10;
        GCR::encodeGcr(view, head, id1);
    }
    head += 10;

    // 0x0F, 0x0F
    GCR::encodeGcr(view, head, 0x0F);
    head += 10;
    GCR::encodeGcr(view, head, 0x0F);
    head += 10;

    // 0x55 0x55 0x55 0x55 0x55 0x55 0x55 0x55 0x55
    view.setBytes(head, std::vector<u8>(9, 0x55));
    // writeGapToTrack(t, offset, 9);
    head += 9 * 8;

    // SYNC (0xFF 0xFF 0xFF 0xFF 0xFF)
    if (errorCode == 3) {
        view.setBytes(head, std::vector<u8>(5, 0x00)); // NO_SYNC_SEQUENCE_ERROR
    } else {
        view.setBytes(head, std::vector<u8>(5, 0xFF));
    }
    head += 40;

    // Data ID
    if (errorCode == 0x4) {
        // The error value is important here:
        // (1) If the first GCR bit equals 0, the sector can still be read.
        // (2) If the first GCR bit equals 1, the SYNC sequence continues.
        //     In this case, the bit sequence gets out of sync and the data
        //     can't be read.
        // Hoxs64 and VICE 3.2 write 0x00 which results in option (1)
        GCR::encodeGcr(view, head, 0x00); // DATA_BLOCK_NOT_FOUND_ERROR
    } else {
        GCR::encodeGcr(view, head, 0x07);
    }
    head += 10;

    // Data bytes
    checksum = 0;
    for (isize i = 0; i < 256; i++, head += 10) {
        u8 byte = data[b * 256 + i]; // fs.readByte(ts, (u32)i);
        checksum ^= byte;
        GCR::encodeGcr(view, head, byte);
    }

    // Checksum
    if (errorCode == 0x5) {
        GCR::encodeGcr(view, head, checksum ^ 0xFF); // DATA_BLOCK_CHECKSUM_ERROR
    } else {
        GCR::encodeGcr(view, head, checksum);
    }
    head += 10;

    // 0x00, 0x00
    GCR::encodeGcr(view, head, 0x00);
    head += 10;
    GCR::encodeGcr(view, head, 0x00);
    head += 10;

    // Tail gap (0x55 0x55 ... 0x55)
    view.setBytes(head, std::vector<u8>(defaults.tailGap, 0x55));
    // writeGapToTrack(t, offset, tailGap);
    head += defaults.tailGap * 8;

    // Return the number of encoded bits
    return head - offset;
}

void
D64File::decode(TrackNr t, BitView bits)
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

}
