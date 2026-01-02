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
#include "FloppyDrive.h"
#include "ADFFile.h"
#include "ADZFile.h"
#include "EADFFile.h"
#include "HDFFile.h"
#include "HDZFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "DMSFile.h"
#include "EXEFile.h"

using retro::image::HDFFile;
using retro::image::HDZFile;

namespace vamiga {

class Codec {

public:

    // Factory methods
    static std::unique_ptr<ADFFile> makeADF(const FloppyDisk &disk);
    static std::unique_ptr<ADFFile> makeADF(const FloppyDrive &drive);

    static std::unique_ptr<ADZFile> makeADZ(const FloppyDisk &disk);
    static std::unique_ptr<ADZFile> makeADZ(const FloppyDrive &drive);

    static std::unique_ptr<EADFFile> makeEADF(const FloppyDisk &disk);
    static std::unique_ptr<EADFFile> makeEADF(const FloppyDrive &drive);

    static std::unique_ptr<IMGFile> makeIMG(const FloppyDisk &disk);
    static std::unique_ptr<IMGFile> makeIMG(const FloppyDrive &drive);

    static std::unique_ptr<STFile> makeST(const FloppyDisk &disk);
    static std::unique_ptr<STFile> makeST(const FloppyDrive &drive);

    static std::unique_ptr<HDFFile> makeHDF(const HardDrive &hd);

    static std::unique_ptr<HDZFile> makeHDZ(const HardDrive &hd);


    // Encoders and Decoders
    static void encodeADZ(const ADZFile &source, FloppyDisk &target);

    static void encodeEADF(const EADFFile &source, FloppyDisk &target);
    static void decodeEADF(EADFFile &target, const FloppyDisk &source);

    static void encodeIMG(const IMGFile &source, FloppyDisk &target);
    static void decodeIMG(IMGFile &target, const FloppyDisk &source);

    static void encodeST(const STFile &source, FloppyDisk &target);
    static void decodeST(STFile &target, const FloppyDisk &source);

    static void encodeDMS(const DMSFile &source, FloppyDisk &target);

    static void encodeEXE(const EXEFile &source, FloppyDisk &target);

private:

    static void encodeStandardTrack(const EADFFile &eadf, ADFFile &adf, TrackNr t);
    static void encodeExtendedTrack(const EADFFile &eadf, FloppyDisk &disk, TrackNr t);

};

}

