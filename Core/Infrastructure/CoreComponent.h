// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreComponentTypes.h"
#include "EmulatorTypes.h"
#include "CoreObject.h"
#include "Concurrency.h"
#include "Configurable.h"
#include "Inspectable.h"
#include "Serializable.h"
#include "Synchronizable.h"
#include <functional>

namespace vamiga {

struct Description {

    const Class type;               // Class identifier
    const char *name;               // Short name
    const char *description;        // Textual descripiton
    const char *shell;              // RetroShell access
    // std::vector<string> help;       // RetroShell custom help strings
};

typedef std::vector<Description> Descriptions;

class CoreComponent :
public CoreObject, public Serializable, public Synchronizable, public Configurable {

public:

    // Reference to the emulator this instance belongs to
    class Emulator &emulator;

    // Object identifier (to distinguish instances of the same component)
    const isize objid;

    // Subcomponents
    std::vector<CoreComponent *> subComponents;


    //
    // Initializers
    //
    
public:

    CoreComponent(Emulator& ref, isize id = 0) : emulator(ref), objid(id) { }


    //
    // Operators
    //

public:

    bool operator== (CoreComponent &other);
    bool operator!= (CoreComponent &other) { return !(other == *this); }


    //
    // Querying properties
    //

public:

    // Returns the description struct of this component
    virtual const Descriptions &getDescriptions() const = 0;

    // Returns certain elements from the description struct
    const char *objectName() const override;
    const char *description() const override;
    const char *shellName() const;
    
    // State properties (see Thread class for details)
    virtual bool isInitialized() const;
    virtual bool isPoweredOff() const;
    virtual bool isPoweredOn() const;
    virtual bool isPaused() const;
    virtual bool isRunning() const;
    virtual bool isHalted() const;

    // Throws an exception if the emulator is not ready to power on
    virtual void isReady() const throws;

    // Computes a checksum
    u64 checksum(bool recursive);

    // Performs sanity checks
    bool isEmulatorThread() const;
    bool isUserThread() const;
 

    //
    // Configuring
    //

public:

    // Initializes all configuration items with their default values
    virtual void resetConfig();

    // Returns the target component for a given configuration option
    Configurable *routeOption(Opt opt, isize objid);

    // Returns the fallback value for a config option
    i64 getFallback(Opt opt) const override;


    //
    // Controlling the state
    //

public:

    void initialize();
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


    //
    // Performing state changes
    //

private:

    virtual void _initialize() { }
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
    isize size(bool recursive = true);

    // Resets the internal state
    void reset(bool hard);
    virtual void _willReset(bool hard) { }
    virtual void _didReset(bool hard) { }

    // Convenience wrappers
    void hardReset() { reset(true); }
    void softReset() { reset(false); }

    // Loads the internal state from a memory buffer
    isize load(const u8 *buf) throws;
    virtual void _didLoad() { }

    // Saves the internal state to a memory buffer
    isize save(u8 *buf);
    virtual void _didSave() { }


    //
    // Working with subcomponents
    //

public:

    // Collects references to this components and all subcomponents
    std::vector<CoreComponent *> collectComponents();
    void collectComponents(std::vector<CoreComponent *> &result);

    // Traverses the component tree and applies a function
    void preoderWalk(std::function<void(CoreComponent *)> func);
    void postorderWalk(std::function<void(CoreComponent *)> func);


    //
    // Misc
    //

public:
    
    // Compares two components and reports differences (for debugging)
    void diff(CoreComponent &other);

    // Exports the current configuration as a script
    void exportConfig(const fs::path &path, bool diff = false, std::vector<Class> exclude = {}) const;
    void exportConfig(std::ostream &ss, bool diff = false, std::vector<Class> exclude = {}) const;

    // Exports only those options that differ from the default config
    void exportDiff(const fs::path &path, std::vector<Class> exclude = {}) const;
    void exportDiff(std::ostream &ss, std::vector<Class> exclude = {}) const;
};

}
