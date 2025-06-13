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
    if (FORCE_LAUNCH_ERROR) throw AppError(Fault::LAUNCH);

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
    baseTime = util::Time::now();

    // Make sure this function is only called once
    if (isInitialized()) throw AppError(Fault::LAUNCH, "The emulator is already initialized.");

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
    using namespace util;

    if (category == Category::Debug) {

        for (const auto &i : DebugFlagEnum::elements()) {

            os << tab(DebugFlagEnum::key(i));
            os << dec(getDebugVariable(DebugFlag(i))) << std::endl;
        }
    }
    
    if (category == Category::Defaults) {

        defaults.dump(category, os);
    }

    if (category == Category::RunAhead) {

         auto &pos = main.agnus.pos;
         auto &rua = ahead.agnus.pos;

         os << "Primary instance:" << std::endl << std::endl;

         os << tab("Frame");
         os << dec(pos.frame) << std::endl;
         os << tab("Beam");
         os << "(" << dec(pos.v) << "," << dec(pos.h) << ")" << std::endl;

         os << "Run-ahead instance:" << std::endl << std::endl;

         os << tab("Clone nr");
         os << dec(stats.clones) << std::endl;
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
    auto elapsed = util::Time::now() - baseTime;

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
Emulator::isReady()
{
    main.isReady();
}

void
Emulator::cloneRunAheadInstance()
{
    stats.clones++;

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
    assert(main.config.runAhead > 0);

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

int
Emulator::getDebugVariable(DebugFlag flag)
{
#ifdef NDEBUG
    
    throw AppError(Fault::OPT_UNSUPPORTED, "Debug variables are only accessible in debug builds.");
    
#else
    
    switch (flag) {
            
        case DebugFlag::XFILES:           return XFILES;
        case DebugFlag::CNF_DEBUG:        return CNF_DEBUG;
        case DebugFlag::OBJ_DEBUG:        return OBJ_DEBUG;
        case DebugFlag::DEF_DEBUG:        return DEF_DEBUG;
        case DebugFlag::MIMIC_UAE:        return MIMIC_UAE;
            
        case DebugFlag::RUN_DEBUG:        return RUN_DEBUG;
        case DebugFlag::TIM_DEBUG:        return TIM_DEBUG;
        case DebugFlag::WARP_DEBUG:       return WARP_DEBUG;
        case DebugFlag::CMD_DEBUG:        return CMD_DEBUG;
        case DebugFlag::MSG_DEBUG:        return MSG_DEBUG;
        case DebugFlag::SNP_DEBUG:        return SNP_DEBUG;
            
        case DebugFlag::RUA_DEBUG:        return RUA_DEBUG;
        case DebugFlag::RUA_CHECKSUM:     return RUA_CHECKSUM;
        case DebugFlag::RUA_ON_STEROIDS:  return RUA_ON_STEROIDS;
            
        case DebugFlag::CPU_DEBUG:        return CPU_DEBUG;
            
        case DebugFlag::OCSREG_DEBUG:     return OCSREG_DEBUG;
        case DebugFlag::ECSREG_DEBUG:     return ECSREG_DEBUG;
        case DebugFlag::INVREG_DEBUG:     return INVREG_DEBUG;
        case DebugFlag::MEM_DEBUG:        return MEM_DEBUG;
            
        case DebugFlag::DMA_DEBUG:        return DMA_DEBUG;
        case DebugFlag::DDF_DEBUG:        return DDF_DEBUG;
        case DebugFlag::SEQ_DEBUG:        return SEQ_DEBUG;
        case DebugFlag::SEQ_ON_STEROIDS:  return SEQ_ON_STEROIDS;
        case DebugFlag::NTSC_DEBUG:       return NTSC_DEBUG;
            
        case DebugFlag::COP_CHECKSUM:     return COP_CHECKSUM;
        case DebugFlag::COPREG_DEBUG:     return COPREG_DEBUG;
        case DebugFlag::COP_DEBUG:        return COP_DEBUG;
            
        case DebugFlag::BLT_CHECKSUM:     return BLT_CHECKSUM;
        case DebugFlag::BLTREG_DEBUG:     return BLTREG_DEBUG;
        case DebugFlag::BLT_REG_GUARD:    return BLT_REG_GUARD;
        case DebugFlag::BLT_MEM_GUARD:    return BLT_MEM_GUARD;
        case DebugFlag::BLT_DEBUG:        return BLT_DEBUG;
        case DebugFlag::BLTTIM_DEBUG:     return BLTTIM_DEBUG;
        case DebugFlag::SLOW_BLT_DEBUG:   return SLOW_BLT_DEBUG;
            
        case DebugFlag::BPLREG_DEBUG:     return BPLREG_DEBUG;
        case DebugFlag::BPLDAT_DEBUG:     return BPLDAT_DEBUG;
        case DebugFlag::BPLMOD_DEBUG:     return BPLMOD_DEBUG;
        case DebugFlag::SPRREG_DEBUG:     return SPRREG_DEBUG;
        case DebugFlag::COLREG_DEBUG:     return COLREG_DEBUG;
        case DebugFlag::CLXREG_DEBUG:     return CLXREG_DEBUG;
        case DebugFlag::BPL_ON_STEROIDS:  return BPL_ON_STEROIDS;
        case DebugFlag::DIW_DEBUG:        return DIW_DEBUG;
        case DebugFlag::SPR_DEBUG:        return SPR_DEBUG;
        case DebugFlag::CLX_DEBUG:        return CLX_DEBUG;
        case DebugFlag::BORDER_DEBUG:     return BORDER_DEBUG;
        case DebugFlag::LINE_DEBUG:       return LINE_DEBUG;
            
        case DebugFlag::INTREG_DEBUG:     return INTREG_DEBUG;
        case DebugFlag::INT_DEBUG:        return INT_DEBUG;
            
        case DebugFlag::CIAREG_DEBUG:     return CIAREG_DEBUG;
        case DebugFlag::CIASER_DEBUG:     return CIASER_DEBUG;
        case DebugFlag::CIA_DEBUG:        return CIA_DEBUG;
        case DebugFlag::TOD_DEBUG:        return TOD_DEBUG;
            
        case DebugFlag::ALIGN_HEAD:       return ALIGN_HEAD;
        case DebugFlag::DSK_CHECKSUM:     return DSK_CHECKSUM;
        case DebugFlag::DSKREG_DEBUG:     return DSKREG_DEBUG;
        case DebugFlag::DSK_DEBUG:        return DSK_DEBUG;
        case DebugFlag::MFM_DEBUG:        return MFM_DEBUG;
        case DebugFlag::FS_DEBUG:         return FS_DEBUG;
            
        case DebugFlag::HDR_ACCEPT_ALL:   return HDR_ACCEPT_ALL;
        case DebugFlag::HDR_FS_LOAD_ALL:  return HDR_FS_LOAD_ALL;
        case DebugFlag::WT_DEBUG:         return WT_DEBUG;
            
        case DebugFlag::AUDREG_DEBUG:     return AUDREG_DEBUG;
        case DebugFlag::AUD_DEBUG:        return AUD_DEBUG;
        case DebugFlag::AUDBUF_DEBUG:     return AUDBUF_DEBUG;
        case DebugFlag::AUDVOL_DEBUG:     return AUDVOL_DEBUG;
        case DebugFlag::DISABLE_AUDIRQ:   return DISABLE_AUDIRQ;
            
        case DebugFlag::POSREG_DEBUG:     return POSREG_DEBUG;
        case DebugFlag::JOYREG_DEBUG:     return JOYREG_DEBUG;
        case DebugFlag::POTREG_DEBUG:     return POTREG_DEBUG;
        case DebugFlag::VID_DEBUG:        return VID_DEBUG;
        case DebugFlag::PRT_DEBUG:        return PRT_DEBUG;
        case DebugFlag::SER_DEBUG:        return SER_DEBUG;
        case DebugFlag::POT_DEBUG:        return POT_DEBUG;
        case DebugFlag::HOLD_MOUSE_L:     return HOLD_MOUSE_L;
        case DebugFlag::HOLD_MOUSE_M:     return HOLD_MOUSE_M;
        case DebugFlag::HOLD_MOUSE_R:     return HOLD_MOUSE_R;
            
        case DebugFlag::ZOR_DEBUG:        return ZOR_DEBUG;
        case DebugFlag::ACF_DEBUG:        return ACF_DEBUG;
        case DebugFlag::FAS_DEBUG:        return FAS_DEBUG;
        case DebugFlag::HDR_DEBUG:        return HDR_DEBUG;
        case DebugFlag::DBD_DEBUG:        return DBD_DEBUG;
            
        case DebugFlag::ADF_DEBUG:        return ADF_DEBUG;
        case DebugFlag::HDF_DEBUG:        return HDF_DEBUG;
        case DebugFlag::DMS_DEBUG:        return DMS_DEBUG;
        case DebugFlag::IMG_DEBUG:        return IMG_DEBUG;
            
        case DebugFlag::RTC_DEBUG:        return RTC_DEBUG;
        case DebugFlag::KBD_DEBUG:        return KBD_DEBUG;
        case DebugFlag::KEY_DEBUG:        return KEY_DEBUG;
            
        case DebugFlag::RSH_DEBUG:        return RSH_DEBUG;
        case DebugFlag::REC_DEBUG:        return REC_DEBUG;
        case DebugFlag::SCK_DEBUG:        return SCK_DEBUG;
        case DebugFlag::SRV_DEBUG:        return SRV_DEBUG;
        case DebugFlag::GDB_DEBUG:        return GDB_DEBUG;
            
        default:
            throw AppError(Fault::OPT_UNSUPPORTED,
                        "Unhandled debug variable: " + string(DebugFlagEnum::key(flag)));
    }
    
#endif
}

void
Emulator::setDebugVariable(DebugFlag flag, bool val)
{
#ifdef NDEBUG
    
    throw AppError(Fault::OPT_UNSUPPORTED, "Debug variables are only accessible in debug builds.");

#else
    
    switch (flag) {
            
            // General
        case DebugFlag::XFILES:           XFILES = val; break;
        case DebugFlag::CNF_DEBUG:        CNF_DEBUG = val; break;
        case DebugFlag::OBJ_DEBUG:        OBJ_DEBUG = val; break;
        case DebugFlag::DEF_DEBUG:        DEF_DEBUG = val; break;
        case DebugFlag::MIMIC_UAE:        MIMIC_UAE = val; break;
            
            // Runloop
        case DebugFlag::RUN_DEBUG:        RUN_DEBUG = val; break;
        case DebugFlag::TIM_DEBUG:        TIM_DEBUG = val; break;
        case DebugFlag::WARP_DEBUG:       WARP_DEBUG = val; break;
        case DebugFlag::CMD_DEBUG:        CMD_DEBUG = val; break;
        case DebugFlag::MSG_DEBUG:        MSG_DEBUG = val; break;
        case DebugFlag::SNP_DEBUG:        SNP_DEBUG = val; break;
            
            // Run-ahead
        case DebugFlag::RUA_DEBUG:        RUA_DEBUG = val; break;
        case DebugFlag::RUA_CHECKSUM:     RUA_CHECKSUM = val; break;
        case DebugFlag::RUA_ON_STEROIDS:  RUA_ON_STEROIDS = val; break;
            
            // CPU
        case DebugFlag::CPU_DEBUG:        CPU_DEBUG = val; break;
            
            // Memory access
        case DebugFlag::OCSREG_DEBUG:     OCSREG_DEBUG = val; break;
        case DebugFlag::ECSREG_DEBUG:     ECSREG_DEBUG = val; break;
        case DebugFlag::INVREG_DEBUG:     INVREG_DEBUG = val; break;
        case DebugFlag::MEM_DEBUG:        MEM_DEBUG = val; break;
            
            // Agnus
        case DebugFlag::DMA_DEBUG:        DMA_DEBUG = val; break;
        case DebugFlag::DDF_DEBUG:        DDF_DEBUG = val; break;
        case DebugFlag::SEQ_DEBUG:        SEQ_DEBUG = val; break;
        case DebugFlag::SEQ_ON_STEROIDS:  SEQ_ON_STEROIDS = val; break;
        case DebugFlag::NTSC_DEBUG:       NTSC_DEBUG = val; break;
            
            // Copper
        case DebugFlag::COP_CHECKSUM:     COP_CHECKSUM = val; break;
        case DebugFlag::COPREG_DEBUG:     COPREG_DEBUG = val; break;
        case DebugFlag::COP_DEBUG:        COP_DEBUG = val; break;
            
            // Blitter
        case DebugFlag::BLT_CHECKSUM:     BLT_CHECKSUM = val; break;
        case DebugFlag::BLTREG_DEBUG:     BLTREG_DEBUG = val; break;
        case DebugFlag::BLT_REG_GUARD:    BLT_REG_GUARD = val; break;
        case DebugFlag::BLT_MEM_GUARD:    BLT_MEM_GUARD = val; break;
        case DebugFlag::BLT_DEBUG:        BLT_DEBUG = val; break;
        case DebugFlag::BLTTIM_DEBUG:     BLTTIM_DEBUG = val; break;
        case DebugFlag::SLOW_BLT_DEBUG:   SLOW_BLT_DEBUG = val; break;
            
            // Denise
        case DebugFlag::BPLREG_DEBUG:     BPLREG_DEBUG = val; break;
        case DebugFlag::BPLDAT_DEBUG:     BPLDAT_DEBUG = val; break;
        case DebugFlag::BPLMOD_DEBUG:     BPLMOD_DEBUG = val; break;
        case DebugFlag::SPRREG_DEBUG:     SPRREG_DEBUG = val; break;
        case DebugFlag::COLREG_DEBUG:     COLREG_DEBUG = val; break;
        case DebugFlag::CLXREG_DEBUG:     CLXREG_DEBUG = val; break;
        case DebugFlag::BPL_ON_STEROIDS:  BPL_ON_STEROIDS = val; break;
        case DebugFlag::DIW_DEBUG:        DIW_DEBUG = val; break;
        case DebugFlag::SPR_DEBUG:        SPR_DEBUG = val; break;
        case DebugFlag::CLX_DEBUG:        CLX_DEBUG = val; break;
        case DebugFlag::BORDER_DEBUG:     BORDER_DEBUG = val; break;
        case DebugFlag::LINE_DEBUG:       LINE_DEBUG = val; break;
            
            // Paula
        case DebugFlag::INTREG_DEBUG:     INTREG_DEBUG = val; break;
        case DebugFlag::INT_DEBUG:        INT_DEBUG = val; break;
            
            // CIAs
        case DebugFlag::CIAREG_DEBUG:     CIAREG_DEBUG = val; break;
        case DebugFlag::CIASER_DEBUG:     CIASER_DEBUG = val; break;
        case DebugFlag::CIA_DEBUG:        CIA_DEBUG = val; break;
        case DebugFlag::TOD_DEBUG:        TOD_DEBUG = val; break;
            
            // Floppy Drives
        case DebugFlag::ALIGN_HEAD:       ALIGN_HEAD = val; break;
        case DebugFlag::DSK_CHECKSUM:     DSK_CHECKSUM = val; break;
        case DebugFlag::DSKREG_DEBUG:     DSKREG_DEBUG = val; break;
        case DebugFlag::DSK_DEBUG:        DSK_DEBUG = val; break;
        case DebugFlag::MFM_DEBUG:        MFM_DEBUG = val; break;
        case DebugFlag::FS_DEBUG:         FS_DEBUG = val; break;
            
            // Hard Drives
        case DebugFlag::HDR_ACCEPT_ALL:   HDR_ACCEPT_ALL = val; break;
        case DebugFlag::HDR_FS_LOAD_ALL:  HDR_FS_LOAD_ALL = val; break;
        case DebugFlag::WT_DEBUG:         WT_DEBUG = val; break;
            
            // Audio
        case DebugFlag::AUDREG_DEBUG:     AUDREG_DEBUG = val; break;
        case DebugFlag::AUD_DEBUG:        AUD_DEBUG = val; break;
        case DebugFlag::AUDBUF_DEBUG:     AUDBUF_DEBUG = val; break;
        case DebugFlag::AUDVOL_DEBUG:     AUDVOL_DEBUG = val; break;
        case DebugFlag::DISABLE_AUDIRQ:   DISABLE_AUDIRQ = val; break;
            
            // Ports
        case DebugFlag::POSREG_DEBUG:     POSREG_DEBUG = val; break;
        case DebugFlag::JOYREG_DEBUG:     JOYREG_DEBUG = val; break;
        case DebugFlag::POTREG_DEBUG:     POTREG_DEBUG = val; break;
        case DebugFlag::VID_DEBUG:        VID_DEBUG = val; break;
        case DebugFlag::PRT_DEBUG:        PRT_DEBUG = val; break;
        case DebugFlag::SER_DEBUG:        SER_DEBUG = val; break;
        case DebugFlag::POT_DEBUG:        POT_DEBUG = val; break;
        case DebugFlag::HOLD_MOUSE_L:     HOLD_MOUSE_L = val; break;
        case DebugFlag::HOLD_MOUSE_M:     HOLD_MOUSE_M = val; break;
        case DebugFlag::HOLD_MOUSE_R:     HOLD_MOUSE_R = val; break;
            
            // Expansion boards
        case DebugFlag::ZOR_DEBUG:        ZOR_DEBUG = val; break;
        case DebugFlag::ACF_DEBUG:        ACF_DEBUG = val; break;
        case DebugFlag::FAS_DEBUG:        FAS_DEBUG = val; break;
        case DebugFlag::HDR_DEBUG:        HDR_DEBUG = val; break;
        case DebugFlag::DBD_DEBUG:        DBD_DEBUG = val; break;
            
            // Media types
        case DebugFlag::ADF_DEBUG:        ADF_DEBUG = val; break;
        case DebugFlag::HDF_DEBUG:        HDF_DEBUG = val; break;
        case DebugFlag::DMS_DEBUG:        DMS_DEBUG = val; break;
        case DebugFlag::IMG_DEBUG:        IMG_DEBUG = val; break;
            
            // Other components
        case DebugFlag::RTC_DEBUG:        RTC_DEBUG = val; break;
        case DebugFlag::KBD_DEBUG:        KBD_DEBUG = val; break;
        case DebugFlag::KEY_DEBUG:        KEY_DEBUG = val; break;
            
            // Misc
        case DebugFlag::RSH_DEBUG:        RSH_DEBUG = val; break;
        case DebugFlag::REC_DEBUG:        REC_DEBUG = val; break;
        case DebugFlag::SCK_DEBUG:        SCK_DEBUG = val; break;
        case DebugFlag::SRV_DEBUG:        SRV_DEBUG = val; break;
        case DebugFlag::GDB_DEBUG:        GDB_DEBUG = val; break;
            
        default:
            throw AppError(Fault::OPT_UNSUPPORTED,
                        "Unhandled debug variable: " + string(DebugFlagEnum::key(flag)));
    }
    
#endif
}

}
