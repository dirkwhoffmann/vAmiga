// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ADFFile.h"

namespace vamiga {

class ADZFile : public FloppyFile {

    ADFFile adf;

public:
    
    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    ADZFile(const fs::path &path) throws { init(path); }
    ADZFile(const u8 *buf, isize len) throws { init(buf, len); }
    ADZFile(const class ADFFile &adf) throws { init(adf); }
    
    void init(const class ADFFile &adf) throws;
    
    const char *objectName() const override { return "ADZ"; }

    
    //
    // Methods from AnyFile
    //
    
    FileType type() const override { return FileType::ADZ; }
    u64 fnv64() const override { return adf.fnv64(); }
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) const override { return isCompatible(buf, len); }
    void finalizeRead() throws override;

    
    //
    // Methods from DiskFile
    //

    isize numCyls() const override { return adf.numCyls(); }
    isize numHeads() const override { return adf.numHeads(); }
    isize numSectors() const override { return adf.numSectors(); }

    
    //
    // Methods from FloppyFile
    //
    
    FSFormat getDos() const override { return adf.getDos(); }
    void setDos(FSFormat dos) override { adf.setDos(dos); }
    Diameter getDiameter() const override { return adf.getDiameter(); }
    Density getDensity() const override { return adf.getDensity(); }
    BootBlockType bootBlockType() const override { return adf.bootBlockType(); }
    const char *bootBlockName() const override { return adf.bootBlockName(); }
    void readSector(u8 *target, isize s) const override { return adf.readSector(target, s); }
    void readSector(u8 *target, isize t, isize s) const override { return adf.readSector(target, t, s); }
    void encodeDisk(class FloppyDisk &disk) const throws override { return adf.encodeDisk(disk); }
};

}
