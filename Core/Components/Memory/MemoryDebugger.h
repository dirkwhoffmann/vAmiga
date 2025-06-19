// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MemoryTypes.h"
#include "SubComponent.h"

namespace vamiga {

class MemoryDebugger final : public SubComponent
{
    Descriptions descriptions = {{

        .type           = Class::MemoryDebugger,
        .name           = "MemoryDebugger",
        .description    = "Memory Debugger",
        .shell          = ""
    }};

    Options options = {

    };


    //
    // Methods
    //

public:

    using SubComponent::SubComponent;

    MemoryDebugger& operator= (const Host& other) {

        return *this;
    }


    //
    // Methods from Serializable
    //

public:

    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override { };


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Managing memory
    //

public:
    
    // Returns a memory dump in ASCII, hex, or both
    template <Accessor A> const char *ascDump(u32 addr, isize bytes) const;
    template <Accessor A> const char *hexDump(u32 addr, isize bytes, isize sz = 1) const;
    template <Accessor A> const char *memDump(u32 addr, isize bytes, isize sz = 1) const;

    // Writes a memory dump into a stream. Returns the number of written bytes
    template <Accessor A> isize ascDump(std::ostream &os, u32 addr, isize lines) const;
    template <Accessor A> isize hexDump(std::ostream &os, u32 addr, isize lines, isize sz) const;
    template <Accessor A> isize memDump(std::ostream &os, u32 addr, isize lines, isize sz) const;

    // Searches a number sequence in memory
    i64 memSearch(const string &pattern, u32 addr, isize align);

    // Reads a value from memory
    u32 read(u32 addr, isize sz);

    // Writes a value into memory (multiple times). . Returns the number of written bytes
    isize write(u32 addr, u32 val, isize sz, isize repeats = 1);

    // Loads a chunk of memory from a stream or file
    void load(std::istream &is, u32 addr);
    void load(const fs::path &path, u32 addr);

    // Saves a chunk of memory to a stream or file
    void save(std::ostream &is, u32 addr, isize count);
    void save(const fs::path &path, u32 addr, isize count);


    //
    // Handling registers
    //
    
public:
    
    // Returns the name of a chipset register
    static const char *regName(u32 addr);

    bool isUnused(Reg reg) const;
    bool isReadable(Reg reg) const;
    bool isWritable(Reg reg) const;

    u16 readCs(Reg reg) const;
    void writeCs(Reg reg, u16 value);


    //
    // Displaying expressions
    //

public:
        
    // Displays a value in different number formats (hex, dec, bin, alpha)
    void convertNumeric(std::ostream &os, u8 value) const;
    void convertNumeric(std::ostream &os, u16 value) const;
    void convertNumeric(std::ostream &os, u32 value) const;
    void convertNumeric(std::ostream &os, const string &value) const;

private:
    
    template <typename T> void convertNumeric(std::ostream &os, T value, const char *fmt) const;
};

}
