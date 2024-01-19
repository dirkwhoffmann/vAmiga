// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Constants.h"
#include "AmigaTypes.h"

namespace vamiga {

class RegressionTester : public SubComponent {

    static constexpr isize X1 = 4 * 0x31;
    static constexpr isize Y1 = VBLANK_MAX + 1;
    static constexpr isize X2 = HPIXELS;
    static constexpr isize Y2 = VPIXELS - 2;

public:

    // Filename of the test image
    string dumpTexturePath = "texture";

    // Pixel area which is written to the test image
    isize x1 = X1;
    isize y1 = Y1;
    isize x2 = X2;
    isize y2 = Y2;
    
private:
    
    // When the emulator exits, this value is returned to the test script
    u8 retValue = 0;

    
    //
    // Constructing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "RegressionTester"; }
    void _dump(Category category, std::ostream& os) const override { }

    
    //
    // Methods from CoreComponent
    //

private:
    
    void _reset(bool hard) override { };
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }
    
    
    //
    // Running a regression test
    //

public:

    // Reverts to factory settings
    void prepare(ConfigScheme scheme, string rom = "", string ext = "");
    
    // Runs a test case
    void run(string adf);
    
    // Creates the test image and exits the emulator
    void dumpTexture(Amiga &amiga);
    void dumpTexture(Amiga &amiga, const string &filename);
    void dumpTexture(Amiga &amiga, std::ostream& os);

    
    //
    // Handling errors
    //

public:
    
    // Assigns the return code
    void setErrorCode(u8 value);
};

}
