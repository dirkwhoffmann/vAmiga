// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskFile.h"
#include "FSDevice.h"

class ADFFile : public DiskFile {

public:

    static constexpr isize ADFSIZE_35_DD    = 901120;   //  880 KB
    static constexpr isize ADFSIZE_35_DD_81 = 912384;   //  891 KB (+ 1 cyl)
    static constexpr isize ADFSIZE_35_DD_82 = 923648;   //  902 KB (+ 2 cyls)
    static constexpr isize ADFSIZE_35_DD_83 = 934912;   //  913 KB (+ 3 cyls)
    static constexpr isize ADFSIZE_35_DD_84 = 946176;   //  924 KB (+ 4 cyls)
    static constexpr isize ADFSIZE_35_HD    = 1802240;  // 1760 KB
    
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);
    
private:
    
    // Returns the size of an ADF file of a given disk type in bytes
    static isize fileSize(DiskDiameter diameter, DiskDensity density) throws;

    
    //
    // Initializing
    //
    
public:
    
    ADFFile() { };
    ADFFile(const string &path) throws { AmigaFile::init(path); }
    ADFFile(const string &path, std::istream &stream) throws { AmigaFile::init(path, stream); }
    ADFFile(const u8 *buf, isize len) throws { AmigaFile::init(buf, len); }
    ADFFile(FILE *file) throws { AmigaFile::init(file); }
    ADFFile(DiskDiameter dia, DiskDensity den) throws { init(dia, den); }
    ADFFile(class Disk &disk) throws { init(disk); }
    ADFFile(class Drive &drive) throws { init(drive); }
    ADFFile(FSDevice &volume) throws { init(volume); }
    
    void init(DiskDiameter dia, DiskDensity den) throws;
    void init(Disk &disk) throws;
    void init(Drive &drive) throws;
    void init(FSDevice &volume) throws;

    
    //
    // Methods from AmigaObject
    //

public:
    
    const char *getDescription() const override { return "ADF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }
    FileType type() const override { return FILETYPE_ADF; }
    
    
    //
    // Methods from DiskFile
    //
    
public:
    
    FSVolumeType getDos() const override; 
    void setDos(FSVolumeType dos) override;
    DiskDiameter getDiskDiameter() const override;
    DiskDensity getDiskDensity() const override;
    isize numSides() const override;
    isize numCyls() const override;
    isize numSectors() const override;
    BootBlockType bootBlockType() const override;
    const char *bootBlockName() const override;
    
    void killVirus() override;

    void encodeDisk(class Disk &disk) const throws override;
    void decodeDisk(class Disk &disk) throws override;

private:
    
    void encodeTrack(class Disk &disk, Track t) const throws;
    void encodeSector(class Disk &disk, Track t, Sector s) const throws;

    void decodeTrack(class Disk &disk, Track t) throws;
    void decodeSector(u8 *dst, u8 *src) throws;

    
    //
    // Querying disk properties
    //
    
public:

    // Returns the layout of this disk in form of a device descriptor
    struct FSDeviceDescriptor layout();
    
    
    //
    // Formatting
    //
 
public:
    
    void formatDisk(FSVolumeType fs, BootBlockId id) throws;

    
    //
    // Debugging
    //
 
public:
    
    void dumpSector(Sector s) const;
};
