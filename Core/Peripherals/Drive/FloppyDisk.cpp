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
#include "AmigaEncoder.h"
#include "Codecs.h"
#include "IBMEncoder.h"
#include "Media.h"
#include "MediaFile.h"
#include "DeviceError.h"
#include "DiskEncoder.h"
#include "utl/io.h"

namespace vamiga {

void
FloppyDisk::init(Diameter dia, Density den, bool wp)
{
    diameter = dia;
    density = den;

    u32 numTrackBytes = 0;

    if (dia == Diameter::INCH_35  && den == Density::DD) numTrackBytes = 12668;
    if (dia == Diameter::INCH_35  && den == Density::HD) numTrackBytes = 24636;
    if (dia == Diameter::INCH_525 && den == Density::DD) numTrackBytes = 12668;

    if (numTrackBytes == 0 || FORCE_DISK_INVALID_LAYOUT) {
        throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
    }

    for (isize i = 0; i < 168; i++) {

        track[i] = MutableBitView(data.track[i], numTrackBytes * 8);
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

        isize oldlen = track[0].size();
        for (isize i = 0, oldi = 0; i <= numTracks(); i++) {

            isize newlen = i < numTracks() ? track[i].size() : -1;

            if (oldlen != newlen) {

                os << tab("Track " + std::to_string(oldi) + " - " + std::to_string(i));
                os << dec(oldlen) << " Bits" << std::endl;

                oldlen = newlen;
                oldi = i;
            }
        }
    }
}

void
FloppyDisk::readBlock(u8 *dst, isize nr) const
{
    debug(MFM_DEBUG, "readBlock: %ld\n", nr);

    auto [t,s]  = ts(nr);
    auto tdata  = track[t].byteView();
    auto offset = Encoder::amiga.trySeekSector(tdata, s);

    if (!offset)
        throw IOError(DeviceError::DEV_SEEK_ERR, "Block " + std::to_string(nr));

    debug(MFM_DEBUG, "Found (%ld,%ld) at offset %ld\n", t, s, *offset);
    Encoder::amiga.decodeSector(tdata, *offset, std::span<u8>(dst, 512));
}

void
FloppyDisk::writeBlock(const u8 *src, isize nr)
{
    debug(MFM_DEBUG, "writeBlock: %ld\n", nr);

    auto [t,s]  = ts(nr);
    auto tdata  = track[t].byteView();
    auto offset = Encoder::amiga.trySeekSector(tdata, s);

    if (!offset)
        throw IOError(DeviceError::DEV_SEEK_ERR, "Block " + std::to_string(nr));

    debug(MFM_DEBUG, "Found (%ld,%ld) at offset %ld\n", t, s, *offset);
    Encoder::amiga.encodeSector(tdata, *offset, t, s, std::span<const u8>(src, 512));
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
    return Hashable::fnv64(data.track[t], track[t].size() / 8);
}

u64
FloppyDisk::checksum(CylNr c, HeadNr h) const
{
    return checksum(c * numHeads() + h);
}

BitView
FloppyDisk::bitView(TrackNr t) const
{
    return BitView(data.track[t], track[t].size());
}

BitView
FloppyDisk::bitView(TrackNr t, SectorNr s) const
{
    return BitView(data.track[t] + s * 1088 * 8, 1088 * 8);
}

MutableBitView
FloppyDisk::bitView(TrackNr t)
{
    return MutableBitView(data.track[t], track[t].size());
}

MutableBitView
FloppyDisk::bitView(TrackNr t, SectorNr s)
{
    return MutableBitView(data.track[t] + s * 1088 * 8, 1088 * 8);
}

ByteView
FloppyDisk::byteView(TrackNr t) const
{
    return ByteView(data.track[t], track[t].size() / 8);
}

ByteView
FloppyDisk::byteView(TrackNr t, SectorNr s) const
{
    return ByteView(data.track[t] + s * 1088, 1088);
}

MutableByteView
FloppyDisk::byteView(TrackNr t)
{
    return MutableByteView(data.track[t], track[t].size() / 8);
}

MutableByteView
FloppyDisk::byteView(TrackNr t, SectorNr s)
{
    return MutableByteView(data.track[t] + s * 1088, 1088);
}

u8
FloppyDisk::read8(CylNr c, HeadNr h, isize offset) const
{
    isize t = 2 * c + h;

    assert(t >= 0 && t < numTracks());
    assert(offset >= 0 && offset < track[t].size());

    return track[t].getByte(offset);
}

void
FloppyDisk::write8(CylNr c, HeadNr h, isize offset, u8 value)
{
    isize t = 2 * c + h;

    assert(t >= 0 && t < numTracks());
    assert(offset >= 0 && offset < track[t].size());

    track[t].setByte(offset, value);
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
    for (usize i = 0; i < sizeof(data.track[t]); ++i) {
        data.track[t][i] = rand() & 0xFF;
    }
}

void
FloppyDisk::clearTrack(TrackNr t, u8 value)
{
    assert(t < numTracks());

    for (usize i = 0; i < sizeof(data.track[t]); ++i) {
        data.track[t][i] = value;
    }
}

void
FloppyDisk::clearTrack(TrackNr t, u8 value1, u8 value2)
{
    assert(t < numTracks());

    for (usize i = 0; i < sizeof(data.track[t]); ++i) {
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

        case FileType::ADF:  encode(dynamic_cast<const ADFFile &>(file)); break;
        case FileType::ADZ:  Codec::encodeADZ(dynamic_cast<const ADZFile &>(file), *this); break;
        case FileType::EADF: Codec::encodeEADF(dynamic_cast<const EADFFile &>(file), *this); break;
        case FileType::IMG:  Codec::encodeIMG(dynamic_cast<const IMGFile &>(file), *this); break;
        case FileType::ST:   Codec::encodeST(dynamic_cast<const STFile &>(file), *this); break;
        case FileType::DMS:  Codec::encodeDMS(dynamic_cast<const DMSFile &>(file), *this); break;
        case FileType::EXE:  Codec::encodeEXE(dynamic_cast<const EXEFile &>(file), *this); break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }
}

void
FloppyDisk::encode(const ADFFile &adf)
{
    isize tracks = adf.numTracks();
    if (ADF_DEBUG) fprintf(stderr, "Encoding Amiga disk with %ld tracks\n", tracks);

    if (getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        Encoder::amiga.encodeTrack(byteView(t), t, adf.byteView(t));

    // In debug mode, also run the decoder
    if (ADF_DEBUG) {

        string tmp = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeADF(*this)->writeToFile(tmp);
    }
}

void
FloppyDisk::decode(ADFFile &adf) const
{
    auto tracks = adf.numTracks();
    if (ADF_DEBUG) fprintf(stderr, "Decoding Amiga disk with %ld tracks\n", tracks);

    if (getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        Encoder::amiga.decodeTrack(byteView(t), t, adf.byteView(t));
}

void
FloppyDisk::encode(const class IMGFile &img)
{
    isize tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS disk with %ld tracks\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        Encoder::ibm.encodeTrack(byteView(t), t, img.byteView(t));

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        string tmp = "/tmp/debug.img";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeIMG(*this)->writeToFile(tmp);
    }
}

void
FloppyDisk::decode(class IMGFile &img) const
{
    auto tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Decoding DOS disk (%ld tracks)\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        Encoder::ibm.decodeTrack(byteView(t), t, img.byteView(t));
}

void
FloppyDisk::encode(const class STFile &img)
{
    isize tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS disk with %ld tracks\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        Encoder::ibm.encodeTrack(byteView(t), t, img.byteView(t));

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        string tmp = "/tmp/debug.img";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeIMG(*this)->writeToFile(tmp);
    }
}

void
FloppyDisk::decode(class STFile &img) const
{
    auto tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Decoding Atari ST disk (%ld tracks)\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        Encoder::ibm.decodeTrack(byteView(t), t, img.byteView(t));
}

void
FloppyDisk::shiftTracks(isize offset)
{
    debug(DSK_DEBUG, "Shifting tracks by %ld bytes against each other\n", offset);

    u8 spare[2 * 32768];

    for (TrackNr t = 0; t < 168; t++) {

        // Right now, this function only works on tracks with byte-aligned sizes
        assert(track[t].size() % 8 == 0);

        isize len = track[t].size() / 8;
        memcpy(spare, data.track[t], len);
        memcpy(spare + len, data.track[t], len);
        memcpy(data.track[t], spare + (len + t * offset) % len, len);
    }
}

void
FloppyDisk::repeatTracks()
{
    for (TrackNr t = 0; t < 168; t++) {

        assert(track[t].size() % 8 == 0);

        auto end = isize(track[t].size() / 8);
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
    assert(track[t].size() % 8 == 0);

    string result;
    result.reserve(track[t].size() / 8);

    for (isize i = 0; i < track[t].size() / 8; i++) {
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
