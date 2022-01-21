// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "Amiga.h"

Agnus::Agnus(Amiga& ref) : SubComponent(ref)
{    
    subComponents = std::vector<AmigaComponent *> {
        
        &scheduler,
        &sequencer,
        &copper,
        &blitter,
        &dmaDebugger
    };
}

void
Agnus::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Start with a long frame
    frame = Frame();
    
    // Initialize statistical counters
    clearStats();
            
    // Schedule initial events
    scheduleRel<SLOT_SEC>(NEVER, SEC_TRIGGER);
    scheduleRel<SLOT_TER>(NEVER, TER_TRIGGER);
    scheduleRel<SLOT_RAS>(DMA_CYCLES(HPOS_MAX), RAS_HSYNC);
    scheduleRel<SLOT_CIAA>(CIA_CYCLES(AS_CIA_CYCLES(clock)), CIA_EXECUTE);
    scheduleRel<SLOT_CIAB>(CIA_CYCLES(AS_CIA_CYCLES(clock)), CIA_EXECUTE);
    scheduleStrobe0Event();
    scheduleRel<SLOT_IRQ>(NEVER, IRQ_CHECK);
    diskController.scheduleFirstDiskEvent();
    scheduleFirstBplEvent();
    scheduleFirstDasEvent();
    scheduleRel<SLOT_SRV>(SEC(0.5), SRV_LAUNCH_DAEMON);
}

AgnusConfig
Agnus::getDefaultConfig()
{
    AgnusConfig defaults;

    defaults.revision = AGNUS_ECS_1MB;
    defaults.slowRamMirror = true;
        
    return defaults;
}

void
Agnus::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    setConfigItem(OPT_AGNUS_REVISION, defaults.revision);
    setConfigItem(OPT_SLOW_RAM_MIRROR, defaults.slowRamMirror);
}

i64
Agnus::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_AGNUS_REVISION: return config.revision;
        case OPT_SLOW_RAM_MIRROR: return config.slowRamMirror;
            
        default:
            fatalError;
    }
}

void
Agnus::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_AGNUS_REVISION:
                        
            if (!isPoweredOff()) {
                throw VAError(ERROR_OPT_LOCKED);
            }
            if (!AgnusRevisionEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, AgnusRevisionEnum::keyList());
            }            
                                    
            switch (config.revision = (AgnusRevision)value) {
                    
                case AGNUS_OCS_DIP:
                case AGNUS_OCS_PLCC: ptrMask = 0x07FFFF; break;
                case AGNUS_ECS_1MB:  ptrMask = 0x0FFFFF; break;
                case AGNUS_ECS_2MB:  ptrMask = 0x1FFFFF; break;
                
                default:
                    fatalError;
            }
            mem.updateMemSrcTables();
            return;
            
        case OPT_SLOW_RAM_MIRROR:
            
            config.slowRamMirror = value;
            return;
            
        default:
            fatalError;
    }
}

bool
Agnus::isOCS() const
{
    return config.revision == AGNUS_OCS_DIP || config.revision == AGNUS_OCS_PLCC;
}

bool
Agnus::isECS() const
{
    return config.revision == AGNUS_ECS_1MB || config.revision == AGNUS_ECS_2MB;
}

u16
Agnus::idBits()
{
    switch (config.revision) {
            
        case AGNUS_ECS_2MB: return 0x2000; // TODO: CHECK ON REAL MACHINE
        case AGNUS_ECS_1MB: return 0x2000;
        default:            return 0x0000;
    }
}

isize
Agnus::chipRamLimit()
{
    switch (config.revision) {

        case AGNUS_ECS_2MB: return 2048;
        case AGNUS_ECS_1MB: return 1024;
        default:            return 512;
    }
}

