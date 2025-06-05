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

/* This class provides functionality for managing a guard list. The term "guard"
 * is used as a general term to denote breakpoints, watchpoints, catchpoints,
 * beamtraps, and similar constructs. Internally, the class utilizes an object
 * of type moira::Guards, which already offers the necessary functionality.
 * Encapsulating this functionality in a new class enables its use by other
 * components, such as the Copper, in addition to the CPU.
 */
class GuardList {

protected:

    // Reference to the emulator object
    // class Emulator &emu;

    // Reference to the emulator core
    class Amiga &amiga;

    // Reference to the guard list
    moira::Guards &guards;

    // This guard list is used if no custom list is provided in the constructor
    moira::Guards _guards;

    // Indicates if active guards are present
    bool needsCheck = false;


    //
    // Constructing
    //

public:

    GuardList(Amiga &amiga) : amiga(amiga), guards(_guards) { }
    GuardList(Amiga &amiga, moira::Guards &guards) : amiga(amiga), guards(guards) { }
    virtual ~GuardList() { }


    //
    // Inspecting the guard list
    //

public:

    long elements() const { return guards.elements(); }
    std::optional<GuardInfo> guardNr(long nr) const;
    std::optional<GuardInfo> guardAt(u32 addr) const;
    std::optional<GuardInfo> hit() const;


    //
    // Adding or removing guards
    //

public:

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

public:

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
    // Delegates
    //

public:

    virtual void setNeedsCheck(bool value) { };


    //
    // Internals
    //

private:

    // Updates the needsCheck variable
    void update();
};

}
