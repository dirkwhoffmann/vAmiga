// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DebuggerTypes.h"
#include "SubComponent.h"
// #include "Constants.h"

namespace vamiga {

class Debugger : public SubComponent {

private:

    //
    // Constructing
    //

public:

    using SubComponent::SubComponent;


    //
    // Methods from CoreObject
    //

private:

    const char *getDescription() const override { return "Debugger"; }
    void _dump(Category category, std::ostream& os) const override { }


    //
    // Methods from CoreComponent
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
    // Executing
    //

public:

    // Pauses or continues the emulation
    void stopAndGo();

    // Executes one CPU instruction
    void stepInto();

    // Executes to the instruction below the current PC
    void stepOver();


    //
    // Displaying expressions
    //

    // Displays a value in different number formats (hex, dec, bin, alpha)
    void convertNumeric(std::ostream& os, isize value) const;
    void convertNumeric(std::ostream& os, string value) const;
};

}
