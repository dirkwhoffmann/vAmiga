// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "va_std.h"

void sprintd(char *s, int digits, uint64_t value)
{
    for (int i = digits - 1; i >= 0; i--) {
        uint8_t digit = value % 10;
        s[i] = '0' + digit;
        value /= 10;
    }
    s[digits] = 0;
}

void sprintx(char *s, int digits, uint64_t value)
{
    for (int i = digits - 1; i >= 0; i--) {
        uint8_t digit = value % 16;
        s[i] = (digit <= 9) ? ('0' + digit) : ('A' + digit - 10);
        value /= 16;
    }
    s[digits] = 0;
}

void sprintb(char *s, int digits, uint64_t value)
{
    for (int i = digits - 1; i >= 0; i--) {
        s[i] = (value & 0x01) ? '1' : '0';
        value >>= 1;
    }
    s[digits] = 0;
}

char *
extractFilename(const char *path)
{
    assert(path != NULL);
    
    const char *pos = strrchr(path, '/');
    return pos ? strdup(pos + 1) : strdup(path);
}

char *
extractSuffix(const char *path)
{
    assert(path != NULL);
    
    const char *pos = strrchr(path, '.');
    return pos ? strdup(pos + 1) : strdup("");
}

char *
extractFilenameWithoutSuffix(const char *path)
{
    assert(path != NULL);
    
    char *result;
    char *filename = extractFilename(path);
    char *suffix   = extractSuffix(filename);
    
    if (strlen(suffix) == 0)
        result = strdup(filename);
    else
        result = strndup(filename, strlen(filename) - strlen(suffix) - 1);
    
    free(filename);
    free(suffix);
    return result;
}

bool
checkFileSuffix(const char *filename, const char *suffix)
{
    assert(filename != NULL);
    assert(suffix != NULL);
    
    if (strlen(suffix) > strlen(filename))
        return false;
    
    filename += (strlen(filename) - strlen(suffix));
    if (strcmp(filename, suffix) == 0)
        return true;
    else
        return false;
}

long
getSizeOfFile(const char *filename)
{
    struct stat fileProperties;
    
    if (filename == NULL)
        return -1;
    
    if (stat(filename, &fileProperties) != 0)
        return -1;
    
    return fileProperties.st_size;
}

bool
checkFileSize(const char *filename, long size)
{
    return checkFileSizeRange(filename, size, size);
}

bool
checkFileSizeRange(const char *filename, long min, long max)
{
    long filesize = getSizeOfFile(filename);
    
    if (filesize == -1)
        return false;
    
    if (min > 0 && filesize < min)
        return false;
    
    if (max > 0 && filesize > max)
        return false;
    
    return true;
}

bool
checkFileHeader(const char *filename, const uint8_t *header)
{
    int i, c;
    bool result = true;
    FILE *file;
    
    assert(filename != NULL);
    assert(header != NULL);
    
    if ((file = fopen(filename, "r")) == NULL)
        return false;
    
    for (i = 0; header[i] != 0; i++) {
        c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}

bool
matchingFileHeader(const char *path, const uint8_t *header, size_t length)
{
    assert(path != NULL);
    assert(header != NULL);
    
    bool result = true;
    FILE *file;
    
    if ((file = fopen(path, "r")) == NULL)
        return false;
    
    for (unsigned i = 0; i < length; i++) {
        int c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}


bool matchingBufferHeader(const uint8_t *buffer, const uint8_t *header, size_t length)
{
    assert(buffer != NULL);
    assert(header != NULL);
    
    for (unsigned i = 0; i < length; i++) {
        if (header[i] != buffer[i])
            return false;
    }
    
    return true;
}

void
sleepMicrosec(unsigned usec)
{
    if (usec > 0 && usec < 1000000) {
        usleep(usec);
    }
}

int64_t
sleepUntil(uint64_t kernelTargetTime, uint64_t kernelEarlyWakeup)
{
    uint64_t now = mach_absolute_time();
    int64_t jitter;
    
    if (now > kernelTargetTime) {
        printf("Too slow\n");
        return 0;
    }
    
    // Sleep
    // printf("Sleeping for %lld\n", kernelTargetTime - kernelEarlyWakeup);
    mach_wait_until(kernelTargetTime - kernelEarlyWakeup);
    
    // Count some sheep to increase precision
    unsigned sheep = 0;
    do {
        jitter = mach_absolute_time() - kernelTargetTime;
        sheep++;
    } while (jitter < 0);
    // printf("Counted %d sheep (%lld)\n", sheep, jitter);
    
    return jitter;
}

uint64_t
fnv_1a(uint8_t *addr, size_t size)
{
    if (addr == NULL || size == 0) return 0;
    
    uint64_t hash = fnv_1a_init();
    
    for (size_t i = 0; i < size; i++) {
        hash = fnv_1a_it(hash, (uint64_t)addr[i]);
    }

    return hash;
}

bool releaseBuild()
{
#ifdef NDEBUG
    return true;
#else
    return false;
#endif
}
