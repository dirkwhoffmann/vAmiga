// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "GuardListTypes.h"
#include "MoiraDebugger.h"

namespace vamiga {

class GuardList {

protected:

    class Emulator &emu;
    moira::Guards _guards;
    moira::Guards &guards;

public:

    bool needsCheck = false;


    //
    // Constructing
    //

public:

    GuardList(Emulator &emu) : emu(emu), guards(_guards) { }
    GuardList(Emulator &emu, moira::Guards &guards) : emu(emu), guards(guards) { }
    virtual ~GuardList() { }

    //
    // Inspecting the guard list
    //

    long elements() const { return guards.elements(); }
    std::optional<GuardInfo> guardNr(long nr) const;
    std::optional<GuardInfo> guardAt(u32 addr) const;
    std::optional<GuardInfo> hit() const;


    //
    // Adding or removing guards
    //

    bool isSet(long nr) const { return guards.isSet(nr); }
    bool isSetAt(u32 addr) const { return guards.isSetAt(addr); }

    void setAt(u32 target, isize ignores = 0);
    void moveTo(isize nr, u32 newTarget);

    void remove(isize nr);
    void removeAt(u32 target);
    void removeAll();


    //
    // Enabling or disabling guards
    //

    bool isEnabled(long nr) const { return guards.isEnabled(nr); }
    bool isEnabledAt(u32 addr) const { return guards.isEnabledAt(addr); }
    bool isDisabled(long nr) const { return guards.isDisabled(nr); }
    bool isDisabledAt(u32 addr) const { return guards.isDisabledAt(addr); }
    bool eval(u32 addr, moira::Size S = moira::Byte) { return guards.eval(addr, S); }

    void enable(isize nr);
    void enableAt(u32 target);
    void enableAll();
    void disable(isize nr);
    void disableAt(u32 target);
    void disableAll();
    void toggle(isize nr);
    void toggleAt(u32 target);

    void ignore(long nr, long count);


    //
    // Internals
    //

    // Updates the needsCheck variable
    void update();

public:

    virtual void setNeedsCheck(bool value) { };
};

}
