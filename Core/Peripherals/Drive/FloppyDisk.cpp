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
#include "ImageTypes.h"
#include "ADFFile.h"
#include "AmigaEncoder.h"
#include "AmigaDecoder.h"
#include "Codecs.h"
#include "DOSEncoder.h"
#include "DOSDecoder.h"
#include "Media.h"
#include "DeviceError.h"
#include "DiskEncoder.h"
#include "utl/io.h"

namespace vamiga {

using namespace retro::vault;

void
FloppyDisk::init(Diameter dia, Density den, bool wp)
{
    diameter = dia;
    density = den;

    u32 numTrackBytes = 0;

    if (dia == Diameter::INCH_35  && den == Density::DD) numTrackBytes = 12668;
    if (dia == Diameter::INCH_35  && den == Density::HD) numTrackBytes = 24636;
    if (dia == Diameter::INCH_525 && den == Density::DD) numTrackBytes = 12668;

    if (numTrackBytes == 0 || force::DISK_INVALID_LAYOUT) {
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
    loginfo(OBJ_DEBUG, "Deleting disk\n");
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
    AmigaDecoder decoder;

    auto [t,s] = b2ts(nr);
    loginfo(MFM_DEBUG, "readBlock: %ld (%ld,%ld)\n", nr, t, s);

    auto bytes = decoder.decodeSector(track[t], t, s);
    assert(bytes.size() == bsize());
    memcpy(dst, bytes.data(), bytes.size());
}

void
FloppyDisk::writeBlock(const u8 *src, isize nr)
{
    AmigaEncoder encoder;
    AmigaDecoder decoder;

    auto [t,s]  = b2ts(nr);
    loginfo(MFM_DEBUG, "writeBlock: %ld (%ld,%ld)\n", nr, t, s);

    // Compute the MFM bit stream
    auto mfm = encoder.encodeSector(ByteView(src, bsize()), t, s);

    // Locate the sector inside the track
    auto sector = decoder.seekSectorNew(track[t], s);

    if (!sector.has_value())
        throw IOError(DeviceError::SEEK_ERR, "Block " + std::to_string(nr));

    auto tr = track[t];
    auto it = track[t].cyclic_begin() + sector->lower;

    // Replace the sector data
    assert(mfm.size() == (*sector).size());
    for (isize i = 0; i < mfm.size(); ++i, ++it)
        tr.set(it.offset(), mfm[i]);

    // Rectify clock bits
    encoder.rectifyClockBit(tr, sector->lower);
    encoder.rectifyClockBit(tr, sector->upper);
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
    setModified(force::DISK_MODIFIED);

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
FloppyDisk::encodeDisk(const FloppyDiskImage &image)
{
    loginfo(DSK_DEBUG,
            "Encoding floppy disk image %s...\n", image.path.string().c_str());

    if (getDiameter() != image.getDiameter())
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);

    if (getDensity() != image.getDensity())
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < image.numTracks(); ++t)
        replaceTrack(t, image.encode(t));

    // In debug mode, also run the decoder
    /*
    if constexpr (debug::ADF_DEBUG) {

        string tmp = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeADF(*this)->writeToFile(tmp);
    }
    */

    /*
    switch (image.format()) {

        case ImageFormat::ADF:  encode(dynamic_cast<const ADFFile &>(image)); break;
        case ImageFormat::ADZ:  Codec::encodeADZ(dynamic_cast<const ADZFile &>(image), *this); break;
        case ImageFormat::EADF: Codec::encodeEADF(dynamic_cast<const EADFFile &>(image), *this); break;
        case ImageFormat::IMG:  encode(dynamic_cast<const IMGFile &>(image)); break;
        case ImageFormat::ST:   encode(dynamic_cast<const STFile &>(image)); break;
        case ImageFormat::DMS:  Codec::encodeDMS(dynamic_cast<const DMSFile &>(image), *this); break;
        case ImageFormat::EXE:  Codec::encodeEXE(dynamic_cast<const EXEFile &>(image), *this); break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }
    */
}

void
FloppyDisk::encode(const ADFFile &adf)
{
    isize tracks = adf.numTracks();
    loginfo(IMG_DEBUG, "Encoding Amiga disk with %ld tracks\n", tracks);

    if (getDiameter() != adf.getDiameter())
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);

