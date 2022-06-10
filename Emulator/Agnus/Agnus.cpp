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
        
        &sequencer,
        &copper,
        &blitter,
        &dmaDebugger
    };
}

void
Agnus::_reset(bool hard)
{
    auto insEvent = id[SLOT_INS];

    RESET_SNAPSHOT_ITEMS(hard)
    
    // Start with a long frame
    pos.lof = true;

    // Adjust to the correct video mode
    setVideoFormat(amiga.getConfig().type);

    // Initialize statistical counters
    clearStats();
            
    // Initialize all event slots
    for (isize i = 0; i < SLOT_COUNT; i++) {
        
        trigger[i] = NEVER;
        id[i] = (EventID)0;
        data[i] = 0;
    }
    
    if (hard) assert(clock == 0);

    // Schedule initial events
    scheduleRel<SLOT_SEC>(NEVER, SEC_TRIGGER);
    scheduleRel<SLOT_TER>(NEVER, TER_TRIGGER);
    scheduleRel<SLOT_CIAA>(CIA_CYCLES(AS_CIA_CYCLES(clock)), CIA_EXECUTE);
    scheduleRel<SLOT_CIAB>(CIA_CYCLES(AS_CIA_CYCLES(clock)), CIA_EXECUTE);
    scheduleRel<SLOT_IRQ>(NEVER, IRQ_CHECK);
    diskController.scheduleFirstDiskEvent();
    scheduleFirstBplEvent();
    scheduleFirstDasEvent();
    scheduleRel<SLOT_SRV>(SEC(0.5), SRV_LAUNCH_DAEMON);
    if (insEvent) scheduleRel <SLOT_INS> (0, insEvent);
}

void
Agnus::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {

        OPT_AGNUS_REVISION,
        OPT_SLOW_RAM_MIRROR,
        OPT_PTR_DROPS
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
Agnus::getConfigItem(Option option) const
{
    switch (option) {

        case OPT_AGNUS_REVISION:    return config.revision;
        case OPT_SLOW_RAM_MIRROR:   return config.slowRamMirror;
        case OPT_PTR_DROPS:         return config.ptrDrops;
            
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
                    
                case AGNUS_OCS_OLD:
                case AGNUS_OCS:     ptrMask = 0x07FFFF; break;
                case AGNUS_ECS_1MB: ptrMask = 0x0FFFFF; break;
                case AGNUS_ECS_2MB: ptrMask = 0x1FFFFF; break;
                
                default:
                    fatalError;
            }
            mem.updateMemSrcTables();
            return;
            
        case OPT_SLOW_RAM_MIRROR:
            
            config.slowRamMirror = value;
            return;

        case OPT_PTR_DROPS:

            config.ptrDrops = value;
            return;
            
        default:
            fatalError;
    }
}

void
Agnus::setVideoFormat(VideoFormat newFormat)
{
    trace(NTSC_DEBUG, "Video format = %s\n", VideoFormatEnum::key(newFormat));

    // Change the frame type
    agnus.pos.switchMode(newFormat);

    // Rectify pending events that rely on exact beam positions
    agnus.rectifyVBLEvent();
    
    // Clear frame buffers
    denise.pixelEngine.clearAll();

    // Inform the GUI
    msgQueue.put(MSG_VIDEO_FORMAT, newFormat);
}

bool
Agnus::isOCS() const
{
    return config.revision == AGNUS_OCS_OLD || config.revision == AGNUS_OCS;
}

bool
Agnus::isECS() const
{
    return config.revision == AGNUS_ECS_1MB || config.revision == AGNUS_ECS_2MB;
}

u16
Agnus::idBits() const
{
    switch (config.revision) {
            
        case AGNUS_ECS_2MB: return 0x2000; // TODO: CHECK ON REAL MACHINE
        case AGNUS_ECS_1MB: return 0x2000;
        default:            return 0x0000;
    }
}

isize
Agnus::chipRamLimit() const
{
    switch (config.revision) {

        case AGNUS_ECS_2MB: return 2048;
        case AGNUS_ECS_1MB: return 1024;
        default:            return 512;
    }
}