bool
Agnus::slowRamIsMirroredIn()
{

    /* The ECS revision of Agnus has a special feature that makes Slow Ram
     * accessible for DMA. In the 512 MB Chip Ram + 512 Slow Ram configuration,
     * Slow Ram is mapped into the second Chip Ram segment. OCS Agnus does not
     * have this feature. It is able to access Chip Ram, only.
     */
    
    if (config.slowRamMirror && isECS()) {
        return mem.chipRamSize() == KB(512) && mem.slowRamSize() == KB(512);
    } else {
        return false;
    }
}

Cycle
Agnus::cyclesInFrame() const
{
    return DMA_CYCLES(frame.numLines() * HPOS_CNT);
}

Cycle
Agnus::startOfFrame() const
{
    return clock - DMA_CYCLES(pos.v * HPOS_CNT + pos.h);
}

Cycle
Agnus::startOfNextFrame() const
{
    return startOfFrame() + cyclesInFrame();
}

bool
Agnus::belongsToPreviousFrame(Cycle cycle) const
{
    return cycle < startOfFrame();
}

bool
Agnus::belongsToCurrentFrame(Cycle cycle) const
{
    return !belongsToPreviousFrame(cycle) && !belongsToNextFrame(cycle);
}

bool
Agnus::belongsToNextFrame(Cycle cycle) const
{
    return cycle >= startOfNextFrame();
}

Cycle
Agnus::beamToCycle(Beam beam) const
{
    return startOfFrame() + DMA_CYCLES(beam.v * HPOS_CNT + beam.h);
}

Beam
Agnus::cycleToBeam(Cycle cycle) const
{
    Beam result;

    Cycle diff = AS_DMA_CYCLES(cycle - startOfFrame());
    assert(diff >= 0);

    result.v = (isize)(diff / HPOS_CNT);
    result.h = (isize)(diff % HPOS_CNT);
    return result;
}

void
Agnus::execute()
{
    // Process pending events
    if (scheduler.nextTrigger <= clock) scheduler.executeUntil(clock);

    // If this assertion hits, the HSYNC event hasn't been served
    assert(pos.h < HPOS_MAX);

    // Advance the internal clock and the horizontal counter
    clock += DMA_CYCLES(1);
    pos.h++;
}

void
Agnus::execute(DMACycle cycles)
{
    for (DMACycle i = 0; i < cycles; i++) execute();
}

void
Agnus::syncWithEClock()
{
    // Check if E clock syncing is disabled
    if (!ciaa.getConfig().eClockSyncing) return;

    /* The E clock is 6 clocks low and 4 clocks high:
     *
     *     |   |   |   |   |   |   |---|---|---|---|
     *     |---|---|---|---|---|---|   |   |   |   |
     *      (4) (5) (6) (7) (8) (9) (0) (1) (2) (3)   (eClk)
     */

    // Determine where we are in the current E clock cycle
    Cycle eClk = (clock >> 2) % 10;
    
    // We want to sync to position (2).
    // If we are already too close, we seek (2) in the next E clock cycle.
    Cycle delay = 0;
    
    switch (eClk) {
            
        case 0: delay = 4 * (2 + 10); break;
        case 1: delay = 4 * (1 + 10); break;
        case 2: delay = 4 * (0 + 10); break;
        case 3: delay = 4 * 9;        break;
        case 4: delay = 4 * 8;        break;
        case 5: delay = 4 * 7;        break;
        case 6: delay = 4 * 6;        break;
        case 7: delay = 4 * (5 + 10); break;
        case 8: delay = 4 * (4 + 10); break;
        case 9: delay = 4 * (3 + 10); break;

        default:
            fatalError;
    }
    
    // Doublecheck that we are going to sync to a DMA cycle
    assert(DMA_CYCLES(AS_DMA_CYCLES(clock + delay)) == clock + delay);
    
    // Execute Agnus until the target cycle has been reached
    execute(AS_DMA_CYCLES(delay));

    // Add wait states to the CPU
    cpu.addWaitStates(delay);
}

