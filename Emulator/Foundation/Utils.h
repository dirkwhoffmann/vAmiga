// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Commons.h"

#include "AmigaConstants.h"
#include "Debug.h"
#include "Errors.h"
#include "AmigaTypes.h"

#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <list>
#include <set>
#include <sstream>
#include <fstream>


//
// Pretty printing
//




//
// Handling strings
//


//
// Handling files
//

// Extracts a certain component from a path
string extractPath(const string &path);
string extractName(const string &path);
string extractSuffix(const string &path);

// Strips a certain component from a path
string stripPath(const string &path);
string stripName(const string &path);
string stripSuffix(const string &path);

// Returns the size of a file in bytes
isize getSizeOfFile(const string &path);
isize getSizeOfFile(const char *path);

// Checks if a path points to a directory
bool isDirectory(const string &path);
bool isDirectory(const char *path);

// Returns the number of files in a directory
isize numDirectoryItems(const string &path);
isize numDirectoryItems(const char *path);

// Checks the header signature (magic bytes) of a stream or buffer
bool matchingStreamHeader(std::istream &stream, const u8 *header, isize len);
bool matchingBufferHeader(const u8 *buffer, const u8 *header, isize len);

// Loads a file from disk
bool loadFile(const char *path, u8 **buffer, isize *size);
bool loadFile(const char *path, const char *name, u8 **buffer, isize *size);


//
// Handling streams
//

isize streamLength(std::istream &stream);

