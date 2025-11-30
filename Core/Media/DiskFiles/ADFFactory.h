// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ADFFile.h"
#include "FileSystem.h"

namespace vamiga {

class ADFFactory {

public:

    static std::unique_ptr<ADFFile> make(const fs::path &path);
    static std::unique_ptr<ADFFile> make(const u8 *buf, isize len);
    static std::unique_ptr<ADFFile> make(Diameter dia, Density den);
    static std::unique_ptr<ADFFile> make(const FloppyDiskDescriptor &descr);
    static std::unique_ptr<ADFFile> make(const class FloppyDisk &disk);
    static std::unique_ptr<ADFFile> make(const class FloppyDrive &drive);
    static std::unique_ptr<ADFFile> make(const FileSystem &volume);
};

}
