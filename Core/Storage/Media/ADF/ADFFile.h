// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDiskImage.h"
#include "DeviceDescriptors.h"

namespace vamiga {

class FileSystem;

class ADFFile : public FloppyDiskImage {

public:

    static constexpr isize ADFSIZE_35_DD    = 901120;   //  880 KB
    static constexpr isize ADFSIZE_35_DD_81 = 912384;   //  891 KB (+ 1 cyl)
    static constexpr isize ADFSIZE_35_DD_82 = 923648;   //  902 KB (+ 2 cyls)
    static constexpr isize ADFSIZE_35_DD_83 = 934912;   //  913 KB (+ 3 cyls)
    static constexpr isize ADFSIZE_35_DD_84 = 946176;   //  924 KB (+ 4 cyls)
    static constexpr isize ADFSIZE_35_HD    = 1802240;  // 1760 KB
    
    static bool isCompatible(const fs::path &path);

    // Returns the size of an ADF file of a given disk type in bytes
    static isize fileSize(Diameter diameter, Density density);
    static isize fileSize(Diameter diameter, Density density, isize tracks);

    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    ADFFile() { }
    ADFFile(const fs::path &path) { init(path); }
    ADFFile(isize len) { init(len); }
    ADFFile(const u8 *buf, isize len) { init(buf, len); }
    ADFFile(Diameter dia, Density den) { init(dia, den); }
    ADFFile(const GeometryDescriptor &descr) { init(descr); }
    ADFFile(const class FileSystem &volume) { init(volume); }

    void init(Diameter dia, Density den);
    void init(const GeometryDescriptor &descr);
    void init(const class FileSystem &volume);


    //
    // Methods from AnyFile
    //
    
public:
    
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    void didLoad() override;


    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return 512; }


    //
    // Methods from DiskImage
    //

public:
    
    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors(isize) const override { return numSectors(); }
    isize numSectors() const;


    //
    // Methods from FloppyDiskImage
    //
    
public:
    
    FSFormat getDos() const override;
    Diameter getDiameter() const override;
    Density getDensity() const override;

    
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
};

}
