// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Folder.h"
#include "MutableFileSystem.h"
#include "IOUtils.h"

namespace vamiga {

bool
Folder::isCompatible(const string &path)
{
    return util::isDirectory(path);
}

void
Folder::init(const string &path)
{
    debug(FS_DEBUG, "make(%s)\n", path.c_str());

    // Only proceed if the provided filename points to a directory
    if (!isCompatiblePath(path)) throw VAError(ERROR_FILE_TYPE_MISMATCH);

    // Create a file system and import the directory
    MutableFileSystem volume(FS_OFS, path.c_str());
    
    // Make the volume bootable
    volume.makeBootable(BB_AMIGADOS_13);
    
    // Check the file system for errors
    volume.dump(Category::State);
    volume.printDirectory(true);

    // Check the file system for consistency
    FSErrorReport report = volume.check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        if constexpr (FS_DEBUG) volume.dump(Category::Blocks);
    }

    // Convert the file system into an ADF
    adf = new ADFFile(volume);
}

}
