// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DMS_FILE_H
#define _DMS_FILE_H

#include "ADFFile.h"

class DMSFile : public AmigaFile {

    ADFFile *adf = nullptr;
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff the provided buffer contains a DMS file.
    static bool isDMSBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to a DMS file.
    static bool isDMSFile(const char *path);
    
    
    //
    // Creating and destructing
    //
    
    DMSFile();
    
    // Factory methods
    static DMSFile *makeWithBuffer(const u8 *buffer, size_t length);
    static DMSFile *makeWithFile(const char *path);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_DMS; }
    const char *typeAsString() override { return "DMS"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isDMSBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isDMSFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
};

#endif
