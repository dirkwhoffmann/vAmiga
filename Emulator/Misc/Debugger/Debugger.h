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
#include "AmigaTypes.h"
#include "MemoryTypes.h"
#include "SubComponent.h"

namespace vamiga {

class Debugger : public SubComponent {

    Descriptions descriptions = {{

        .name           = "Debugger",
        .description    = "Hardware Debugger",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

public:

    // Last used address (current object location)
    u32 current = 0;


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

    void _dump(Category category, std::ostream& os) const override { }


    //
    // Methods from CoreComponent
    //

private:

    void _pause() override;
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }

    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


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

    // Returns a memory dump in ASCII, hex, or both
    template <Accessor A> const char *ascDump(u32 addr, isize bytes) const;
    template <Accessor A> const char *hexDump(u32 addr, isize bytes, isize sz = 1) const;
    template <Accessor A> const char *memDump(u32 addr, isize bytes, isize sz = 1) const;

    // Writes a memory dump into a stream
    template <Accessor A> void ascDump(std::ostream& os, u32 addr, isize lines);
    template <Accessor A> void hexDump(std::ostream& os, u32 addr, isize lines, isize sz);
    template <Accessor A> void memDump(std::ostream& os, u32 addr, isize lines, isize sz);

    // Searches a number sequence in memory
    i64 memSearch(const string &pattern, u32 addr, isize align);

    // Reads a value from memory
    u32 read(u32 addr, isize sz);

    // Writes a value into memory (multiple times)
    void write(u32 addr, u32 val, isize sz, isize repeats = 1);

    
    //
    // Handling registers
    //

    bool isUnused(ChipsetReg reg) const;
    bool isReadable(ChipsetReg reg) const;
    bool isWritable(ChipsetReg reg) const;

    u16 readCs(ChipsetReg reg) const;
    void writeCs(ChipsetReg reg, u16 value);


    //
    // Displaying expressions
    //

    // Displays a value in different number formats (hex, dec, bin, alpha)
    void convertNumeric(std::ostream& os, u8 value) const;
    void convertNumeric(std::ostream& os, u16 value) const;
    void convertNumeric(std::ostream& os, u32 value) const;
    void convertNumeric(std::ostream& os, string value) const;
};

}
