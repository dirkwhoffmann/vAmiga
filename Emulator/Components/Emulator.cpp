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
// #include "Option.h"
#include "Amiga.h"
#include "Types.h"
#include "CmdQueue.h"
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

}

Emulator::~Emulator()
{
    halt();
}

void
Emulator::launch(const void *listener, Callback *func)
{
    // Initialize the emulator if needed
    if (!isInitialized()) initialize();

    // Connect the listener to the message queue of the main instance
    main.msgQueue.setListener(listener, func);

    // Disable the message queue of the run-ahead instance
    ahead.msgQueue.disable();

    // Launch the emulator thread
    Thread::launch();
}

void
Emulator::initialize()
{
    // Make sure this function is only called once
    if (isInitialized()) throw Error(VAERROR_LAUNCH, "The emulator is already initialized.");

    // Initialize all components
    main.initialize();
    ahead.initialize();

    // Setup the default configuration
    main.resetConfig();

    // Perform a hard reset
    main.hardReset();

    // Get the runahead instance up-to-date
    ahead = main;

    // Switch state
    state = newState = STATE_OFF;
    assert(isInitialized());
}

void
Emulator::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Debug) {

        for (isize i = DebugFlagEnum::minVal; i < DebugFlagEnum::maxVal; i++) {

            os << tab(DebugFlagEnum::key(i));
            os << dec(getDebugVariable(DebugFlag(i))) << std::endl;
        }
    }

    if (category == Category::Defaults) {

        defaults.dump(category, os);
    }

    /*
    if (category == Category::RunAhead) {

        os << "Primary instance:" << std::endl << std::endl;

        os << tab("Frame");
        os << dec(main.frame) << std::endl;
        os << tab("Beam");
        os << "(" << dec(main.scanline) << "," << dec(main.rasterCycle) << ")" << std::endl;
        os << tab("Cycle");
        os << dec(main.cpu.clock) << std::endl << std::endl;

        os << "Run-ahead instance:" << std::endl << std::endl;

        os << tab("Clone nr");
        os << dec(clones) << std::endl;
        os << tab("Frame");
        os << dec(ahead.frame) << std::endl;
        os << tab("Beam");
        os << " (" << dec(ahead.scanline) << "," << dec(ahead.rasterCycle) << ")" << std::endl;
        os << tab("Cycle");
        os << dec(ahead.cpu.clock) << std::endl;
    }
    */

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

void
Emulator::cacheInfo(EmulatorInfo &result) const
{
    {   SYNCHRONIZED

        result.state = state;
        result.powered = isPoweredOn();
        result.paused = isPaused();
        result.running = isRunning();
        result.suspended = isSuspended();
        result.warping = isWarping();
        result.tracking = isTracking();
    }
}

void
Emulator::cacheStats(EmulatorStats &result) const
{
    {   SYNCHRONIZED

        result.cpuLoad = cpuLoad;
        result.fps = fps;
        result.resyncs = resyncs;
    }

}

void
Emulator::put(const Cmd &cmd)
{
    cmdQueue.put(cmd);
}

i64
Emulator::get(Option opt, isize objid) const
{
    return main.get(opt, objid);
}

void
Emulator::check(Option opt, i64 value, const std::vector<isize> objids)
{
    return main.check(opt, value, objids);
}

void
Emulator::set(Option opt, i64 value, const std::vector<isize> objids)
{
    return main.set(opt, value, objids);
}

void 
Emulator::set(Option opt, const string &value, const std::vector<isize> objids)
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
    
    // Compute which frame should be reached by now
    auto target = elapsed.asNanoseconds() * i64(main.refreshRate()) / 1000000000;

    // Compute the number of missing frames
    return isize(target - frameCounter);
}

const FrameBuffer &
Emulator::getTexture() const
{
    auto &result = main.config.runAhead && isRunning() ?
    ahead.videoPort.getTexture() :
    main.videoPort.getTexture();

    return result;
}

void
Emulator::hardReset()
{
    {   SUSPENDED

        main.hardReset();
    }
}

