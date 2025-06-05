// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "Emulator.h"

namespace vamiga {

Agnus::Agnus(Amiga& ref) : SubComponent(ref)
{    
    subComponents = std::vector<CoreComponent *> {
        
        &sequencer,
        &copper,
        &blitter,
        &dmaDebugger
    };
}

Agnus&
Agnus::operator= (const Agnus& other) {

    // Clear textures if PAL / NTSC settings do not match
    if (pos.type != other.pos.type) { denise.pixelEngine.clearAll(); }
    
    CLONE(sequencer)
    CLONE(copper)
    CLONE(blitter)
    CLONE(dmaDebugger)
    
    CLONE_ARRAY(trigger)
    CLONE_ARRAY(id)
    CLONE_ARRAY(data)
    CLONE(nextTrigger)
    CLONE(changeRecorder)
    CLONE(syncEvent)

    CLONE(pos)
    CLONE(latchedPos)

    CLONE(bplcon0)
    CLONE(bplcon0Initial)
    CLONE(bplcon1)
    CLONE(bplcon1Initial)
    CLONE(dmacon)
    CLONE(dmaconInitial)
    CLONE(dskpt)
    CLONE_ARRAY(audpt)
    CLONE_ARRAY(audlc)
    CLONE_ARRAY(bplpt)
    CLONE(bpl1mod)
    CLONE(bpl2mod)
    CLONE_ARRAY(sprpt)
    CLONE(res)
    CLONE(scrollOdd)
    CLONE(scrollEven)

    CLONE_ARRAY(busData)
    CLONE_ARRAY(busAddr)
    CLONE_ARRAY(busOwner)
    CLONE_ARRAY(lastCtlWrite)

    CLONE_ARRAY(audxDR)
    CLONE_ARRAY(audxDSR)
    CLONE(bls)

    CLONE_ARRAY(sprVStrt)
    CLONE_ARRAY(sprVStop)
    CLONE_ARRAY(sprDmaEnabled)

    CLONE(clock)

    CLONE(config)
    CLONE(ptrMask)

    return *this;
}

void
Agnus::operator << (SerResetter &worker)
{
    // Remember some events
    auto insEvent = id[SLOT_INS];

    serialize(worker);

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

    // Schedule initial events
    if (isHardResetter(worker)) assert(clock == 0);
    scheduleAbs<SLOT_SEC>(NEVER, SEC_TRIGGER);
    scheduleAbs<SLOT_TER>(NEVER, TER_TRIGGER);
    scheduleAbs<SLOT_CIAA>(CIA_CYCLES(AS_CIA_CYCLES(clock)), CIA_EXECUTE);
    scheduleAbs<SLOT_CIAB>(CIA_CYCLES(AS_CIA_CYCLES(clock)), CIA_EXECUTE);
    scheduleAbs<SLOT_IRQ>(NEVER, IRQ_CHECK);
    diskController.scheduleFirstDiskEvent();
    scheduleFirstBplEvent();
    scheduleFirstDasEvent();
    scheduleRel<SLOT_SRV>(SEC(0.5), SRV_LAUNCH_DAEMON);
    if (insEvent) scheduleRel <SLOT_INS> (0, insEvent);
}

i64
Agnus::getOption(Opt option) const
{
    switch (option) {

        case Opt::AGNUS_REVISION:        return (i64)config.revision;
        case Opt::AGNUS_PTR_DROPS:       return config.ptrDrops;
            
        default:
            fatalError;
    }
}

void
Agnus::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::AGNUS_REVISION:

            if (!isPoweredOff()) {
                throw AppError(Fault::OPT_LOCKED);
            }
            if (!AgnusRevisionEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, AgnusRevisionEnum::keyList());
            }
            return;

        case Opt::AGNUS_PTR_DROPS:

            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
Agnus::setOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::AGNUS_REVISION:

            switch (config.revision = AgnusRevision(value)) {
                    
                case AgnusRevision::OCS_OLD:
                case AgnusRevision::OCS:     ptrMask = 0x07FFFF; break;
                case AgnusRevision::ECS_1MB: ptrMask = 0x0FFFFF; break;
                case AgnusRevision::ECS_2MB: ptrMask = 0x1FFFFF; break;

                default:
                    fatalError;
            }
            mem.updateMemSrcTables();
            return;
            
        case Opt::AGNUS_PTR_DROPS:

            config.ptrDrops = value;
            return;
            
        default:
            fatalError;
    }
}

