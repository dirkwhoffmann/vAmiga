// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Amiga.h"
#include "Snapshot.h"
#include "ADFFile.h"
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

Defaults Amiga::defaults;

string
Amiga::version()
{
    string result;

    result = std::to_string(VER_MAJOR) + "." + std::to_string(VER_MINOR);
    if constexpr (VER_SUBMINOR > 0) result += "." + std::to_string(VER_SUBMINOR);
    if constexpr (VER_BETA > 0) result += 'b' + std::to_string(VER_BETA);

    return result;
}

string
Amiga::build()
{
    string db = debugBuild ? " [DEBUG BUILD]" : "";

    return version() + db + " (" + __DATE__ + " " + __TIME__ + ")";
}

Amiga::Amiga()
{
    /* The order of subcomponents is important here, because some components
     * are dependend on others during initialization. I.e.,
     *
     * - The control ports, the serial Controller, the disk controller, and the
     *   disk drives must preceed the CIAs, because the CIA port values depend
     *   on these devices.
     *
     * - The CIAs must preceed memory, because they determine if the lower
     *   memory banks are overlayed by Rom.
     *
     * - Memory must preceed the CPU, because it contains the CPU reset vector.
     */

    subComponents = std::vector<CoreComponent *> {

        &agnus,
        &rtc,
        &denise,
        &paula,
        &zorro,
        &controlPort1,
        &controlPort2,
        &serialPort,
        &keyboard,
        &df0,
        &df1,
        &df2,
        &df3,
        &hd0,
        &hd1,
        &hd2,
        &hd3,
        &hd0con,
        &hd1con,
        &hd2con,
        &hd3con,
        &ramExpansion,
        &diagBoard,
        &ciaA,
        &ciaB,
        &mem,
        &cpu,
        &remoteManager,
        &retroShell,
        &regressionTester,
        &msgQueue
    };
}

Amiga::~Amiga()
{
    debug(RUN_DEBUG, "Destroying emulator instance\n");
    if (thread.joinable()) { halt(); }
}

void
Amiga::launch(const void *listener, Callback *func)
{
    msgQueue.setListener(listener, func);

    launch();
}

void
Amiga::launch()
{
    // Initialize all components
    initialize();

    // Reset the emulator
    hardReset();

    // Initialize the sync timer
    targetTime = util::Time::now();

    // Print some debug information
    if (SNP_DEBUG) {

        msg("             Amiga : %zu bytes\n", sizeof(Amiga));
        msg("             Agnus : %zu bytes\n", sizeof(Agnus));
        msg("       AudioFilter : %zu bytes\n", sizeof(AudioFilter));
        msg("               CIA : %zu bytes\n", sizeof(CIA));
        msg("       ControlPort : %zu bytes\n", sizeof(ControlPort));
        msg("               CPU : %zu bytes\n", sizeof(CPU));
        msg("            Denise : %zu bytes\n", sizeof(Denise));
        msg("             Drive : %zu bytes\n", sizeof(FloppyDrive));
        msg("          Keyboard : %zu bytes\n", sizeof(Keyboard));
        msg("            Memory : %zu bytes\n", sizeof(Memory));
        msg("moira::Breakpoints : %zu bytes\n", sizeof(moira::Breakpoints));
        msg("moira::Watchpoints : %zu bytes\n", sizeof(moira::Watchpoints));
        msg("   moira::Debugger : %zu bytes\n", sizeof(moira::Debugger));
        msg("      moira::Moira : %zu bytes\n", sizeof(moira::Moira));
        msg("             Muxer : %zu bytes\n", sizeof(Muxer));
        msg("             Paula : %zu bytes\n", sizeof(Paula));
        msg("       PixelEngine : %zu bytes\n", sizeof(PixelEngine));
        msg("     RemoteManager : %zu bytes\n", sizeof(RemoteManager));
        msg("               RTC : %zu bytes\n", sizeof(RTC));
        msg("        RetroShell : %zu bytes\n", sizeof(RetroShell));
        msg("           Sampler : %zu bytes\n", sizeof(Sampler));
        msg("        SerialPort : %zu bytes\n", sizeof(SerialPort));
        msg("            Volume : %zu bytes\n", sizeof(Volume));
        msg("             Zorro : %zu bytes\n", sizeof(ZorroManager));
        msg("\n");
    }

    if (!thread.joinable()) {

        thread = std::thread(&Thread::main, this);
        assert(thread.joinable());
    }
}

