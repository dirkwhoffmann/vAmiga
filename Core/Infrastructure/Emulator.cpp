// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Emulator.h"
#include "Amiga.h"
#include "CmdQueue.h"
#include "utl/io.h"
#include <algorithm>

namespace vamiga {

// Perform some consistency checks
static_assert(sizeof(i8)  == 1, "i8  size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8)  == 1, "u8  size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");

Defaults
Emulator::defaults;

Emulator::Emulator()
{
    info.bind([this] { return cacheInfo(); } );
    metrics.bind([this] { return cacheMetrics(); } );
}

Emulator::~Emulator()
{
    halt();
}

void
Emulator::launch(const void *listener, Callback *func)
{
    if (force::LAUNCH_ERROR) throw CoreError(CoreError::LAUNCH);

    // Connect the listener to the message queue of the main instance
    if (listener && func) { main.msgQueue.setListener(listener, func); }

    // Disable the message queue of the run-ahead instance
    ahead.msgQueue.disable();

    // Launch the emulator thread
    Thread::launch();
}

void
Emulator::initialize()
{
    baseTime = utl::Time::now();

    // Make sure this function is only called once
    if (isInitialized()) throw CoreError(CoreError::LAUNCH, "The emulator is already initialized.");

    // Initialize all components
    main.initialize();
    ahead.initialize();

    // Setup the default configuration
    revertToDefaultConfig();

    // Get the runahead instance up-to-date
    ahead = main;

    // Switch state
    state = ExecState::OFF;

    // Mark the thread as initialized
    initLatch.count_down();
}

void
Emulator::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    if (category == Category::Debug) {

        auto cs = getChannels();
        std::sort(cs.begin(), cs.end(),
                  [](const auto& a, const auto& b) { return a.name < b.name; });

        for (auto c : cs) {

            os << tab(c.name);
            if (c.level.has_value()) {
                os << LogLevelEnum::key(*c.level) << std::endl;
            } else {
                os << "-" << std::endl;
            }
        }
    }
    
    if (category == Category::Defaults) {

        defaults.dump(category, os);
    }

    if (category == Category::RunAhead) {

        auto &pos = main.agnus.pos;
        auto &rua = ahead.agnus.pos;
        auto metr = metrics.current();

        os << "Primary instance:" << std::endl << std::endl;

        os << tab("Frame");
        os << dec(pos.frame) << std::endl;
        os << tab("Beam");
        os << "(" << dec(pos.v) << "," << dec(pos.h) << ")" << std::endl;

        os << "Run-ahead instance:" << std::endl << std::endl;

        os << tab("Clone nr");
        os << dec(metr.clones) << std::endl;
        os << tab("Frame");
        os << dec(rua.frame) << std::endl;
        os << tab("Beam");
        os << " (" << dec(rua.v) << "," << dec(rua.h) << ")" << std::endl;
    }
    
    if (category == Category::State) {

        os << tab("Execution state");
        os << ExecStateEnum::key(state) << std::endl;
        os << tab("Power");
        os << bol(isPoweredOn()) << std::endl;
        os << tab("Running");
        os << bol(isRunning()) << std::endl;
        os << tab("Suspended");
        os << bol(isSuspended()) << std::endl;
        os << tab("Warping");
        os << bol(isWarping()) << std::endl;
        os << tab("Tracking");
        os << bol(isTracking()) << std::endl;
        os << std::endl;
    }
}

EmulatorInfo
Emulator::cacheInfo() const
{
    EmulatorInfo info;

    info.state = state;
    info.powered = isPoweredOn();
    info.paused = isPaused();
    info.running = isRunning();
    info.suspended = isSuspended();
    info.warping = isWarping();
    info.tracking = isTracking();

    return info;
}

EmulatorMetrics
Emulator::cacheMetrics() const
{
    EmulatorMetrics stats;

    stats.cpuLoad = cpuLoad;
    stats.fps     = fps;
    stats.resyncs = resyncs;
    stats.clones  = clones;

    return stats;
}

i64
Emulator::get(Opt opt, isize objid) const
{
    return main.get(opt, objid);
}

void
Emulator::check(Opt opt, i64 value, const std::vector<isize> objids)
{
    return main.check(opt, value, objids);
}

void
Emulator::set(Opt opt, i64 value, const std::vector<isize> objids)
{
    return main.set(opt, value, objids);
}

void
Emulator::set(Opt opt, const string &value, const std::vector<isize> objids)
{
    return main.set(opt, value, objids);
}

void
Emulator::set(const string &opt, const string &value, const std::vector<isize> objids)
{
    return main.set(opt, value, objids);
}

void
Emulator::set(ConfigScheme scheme)
{
    main.set(scheme);
}

void
Emulator::revertToDefaultConfig()
{
    // Power off
    powerOff();

    // Setup the default configuration
    main.resetConfig();
    ahead.resetConfig();

    // Perform a hard reset
    main.hardReset();
    ahead.hardReset();
}

void
Emulator::update()
{
    // Switch warp mode on or off
    shouldWarp() ? warpOn() : warpOff();

    // Mark the run-ahead instance dirty when the command queue has entries
    isDirty |= !cmdQueue.empty;

    // Process all commands
    main.update(cmdQueue);
}

bool
Emulator::shouldWarp() const
{
    auto &config = main.getConfig();
    
    if (main.agnus.clock < SEC(config.warpBoot)) {
        
        return true;
    }
    
    switch (config.warpMode) {
            
        case Warp::AUTO:     return main.paula.diskController.spinning();
        case Warp::NEVER:    return false;
        case Warp::ALWAYS:   return true;
            
        default:
            fatalError;
    }
}

isize
Emulator::missingFrames() const
{
    auto &config = main.getConfig();

    // In VSYNC mode, compute exactly one frame per wakeup call
    if (config.vsync) return 1;

    // Compute the elapsed time
    auto elapsed = utl::Time::now() - baseTime;

    // Compute which frame should be reached by now
    auto target = elapsed.asNanoseconds() * i64(main.refreshRate()) / 1000000000;

    // Compute the number of missing frames
    return isize(target - frameCounter);
}

void
Emulator::computeFrame()
{
    auto &config = main.getConfig();

    if (config.runAhead > 0) {

        try {

            // Run the main instance
            main.computeFrame();

            // Recreate the run-ahead instance if necessary
            if (isDirty || debug::RUA_ON_STEROIDS) recreateRunAheadInstance();

            // Run the runahead instance
            ahead.computeFrame();

        } catch (StateChangeException &) {

            isDirty = true;
            throw;
        }

    } else {

        // Only run the main instance
        main.computeFrame();
    }
}

void
Emulator::isReady()
{
    main.isReady();
}

void
Emulator::cloneRunAheadInstance()
{
    clones++;

    // Recreate the runahead instance from scratch
    ahead = main; isDirty = false;

    if constexpr (debug::RUA_CHECKSUM) {

        if (ahead != main) {
            
            main.diff(ahead);
            fatal("Corrupted run-ahead clone detected");
        }
    }
}

void
Emulator::recreateRunAheadInstance()
{
    assert(main.config.runAhead > 0);

    auto &config = main.getConfig();

    // Clone the main instance
    if constexpr (debug::RUA_DEBUG) {
        utl::StopWatch watch("Run-ahead: Clone");
        cloneRunAheadInstance();
    } else {
        cloneRunAheadInstance();
    }

    // Advance to the proper frame
    if constexpr (debug::RUA_DEBUG) {
        utl::StopWatch watch("Run-ahead: Fast-forward");
        ahead.fastForward(config.runAhead - 1);
    } else {
        ahead.fastForward(config.runAhead - 1);
    }
}

void
Emulator::hardReset()
{
    main.hardReset();
}

void
Emulator::softReset()
{
    main.softReset();
}

void
Emulator::stepInto()
{
    if (isRunning()) return;
    main.cpu.debugger.stepInto();
    run();
}

void
Emulator::stepOver()
{
    if (isRunning()) return;
    main.cpu.debugger.stepOver();
    run();
}

void
Emulator::finishLine()
{
    if (isRunning()) return;
    main.agnus.dmaDebugger.eolTrap = true;
    run();
}

void
Emulator::finishFrame()
{
    if (isRunning()) return;
    main.agnus.dmaDebugger.eofTrap = true;
    run();
}

const Texture &
Emulator::getTexture() const
{
    if (isRunning()) {

        // In run-ahead mode, return the texture from the run-ahead instance
        if (main.config.runAhead > 0) {
            return ahead.videoPort.getTexture();
        }

        // In run-behind mode, return a texture from the texture buffer
        if (main.config.runAhead < 0) {
            return main.videoPort.getTexture(main.config.runAhead);
        }
    }

    // Return the most recent texture from the main instance
    return main.videoPort.getTexture();
}

void
Emulator::put(const Command &cmd)
{
    cmdQueue.put(cmd);
}

}