void
Agnus::setVideoFormat(TV newFormat)
{
    trace(NTSC_DEBUG, "Video format = %s\n", TVEnum::key(newFormat));

    // Change the frame type
    agnus.pos.switchMode(newFormat);

    // Rectify pending events that rely on exact beam positions
    agnus.rectifyVBLEvent();
    
    // Clear frame buffers
    denise.pixelEngine.clearAll();

    // Inform the GUI
    msgQueue.put(Msg::VIDEO_FORMAT, (i64)newFormat);
}

AgnusTraits 
Agnus::getTraits() const
{
    return AgnusTraits {

        .isOCS = isOCS(),
        .isECS = isECS(),
        .isPAL = isPAL(),
        .isNTSC = isNTSC(),
        .idBits = idBits(),
        .chipRamLimit = chipRamLimit(),
        .vStrobeLine = vStrobeLine(),
        .ddfMask = ddfMask()
    };
}

bool
Agnus::isOCS() const
{
    return config.revision == AgnusRevision::OCS_OLD || config.revision == AgnusRevision::OCS;
}

bool
Agnus::isECS() const
{
    return config.revision == AgnusRevision::ECS_1MB || config.revision == AgnusRevision::ECS_2MB;
}

u16
Agnus::idBits() const
{
    switch (config.revision) {
            
        case AgnusRevision::ECS_2MB: return 0x2000; // TODO: CHECK ON REAL MACHINE
        case AgnusRevision::ECS_1MB: return 0x2000;
        default:            return 0x0000;
    }
}

isize
Agnus::chipRamLimit() const
{
    switch (config.revision) {

        case AgnusRevision::ECS_2MB: return 2048;
        case AgnusRevision::ECS_1MB: return 1024;
        default:            return 512;
    }
}

