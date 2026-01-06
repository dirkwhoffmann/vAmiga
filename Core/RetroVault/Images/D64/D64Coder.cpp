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
#include "utl/io.h"
#include "utl/support/Strings.h"
#include <format>

namespace retro::vault::image {

// Disk parameters of a standard floppy disk
typedef struct
{
    u8     sectors;         // Typical number of sectors in this track
    u8     speedZone;       // Default speed zone for this track
    u16    lengthInBytes;   // Typical track size in bits
    u16    lengthInBits;    // Typical track size in bits
    isize  firstSectorNr;   // Logical number of first sector in track
    double stagger;         // Relative position of first bit (from Hoxs64)
}
TrackDefaults;

static constexpr TrackDefaults trackDefaults[43] = {

    { 0, 0, 0, 0, 0, 0 },                       // Padding

    // Speedzone 3 (outer tracks)
    { 21, 3, 7693, 7693 * 8,   0, 0.268956 },   // Track 1
    { 21, 3, 7693, 7693 * 8,  21, 0.724382 },   // Track 2
    { 21, 3, 7693, 7693 * 8,  42, 0.177191 },   // Track 3
    { 21, 3, 7693, 7693 * 8,  63, 0.632698 },   // Track 4
    { 21, 3, 7693, 7693 * 8,  84, 0.088173 },   // Track 5
    { 21, 3, 7693, 7693 * 8, 105, 0.543583 },   // Track 6
    { 21, 3, 7693, 7693 * 8, 126, 0.996409 },   // Track 7
    { 21, 3, 7693, 7693 * 8, 147, 0.451883 },   // Track 8
    { 21, 3, 7693, 7693 * 8, 168, 0.907342 },   // Track 9
    { 21, 3, 7693, 7693 * 8, 289, 0.362768 },   // Track 10
    { 21, 3, 7693, 7693 * 8, 210, 0.815512 },   // Track 11
    { 21, 3, 7693, 7693 * 8, 231, 0.268338 },   // Track 12
    { 21, 3, 7693, 7693 * 8, 252, 0.723813 },   // Track 13
    { 21, 3, 7693, 7693 * 8, 273, 0.179288 },   // Track 14
    { 21, 3, 7693, 7693 * 8, 294, 0.634779 },   // Track 15
    { 21, 3, 7693, 7693 * 8, 315, 0.090253 },   // Track 16
    { 21, 3, 7693, 7693 * 8, 336, 0.545712 },   // Track 17

    // Speedzone 2
    { 19, 2, 7143, 7143 * 8, 357, 0.945418 },   // Track 18
    { 19, 2, 7143, 7143 * 8, 376, 0.506081 },   // Track 19
    { 19, 2, 7143, 7143 * 8, 395, 0.066622 },   // Track 20
    { 19, 2, 7143, 7143 * 8, 414, 0.627303 },   // Track 21
    { 19, 2, 7143, 7143 * 8, 433, 0.187862 },   // Track 22
    { 19, 2, 7143, 7143 * 8, 452, 0.748403 },   // Track 23
    { 19, 2, 7143, 7143 * 8, 471, 0.308962 },   // Track 24

    // Speedzone 1
    { 18, 1, 6667, 6667 * 8, 490, 0.116926 },   // Track 25
    { 18, 1, 6667, 6667 * 8, 508, 0.788086 },   // Track 26
    { 18, 1, 6667, 6667 * 8, 526, 0.459190 },   // Track 27
    { 18, 1, 6667, 6667 * 8, 544, 0.130238 },   // Track 28
    { 18, 1, 6667, 6667 * 8, 562, 0.801286 },   // Track 29
    { 18, 1, 6667, 6667 * 8, 580, 0.472353 },   // Track 30

    // Speedzone 0 (inner tracks)
    { 17, 0, 6250, 6250 * 8, 598, 0.834120 },   // Track 31
    { 17, 0, 6250, 6250 * 8, 615, 0.614880 },   // Track 32
    { 17, 0, 6250, 6250 * 8, 632, 0.395480 },   // Track 33
    { 17, 0, 6250, 6250 * 8, 649, 0.176140 },   // Track 34
    { 17, 0, 6250, 6250 * 8, 666, 0.956800 },   // Track 35

    // Speedzone 0 (usually unused tracks)
    { 17, 0, 6250, 6250 * 8, 683, 0.300 },      // Track 36
    { 17, 0, 6250, 6250 * 8, 700, 0.820 },      // Track 37
    { 17, 0, 6250, 6250 * 8, 717, 0.420 },      // Track 38
    { 17, 0, 6250, 6250 * 8, 734, 0.940 },      // Track 39
    { 17, 0, 6250, 6250 * 8, 751, 0.540 },      // Track 40
    { 17, 0, 6250, 6250 * 8, 768, 0.130 },      // Track 41
    { 17, 0, 6250, 6250 * 8, 785, 0.830 }       // Track 42
};

BitView
D64File::encode(TrackNr t) const
{
    // TO BE IMPLEMENTED. WE RETURN AN EMPTY TRACK FOR NOW
    static constexpr size_t TrackSize = 6250;
     static const std::array<uint8_t, TrackSize> emptyTrack = [] {
         std::array<uint8_t, TrackSize> data{};
         data.fill(0x55);
         return data;
     }();

     return BitView(emptyTrack.data(), emptyTrack.size() * 8);
}

isize
D64File::encodeSector(TrackNr t, SectorNr s, isize offset) const
{
    return 0;
}

void
D64File::decode(TrackNr t, BitView bits)
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

}
