// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "AmigaFile.h"

class DiskFile : public AmigaFile {

    //
    // Querying disk properties
    //
    
public:
       
    virtual isize numCyls() const = 0;
    virtual isize numHeads() const = 0;
    virtual isize numSectors() const = 0;
    isize bsize() const { return 512; }
    isize numTracks() const { return numHeads() * numCyls(); }
    isize numBlocks() const { return numTracks() * numSectors(); }
    isize numBytes() const { return numBlocks() * bsize(); }

    
    //
    // Pretty-printing
    //
    
public:
    
    string describeGeometry();
    string describeCapacity();
};
