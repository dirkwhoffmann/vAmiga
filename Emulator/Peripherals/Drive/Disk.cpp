// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Disk.h"
#include "DiskFile.h"

void
Disk::init(DiskDiameter dia, DiskDensity den)
{
    diameter = dia;
    density = den;
    
    u32 trackLength = 0;
    
    if (dia == INCH_35  && den == DISK_DD) trackLength = 12668;
    if (dia == INCH_35  && den == DISK_HD) trackLength = 24636;
    if (dia == INCH_525 && den == DISK_DD) trackLength = 12668;
    
    if (trackLength == 0 || FORCE_DISK_INVALID_LAYOUT) {
        throw VAError(ERROR_DISK_INVALID_LAYOUT);
    }
    
    for (isize i = 0; i < 168; i++) length.track[i] = trackLength;
    clearDisk();
}

void
Disk::init(const class DiskFile &file)
{
    init(file.getDiskDiameter(), file.getDiskDensity());
    encodeDisk(file);
}

void
Disk::init(util::SerReader &reader, DiskDiameter dia, DiskDensity den)
{
    init(dia, den);
    applyToPersistentItems(reader);
}

Disk::~Disk()
{
    debug(OBJ_DEBUG, "Deleting disk\n");
}

void
Disk::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
        
        os << tab("Type");
        os << DiskDiameterEnum::key(diameter) << std::endl;
        os << tab("Density");
        os << DiskDensityEnum::key(density) << std::endl;
        os << tab("numCyls()");
        os << dec(numCyls()) << std::endl;
        os << tab("numSides()");
        os << dec(numSides()) << std::endl;
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
Disk::readByte(Track t, isize offset) const
{
    assert(t < numTracks());
    assert(offset < length.track[t]);

    return data.track[t][offset];
}

u8
Disk::readByte(Cylinder c, Side s, isize offset) const
{
    assert(c < numCyls());
    assert(s < numSides());
    assert(offset < length.cylinder[c][s]);

    return data.cylinder[c][s][offset];
}

void
Disk::writeByte(u8 value, Track t, isize offset)
{
    assert(t < numTracks());
    assert(offset < length.track[t]);

    data.track[t][offset] = value;
}

void
Disk::writeByte(u8 value, Cylinder c, Side s, isize offset)
{
    assert(c < numCyls());
    assert(s < numSides());
    assert(offset < length.cylinder[c][s]);

    data.cylinder[c][s][offset] = value;
}

void
Disk::clearDisk()
{
    fnv = 0;

    // Initialize with random data
    srand(0);
    for (isize i = 0; i < isizeof(data.raw); i++) {
        data.raw[i] = rand() & 0xFF;
    }
    
    /* In order to make some copy protected game titles work, we smuggle in
     * some magic values. E.g., Crunch factory expects 0x44A2 on cylinder 80.
     */
    if (diameter == INCH_35 && density == DISK_DD) {
        
        for (isize t = 0; t < numTracks(); t++) {
            data.track[t][0] = 0x44;
            data.track[t][1] = 0xA2;
        }
    }
}

void
Disk::clearTrack(Track t)
{
    assert(t < numTracks());

    srand(0);
    for (isize i = 0; i < length.track[t]; i++) {
        data.track[t][i] = rand() & 0xFF;
    }
}

void
Disk::clearTrack(Track t, u8 value)
{
    assert(t < numTracks());

    for (isize i = 0; i < isizeof(data.track[t]); i++) {
        data.track[t][i] = value;
    }
}

void
Disk::clearTrack(Track t, u8 value1, u8 value2)
{
    assert(t < numTracks());

    for (isize i = 0; i < length.track[t]; i++) {
        data.track[t][i] = IS_ODD(i) ? value2 : value1;
    }
}

void
Disk::encodeDisk(const DiskFile &file)
{
    assert(file.getDiskDiameter() == getDiameter());

    // Start with an unformatted disk
    clearDisk();

    // Call the MFM encoder
    file.encodeDisk(*this);
}

void
Disk::encodeMFM(u8 *dst, u8 *src, isize count)
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
Disk::decodeMFM(u8 *dst, u8 *src, isize count)
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
Disk::encodeOddEven(u8 *dst, u8 *src, isize count)
{
    // Encode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (src[i] >> 1) & 0x55;
    
    // Encode even bits
    for(isize i = 0; i < count; i++)
        dst[i + count] = src[i] & 0x55;
}

void
Disk::decodeOddEven(u8 *dst, u8 *src, isize count)
{
    // Decode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (u8)((src[i] & 0x55) << 1);
    
    // Decode even bits
    for(isize i = 0; i < count; i++)
        dst[i] |= src[i + count] & 0x55;
}

void
Disk::addClockBits(u8 *dst, isize count)
{
    for (isize i = 0; i < count; i++) {
        dst[i] = addClockBits(dst[i], dst[i-1]);
    }
}

u8
Disk::addClockBits(u8 value, u8 previous)
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
Disk::repeatTracks()
{
    for (Track t = 0; t < 168; t++) {
        
        isize end = length.track[t];
        for (isize i = end, j = 0; i < isizeof(data.track[t]); i++, j++) {
            data.track[t][i] = data.track[t][j];
        }
    }
}
