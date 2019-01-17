// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAFile.h"

VAFile::VAFile()
{
}

VAFile::~VAFile()
{
    dealloc();
    
    if (path)
        free(path);
}

void
VAFile::dealloc()
{
    if (data == NULL) {
        assert(size == 0);
        return;
    }
    
    delete[] data;
    data = NULL;
    size = 0;
    fp = -1;
    eof = -1;
}

void
VAFile::setPath(const char *str)
{
    assert(str != NULL);
    
    // Set path
    if (path) free(path);
    path = strdup(str);
}

void
VAFile::seek(long offset)
{
    eof = size;
    fp = (offset < eof) ? offset : -1;
}

int
VAFile::read()
{
    int result;
    
    assert(eof <= size);
    
    if (fp < 0)
        return -1;
    
    // Get byte
    result = data[fp++];
    
    // Check for end of file
    if (fp == eof)
        fp = -1;
    
    return result;
}

/*
const char *
VAFile::readHex(size_t num)
{
    assert(sizeof(name) > 3 * num);
    
    for (unsigned i = 0; i < num; i++) {
        
        int byte = read();
        if (byte == EOF) break;
        sprintf(name + (3 * i), "%02X ", byte);
    }
    
    return name;
}
*/

void
VAFile::flash(uint8_t *buffer, size_t offset)
{
    int byte;
    assert(buffer != NULL);
    
    seek(0);
    
    while ((byte = read()) != EOF) {
        buffer[offset++] = (uint8_t)byte;
    }
}

bool
VAFile::readFromBuffer(const uint8_t *buffer, size_t length)
{
    assert (buffer != NULL);
    
    dealloc();
    if ((data = new uint8_t[length]) == NULL)
        return false;
    
    memcpy(data, buffer, length);
    size = length;
    eof = length;
    fp = 0;
    return true;
}

bool
VAFile::readFromFile(const char *filename)
{
    assert (filename != NULL);
    
    bool success = false;
    uint8_t *buffer = NULL;
    FILE *file = NULL;
    struct stat fileProperties;
    
    // Check file type
    if (!hasSameType(filename)) {
        goto exit;
    }
    
    // Get file properties
    if (stat(filename, &fileProperties) != 0) {
        goto exit;
    }
    
    // Open file
    if (!(file = fopen(filename, "r"))) {
        goto exit;
    }
    
    // Allocate memory
    if (!(buffer = new uint8_t[fileProperties.st_size])) {
        goto exit;
    }
    
    // Read from file
    int c;
    for (unsigned i = 0; i < fileProperties.st_size; i++) {
        c = fgetc(file);
        if (c == EOF)
            break;
        buffer[i] = (uint8_t)c;
    }
    
    // Read from buffer (subclass specific behaviour)
    dealloc();
    if (!readFromBuffer(buffer, (unsigned)fileProperties.st_size)) {
        goto exit;
    }
    
    setPath(filename);
    success = true;
    
    debug(1, "File %s read successfully\n", path);
    
exit:
    
    if (file)
        fclose(file);
    if (buffer)
        delete[] buffer;
    
    return success;
}

size_t
VAFile::writeToBuffer(uint8_t *buffer)
{
    assert(data != NULL);
    
    if (buffer) {
        memcpy(buffer, data, size);
    }
    return size;
}

bool
VAFile::writeToFile(const char *filename)
{
    bool success = false;
    uint8_t *data = NULL;
    FILE *file;
    size_t filesize;
    
    // Determine file size
    filesize = writeToBuffer(NULL);
    if (filesize == 0)
        return false;
    
    // Open file
    assert (filename != NULL);
    if (!(file = fopen(filename, "w"))) {
        goto exit;
    }
    
    // Allocate memory
    if (!(data = new uint8_t[filesize])) {
        goto exit;
    }
    
    // Write to buffer
    if (!writeToBuffer(data)) {
        goto exit;
    }
    
    // Write to file
    for (unsigned i = 0; i < filesize; i++) {
        fputc(data[i], file);
    }
    
    success = true;
    
exit:
    
    if (file)
        fclose(file);
    if (data)
        delete[] data;
    
    return success;
}
