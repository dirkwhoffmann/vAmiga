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
#include "Defaults.h"
#include "Amiga.h"
#include "Host.h"
#include "Thread.h"
#include "CmdQueue.h"

namespace vamiga {

class Emulator : public Thread, public Synchronizable, public Inspectable<EmulatorInfo, EmulatorStats> {

    friend class API;
    friend class VAmiga;

public:

    // The virtual Amiga
    Amiga main = Amiga(*this, 0);

public:

    // User default settings
    static Defaults defaults;

    // Incoming external events
    CmdQueue cmdQueue;

    // Host system information
    Host host = Host(*this);


    //
    // Methods
    //

public:

    Emulator();
    ~Emulator();

    // Launches the emulator thread
    void launch(const void *listener, Callback *func);

    // Initializes all components
    void initialize();


    //
    // Methods from CoreComponent
    //

public:

    const char *objectName() const override { return "Emulator"; }

private:

    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(EmulatorInfo &result) const override;
    void cacheStats(EmulatorStats &result) const override;
    

    //
    // Main API for configuring the emulator
    //

public:

    // Queries an option
    i64 get(Option opt, isize id = 0) const throws;

    // Checks an option
    void check(Option opt, i64 value, const std::vector<isize> objids = { }) throws;

    // Sets an option
    void set(Option opt, i64 value, const std::vector<isize> objids = { }) throws;

    // Convenience wrappers
    void set(Option opt, const string &value, const std::vector<isize> objids = { }) throws;
    void set(const string &opt, const string &value, const std::vector<isize> objids = { }) throws;

    // Configures the emulator to match a specific Amiga model
    void set(ConfigScheme model);

public: // private

    // Returns the target component for an option
    std::vector<Configurable *> routeOption(Option opt);
    std::vector<const Configurable *> routeOption(Option opt) const;

    // Overrides a config option if the corresponding debug option is enabled
    i64 overrideOption(Option opt, i64 value) const;


    //
    // Methods from Thread
    //

private:

    void update() override;
    bool shouldWarp() const;
    isize missingFrames() const override;
    void computeFrame() override;
    void recreateRunAheadInstance();

    void _powerOn() override { main.powerOn(); }
    void _powerOff() override { main.powerOff(); }
    void _pause() override { main.pause(); }
    void _run() override { main.run(); }
    void _halt() override { main.halt(); }
    void _warpOn() override { main.warpOn(); }
    void _warpOff() override { main.warpOff(); }
    void _trackOn() override { main.trackOn(); }
    void _trackOff() override { main.trackOff(); }

    void isReady() override;

public:

    double refreshRate() const override;


    //
    // Execution control
    //

public:

    void stepInto();
    void stepOver();


    //
    // Audio and Video
    //

    /*
    u32 *getTexture() const;
    u32 *getDmaTexture() const;
    */
    

    //
    // Command queue
    //

public:

    // Feeds a command into the command queue
    void put(const Cmd &cmd);
    void put(CmdType type, i64 payload) { put (Cmd(type, payload)); }

private:

    // Processes a command from the command queue
    void process(const Cmd &cmd);


    //
    // Debugging
    //

public:

    // Gets or sets an internal debug variable (only available in debug builds)
    static bool getDebugVariable(DebugFlag flag);
    static void setDebugVariable(DebugFlag flag, bool val);
};

}

