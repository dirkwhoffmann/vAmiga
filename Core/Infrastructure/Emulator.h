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
#include <future>
#include <functional>

namespace vamiga {

class Emulator : public Thread, public Synchronizable {

    friend class API;
    friend class VAmiga;

public:

    // User default settings
    static Defaults defaults;

    // Result of the latest inspection
    utl::Backed<EmulatorInfo> info;
    utl::Backed<EmulatorMetrics> metrics;

private:

    // The main emulator instance
    Amiga main = Amiga(*this, 0);

    // The run-ahead instance (created if RUN_AHEAD_FRAMES > 0)
    std::unique_ptr<Amiga> ahead;

    // Counts the number of created clones
    isize clones = 0;
    
    // Indicates if the run-ahead instance needs to be updated
    bool isDirty = true;

    // Incoming external events
    CmdQueue cmdQueue;

    // Texture lock
    utl::Mutex textureLock;


    //
    // Methods
    //

public:

    Emulator();
    ~Emulator();

    // Launches the emulator thread
    void launch(const void *listener, Callback *func);

    // Initializes all components
    void initialize() override;

    // Forces to recreate the run-ahead instance in the next frame
    void markAsDirty() { isDirty = true; }


    //
    // Methods from CoreComponent
    //

public:

    const char *objectName() const override { return "Emulator"; }

private:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Analyzing
    //

public:

    EmulatorInfo cacheInfo() const;
    EmulatorMetrics cacheMetrics() const;


    //
    // Main API for configuring the emulator
    //

public:

    // Queries an option
    i64 get(Opt opt, isize objid = 0) const;

    // Checks an option
    void check(Opt opt, i64 value, const std::vector<isize> objids = { });

    // Sets an option
    void set(Opt opt, i64 value, const std::vector<isize> objids = { });

    // Convenience wrappers
    void set(Opt opt, const string &value, const std::vector<isize> objids = { });
    void set(const string &opt, const string &value, const std::vector<isize> objids = { });

    // Configures the emulator to match a specific configuration
    void set(ConfigScheme scheme);

    // Powers off and reverts to the default configuration
    void revertToDefaultConfig();


    //
    // Methods from Thread
    //

private:

    void update() override;
    bool shouldWarp() const;
    isize missingFrames() const override;
    void computeFrame() override;

    void _powerOn() override { main.powerOn(); }
    void _powerOff() override { main.powerOff(); }
    void _pause() override { main.pause(); }
    void _run() override { main.run(); }
    void _halt() override { main.halt(); }
    void _warpOn() override { main.warpOn(); }
    void _warpOff() override { main.warpOff(); }
    void _trackOn() override { main.trackOn(); }
    void _trackOff() override { main.trackOff(); }

    void checkReady() override;


    //
    // Managing the run-ahead instance
    //

private:

    // Clones the run-ahead instance
    void cloneRunAheadInstance();

    // Clones the run-ahead instance and fast forwards it to the proper frame
    void recreateRunAheadInstance();


    //
    // Execution control
    //

public:

    void hardReset();
    void softReset();
    void stepInto();
    void stepOver();
    void finishLine();
    void finishFrame();

    //
    // Audio and Video
    //

    const Texture &getTexture() const;
    const Texture &getXrayTexture() const;
    void lockTexture() { textureLock.lock(); }
    void unlockTexture() { textureLock.unlock(); }


    //
    // Command queue
    //

public:

    // Feeds a command into the command queue
    void put(const Command &cmd, const string &str = "");
    void put(const Command &cmd, const string &str, CmdQueue::Completion completion);

    /* Runs a function on the emulator thread and waits until it is done. The
     * function is executed in update(), i.e., in the order of the commands
     * that were queued before, and never while a frame is computed or while
     * another thread has suspended the emulator. An exception thrown by the
     * function is passed on to the caller. Throws a CoreError (NOT_LAUNCHED)
     * if the emulator thread is not running, because the function must not
     * run on any other thread.
     */
    void call(const std::function<void()> &fn);

    /* Queues a function for the emulator thread and returns at once. The
     * function is executed under the same conditions as in call(), but the
     * caller does not wait for it. A failure of the function cannot be passed
     * on to the caller. If 'done' is given, it is called on the emulator
     * thread when the function has been carried out, with the exception it
     * threw (or null). It is also called, on the calling thread, if the
     * command queue is full and the request is dropped. Nothing is executed
     * as long as the emulator thread has not been launched.
     */
    void callAsync(std::function<void()> fn, CmdQueue::Completion done = nullptr);

    /* Sends a command and returns a future that becomes ready once the
     * emulator thread has carried it out. A command that fails makes get()
     * throw the exception. This only works while the emulator
     * thread is running: a thread that polls nothing never fulfills the
     * promise, so waiting on the future would block forever.
     */
    std::future<void> send(const Command &cmd, const string &str = "");

    /* Sends a command and waits until it has been carried out. A command that
     * fails makes this function throw the exception.
     *
     * Because waiting for a thread that does not process commands would block
     * forever, the function throws CoreError::NOT_LAUNCHED if the emulator
     * thread has not been launched yet, is halted or suspended, or if the
     * function is called from the emulator thread itself.
     */
    void sendAndWait(const Command &cmd, const string &str = "");
    void put(Cmd type, const string &str) { put(Command(type), str); }
    void put(Cmd type, i64 payload = 0, i64 payload2 = 0) { put(Command(type, payload, payload2)); }
    void put(Cmd type, ConfigCmd payload) { put(Command(type, payload)); }
    void put(Cmd type, KeyCmd payload) { put(Command(type, payload)); }
    void put(Cmd type, CoordCmd payload) { put(Command(type, payload)); }
    void put(Cmd type, GamePadCmd payload) { put(Command(type, payload)); }
    void put(Cmd type, AlarmCmd payload) { put(Command(type, payload)); }


    //
    // Debugging
    //

public:

    // Gets or sets an internal debug variable (only available in debug builds)
    /*
    static int getDebugVariable(DebugFlag flag);
    static void setDebugVariable(DebugFlag flag, bool val);
    */
};

}
