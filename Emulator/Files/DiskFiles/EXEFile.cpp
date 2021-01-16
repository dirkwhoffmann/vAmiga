// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "EXEFile.h"
#include "FSDevice.h"

EXEFile::EXEFile()
{
}

bool
EXEFile::isEXEBuffer(const u8 *buffer, size_t length)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
                                                                                            
    assert(buffer != nullptr);
    
    // Only accept the file if it fits onto a HD disk
    if (length > 1710000) return false;

    return matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
EXEFile::isEXEFile(const char *path)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
    
    assert(path != nullptr);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
EXEFile::readFromBuffer(const u8 *buffer, size_t length, FileError *error)
{
    bool success = false;
    
    if (!isEXEBuffer(buffer, length)) {
        if (error) *error = ERR_INVALID_TYPE;
        return false;
    }

    if (!AmigaFile::readFromBuffer(buffer, length, error)) {
        return false;
    }
    
    // Check if this file requires an HD disk
    bool hd = length > 853000;
        
    // Create a new file system
    // FSDevice *volume = FSDevice::make(FS_OFS, 80, 2, hd ? 22 : 11);
    FSDevice *volume = FSDevice::makeWithFormat(INCH_35, hd ? DISK_HD : DISK_DD);
    volume->setName(FSName("Disk"));
    
    // Make the volume bootable
    volume->makeBootable(0);
    
    // Add the executable
    FSBlock *file = volume->makeFile("file", buffer, length);
    success = file != nullptr;
    
    // Add a script directory
    volume->makeDir("s");
    volume->changeDir("s");
    
    // Add a startup sequence
    file = volume->makeFile("startup-sequence", "file");
    success &= file != nullptr;
    
    // Finalize
    volume->updateChecksums();
    
    // Check for file system errors
    volume->changeDir("/");
    volume->info();
    volume->printDirectory(true);

    // Check the file system for consistency
    FSErrorReport report = volume->check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        volume->dump();
    }
    
    // Convert the volume into an ADF
    if (success) {
        ErrorCode fsError;
        assert(adf == nullptr);
        adf = ADFFile::makeWithVolume(*volume, &fsError);
        if (fsError != ERROR_OK) {
            warn("readFromBuffer: Cannot export volume (%s)\n",
                 ErrorCodeEnum::key(fsError));
        }
    }
    
    // REMOVE ASAP
    const char *path = "/tmp/test";
    msg("Doing a test export to %s\n", path);
    
    volume->exportDirectory(path);
    
    if (error) *error = adf ? ERR_FILE_OK : ERR_UNKNOWN;
    return adf != nullptr;
}
