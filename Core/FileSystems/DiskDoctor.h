// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "CoreObject.h"

namespace vamiga {

class DiskDoctor final : public CoreObject {

    // Reference to the patient
    class FileSystem &fs;

public:

    // Result of the latest examination
    FSDiagnosis diagnosis;

public:

    DiskDoctor(FileSystem& fs) : fs(fs) { }


    //
    // Methods from CoreObject
    //

private:

    const char *objectName() const override { return "DiskDoctor"; }
    void _dump(Category category, std::ostream &os) const override { }


    //
    // Dumping debug information
    //

public:
    
    void dump(Block nr, std::ostream &os);


    //
    // Checking the file system integrity
    //

public:

    // Performs a full sanity check. Returns the number of errors. Stores details in 'diagnosis'
    isize xray(bool strict);
    isize xray(std::ostream &os, bool strict);

    // Scans all blocks. Returns the number of errors. Stores details in 'diagnosis'
    isize xrayBlocks(bool strict);
    isize xrayBlocks(std::ostream &os, bool strict);

    // Checks the block allocation table. Returns the number of errors. Stores details in 'diagnosis'
    isize xrayBitmap(bool strict);
    isize xrayBitmap(std::ostream &os, bool strict);

    // Scans a single block and returns the number of errors
    isize xray(Block ref, bool strict) const;
    isize xray(FSBlock &node, bool strict) const;

    // Checks the integrity of a certain byte in this block
    Fault xray(Block ref, isize pos, bool strict) const;
    Fault xray(Block ref, isize pos, bool strict, u8 *expected) const;
    Fault xray(FSBlock &node, isize pos, bool strict) const;
    Fault xray(FSBlock &node, isize pos, bool strict, u8 *expected) const;

private:

    // Checks if the type of a block matches one of the provides types
    Fault checkBlockType(Block nr, FSBlockType type) const;
    Fault checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const;

};

}
