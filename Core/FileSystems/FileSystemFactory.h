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

namespace vamiga {

class FileSystemFactory {
    
public:
    
    static FileSystem fromMediaFile(const MediaFile &file, isize part = 0);
    static FileSystem fromADF(const ADFFile &adf);
    static FileSystem fromHDF(const HDFFile &hdf, isize part = 0);
    static FileSystem fromFloppyDrive(const FloppyDrive &df);
    static FileSystem fromHardDrive(const HardDrive &hd, isize part = 0);
    static FileSystem createEmpty(isize capacity, isize blockSize = 512);
    static FileSystem createFromDescriptor(const FSDescriptor &, const fs::path &path = {});
    static FileSystem createLowLevel(Diameter dia, Density den, FSFormat dos, const fs::path &path = {});
};

}
