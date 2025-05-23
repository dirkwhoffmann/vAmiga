// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyFile.h"

namespace vamiga {

class STFile : public FloppyFile {

public:

    static constexpr isize STSIZE_35_DD = 737280;  // 720 KB Atari ST disk

    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);


    //
    // Initializing
    //

public:

    using AnyFile::init;

    STFile(const fs::path &path) throws { init(path); }
    // STFile(const fs::path &path, std::istream &stream) throws { init(path, stream); }
    STFile(const u8 *buf, isize len) throws { init(buf, len); }
    STFile(Diameter dia, Density den) throws { init(dia, den); }
    STFile(class FloppyDisk &disk) throws { init(disk); }

private:

    void init(Diameter dia, Density den) throws;
    void init(class FloppyDisk &disk) throws;


    //
    // Methods from CoreObject
    //

public:

    const char *objectName() const override { return "ST"; }


    //
    // Methods from AnyFile
    //

    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }
    FileType type() const override { return FileType::IMG; }


    //
    // Methods from DiskFile
    //

    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors() const override;


    //
    // Methods from FloppyFile
    //

    FSVolumeType getDos() const override { return FSVolumeType::NODOS; }
    void setDos(FSVolumeType dos) override { };
    Diameter getDiameter() const override { return Diameter::INCH_35; }
    Density getDensity() const override { return Density::DD; }
    void encodeDisk(class FloppyDisk &disk) const throws override;
    void decodeDisk(class FloppyDisk &disk) throws override;

private:

    void encodeTrack(class FloppyDisk &disk, Track t) const throws;
    void encodeSector(class FloppyDisk &disk, Track t, Sector s) const throws;

    void decodeTrack(class FloppyDisk &disk, Track t) throws;
    void decodeSector(u8 *dst, u8 *src);
};

}
