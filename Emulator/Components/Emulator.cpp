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
#include "Option.h"
#include "Amiga.h"
#include "Aliases.h"
#include <algorithm>

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
    assert(!isInitialized());

    // Initialize all components
    main.initialize();

    // Setup the default configuration
    host.resetConfig();
    main.resetConfig();

    // Perform a hard reset
    main.hardReset();

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

void
Emulator::put(const Cmd &cmd)
{
    cmdQueue.put(cmd);
}

/*
i64
Emulator::getOption(Option opt) const
{
    switch (opt) {

        case OPT_EMU_WARP_BOOT:         return config.warpBoot;
        case OPT_EMU_WARP_MODE:         return config.warpMode;
        case OPT_EMU_SNAPSHOTS:         return config.snapshots;
        case OPT_EMU_SNAPSHOT_DELAY:    return config.snapshotDelay;

        default:
            fatalError;
    }
}

void
Emulator::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_EMU_WARP_BOOT:

            return;

        case OPT_EMU_WARP_MODE:

            if (!WarpModeEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, WarpModeEnum::keyList());
            }
            return;

        case OPT_EMU_SNAPSHOTS:

            return;

        case OPT_EMU_SNAPSHOT_DELAY:

            if (value < 10 || value > 3600) {
                throw Error(ERROR_OPT_INV_ARG, "10...3600");
            }
            return;

        default:
            throw Error(ERROR_OPT_UNSUPPORTED);
    }
}

void
Emulator::setOption(Option opt, i64 value)
{
    checkOption(opt, value);

    switch (opt) {

        case OPT_EMU_SNAPSHOTS:

            config.snapshots = bool(value);
            main.scheduleNextSnpEvent();
            return;

        case OPT_EMU_SNAPSHOT_DELAY:

            config.snapshotDelay = isize(value);
            main.scheduleNextSnpEvent();
            return;

        default:
            fatalError;
    }
}
*/

i64
Emulator::get(Option opt, isize id) const
{
    auto targets = routeOption(opt);
    
    if (usize(id) >= targets.size()) {
        throw Error(ERROR_OPT_INV_ID, "0..." + std::to_string(targets.size() - 1));
    }
    return targets.at(id)->getOption(opt);
}

void
Emulator::check(Option opt, i64 value, const std::vector<isize> objids)
{
    // Check if this option is overridden for debugging
    value = overrideOption(opt, value);

    // Determine all option providers
    auto targets = routeOption(opt);

    // Check the components
    for(isize i = 0; i < isize(targets.size()); i++){

        if (objids.empty() || std::find(objids.begin(), objids.end(), i) != objids.end()) {

            debug(CNF_DEBUG, "set(%s, %lld, %ld)\n", OptionEnum::key(opt), value, i);
            targets.at(i)->checkOption(opt, value);
        }
    }
    /*
    for (auto &id: objids) {

        debug(CNF_DEBUG, "check(%s, %lld, %ld)\n", OptionEnum::key(opt), value, id);
        if (usize(id) >= targets.size()) {
            throw Error(ERROR_OPT_INV_ID, "0..." + std::to_string(targets.size() - 1));
        }

        targets.at(id)->checkOption(opt, value);
    }
    */
}

void
Emulator::set(Option opt, i64 value, const std::vector<isize> objids)
{
    // Check if this option is overridden for debugging
    value = overrideOption(opt, value);

    // Determine all option providers
    auto targets = routeOption(opt);

    // Configure the components
    for(isize i = 0; i < isize(targets.size()); i++){

        if (objids.empty() || std::find(objids.begin(), objids.end(), i) != objids.end()) {

            debug(CNF_DEBUG, "set(%s, %lld, %ld)\n", OptionEnum::key(opt), value, i);
            targets.at(i)->setOption(opt, value);
        }
    }
    /*
    for (auto &id: objids) {

        if (usize(id) >= targets.size()) {
            throw Error(ERROR_OPT_INV_ID, "0..." + std::to_string(targets.size() - 1));
        }

        targets.at(id)->setOption(opt, value);
    }
    */
}

void 
Emulator::set(Option opt, const string &value, const std::vector<isize> objids)
{
    set(opt, OptionParser::parse(opt, value), objids);
}

void
Emulator::set(const string &opt, const string &value, const std::vector<isize> objids)
{
    set(Option(util::parseEnum<OptionEnum>(opt)), value, objids);
}

void
Emulator::set(ConfigScheme scheme)
{
    assert_enum(ConfigScheme, scheme);

    {   SUSPENDED

        switch(scheme) {

            case CONFIG_A1000_OCS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_OCS_OLD);
                set(OPT_DENISE_REVISION, DENISE_OCS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_CHIP_RAM, 512);
                set(OPT_SLOW_RAM, 512);
                break;

            case CONFIG_A500_OCS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_OCS);
                set(OPT_DENISE_REVISION, DENISE_OCS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_CHIP_RAM, 512);
                set(OPT_SLOW_RAM, 512);
                break;

            case CONFIG_A500_ECS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
                set(OPT_DENISE_REVISION, DENISE_OCS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_CHIP_RAM, 512);
                set(OPT_SLOW_RAM, 512);
                break;

            case CONFIG_A500_PLUS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_ECS_2MB);
                set(OPT_DENISE_REVISION, DENISE_ECS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_CHIP_RAM, 512);
                set(OPT_SLOW_RAM, 512);
                break;

            default:
                fatalError;
        }
    }
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

    // Check global components
    if (host.isValidOption(opt)) result.push_back(&host);

    // Check components of the main instance
    main.routeOption(opt, result);

    assert(!result.empty());
    return result;
}

i64
Emulator::overrideOption(Option opt, i64 value) const
{
    static std::map<Option,i64> overrides = OVERRIDES;

    if (overrides.find(opt) != overrides.end()) {

        msg("Overriding option: %s = %lld\n", OptionEnum::key(opt), value);
        return overrides[opt];
    }

    return value;
}

void
Emulator::update()
{
    Cmd cmd;
    bool cmdConfig = false;
    
    shouldWarp() ? warpOn() : warpOff();

    while (cmdQueue.poll(cmd)) {

        switch (cmd.type) {

            case CMD_CONFIG:

                cmdConfig = true;
                set(cmd.config.option, cmd.config.value, { cmd.config.id });
                break;

            case CMD_CONFIG_ALL:

                cmdConfig = true;
                set(cmd.config.option, cmd.config.value, { });
                break;

            case CMD_FOCUS:

                cmd.value ? main.focus() : main.unfocus();
                break;

            default:
                fatal("Unhandled command: %s\n", CmdTypeEnum::key(cmd.type));
        }
    }

    if (cmdConfig) {
        main.msgQueue.put(MSG_CONFIG);
    }
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
