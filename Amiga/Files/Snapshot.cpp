// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

bool
Snapshot::isSnapshot(const uint8_t *buffer, size_t length)
{
    uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(buffer != NULL);
    
    if (length < sizeof(SnapshotHeader)) return false;
    return matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
Snapshot::isSnapshot(const uint8_t *buffer, size_t length,
                          uint8_t major, uint8_t minor, uint8_t subminor)
{
    if (!isSnapshot(buffer, length)) return false;
    return buffer[6] == major && buffer[7] == minor && buffer[8] == subminor;
}

bool
Snapshot::isSupportedSnapshot(const uint8_t *buffer, size_t length)
{
    return isSnapshot(buffer, length, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
Snapshot::isUnsupportedSnapshot(const uint8_t *buffer, size_t length)
{
    return isSnapshot(buffer, length) && !isSupportedSnapshot(buffer, length);
}

bool
Snapshot::isSnapshotFile(const char *path)
{
     uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
Snapshot::isSnapshotFile(const char *path, uint8_t major, uint8_t minor, uint8_t subminor)
{
    uint8_t signature[] = { 'V', 'C', '6', '4', major, minor, subminor };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
Snapshot::isSupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
Snapshot::isUnsupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path) && !isSupportedSnapshotFile(path);
}

Snapshot::Snapshot()
{
    setDescription("Snapshot");
}

Snapshot::Snapshot(size_t capacity)
{
    uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    size = capacity + sizeof(SnapshotHeader);
    data = new uint8_t[size];
    
    SnapshotHeader *header = (SnapshotHeader *)data;
    
    for (unsigned i = 0; i < sizeof(signature); i++)
        header->magic[i] = signature[i];
    header->major = V_MAJOR;
    header->minor = V_MINOR;
    header->subminor = V_SUBMINOR;
    header->timestamp = time(NULL);
}

Snapshot *
Snapshot::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    Snapshot *snapshot = NULL;
    
    if (isSnapshot(buffer, length)) {
        
        snapshot = new Snapshot();
        
        if (!snapshot->readFromBuffer(buffer, length)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

Snapshot *
Snapshot::makeWithFile(const char *path)
{
    Snapshot *snapshot = NULL;
    
    if (isSnapshotFile(path)) {
        
        snapshot = new Snapshot();
        
        if (!snapshot->readFromFile(path)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

Snapshot *
Snapshot::makeWithAmiga(Amiga *amiga)
{
    Snapshot *snapshot = new Snapshot(amiga->size());

    snapshot->takeScreenshot(amiga);
    amiga->save(snapshot->getData());

    return snapshot;
}

bool
Snapshot::bufferHasSameType(const uint8_t* buffer, size_t length)
{
    return Snapshot::isSnapshot(buffer, length);
}

bool
Snapshot::fileHasSameType(const char *path)
{
    return Snapshot::isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

void
Snapshot::takeScreenshot(Amiga *amiga)
{
    SnapshotHeader *header = (SnapshotHeader *)data;
    
    uint32_t *source = (uint32_t *)amiga->denise.pixelEngine.getStableLongFrame().data;
    uint32_t *target = header->screenshot.screen;

    // Texture cutout and scaling factors
    unsigned dx = 4;
    unsigned dy = 2;
    unsigned xStart = 4 * HBLANK_MAX, xEnd = HPIXELS + 4 * HBLANK_MIN;
    unsigned yStart = VBLANK_CNT, yEnd = VPIXELS;
    unsigned width  = (xEnd - xStart) / dx;
    unsigned height = (yEnd - yStart) / dy;

    source += xStart + yStart * HPIXELS;

    header->screenshot.width  = width;
    header->screenshot.height = height;
    
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            target[x] = source[x * dx];
        }
        source += dy * HPIXELS;
        target += width;
    }
}
