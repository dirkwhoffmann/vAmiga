// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystems/CBM/FSTypes.h"
#include "DeviceDescriptors.h"

namespace retro::vault::image { class D64File; }

namespace retro::vault::cbm {

using image::D64File;

/* To create a FileSystem, several layout parameters need to to be provided.
 * This is done by passing a FSDescriptor which contains the necessary
 * information.
 */
struct FSTraits {

    // File system type
    FSFormat dos = FSFormat::NODOS;

    // Number of blocks
    isize blocks = 0;

    // Number of bytes
    isize bytes = 0;

    // Number of cylinders
    isize numCyls = 0;

    // Number of heads
    isize numHeads = 0;

    // Block size in bytes
    isize bsize = 256;

    FSTraits() { }

    // Creates a device descriptor for a given block count
    FSTraits(FSFormat format, isize numBlocks) { init(format, numBlocks); }

    // Creates a device descriptor fitting a D64 file
    FSTraits(const D64File &d64) { init(d64); }

    void init(FSFormat format, isize numBlocks);
    void init(const D64File &d64);

    // Prints debug information
    void dump() const;
    void dump(std::ostream &os) const;

    // Throws an exception if the descriptor contains unsupported values
    void checkCompatibility() const;


    //
    // Performing integrity checks
    //

    bool isCylinderNr(CylNr c) const { return 1 <= c && c <= numCyls; }
    bool isHeadNr(HeadNr h) const { return h == 0 || h == 1; }
    bool isTrackNr(TrackNr t) const { return 1 <= t && t <= numCyls * numHeads; }
    bool isValidLink(TSLink ref) const;


    //
    // Querying device properties
    //

    isize numTracks() const { return numCyls * numHeads; }
    isize speedZone(CylNr track) const;
    isize numSectors(CylNr track) const;
    isize numBlocks() const;


    //
    // Translating blocks, tracks, sectors, and heads
    //

    // DEPRECATED
    CylNr cylNr(TrackNr t) const { return t <= numCyls ? t : t - numCyls; }
    HeadNr headNr(TrackNr t) const { return t <= numCyls ? 0 : 1; }
    TrackNr trackNr(CylNr c, HeadNr h) const { return c + h * numCyls; }
    TSLink tsLink(BlockNr b) const;
    TrackNr trackNr(BlockNr b) const { return tsLink(b).t; }
    SectorNr sectorNr(BlockNr b) const { return tsLink(b).s; }


    optional<BlockNr> blockNr(TSLink ts) const;


    // DEPRECATED
    optional<BlockNr> blockNr(TrackNr t, SectorNr s) const { return blockNr(TSLink{t,s}); }
    optional<BlockNr> blockNr(CylNr c, HeadNr h, SectorNr s) const { return blockNr(trackNr(c,h), s); }


    //
    // Ordering blocks
    //

    bool nextBlock(BlockNr b, BlockNr *nb) const;
    TSLink nextBlockRef(TSLink ts) const;
    TSLink nextBlockRef(BlockNr b) const;
};

}