    if (getDensity() != adf.getDensity())
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t) replaceTrack(t, adf.encode(t));

    // In debug mode, also run the decoder
    if constexpr (debug::IMG_DEBUG) {

        string tmp = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeADF(*this)->writeToFile(tmp);
    }
}

void
FloppyDisk::decode(ADFFile &adf) const
{
    auto tracks = adf.numTracks();

    loginfo(ADF_DEBUG, "Decoding Amiga disk with %ld tracks\n", tracks);

    if (getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t) adf.decode(t, track[t]);
}

void
FloppyDisk::encode(const class IMGFile &img)
{
    isize tracks = img.numTracks();

    loginfo(IMG_DEBUG, "Encoding DOS disk with %ld tracks\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t) replaceTrack(t, img.encode(t));

    // In debug mode, also run the decoder
    if constexpr (debug::IMG_DEBUG) {

        string tmp = "/tmp/debug.img";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeIMG(*this)->writeToFile(tmp);
    }
}

void
FloppyDisk::decode(class IMGFile &img) const
{
    auto tracks = img.numTracks();

    loginfo(IMG_DEBUG, "Decoding DOS disk (%ld tracks)\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t) img.decode(t, track[t]);
}

void
FloppyDisk::encode(const class STFile &img)
{
    isize tracks = img.numTracks();
    loginfo(IMG_DEBUG, "Encoding ST disk with %ld tracks\n", tracks);

    if (getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t) replaceTrack(t, img.encode(t));

    // In debug mode, also run the decoder
    if constexpr (debug::IMG_DEBUG) {

        string tmp = "/tmp/debug.img";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        Codec::makeIMG(*this)->writeToFile(tmp);
    }
}

void
FloppyDisk::decode(class STFile &st) const
{
    auto tracks = st.numTracks();

    loginfo(IMG_DEBUG, "Decoding Atari ST disk (%ld tracks)\n", tracks);

    if (getDiameter() != st.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (getDensity() != st.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t) st.decode(t, track[t]);
}

void
FloppyDisk::replaceTrack(TrackNr t, BitView mfm)
{
    auto numBits  = mfm.size();
    auto numBytes = mfm.bytes().size();

    assert(numBytes < sizeof(data.track[t]));

    // Copy track data
    memcpy(data.track[t], mfm.data(), numBytes);

    // Adapt the new length
    track[t] = MutableBitView(track[t].data(), numBits);
}

void
FloppyDisk::shiftTracks(isize offset)
{
    loginfo(DSK_DEBUG, "Shifting tracks by %ld bytes against each other\n", offset);

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
FloppyDisk::writeToFile(const fs::path& path) const
{
    auto ext = utl::uppercased(path.extension().string());

    if (ext == ".ADF")  writeToFile(path, ImageFormat::ADF);
    if (ext == ".EADF") writeToFile(path, ImageFormat::EADF);
    if (ext == ".IMG")  writeToFile(path, ImageFormat::IMG);
    if (ext == ".IMA")  writeToFile(path, ImageFormat::IMG);
    if (ext == ".ST")   writeToFile(path, ImageFormat::ST);

    throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
}

void
FloppyDisk::writeToFile(const fs::path& path, ImageFormat fmt) const
{
    switch (fmt) {

        case ImageFormat::ADF:  Codec::makeADF(*this)->writeToFile(path); break;
        case ImageFormat::EADF: Codec::makeEADF(*this)->writeToFile(path); break;
        case ImageFormat::IMG:  Codec::makeIMG(*this)->writeToFile(path); break;
        case ImageFormat::ST:   Codec::makeST(*this)->writeToFile(path); break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }
}

/*
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
*/

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
