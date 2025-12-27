// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EADFEncoder.h"
#include "EADFFactory.h"
#include "ADFFactory.h"

namespace vamiga {

void
EADFEncoder::encode(const EADFFile &eadf, FloppyDisk &disk)
{
    assert(!eadf.data.empty());

    isize tracks   = eadf.storedTracks();
    auto  diameter = eadf.getDiameter();
    auto  density  = eadf.getDensity();

    if (MFM_DEBUG) fprintf(stderr, "Encoding Amiga disk with %ld tracks\n", tracks);

    // Create an empty ADF
    auto adf = ADFFactory::make(ADFFile::fileSize(diameter, density, tracks));

    // Wipe out all data
    disk.clearDisk(0);

    // Encode all standard tracks
    for (Track t = 0; t < tracks; t++) encodeStandardTrack(eadf, *adf, t);

    // Convert the ADF to a disk
    disk.encodeDisk(*adf);

    // Encode all extended tracks
    for (Track t = 0; t < tracks; t++) encodeExtendedTrack(eadf, disk, t);
}

void
EADFEncoder::encodeStandardTrack(const EADFFile &eadf, ADFFile &adf, Track t)
{
    if (eadf.typeOfTrack(t) == 0) {

        if (MFM_DEBUG) fprintf(stderr, "Encoding standard track %ld\n", t);

        auto numBits = eadf.usedBitsForTrack(t);
        assert(numBits % 8 == 0);

        auto ptr = adf.data.ptr + t * 11 * 512;
        std::memcpy(ptr, eadf.trackData(t), size_t(numBits / 8));
    }
}

void
EADFEncoder::encodeExtendedTrack(const EADFFile &eadf, FloppyDisk &disk, Track t)
{
    if (eadf.typeOfTrack(t) == 1) {

        if (MFM_DEBUG) fprintf(stderr, "Encoding extended track %ld\n", t);

        auto numBits = eadf.usedBitsForTrack(t);
        assert(numBits % 8 == 0);

        std::memcpy(disk.data.track[t], eadf.trackData(t), size_t(numBits / 8));
        disk.length.track[t] = i32(numBits / 8);
    }
}

void
EADFEncoder::decode(EADFFile &eadf, const FloppyDisk &disk)
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
    for (Track t = 0; t < numTracks; t++, p += 12) {

        auto bytes = disk.length.track[t];
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
    for (Track t = 0; t < numTracks; t++) {

        auto bytes = disk.length.track[t];

        for (isize i = 0; i < bytes; i++, p++) {
            *p = disk.data.track[t][i];
        }
    }

    if (MFM_DEBUG) fprintf(stderr, "Wrote %td bytes\n", p - eadf.data.ptr);
}

}