void
Agnus::executeUntilBusIsFree()
{    
    isize posh = pos.h == 0 ? HPOS_MAX : pos.h - 1;

    // Check if the bus is blocked
    if (busOwner[posh] != BUS_NONE) {

        // This variable counts the number of DMA cycles the CPU will be suspended
        DMACycle delay = 0;

        // Execute Agnus until the bus is free
        do {
            
            posh = pos.h;
            execute();
            if (++delay == 2) bls = true;
            
        } while (busOwner[posh] != BUS_NONE);

        // Clear the BLS line (Blitter slow down)
        bls = false;

        // Add wait states to the CPU
        cpu.addWaitStates(DMA_CYCLES(delay));
    }

    // Assign bus to the CPU
    busOwner[posh] = BUS_CPU;
}

void
Agnus::executeUntilBusIsFreeForCIA()
{
    // Sync with the E clock driving the CIA
    syncWithEClock();
    
    isize posh = pos.h == 0 ? HPOS_MAX : pos.h - 1;
    
    // Check if the bus is blocked
    if (busOwner[posh] != BUS_NONE) {

        // This variable counts the number of DMA cycles the CPU will be suspended
        DMACycle delay = 0;

        // Execute Agnus until the bus is free
        do {

            posh = pos.h;
            execute();
            if (++delay == 2) bls = true;
            
        } while (busOwner[posh] != BUS_NONE);

        // Clear the BLS line (Blitter slow down)
        bls = false;

        // Add wait states to the CPU
        cpu.addWaitStates(DMA_CYCLES(delay));
    }

    // Assign bus to the CPU
    busOwner[posh] = BUS_CPU;
}

void
Agnus::recordRegisterChange(Cycle delay, u32 addr, u16 value, Accessor acc)
{
    // Record the new register value
    changeRecorder.insert(clock + delay, RegChange { addr, value, (u16)acc } );
    
    // Schedule the register change
    scheduleNextREGEvent();
}

template <isize nr> void
Agnus::executeFirstSpriteCycle()
{
    trace(SPR_DEBUG, "executeFirstSpriteCycle<%d>\n", nr);

    if (pos.v == sprVStop[nr]) {

        sprDmaState[nr] = SPR_DMA_IDLE;

        if (busOwner[pos.h] == BUS_NONE) {

            // Read in the next control word (POS part)
            auto value = doSpriteDmaRead<nr>();
            agnus.pokeSPRxPOS<nr>(value);
            denise.pokeSPRxPOS<nr>(value);
        }

    } else if (sprDmaState[nr] == SPR_DMA_ACTIVE) {

        if (busOwner[pos.h] == BUS_NONE) {

            // Read in the next data word (part A)
            auto value = doSpriteDmaRead<nr>();
            denise.pokeSPRxDATA<nr>(value);
        }
    }
}

template <isize nr> void
Agnus::executeSecondSpriteCycle()
{
    trace(SPR_DEBUG, "executeSecondSpriteCycle<%d>\n", nr);

    if (pos.v == sprVStop[nr]) {

        sprDmaState[nr] = SPR_DMA_IDLE;

        if (busOwner[pos.h] == BUS_NONE) {
            
            // Read in the next control word (CTL part)
            auto value = doSpriteDmaRead<nr>();
            agnus.pokeSPRxCTL<nr>(value);
            denise.pokeSPRxCTL<nr>(value);
        }

    } else if (sprDmaState[nr] == SPR_DMA_ACTIVE) {

        if (busOwner[pos.h] == BUS_NONE) {

            // Read in the next data word (part B)
            auto value = doSpriteDmaRead<nr>();
            denise.pokeSPRxDATB<nr>(value);
        }
    }
}

