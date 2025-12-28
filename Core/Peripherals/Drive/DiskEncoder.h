// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDisk.h"

namespace vamiga {

class DiskEncoder {

    //
    // MFM
    //

public:

    static void encodeMFM(u8 *dst, const u8 *src, isize count);
    static void decodeMFM(u8 *dst, const u8 *src, isize count);

    static void encodeOddEven(u8 *dst, const u8 *src, isize count);
    static void decodeOddEven(u8 *dst, const u8 *src, isize count);

    static void addClockBits(u8 *dst, isize count);
    static u8 addClockBits(u8 value, u8 previous);


    //
    // Amiga tracks
    //

public:

    static void encodeAmigaTrack(MutableByteView track, TrackNr t, ByteView src);
    static void encodeAmigaSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView src);

    static void decodeAmigaTrack(ByteView track, TrackNr t, MutableByteView dst);
    static void decodeAmigaSector(ByteView track, isize offset, TrackNr t, SectorNr s, MutableByteView dst);


    //
    // ADF
    //

public:

    static void encode(const class ADFFile &source, FloppyDisk &target);
    static void decode(class ADFFile &target, const FloppyDisk &source);


    //
    // IMG
    //

public:

    static void encode(const class IMGFile &source, FloppyDisk &target);
    static void decode(class IMGFile &target, const FloppyDisk &source);

private:

    static void encodeTrack(const IMGFile &img, FloppyDisk &disk, TrackNr t);
    static void encodeSector(const IMGFile &img, FloppyDisk &disk, TrackNr t, SectorNr s);

    static void decodeTrack(IMGFile &img, const class FloppyDisk &disk, TrackNr t);
    static void decodeSector(IMGFile &img, u8 *dst, const u8 *src);


    //
    // ST
    //

public:

    static void encode(const class STFile &source, FloppyDisk &target);
    static void decode(class STFile &target, const FloppyDisk &source);

private:

    static void encodeTrack(const STFile &img, FloppyDisk &disk, TrackNr t);
    static void encodeSector(const STFile &img, FloppyDisk &disk, TrackNr t, SectorNr s);

    static void decodeTrack(STFile &img, const class FloppyDisk &disk, TrackNr t);
    static void decodeSector(STFile &img, u8 *dst, const u8 *src);


};

}
