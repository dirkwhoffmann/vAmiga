// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_SNAPSHOT_INC
#define _AMIGA_SNAPSHOT_INC

#include "AmigaFile.h"

class Amiga;

// Snapshot header
typedef struct {
    
    // Magic bytes ('V','A','S','N','A','P')
    char magic[6];
    
    // Version number (V major.minor.subminor)
    u8 major;
    u8 minor;
    u8 subminor;
    
    // Screenshot
    struct {
        
        // Image width and height
        u16 width, height;
        
        // Raw screen buffer data
        u32 screen[(HPIXELS / 4) * (VPIXELS / 2)];
        
    } screenshot;
    
    // Date and time of snapshot creation
    time_t timestamp;
    
} SnapshotHeader;

class Snapshot : public AmigaFile {
 
    //
    // Class methods
    //
    
public:
    
    // Returns true iff buffer contains a snapshot.
    static bool isSnapshot(const u8 *buffer, size_t length);
    
    // Returns true iff buffer contains a snapshot of a specific version.
    static bool isSnapshot(const u8 *buffer, size_t length,
                           u8 major, u8 minor, u8 subminor);
    
    // Returns true iff buffer contains a snapshot with a supported version number.
    static bool isSupportedSnapshot(const u8 *buffer, size_t length);
    
    // Returns true iff buffer contains a snapshot with an outdated version number.
    static bool isUnsupportedSnapshot(const u8 *buffer, size_t length);
    
    // Returns true if path points to a snapshot file.
    static bool isSnapshotFile(const char *path);
    
    // Returns true if file points to a snapshot file of a specific version.
    static bool isSnapshotFile(const char *path,
                               u8 major, u8 minor, u8 subminor);
    
    // Returns true if file is a snapshot with a supported version number.
    static bool isSupportedSnapshotFile(const char *path);
    
    // Returns true if file is a snapshot with an outdated version number.
    static bool isUnsupportedSnapshotFile(const char *path);
    
    
    //
    // Creating and destructing
    //
    
    Snapshot();
    Snapshot(size_t capacity);
    
    // Allocates memory for storing the emulator state.
    bool setCapacity(size_t size);
    
    // Factory methods
    static Snapshot *makeWithFile(const char *filename);
    static Snapshot *makeWithBuffer(const u8 *buffer, size_t size);
    static Snapshot *makeWithAmiga(Amiga *amiga);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_SNAPSHOT; }
    const char *typeAsString() override { return "VAMIGA"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override;
    bool fileHasSameType(const char *filename) override;
    
    
    //
    // Accessing snapshot properties
    //
    
public:
    
    // Returns pointer to header data
    SnapshotHeader *getHeader() { return (SnapshotHeader *)data; }
    
    // Returns pointer to core data
    u8 *getData() { return data + sizeof(SnapshotHeader); }
    
    // Returns the timestamp
    time_t getTimestamp() { return getHeader()->timestamp; }
    
    // Returns a pointer to the screenshot data.
    unsigned char *getImageData() { return (unsigned char *)(getHeader()->screenshot.screen); }
    
    // Returns the screenshot image width
    unsigned getImageWidth() { return getHeader()->screenshot.width; }
    
    // Returns the screenshot image height
    unsigned getImageHeight() { return getHeader()->screenshot.height; }
    
    // Stores a screenshot inside this snapshot
    void takeScreenshot(Amiga *amiga);
    
};

#endif
