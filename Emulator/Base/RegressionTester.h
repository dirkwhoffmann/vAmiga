// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"
#include "Constants.h"

class RegressionTester : public AmigaObject {

public:
        
    // Filename of the test image
    string dumpTexturePath = "texture";
    
    // Texture cutout
    isize x1 = 4 * (HBLANK_MAX + 1);
    isize y1 = VBLANK_MAX + 1;
    isize x2 = HPIXELS;
    isize y2 = VPIXELS;
    
private:
    
    // When the emulator exits, this value is returned to the test script
    u8 retValue = 0;

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "RegressionTester"; }
    
    
    //
    // Taking screenshots
    //

public:
    
    // Creates the test image and exits the emulator
    void dumpTexture(class Amiga &amiga) const;
    void dumpTexture(class Amiga &amiga, const string &filename) const;
    void dumpTexture(class Amiga &amiga, std::ostream& os) const;

    
    //
    // Handling errors
    //

public:
    
    // Assigns the return code
    void setErrorCode(u8 value);
};
