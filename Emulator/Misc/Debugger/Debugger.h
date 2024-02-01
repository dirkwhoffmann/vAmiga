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
    
    // Runs or pauses the emulator
    void stopAndGo();

    /* Executes a single instruction. This function is used for single-stepping
     * through the code inside the debugger. It starts the execution thread and
     * terminates it after the next instruction has been executed.
     */
    void stepInto();

    /* Runs the emulator until the instruction following the current one is
     * reached. This function is used for single-stepping through the code
     * inside the debugger. It sets a soft breakpoint to PC+n where n is the
     * length bytes of the current instruction and starts the emulator thread.
     */
    void stepOver();
};

}
