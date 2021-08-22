// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"
#include "FSDevice.h"

class Disk;

class HDFFile : public AmigaFile {
    
public:
    
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);
    
    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }

    
    //
    // Initializing
    //

public:

    HDFFile(const string &path) throws { init(path); }
    HDFFile(const u8 *buf, isize len) throws { init(buf, len); }

    const char *getDescription() const override { return "HDF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_HDF; }


    //
    // Querying volume information
    //

public:
    
    // Returns true if this image contains a rigid disk block
    bool hasRDB() const;
    
    // Returns the layout parameters of the hard drive
    isize numCyls() const;
    isize numSides() const;
    isize numSectors() const;
    isize numReserved() const;
    isize numBlocks() const;
    isize bsize() const;
    struct FSDeviceDescriptor layout();

    
    //
    // Querying partition information
    //

private:
    
    // Extracts the DOS revision number from a certain block
    FSVolumeType dos(isize blockNr);    
};
