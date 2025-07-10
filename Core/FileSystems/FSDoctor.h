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

class FSDoctor final : public CoreObject {

    // Reference to the patient
    class FileSystem &fs;

public:

    // Result of the latest examination
    FSDiagnosis diagnosis;

public:

    FSDoctor(FileSystem& fs) : fs(fs) { }


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

    // Scans all blocks. Returns the number of errors. Stores details in 'diagnosis'
    isize xray(bool strict);
    isize xray(bool strict, std::ostream &os, bool verbose);

    // Scans a single block and returns the number of errors
    isize xray(Block ref, bool strict) const;
    isize xray(Block ref, bool strict, std::ostream &os) const;
    isize xray(FSBlock &node, bool strict) const;
    isize xray(FSBlock &node, bool strict, std::ostream &os) const;

    // Checks the integrity of a certain byte or long word in this block
    FSBlockError xray8(Block ref, isize pos, bool strict, optional<u8> &expected) const;
    FSBlockError xray8(FSBlock &node, isize pos, bool strict, optional<u8> &expected) const;
    FSBlockError xray32(Block ref, isize pos, bool strict, optional<u32> &expected) const;
    FSBlockError xray32(FSBlock &node, isize pos, bool strict, optional<u32> &expected) const;

    // Checks the allocation table. Returns the number of errors. Stores details in 'diagnosis'
    isize xrayBitmap(bool strict);
    isize xrayBitmap(bool strict, std::ostream &os);

    // Rectifies all blocks
    void rectify(bool strict);

    // Rectifies a single block
    void rectify(Block ref, bool strict);
    void rectify(FSBlock &node, bool strict);

    // Rectifies the allocation table
    void rectifyBitmap(bool strict);
};

}
