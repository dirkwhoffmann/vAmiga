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

    static void encodeTrack(MutableByteView track, Track t, ByteView src);
    static void encodeSector(MutableByteView track, isize offset, Track t, Sector s, ByteView src);

    static void decodeTrack(ByteView track, Track t, MutableByteView dst);
    static void decodeSector(ByteView track, isize offset, Track t, Sector s, MutableByteView dst);


    //
    // ADF
    //

public:

    static void encode(const class ADFFile &source, FloppyDisk &target);
    static void decode(class ADFFile &target, const FloppyDisk &source);

private:

    // [[deprecated]] static void encodeTrack(const ADFFile &adf, FloppyDisk &disk, Track t);
    // [[deprecated]] static void encodeSector(const ADFFile &adf, FloppyDisk &disk, Track t, Sector s);

    static void decodeTrack(ADFFile &adf, const class FloppyDisk &disk, Track t);
    static void decodeSector(ADFFile &adf, u8 *dst, const u8 *src);


    //
    // IMG
    //

public:

    static void encode(const class IMGFile &source, FloppyDisk &target);
    static void decode(class IMGFile &target, const FloppyDisk &source);

private:

    static void encodeTrack(const IMGFile &img, FloppyDisk &disk, Track t);
    static void encodeSector(const IMGFile &img, FloppyDisk &disk, Track t, Sector s);

    static void decodeTrack(IMGFile &img, const class FloppyDisk &disk, Track t);
    static void decodeSector(IMGFile &img, u8 *dst, const u8 *src);


    //
    // ST
    //

public:

    static void encode(const class STFile &source, FloppyDisk &target);
    static void decode(class STFile &target, const FloppyDisk &source);

private:

    static void encodeTrack(const STFile &img, FloppyDisk &disk, Track t);
    static void encodeSector(const STFile &img, FloppyDisk &disk, Track t, Sector s);

    static void decodeTrack(STFile &img, const class FloppyDisk &disk, Track t);
    static void decodeSector(STFile &img, u8 *dst, const u8 *src);


};

}
