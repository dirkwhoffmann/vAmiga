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

    DiskDoctor(FileSystem& fs) : fs(fs) { }


    //
    // Methods from CoreObject
    //

private:

    const char *objectName() const override { return "DiskDoctor"; }
    void _dump(Category category, std::ostream &os) const override { }


    //
    // Integrity checking
    //

public:
    
    // Scans all long words in this block and returns the number of errors
    isize check(Block ref, bool strict) const;

    // Checks the integrity of a certain byte in this block
    Fault check(Block ref, isize pos, u8 *expected, bool strict) const;



};

}
