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

class DMSFile : public FloppyFile {

    ADFFile adf;

public:
    
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);
    
    
    //
    // Initializing
    //
    
public:
    
    using AmigaFile::init;
    
    DMSFile(const string &path) throws { init(path); }
    DMSFile(const string &path, std::istream &stream) throws { init(path, stream); }
    DMSFile(const u8 *buf, isize len) throws { init(buf, len); }
    
    const char *getDescription() const override { return "DMS"; }
        
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_DMS; }
    u64 fnv() const override { return adf.fnv(); }
    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }
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
    
    FSVolumeType getDos() const override { return adf.getDos(); }
    void setDos(FSVolumeType dos) override { adf.setDos(dos); }
    Diameter getDiameter() const override { return adf.getDiameter(); }
    Density getDensity() const override { return adf.getDensity(); }
    BootBlockType bootBlockType() const override { return adf.bootBlockType(); }
    const char *bootBlockName() const override { return adf.bootBlockName(); }
    void readSector(u8 *target, isize s) const override { return adf.readSector(target, s); }
    void readSector(u8 *target, isize t, isize s) const override { return adf.readSector(target, t, s); }
    void encodeDisk(class FloppyDisk &disk) const throws override { return adf.encodeDisk(disk); }
};
