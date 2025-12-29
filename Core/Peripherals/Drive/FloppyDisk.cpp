// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FloppyDisk.h"
#include "Media.h"
#include "MediaFile.h"
#include "DeviceError.h"
#include "DiskEncoder.h"
#include "utl/io.h"

namespace vamiga {

/*
FloppyDisk::FloppyDisk()
{
    for (isize i = 0; i < 84; ++i) {
        track[i] = MutableByteView(data.track[168], 0);
    }
}
*/

void
FloppyDisk::init(Diameter dia, Density den, bool wp)
{
    diameter = dia;
    density = den;

    u32 trackLength = 0;

    if (dia == Diameter::INCH_35  && den == Density::DD) trackLength = 12668;
    if (dia == Diameter::INCH_35  && den == Density::HD) trackLength = 24636;
    if (dia == Diameter::INCH_525 && den == Density::DD) trackLength = 12668;

    if (trackLength == 0 || FORCE_DISK_INVALID_LAYOUT) {
        throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
    }

    for (isize i = 0; i < 168; i++) {

        track[i] = MutableBitView(data.track[i], trackLength * 8);
        length.track[i] = trackLength;
    }
    clearDisk();
    setWriteProtection(wp);
}

void
FloppyDisk::init(const class FloppyDiskImage &file, bool wp)
{
    init(file.getDiameter(), file.getDensity(), wp);
    encodeDisk(file);
}

void
FloppyDisk::init(unique_ptr<FloppyDiskImage> file, bool wp)
{
    init(*file, wp);
}

void
FloppyDisk::init(SerReader &reader, Diameter dia, Density den, bool wp)
{
    init(dia, den, wp);
    serialize(reader);
}

FloppyDisk::~FloppyDisk()
{
    debug(OBJ_DEBUG, "Deleting disk\n");
}

void
FloppyDisk::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    if (category == Category::State) {

        os << tab("Type");
        os << DiameterEnum::key(diameter) << std::endl;
        os << tab("Density");
        os << DensityEnum::key(density) << std::endl;
        os << tab("Flags");
        os << DiskFlagsEnum::mask(flags) << std::endl;
        os << tab("numCyls()");
        os << dec(numCyls()) << std::endl;
        os << tab("numHeads()");
        os << dec(numHeads()) << std::endl;
        os << tab("numTracks()");
        os << dec(numTracks()) << std::endl;

        isize oldlen = length.track[0];
        for (isize i = 0, oldi = 0; i <= numTracks(); i++) {

            isize newlen = i < numTracks() ? length.track[i] : -1;

            if (oldlen != newlen) {

                os << tab("Track " + std::to_string(oldi) + " - " + std::to_string(i));
                os << dec(oldlen) << " Bytes" << std::endl;

                oldlen = newlen;
                oldi = i;
            }
        }
    }
}

void
FloppyDisk::readBlock(u8 *dst, isize nr) const
{
    fatalError;
}

void
FloppyDisk::writeBlock(const u8 *src, isize nr)
{
    fatalError;
}

void
FloppyDisk::readTrack(u8 *dst, isize nr) const
{
    fatalError;
}

void
FloppyDisk::writeTrack(const u8 *src, isize nr)
{
    fatalError;
}

bool
FloppyDisk::isValidHeadPos(TrackNr t, isize offset) const
{
    assert(length.track[t] * 8 == track[t].size() );
    return isValidTrackNr(t) && offset >= 0 && offset < 8 * length.track[t];
}

bool
FloppyDisk::isValidHeadPos(CylNr c, HeadNr h, isize offset) const
{
    assert(length.cylinder[c][h] * 8 == track[2*c+h].size());
    return isValidCylinderNr(c) && isValidHeadNr(h) && offset >= 0 && offset < 8 * length.cylinder[c][h];
}

