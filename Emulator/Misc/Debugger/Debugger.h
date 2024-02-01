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
#include "MemoryTypes.h"
#include "SubComponent.h"

namespace vamiga {

class Debugger : public SubComponent {

    //
    // Static functions
    //

public:

    // Returns the name of a chipset register
    static const char *regName(u32 addr);


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
    // Controlling program execution
    //

public:

    // Pauses or continues the emulation
    void stopAndGo();

    // Executes one CPU instruction
    void stepInto();

    // Executes to the instruction below the current PC
    void stepOver();

    // Continues execution at the specified address
    void jump(u32 addr);


    //
    // Managing memory
    //

    // Returns 16 bytes of memory as an ASCII string
    template <Accessor A> const char *ascDump(u32 addr, isize numBytes) const;

    // Returns a certain amount of bytes as a string containing hex words
    template <Accessor A> const char *hexDump(u32 addr, isize numBytes) const;

    // Creates a memory dump
    template <Accessor A> void memDump(std::ostream& os, u32 addr, isize numLines = 16) const;

    // Creates a memory dump
    // void dumpMemory(std::ostream& os, u32 addr, isize fmt) const;


    //
    // Displaying expressions
    //

    // Displays a value in different number formats (hex, dec, bin, alpha)
    void convertNumeric(std::ostream& os, isize value) const;
    void convertNumeric(std::ostream& os, string value) const;
};

}
