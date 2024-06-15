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
#include "Aliases.h"

namespace vamiga {

Emulator::Emulator()
{

}

Emulator::~Emulator()
{

}

void
Emulator::launch(const void *listener, Callback *func)
{
    // Initialize the emulator if needed
    if (!isInitialized()) initialize();

    // Connect the listener to the message queue of the main instance
    main.msgQueue.setListener(listener, func);

    // Disable the message queue of the run-ahead instance
    // ahead.msgQueue.disable();

    // Launch the emulator thread
    Thread::launch();
}

void
Emulator::initialize()
{
    // Initialize all components
    // resetConfig();
    // host.resetConfig();
    main.initialize();
    // ahead.initialize();

    // Perform a hard reset
    main.hardReset();
    // ahead.hardReset();

    initialized = true;
}

bool
Emulator::isInitialized() const
{
    return initialized;
}

void
Emulator::_dump(Category category, std::ostream& os) const
{
    using namespace util;

}

std::vector<const Configurable *>
Emulator::routeOption(Option opt) const
{
    std::vector<const Configurable *> result;

    for (const auto &target : const_cast<Emulator *>(this)->routeOption(opt)) {
        result.push_back(const_cast<const Configurable *>(target));
    }
    return result;
}

std::vector<Configurable *>
Emulator::routeOption(Option opt)
{
    std::vector<Configurable *> result;

    // if (isValidOption(opt)) result.push_back(this);
    // if (host.isValidOption(opt)) result.push_back(&host);
    main.routeOption(opt, result);

    assert(!result.empty());
    return result;
}

void
Emulator::update()
{
    shouldWarp() ? warpOn() : warpOff();
}

bool
Emulator::shouldWarp() const
{
    auto config = main.getConfig();

    if (main.agnus.clock < SEC(config.warpBoot)) {

        return true;
    }

    switch (config.warpMode) {

        case WARP_AUTO:     return main.paula.diskController.spinning();
        case WARP_NEVER:    return false;
        case WARP_ALWAYS:   return true;

        default:
            fatalError;
    }
}

isize
Emulator::missingFrames() const
{
    auto config = main.getConfig();

    // In VSYNC mode, compute exactly one frame per wakeup call
    if (config.vsync) return 1;

    // Compute the elapsed time
    auto elapsed = util::Time::now() - baseTime;

    // Compute which slice should be reached by now
    auto target = elapsed.asNanoseconds() * i64(refreshRate()) / 1000000000;

    // Compute the number of missing slices
    return isize(target - frameCounter);
}

double
Emulator::refreshRate() const
{
    auto config = main.getConfig();

    if (config.vsync) {

        return 60.0; // TODO: double(host.getOption(OPT_HOST_REFRESH_RATE));

    } else {

        return main.agnus.isPAL() ? 50.0 : 60.0; 
    }
}

void
Emulator::computeFrame()
{
    main.computeFrame();

    /*
    if (config.runAhead) {

        try {

            // Run the main instance
            main.execute();

            // Recreate the run-ahead instance if necessary
            if (main.isDirty || RUA_ON_STEROIDS) recreateRunAheadInstance();

            // Run the runahead instance
            ahead.execute();

        } catch (StateChangeException &) {

            main.markAsDirty();
            throw;
        }

    } else {

        // Only run the main instance
        main.execute();
    }
    */
}

void
Emulator::isReady()
{
    main.isReady();
}

}
