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
#include "D64File.h"
#include "EADFFile.h"
#include "HDFFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "DMSFile.h"
#include "EXEFile.h"

namespace vamiga {

using retro::vault::image::ADFFile;
using retro::vault::image::D64File;
using retro::vault::image::EADFFile;
using retro::vault::image::HDFFile;
using retro::vault::image::DMSFile;
using retro::vault::image::EXEFile;

class Codec {

public:

    // Factory methods
    static std::unique_ptr<ADFFile> makeADF(const FloppyDisk &disk);
    static std::unique_ptr<ADFFile> makeADF(const FloppyDrive &drive);

    static std::unique_ptr<EADFFile> makeEADF(const FloppyDisk &disk);
    static std::unique_ptr<EADFFile> makeEADF(const FloppyDrive &drive);

    static std::unique_ptr<IMGFile> makeIMG(const FloppyDisk &disk);
    static std::unique_ptr<IMGFile> makeIMG(const FloppyDrive &drive);

    static std::unique_ptr<STFile> makeST(const FloppyDisk &disk);
    static std::unique_ptr<STFile> makeST(const FloppyDrive &drive);

    static std::unique_ptr<D64File> makeD64(const FloppyDisk &disk);
    static std::unique_ptr<D64File> makeD64(const FloppyDrive &drive);

    static std::unique_ptr<HDFFile> makeHDF(const HardDrive &hd);


    // Encoders and Decoders
    static void encodeEADF(const EADFFile &source, FloppyDisk &target);
    static void decodeEADF(EADFFile &target, const FloppyDisk &source);

    static void encodeIMG(const IMGFile &source, FloppyDisk &target);
    static void decodeIMG(IMGFile &target, const FloppyDisk &source);

    static void encodeST(const STFile &source, FloppyDisk &target);
    static void decodeST(STFile &target, const FloppyDisk &source);

    static void encodeDMS(const DMSFile &source, FloppyDisk &target);

    static void encodeEXE(const EXEFile &source, FloppyDisk &target);

private:

    /*
    static void encodeStandardTrack(const EADFFile &eadf, ADFFile &adf, TrackNr t);
    static void encodeExtendedTrack(const EADFFile &eadf, FloppyDisk &disk, TrackNr t);
    */
};

}

