// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Constants.h"
#include "AmigaTypes.h"

class RegressionTester : public SubComponent {

public:
        
    // Filename of the test image
    string dumpTexturePath = "texture";
    
    // Texture cutout
    isize x1 = 4 * 0x31;
    isize y1 = VBLANK_MAX + 1;
    isize x2 = HPIXELS;
    isize y2 = VPIXELS - 2;
    
private:
    
    // When the emulator exits, this value is returned to the test script
    u8 retValue = 0;

    
    //
    // Constructing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "RegressionTester"; }
    void _dump(Category category, std::ostream& os) const override { }

    
    //
    // Methods from AmigaComponent
    //
        
private:
    
    void _reset(bool hard) override { };
    
    
    //
    // Serializing
    //
    
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }
    
    
    //
    // Running a regression test
    //

public:

    // Reverts everything to factory settings
    void prepare(ConfigScheme scheme, string rom, string ext = "");
    
    // Runs a test case
    void run(string adf);
    
    // Creates the test image and exits the emulator
    void dumpTexture(class Amiga &amiga);
    void dumpTexture(class Amiga &amiga, const string &filename);
    void dumpTexture(class Amiga &amiga, std::ostream& os);

    
    //
    // Handling errors
    //

public:
    
    // Assigns the return code
    void setErrorCode(u8 value);
};
