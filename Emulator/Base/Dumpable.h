// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <iostream>

namespace vamiga {

enum class Category
{
    BankMap, 
    Beam,
    Beamtraps,
    Blocks,
    Breakpoints,
    Bus,
    Catchpoints,
    Config,
    Current,
    Debug,
    Defaults,
    Disk,
    Dma,
    Drive,
    Events,
    FileSystem,
    Geometry,
    Hunks,
    List1, 
    List2,
    Parameters,
    Partitions,
    Properties,
    Registers,
    Sections,
    Segments, 
    Signals,
    Slots,
    State,
    Stats,
    Status,
    SwTraps,
    Tod,
    Vectors,
    Volumes, 
    Watchpoints
};

class Dumpable {

public:

    virtual ~Dumpable() = default;
    virtual void _dump(Category category, std::ostream& ss) const { }

    void dump(Category category, std::ostream& ss) const;
    void dump(Category category) const;
};

}
