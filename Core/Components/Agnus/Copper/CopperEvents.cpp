// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Copper.h"
#include "Amiga.h"

namespace vamiga {

void
Copper::serviceEvent()
{
    serviceEvent(agnus.id[SLOT_COP]);
}

void
Copper::serviceEvent(EventID id)
{
    u16 reg;
    
    servicing = true;

    switch (id) {
            
        case COP_REQ_DMA:

            trace(COP_DEBUG, "COP_REQ_DMA\n");
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Don't wake up in an odd cycle
            if (IS_ODD(agnus.pos.h)) { reschedule(); break; }

            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;
            
        case COP_WAKEUP:
            
            trace(COP_DEBUG, "COP_WAKEUP\n");
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }
            
            // Don't wake up in an odd cycle
            if (IS_ODD(agnus.pos.h)) { reschedule(); break; }

            // Check if the wakeup condition is still true
            if (runComparator()) {

                // Continue with fetching the first instruction word
                schedule(COP_FETCH);

            } else {

                // Reschedule the wakeup event
                xfiles("Copper wakeup aborted\n");
                scheduleWaitWakeup(getBFD());
            }
            break;
            
        case COP_WAKEUP_BLIT:
            
            trace(COP_DEBUG, "COP_WAKEUP_BLIT\n");
            
            // Check if the Blitter is busy, keep on waiting
            if (agnus.blitter.isActive()) {
                agnus.scheduleAbs<SLOT_COP>(NEVER, COP_WAIT_BLIT);
                break;
            }
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }
            
            // Don't wake up in an odd cycle
            if (IS_ODD(agnus.pos.h)) { reschedule(); break; }
            
            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;
            
        case COP_FETCH:

            trace(COP_DEBUG, "COP_FETCH\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            if (isSkipCmd()) {
                
                // Set the skip flag if the previous command was a SKIP command
                skip = runComparator();

                // If the BFD flag is cleared, we also need to check the Blitter
                if (!getBFD()) skip &= !agnus.blitter.isActive();
            }

            // Remember the program counter (picked up by the debugger)
            coppc0 = coppc;
            
            // Check if a breakpoint has been reached
            if (checkForBreakpoints && debugger.breakpoints.eval(coppc)) {
                amiga.setFlag(RL::COPPERBP_REACHED);
            }
            
            // Load the first instruction word
            cop1ins = agnus.doCopperDmaRead(coppc);
            advancePC();

            if (COP_CHECKSUM) {
                checkcnt++;
                checksum = util::fnvIt32(checksum, cop1ins);
            }

            // Fork execution depending on the instruction type
            schedule(isMoveCmd() ? COP_MOVE : COP_WAIT_OR_SKIP);
            break;
            
        case COP_MOVE:

            trace(COP_DEBUG, "COP_MOVE\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.doCopperDmaRead(coppc);
            advancePC();

            if (COP_CHECKSUM) checksum = util::fnvIt32(checksum, cop2ins);

            // Extract register number from the first instruction word
            reg = (cop1ins & 0x1FE);

            // Stop the Copper if address is illegal
            if (isIllegalAddress(reg)) { agnus.cancel<SLOT_COP>(); break; }

            // Continue with fetching the new command
            schedule(COP_FETCH);

            // Only proceed if the skip flag is not set
            if (skip) { skip = false; break; }

            // Write value into custom register
            switch (reg) {
                case 0x88:
                    schedule(COP_JMP1);
                    agnus.data[SLOT_COP] = 1;
                    break;
                case 0x8A:
                    schedule(COP_JMP1);
                    agnus.data[SLOT_COP] = 2;
                    break;
                default:
                    move(reg, cop2ins);
            }
            
            // Check if a watchpoint has been reached
            if (checkForWatchpoints && debugger.watchpoints.eval(reg)) {
                amiga.setFlag(RL::COPPERWP_REACHED);
            }

            break;
            
        case COP_WAIT_OR_SKIP:

            trace(COP_DEBUG, "COP_WAIT_OR_SKIP\n");
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.doCopperDmaRead(coppc);
            advancePC();

            if (COP_CHECKSUM) checksum = util::fnvIt32(checksum, cop2ins);

            // Fork execution depending on the instruction type
            schedule(isWaitCmd() ? COP_WAIT1 : COP_SKIP1);
            break;

        case COP_WAIT1:
            
            trace(COP_DEBUG, "COP_WAIT1\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_WAIT2);
            break;

        case COP_WAIT2:

            trace(COP_DEBUG, "COP_WAIT2\n");

            // Clear the skip flag
            skip = false;
            
            // Check if we need to wait for the Blitter
            if (!getBFD() && agnus.blitter.isActive()) {
                agnus.scheduleAbs<SLOT_COP>(NEVER, COP_WAIT_BLIT);
                break;
            }
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Schedule a wakeup event at the target position
            scheduleWaitWakeup(getBFD());
            break;

        case COP_WAIT_BLIT:
            
            trace(COP_DEBUG, "COP_WAIT_BLIT\n");
            
            // Wait for the next free cycle
            if (agnus.busOwner[agnus.pos.h] != BusOwner::NONE &&
                agnus.busOwner[agnus.pos.h] != BusOwner::BLITTER) {
                // debug("COP_WAIT_BLIT delay\n");
                reschedule(); break;
            }
            
            // Schedule a wakeup event at the target position
            scheduleWaitWakeup(false /* BFD */);
            break;

        case COP_SKIP1:

            trace(COP_DEBUG, "COP_SKIP1\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_SKIP2);
            break;

        case COP_SKIP2:

            trace(COP_DEBUG, "COP_SKIP2\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            // Continue with the next command
            schedule(COP_FETCH);
            break;

        case COP_JMP1:

            // The bus is not needed in this cycle, but still allocated
            (void)agnus.allocateBus<BusOwner::COPPER>();

            // In cycle $E0, Copper continues with the next state in $E1 (?!)
            if (agnus.pos.h == 0xE0) {
                schedule(COP_JMP2, 1);
                break;
            }
            schedule(COP_JMP2);
            break;

        case COP_JMP2:

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BusOwner::COPPER>()) { reschedule(); break; }

            switchToCopperList((isize)agnus.data[SLOT_COP]);
            schedule(COP_FETCH);
            break;

        case COP_VBLANK:
            
            // Allocate the bus
            if (agnus.copdma() && !agnus.allocateBus<BusOwner::COPPER>()) { reschedule(); break; }

            switchToCopperList(1);
            activeInThisFrame = agnus.copdma();
            schedule(COP_FETCH);
            break;

        default:
            fatalError;
    }

    servicing = false;
}

void
Copper::schedule(EventID next, int delay)
{
    agnus.scheduleRel <SLOT_COP> (DMA_CYCLES(delay), next);
}

void
Copper::reschedule(int delay)
{
    agnus.rescheduleRel <SLOT_COP> (DMA_CYCLES(delay));
}

}
