// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FloppyDisk.h"
#include "FloppyFile.h"

void
FloppyDisk::init(Diameter dia, Density den)
{
    diameter = dia;
    density = den;
    
    u32 trackLength = 0;
    
    if (dia == INCH_35  && den == DENSITY_DD) trackLength = 12668;
    if (dia == INCH_35  && den == DENSITY_HD) trackLength = 24636;
    if (dia == INCH_525 && den == DENSITY_DD) trackLength = 12668;
    
    if (trackLength == 0 || FORCE_DISK_INVALID_LAYOUT) {
        throw VAError(ERROR_DISK_INVALID_LAYOUT);
    }
    
    for (isize i = 0; i < 168; i++) length.track[i] = trackLength;
    clearDisk();
}

void
FloppyDisk::init(const class FloppyFile &file)
{
    init(file.getDiameter(), file.getDensity());
    encodeDisk(file);
}

void
FloppyDisk::init(util::SerReader &reader, Diameter dia, Density den)
{
    init(dia, den);
    applyToPersistentItems(reader);
}

FloppyDisk::~FloppyDisk()
{
    debug(OBJ_DEBUG, "Deleting disk\n");
}

void
FloppyDisk::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::State) {
        
        os << tab("Type");
        os << DiameterEnum::key(diameter) << std::endl;
        os << tab("Density");
        os << DensityEnum::key(density) << std::endl;
        os << tab("numCyls()");
        os << dec(numCyls()) << std::endl;
        os << tab("numHeads()");
        os << dec(numHeads()) << std::endl;
        os << tab("numTracks()");
        os << dec(numTracks()) << std::endl;
        os << tab("Track 0 length");
        os << dec(length.track[0]) << std::endl;
        os << tab("Write protected");
        os << bol(writeProtected) << std::endl;
        os << tab("Modified");
        os << bol(modified) << std::endl;
        os << tab("FNV checksum");
        os << hex(fnv) << " / " << dec(fnv) << std::endl;
    }
}

u8
FloppyDisk::readByte(Track t, isize offset) const
{
    assert(t < numTracks());
    assert(offset < length.track[t]);

    return data.track[t][offset];
}

u8
FloppyDisk::readByte(Cylinder c, Head h, isize offset) const
{
    assert(c < numCyls());
    assert(h < numHeads());
    assert(offset < length.cylinder[c][h]);

    return data.cylinder[c][h][offset];
}

void
FloppyDisk::writeByte(u8 value, Track t, isize offset)
{
    assert(t < numTracks());
    assert(offset < length.track[t]);

    data.track[t][offset] = value;
    modified = true;
}

void
FloppyDisk::writeByte(u8 value, Cylinder c, Head h, isize offset)
{
    assert(c < numCyls());
    assert(h < numHeads());
    assert(offset < length.cylinder[c][h]);

    data.cylinder[c][h][offset] = value;
    modified = true;
}

void
FloppyDisk::clearDisk()
{
    fnv = 0;
    modified = bool(FORCE_DISK_MODIFIED);
    
    // Initialize with random data
    srand(0);
    for (isize i = 0; i < isizeof(data.raw); i++) {
        data.raw[i] = rand() & 0xFF;
    }
    
    /* In order to make some copy protected game titles work, we smuggle in
     * some magic values. E.g., Crunch factory expects 0x44A2 on cylinder 80.
     */
    if (diameter == INCH_35 && density == DENSITY_DD) {
        
        for (isize t = 0; t < numTracks(); t++) {
            data.track[t][0] = 0x44;
            data.track[t][1] = 0xA2;
        }
    }
}

void
FloppyDisk::clearDisk(u8 value)
{
    for (isize i = 0; i < isizeof(data.raw); i++) {
        data.raw[i] = value;
    }
}

void
FloppyDisk::clearTrack(Track t)
{
    assert(t < numTracks());

    srand(0);
    for (isize i = 0; i < length.track[t]; i++) {
        data.track[t][i] = rand() & 0xFF;
    }
}

void
FloppyDisk::clearTrack(Track t, u8 value)
{
    assert(t < numTracks());

    for (isize i = 0; i < isizeof(data.track[t]); i++) {
        data.track[t][i] = value;
    }
}

