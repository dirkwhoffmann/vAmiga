// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "EmulatorTypes.h"
#include "ThreadTypes.h"
#include "CoreObject.h"
#include "Inspectable.h"
#include "Synchronizable.h"
#include "Configurable.h"
#include "Suspendable.h"
#include "Serializable.h"
#include "Concurrency.h"
#include <vector>
#include <functional>

namespace vamiga {

struct Description {

    const char *name;               //! Short name
    const char *description;        //! Textual descripiton
    const char *shell;              //! RetroShell access
};

typedef std::vector<Description> Descriptions;

class CoreComponent : 
public CoreObject, public Serializable, public Synchronizable, public Suspendable, public Configurable {

public:
    
    // Reference to the emulator this instance belongs to
    class Emulator &emulator;

    // Object identifier (to distinguish instances of the same component)
    const isize objid;

    // Sub components
    std::vector<CoreComponent *> subComponents;


    //
    // Initializing
    //

public:

    CoreComponent(Emulator& ref) : emulator(ref), objid(0) { }
    CoreComponent(Emulator& ref, isize id) : emulator(ref), objid(id) { }

    // Returns a reference to the description of this component
    virtual const Descriptions &getDescriptions() const = 0;
    const char *objectName() const override;
    const char *description() const override;
    const char *shellName() const;

    bool operator== (CoreComponent &other);
    bool operator!= (CoreComponent &other) { return !(other == *this); }

    /* This function is called inside the emulator's launch routine. It iterates
     * through all components and calls the _initialize() delegate.
     */
    void initialize();
    virtual void _initialize() { }
    
    /* Resets the component and it's subcomponents. Two reset modes are
     * distinguished:
     *
     *     hard: A hard reset restores the initial state. It resets the Amiga
     *           from an emulator point of view.
     *
     *     soft: A soft reset emulates a reset inside the virtual Amiga. It is
     *           used to emulate the RESET instruction of the CPU.
     */
    // void hardReset();
    // void softReset();

public:

    /* This function is called inside the C64 reset routines. It iterates
     * through all components and calls the _reset() delegate.
     */
    void reset(bool hard);
    virtual void _reset(bool hard) = 0;


    //
    // Configuring
    //

public:

    // Initializes all configuration items with their default values
    virtual void resetConfig();

    // Returns the target components for a given configuration option
    void routeOption(Option opt, std::vector<Configurable *> &result);


    //
    // Controlling the state (see Thread class for details)
    //
    
public:
    
    virtual bool isPoweredOff() const;
    virtual bool isPoweredOn() const;
    virtual bool isPaused() const;
    virtual bool isRunning() const;
    virtual bool isSuspended() const;
    virtual bool isHalted() const;

    void suspend() override;
    void resume() override;
    
    // Throws an exception if the emulator is not ready to power on
    virtual void isReady() const throws;

protected:
    
    void powerOn();
    void powerOff();
    void run();
    void pause();
    void halt();
    void warpOn();
    void warpOff();
    void trackOn();
    void trackOff();
    void focus();
    void unfocus();

    void powerOnOff(bool value) { value ? powerOn() : powerOff(); }
    void warpOnOff(bool value) { value ? warpOn() : warpOff(); }
    void trackOnOff(bool value) { value ? trackOn() : trackOff(); }

private:
    
    virtual void _isReady() const throws { }
    virtual void _powerOn() { }
    virtual void _powerOff() { }
    virtual void _run() { }
    virtual void _pause() { }
    virtual void _halt() { }
    virtual void _warpOn() { }
    virtual void _warpOff() { }
    virtual void _trackOn() { }
    virtual void _trackOff() { }
    virtual void _focus() { }
    virtual void _unfocus() { }


    //
    // Serializing
    //
    
public:
    
    // Returns the size of the internal state in bytes
    isize size();

    // Loads the internal state from a memory buffer
    virtual isize load(const u8 *buf) throws;
    virtual void _didLoad() { };

    // Saves the internal state to a memory buffer
    virtual isize save(u8 *buf);
    virtual void _didSave() { };


    //
    // Walking the component tree
    //

    void preoderWalk(std::function<void(CoreComponent *)> func);
    void postorderWalk(std::function<void(CoreComponent *)> func);


    //
    // Misc
    //

public:

    // Computes a checksum for this component
    u64 checksum(bool recursive = false);

    // Collects references to this components and all sub-components
    void collectComponents(std::vector<CoreComponent *> &result);

    // Exports the current configuration to a script file
    void exportConfig(std::ostream& ss, bool diff = false) const;

    // Exports only those options that differ from the default config
    void exportDiff(std::ostream& ss) const { exportConfig(ss, true); }
};

//
// Standard implementations of _reset
//

#define RESET_SNAPSHOT_ITEMS(hard) \
if (hard) { \
SerResetter resetter(true); \
*this << resetter; \
} else { \
SerResetter resetter(false); \
*this << resetter; \
}

}