void 
Emulator::softReset()
{
    {   SUSPENDED

        main.softReset();
    }
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
Emulator::computeFrame()
{
    auto &config = main.getConfig();

    if (config.runAhead) {

        try {

            // Run the main instance
            main.computeFrame();

            // Recreate the run-ahead instance if necessary
            if (isDirty || RUA_ON_STEROIDS) recreateRunAheadInstance();

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
Emulator::cloneRunAheadInstance()
{
    // clones++;

    // Recreate the runahead instance from scratch
    ahead = main; isDirty = false;

    if (RUA_CHECKSUM && ahead != main) {

        main.diff(ahead);
        fatal("Corrupted run-ahead clone detected");
    }
}

void
Emulator::recreateRunAheadInstance()
{
    auto &config = main.getConfig();

    // Clone the main instance
    if (RUA_DEBUG) {
        util::StopWatch watch("Run-ahead: Clone");
        cloneRunAheadInstance();
    } else {
        cloneRunAheadInstance();
    }

    // Advance to the proper frame
    if (RUA_DEBUG) {
        util::StopWatch watch("Run-ahead: Fast-forward");
        ahead.fastForward(config.runAhead - 1);
    } else {
        ahead.fastForward(config.runAhead - 1);
    }
}

void
Emulator::isReady()
{
    main.isReady();
}

int
Emulator::getDebugVariable(DebugFlag flag)
{
#ifdef RELEASEBUILD

    throw Error(VAERROR_OPT_UNSUPPORTED, "Debug variables are only accessible in debug builds.");

#else

    switch (flag) {

        case FLAG_XFILES:           return XFILES;
        case FLAG_CNF_DEBUG:        return CNF_DEBUG;
        case FLAG_OBJ_DEBUG:        return OBJ_DEBUG;
        case FLAG_DEF_DEBUG:        return DEF_DEBUG;
        case FLAG_MIMIC_UAE:        return MIMIC_UAE;

        case FLAG_RUN_DEBUG:        return RUN_DEBUG;
        case FLAG_TIM_DEBUG:        return TIM_DEBUG;
        case FLAG_WARP_DEBUG:       return WARP_DEBUG;
        case FLAG_CMD_DEBUG:        return CMD_DEBUG;
        case FLAG_MSG_DEBUG:        return MSG_DEBUG;
        case FLAG_SNP_DEBUG:        return SNP_DEBUG;

        case FLAG_RUA_DEBUG:        return RUA_DEBUG;
        case FLAG_RUA_CHECKSUM:     return RUA_CHECKSUM;
        case FLAG_RUA_ON_STEROIDS:  return RUA_ON_STEROIDS;

        case FLAG_CPU_DEBUG:        return CPU_DEBUG;

        case FLAG_OCSREG_DEBUG:     return OCSREG_DEBUG;
        case FLAG_ECSREG_DEBUG:     return ECSREG_DEBUG;
        case FLAG_INVREG_DEBUG:     return INVREG_DEBUG;
        case FLAG_MEM_DEBUG:        return MEM_DEBUG;

        case FLAG_DMA_DEBUG:        return DMA_DEBUG;
        case FLAG_DDF_DEBUG:        return DDF_DEBUG;
        case FLAG_SEQ_DEBUG:        return SEQ_DEBUG;
        case FLAG_SEQ_ON_STEROIDS:  return SEQ_ON_STEROIDS;
        case FLAG_NTSC_DEBUG:       return NTSC_DEBUG;

        case FLAG_COP_CHECKSUM:     return COP_CHECKSUM;
        case FLAG_COPREG_DEBUG:     return COPREG_DEBUG;
        case FLAG_COP_DEBUG:        return COP_DEBUG;

        case FLAG_BLT_CHECKSUM:     return BLT_CHECKSUM;
        case FLAG_BLTREG_DEBUG:     return BLTREG_DEBUG;
        case FLAG_BLT_REG_GUARD:    return BLT_REG_GUARD;
        case FLAG_BLT_MEM_GUARD:    return BLT_MEM_GUARD;
        case FLAG_BLT_DEBUG:        return BLT_DEBUG;
        case FLAG_BLTTIM_DEBUG:     return BLTTIM_DEBUG;
        case FLAG_SLOW_BLT_DEBUG:   return SLOW_BLT_DEBUG;

        case FLAG_BPLREG_DEBUG:     return BPLREG_DEBUG;
        case FLAG_BPLDAT_DEBUG:     return BPLDAT_DEBUG;
        case FLAG_BPLMOD_DEBUG:     return BPLMOD_DEBUG;
        case FLAG_SPRREG_DEBUG:     return SPRREG_DEBUG;
        case FLAG_COLREG_DEBUG:     return COLREG_DEBUG;
        case FLAG_CLXREG_DEBUG:     return CLXREG_DEBUG;
        case FLAG_BPL_ON_STEROIDS:  return BPL_ON_STEROIDS;
        case FLAG_DIW_DEBUG:        return DIW_DEBUG;
        case FLAG_SPR_DEBUG:        return SPR_DEBUG;
        case FLAG_CLX_DEBUG:        return CLX_DEBUG;
        case FLAG_BORDER_DEBUG:     return BORDER_DEBUG;
        case FLAG_LINE_DEBUG:       return LINE_DEBUG;

        case FLAG_INTREG_DEBUG:     return INTREG_DEBUG;
        case FLAG_INT_DEBUG:        return INT_DEBUG;

        case FLAG_CIAREG_DEBUG:     return CIAREG_DEBUG;
        case FLAG_CIASER_DEBUG:     return CIASER_DEBUG;
        case FLAG_CIA_DEBUG:        return CIA_DEBUG;
        case FLAG_TOD_DEBUG:        return TOD_DEBUG;

        case FLAG_ALIGN_HEAD:       return ALIGN_HEAD;
        case FLAG_DSK_CHECKSUM:     return DSK_CHECKSUM;
        case FLAG_DSKREG_DEBUG:     return DSKREG_DEBUG;
        case FLAG_DSK_DEBUG:        return DSK_DEBUG;
        case FLAG_MFM_DEBUG:        return MFM_DEBUG;
        case FLAG_FS_DEBUG:         return FS_DEBUG;

        case FLAG_HDR_ACCEPT_ALL:   return HDR_ACCEPT_ALL;
        case FLAG_HDR_FS_LOAD_ALL:  return HDR_FS_LOAD_ALL;
        case FLAG_WT_DEBUG:         return WT_DEBUG;

        case FLAG_AUDREG_DEBUG:     return AUDREG_DEBUG;
        case FLAG_AUD_DEBUG:        return AUD_DEBUG;
        case FLAG_AUDBUF_DEBUG:     return AUDBUF_DEBUG;
        case FLAG_AUDVOL_DEBUG:     return AUDVOL_DEBUG;
        case FLAG_DISABLE_AUDIRQ:   return DISABLE_AUDIRQ;

        case FLAG_POSREG_DEBUG:     return POSREG_DEBUG;
        case FLAG_JOYREG_DEBUG:     return JOYREG_DEBUG;
        case FLAG_POTREG_DEBUG:     return POTREG_DEBUG;
        case FLAG_VID_DEBUG:        return VID_DEBUG;
        case FLAG_PRT_DEBUG:        return PRT_DEBUG;
        case FLAG_SER_DEBUG:        return SER_DEBUG;
        case FLAG_POT_DEBUG:        return POT_DEBUG;
        case FLAG_HOLD_MOUSE_L:     return HOLD_MOUSE_L;
        case FLAG_HOLD_MOUSE_M:     return HOLD_MOUSE_M;
        case FLAG_HOLD_MOUSE_R:     return HOLD_MOUSE_R;

        case FLAG_ZOR_DEBUG:        return ZOR_DEBUG;
        case FLAG_ACF_DEBUG:        return ACF_DEBUG;
        case FLAG_FAS_DEBUG:        return FAS_DEBUG;
        case FLAG_HDR_DEBUG:        return HDR_DEBUG;
        case FLAG_DBD_DEBUG:        return DBD_DEBUG;

        case FLAG_ADF_DEBUG:        return ADF_DEBUG;
        case FLAG_DMS_DEBUG:        return DMS_DEBUG;
        case FLAG_IMG_DEBUG:        return IMG_DEBUG;

        case FLAG_RTC_DEBUG:        return RTC_DEBUG;
        case FLAG_KBD_DEBUG:        return KBD_DEBUG;
        case FLAG_KEY_DEBUG:        return KEY_DEBUG;

        case FLAG_REC_DEBUG:        return REC_DEBUG;
        case FLAG_SCK_DEBUG:        return SCK_DEBUG;
        case FLAG_SRV_DEBUG:        return SRV_DEBUG;
        case FLAG_GDB_DEBUG:        return GDB_DEBUG;

        default:
            throw Error(VAERROR_OPT_UNSUPPORTED,
                        "Unhandled debug variable: " + string(DebugFlagEnum::key(flag)));
    }

#endif
}

void
Emulator::setDebugVariable(DebugFlag flag, bool val)
{
#ifdef RELEASEBUILD

    throw Error(VAERROR_OPT_UNSUPPORTED, "Debug variables are only accessible in debug builds.");

#else

    switch (flag) {

            // General
        case FLAG_XFILES:           XFILES = val; break;
        case FLAG_CNF_DEBUG:        CNF_DEBUG = val; break;
        case FLAG_OBJ_DEBUG:        OBJ_DEBUG = val; break;
        case FLAG_DEF_DEBUG:        DEF_DEBUG = val; break;
        case FLAG_MIMIC_UAE:        MIMIC_UAE = val; break;

            // Runloop
        case FLAG_RUN_DEBUG:        RUN_DEBUG = val; break;
        case FLAG_TIM_DEBUG:        TIM_DEBUG = val; break;
        case FLAG_WARP_DEBUG:       WARP_DEBUG = val; break;
        case FLAG_CMD_DEBUG:        CMD_DEBUG = val; break;
        case FLAG_MSG_DEBUG:        MSG_DEBUG = val; break;
        case FLAG_SNP_DEBUG:        SNP_DEBUG = val; break;

            // Run-ahead
        case FLAG_RUA_DEBUG:        RUA_DEBUG = val; break;
        case FLAG_RUA_CHECKSUM:     RUA_CHECKSUM = val; break;
        case FLAG_RUA_ON_STEROIDS:  RUA_ON_STEROIDS = val; break;

            // CPU
        case FLAG_CPU_DEBUG:        CPU_DEBUG = val; break;

            // Memory access
        case FLAG_OCSREG_DEBUG:     OCSREG_DEBUG = val; break;
        case FLAG_ECSREG_DEBUG:     ECSREG_DEBUG = val; break;
        case FLAG_INVREG_DEBUG:     INVREG_DEBUG = val; break;
        case FLAG_MEM_DEBUG:        MEM_DEBUG = val; break;

            // Agnus
        case FLAG_DMA_DEBUG:        DMA_DEBUG = val; break;
        case FLAG_DDF_DEBUG:        DDF_DEBUG = val; break;
        case FLAG_SEQ_DEBUG:        SEQ_DEBUG = val; break;
        case FLAG_SEQ_ON_STEROIDS:  SEQ_ON_STEROIDS = val; break;
        case FLAG_NTSC_DEBUG:       NTSC_DEBUG = val; break;

            // Copper
        case FLAG_COP_CHECKSUM:     COP_CHECKSUM = val; break;
        case FLAG_COPREG_DEBUG:     COPREG_DEBUG = val; break;
        case FLAG_COP_DEBUG:        COP_DEBUG = val; break;

            // Blitter
        case FLAG_BLT_CHECKSUM:     BLT_CHECKSUM = val; break;
        case FLAG_BLTREG_DEBUG:     BLTREG_DEBUG = val; break;
        case FLAG_BLT_REG_GUARD:    BLT_REG_GUARD = val; break;
        case FLAG_BLT_MEM_GUARD:    BLT_MEM_GUARD = val; break;
        case FLAG_BLT_DEBUG:        BLT_DEBUG = val; break;
        case FLAG_BLTTIM_DEBUG:     BLTTIM_DEBUG = val; break;
        case FLAG_SLOW_BLT_DEBUG:   SLOW_BLT_DEBUG = val; break;

            // Denise
        case FLAG_BPLREG_DEBUG:     BPLREG_DEBUG = val; break;
        case FLAG_BPLDAT_DEBUG:     BPLDAT_DEBUG = val; break;
        case FLAG_BPLMOD_DEBUG:     BPLMOD_DEBUG = val; break;
        case FLAG_SPRREG_DEBUG:     SPRREG_DEBUG = val; break;
        case FLAG_COLREG_DEBUG:     COLREG_DEBUG = val; break;
        case FLAG_CLXREG_DEBUG:     CLXREG_DEBUG = val; break;
        case FLAG_BPL_ON_STEROIDS:  BPL_ON_STEROIDS = val; break;
        case FLAG_DIW_DEBUG:        DIW_DEBUG = val; break;
        case FLAG_SPR_DEBUG:        SPR_DEBUG = val; break;
        case FLAG_CLX_DEBUG:        CLX_DEBUG = val; break;
        case FLAG_BORDER_DEBUG:     BORDER_DEBUG = val; break;
        case FLAG_LINE_DEBUG:       LINE_DEBUG = val; break;

            // Paula
        case FLAG_INTREG_DEBUG:     INTREG_DEBUG = val; break;
        case FLAG_INT_DEBUG:        INT_DEBUG = val; break;

            // CIAs
        case FLAG_CIAREG_DEBUG:     CIAREG_DEBUG = val; break;
        case FLAG_CIASER_DEBUG:     CIASER_DEBUG = val; break;
        case FLAG_CIA_DEBUG:        CIA_DEBUG = val; break;
        case FLAG_TOD_DEBUG:        TOD_DEBUG = val; break;

            // Floppy Drives
        case FLAG_ALIGN_HEAD:       ALIGN_HEAD = val; break;
        case FLAG_DSK_CHECKSUM:     DSK_CHECKSUM = val; break;
        case FLAG_DSKREG_DEBUG:     DSKREG_DEBUG = val; break;
        case FLAG_DSK_DEBUG:        DSK_DEBUG = val; break;
        case FLAG_MFM_DEBUG:        MFM_DEBUG = val; break;
        case FLAG_FS_DEBUG:         FS_DEBUG = val; break;

            // Hard Drives
        case FLAG_HDR_ACCEPT_ALL:   HDR_ACCEPT_ALL = val; break;
        case FLAG_HDR_FS_LOAD_ALL:  HDR_FS_LOAD_ALL = val; break;
        case FLAG_WT_DEBUG:         WT_DEBUG = val; break;

            // Audio
        case FLAG_AUDREG_DEBUG:     AUDREG_DEBUG = val; break;
        case FLAG_AUD_DEBUG:        AUD_DEBUG = val; break;
        case FLAG_AUDBUF_DEBUG:     AUDBUF_DEBUG = val; break;
        case FLAG_AUDVOL_DEBUG:     AUDVOL_DEBUG = val; break;
        case FLAG_DISABLE_AUDIRQ:   DISABLE_AUDIRQ = val; break;

            // Ports
        case FLAG_POSREG_DEBUG:     POSREG_DEBUG = val; break;
        case FLAG_JOYREG_DEBUG:     JOYREG_DEBUG = val; break;
        case FLAG_POTREG_DEBUG:     POTREG_DEBUG = val; break;
        case FLAG_VID_DEBUG:        VID_DEBUG = val; break;
        case FLAG_PRT_DEBUG:        PRT_DEBUG = val; break;
        case FLAG_SER_DEBUG:        SER_DEBUG = val; break;
        case FLAG_POT_DEBUG:        POT_DEBUG = val; break;
        case FLAG_HOLD_MOUSE_L:     HOLD_MOUSE_L = val; break;
        case FLAG_HOLD_MOUSE_M:     HOLD_MOUSE_M = val; break;
        case FLAG_HOLD_MOUSE_R:     HOLD_MOUSE_R = val; break;

            // Expansion boards
        case FLAG_ZOR_DEBUG:        ZOR_DEBUG = val; break;
        case FLAG_ACF_DEBUG:        ACF_DEBUG = val; break;
        case FLAG_FAS_DEBUG:        FAS_DEBUG = val; break;
        case FLAG_HDR_DEBUG:        HDR_DEBUG = val; break;
        case FLAG_DBD_DEBUG:        DBD_DEBUG = val; break;

            // Media types
        case FLAG_ADF_DEBUG:        ADF_DEBUG = val; break;
        case FLAG_DMS_DEBUG:        DMS_DEBUG = val; break;
        case FLAG_IMG_DEBUG:        IMG_DEBUG = val; break;

            // Other components
        case FLAG_RTC_DEBUG:        RTC_DEBUG = val; break;
        case FLAG_KBD_DEBUG:        KBD_DEBUG = val; break;
        case FLAG_KEY_DEBUG:        KEY_DEBUG = val; break;

            // Misc
        case FLAG_REC_DEBUG:        REC_DEBUG = val; break;
        case FLAG_SCK_DEBUG:        SCK_DEBUG = val; break;
        case FLAG_SRV_DEBUG:        SRV_DEBUG = val; break;
        case FLAG_GDB_DEBUG:        GDB_DEBUG = val; break;

        default:
            throw Error(VAERROR_OPT_UNSUPPORTED,
                        "Unhandled debug variable: " + string(DebugFlagEnum::key(flag)));
    }
#endif
}

}