bool
Agnus::slowRamIsMirroredIn() const
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

void
Agnus::execute()
{
    // Advance the internal clock and the horizontal counter
    clock += DMA_CYCLES(1);
    pos.h += 1;

    // Process pending events
    if (nextTrigger <= clock) executeUntil(clock);
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
    // If the CPU is overclocked, sync it with Agnus
    cpu.resyncOverclockedCpu();

    // Disable overclocking temporarily
    cpu.slowCycles = 1;
    
    // Check if the bus is blocked
    if (busOwner[pos.h] != BUS_NONE) {

        // This variable counts the number of DMA cycles the CPU will be suspended
        DMACycle delay = 0;

        // Execute Agnus until the bus is free
        do {

            execute();
            if (++delay == 2) bls = true;

        } while (busOwner[pos.h] != BUS_NONE);

        // Clear the BLS line (Blitter slow down)
        bls = false;

        // Add wait states to the CPU
        cpu.addWaitStates(DMA_CYCLES(delay));
    }

    // Assign bus to the CPU
    busOwner[pos.h] = BUS_CPU;
}

void
Agnus::executeUntilBusIsFreeForCIA()
{
    // If the CPU is overclocked, sync it with Agnus
    cpu.resyncOverclockedCpu();

    // Sync with the E clock driving the CIA
    syncWithEClock();
}

void
Agnus::recordRegisterChange(Cycle delay, u32 addr, u16 value, Accessor acc)
{
    // Record the new register value
    changeRecorder.insert(clock + delay, RegChange { addr, value, (u16)acc } );

    // Schedule the register change
    scheduleNextREGEvent();
}