u64
FloppyDisk::checksum() const
{
    auto result = Hashable::fnvInit64();

    for (TrackNr t = 0; t < numTracks(); t++) {
        result = Hashable::fnvIt64(result, checksum(t));
    }

    return result;
}

u64
FloppyDisk::checksum(TrackNr t) const
{
    assert(length.track[t] * 8 == track[t].size() );
    return Hashable::fnv64(data.track[t], length.track[t]);
}

u64
FloppyDisk::checksum(CylNr c, HeadNr h) const
{
    return checksum(c * numHeads() + h);
}


ByteView
FloppyDisk::byteView(TrackNr t) const
{
    assert(length.track[t] * 8 == track[t].size() );
    return ByteView(data.track[t], length.track[t]);
}

ByteView
FloppyDisk::byteView(TrackNr t, SectorNr s) const
{
    return ByteView(data.track[t] + s * 1088, 1088);
}

MutableByteView
FloppyDisk::byteView(TrackNr t)
{
    assert(length.track[t] * 8 == track[t].size() );
    return MutableByteView(data.track[t], length.track[t]);
}

MutableByteView
FloppyDisk::byteView(TrackNr t, SectorNr s)
{
    return MutableByteView(data.track[t] + s * 1088, 1088);
}

u8
FloppyDisk::readBit(TrackNr t, isize offset) const
{
    assert(isValidHeadPos(t, offset));

    return (data.track[t][offset / 8] & (0x80 >> (offset & 7))) != 0;
}

u8
FloppyDisk::readBit(CylNr c, HeadNr h, isize offset) const
{
    assert(isValidHeadPos(c, h, offset));

    return (data.cylinder[c][h][offset / 8] & (0x80 >> (offset & 7))) != 0;
}

void
FloppyDisk::writeBit(TrackNr t, isize offset, bool value) {

    assert(isValidHeadPos(t, offset));

    if (value) {
        data.track[t][offset / 8] |= (0x0080 >> (offset & 7));
    } else {
        data.track[t][offset / 8] &= (0xFF7F >> (offset & 7));
    }
}

void
FloppyDisk::writeBit(CylNr c, HeadNr h, isize offset, bool value) {

    assert(isValidHeadPos(c, h, offset));

    if (value) {
        data.cylinder[h][c][offset / 8] |= (0x0080 >> (offset & 7));
    } else {
        data.cylinder[h][c][offset / 8] &= (0xFF7F >> (offset & 7));
    }
}

u8
FloppyDisk::read8(TrackNr t, isize offset) const
{
    assert(t < numTracks());
    assert(offset < length.track[t]);
    assert(length.track[t] * 8 == track[t].size() );

    return data.track[t][offset];
}

u8
FloppyDisk::read8(CylNr c, HeadNr h, isize offset) const
{
    assert(c < numCyls());
    assert(h < numHeads());
    assert(offset < length.cylinder[c][h]);
    assert(length.cylinder[c][h] * 8 == track[2*c+h].size());

    return data.cylinder[c][h][offset];
}

void
FloppyDisk::write8(TrackNr t, isize offset, u8 value)
{
    assert(t < numTracks());
    assert(offset < length.track[t]);
    assert(length.track[t] * 8 == track[t].size() );

    data.track[t][offset] = value;
    setModified(true);
}

void
FloppyDisk::write8(CylNr c, HeadNr h, isize offset, u8 value)
{
    assert(c < numCyls());
    assert(h < numHeads());
    assert(offset < length.cylinder[c][h]);
    assert(length.cylinder[c][h] * 8 == track[2*c+h].size());

    data.cylinder[c][h][offset] = value;
    setModified(true);
}

void
FloppyDisk::clearDisk()
{
    setModified(FORCE_DISK_MODIFIED);

    // Initialize with random data
    srand(0);
    for (usize i = 0; i < sizeof(data.raw); i++) {
        data.raw[i] = rand() & 0xFF;
    }
    
    /* In order to make some copy protected game titles work, we smuggle in
     * some magic values. E.g., Crunch factory expects 0x44A2 on cylinder 80.
     */
    if (diameter == Diameter::INCH_35 && density == Density::DD) {
        
        for (isize t = 0; t < numTracks(); t++) {
            data.track[t][0] = 0x44;
            data.track[t][1] = 0xA2;
        }
    }
}

