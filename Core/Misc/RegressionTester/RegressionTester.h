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

/* Machine setups used by the regression tests. They are frozen test fixtures
 * and intentionally decoupled from ModelPreset, which describes the models
 * offered to the user. Changing a fixture alters the output of the vAmigaTS
 * tests and requires the reference images to be regenerated.
 */
enum class RegressionScheme : long
{
    A1000_OCS_1MB,
    A500_OCS_1MB,
    A500_ECS_1MB,
    A500_PLUS_1MB,
    A1200_2MB
};

struct RegressionSchemeEnum : utl::Reflectable<RegressionSchemeEnum, RegressionScheme>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(RegressionScheme::A1200_2MB);

    static const char *_key(RegressionScheme value)
    {
        switch (value) {

            case RegressionScheme::A1000_OCS_1MB:  return "A1000_OCS_1MB";
            case RegressionScheme::A500_OCS_1MB:   return "A500_OCS_1MB";
            case RegressionScheme::A500_ECS_1MB:   return "A500_ECS_1MB";
            case RegressionScheme::A500_PLUS_1MB:  return "A500_PLUS_1MB";
            case RegressionScheme::A1200_2MB:      return "A1200_2MB";
        }
        return "???";
    }
    static const char *help(RegressionScheme value)
    {
        switch (value) {

            case RegressionScheme::A1000_OCS_1MB:  return "Amiga 1000, OCS Chipset, 1MB RAM";
            case RegressionScheme::A500_OCS_1MB:   return "Amiga 500, OCS Chipset, 1MB RAM";
            case RegressionScheme::A500_ECS_1MB:   return "Amiga 500, ECS Chipset, 1MB RAM";
            case RegressionScheme::A500_PLUS_1MB:  return "Amiga 500+, ECS Chipset, 1MB RAM";
            case RegressionScheme::A1200_2MB:      return "Amiga 1200, AGA Chipset, 2MB RAM";
        }
        return "???";
    }
};

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

    // Reverts to factory settings and applies a test fixture
    void prepare(RegressionScheme scheme, const fs::path &rom = {}, const fs::path &ext = {});
    
private:

    // Configures the emulator according to a test fixture
    void apply(RegressionScheme scheme);

public:

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