Resolution
Agnus::resolution(u16 v)
{
    if (GET_BIT(v,6) && isECS()) {
        return Resolution::SHRES;
    } else if (GET_BIT(v,15)) {
        return Resolution::HIRES;
    } else {
        return Resolution::LORES;
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
    if (busOwner[pos.h] != BusOwner::NONE) {

        // This variable counts the number of DMA cycles the CPU will be suspended
        DMACycle delay = 0;

        // Execute Agnus until the bus is free
        do {

            execute();
            if (++delay == 2) bls = true;

        } while (busOwner[pos.h] != BusOwner::NONE);

        // Clear the BLS line (Blitter slow down)
        bls = false;

        // Add wait states to the CPU
        cpu.addWaitStates(DMA_CYCLES(delay));
    }

    // Assign bus to the CPU
    busOwner[pos.h] = BusOwner::CPU;
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
Agnus::recordRegisterChange(Cycle delay, RegChange regChange)
{
    // Record the new register value
    changeRecorder.insert(clock + delay, regChange);

    // Schedule the register change
    scheduleNextREGEvent();
}

void
Agnus::recordRegisterChange(Cycle delay, Reg reg, u16 value, Accessor acc)
{
    // Record the new register value
    changeRecorder.insert(clock + delay, RegChange { .reg = reg, .value = value, .accessor = acc } );

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
            if (isDue<SLOT_SNP>(cycle)) {
                amiga.serviceSnpEvent(id[SLOT_KEY]);
            }
            if (isDue<SLOT_RSH>(cycle)) {
                retroShell.serviceEvent();
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
            if (isDue<SLOT_BTR>(cycle)) {
                dmaDebugger.beamtraps.serviceEvent();
            }
            if (isDue<SLOT_ALA>(cycle)) {
                amiga.serviceAlarmEvent();
            }
            if (isDue<SLOT_INS>(cycle)) {
                agnus.serviceINSEvent();
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

        sprDmaEnabled[nr] = false;

        if (!spriteCycleIsBlocked()) {

            // Read in the next control word (POS part)
            if (sprdma()) {

                auto value = doSpriteDmaRead<nr>();
                agnus.pokeSPRxPOS<nr, Accessor::AGNUS>(value);
                denise.pokeSPRxPOS<nr>(value);

            } else {

                busOwner[pos.h] = BusOwner::BLOCKED;
            }
        }

    } else if (sprDmaEnabled[nr]) {

        if (!spriteCycleIsBlocked()) {

            // Read in the next data word (part A)
            if (sprdma()) {
                
                auto value = doSpriteDmaRead<nr>();
                denise.pokeSPRxDATA<nr>(value);
                
            } else {

                busOwner[pos.h] = BusOwner::BLOCKED;
            }
        }
    }
}

template <isize nr> void
Agnus::executeSecondSpriteCycle()
{
    trace(SPR_DEBUG, "executeSecondSpriteCycle<%ld>\n", nr);

    if (pos.v == sprVStop[nr]) {

        sprDmaEnabled[nr] = false;

        if (!spriteCycleIsBlocked()) {

            if (sprdma()) {
                
                // Read in the next control word (CTL part)
                auto value = doSpriteDmaRead<nr>();
                agnus.pokeSPRxCTL<nr, Accessor::AGNUS>(value);
                denise.pokeSPRxCTL<nr>(value);
                
            } else {

                busOwner[pos.h] = BusOwner::BLOCKED;
            }
        }

    } else if (sprDmaEnabled[nr]) {

        if (!spriteCycleIsBlocked()) {

            if (sprdma()) {
                
                // Read in the next data word (part B)
                auto value = doSpriteDmaRead<nr>();
                denise.pokeSPRxDATB<nr>(value);
                
            } else {

                busOwner[pos.h] = BusOwner::BLOCKED;
            }
        }
    }
}

bool 
Agnus::spriteCycleIsBlocked()
{
    if (isOCS()) {
        return sequencer.bprunUp <= pos.h + 1;
    } else {
        return sequencer.bprunUp <= pos.h;
    }
}

void
Agnus::updateSpriteDMA()
{
    // When the function is called, the sprite logic already sees an inremented
    // vertical position counter
    isize v = pos.v + 1;

    // Reset the vertical trigger coordinates in line 25 (PAL) or 20 (NTSC)
    isize resetLine = isPAL() ? 25 : 19;
    if (v == resetLine && sprdma()) {
        for (isize i = 0; i < 8; i++) sprVStop[i] = resetLine;
        return;
    }

    // Disable DMA in the last rasterline
    if (v == pos.vMax()) {
        for (isize i = 0; i < 8; i++) sprDmaEnabled[i] = false;
        return;
    }

    // Update the DMA status for all sprites
    for (isize i = 0; i < 8; i++) {
        if (v == sprVStrt[i]) sprDmaEnabled[i] = true;
        if (v == sprVStop[i]) sprDmaEnabled[i] = false;
    }
}

void
Agnus::eolHandler()
{
    assert(pos.h == PAL::HPOS_CNT || pos.h == NTSC::HPOS_CNT);

    // Pass control to the DMA debugger
    dmaDebugger.eolHandler();

    // Move to the next line
    pos.eol();

    // Update pot counters
    if (paula.chargeX0 < 1.0 || controlPort1.mouse.MMB()) U8_INC(paula.potCntX0, 1);
    if (paula.chargeY0 < 1.0 || controlPort1.mouse.RMB()) U8_INC(paula.potCntY0, 1);
    if (paula.chargeX1 < 1.0 || controlPort2.mouse.MMB()) U8_INC(paula.potCntX1, 1);
    if (paula.chargeY1 < 1.0 || controlPort2.mouse.RMB()) U8_INC(paula.potCntY1, 1);

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
    amiga.eolHandler();
    sequencer.eolHandler();
    denise.eolHandler();

    // Clear the bus usage table
    for (isize i = 0; i < HPOS_CNT; i++) busOwner[i] = BusOwner::NONE;

    // Clear other variables
    for (isize i = 0; i < 8; i++) lastCtlWrite[i] = 0xFF;

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

    scheduleStrobe0Event();

    // Run the screen recorder
    denise.screenRecorder.vsyncHandler(clock - 50 * DMA_CYCLES(PAL::HPOS_CNT));
    denise.eofHandler();

    // Let the other components finish the current frame
    paula.eofHandler();
    sequencer.eofHandler();
    copper.eofHandler();
    ciaa.eofHandler();
    ciab.eofHandler();
    controlPort1.joystick.eofHandler();
    controlPort2.joystick.eofHandler();
    mem.eofHandler();

    // Update statistics
    updateStats();
}

void
Agnus::hsyncHandler()
{
    assert(pos.h == 0x12);
    
    // Draw the previous line
    isize vpos = agnus.pos.vPrev();
    denise.hsyncHandler(vpos);
    dmaDebugger.hsyncHandler(vpos);

    // Encode a LORES marker in the first HBLANK pixel
    REPLACE_BIT(*pixelEngine.workingPtr(vpos), 28, res != Resolution::LORES);

    // Call the vsyncHandler once we've finished a frame
    if (pos.v == 0) vsyncHandler();
}

void
Agnus::vsyncHandler()
{
    denise.vsyncHandler();
    
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

}