void
Agnus::updateSpriteDMA()
{
    // When the function is called, the sprite logic already sees an inremented
    // vertical position counter
    isize v = pos.v + 1;

    // Reset the vertical trigger coordinates in line 25
    if (v == 25 && sprdma()) {
        for (isize i = 0; i < 8; i++) sprVStop[i] = 25;
        return;
     }

    // Disable DMA in the last rasterline
    if (v == frame.lastLine()) {
        for (isize i = 0; i < 8; i++) sprDmaState[i] = SPR_DMA_IDLE;
        return;
    }

    // Update the DMA status for all sprites
    for (isize i = 0; i < 8; i++) {
        if (v == sprVStrt[i]) sprDmaState[i] = SPR_DMA_ACTIVE;
        if (v == sprVStop[i]) sprDmaState[i] = SPR_DMA_IDLE;
    }
}

void
Agnus::hsyncHandler()
{
    assert(pos.h == 0);
    
    // Let Denise finish up the current line
    denise.endOfLine(pos.v);

    // Update pot counters
    if (paula.chargeX0 < 1.0) paula.potCntX0++;
    if (paula.chargeY0 < 1.0) paula.potCntY0++;
    if (paula.chargeX1 < 1.0) paula.potCntX1++;
    if (paula.chargeY1 < 1.0) paula.potCntY1++;

    // Transfer DMA requests from Paula to Agnus
    paula.channel0.requestDMA();
    paula.channel1.requestDMA();
    paula.channel2.requestDMA();
    paula.channel3.requestDMA();

    // Advance the vertical counter
    if (++pos.v >= frame.numLines()) vsyncHandler();

    // Save the current value of certain variables
    dmaconInitial = dmacon;
    bplcon0Initial = bplcon0;
    bplcon1Initial = bplcon1;
    
    // Clear the bus usage table
    for (isize i = 0; i < HPOS_CNT; i++) busOwner[i] = BUS_NONE;

    // Pass control to the sequencer
    sequencer.hsyncHandler();
    
    // Schedule the first BPL and DAS events
    scheduleFirstBplEvent();
    scheduleFirstDasEvent();
    
    // Let Denise prepare for the next line
    denise.beginOfLine(pos.v);
}

void
Agnus::vsyncHandler()
{
    // Run the screen recorder
    denise.screenRecorder.vsyncHandler(clock - 50 * DMA_CYCLES(HPOS_CNT));
    
    // Synthesize sound samples
    paula.executeUntil(clock - 50 * DMA_CYCLES(HPOS_CNT));

    // Advance to the next frame
    frame.next(denise.lace());

    // Reset vertical position counter
    pos.v = 0;
            
    // Let other components do their own VSYNC stuff
    sequencer.vsyncHandler();
    copper.vsyncHandler();
    denise.vsyncHandler();
    controlPort1.joystick.vsyncHandler();
    controlPort2.joystick.vsyncHandler();
    retroShell.vsyncHandler();

    // Update statistics
    updateStats();
    mem.updateStats();
    
    // Let the thread synchronize
    amiga.setFlag(RL::SYNC_THREAD);
}

//
// Instantiate template functions
//

template void Agnus::executeFirstSpriteCycle<0>();
template void Agnus::executeFirstSpriteCycle<1>();
template void Agnus::executeFirstSpriteCycle<2>();
template void Agnus::executeFirstSpriteCycle<3>();
template void Agnus::executeFirstSpriteCycle<4>();
template void Agnus::executeFirstSpriteCycle<5>();
template void Agnus::executeFirstSpriteCycle<6>();
template void Agnus::executeFirstSpriteCycle<7>();

template void Agnus::executeSecondSpriteCycle<0>();
template void Agnus::executeSecondSpriteCycle<1>();
template void Agnus::executeSecondSpriteCycle<2>();
template void Agnus::executeSecondSpriteCycle<3>();
template void Agnus::executeSecondSpriteCycle<4>();
template void Agnus::executeSecondSpriteCycle<5>();
template void Agnus::executeSecondSpriteCycle<6>();
template void Agnus::executeSecondSpriteCycle<7>();
