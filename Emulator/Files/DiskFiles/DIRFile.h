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

class DIRFile : public DiskFile {
    
public:
    
    ADFFile *adf = nullptr;
    
    
    //
    // Class methods
    //
        
    // Returns true iff if the provided path points to a suitable directory
    static bool isDIRFile(const char *path);
    
    
    //
    // Initializing
    //
    
    DIRFile();
    
    const char *getDescription() const override { return "DIR"; }
        
    
    //
    // Methods from AmigaFile
    //
    
    FileType fileType() const override { return FILETYPE_DIR; }
    u64 fnv() const override { return adf->fnv(); }
    bool matchingBuffer(const u8 *buffer, size_t length) override;
    bool matchingFile(const char *path) override { return isDIRFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length, FileError *error = nullptr) override;
    bool readFromFile(const char *filename, FileError *error = nullptr) override;
    
    
    //
    // Methods from DiskFile
    //
    
public:
    
    FSVolumeType getDos() const override { return adf->getDos(); }
    void setDos(FSVolumeType dos) override { adf->setDos(dos); }
    DiskDiameter getDiskDiameter() const override { return adf->getDiskDiameter(); }
    DiskDensity getDiskDensity() const override { return adf->getDiskDensity(); }
    long numSides() const override { return adf->numSides(); }
    long numCyls() const override { return adf->numCyls(); }
    long numSectors() const override { return adf->numSectors(); }
    BootBlockType bootBlockType() const override { return adf->bootBlockType(); }
    const char *bootBlockName() const override { return adf->bootBlockName(); }
    void killVirus() override { adf->killVirus(); }
    void readSector(u8 *target, long s) const override { return adf->readSector(target, s); }
    void readSector(u8 *target, long t, long s) const override { return adf->readSector(target, t, s); }
    bool encodeDisk(class Disk *disk) override { return adf->encodeDisk(disk); }
};
