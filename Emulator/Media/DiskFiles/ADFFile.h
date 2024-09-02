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

class MutableFileSystem;

class ADFFile : public FloppyFile {

public:

    static constexpr isize ADFSIZE_35_DD    = 901120;   //  880 KB
    static constexpr isize ADFSIZE_35_DD_81 = 912384;   //  891 KB (+ 1 cyl)
    static constexpr isize ADFSIZE_35_DD_82 = 923648;   //  902 KB (+ 2 cyls)
    static constexpr isize ADFSIZE_35_DD_83 = 934912;   //  913 KB (+ 3 cyls)
    static constexpr isize ADFSIZE_35_DD_84 = 946176;   //  924 KB (+ 4 cyls)
    static constexpr isize ADFSIZE_35_HD    = 1802240;  // 1760 KB
    
    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
private:
    
    // Returns the size of an ADF file of a given disk type in bytes
    static isize fileSize(Diameter diameter, Density density) throws;

    
    //
    // Initializing
    //
    
public:
    
    using AmigaFile::init;
    
    ADFFile() { }
    ADFFile(const std::filesystem::path &path) throws { init(path); }
    // ADFFile(const std::filesystem::path &path, std::istream &stream) throws { init(path, stream); }
    ADFFile(const u8 *buf, isize len) throws { init(buf, len); }
    // ADFFile(FILE *file) throws { init(file); }
    ADFFile(Diameter dia, Density den) throws { init(dia, den); }
    ADFFile(const FloppyDiskDescriptor &descr) throws { init(descr); }
    ADFFile(class FloppyDisk &disk) throws { init(disk); }
    ADFFile(class FloppyDrive &drive) throws { init(drive); }
    ADFFile(MutableFileSystem &volume) throws { init(volume); }
    
    void init(Diameter dia, Density den) throws;
    void init(const FloppyDiskDescriptor &descr) throws;
    void init(FloppyDisk &disk) throws;
    void init(FloppyDrive &drive) throws;
    void init(MutableFileSystem &volume) throws;

    
    //
    // Methods from CoreObject
    //

public:
    
    const char *objectName() const override { return "ADF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }
    FileType type() const override { return FILETYPE_ADF; }
    void finalizeRead() override;
    
    //
    // Methods from DiskFile
    //

    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors() const override;

    
    //
    // Methods from FloppyFile
    //
    
public:
    
    FSVolumeType getDos() const override;
    void setDos(FSVolumeType dos) override;
    Diameter getDiameter() const override;
    Density getDensity() const override;
    BootBlockType bootBlockType() const override;
    const char *bootBlockName() const override;
    void killVirus() override;

    void encodeDisk(class FloppyDisk &disk) const throws override;
    void decodeDisk(class FloppyDisk &disk) throws override;

private:
    
    void encodeTrack(class FloppyDisk &disk, Track t) const throws;
    void encodeSector(class FloppyDisk &disk, Track t, Sector s) const throws;

    void decodeTrack(class FloppyDisk &disk, Track t) throws;
    void decodeSector(u8 *dst, u8 *src) throws;

    
    //
    // Querying disk properties
    //
    
public:
    
    // Returns a file system descriptor for this volume
    struct FileSystemDescriptor getFileSystemDescriptor() const;

    
    //
    // Formatting
    //

public:
    
    void formatDisk(FSVolumeType fs, BootBlockId id, string name) throws;

    
    //
    // Debugging
    //

public:
    
    void dumpSector(Sector s) const;
};

}