void
FloppyDisk::clearTrack(Track t, u8 value1, u8 value2)
{
    assert(t < numTracks());

    for (isize i = 0; i < length.track[t]; i++) {
        data.track[t][i] = IS_ODD(i) ? value2 : value1;
    }
}

void
FloppyDisk::encodeDisk(const FloppyFile &file)
{
    assert(file.getDiameter() == getDiameter());

    // Start with an unformatted disk
    clearDisk();

    // Call the MFM encoder
    file.encodeDisk(*this);
}

void
FloppyDisk::encodeMFM(u8 *dst, u8 *src, isize count)
{
    for(isize i = 0; i < count; i++) {
        
        auto mfm =
        ((src[i] & 0b10000000) << 7) |
        ((src[i] & 0b01000000) << 6) |
        ((src[i] & 0b00100000) << 5) |
        ((src[i] & 0b00010000) << 4) |
        ((src[i] & 0b00001000) << 3) |
        ((src[i] & 0b00000100) << 2) |
        ((src[i] & 0b00000010) << 1) |
        ((src[i] & 0b00000001) << 0);
        
        dst[2*i+0] = HI_BYTE(mfm);
        dst[2*i+1] = LO_BYTE(mfm);
    }
}

void
FloppyDisk::decodeMFM(u8 *dst, u8 *src, isize count)
{
    for(isize i = 0; i < count; i++) {
        
        u16 mfm = HI_LO(src[2*i], src[2*i+1]);
        
        auto decoded =
        ((mfm & 0b0100000000000000) >> 7) |
        ((mfm & 0b0001000000000000) >> 6) |
        ((mfm & 0b0000010000000000) >> 5) |
        ((mfm & 0b0000000100000000) >> 4) |
        ((mfm & 0b0000000001000000) >> 3) |
        ((mfm & 0b0000000000010000) >> 2) |
        ((mfm & 0b0000000000000100) >> 1) |
        ((mfm & 0b0000000000000001) >> 0);
        
        dst[i] = (u8)decoded;
    }
}

void
FloppyDisk::encodeOddEven(u8 *dst, u8 *src, isize count)
{
    // Encode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (src[i] >> 1) & 0x55;
    
    // Encode even bits
    for(isize i = 0; i < count; i++)
        dst[i + count] = src[i] & 0x55;
}

void
FloppyDisk::decodeOddEven(u8 *dst, u8 *src, isize count)
{
    // Decode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (u8)((src[i] & 0x55) << 1);
    
    // Decode even bits
    for(isize i = 0; i < count; i++)
        dst[i] |= src[i + count] & 0x55;
}

void
FloppyDisk::addClockBits(u8 *dst, isize count)
{
    for (isize i = 0; i < count; i++) {
        dst[i] = addClockBits(dst[i], dst[i-1]);
    }
}

u8
FloppyDisk::addClockBits(u8 value, u8 previous)
{
    // Clear all previously set clock bits
    value &= 0x55;

    // Compute clock bits (clock bit values are inverted)
    u8 lShifted = (u8)(value << 1);
    u8 rShifted = (u8)(value >> 1 | previous << 7);
    u8 cBitsInv = (u8)(lShifted | rShifted);

    // Reverse the computed clock bits
    u8 cBits = cBitsInv ^ 0xAA;
    
    // Return original value with the clock bits added
    return value | cBits;
}

void
FloppyDisk::repeatTracks()
{
    for (Track t = 0; t < 168; t++) {
        
        isize end = length.track[t];
        for (isize i = end, j = 0; i < isizeof(data.track[t]); i++, j++) {
            data.track[t][i] = data.track[t][j];
        }
    }
}

string
FloppyDisk::readTrackBits(Track t) const
{
    assert(t < numTracks());

    string result;
    result.reserve(length.track[t]);

    for (isize i = 0; i < length.track[t]; i++) {
        for (isize j = 7; j >= 0; j--) {
            result += GET_BIT(data.track[t][i], j) ? '1' : '0';
        }
    }
    
    return result;
}

string
FloppyDisk::readTrackBits(Cylinder c, Head h) const
{
    return readTrackBits(2 * c + h);
}