void
Agnus::executeUntil(Cycle cycle) {

    //
    // Check primary slots
    //

    if (isDue<SLOT_REG>(cycle)) {
        agnus.serviceREGEvent(cycle);
    }
    if (isDue<SLOT_CIAA>(cycle)) {
        ciaa.serviceEvent(id[SLOT_CIAA]);
    }
    if (isDue<SLOT_CIAB>(cycle)) {
        ciab.serviceEvent(id[SLOT_CIAB]);
    }
    if (isDue<SLOT_BPL>(cycle)) {
        agnus.serviceBPLEvent(id[SLOT_BPL]);
    }
    if (isDue<SLOT_DAS>(cycle)) {
        agnus.serviceDASEvent(id[SLOT_DAS]);
    }
    if (isDue<SLOT_COP>(cycle)) {
        copper.serviceEvent(id[SLOT_COP]);
    }
    if (isDue<SLOT_BLT>(cycle)) {
        blitter.serviceEvent(id[SLOT_BLT]);
    }

    if (isDue<SLOT_SEC>(cycle)) {

        //
        // Check secondary slots
        //

        if (isDue<SLOT_CH0>(cycle)) {
            paula.channel0.serviceEvent();
        }
        if (isDue<SLOT_CH1>(cycle)) {
            paula.channel1.serviceEvent();
        }
        if (isDue<SLOT_CH2>(cycle)) {
            paula.channel2.serviceEvent();
        }
        if (isDue<SLOT_CH3>(cycle)) {
            paula.channel3.serviceEvent();
        }
        if (isDue<SLOT_DSK>(cycle)) {
            paula.diskController.serviceDiskEvent();
        }
        if (isDue<SLOT_VBL>(cycle)) {
            agnus.serviceVBLEvent(id[SLOT_VBL]);
        }
        if (isDue<SLOT_IRQ>(cycle)) {
            paula.serviceIrqEvent();
        }
        if (isDue<SLOT_KBD>(cycle)) {
            keyboard.serviceKeyboardEvent(id[SLOT_KBD]);
        }
        if (isDue<SLOT_TXD>(cycle)) {
            uart.serviceTxdEvent(id[SLOT_TXD]);
        }
        if (isDue<SLOT_RXD>(cycle)) {
            uart.serviceRxdEvent(id[SLOT_RXD]);
        }
        if (isDue<SLOT_POT>(cycle)) {
            paula.servicePotEvent(id[SLOT_POT]);
        }
        if (isDue<SLOT_IPL>(cycle)) {
            paula.serviceIplEvent();
        }
        if (isDue<SLOT_TER>(cycle)) {

            //
            // Check tertiary slots
            //

            if (isDue<SLOT_DC0>(cycle)) {
                df0.serviceDiskChangeEvent <SLOT_DC0> ();
            }
            if (isDue<SLOT_DC1>(cycle)) {
                df1.serviceDiskChangeEvent <SLOT_DC1> ();
            }
            if (isDue<SLOT_DC2>(cycle)) {
                df2.serviceDiskChangeEvent <SLOT_DC2> ();
            }
            if (isDue<SLOT_DC3>(cycle)) {
                df3.serviceDiskChangeEvent <SLOT_DC3> ();
            }
            if (isDue<SLOT_HD0>(cycle)) {
                hd0.serviceHdrEvent <SLOT_HD0> ();
            }
            if (isDue<SLOT_HD1>(cycle)) {
                hd1.serviceHdrEvent <SLOT_HD1> ();
            }
            if (isDue<SLOT_HD2>(cycle)) {
                hd2.serviceHdrEvent <SLOT_HD2> ();
            }
            if (isDue<SLOT_HD3>(cycle)) {
                hd3.serviceHdrEvent <SLOT_HD3> ();
            }
            if (isDue<SLOT_MSE1>(cycle)) {
                controlPort1.mouse.serviceMouseEvent <SLOT_MSE1> ();
            }
            if (isDue<SLOT_MSE2>(cycle)) {
                controlPort2.mouse.serviceMouseEvent <SLOT_MSE2> ();
            }
            if (isDue<SLOT_KEY>(cycle)) {
                keyboard.serviceKeyEvent();
            }
            if (isDue<SLOT_SRV>(cycle)) {
                remoteManager.serviceServerEvent();
            }
            if (isDue<SLOT_SER>(cycle)) {
                remoteManager.serServer.serviceSerEvent();
            }
            if (isDue<SLOT_INS>(cycle)) {
                agnus.serviceINSEvent(id[SLOT_INS]);
            }

            // Determine the next trigger cycle for all tertiary slots
            Cycle next = trigger[SLOT_TER + 1];
            for (isize i = SLOT_TER + 2; i < SLOT_COUNT; i++) {
                if (trigger[i] < next) next = trigger[i];
            }
            rescheduleAbs<SLOT_TER>(next);
        }
        
        // Determine the next trigger cycle for all secondary slots
        Cycle next = trigger[SLOT_SEC + 1];
        for (isize i = SLOT_SEC + 2; i <= SLOT_TER; i++) {
            if (trigger[i] < next) next = trigger[i];
        }
        rescheduleAbs<SLOT_SEC>(next);
    }

    // Determine the next trigger cycle for all primary slots
    Cycle next = trigger[0];
    for (isize i = 1; i <= SLOT_SEC; i++) {
        if (trigger[i] < next) next = trigger[i];
    }
    nextTrigger = next;
}

template <isize nr> void
Agnus::executeFirstSpriteCycle()
{
    trace(SPR_DEBUG, "executeFirstSpriteCycle<%ld>\n", nr);
    
    if (pos.v == sprVStop[nr]) {

        sprDmaState[nr] = SPR_DMA_IDLE;

        if (busOwner[pos.h] == BUS_NONE) {

            // Read in the next control word (POS part)
            if (sprdma()) {
                
                auto value = doSpriteDmaRead<nr>();
                agnus.pokeSPRxPOS<nr>(value);
                denise.pokeSPRxPOS<nr>(value);
                
            } else {

                busOwner[pos.h] = BUS_BLOCKED;
            }
        }

    } else if (sprDmaState[nr] == SPR_DMA_ACTIVE) {

        if (busOwner[pos.h] == BUS_NONE) {

            // Read in the next data word (part A)
            if (sprdma()) {
                
                auto value = doSpriteDmaRead<nr>();
                denise.pokeSPRxDATA<nr>(value);
                
            } else {

                busOwner[pos.h] = BUS_BLOCKED;
            }
        }
    }
}

