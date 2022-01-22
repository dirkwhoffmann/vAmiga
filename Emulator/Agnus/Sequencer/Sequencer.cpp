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
    
    clearBplEvents();
    clearDasEvents();
}

void
Sequencer::hsyncHandler()
{
    diwVstrtInitial = diwVstrt;
    diwVstopInitial = diwVstop;

    // Check if we need to recompute all events
    if (ddfInitial != ddf) {
        
        ddfInitial = ddf;
        trace(SEQ_DEBUG, "hsyncHandler: Forcing an event table update\n");
        hsyncActions |= UPDATE_BPL_TABLE;
    }
    
    // Check if we need to reinitialize the signal recorder
    if (sigRecorder.modified ||
        agnus.pos.v == diwVstrt ||
        agnus.pos.v == diwVstop ||
        agnus.inLastRasterline()) {

        trace(SEQ_DEBUG, "hsyncHandler: Forcing a recorder update\n");
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
        if (agnus.pos.v < 25 || agnus.pos.v >= agnus.frame.lastLine()) {
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
            computeBplEvents(sigRecorder);
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
