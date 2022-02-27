// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EXEFile.h"
#include "AmigaFile.h"
#include "MutableFileSystem.h"
#include "IOUtils.h"

bool
EXEFile::isCompatible(const string &path)
{
    auto suffix = util::uppercased(util::extractSuffix(path));
    return suffix == "EXE";
}

bool
EXEFile::isCompatible(std::istream &stream)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
                                                                                            
    // Only accept the file if it fits onto a HD disk
    if (util::streamLength(stream) > 1710000) return false;

    return util::matchingStreamHeader(stream, signature, sizeof(signature));
}

void
EXEFile::finalizeRead()
{
    // Check if this file requires an HD disk
    bool hd = size > 853000;
        
    // Create a new file system
    MutableFileSystem volume(INCH_35, hd ? DENSITY_HD : DENSITY_DD, FS_OFS);
    volume.setName(FSName("Disk"));
    
    // Make the volume bootable
    volume.makeBootable(BB_AMIGADOS_13);
    
    // Add the executable
    FSBlock *file = volume.createFile("file", data, size);
    if (!file) throw VAError(ERROR_FS_OUT_OF_SPACE);
    
    // Add a script directory
    volume.createDir("s");
    volume.changeDir("s");
    
    // Add a startup sequence
    file = volume.createFile("startup-sequence", "file");
    if (!file) throw VAError(ERROR_FS_OUT_OF_SPACE);

    // Finalize
    volume.updateChecksums();
    
    // Move to the to root directory
    volume.changeDir("/");

    // Print some debug information about the volume
    if constexpr (FS_DEBUG) {
        volume.dump(dump::Summary);
        volume.printDirectory(true);
    }
    
    // Check file system integrity
    FSErrorReport report = volume.check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        if constexpr (FS_DEBUG) volume.dump();
    }
        
    // Convert the volume into an ADF
    adf = new ADFFile(volume);
}
