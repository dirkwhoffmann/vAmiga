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
#include "FSDevice.h"
#include "IO.h"

bool
EXEFile::isCompatible(const string &path)
{
    auto suffix = util::extractSuffix(path);
    return suffix == "exe" || suffix == "EXE";
}

bool
EXEFile::isCompatible(std::istream &stream)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
                                                                                            
    // Only accept the file if it fits onto a HD disk
    if (util::streamLength(stream) > 1710000) return false;

    return util::matchingStreamHeader(stream, signature, sizeof(signature));
}

isize
EXEFile::readFromStream(std::istream &stream)
{
    isize result = AmigaFile::readFromStream(stream);
    
    // Check if this file requires an HD disk
    bool hd = size > 853000;
        
    // Create a new file system
    FSDevice volume(INCH_35, hd ? DISK_HD : DISK_DD);
    volume.setName(FSName("Disk"));
    
    // Make the volume bootable
    volume.makeBootable(BB_AMIGADOS_13);
    
    // Add the executable
    FSBlock *file = volume.makeFile("file", data, size);
    if (!file) throw VAError(ERROR_FS_OUT_OF_SPACE);
    
    // Add a script directory
    volume.makeDir("s");
    volume.changeDir("s");
    
    // Add a startup sequence
    file = volume.makeFile("startup-sequence", "file");
    if (!file) throw VAError(ERROR_FS_OUT_OF_SPACE);

    // Finalize
    volume.updateChecksums();
    
    // Check for file system errors
    volume.changeDir("/");
    volume.info();
    volume.printDirectory(true);

    // Check the file system for consistency
    FSErrorReport report = volume.check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        if constexpr (FS_DEBUG) volume.dump();
    }
        
    // Convert the volume into an ADF
    adf = new ADFFile(volume);
        
    return result;
}