void
FloppyDisk::clearDisk(u8 value)
{
    for (usize i = 0; i < sizeof(data.raw); i++) {
        data.raw[i] = value;
    }
}

void
FloppyDisk::clearTrack(TrackNr t)
{
    assert(t < numTracks());

    srand(0);
    for (isize i = 0; i < length.track[t]; i++) {
        assert(length.track[t] * 8 == track[t].size() );
        data.track[t][i] = rand() & 0xFF;
    }
}

void
FloppyDisk::clearTrack(TrackNr t, u8 value)
{
    assert(t < numTracks());

    for (usize i = 0; i < sizeof(data.track[t]); i++) {
        data.track[t][i] = value;
    }
}

void
FloppyDisk::clearTrack(TrackNr t, u8 value1, u8 value2)
{
    assert(t < numTracks());

    for (isize i = 0; i < length.track[t]; i++) {
        assert(length.track[t] * 8 == track[t].size() );
        data.track[t][i] = IS_ODD(i) ? value2 : value1;
    }
}

void
FloppyDisk::encodeDisk(const FloppyDiskImage &file)
{
    assert(file.getDiameter() == getDiameter());

    // Start with an unformatted disk
    clearDisk();

    switch (MediaFile::type(file)) {
    // switch (file.type()) {

        case FileType::ADF:  DiskEncoder::encode(dynamic_cast<const ADFFile &>(file), *this); break;
        case FileType::ADZ:  ADZEncoder::encode(dynamic_cast<const ADZFile &>(file), *this); break;
        case FileType::EADF: EADFEncoder::encode(dynamic_cast<const EADFFile &>(file), *this); break;
        case FileType::IMG:  IMGEncoder::encode(dynamic_cast<const IMGFile &>(file), *this); break;
        case FileType::ST:   STEncoder::encode(dynamic_cast<const STFile &>(file), *this); break;
        case FileType::DMS:  DMSEncoder::encode(dynamic_cast<const DMSFile &>(file), *this); break;
        case FileType::EXE:  EXEEncoder::encode(dynamic_cast<const EXEFile &>(file), *this); break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }
}

void
FloppyDisk::shiftTracks(isize offset)
{
    debug(DSK_DEBUG, "Shifting tracks by %ld bytes against each other\n", offset);

    u8 spare[2 * 32768];

    for (TrackNr t = 0; t < 168; t++) {

        isize len = length.track[t];
        assert(length.track[t] * 8 == track[t].size() );

        memcpy(spare, data.track[t], len);
        memcpy(spare + len, data.track[t], len);
        memcpy(data.track[t], spare + (len + t * offset) % len, len);
    }
}

void
FloppyDisk::repeatTracks()
{
    for (TrackNr t = 0; t < 168; t++) {
        
        auto end = isize(length.track[t]);
        assert(length.track[t] * 8 == track[t].size() );
        auto max = isize(sizeof(data.track[t]));

        for (isize i = end, j = 0; i < max; i++, j++) {
            data.track[t][i] = data.track[t][j];
        }
    }
}

string
FloppyDisk::readTrackBits(TrackNr t) const
{
    assert(t < numTracks());

    string result;
    result.reserve(length.track[t]);
    assert(length.track[t] * 8 == track[t].size() );

    for (isize i = 0; i < length.track[t]; i++) {
        for (isize j = 7; j >= 0; j--) {
            result += GET_BIT(data.track[t][i], j) ? '1' : '0';
        }
    }
    
    return result;
}

string
FloppyDisk::readTrackBits(CylNr c, HeadNr h) const
{
    return readTrackBits(2 * c + h);
}

}
