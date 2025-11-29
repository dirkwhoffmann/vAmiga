// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystem.h"
#include "ADFFile.h"
#include "HDFFile.h"

namespace vamiga {

class FileSystemFactory {
    
public:

    // Create a new filesystem (returns by value)
    static FileSystem fromADF(const ADFFile &adf);
    static FileSystem fromHDF(const HDFFile &hdf, isize part = 0);
    static FileSystem fromMediaFile(const MediaFile &file, isize part = 0);
    static FileSystem fromFloppyDrive(const FloppyDrive &df);
    static FileSystem fromHardDrive(const HardDrive &hd, isize part = 0);
    static FileSystem createEmpty(isize capacity, isize blockSize = 512);
    static FileSystem createFromDescriptor(const FSDescriptor &, const fs::path &path = {});
    static FileSystem createLowLevel(Diameter dia, Density den, FSFormat dos, const fs::path &path = {});

    // Initialize an existing filesystem
    static void initFromADF(FileSystem &fs, const ADFFile &adf);
    static void initFromHDF(FileSystem &fs, const HDFFile &hdf, isize part = 0);
    static void initFromMedia(FileSystem &fs, const MediaFile &file, isize part = 0);
    static void initFromFloppy(FileSystem &fs, const FloppyDrive &df);
    static void initFromHardDrive(FileSystem &fs, const HardDrive &hd, isize part = 0);
    static void initCreateEmpty(FileSystem &fs, isize capacity, isize blockSize = 512);
    static void initFromDescriptor(FileSystem &fs, const FSDescriptor &, const fs::path &path = {});
    static void initLowLevel(FileSystem &fs, Diameter dia, Density den, FSFormat dos, const fs::path &path = {});
};

}
