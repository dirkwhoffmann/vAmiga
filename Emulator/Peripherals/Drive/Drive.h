// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Drive.h"
#include "SubComponent.h"

class Drive : public SubComponent {

protected:
    
    // Number of the emulated drive (0 = df0 or hd0, 1 = df1 or hd1, etc.)
    const isize nr;
    
    
    //
    // Initializing
    //

public:

    Drive(Amiga& ref, isize nr);
    
    
    //
    // Querying device properties
    //
    
    
};
