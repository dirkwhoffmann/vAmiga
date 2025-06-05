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

class RegressionTester final : public SubComponent {

    Descriptions descriptions = {{

        .type           = Class::RegressionTester,
        .name           = "Regression",
        .description    = "Regression Tester",
        .shell          = "regression"
    }};

    Options options = {

    };

    static constexpr isize X1 = 4 * 0x31;
    static constexpr isize Y1 = PAL::VBLANK_MAX + 1;
    static constexpr isize X2 = HPIXELS;
    static constexpr isize Y2 = VPIXELS - 2;

public:

    // Filename of the test image
    fs::path dumpTexturePath = fs::path("texture");

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
    
    RegressionTester& operator= (const RegressionTester& other) {

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream &os) const override { }

    
    //
    // Methods from CoreComponent
    //

private:
    
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);
        
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Running a regression test
    //

public:

    // Reverts to factory settings
    void prepare(ConfigScheme scheme, const fs::path &rom = {}, const fs::path &ext = {});
    
    // Runs a test case
    void run(const fs::path &adf);
    
    // Creates the test image and exits the emulator
    void dumpTexture(Amiga &amiga);
    void dumpTexture(Amiga &amiga, const fs::path &filename);
    void dumpTexture(Amiga &amiga, std::ostream &os);

    
    //
    // Handling errors
    //

public:
    
    // Assigns the return code
    void setErrorCode(u8 value);
};

}
