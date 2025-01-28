// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Keyboard.h"
#include "Agnus.h"
#include "CIA.h"

namespace vamiga {

void
Keyboard::serviceKeyboardEvent(EventID id)
{
    u64 nr = agnus.data[SLOT_KBD];

    switch(id) {
            
        case KBD_TIMEOUT:
            
            trace(KBD_DEBUG, "KBD_TIMEOUT\n");

            // A timeout has occured. Try to resynchronize with the Amiga.
            state = KbState::SYNC;
            execute();
            break;
            
        case KBD_DAT:
            
            trace(KBD_DEBUG, "KBD_DAT [%llu]\n", nr);
            
            if (nr < 8) {
                
                // Put a bit from the shift register onto the SP line
                ciaa.setSP(GET_BIT(shiftReg, 7 - nr));
                agnus.scheduleRel<SLOT_KBD>(USEC(20), KBD_CLK0);

            } else {
                
                // Put the SP line back to normal
                ciaa.setSP(1);
                agnus.scheduleRel<SLOT_KBD>(MSEC(143), KBD_TIMEOUT);
            }
            break;
            
        case KBD_CLK0:

            trace(KBD_DEBUG, "KBD_CLK0 [%llu]\n", nr);

            // Pull the clock line low
            ciaa.emulateFallingEdgeOnCntPin();
            agnus.scheduleRel<SLOT_KBD>(USEC(20), KBD_CLK1);
            break;
            
        case KBD_CLK1:

            trace(KBD_DEBUG, "KBD_CLK1 [%llu]\n", nr);

            // Pull the clock line high
            ciaa.emulateRisingEdgeOnCntPin();
            agnus.scheduleRel<SLOT_KBD>(USEC(20), KBD_DAT, nr + 1);
            break;

        case KBD_SYNC_DAT0:

            trace(KBD_DEBUG, "KBD_SYNC_DAT0\n");
            ciaa.setSP(0);
            agnus.scheduleRel<SLOT_KBD>(USEC(20), KBD_SYNC_CLK0);
            break;

        case KBD_SYNC_CLK0:

            trace(KBD_DEBUG, "KBD_SYNC_CLK0\n");
            ciaa.emulateFallingEdgeOnCntPin();
            agnus.scheduleRel<SLOT_KBD>(USEC(20), KBD_SYNC_CLK1);
            break;
            
        case KBD_SYNC_CLK1:
            
            trace(KBD_DEBUG, "KBD_SYNC_CLK1\n");
            ciaa.emulateRisingEdgeOnCntPin();
            agnus.scheduleRel<SLOT_KBD>(USEC(20), KBD_SYNC_DAT1);
            break;
            
        case KBD_SYNC_DAT1:
            
            trace(KBD_DEBUG, "KBD_SYNC_DAT1\n");
            ciaa.setSP(1);
            agnus.scheduleRel<SLOT_KBD>(MSEC(143), KBD_TIMEOUT);
            break;

        default:
            fatalError;
    }
}

void
Keyboard::serviceKeyEvent()
{
    trace(KEY_DEBUG, "Keyboard::serviceKeyEvent()\n");

    switch(agnus.id[SLOT_KEY]) {

        case KEY_AUTO_TYPE:

            // Process all pending events
            while (!pending.isEmpty()) {

                if (pending.keys[pending.r] > agnus.clock) break;

                auto cmd = pending.read();
                assert(cmd.key.delay == 0.0);
                processCommand(cmd);
            }

            // Schedule next event
            if (pending.isEmpty()) {

                releaseAll();
                agnus.cancel<SLOT_KEY>();

            } else {

                agnus.rescheduleAbs<SLOT_KEY>(pending.keys[pending.r]);
            }
            break;

        default:
            fatalError;
    }
}

}
