// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskEncoder.h"
#include "AmigaEncoder.h"
#include "IBMEncoder.h"
#include "FloppyDisk.h"
#include "ADFFactory.h"
#include "IMGFactory.h"
#include "STFactory.h"
#include "CoreError.h"
#include "DeviceError.h"
#include "MFM.h"

namespace vamiga {

//
// ADF
//

void
DiskEncoder::encode(const ADFFile &adf, FloppyDisk &disk)
{
    isize tracks = adf.numTracks();
    if (ADF_DEBUG) fprintf(stderr, "Encoding Amiga disk with %ld tracks\n", tracks);

    if (disk.getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        AmigaEncoder::encodeTrack(disk.byteView(t), t, adf.byteView(t));

    // In debug mode, also run the decoder
    if (ADF_DEBUG) {

        string tmp = "/tmp/debug.adf";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        ADFFactory::make(disk)->writeToFile(tmp);
    }
}

void DiskEncoder::decode(ADFFile &adf, const class FloppyDisk &disk)
{
    auto tracks = adf.numTracks();
    if (ADF_DEBUG) fprintf(stderr, "Decoding Amiga disk with %ld tracks\n", tracks);

    if (disk.getDiameter() != adf.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != adf.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        AmigaEncoder::decodeTrack(disk.byteView(t), t, adf.byteView(t));
}

//
// IMG
//

void
DiskEncoder::encode(const class IMGFile &img, FloppyDisk &disk)
{
    isize tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS disk with %ld tracks\n", tracks);

    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        IBMEncoder::encodeTrack(disk.byteView(t), t, img.byteView(t));

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        string tmp = "/tmp/debug.img";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        IMGFactory::make(disk)->writeToFile(tmp);
    }
}

void
DiskEncoder::decode(class IMGFile &img, const FloppyDisk &disk)
{
    auto tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Decoding DOS disk (%ld tracks)\n", tracks);

    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        IBMEncoder::decodeTrack(disk.byteView(t), t, img.byteView(t));
}


//
// ST
//

void
DiskEncoder::encode(const class STFile &img, FloppyDisk &disk)
{
    isize tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Encoding DOS disk with %ld tracks\n", tracks);

    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        IBMEncoder::encodeTrack(disk.byteView(t), t, img.byteView(t));

    // In debug mode, also run the decoder
    if (IMG_DEBUG) {

        string tmp = "/tmp/debug.img";
        fprintf(stderr, "Saving image to %s for debugging\n", tmp.c_str());
        IMGFactory::make(disk)->writeToFile(tmp);
    }

    disk.byteView(0,0).hexDump();
}

void
DiskEncoder::decode(class STFile &img, const FloppyDisk &disk)
{
    auto tracks = img.numTracks();
    if (IMG_DEBUG) fprintf(stderr, "Decoding Atari ST disk (%ld tracks)\n", tracks);

    if (disk.getDiameter() != img.getDiameter()) {
        throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != img.getDensity()) {
        throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }

    // Decode all tracks
    for (TrackNr t = 0; t < tracks; ++t)
        IBMEncoder::decodeTrack(disk.byteView(t), t, img.byteView(t));
}

}