void
Amiga::prefix() const
{
    fprintf(stderr, "[%lld] (%3ld,%3ld) ",
            agnus.pos.frame, agnus.pos.v, agnus.pos.h);

    fprintf(stderr, "%06X ", cpu.getPC0());
    fprintf(stderr, "%2X ", cpu.getIPL());

    u16 dmacon = agnus.dmacon;
    bool dmaen = dmacon & DMAEN;
    fprintf(stderr, "%c%c%c%c%c%c ",
            (dmacon & BPLEN) ? (dmaen ? 'B' : 'B') : '-',
            (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
            (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
            (dmacon & SPREN) ? (dmaen ? 'S' : 's') : '-',
            (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-',
            (dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : '-');

    fprintf(stderr, "%04X %04X ", paula.intena, paula.intreq);

    if (agnus.copper.servicing) {
        fprintf(stderr, "[%06X] ", agnus.copper.getCopPC0());
    }
}

void
Amiga::reset(bool hard)
{
    if (!isEmulatorThread()) suspend();

    // If a disk change is in progress, finish it
    df0.serviceDiskChangeEvent <SLOT_DC0> ();
    df1.serviceDiskChangeEvent <SLOT_DC1> ();
    df2.serviceDiskChangeEvent <SLOT_DC2> ();
    df3.serviceDiskChangeEvent <SLOT_DC3> ();

    // Execute the standard reset routine
    CoreComponent::reset(hard);

    if (!isEmulatorThread()) resume();

    // Inform the GUI
    if (hard) msgQueue.put(MSG_RESET);
}

void
Amiga::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    // Clear all runloop flags
    flags = 0;

    updateWarpState();
}

void
Amiga::resetConfig()
{
    assert(isPoweredOff());

    std::vector <Option> options = {

        OPT_VIDEO_FORMAT,
        OPT_WARP_BOOT,
        OPT_WARP_MODE,
        OPT_SYNC_MODE,
        OPT_PROPOSED_FPS
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
Amiga::getConfigItem(Option option) const
{
    switch (option) {

        case OPT_VIDEO_FORMAT:

            return config.type;

        case OPT_WARP_BOOT:

            return config.warpBoot;

        case OPT_WARP_MODE:

            return config.warpMode;

        case OPT_SYNC_MODE:

            return config.syncMode;

        case OPT_PROPOSED_FPS:

            return config.proposedFps;

        case OPT_AGNUS_REVISION:
        case OPT_SLOW_RAM_MIRROR:
        case OPT_PTR_DROPS:

            return agnus.getConfigItem(option);

        case OPT_DENISE_REVISION:
        case OPT_VIEWPORT_TRACKING:
        case OPT_HIDDEN_BITPLANES:
        case OPT_HIDDEN_SPRITES:
        case OPT_HIDDEN_LAYERS:
        case OPT_HIDDEN_LAYER_ALPHA:
        case OPT_CLX_SPR_SPR:
        case OPT_CLX_SPR_PLF:
        case OPT_CLX_PLF_PLF:

            return denise.getConfigItem(option);

        case OPT_PALETTE:
        case OPT_BRIGHTNESS:
        case OPT_CONTRAST:
        case OPT_SATURATION:

            return denise.pixelEngine.getConfigItem(option);

        case OPT_DMA_DEBUG_ENABLE:
        case OPT_DMA_DEBUG_MODE:
        case OPT_DMA_DEBUG_OPACITY:

            return agnus.dmaDebugger.getConfigItem(option);

        case OPT_CPU_REVISION:
        case OPT_CPU_DASM_REVISION:
        case OPT_CPU_DASM_SYNTAX:
        case OPT_CPU_OVERCLOCKING:
        case OPT_CPU_RESET_VAL:

            return cpu.getConfigItem(option);

        case OPT_RTC_MODEL:

            return rtc.getConfigItem(option);

        case OPT_CHIP_RAM:
        case OPT_SLOW_RAM:
        case OPT_FAST_RAM:
        case OPT_EXT_START:
        case OPT_SAVE_ROMS:
        case OPT_SLOW_RAM_DELAY:
        case OPT_BANKMAP:
        case OPT_UNMAPPING_TYPE:
        case OPT_RAM_INIT_PATTERN:

            return mem.getConfigItem(option);

        case OPT_SAMPLING_METHOD:
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
        case OPT_FILTER_TYPE:

            return paula.muxer.getConfigItem(option);

        case OPT_BLITTER_ACCURACY:

            return agnus.blitter.getConfigItem(option);

        case OPT_DRIVE_SPEED:
        case OPT_LOCK_DSKSYNC:
        case OPT_AUTO_DSKSYNC:

            return paula.diskController.getConfigItem(option);

        case OPT_SER_DEVICE:
        case OPT_SER_VERBOSE:

            return serialPort.getConfigItem(option);

        case OPT_CIA_REVISION:
        case OPT_TODBUG:
        case OPT_ECLOCK_SYNCING:

            return ciaA.getConfigItem(option);

        case OPT_ACCURATE_KEYBOARD:

            return keyboard.getConfigItem(option);

        case OPT_DIAG_BOARD:

            return diagBoard.getConfigItem(option);

        default:
            fatalError;
    }
}

i64
Amiga::getConfigItem(Option option, long id) const
{
    switch (option) {

        case OPT_DMA_DEBUG_CHANNEL:
        case OPT_DMA_DEBUG_COLOR:

            return agnus.dmaDebugger.getConfigItem(option, id);

        case OPT_AUDPAN:
        case OPT_AUDVOL:

            return paula.muxer.getConfigItem(option, id);

        case OPT_DRIVE_CONNECT:

            return paula.diskController.getConfigItem(option, id);

        case OPT_DRIVE_TYPE:
        case OPT_DRIVE_MECHANICS:
        case OPT_DRIVE_RPM:
        case OPT_DISK_SWAP_DELAY:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:

            return df[id]->getConfigItem(option);

        case OPT_HDC_CONNECT:

            return hdcon[id]->getConfigItem(option);

        case OPT_HDR_TYPE:
        case OPT_HDR_PAN:
        case OPT_HDR_STEP_VOLUME:

            return hd[id]->getConfigItem(option);

        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:

            if (id == ControlPort::PORT1) return controlPort1.mouse.getConfigItem(option);
            if (id == ControlPort::PORT2) return controlPort2.mouse.getConfigItem(option);
            fatalError;

        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:

            if (id == ControlPort::PORT1) return controlPort1.joystick.getConfigItem(option);
            if (id == ControlPort::PORT2) return controlPort2.joystick.getConfigItem(option);
            fatalError;

        case OPT_SRV_PORT:
        case OPT_SRV_PROTOCOL:
        case OPT_SRV_AUTORUN:
        case OPT_SRV_VERBOSE:

            return remoteManager.getConfigItem(option, id);

        default:
            fatalError;
    }
}

void
Amiga::setConfigItem(Option option, i64 value)
{
    switch (option) {

        case OPT_VIDEO_FORMAT:

            if (!VideoFormatEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, VideoFormatEnum::keyList());
            }

            if (value != config.type) {

                SUSPENDED

                config.type = VideoFormat(value);
                agnus.setVideoFormat(config.type);
            }
            return;

        case OPT_WARP_BOOT:

            config.warpBoot = isize(value);
            updateWarpState();
            return;

        case OPT_WARP_MODE:

            if (!WarpModeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, WarpModeEnum::keyList());
            }

            config.warpMode = WarpMode(value);
            updateWarpState();
            return;

        case OPT_SYNC_MODE:

            if (!SyncModeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, SyncModeEnum::keyList());
            }

            config.syncMode = SyncMode(value);
            return;

        case OPT_PROPOSED_FPS:

            if (value < 25 || value > 120) {
                throw VAError(ERROR_OPT_INVARG, "25...120");
            }

            config.proposedFps = isize(value);
            return;

        default:
            fatalError;
    }
}

void
Amiga::configure(Option option, i64 value)
{
    debug(CNF_DEBUG, "configure(%s, %lld)\n", OptionEnum::key(option), value);

    // The following options do not send a message to the GUI
    static std::vector<Option> quiet = {

        OPT_HIDDEN_LAYER_ALPHA,
        OPT_BRIGHTNESS,
        OPT_CONTRAST,
        OPT_SATURATION,
        OPT_DRIVE_PAN,
        OPT_STEP_VOLUME,
        OPT_POLL_VOLUME,
        OPT_INSERT_VOLUME,
        OPT_EJECT_VOLUME,
        OPT_HDR_PAN,
        OPT_HDR_STEP_VOLUME,
        OPT_AUDVOLL,
        OPT_AUDVOLR,
        OPT_AUDPAN,
        OPT_AUDVOL
    };

    // Check if this option has been locked for debugging
    value = overrideOption(option, value);

    switch (option) {

        case OPT_VIDEO_FORMAT:
        case OPT_WARP_BOOT:
        case OPT_WARP_MODE:
        case OPT_SYNC_MODE:
        case OPT_PROPOSED_FPS:

            setConfigItem(option, value);
            break;

        case OPT_AGNUS_REVISION:
        case OPT_SLOW_RAM_MIRROR:
        case OPT_PTR_DROPS:

            agnus.setConfigItem(option, value);
            break;

        case OPT_DENISE_REVISION:
        case OPT_VIEWPORT_TRACKING:
        case OPT_HIDDEN_BITPLANES:
        case OPT_HIDDEN_SPRITES:
        case OPT_HIDDEN_LAYERS:
        case OPT_HIDDEN_LAYER_ALPHA:
        case OPT_CLX_SPR_SPR:
        case OPT_CLX_SPR_PLF:
        case OPT_CLX_PLF_PLF:

            denise.setConfigItem(option, value);
            break;

        case OPT_PALETTE:
        case OPT_BRIGHTNESS:
        case OPT_CONTRAST:
        case OPT_SATURATION:

            denise.pixelEngine.setConfigItem(option, value);
            break;

        case OPT_DMA_DEBUG_ENABLE:
        case OPT_DMA_DEBUG_MODE:
        case OPT_DMA_DEBUG_OPACITY:

            agnus.dmaDebugger.setConfigItem(option, value);
            break;

        case OPT_CPU_REVISION:
        case OPT_CPU_DASM_REVISION:
        case OPT_CPU_OVERCLOCKING:
        case OPT_CPU_RESET_VAL:
        case OPT_CPU_DASM_SYNTAX:

            cpu.setConfigItem(option, value);
            break;

        case OPT_RTC_MODEL:

            rtc.setConfigItem(option, value);
            break;

        case OPT_CHIP_RAM:
        case OPT_SLOW_RAM:
        case OPT_FAST_RAM:
        case OPT_EXT_START:
        case OPT_SAVE_ROMS:
        case OPT_SLOW_RAM_DELAY:
        case OPT_BANKMAP:
        case OPT_UNMAPPING_TYPE:
        case OPT_RAM_INIT_PATTERN:

            mem.setConfigItem(option, value);
            break;

        case OPT_DRIVE_TYPE:
        case OPT_DRIVE_MECHANICS:
        case OPT_DRIVE_RPM:
        case OPT_DISK_SWAP_DELAY:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:

            df[0]->setConfigItem(option, value);
            df[1]->setConfigItem(option, value);
            df[2]->setConfigItem(option, value);
            df[3]->setConfigItem(option, value);
            break;

        case OPT_HDC_CONNECT:

            hdcon[0]->setConfigItem(option, value);
            hdcon[1]->setConfigItem(option, value);
            hdcon[2]->setConfigItem(option, value);
            hdcon[3]->setConfigItem(option, value);
            break;

        case OPT_HDR_TYPE:
        case OPT_HDR_PAN:
        case OPT_HDR_STEP_VOLUME:

            hd[0]->setConfigItem(option, value);
            hd[1]->setConfigItem(option, value);
            hd[2]->setConfigItem(option, value);
            hd[3]->setConfigItem(option, value);
            break;

        case OPT_SAMPLING_METHOD:
        case OPT_FILTER_TYPE:
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:

            paula.muxer.setConfigItem(option, value);
            break;

        case OPT_AUDPAN:
        case OPT_AUDVOL:

            paula.muxer.setConfigItem(option, 0, value);
            paula.muxer.setConfigItem(option, 1, value);
            paula.muxer.setConfigItem(option, 2, value);
            paula.muxer.setConfigItem(option, 3, value);
            break;

        case OPT_BLITTER_ACCURACY:

            agnus.blitter.setConfigItem(option, value);
            break;

        case OPT_DRIVE_SPEED:
        case OPT_LOCK_DSKSYNC:
        case OPT_AUTO_DSKSYNC:

            paula.diskController.setConfigItem(option, value);
            break;

        case OPT_SER_DEVICE:
        case OPT_SER_VERBOSE:

            serialPort.setConfigItem(option, value);
            break;

        case OPT_CIA_REVISION:
        case OPT_TODBUG:
        case OPT_ECLOCK_SYNCING:

            ciaA.setConfigItem(option, value);
            ciaB.setConfigItem(option, value);
            break;

        case OPT_ACCURATE_KEYBOARD:

            keyboard.setConfigItem(option, value);
            break;

        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:

            controlPort1.mouse.setConfigItem(option, value);
            controlPort2.mouse.setConfigItem(option, value);
            break;

        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:

            controlPort1.joystick.setConfigItem(option, value);
            controlPort2.joystick.setConfigItem(option, value);
            break;

        case OPT_DIAG_BOARD:

            diagBoard.setConfigItem(OPT_DIAG_BOARD, value);
            break;

        case OPT_SRV_PORT:
        case OPT_SRV_PROTOCOL:
        case OPT_SRV_AUTORUN:
        case OPT_SRV_VERBOSE:

            remoteManager.setConfigItem(option, value);
            break;

        default:
            fatalError;
    }

    if (std::find(quiet.begin(), quiet.end(), option) == quiet.end()) {
        msgQueue.put(MSG_CONFIG, option);
    }
}

void
Amiga::configure(Option option, long id, i64 value)
{
    debug(CNF_DEBUG, "configure(%s, %ld, %lld)\n", OptionEnum::key(option), id, value);

    // Check if this option has been locked for debugging
    value = overrideOption(option, value);

    // The following options do not send a message to the GUI
    static std::vector<Option> quiet = {

        OPT_DRIVE_PAN,
        OPT_STEP_VOLUME,
        OPT_POLL_VOLUME,
        OPT_INSERT_VOLUME,
        OPT_EJECT_VOLUME,
        OPT_HDR_PAN,
        OPT_HDR_STEP_VOLUME,
        OPT_AUDVOLL,
        OPT_AUDVOLR,
        OPT_AUDPAN,
        OPT_AUDVOL,
        OPT_MOUSE_VELOCITY
    };

    switch (option) {

        case OPT_DMA_DEBUG_CHANNEL:
        case OPT_DMA_DEBUG_COLOR:

            agnus.dmaDebugger.setConfigItem(option, id, value);
            break;

        case OPT_AUDPAN:
        case OPT_AUDVOL:

            paula.muxer.setConfigItem(option, id, value);
            break;

        case OPT_DRIVE_CONNECT:

            paula.diskController.setConfigItem(option, id, value);
            break;

        case OPT_DRIVE_TYPE:
        case OPT_DRIVE_MECHANICS:
        case OPT_DRIVE_RPM:
        case OPT_DISK_SWAP_DELAY:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:

            assert(id >= 0 || id <= 4);
            df[id]->setConfigItem(option, value);
            break;

        case OPT_HDC_CONNECT:

            assert(id >= 0 || id <= 4);
            hdcon[id]->setConfigItem(option, value);
            break;

        case OPT_HDR_TYPE:
        case OPT_HDR_PAN:
        case OPT_HDR_STEP_VOLUME:

            assert(id >= 0 || id <= 4);
            hd[id]->setConfigItem(option, value);
            break;

        case OPT_CIA_REVISION:
        case OPT_TODBUG:
        case OPT_ECLOCK_SYNCING:

            assert(id == 0 || id == 1);
            if (id == 0) ciaA.setConfigItem(option, value);
            if (id == 1) ciaB.setConfigItem(option, value);
            break;

        case OPT_PULLUP_RESISTORS:
        case OPT_SHAKE_DETECTION:
        case OPT_MOUSE_VELOCITY:

            assert(id == ControlPort::PORT1 || id == ControlPort::PORT2);
            if (id == ControlPort::PORT1) controlPort1.mouse.setConfigItem(option, value);
            if (id == ControlPort::PORT2) controlPort2.mouse.setConfigItem(option, value);
            break;

        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:

            if (id == ControlPort::PORT1) controlPort1.joystick.setConfigItem(option, value);
            if (id == ControlPort::PORT2) controlPort2.joystick.setConfigItem(option, value);
            break;

        case OPT_SRV_PORT:
        case OPT_SRV_PROTOCOL:
        case OPT_SRV_AUTORUN:
        case OPT_SRV_VERBOSE:

            remoteManager.setConfigItem(option, id, value);
            break;

        default:
            fatalError;
    }

    if (std::find(quiet.begin(), quiet.end(), option) == quiet.end()) {
        msgQueue.put(MSG_CONFIG, option);
    }
}

void
Amiga::configure(ConfigScheme scheme)
{
    assert_enum(ConfigScheme, scheme);

    {   SUSPENDED

        switch(scheme) {

            case CONFIG_A1000_OCS_1MB:

                configure(OPT_CPU_REVISION, CPU_68000);
                configure(OPT_AGNUS_REVISION, AGNUS_OCS_OLD);
                configure(OPT_DENISE_REVISION, DENISE_OCS);
                configure(OPT_VIDEO_FORMAT, PAL);
                configure(OPT_CHIP_RAM, 512);
                configure(OPT_SLOW_RAM, 512);
                break;

            case CONFIG_A500_OCS_1MB:

                configure(OPT_CPU_REVISION, CPU_68000);
                configure(OPT_AGNUS_REVISION, AGNUS_OCS);
                configure(OPT_DENISE_REVISION, DENISE_OCS);
                configure(OPT_VIDEO_FORMAT, PAL);
                configure(OPT_CHIP_RAM, 512);
                configure(OPT_SLOW_RAM, 512);
                break;

            case CONFIG_A500_ECS_1MB:

                configure(OPT_CPU_REVISION, CPU_68000);
                configure(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
                configure(OPT_DENISE_REVISION, DENISE_OCS);
                configure(OPT_VIDEO_FORMAT, PAL);
                configure(OPT_CHIP_RAM, 512);
                configure(OPT_SLOW_RAM, 512);
                break;

            case CONFIG_A500_PLUS_1MB:

                configure(OPT_CPU_REVISION, CPU_68000);
                configure(OPT_AGNUS_REVISION, AGNUS_ECS_2MB);
                configure(OPT_DENISE_REVISION, DENISE_ECS);
                configure(OPT_VIDEO_FORMAT, PAL);
                configure(OPT_CHIP_RAM, 512);
                configure(OPT_SLOW_RAM, 512);
                break;

            default:
                fatalError;
        }
    }
}

void
Amiga::revertToFactorySettings()
{
    // Switch the emulator off
    powerOff();

    // Revert to the initial state
    initialize();
}

i64
Amiga::overrideOption(Option option, i64 value)
{
    static std::map<Option,i64> overrides = OVERRIDES;

    if (overrides.find(option) != overrides.end()) {

        msg("Overriding option: %s = %lld\n", OptionEnum::key(option), value);
        return overrides[option];
    }

    return value;
}

InspectionTarget
Amiga::getInspectionTarget() const
{
    switch(agnus.id[SLOT_INS]) {

        case EVENT_NONE:  return INSPECTION_NONE;
        case INS_AMIGA:   return INSPECTION_AMIGA;
        case INS_CPU:     return INSPECTION_CPU;
        case INS_MEM:     return INSPECTION_MEM;
        case INS_CIA:     return INSPECTION_CIA;
        case INS_AGNUS:   return INSPECTION_AGNUS;
        case INS_PAULA:   return INSPECTION_PAULA;
        case INS_DENISE:  return INSPECTION_DENISE;
        case INS_PORTS:   return INSPECTION_PORTS;
        case INS_EVENTS:  return INSPECTION_EVENTS;

        default:
            fatalError;
    }
}

void
Amiga::setInspectionTarget(InspectionTarget target, Cycle trigger)
{
    EventID id;

    {   SUSPENDED

        switch(target) {

            case INSPECTION_NONE:    agnus.cancel<SLOT_INS>(); return;

            case INSPECTION_AMIGA:   id = INS_AMIGA; break;
            case INSPECTION_CPU:     id = INS_CPU; break;
            case INSPECTION_MEM:     id = INS_MEM; break;
            case INSPECTION_CIA:     id = INS_CIA; break;
            case INSPECTION_AGNUS:   id = INS_AGNUS; break;
            case INSPECTION_PAULA:   id = INS_PAULA; break;
            case INSPECTION_DENISE:  id = INS_DENISE; break;
            case INSPECTION_PORTS:   id = INS_PORTS; break;
            case INSPECTION_EVENTS:  id = INS_EVENTS; break;

            default:
                fatalError;
        }

        agnus.scheduleRel<SLOT_INS>(trigger, id);
        if (trigger == 0) agnus.serviceINSEvent(id);
    }
}

void
Amiga::_inspect() const
{
    {   SYNCHRONIZED

        info.cpuClock = cpu.getMasterClock();
        info.dmaClock = agnus.clock;
        info.ciaAClock = ciaA.getClock();
        info.ciaBClock = ciaB.getClock();
        info.frame = agnus.pos.frame;
        info.vpos = agnus.pos.v;
        info.hpos = agnus.pos.h;
    }
}

void
Amiga::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {

        os << tab("Machine type");
        os << VideoFormatEnum::key(config.type) << std::endl;
        os << tab("Warp mode");
        os << WarpModeEnum::key(config.warpMode) << std::endl;
        os << tab("Warp boot");
        os << dec(config.warpBoot) << " seconds" << std::endl;
        os << tab("Sync mode");
        os << SyncModeEnum::key(config.syncMode);
        if (config.syncMode == SYNC_FIXED_FPS) os << " (" << config.proposedFps << " fps)";
        os << std::endl;
    }

    if (category == Category::State) {

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

        os << tab("Refresh rate");
        os << dec(isize(refreshRate())) << " Fps" << std::endl;
        os << tab("Master clock frequency");
        os << flt(masterClockFrequency() / float(1000000.0)) << " MHz" << std::endl;
        os << tab("Thread state");
        os << ExecutionStateEnum::key(state) << std::endl;
        os << tab("Thread mode");
        os << ThreadModeEnum::key(getThreadMode()) << std::endl;
        os << std::endl;

        os << tab("Frame");
        os << dec(agnus.pos.frame) << std::endl;
        os << tab("CPU progress");
        os << dec(cpu.getMasterClock()) << " Master cycles (";
        os << dec(cpu.getCpuClock()) << " CPU cycles)" << std::endl;
        os << tab("Agnus progress");
        os << dec(agnus.clock) << " Master cycles (";
        os << dec(AS_DMA_CYCLES(agnus.clock)) << " DMA cycles)" << std::endl;
        os << tab("CIA A progress");
        os << dec(ciaA.getClock()) << " Master cycles (";
        os << dec(AS_CIA_CYCLES(ciaA.getClock())) << " CIA cycles)" << std::endl;
        os << tab("CIA B progress");
        os << dec(ciaB.getClock()) << " Master cycles (";
        os << dec(AS_CIA_CYCLES(ciaA.getClock())) << " CIA cycles)" << std::endl;
        os << std::endl;
    }

    if (category == Category::Defaults) {

        defaults.dump(category, os);
    }

    if (category == Category::Current) {

        auto dmacon = agnus.dmacon;
        bool dmaen = dmacon & DMAEN;
        auto intreq = paula.intreq;
        auto intena = (paula.intena & 0x8000) ? paula.intena : 0;
        auto fc = cpu.readFC();
        char empty = '.';

        char sr[32];
        (void)cpu.disassembleSR(sr);

        os << std::setfill('0');
        os << "   DMACON  INTREQ / INTENA  STATUS REGISTER  IPL FCP" << std::endl;

        os << "   ";
        os << ((dmacon & BPLEN) ? (dmaen ? 'B' : 'b') : empty);
        os << ((dmacon & COPEN) ? (dmaen ? 'C' : 'c') : empty);
        os << ((dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : empty);
        os << ((dmacon & SPREN) ? (dmaen ? 'S' : 's') : empty);
        os << ((dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : empty);
        os << ((dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : empty);

        os << "  ";
        os << ((intena & 0x4000) ? '1' : '0');
        os << ((intreq & 0x2000) ? ((intena & 0x2000) ? 'E' : 'e') : empty);
        os << ((intreq & 0x1000) ? ((intena & 0x1000) ? 'D' : 'd') : empty);
        os << ((intreq & 0x0800) ? ((intena & 0x0800) ? 'R' : 'r') : empty);
        os << ((intreq & 0x0400) ? ((intena & 0x0400) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0200) ? ((intena & 0x0200) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0100) ? ((intena & 0x0100) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0080) ? ((intena & 0x0080) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0040) ? ((intena & 0x0040) ? 'D' : 'd') : empty);
        os << ((intreq & 0x0020) ? ((intena & 0x0020) ? 'V' : 'v') : empty);
        os << ((intreq & 0x0010) ? ((intena & 0x0010) ? 'C' : 'c') : empty);
        os << ((intreq & 0x0008) ? ((intena & 0x0008) ? 'P' : 'p') : empty);
        os << ((intreq & 0x0004) ? ((intena & 0x0004) ? 'S' : 's') : empty);
        os << ((intreq & 0x0002) ? ((intena & 0x0002) ? 'D' : 'd') : empty);
        os << ((intreq & 0x0001) ? ((intena & 0x0001) ? 'T' : 't') : empty);

        os << "  ";
        os << sr;

        os << " [";
        os << std::right << std::setw(1) << std::dec << isize(cpu.getIPL()) << "]";

        os << " ";
        os << ((fc & 0b100) ? '1' : '0');
        os << ((fc & 0b010) ? '1' : '0');
        os << ((fc & 0b001) ? '1' : '0');
        os << std::endl;
    }
}

void
Amiga::_powerOn()
{
    debug(RUN_DEBUG, "_powerOn\n");

    // Perform a reset
    hardReset();

    // Start from a snapshot if requested
    if (string(INITIAL_SNAPSHOT) != "") {

        Snapshot snapshot(INITIAL_SNAPSHOT);
        loadSnapshot(snapshot);
    }

    // Set initial breakpoints
    for (auto &bp : std::vector <u32> (INITIAL_BREAKPOINTS)) {

        cpu.debugger.breakpoints.setAt(bp);
        track = true;
    }

    // Update the recorded debug information
    inspect();

    msgQueue.put(MSG_POWER, 1);
}

void
Amiga::_powerOff()
{
    debug(RUN_DEBUG, "_powerOff\n");

    // Perform a reset
    hardReset();

    // Update the recorded debug information
    inspect();

    msgQueue.put(MSG_POWER, 0);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "_run\n");

    // Enable or disable CPU debugging
    track ? cpu.debugger.enableLogging() : cpu.debugger.disableLogging();

    msgQueue.put(MSG_RUN);
}

void
Amiga::_pause()
{
    debug(RUN_DEBUG, "_pause\n");

    remoteManager.gdbServer.breakpointReached();

    // Update the recorded debug information
    inspect();

    msgQueue.put(MSG_PAUSE);
}

void
Amiga::_halt()
{
    debug(RUN_DEBUG, "_halt\n");

    msgQueue.put(MSG_SHUTDOWN);
}

void
Amiga::_warpOn()
{
    debug(RUN_DEBUG, "_warpOn\n");

    msgQueue.put(MSG_WARP, 1);
}

void
Amiga::_warpOff()
{
    debug(RUN_DEBUG, "_warpOff\n");

    msgQueue.put(MSG_WARP, 0);
}

void
Amiga::_trackOn()
{
    debug(RUN_DEBUG, "_trackOn\n");

    msgQueue.put(MSG_TRACK, 1);
}

void
Amiga::_trackOff()
{
    debug(RUN_DEBUG, "_trackOff\n");

    msgQueue.put(MSG_TRACK, 0);
}

isize
Amiga::load(const u8 *buffer)
{
    auto result = CoreComponent::load(buffer);
    CoreComponent::didLoad();

    return result;
}

isize
Amiga::save(u8 *buffer)
{
    auto result = CoreComponent::save(buffer);
    CoreComponent::didSave();

    return result;
}

ThreadMode
Amiga::getThreadMode() const
{
    return config.syncMode == SYNC_VSYNC ? THREAD_PULSED : THREAD_ADAPTIVE;
}

void
Amiga::execute()
{
    while (1) {

        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {

            // Are we requested to take a snapshot?
            if (flags & RL::AUTO_SNAPSHOT) {
                clearFlag(RL::AUTO_SNAPSHOT);
                takeAutoSnapshot();
            }

            if (flags & RL::USER_SNAPSHOT) {
                clearFlag(RL::USER_SNAPSHOT);
                takeUserSnapshot();
            }

            // Did we reach a soft breakpoint?
            if (flags & RL::SOFTSTOP_REACHED) {
                clearFlag(RL::SOFTSTOP_REACHED);
                inspect();
                switchState(EXEC_PAUSED);
                break;
            }

            // Did we reach a breakpoint?
            if (flags & RL::BREAKPOINT_REACHED) {
                clearFlag(RL::BREAKPOINT_REACHED);
                inspect();
                auto addr = cpu.debugger.breakpoints.hit->addr;
                msgQueue.put(MSG_BREAKPOINT_REACHED, CpuMsg { addr, 0});
                switchState(EXEC_PAUSED);
                break;
            }

            // Did we reach a watchpoint?
            if (flags & RL::WATCHPOINT_REACHED) {
                clearFlag(RL::WATCHPOINT_REACHED);
                inspect();
                auto addr = cpu.debugger.watchpoints.hit->addr;
                msgQueue.put(MSG_WATCHPOINT_REACHED, CpuMsg {addr, 0});
                switchState(EXEC_PAUSED);
                break;
            }

            // Did we reach a catchpoint?
            if (flags & RL::CATCHPOINT_REACHED) {
                clearFlag(RL::CATCHPOINT_REACHED);
                inspect();
                auto vector = u8(cpu.debugger.catchpoints.hit->addr);
                msgQueue.put(MSG_CATCHPOINT_REACHED, CpuMsg {cpu.getPC0(), vector});
                switchState(EXEC_PAUSED);
                break;
            }

            // Did we reach a software trap?
            if (flags & RL::SWTRAP_REACHED) {
                clearFlag(RL::SWTRAP_REACHED);
                inspect();
                msgQueue.put(MSG_SWTRAP_REACHED, CpuMsg {cpu.getPC0(), 0});
                switchState(EXEC_PAUSED);
                break;
            }

            // Did we reach a Copper breakpoint?
            if (flags & RL::COPPERBP_REACHED) {
                clearFlag(RL::COPPERBP_REACHED);
                inspect();
                auto addr = u8(agnus.copper.debugger.breakpoints.hit->addr);
                msgQueue.put(MSG_COPPERBP_REACHED, CpuMsg { addr, 0 });
                switchState(EXEC_PAUSED);
                break;
            }

            // Did we reach a Copper watchpoint?
            if (flags & RL::COPPERWP_REACHED) {
                clearFlag(RL::COPPERWP_REACHED);
                inspect();
                auto addr = u8(agnus.copper.debugger.watchpoints.hit->addr);
                msgQueue.put(MSG_COPPERWP_REACHED, CpuMsg { addr, 0 });
                switchState(EXEC_PAUSED);
                break;
            }

            // Are we requested to terminate the run loop?
            if (flags & RL::STOP) {
                clearFlag(RL::STOP);
                switchState(EXEC_PAUSED);
                break;
            }

            // Are we requested to synchronize the thread?
            if (flags & RL::SYNC_THREAD) {
                clearFlag(RL::SYNC_THREAD);
                break;
            }
        }
    }
}

double
Amiga::refreshRate() const
{
    switch (config.syncMode) {

        case SYNC_NATIVE_FPS:   return config.type == PAL ? 50.0 : 60.0;
        case SYNC_FIXED_FPS:    return config.proposedFps;
        case SYNC_VSYNC:        return host.getHostRefreshRate();

        default:
            fatalError;
    }
}

isize
Amiga::missingFrames(util::Time base) const
{
    // Compute the elapsed time
    auto elapsed = util::Time::now() - base;

    // Compute which frame should have been reached by now
    auto targetFrame = elapsed.asNanoseconds() * i64(refreshRate()) / 1000000000;

    // Compute the number of missing frames
    return isize(targetFrame - agnus.pos.frame);
}

i64
Amiga::masterClockFrequency() const
{
    switch (config.type) {

        case PAL:   return i64(CLK_FREQUENCY_PAL * (refreshRate() / 50.0));
        case NTSC:  return i64(CLK_FREQUENCY_NTSC * (refreshRate() / 60.0));

        default:
            fatalError;
    }
}

void
Amiga::setFlag(u32 flag)
{
    SYNCHRONIZED

    flags |= flag;
}

void
Amiga::clearFlag(u32 flag)
{
    SYNCHRONIZED

    flags &= ~flag;
}

void
Amiga::stopAndGo()
{
    isRunning() ? pause() : run();
}

void
Amiga::stepInto()
{
    if (isRunning()) return;

    cpu.debugger.stepInto();
    run();

    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Amiga::stepOver()
{
    if (isRunning()) return;

    cpu.debugger.stepOver();
    run();

    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Amiga::updateWarpState()
{
    if (agnus.clock < SEC(config.warpBoot)) {

        switchWarp(true);
        return;
    }

    switch (config.warpMode) {

        case WARP_AUTO:     switchWarp(paula.diskController.spinning()); break;
        case WARP_NEVER:    switchWarp(false); break;
        case WARP_ALWAYS:   switchWarp(true); break;

        default:
            fatalError;
    }
}

void
Amiga::serviceWbtEvent()
{
    assert(agnus.id[SLOT_WBT] == WBT_DISABLE);

    updateWarpState();
    agnus.cancel <SLOT_WBT> ();
}

void
Amiga::requestAutoSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        takeAutoSnapshot();

    } else {

        // Schedule the snapshot to be taken
        signalAutoSnapshot();
    }
}

void
Amiga::requestUserSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        takeUserSnapshot();

    } else {

        // Schedule the snapshot to be taken
        signalUserSnapshot();
    }
}

Snapshot *
Amiga::latestAutoSnapshot()
{
    Snapshot *result = autoSnapshot;
    autoSnapshot = nullptr;
    return result;
}

Snapshot *
Amiga::latestUserSnapshot()
{
    Snapshot *result = userSnapshot;
    userSnapshot = nullptr;
    return result;
}

void
Amiga::loadSnapshot(const Snapshot &snapshot)
{
    // bool wasPAL, isPAL;

    {   SUSPENDED

        // wasPAL = agnus.isPAL();

        try {

            // Restore the saved state
            load(snapshot.getData());

        } catch (VAError &error) {

            /* If we reach this point, the emulator has been put into an
             * inconsistent state due to corrupted snapshot data. We cannot
             * continue emulation, because it would likely crash the
             * application. Because we cannot revert to the old state either,
             * we perform a hard reset to eliminate the inconsistency.
             */
            hardReset();
            throw error;
        }

        // isPAL = agnus.isPAL();
    }

    // Inform the GUI
    msgQueue.put(MSG_SNAPSHOT_RESTORED);
    msgQueue.put(MSG_VIDEO_FORMAT, agnus.isPAL() ? PAL : NTSC);
}

void
Amiga::takeAutoSnapshot()
{
    if (autoSnapshot) {

        warn("Old auto-snapshot still present. Ignoring request.\n");
        return;
    }

    autoSnapshot = new Snapshot(*this);
    msgQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
}

void
Amiga::takeUserSnapshot()
{
    if (userSnapshot) {

        warn("Old user-snapshot still present. Ignoring request.\n");
        return;
    }

    userSnapshot = new Snapshot(*this);
    msgQueue.put(MSG_USER_SNAPSHOT_TAKEN);
}

void
Amiga::setAlarmAbs(Cycle trigger, i64 payload)
{
    {   SUSPENDED

        alarms.push_back(Alarm { trigger, payload });
        scheduleNextAlarm();
    }
}

void
Amiga::setAlarmRel(Cycle trigger, i64 payload)
{
    {   SUSPENDED

        alarms.push_back(Alarm { agnus.clock + trigger, payload });
        scheduleNextAlarm();
    }
}

void
Amiga::serviceAlarmEvent()
{
    for (auto it = alarms.begin(); it != alarms.end(); ) {

        if (it->trigger <= agnus.clock) {
            msgQueue.put(MSG_ALARM, it->payload);
            it = alarms.erase(it);
        } else {
            it++;
        }
    }
    scheduleNextAlarm();
}

void
Amiga::scheduleNextAlarm()
{
    Cycle trigger = INT64_MAX;

    agnus.cancel<SLOT_ALA>();

    for(Alarm alarm : alarms) {

        if (alarm.trigger < trigger) {
            agnus.scheduleAbs<SLOT_ALA>(alarm.trigger, ALA_TRIGGER);
            trigger = alarm.trigger;
        }
    }
}

fs::path
Amiga::tmp()
{
    STATIC_SYNCHRONIZED

    static fs::path base;

    if (base.empty()) {

        // Use /tmp as default folder for temporary files
        base = "/tmp";

        // Open a file to see if we have write permissions
        std::ofstream logfile(base / "vAmiga.log");

        // If /tmp is not accessible, use a different directory
        if (!logfile.is_open()) {

            base = fs::temp_directory_path();
            logfile.open(base / "vAmiga.log");

            if (!logfile.is_open()) {

                throw VAError(ERROR_DIR_NOT_FOUND);
            }
        }

        logfile.close();
        fs::remove(base / "vAmiga.log");
    }

    return base;
}

fs::path
Amiga::tmp(const string &name, bool unique)
{
    STATIC_SYNCHRONIZED

    auto base = tmp();
    auto result = base / name;

    // Make the file name unique if requested
    if (unique) result = fs::path(util::makeUniquePath(result.string()));

    return result;
}

void
Amiga::setDebugVariable(const string &name, int val)
{
#ifdef RELEASEBUILD

    throw VAError(ERROR_OPT_UNSUPPORTED, "Debug variables can only be altered in debug builds.");

#else

    if      (name == "XFILES")           XFILES          = val;
    else if (name == "CNF_DEBUG")        CNF_DEBUG       = val;
    else if (name == "OBJ_DEBUG")        OBJ_DEBUG       = val;
    else if (name == "DEF_DEBUG")        DEF_DEBUG       = val;
    else if (name == "MIMIC_UAE")        MIMIC_UAE       = val;

    else if (name == "RUN_DEBUG")        RUN_DEBUG       = val;
    else if (name == "WARP_DEBUG")       WARP_DEBUG      = val;
    else if (name == "QUEUE_DEBUG")      QUEUE_DEBUG     = val;
    else if (name == "SNP_DEBUG")        SNP_DEBUG       = val;

    else if (name == "CPU_DEBUG")        CPU_DEBUG       = val;
    else if (name == "CST_DEBUG")        CST_DEBUG       = val;

    else if (name == "OCSREG_DEBUG")     OCSREG_DEBUG    = val;
    else if (name == "ECSREG_DEBUG")     ECSREG_DEBUG    = val;
    else if (name == "INVREG_DEBUG")     INVREG_DEBUG    = val;
    else if (name == "MEM_DEBUG")        MEM_DEBUG       = val;

    else if (name == "DMA_DEBUG")        DMA_DEBUG       = val;
    else if (name == "DDF_DEBUG")        DDF_DEBUG       = val;
    else if (name == "SEQ_DEBUG")        SEQ_DEBUG       = val;
    else if (name == "NTSC_DEBUG")       NTSC_DEBUG      = val;

    else if (name == "COP_CHECKSUM")     COP_CHECKSUM    = val;
    else if (name == "COPREG_DEBUG")     COPREG_DEBUG    = val;
    else if (name == "COP_DEBUG")        COP_DEBUG       = val;

    else if (name == "BLT_CHECKSUM")     BLT_CHECKSUM    = val;
    else if (name == "BLTREG_DEBUG")     BLTREG_DEBUG    = val;
    else if (name == "BLT_REG_GUARD")    BLT_REG_GUARD   = val;
    else if (name == "BLT_MEM_GUARD")    BLT_MEM_GUARD   = val;
    else if (name == "BLT_DEBUG")        BLT_DEBUG       = val;
    else if (name == "BLTTIM_DEBUG")     BLTTIM_DEBUG    = val;
    else if (name == "SLOW_BLT_DEBUG")   SLOW_BLT_DEBUG  = val;
    else if (name == "OLD_LINE_BLIT")    OLD_LINE_BLIT   = val;

    else if (name == "BPLREG_DEBUG")     BPLREG_DEBUG    = val;
    else if (name == "BPLDAT_DEBUG")     BPLDAT_DEBUG    = val;
    else if (name == "BPLMOD_DEBUG")     BPLMOD_DEBUG    = val;
    else if (name == "SPRREG_DEBUG")     SPRREG_DEBUG    = val;
    else if (name == "COLREG_DEBUG")     COLREG_DEBUG    = val;
    else if (name == "CLXREG_DEBUG")     CLXREG_DEBUG    = val;
    else if (name == "BPL_DEBUG")        BPL_DEBUG       = val;
    else if (name == "DIW_DEBUG")        DIW_DEBUG       = val;
    else if (name == "SPR_DEBUG")        SPR_DEBUG       = val;
    else if (name == "CLX_DEBUG")        CLX_DEBUG       = val;
    else if (name == "BORDER_DEBUG")     BORDER_DEBUG    = val;

    else if (name == "INTREG_DEBUG")     INTREG_DEBUG    = val;
    else if (name == "INT_DEBUG")        INT_DEBUG       = val;

    else if (name == "CIA_ON_STEROIDS")  CIA_ON_STEROIDS = val;
    else if (name == "CIAREG_DEBUG")     CIAREG_DEBUG    = val;
    else if (name == "CIASER_DEBUG")     CIASER_DEBUG    = val;
    else if (name == "CIA_DEBUG")        CIA_DEBUG       = val;
    else if (name == "TOD_DEBUG")        TOD_DEBUG       = val;

    else if (name == "ALIGN_HEAD")       ALIGN_HEAD      = val;
    else if (name == "DSK_CHECKSUM")     DSK_CHECKSUM    = val;
    else if (name == "DSKREG_DEBUG")     DSKREG_DEBUG    = val;
    else if (name == "DSK_DEBUG")        DSK_DEBUG       = val;
    else if (name == "MFM_DEBUG")        MFM_DEBUG       = val;
    else if (name == "FS_DEBUG")         FS_DEBUG        = val;

    else if (name == "HDR_ACCEPT_ALL")   HDR_ACCEPT_ALL  = val;
    else if (name == "HDR_FS_LOAD_ALL")  HDR_FS_LOAD_ALL = val;
    else if (name == "WT_DEBUG")         WT_DEBUG        = val;

    else if (name == "AUDREG_DEBUG")     AUDREG_DEBUG    = val;
    else if (name == "AUD_DEBUG")        AUD_DEBUG       = val;
    else if (name == "AUDBUF_DEBUG")     AUDBUF_DEBUG    = val;
    else if (name == "DISABLE_AUDIRQ")   DISABLE_AUDIRQ  = val;

    else if (name == "POSREG_DEBUG")     POSREG_DEBUG    = val;
    else if (name == "JOYREG_DEBUG")     JOYREG_DEBUG    = val;
    else if (name == "POTREG_DEBUG")     POTREG_DEBUG    = val;
    else if (name == "PRT_DEBUG")        PRT_DEBUG       = val;
    else if (name == "SER_DEBUG")        SER_DEBUG       = val;
    else if (name == "POT_DEBUG")        POT_DEBUG       = val;
    else if (name == "HOLD_MOUSE_L")     HOLD_MOUSE_L    = val;
    else if (name == "HOLD_MOUSE_M")     HOLD_MOUSE_M    = val;
    else if (name == "HOLD_MOUSE_R")     HOLD_MOUSE_R    = val;

    else if (name == "ZOR_DEBUG")        ZOR_DEBUG       = val;
    else if (name == "ACF_DEBUG")        ACF_DEBUG       = val;
    else if (name == "FAS_DEBUG")        FAS_DEBUG       = val;
    else if (name == "HDR_DEBUG")        HDR_DEBUG       = val;
    else if (name == "DBD_DEBUG")        DBD_DEBUG       = val;

    else if (name == "ADF_DEBUG")        ADF_DEBUG       = val;
    else if (name == "DMS_DEBUG")        DMS_DEBUG       = val;
    else if (name == "IMG_DEBUG")        IMG_DEBUG       = val;

    else if (name == "RTC_DEBUG")        RTC_DEBUG       = val;
    else if (name == "KBD_DEBUG")        KBD_DEBUG       = val;

    else if (name == "REC_DEBUG")        REC_DEBUG       = val;
    else if (name == "SCK_DEBUG")        SCK_DEBUG       = val;
    else if (name == "SRV_DEBUG")        SRV_DEBUG       = val;
    else if (name == "GDB_DEBUG")        GDB_DEBUG       = val;

    else {

        throw VAError(ERROR_OPT_UNSUPPORTED, "Unknown debug variable: " + name);
    }

#endif
}

}
