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
AmigaSnapshot::isSnapshot(const uint8_t *buffer, size_t length)
{
    uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(buffer != NULL);
    
    if (length < sizeof(AmigaSnapshotHeader)) return false;
    return matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
AmigaSnapshot::isSnapshot(const uint8_t *buffer, size_t length,
                          uint8_t major, uint8_t minor, uint8_t subminor)
{
    if (!isSnapshot(buffer, length)) return false;
    return buffer[6] == major && buffer[7] == minor && buffer[8] == subminor;
}

bool
AmigaSnapshot::isSupportedSnapshot(const uint8_t *buffer, size_t length)
{
    return isSnapshot(buffer, length, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
AmigaSnapshot::isUnsupportedSnapshot(const uint8_t *buffer, size_t length)
{
    return isSnapshot(buffer, length) && !isSupportedSnapshot(buffer, length);
}

bool
AmigaSnapshot::isSnapshotFile(const char *path)
{
     uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
AmigaSnapshot::isSnapshotFile(const char *path, uint8_t major, uint8_t minor, uint8_t subminor)
{
    uint8_t signature[] = { 'V', 'C', '6', '4', major, minor, subminor };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
AmigaSnapshot::isSupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
AmigaSnapshot::isUnsupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path) && !isSupportedSnapshotFile(path);
}

AmigaSnapshot::AmigaSnapshot()
{
    setDescription("Snapshot");
}

AmigaSnapshot::AmigaSnapshot(size_t capacity)
{
    uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    size = capacity + sizeof(AmigaSnapshotHeader);
    data = new uint8_t[size];
    
    AmigaSnapshotHeader *header = (AmigaSnapshotHeader *)data;
    
    for (unsigned i = 0; i < sizeof(signature); i++)
        header->magic[i] = signature[i];
    header->major = V_MAJOR;
    header->minor = V_MINOR;
    header->subminor = V_SUBMINOR;
    header->timestamp = time(NULL);
}

AmigaSnapshot *
AmigaSnapshot::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    AmigaSnapshot *snapshot = NULL;
    
    if (isSnapshot(buffer, length)) {
        
        snapshot = new AmigaSnapshot();
        
        if (!snapshot->readFromBuffer(buffer, length)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

AmigaSnapshot *
AmigaSnapshot::makeWithFile(const char *path)
{
    AmigaSnapshot *snapshot = NULL;
    
    if (isSnapshotFile(path)) {
        
        snapshot = new AmigaSnapshot();
        
        if (!snapshot->readFromFile(path)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

AmigaSnapshot *
AmigaSnapshot::makeWithAmiga(Amiga *amiga)
{
    /* This routine has temporarily been enriched with debug code.
     * REMOVE ASAP
     */

    // Take a snapshot with the old approach
    AmigaSnapshot *snapshot = new AmigaSnapshot(amiga->stateSize());
    snapshot->takeScreenshot(amiga);
    uint8_t *ptr = snapshot->getData();
    clock_t start = clock();
    size_t count = amiga->saveToBuffer(ptr);
    clock_t end = clock();
    double elapsed = (double(end - start) / double(CLOCKS_PER_SEC));
    printf("First snapshot taken (%f msec)\n", 1000.0 * elapsed);

    // Take the snapshot with the new approach
    AmigaSnapshot *snapshot2 = new AmigaSnapshot(amiga->stateSize());
    uint8_t *ptr2 = snapshot2->getData();
    start = clock();
    size_t count2 = amiga->saveToBufferNew(ptr2);
    end = clock();
    elapsed = (double(end - start) / double(CLOCKS_PER_SEC));
    printf("Second snapshot taken (%f msec)\n", 1000.0 * elapsed);

    // Verify that snapshot 1 and snapshot 2 match
    if (count != count2) {
        printf("Size mismatch (%zu <-> %zu)\n", count, count2);
    }
    size_t size = amiga->stateSize();
    printf("Verifying %zu bytes at %p and %p\n", size, ptr, ptr2);
    for (size_t i = 0; i < size; i++) {
        if (ptr[i] != ptr2[i]) {
            printf("Value mismatch at %zu (%d <-> %d)\n", i, ptr[i], ptr2[i]);
            assert(false);
        }
    }

    // Reload the snapshot to test the loader
    printf("Let's try to reload the snapshot\n");
    amiga->loadFromSnapshotUnsafe(snapshot2);

    // Take another snapshot
    AmigaSnapshot *snapshot3 = new AmigaSnapshot(amiga->stateSize());
    uint8_t *ptr3 = snapshot3->getData();
    start = clock();
    size_t count3 = amiga->saveToBufferNew(ptr3);
    end = clock();
    elapsed = (double(end - start) / double(CLOCKS_PER_SEC));
    printf("Third snapshot taken (%f msec)\n", 1000.0 * elapsed);

    // Verify that snapshot 2 and snapshot 3 match
    if (count2 != count3) {
        printf("Size mismatch (%zu <-> %zu)\n", count2, count3);
    }
    size = amiga->stateSize();
    printf("Verifying %zu bytes at %p and %p\n", size, ptr2, ptr3);
    for (size_t i = 0; i < size; i++) {
        if (ptr2[i] != ptr3[i]) {
            printf("Value mismatch at %zu (%d <-> %d)\n", i, ptr2[i], ptr3[i]);
            assert(false);
        }
    }

    return snapshot;
}

bool
AmigaSnapshot::bufferHasSameType(const uint8_t* buffer, size_t length)
{
    return AmigaSnapshot::isSnapshot(buffer, length);
}

bool
AmigaSnapshot::fileHasSameType(const char *path)
{
    return AmigaSnapshot::isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

void
AmigaSnapshot::takeScreenshot(Amiga *amiga)
{
    AmigaSnapshotHeader *header = (AmigaSnapshotHeader *)data;
    
    uint32_t *source = (uint32_t *)amiga->denise.getStableLongFrame().data;
    uint32_t *target = header->screenshot.screen;

    // Texture cutout and scaling factors
    unsigned dx = 4;
    unsigned dy = 2;
    unsigned xStart = 0, xEnd = HPIXELS;
    unsigned yStart = 0, yEnd = VPIXELS;
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
