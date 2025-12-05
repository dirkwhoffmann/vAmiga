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

class FileSystem;

class ADFFile : public FloppyFile {

public:

    static constexpr isize ADFSIZE_35_DD    = 901120;   //  880 KB
    static constexpr isize ADFSIZE_35_DD_81 = 912384;   //  891 KB (+ 1 cyl)
    static constexpr isize ADFSIZE_35_DD_82 = 923648;   //  902 KB (+ 2 cyls)
    static constexpr isize ADFSIZE_35_DD_83 = 934912;   //  913 KB (+ 3 cyls)
    static constexpr isize ADFSIZE_35_DD_84 = 946176;   //  924 KB (+ 4 cyls)
    static constexpr isize ADFSIZE_35_HD    = 1802240;  // 1760 KB
    
    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
        
    // Returns the size of an ADF file of a given disk type in bytes
    static isize fileSize(Diameter diameter, Density density);

    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    ADFFile() { }
    ADFFile(const fs::path &path) { init(path); }
    ADFFile(isize len) { init(len); }
    ADFFile(const u8 *buf, isize len) { init(buf, len); }

    
    //
    // Methods from CoreObject
    //

public:
    
    // const char *objectName() const override { return "ADF"; }

    
    //
    // Methods from AnyFile
    //
    
public:
    
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) const override { return isCompatible(buf, len); }
    FileType type() const override { return FileType::ADF; }
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
    
    FSFormat getDos() const override;
    void setDos(FSFormat dos) override;
    Diameter getDiameter() const override;
    Density getDensity() const override;
    BootBlockType bootBlockType() const override;
    const char *bootBlockName() const override;
    void killVirus() override;

    
    //
    // Querying disk properties
    //
    
public:
    
    // Returns a file system descriptor for this volume
    struct FSDescriptor getFileSystemDescriptor() const;

    
    //
    // Formatting
    //

public:
    
    void formatDisk(FSFormat fs, BootBlockId id, string name);

    
    //
    // Debugging
    //

public:
    
    void dumpSector(Sector s) const;
};

}