template <isize nr> void
Agnus::executeSecondSpriteCycle()
{
    trace(SPR_DEBUG, "executeSecondSpriteCycle<%ld>\n", nr);

    if (pos.v == sprVStop[nr]) {

        sprDmaState[nr] = SPR_DMA_IDLE;

        if (busOwner[pos.h] == BUS_NONE) {

            if (sprdma()) {
                
                // Read in the next control word (CTL part)
                auto value = doSpriteDmaRead<nr>();
                agnus.pokeSPRxCTL<nr>(value);
                denise.pokeSPRxCTL<nr>(value);
                
            } else {

                busOwner[pos.h] = BUS_BLOCKED;
            }
        }

    } else if (sprDmaState[nr] == SPR_DMA_ACTIVE) {

        if (busOwner[pos.h] == BUS_NONE) {

            if (sprdma()) {
                
                // Read in the next data word (part B)
                auto value = doSpriteDmaRead<nr>();
                denise.pokeSPRxDATB<nr>(value);
                
            } else {

                busOwner[pos.h] = BUS_BLOCKED;
            }
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
    if (v == pos.vMax()) {
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
Agnus::eolHandler()
{
    assert(pos.h == HPOS_CNT_PAL || pos.h == HPOS_CNT_NTSC);

    // Pass control to the DMA debugger
    dmaDebugger.eolHandler();

    // Move to the next line
    pos.eol();

    // Update pot counters
    if (paula.chargeX0 < 1.0) U8_INC(paula.potCntX0, 1);
    if (paula.chargeY0 < 1.0) U8_INC(paula.potCntY0, 1);
    if (paula.chargeX1 < 1.0) U8_INC(paula.potCntX1, 1);
    if (paula.chargeY1 < 1.0) U8_INC(paula.potCntY1, 1);

    // Transfer DMA requests from Paula to Agnus
    paula.channel0.requestDMA();
    paula.channel1.requestDMA();
    paula.channel2.requestDMA();
    paula.channel3.requestDMA();

    // Check if we have reached a new frame
    if (pos.v == 0) eofHandler();

    // Save the current value of certain variables
    dmaconInitial = dmacon;
    bplcon0Initial = bplcon0;
    bplcon1Initial = bplcon1;

    // Pass control to other components
    sequencer.eolHandler();
    denise.eolHandler();

    // Clear the bus usage table
    for (isize i = 0; i < HPOS_CNT; i++) busOwner[i] = BUS_NONE;

    // Schedule the first BPL and DAS events
    scheduleFirstBplEvent();
    scheduleFirstDasEvent();
}

void
Agnus::eofHandler()
{
    assert(clock >= 0);
    assert(pos.v == 0);
    assert(denise.lace() == pos.lofToggle);

    // Run the screen recorder
    denise.screenRecorder.vsyncHandler(clock - 50 * DMA_CYCLES(HPOS_CNT_PAL));
    
    // Synthesize sound samples
    paula.executeUntil(clock - 50 * DMA_CYCLES(HPOS_CNT_PAL));

    scheduleStrobe0Event();

    // Let other components do their own VSYNC stuff
    sequencer.eofHandler();
    copper.eofHandler();
    controlPort1.joystick.eofHandler();
    controlPort2.joystick.eofHandler();
    retroShell.eofHandler();

    // Update statistics
    updateStats();
    mem.updateStats();
    
    // Let the thread synchronize
    amiga.setFlag(RL::SYNC_THREAD);
}

void
Agnus::hsyncHandler()
{
    assert(pos.h == 0x12);
    
    // Draw the previous line
    isize vpos = agnus.pos.vPrev();
    denise.hsyncHandler(vpos);
    dmaDebugger.hsyncHandler(vpos);

    // Encode a HIRES / LORES marker in the first HBLANK pixel
    REPLACE_BIT(*pixelEngine.frameBufferAddr(vpos), 28, hires());

    // Call the vsyncHandler once we've finished a frame
    if (pos.v == 0) vsyncHandler();
}

void
Agnus::vsyncHandler()
{
    denise.vsyncHandler();
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
