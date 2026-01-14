// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FSTraits.h"
#include "FileSystems/CBM/FSError.h"
#include "BlockDevice.h"
#include "D64File.h"
#include "utl/io.h"

namespace retro::vault::cbm {

void
FSTraits::init(const D64File &d64)
{
    init(FSFormat::CBM, d64.numBlocks());
}

void
FSTraits::init(FSFormat format, isize blocks)
{
    this->dos    = format;
    this->bsize  = 256;
    this->blocks = blocks;
    this->bytes  = this->blocks * this->bsize;

    switch (blocks * this->bsize) {

        case D64File::D64_683_SECTORS: numCyls = 35; numHeads = 1; break;
        case D64File::D64_768_SECTORS: numCyls = 40; numHeads = 1; break;
        case D64File::D64_802_SECTORS: numCyls = 42; numHeads = 1; break;

        default:
            throw FSError(FSError::FS_WRONG_CAPACITY);
    }

    // this-> = blockNr(18, 0);
}

void
FSTraits::dump() const
{
    dump(std::cout);
}

void
FSTraits::dump(std::ostream &os) const
{
    using namespace utl;

    os << tab("Blocks");
    os << dec(numBlocks()) << std::endl;
    os << tab("Block size");
    os << dec(bsize) << std::endl;
}

void
FSTraits::checkCompatibility() const
{
    auto blocks = numBlocks();

    if constexpr (force::FS_WRONG_CAPACITY)
        throw FSError(FSError::FS_WRONG_CAPACITY);

    if constexpr (force::FS_WRONG_BSIZE)
        throw FSError(FSError::FS_WRONG_BSIZE);

    if (blocks != 683 && blocks != 768 && blocks != 802)
        throw FSError(FSError::FS_WRONG_CAPACITY);

    if (bsize != 256)
        throw FSError(FSError::FS_WRONG_BSIZE);
}

bool
FSTraits::isValidLink(TSLink ref) const
{
    return isTrackNr(ref.t) && ref.s >= 0 && ref.s < numSectors(ref.t);
}

isize
FSTraits::speedZone(CylNr t) const
{
    assert(isTrackNr(t));

    return (t <= 17) ? 3 : (t <= 24) ? 2 : (t <= 30) ? 1 : 0;
}

isize
FSTraits::numSectors(TrackNr t) const
{
    if (!isTrackNr(t)) return 0;

    switch (speedZone(t)) {

        case 0: return 17;
        case 1: return 18;
        case 2: return 19;
        case 3: return 21;
    }

    assert(false);
    return 0;
}

isize
FSTraits::numBlocks() const
{
    isize result = 0;

    for (TrackNr t = 1; t <= numTracks(); t++) {
        result += numSectors(t);
    }

    return result;
}

TSLink
FSTraits::tsLink(BlockNr b) const
{
    for (TrackNr i = 1; i <= numTracks(); i++) {

        isize num = numSectors(i);
        if (b < num) return TSLink{i,b};
        b -= num;
    }

    return TSLink{0,0};
}

optional<BlockNr>
FSTraits::blockNr(TSLink ts) const
{
    if (!isValidLink(ts)) return {};

    BlockNr result = ts.s;
    for (TrackNr i = 1; i < ts.t; i++) {
        result += numSectors(i);
    }

    return result;
}

TSLink
FSTraits::nextBlockRef(BlockNr b) const
{
    return nextBlockRef(tsLink(b));
}

TSLink
FSTraits::nextBlockRef(TSLink ref) const
{
    assert(isValidLink(ref));

    TrackNr t = ref.t;
    SectorNr s = ref.s;

    // Lookup table for the next sector (interleave patterns)
    SectorNr next[5][21] = {

        // Speed zone 0 - 3
        { 10,11,12,13,14,15,16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 10,11,12,13,14,15,16,17, 1, 0, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 10,11,12,13,14,15,16,17,18, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        { 10,11,12,13,14,15,16,17,18,19,20, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },

        // Directory track
        {  3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18, 0, 1, 2 }
    };

    if (t == 18) {

        // Take care of the directory track
        s = next[4][s];

        // Return immediately if we've wrapped over (directory track is full)
        if (s == 0) return {0,0};

    } else {

        // Take care of all other tracks
        s = next[speedZone(t)][s];

        // Move to the next track if we've wrapped over
        if (s == 0) {

            if (t >= numTracks()) return {0,0};
            t = t == 17 ? 19 : t + 1;
        }
    }

    assert(isValidLink(TSLink{t,s}));
    return TSLink{t,s};
}

}
