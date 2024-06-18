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

class Emulator : public Thread, public Synchronizable, public Configurable {

    friend class API;
    friend class VAmiga;

    ConfigOptions options = {

        OPT_EMU_SNAPSHOTS,
        OPT_EMU_SNAPSHOT_DELAY
    };

    EmulatorConfig config = { };

public:

    // The virtual Amiga
    Amiga main = Amiga(*this, 0);

private:

    // Initialization status
    bool initialized = false;

public:

    // External event queue
    CmdQueue cmdQueue;


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

    // Checks the initialization state
    bool isInitialized() const;


    //
    // Methods from CoreComponent
    //

public:

    const char *objectName() const override { return "Emulator"; }

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option opt) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option opt, i64 value) override;


    //
    // Main API for configuring the emulator
    //

public:

    // Resets all options
    void resetConfig();

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


    /*

    //
    // Audio and Video
    //

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

};

}

