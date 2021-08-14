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
#include "FSDevice.h"

bool
Folder::isFolder(const string &path)
{
    DIR *dir;
    
    // We accept all directories
    if ((dir = opendir(path.c_str())) == nullptr) return false;
    
    closedir(dir);
    return true;
}

Folder *
Folder::make(const string &path)
{
    return new Folder(path);
}

void
Folder::init(const string &path)
{    
    if (FS_DEBUG) msg("make(%s)\n", path.c_str());
              
    // Only proceed if the provided filename points to a directory
    if (!isFolder(path.c_str())) throw VAError(ERROR_FILE_TYPE_MISMATCH);
    
    // Create a file system and import the directory
    FSDevice *volume = FSDevice::make(FS_OFS, path.c_str());
    if (!volume) {
        warn("Contents of %s does not fit on a disk\n", path.c_str());
        throw VAError(ERROR_UNKNOWN);
    }
    
    // Check the file system for errors
    volume->info();
    volume->printDirectory(true);

    // Check the file system for consistency
    FSErrorReport report = volume->check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
    }
    // volume->dump();
    
    // Convert the file system into an ADF
    adf = new ADFFile(*volume);
    delete volume;
}
