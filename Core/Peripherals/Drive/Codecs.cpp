// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Codecs.h"
#include "DiskEncoder.h"
#include "FileSystem.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "HardDrive.h"

namespace vamiga {

std::unique_ptr<ADFFile>
Codec::makeADF(const class FloppyDisk &disk)
{
    auto adf = make_unique<ADFFile>(disk.getDiameter(), disk.getDensity());

    assert(adf->numTracks() == 160);
    assert(adf->numSectors() == 11 || adf->numSectors() == 22);

    disk.decode(*adf);
    return adf;
}

std::unique_ptr<ADFFile>
Codec::makeADF(const class FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
    return makeADF(*drive.disk);
}

std::unique_ptr<ADZFile>
Codec::makeADZ(const FloppyDisk &disk)
{
    return make_unique<ADZFile>(*Codec::makeADF(disk));
}

std::unique_ptr<ADZFile>
Codec::makeADZ(const FloppyDrive &drive)
{
    return makeADZ(*drive.disk);
}

std::unique_ptr<EADFFile>
Codec::makeEADF(const FloppyDisk &disk)
{
    auto numTracks = disk.numTracks();

    auto length = 0;

    length += 12;               // File header
    length += 12 * numTracks;   // Track headers

    for (isize t = 0; t < numTracks; t++) {

        auto numBits = disk.track[t].size();
        assert(numBits % 8 == 0);
        length += numBits / 8;
    }

    auto eadf = make_unique<EADFFile>(length);
    decodeEADF(*eadf, disk);
    return eadf;
}

std::unique_ptr<EADFFile>
Codec::makeEADF(const FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
    return makeEADF(*drive.disk);
}

std::unique_ptr<IMGFile>
Codec::makeIMG(const FloppyDisk &disk)
{
    auto img = make_unique<IMGFile>(disk.getDiameter(), disk.getDensity());
    disk.decode(*img);
    return img;
}

std::unique_ptr<IMGFile>
Codec::makeIMG(const FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
    return makeIMG(*drive.disk);
}

std::unique_ptr<HDFFile>
Codec::makeHDF(const HardDrive &drive)
{
    auto hdf = std::make_unique<HDFFile>(drive.data.ptr, drive.data.size);

    // Overwrite the predicted geometry with the precise one
    hdf->geometry = drive.getGeometry();

    return hdf;
}

std::unique_ptr<HDZFile>
Codec::makeHDZ(const class HardDrive &hd)
{
    return make_unique<HDZFile>(*Codec::makeHDF(hd));
}


void
Codec::encodeADZ(const ADZFile &adz, FloppyDisk &disk)
{
    disk.encode(adz.adf);
}

void
Codec::encodeEADF(const EADFFile &eadf, FloppyDisk &disk)
{
    assert(!eadf.data.empty());

    isize tracks   = eadf.storedTracks();
    auto  diameter = eadf.getDiameter();
    auto  density  = eadf.getDensity();

    if constexpr (MFM_DEBUG) fprintf(stderr, "Encoding Amiga disk with %ld tracks\n", tracks);

    // Create an empty ADF
    auto adf = make_unique<ADFFile>(ADFFile::fileSize(diameter, density, tracks));

    // Wipe out all data
    disk.clearDisk(0);

    // Encode all standard tracks
    for (TrackNr t = 0; t < tracks; t++) encodeStandardTrack(eadf, *adf, t);

    // Convert the ADF to a disk
    disk.encodeDisk(*adf);

    // Encode all extended tracks
    for (TrackNr t = 0; t < tracks; t++) encodeExtendedTrack(eadf, disk, t);
}

std::unique_ptr<STFile>
Codec::makeST(const FloppyDisk &disk)
{
    auto st = make_unique<STFile>(disk.getDiameter(), disk.getDensity());
    disk.decode(*st);
    return st;
}

std::unique_ptr<STFile>
Codec::makeST(const FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
    return makeST(*drive.disk);
}

void
Codec::encodeStandardTrack(const EADFFile &eadf, ADFFile &adf, TrackNr t)
{
    if (eadf.typeOfTrack(t) == 0) {

        if constexpr (MFM_DEBUG) fprintf(stderr, "Encoding standard track %ld\n", t);

        auto numBits = eadf.usedBitsForTrack(t);
        assert(numBits % 8 == 0);

        auto ptr = adf.data.ptr + t * 11 * 512;
        std::memcpy(ptr, eadf.trackData(t), size_t(numBits / 8));
    }
}

void
Codec::encodeExtendedTrack(const EADFFile &eadf, FloppyDisk &disk, TrackNr t)
{
    if (eadf.typeOfTrack(t) == 1) {

        if constexpr (MFM_DEBUG) fprintf(stderr, "Encoding extended track %ld\n", t);

        auto numBits = eadf.usedBitsForTrack(t);
        assert(numBits % 8 == 0);

        std::memcpy(disk.data.track[t], eadf.trackData(t), size_t(numBits / 8));
        disk.track[t] = MutableBitView(disk.data.track[t], numBits);
    }
}

void
Codec::decodeEADF(EADFFile &eadf, const FloppyDisk &disk)
{
    assert(!eadf.data.empty());

    u8 *p = eadf.data.ptr;
    auto numTracks = disk.numTracks();

    // Magic bytes
    p[0] = 'U'; p[1] = 'A'; p[2] = 'E'; p[3] = '-';
    p[4] = '1'; p[5] = 'A'; p[6] = 'D'; p[7] = 'F';

    // Reserved
    assert(p[8] == 0);
    assert(p[9] == 0);

    // Number of tracks
    assert(p[10] == 0);
    p[11] = (u8)numTracks;

    p += 12;

    // Track headers
    for (TrackNr t = 0; t < numTracks; t++, p += 12) {

        assert(disk.track[t].size() % 8 == 0);
        auto bytes = disk.track[t].size() / 8;
        auto bits = 8 * bytes;

        // Reserved
        assert(p[0] == 0);
        assert(p[1] == 0);

        // Type
        assert(p[2] == 0);
        p[3] = 1;

        // Track space in bytes
        p[4] = BYTE3(bytes);
        p[5] = BYTE2(bytes);
        p[6] = BYTE1(bytes);
        p[7] = BYTE0(bytes);

        // Track length in bits
        p[8] = BYTE3(bits);
        p[9] = BYTE2(bits);
        p[10] = BYTE1(bits);
        p[11] = BYTE0(bits);
    }

    // Track headers
    for (TrackNr t = 0; t < numTracks; t++) {

        auto bytes = disk.track[t].size() / 8;

        for (isize i = 0; i < bytes; i++, p++) {
            *p = disk.data.track[t][i];
        }
    }

    if constexpr (MFM_DEBUG) fprintf(stderr, "Wrote %td bytes\n", p - eadf.data.ptr);
}

void
Codec::encodeIMG(const class IMGFile &source, FloppyDisk &disk)
{
    IMGFile img(source.data.ptr, source.data.size);
    disk.encode(img);
}

void
Codec::decodeIMG(class IMGFile &target, const FloppyDisk &disk)
{
    IMGFile img(target.data.ptr, target.data.size);
    disk.decode(img);
    target.data = img.data;
}

void
Codec::encodeST(const class STFile &source, FloppyDisk &disk)
{
    IMGFile img(source.data.ptr, source.data.size);
    disk.encode(img);
}

void
Codec::decodeST(class STFile &target, const FloppyDisk &disk)
{
    IMGFile img(target.data.ptr, target.data.size);
    disk.decode(img);
    target.data = img.data;
}

void
Codec::encodeDMS(const class DMSFile &source, FloppyDisk &disk)
{
    disk.encode(source.getADF());
}

void
Codec::encodeEXE(const class EXEFile &source, FloppyDisk &disk)
{
    disk.encode(source.getADF());
}

}
