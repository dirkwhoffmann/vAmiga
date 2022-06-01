// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Sequencer.h"
#include "Agnus.h"

Sequencer::Sequencer(Amiga& ref) : SubComponent(ref)
{
    initDasEventTable();
}

void
Sequencer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    initBplEvents();
    initDasEvents();
}

void
Sequencer::hsyncHandler()
{
    ddfInitial = ddf;
    
    // Renew the signal recorder if it has been modified
    if (sigRecorder.modified) {

        trace(SEQ_DEBUG, "hsyncHandler: sigRecorder.modified\n");
        hsyncActions |= UPDATE_SIG_RECORDER;
    }

    // Check the vertical DIW flipflop
    if (agnus.pos.v == vstop || agnus.inLastRasterline()) {

        trace(SEQ_DEBUG, "hsyncHandler: Vertical flipflop off\n");
        ddfInitial.bpv = ddf.bpv = false;
        hsyncActions |= UPDATE_SIG_RECORDER;

    } else if (agnus.pos.v == vstrt) {

        trace(SEQ_DEBUG, "hsyncHandler: Vertical flipflop on\n");
        ddfInitial.bpv = ddf.bpv = true;
        hsyncActions |= UPDATE_SIG_RECORDER;
    }
    
    lineIsBlank = !ddfInitial.bpv;

    //
    // Determine the disk, audio and sprite DMA status for the line to come
    //

    u16 newDmaDAS = 0;
    
    if (agnus.dmacon & DMAEN) {

        // Copy DMA enable bits from DMACON
        newDmaDAS = agnus.dmacon & 0b111111;

        // Disable sprites outside the sprite DMA area
        if (agnus.pos.v < 25 || agnus.pos.v >= agnus.pos.vMax()) {
            newDmaDAS &= 0b011111;
        }
    }
    if (newDmaDAS != dmaDAS) {
        
        hsyncActions |= UPDATE_DAS_TABLE;
        dmaDAS = newDmaDAS;
    }

    //
    // Process pending actions
    //

    if (hsyncActions) {

        if (hsyncActions & UPDATE_SIG_RECORDER) {

            hsyncActions &= ~UPDATE_SIG_RECORDER;
            hsyncActions |= UPDATE_BPL_TABLE;
            initSigRecorder();
        }
        if (hsyncActions & UPDATE_BPL_TABLE) {
            
            hsyncActions &= ~UPDATE_BPL_TABLE;
            computeBplEventTable(sigRecorder);
        }
        if (hsyncActions & UPDATE_DAS_TABLE) {
            
            hsyncActions &= ~UPDATE_DAS_TABLE;
            updateDasEvents(dmaDAS);
        }
    }
}

void
Sequencer::vsyncHandler()
{

}
