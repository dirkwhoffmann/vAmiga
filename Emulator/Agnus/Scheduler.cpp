// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Scheduler.h"
#include "CIA.h"
#include "ControlPort.h"
#include "CPU.h"
#include "Agnus.h"
#include "Paula.h"
#include "HardDrive.h"
#include "Keyboard.h"
#include "FloppyDrive.h"
#include "IOUtils.h"
#include "RemoteManager.h"
#include <iomanip>


void
Scheduler::_initialize()
{

}

void
Scheduler::_reset(bool hard)
{
    
    RESET_SNAPSHOT_ITEMS(hard)
}

void
Scheduler::executeUntil(Cycle cycle) {

    //
    // Check primary slots
    //

    if (isDue<SLOT_REG>(cycle)) {
        agnus.serviceREGEvent(cycle);
    }
    if (isDue<SLOT_CIAA>(cycle)) {
        ciaa.serviceEvent(scheduler.id[SLOT_CIAA]);
    }
    if (isDue<SLOT_CIAB>(cycle)) {
        ciab.serviceEvent(scheduler.id[SLOT_CIAB]);
    }
    if (isDue<SLOT_BPL>(cycle)) {
        agnus.serviceBPLEvent(scheduler.id[SLOT_BPL]);
    }
    if (isDue<SLOT_DAS>(cycle)) {
        agnus.serviceDASEvent(scheduler.id[SLOT_DAS]);
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
            agnus.serviceVblEvent(scheduler.id[SLOT_VBL]);
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
        if (isDue<SLOT_RAS>(cycle)) {
            agnus.serviceRASEvent();
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
    scheduler.nextTrigger = next;
}
